import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import { GoogleGenerativeAI } from '@google/generative-ai';
import { createClient } from '@supabase/supabase-js';

dotenv.config();

const app = express();
const port = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());

let genAI = null;
let supabase = null;

function getGemini() {
  const apiKey = process.env.GEMINI_API_KEY;
  if (!apiKey) return null;
  if (!genAI) genAI = new GoogleGenerativeAI(apiKey);
  return genAI;
}

function getSupabase() {
  const url = process.env.SUPABASE_URL;
  const key = process.env.SUPABASE_KEY;
  if (!url || !key) return null;
  if (!supabase) supabase = createClient(url, key);
  return supabase;
}

// Health check
app.get('/health', (req, res) => {
  res.json({
    status: 'healthy',
    geminiConfigured: Boolean(process.env.GEMINI_API_KEY),
    supabaseConfigured: Boolean(process.env.SUPABASE_URL && process.env.SUPABASE_KEY),
    timestamp: new Date().toISOString(),
  });
});

// AI Agent endpoint
app.post('/api/agent', async (req, res) => {
  try {
    const { message, userId } = req.body;

    if (!message) {
      return res.status(400).json({ error: 'Message is required' });
    }

    const gemini = getGemini();
    if (!gemini) {
      return res.status(503).json({ error: 'GEMINI_API_KEY is not configured' });
    }

    // Call Gemini AI
    const model = gemini.getGenerativeModel({ model: 'gemini-pro' });
    const result = await model.generateContent(message);
    const aiResponse = result.response.text();

    // Log to Supabase (optional)
    const supabaseClient = getSupabase();
    if (userId && supabaseClient) {
      const { error } = await supabaseClient
        .from('conversations')
        .insert([
          {
            user_id: userId,
            user_message: message,
            ai_response: aiResponse,
            created_at: new Date().toISOString(),
          },
        ]);

      if (error) {
        console.error('Supabase insert error:', error);
      }
    }

    res.json({
      success: true,
      userMessage: message,
      aiResponse: aiResponse,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    console.error('Error:', error);
    res.status(500).json({
      error: 'Failed to process request',
      details: error.message,
    });
  }
});

// List conversations endpoint
app.get('/api/conversations/:userId', async (req, res) => {
  try {
    const { userId } = req.params;
    const supabaseClient = getSupabase();
    if (!supabaseClient) {
      return res.status(503).json({ error: 'Supabase is not configured' });
    }

    const { data, error } = await supabaseClient
      .from('conversations')
      .select('*')
      .eq('user_id', userId)
      .order('created_at', { ascending: false })
      .limit(50);

    if (error) throw error;

    res.json({ success: true, conversations: data || [] });
  } catch (error) {
    console.error('Error:', error);
    res.status(500).json({
      error: 'Failed to fetch conversations',
      details: error.message,
    });
  }
});

// Start server
app.listen(port, () => {
  console.log(`AI Agent Team running on port ${port}`);
  console.log(`Health check: http://localhost:${port}/health`);
});

