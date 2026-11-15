# RAGFlow knowledge base — DSP + business (MCP)

Use RAGFlow as **retrieval backend**, not as your compiler. Connect to Cursor/Claude via **MCP** (when configured) so the coding agent queries manuals without leaving the repo.

**Canvas autoresearch:** `docs/dev/autoresearch-ragflow.md`  
**C++ gates:** `scripts/vst-build-pluginval-gate.ps1`

---

## What to index

### DSP / hardware (agent queries)

| Corpus | Example queries |
|--------|-------------------|
| Julius O. Smith (licensed copies) | State-variable filter stability, bilinear transform |
| **Your** schematic notes | 808-style bridged T, soft-clip variants |
| Internal `TRAP-PHONK-ARCHITECTURE.md` export | Oversampling placement, voice pool |

### Business / product (marketing agent)

| Corpus | Example queries |
|--------|-------------------|
| `vst/LICENSING.md`, `PRODUCT.md` | Tier limits, DRM policy |
| `storefront/` copy, `GEMINI.md` | Hero promise vs shipped features |
| Support FAQ drafts | FL drag export, Lite vs Core |

**Do not index:** pirated PDFs, commercial sample WAVs, stolen schematics.

---

## Suggested datasets (RAGFlow UI)

1. **`dlz-dsp-internal`** — export md/html from this repo (`docs/`, `vst/*.md`, no binaries).  
2. **`dlz-business`** — `PRODUCT.md`, `LICENSING.md`, approved storefront text.  
3. **`dlz-circuit-notes`** — operator-uploaded **original** derivation PDFs.

Chunk size: ~512 tokens; hybrid search on.

---

## MCP wiring (pattern)

Example `mcp.json` entry (host/port per your RAGFlow deploy):

```json
{
  "mcpServers": {
    "ragflow-dsp-brain": {
      "url": "http://localhost:9380/mcp/sse",
      "headers": {
        "Authorization": "Bearer YOUR_RAGFLOW_API_KEY"
      }
    }
  }
}
```

Add the **Hardware Research Rule** from `vst/autoresearch-cpp/program_cpp.md` to your C++ agent when loss plateaus.

Exact steps depend on your RAGFlow version; typical pattern:

1. Enable RAGFlow **MCP server** or HTTP retrieval API.  
2. In Cursor: **Settings → MCP** → add RAGFlow endpoint + API key.  
3. Agent rule (opt-in `.mdc`): *Before DSP refactors on filters/nonlinear, query `dlz-dsp-internal`.*

**Never** let MCP write `PluginProcessor.cpp` without local build + pluginval.

---

## Storefront / support agent (separate canvas)

Optional RAGFlow **Agent** (not Code sandbox):

- **Input:** user question or “draft Core tier bullets”  
- **Retrieve:** `dlz-business` + `PRODUCT.md` chunks  
- **Output:** Lemon/Gumroad copy — **human approve** before publish (Zapier/write rules)

OpenClaw remains cron/intel; RAGFlow is **on-demand** Q&A.

---

## Quick start checklist

- [ ] Deploy RAGFlow (Docker or cloud)  
- [ ] Create datasets above; upload licensed docs only  
- [ ] Test query: “What is Lite drag-export policy?” → must cite `LICENSING.md`  
- [ ] MCP connect to Cursor; run read-only retrieval test  
- [ ] Keep **C++ commits** on Windows with `vst-build-pluginval-gate.ps1`

**Default for your question:** implement **C++ gate script first** (done in repo); add **RAGFlow index** when you have a document corpus worth hosting.