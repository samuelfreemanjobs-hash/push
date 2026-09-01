import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import agentRoutes from './routes/agent.js';
import eaRoutes from './routes/ea.js';
import webhookRoutes from './routes/webhooks.js';

dotenv.config();

const app = express();
const port = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

app.get('/health', (req, res) => {
  res.json({
    status: 'healthy',
    service: 'push-runtime',
    product: process.env.PRIMARY_PRODUCT || 'medflow',
    timestamp: new Date().toISOString(),
  });
});

app.use('/api', agentRoutes);
app.use('/api/ea', eaRoutes);
app.use('/api/webhooks/n8n', webhookRoutes);

app.listen(port, () => {
  console.log(`Push runtime (MedFlow) listening on port ${port}`);
  console.log(`Health: http://localhost:${port}/health`);
  console.log(`EA dashboard: POST http://localhost:${port}/api/ea/dashboard`);
  console.log(`n8n daily: POST http://localhost:${port}/api/webhooks/n8n/ea-daily`);
});
