import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import { GoogleGenerativeAI } from '@google/generative-ai';
import { createClient } from '@supabase/supabase-js';
import { createManuscriptMasterRouter } from './routes/manuscript-master.js';
import { createPmRouter } from './routes/pm.js';

dotenv.config();

const app = express();
const port = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json({ limit: '2mb' }));

// Initialize Gemini AI
const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);

// Initialize Supabase (optional — conversation logging)
const supabase =
  process.env.SUPABASE_URL && process.env.SUPABASE_KEY
    ? createClient(process.env.SUPABASE_URL, process.env.SUPABASE_KEY)
    : null;

// Health check
app.get('/health', (req, res) => {
  res.json({ status: 'healthy', timestamp: new Date().toISOString() });
});

app.use('/api/pm', createPmRouter());

// Manuscript Master — KDP book architect agent
if (process.env.GEMINI_API_KEY) {
  app.use('/api/manuscript-master', createManuscriptMasterRouter(process.env.GEMINI_API_KEY));
}

// AI Agent endpoint
app.post('/api/agent', async (req, res) => {
  try {
    const { message, userId } = req.body;

    if (!message) {
      return res.status(400).json({ error: 'Message is required' });
    }

    // Call Gemini AI
    const model = genAI.getGenerativeModel({ model: 'gemini-pro' });
    const result = await model.generateContent(message);
    const aiResponse = result.response.text();

    // Log to Supabase (optional)
    if (userId && supabase) {
      const { data, error } = await supabase
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
    if (!supabase) {
      return res.status(503).json({
        error: 'Supabase not configured',
        hint: 'Set SUPABASE_URL and SUPABASE_KEY',
      });
    }

    const { userId } = req.params;

    const { data, error } = await supabase
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

