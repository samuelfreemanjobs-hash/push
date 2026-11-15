#!/usr/bin/env node
/**
 * Validates tripwire QC manifest JSON (factory milestone M1·3).
 * Usage: node scripts/validate-qc-manifest.mjs path/to/qc-manifest.json
 */
import { readFileSync } from "node:fs";
import schema from "../handoff/qc-manifest.schema.json" with { type: "json" };

const path = process.argv[2];
if (!path) {
  console.error("Usage: node validate-qc-manifest.mjs <qc-manifest.json>");
  process.exit(2);
}

let data;
try {
  data = JSON.parse(readFileSync(path, "utf8"));
} catch (e) {
  console.error("JSON parse error:", e.message);
  process.exit(1);
}

const errors = [];

function checkRequired(obj, keys, prefix = "") {
  for (const key of keys) {
    if (obj[key] === undefined || obj[key] === null) {
      errors.push(`${prefix}missing ${key}`);
    }
  }
}

checkRequired(data, schema.required);

if (data.format !== undefined && data.format !== "wav") {
  errors.push('format must be "wav"');
}

const statusEnum = schema.properties.qc_status.enum;
if (data.qc_status && !statusEnum.includes(data.qc_status)) {
  errors.push(`qc_status must be one of: ${statusEnum.join(", ")}`);
}

if (Array.isArray(data.samples)) {
  data.samples.forEach((s, i) => {
    checkRequired(s, ["id", "path", "peak_db", "duration_ms"], `samples[${i}].`);
    if (typeof s.peak_db === "number" && s.peak_db > 0) {
      errors.push(`samples[${i}].peak_db must be <= 0 dBFS`);
    }
  });
} else if (data.samples !== undefined) {
  errors.push("samples must be an array");
}

if (errors.length) {
  console.error("Invalid qc-manifest:");
  errors.forEach((e) => console.error(" -", e));
  process.exit(1);
}

console.log(
  "OK",
  data.kit_id,
  "v" + data.version,
  "qc_status=" + data.qc_status,
  "samples=" + (data.samples?.length ?? 0),
);
process.exit(0);