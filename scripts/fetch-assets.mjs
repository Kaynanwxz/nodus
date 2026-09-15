import fs from "node:fs/promises";
import path from "node:path";

const assetsDir = path.join(process.cwd(), "public", "assets");
const required = [
  "hero.webp",
  "left-rail.webp",
  "right-rail.webp",
  "profile.webp",
  "clio.webp",
  "daedalus.webp",
  "scribe.webp",
  "athena.webp",
];

for (const file of required) {
  const fullPath = path.join(assetsDir, file);
  try {
    await fs.access(fullPath);
  } catch {
    throw new Error(`Missing bundled Nodus visual asset: ${fullPath}`);
  }
}

console.log(`[assets] ${required.length} bundled reference assets ready`);
