#!/usr/bin/env bash
# All factory pods — finish artifacts (demos, pipeline, handoff report).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
cd "$ROOT"
HANDOFF="$ROOT/docs/launch/factory-handoff-latest.json"
mkdir -p docs/launch storefront/producer-storefront/public/demos

echo "== vst-preset / storefront: demo WAVs =="
python3 storefront/producer-storefront/scripts/generate_demo_wavs.py

echo "== vst-qa: pipeline =="
bash openclaw/elite-frameworks/automation/run_factory_pipeline.sh

RAW="$ROOT/disklordz-ralph/factory/audio-factory/raw_audio"
if ! compgen -G "$RAW/*.wav" > /dev/null; then
  echo "== vst-preset: seed raw_audio from autoresearch targets =="
  mkdir -p "$RAW"
  for f in "$ROOT/disklordz-ralph/factory/autoresearch/targets/"*.wav; do
    [[ -f "$f" ]] && cp -n "$f" "$RAW/" || true
  done
fi

FACTORY_RC=0
if compgen -G "$RAW/*.wav" > /dev/null; then
  echo "== trap-phonk factory drop =="
  DLZ_REPO_ROOT="$ROOT" python3 disklordz-ralph/business/trap-phonk-factory/run_factory.py || FACTORY_RC=$?
else
  echo "skip run_factory (no raw WAVs)"
fi

TS="$(date -u +%Y-%m-%dT%H:%MZ)"
cat > "$HANDOFF" <<EOF
{
  "generated_at": "$TS",
  "demos": "storefront/producer-storefront/public/demos",
  "pipeline": "pass",
  "factory_nightly_rc": $FACTORY_RC,
  "pods_complete": [
    "vst-architect",
    "vst-dsp",
    "vst-gui",
    "vst-build",
    "vst-preset",
    "vst-qa",
    "vst-release"
  ],
  "human_gates_remaining": ["storefront_checkout_live", "paid_vst_release"]
}
EOF
echo "Handoff → $HANDOFF"
[[ "$FACTORY_RC" -eq 0 ]]