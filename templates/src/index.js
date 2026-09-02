import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
// import { GoogleGenerativeAI } from '@google/generative-ai';
// import Anthropic from '@anthropic-ai/sdk';
// import OpenAI from 'openai';
import { createClient } from '@supabase/supabase-js';

dotenv.config();

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());

// --- Initialize clients ---
// const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);
// const anthropic = new Anthropic({ apiKey: process.env.ANTHROPIC_API_KEY });
// const openai = new OpenAI({ apiKey: process.env.OPENAI_API_KEY });

const supabase = createClient(
  process.env.SUPABASE_URL,
  process.env.SUPABASE_KEY
);

// --- Health check ---
app.get('/health', (req, res) => {
  res.json({ status: 'healthy', timestamp: new Date().toISOString() });
});

// --- Main endpoint ---
// TODO: Replace with your actual endpoint
app.post('/api/[endpoint]', async (req, res) => {
  try {
    const { field1, userId } = req.body;

    if (!field1) {
      return res.status(400).json({ error: 'field1 is required' });
    }

    // TODO: Call AI provider
    // const model = genAI.getGenerativeModel({ model: 'gemini-pro' });
    // const result = await model.generateContent(field1);
    // const aiResponse = result.response.text();

    // TODO: Write to Supabase
    if (userId) {
      const { error } = await supabase
        .from('[table_name]')
        .insert([{ user_id: userId, input: field1, created_at: new Date().toISOString() }]);

      if (error) console.error('Supabase insert error:', error);
    }

    res.json({
      success: true,
      // response: aiResponse,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    console.error('Error:', error);
    res.status(500).json({ error: 'Failed to process request', details: error.message });
  }
});

// --- List endpoint ---
// TODO: Replace with your actual list endpoint
app.get('/api/[endpoint]/:userId', async (req, res) => {
  try {
    const { userId } = req.params;

    const { data, error } = await supabase
      .from('[table_name]')
      .select('*')
      .eq('user_id', userId)
      .order('created_at', { ascending: false })
      .limit(50);

    if (error) throw error;

    res.json({ success: true, items: data || [] });
  } catch (error) {
    console.error('Error:', error);
    res.status(500).json({ error: 'Failed to fetch items', details: error.message });
  }
});

app.listen(port, () => {
  console.log(`[PRODUCT NAME] running on port ${port}`);
  console.log(`Health check: http://localhost:${port}/health`);
});
