# 15-Minute Product Creation SOP

A repeatable system for shipping a working product fast. Each section has the exact prompt to paste into Claude Code, Cursor, or wherever you're building.

---

## Before You Start — Answer These 3 Questions

Write your answers down before touching any tool.

1. **What does it do?** (one sentence, no jargon)
2. **Who is it for?** (be specific — not "developers", say "solo founders who need X")
3. **What's the simplest version that proves it works?** (one endpoint, one page, one function)

> **Example from the AI Agent Team build:**
> 1. It lets users send messages to an AI and stores the conversation history.
> 2. Developers building their first AI-powered app.
> 3. One POST endpoint that calls Gemini and saves to Supabase.

---

## PHASE 1 — Define (2 min)

**Goal:** Lock the stack and project name before writing a single line.

### Decisions to make:

| Decision | Options | Chose for AI Agent Team |
|----------|---------|------------------------|
| Language | Node.js / Python / Go | Node.js (ESM) |
| AI provider | Gemini / Claude / OpenAI | Gemini (free tier) |
| Database | Supabase / PlanetScale / Mongo | Supabase (Postgres + free tier) |
| Hosting | Railway / Render / Fly.io | Railway (auto-deploy from GitHub) |
| Framework | Express / Fastify / Hono | Express |

### Prompt — use this in Claude to finalize the stack:

```
I'm building [PRODUCT NAME]. It does [ONE SENTENCE DESCRIPTION].

Users will [PRIMARY ACTION]. The MVP needs exactly [N] endpoints/screens/functions.

Recommend a stack from these constraints:
- Must deploy in under 5 minutes on Railway
- Free tier on all services if possible
- Node.js unless there's a strong reason not to

Give me: project name, folder name, stack choice, and 3 bullet points on what the MVP includes. Nothing else.
```

---

## PHASE 2 — Scaffold (3 min)

**Goal:** Generate the full folder structure and boilerplate in one shot.

### Prompt — paste into Claude Code:

```
Create a new Node.js project called [FOLDER-NAME] with this structure:

[FOLDER-NAME]/
  src/
    index.js       ← main server file
  .env.example     ← all required env vars with placeholder values
  .gitignore       ← node_modules, .env
  package.json     ← ESM ("type": "module"), scripts: start + dev, engines: node >=22
  README.md        ← what it does, endpoints, env vars, Railway deploy steps

Stack:
- Express 4
- [AI PACKAGE] for AI calls
- @supabase/supabase-js for database
- cors, dotenv

The server should:
1. [ENDPOINT 1 — describe it]
2. [ENDPOINT 2 — describe it]
3. GET /health — returns { status: "healthy", timestamp }

Initialize Supabase and [AI CLIENT] from env vars at startup.
Add error handling on every async route.
Use console.error for errors, never throw unhandled.
```

---

## PHASE 3 — Build Core Logic (5 min)

**Goal:** Wire up the real logic. One prompt per endpoint.

### Prompt — for each endpoint:

```
In [FOLDER-NAME]/src/index.js, implement the [ENDPOINT NAME] endpoint:

Route: [METHOD] [PATH]
Input: [what comes in — body, params, query]
Steps:
  1. Validate [required fields]
  2. Call [AI/DB/service] with [specific call]
  3. [If DB write] Insert row to [table] with fields: [list fields]
  4. Return [response shape]

Error cases to handle:
- Missing [field] → 400 with message "[error text]"
- [Service] failure → 500 with { error, details }

Don't change anything outside this route.
```

### Prompt — Supabase table setup (run this once):

```
Write the SQL to create a [TABLE NAME] table in Supabase for this use case:
[describe what data is stored]

Requirements:
- id: uuid primary key, default gen_random_uuid()
- created_at: timestamptz, default now()
- [other fields based on your product]
- Add an index on [field used for filtering]

Output only the SQL, nothing else.
```

---

## PHASE 4 — Context Files (2 min)

**Goal:** Set up the three context files so every tool knows the project.
Copy the templates from `templates/` and fill in your project details.

### CLAUDE file — tells Claude Code what this project is:

```
Copy templates/CLAUDE → [FOLDER-NAME]/CLAUDE
Fill in: project name, what it does, stack, key files, rules
```

### CURSOR file — tells Cursor what this project is:

