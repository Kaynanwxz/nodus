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
  "bottom-center.webp",
];

for (const file of required) {
  const fullPath = path.join(assetsDir, file);
  try {
    const data = await fs.readFile(fullPath);
    if (data.length < 20 || data.toString("ascii", 0, 4) !== "RIFF" || data.toString("ascii", 8, 12) !== "WEBP" || data.readUInt32LE(4) + 8 !== data.length) {
      throw new Error(`Invalid WebP image: ${fullPath}`);
    }
  } catch {
    throw new Error(`Missing or invalid bundled Nodus visual asset: ${fullPath}`);
  }
}

console.log(`[assets] ${required.length} bundled reference assets ready`);

