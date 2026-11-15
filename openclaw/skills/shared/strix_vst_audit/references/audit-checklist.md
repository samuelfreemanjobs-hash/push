# Strix checklist (DiskLordz)

Full playbook: `docs/security/strix-vst-audit-playbook.md` (repo root).

## License API (when live)

- [ ] IDOR on activate/refresh  
- [ ] Replay / machine binding  
- [ ] Offline license tamper  
- [ ] Rate limits on serial brute force  
- [ ] Plugin fails closed (no audio-thread network)

## Downloads

- [ ] Unauthenticated bucket access blocked  
- [ ] Presigned URL expiry enforced  
- [ ] Download token bound to order  
- [ ] No path traversal in filenames

## C++ / JUCE

- [ ] User WAV size & header validation  
- [ ] Preset/state parse bounds  
- [ ] No alloc in `processBlock` on error paths  
- [ ] pluginval @ strictness 8 after parser changes

## Webhooks

- [ ] Signature verification required  
- [ ] Idempotent event handling  
- [ ] No fulfillment on amount mismatch

Commands: `scripts/strix-pre-release.ps1` · setup: `docs/dev/strix-opt-in.md`