```
Copy templates/CURSOR → [FOLDER-NAME]/CURSOR
Fill in: same info as CLAUDE, plus any Cursor-specific preferences
```

### LINEAR file — sets up your task board:

```
Copy templates/LINEAR → [FOLDER-NAME]/LINEAR
Fill in: project name, initial tickets for what's left after MVP
```

---

## PHASE 5 — Deploy (3 min)

**Goal:** Live URL in 3 minutes.

### Step-by-step:

1. **Push to GitHub**
   ```bash
   git add .
   git commit -m "Initial [PRODUCT NAME] setup"
   git push -u origin main
   ```

2. **Railway setup** (railway.app)
   - New Project → Deploy from GitHub repo
   - Set root directory to `[FOLDER-NAME]`
   - Add all env vars from `.env.example` (with real values)
   - Deploy

3. **Supabase setup** (supabase.com)
   - New project
   - SQL Editor → paste your table SQL from Phase 3
   - Settings → API → copy URL and anon key → paste into Railway env vars

4. **Verify**
   ```bash
   curl https://[YOUR-RAILWAY-URL]/health
   # Should return: {"status":"healthy","timestamp":"..."}
   ```

### Prompt — if Railway deploy fails:

```
My Railway deploy failed. Here's the error log:

[PASTE ERROR LOG]

The project is a Node.js ESM app. Root directory is [FOLDER-NAME].
The start script in package.json is: [PASTE SCRIPT]
Node engine requirement: [PASTE ENGINES FIELD]

What's wrong and what's the exact fix?
```

---

## PHASE 6 — Validate (30 seconds)

**Goal:** Confirm it actually works end to end before calling it done.

```bash
# Test the main endpoint
curl -X POST https://[YOUR-URL]/api/[ENDPOINT] \
  -H "Content-Type: application/json" \
  -d '{"[field]": "[test value]", "[field2]": "test-user"}'

# Check Supabase — go to Table Editor and verify a row was inserted
```

If it works: you're done. Ship it.

---

## Reuse Checklist

Run through this every time:

- [ ] Answered the 3 questions (what, who, simplest version)
- [ ] Locked the stack (Phase 1 prompt)
- [ ] Scaffolded the project (Phase 2 prompt)
- [ ] Built each endpoint (Phase 3 prompts)
- [ ] Created SQL for Supabase tables (Phase 3 SQL prompt)
- [ ] Filled in CLAUDE, CURSOR, LINEAR context files (Phase 4)
- [ ] Pushed to GitHub and deployed on Railway (Phase 5)
- [ ] Hit the health endpoint to confirm it's live (Phase 6)

**Total: ~15 minutes.**

---

## Adapting for Different Products

### Swapping the AI provider

| Provider | Package | Init code |
|----------|---------|-----------|
| Gemini | `@google/generative-ai` | `new GoogleGenerativeAI(key)` |
| Claude | `@anthropic-ai/sdk` | `new Anthropic({ apiKey: key })` |
| OpenAI | `openai` | `new OpenAI({ apiKey: key })` |

### Swapping the database

| DB | Package | Notes |
|----|---------|-------|
| Supabase | `@supabase/supabase-js` | Postgres, free tier, auth built in |
| PlanetScale | `@planetscale/database` | MySQL, serverless driver |
| Upstash Redis | `@upstash/redis` | Key-value, great for sessions/queues |

### Adding auth

Paste this prompt in Phase 3 after your core endpoints:

```
Add a simple API key auth middleware to [FOLDER-NAME]/src/index.js.

- Read VALID_API_KEYS from env (comma-separated list)
- Check Authorization: Bearer [key] header on all /api/* routes
- Return 401 { error: "Unauthorized" } if missing or invalid
- Skip auth on GET /health

Don't touch any existing route logic.
```

---

## Troubleshooting Common Failures

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| Supabase insert silently fails | Table doesn't exist | Run the SQL from Phase 3 |
| `Cannot use import statement` | Missing `"type": "module"` in package.json | Add it |
| Railway crashes on start | Node version too old | Add `"engines": { "node": ">=22" }` |
| 500 on AI calls | API key not set in Railway env vars | Add env var in Railway dashboard |
| CORS errors from browser | cors() not applied before routes | Move `app.use(cors())` to top |
