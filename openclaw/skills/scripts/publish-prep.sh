#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MANIFEST="$ROOT/clawhub/manifest.yaml"

node "$ROOT/scripts/verify-skills.mjs"

if [[ "${1:-}" != "--publish" ]]; then
  echo "Verify OK. Dry-run publish commands:"
  node -e "
const fs=require('fs'); const y=require('yaml');
const m=y.parse(fs.readFileSync('$MANIFEST','utf8'));
for (const s of m.skills) {
  console.log('  clawhub skill publish $ROOT/shared/'+s.dir+' --slug '+s.slug+' --version '+m.version+' --dry-run');
}
"
  echo ""
  echo "Run: $0 --publish   (requires clawhub login)"
  exit 0
fi

command -v clawhub >/dev/null || { echo "Install: npm i -g clawhub"; exit 1; }

node -e "
const fs=require('fs'); const y=require('yaml'); const {execSync}=require('child_process');
const m=y.parse(fs.readFileSync('$MANIFEST','utf8'));
for (const s of m.skills) {
  const dir='$ROOT/shared/'+s.dir;
  console.log('Publishing '+s.dir+' as '+s.slug+' ...');
  execSync(
    'clawhub skill publish '+JSON.stringify(dir)+
    ' --slug '+JSON.stringify(s.slug)+
    ' --version '+JSON.stringify(m.version)+
    ' --changelog \"DiskLordz solo-founder skill pack v'+m.version+'\"'+
    ' --tags latest,disklordz,solo-founder',
    {stdio:'inherit'}
  );
}
"