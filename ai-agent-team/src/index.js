import { randomUUID } from "node:crypto";
import express from "express";
import cors from "cors";
import dotenv from "dotenv";
import { GoogleGenerativeAI } from "@google/generative-ai";
import { createClient } from "@supabase/supabase-js";
import { listEliteAgents } from "./agents/eliteRoster.js";
import { skillLineForAgent } from "./agents/openclawSkillsMap.js";
import { modelForAgent, resolveAgent } from "./agents/router.js";
import { dispatchWorkflow } from "./github/workflowDispatch.js";
import {
  formatTaskUserMessage,
  withTaskSystem,
} from "./tasks/runAgentTask.js";
import { ollamaGenerate, shouldUseOllama } from "./providers/ollama.js";
import { buildConnectionStatus } from "./connections/status.js";
import { pullVaultOpsFragment } from "./github/vaultOps.js";

dotenv.config();

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(express.json({ limit: "1mb" }));

const genAI = process.env.GEMINI_API_KEY
  ? new GoogleGenerativeAI(process.env.GEMINI_API_KEY)
  : null;

const supabase =
  process.env.SUPABASE_URL && process.env.SUPABASE_KEY
    ? createClient(process.env.SUPABASE_URL, process.env.SUPABASE_KEY)
    : null;

app.get("/health", (_req, res) => {
  res.json({
    status: "healthy",
    gemini: Boolean(genAI),
    ollama: Boolean(process.env.OLLAMA_BASE_URL),
    defaultCodingAgent: "code",
    timestamp: new Date().toISOString(),
  });
});

app.get("/api/agents", (_req, res) => {
  res.json({ success: true, agents: listEliteAgents() });
});

app.get("/api/connections/status", async (_req, res) => {
  try {
    const status = await buildConnectionStatus();
    res.json(status);
  } catch (error) {
    console.error("connections/status:", error);
    res.status(500).json({
      error: "Connection check failed",
      details: error.message,
    });
  }
});

app.get("/api/vault/ops-fragment", async (_req, res) => {
  try {
    const fragment = await pullVaultOpsFragment();
    if (!fragment.kanban.cards.length && fragment.syncErrors.length) {
      return res.status(503).json({
        error: "Vault not reachable",
        details: fragment.syncErrors.join("; "),
      });
    }
    res.json(fragment);
  } catch (error) {
    console.error("vault/ops-fragment:", error);
    res.status(500).json({
      error: "Vault pull failed",
      details: error.message,
    });
  }
});

async function generateReply(agent, userMessage) {
  const system = agent.system + skillLineForAgent(agent.id);
  const prompt = userMessage;

  if (shouldUseOllama(agent.id)) {
    return ollamaGenerate({
      baseUrl: process.env.OLLAMA_BASE_URL,
      model: process.env.OLLAMA_MODEL,
      system,
      user: prompt,
    });
  }

  if (!genAI) {
    throw new Error(
      "GEMINI_API_KEY not set — add a free key from Google AI Studio, or set OLLAMA_BASE_URL + OLLAMA_MODEL",
    );
  }

  const modelName = modelForAgent(agent);
  const model = genAI.getGenerativeModel({
    model: modelName,
    systemInstruction: system,
  });
  const result = await model.generateContent(prompt);
  return result.response.text();
}

app.post("/api/agent", async (req, res) => {
  try {
    const { message, userId, agentId } = req.body;

    if (!message || typeof message !== "string") {
      return res.status(400).json({ error: "message is required" });
    }

    const { agent, message: routedMessage, matchedBy } = resolveAgent(
      message,
      agentId,
    );
    const aiResponse = await generateReply(agent, routedMessage);

    if (userId && supabase) {
      const { error } = await supabase.from("conversations").insert([
        {
          user_id: userId,
          agent_id: agent.id,
          user_message: message,
          ai_response: aiResponse,
          created_at: new Date().toISOString(),
        },
      ]);
      if (error) console.error("Supabase insert error:", error);
    }

    res.json({
      success: true,
      agent: {
        id: agent.id,
        prefix: agent.prefix,
        displayName: agent.displayName,
        matchedBy,
      },
      userMessage: message,
      aiResponse,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    console.error("Error:", error);
    res.status(500).json({
      error: "Failed to process request",
      details: error.message,
    });
  }
});

app.post("/api/tasks/run", async (req, res) => {
  try {
    const {
      title,
      instructions,
      agentId,
      taskId,
      kanbanCardId,
      userId,
    } = req.body ?? {};

    if (!title || !instructions) {
      return res
        .status(400)
        .json({ error: "title and instructions are required" });
    }

    const userMessage = formatTaskUserMessage({
      title,
      instructions,
      kanbanCardId,
    });
    const { agent, message: routedMessage, matchedBy } = resolveAgent(
      userMessage,
      agentId,
    );

    const system = withTaskSystem(agent.system + skillLineForAgent(agent.id));
    let aiResponse;

    if (shouldUseOllama(agent.id)) {
      aiResponse = await ollamaGenerate({
        baseUrl: process.env.OLLAMA_BASE_URL,
        model: process.env.OLLAMA_MODEL,
        system,
        user: routedMessage,
      });
    } else if (!genAI) {
      throw new Error("GEMINI_API_KEY or Ollama required");
    } else {
      const model = genAI.getGenerativeModel({
        model: modelForAgent(agent),
        systemInstruction: system,
      });
      const result = await model.generateContent(routedMessage);
      aiResponse = result.response.text();
    }

    if (userId && supabase) {
      const { error } = await supabase.from("agent_tasks").insert([
        {
          id: taskId ?? randomUUID(),
          user_id: userId,
          agent_id: agent.id,
          title,
          instructions,
          output: aiResponse,
          status: "completed",
          created_at: new Date().toISOString(),
        },
      ]);
      if (error) console.error("agent_tasks insert:", error.message);
    }

    res.json({
      success: true,
      taskId,
      output: aiResponse,
      aiResponse,
      agent: {
        id: agent.id,
        prefix: agent.prefix,
        displayName: agent.displayName,
        matchedBy,
      },
    });
  } catch (error) {
    console.error("tasks/run:", error);
    res.status(500).json({
      error: "Task execution failed",
      details: error.message,
    });
  }
});

app.post("/api/github/workflow-dispatch", async (req, res) => {
  try {
    const { workflowFile, ref, inputs } = req.body ?? {};
    if (!workflowFile || !ref) {
      return res
        .status(400)
        .json({ error: "workflowFile and ref are required" });
    }
    const result = await dispatchWorkflow({ workflowFile, ref, inputs });
    res.json({ success: true, ...result });
  } catch (error) {
    console.error("workflow-dispatch:", error);
    res.status(500).json({
      error: "Workflow dispatch failed",
      details: error.message,
    });
  }
});

app.get("/api/conversations/:userId", async (req, res) => {
  try {
    if (!supabase) {
      return res.json({ success: true, conversations: [] });
    }
    const { userId } = req.params;
    const { data, error } = await supabase
      .from("conversations")
      .select("*")
      .eq("user_id", userId)
      .order("created_at", { ascending: false })
      .limit(50);

    if (error) throw error;
    res.json({ success: true, conversations: data || [] });
  } catch (error) {
    console.error("Error:", error);
    res.status(500).json({
      error: "Failed to fetch conversations",
      details: error.message,
    });
  }
});

app.listen(port, () => {
  console.log(`AI Agent Team (elite roster) on port ${port}`);
  console.log(`Agents: GET /api/agents`);
  console.log(`Chat: POST /api/agent  — try "CODE: explain useEffect"`);
});