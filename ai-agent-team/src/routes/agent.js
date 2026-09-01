import { Router } from 'express';
import { GoogleGenerativeAI } from '@google/generative-ai';
import { requireSupabase } from '../lib/supabase.js';

const router = Router();

router.post('/agent', async (req, res) => {
  try {
    const { message, userId } = req.body;

    if (!message) {
      return res.status(400).json({ error: 'Message is required' });
    }

    if (!process.env.GEMINI_API_KEY) {
      return res.status(503).json({ error: 'GEMINI_API_KEY not configured' });
    }

    const genAI = new GoogleGenerativeAI(process.env.GEMINI_API_KEY);
    const model = genAI.getGenerativeModel({ model: 'gemini-pro' });
    const result = await model.generateContent(message);
    const aiResponse = result.response.text();

    if (userId) {
      const db = requireSupabase();
      const { error } = await db.from('conversations').insert([
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
      aiResponse,
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

router.get('/conversations/:userId', async (req, res) => {
  try {
    const { userId } = req.params;
    const db = requireSupabase();

    const { data, error } = await db
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

export default router;
