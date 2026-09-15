import fs from "node:fs/promises";
import path from "node:path";
import sharp from "sharp";

const ROOT = process.cwd();
const OUT = path.join(ROOT, "public", "assets");
const CACHE = path.join(ROOT, ".asset-cache");

const commons = (name) => `https://commons.wikimedia.org/wiki/Special:Redirect/file/${encodeURIComponent(name)}`;

const sources = {
  david: commons("Michelangelo Buonarroti, Hoved fra statuen af David, , KAS2232, Statens Museum for Kunst.jpg"),
  victory: commons("Louvre Victoire de Samothrace (Winged Victory) (9811971386).jpg"),
  celestial: commons("John Bevis - celestial map.jpg"),
  eye: commons("Eye (1).jpg"),
  butterfly: commons("Butterfly Image.jpg"),
  clio: commons("Roman Marble Bust of Severan Woman, 220-230 AD (28702800686).jpg"),
  daedalus: commons("Ancient Greece Marble Bust (28738468135).jpg"),
  scribe: commons("Roman Marble Bust of Woman, Trajan Era (28138662634).jpg"),
  athena: commons("Bust of Athena Parthenos, Vatican Museums.jpg"),
};

await fs.mkdir(OUT, { recursive: true });
await fs.mkdir(CACHE, { recursive: true });

async function download(key, url) {
  const file = path.join(CACHE, `${key}.source`);
  try {
    const existing = await fs.readFile(file);
    if (existing.length > 500) return existing;
  } catch {}

  console.log(`[assets] downloading ${key}`);
  const response = await fetch(url, {
    headers: { "User-Agent": "Nodus/0.5 asset builder (github.com/Kaynanwxz/nodus)" },
    redirect: "follow",
  });
  if (!response.ok) throw new Error(`Could not download ${key}: ${response.status} ${response.statusText}`);
  const buffer = Buffer.from(await response.arrayBuffer());
  await fs.writeFile(file, buffer);
  return buffer;
}

const raw = {};
for (const [key, url] of Object.entries(sources)) raw[key] = await download(key, url);

const sepia = (buffer, width, height, fit = "cover", position = "centre") =>
  sharp(buffer)
    .resize(width, height, { fit, position, withoutEnlargement: false })
    .grayscale()
    .tint("#c9a47a")
    .modulate({ brightness: 0.72, saturation: 0.78 })
    .linear(1.08, -5)
    .webp({ quality: 82 })
    .toBuffer();

async function avatar(name, buffer, position = "centre") {
  const out = await sharp(buffer)
    .resize(180, 180, { fit: "cover", position })
    .grayscale()
    .tint("#c7ae8d")
    .modulate({ brightness: 0.82, saturation: 0.70 })
    .webp({ quality: 86 })
    .toBuffer();
  await fs.writeFile(path.join(OUT, `${name}.webp`), out);
}

function svgLayer(width, height, body) {
  return Buffer.from(`<svg width="${width}" height="${height}" xmlns="http://www.w3.org/2000/svg">${body}</svg>`);
}

const celestialHero = await sepia(raw.celestial, 1200, 280, "cover", "centre");
const victoryHero = await sepia(raw.victory, 575, 310, "cover", "centre");
const heroShade = svgLayer(1200, 280, `
  <defs>
    <linearGradient id="g" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="#070604" stop-opacity="0.93"/>
      <stop offset="0.48" stop-color="#070604" stop-opacity="0.55"/>
      <stop offset="1" stop-color="#070604" stop-opacity="0.18"/>
    </linearGradient>
  </defs>
  <rect width="1200" height="280" fill="url(#g)"/>
  <g stroke="#bd8540" stroke-opacity=".28" fill="none">
    <circle cx="830" cy="132" r="102"/><circle cx="830" cy="132" r="76"/>
    ${Array.from({ length: 16 }, (_, i) => {
      const a = (Math.PI * 2 * i) / 16;
      const x = 830 + Math.cos(a) * 144;
      const y = 132 + Math.sin(a) * 144;
      return `<path d="M830 132 L${x.toFixed(1)} ${y.toFixed(1)}"/>`;
    }).join("")}
  </g>
`);

await sharp({ create: { width: 1200, height: 280, channels: 4, background: "#090806" } })
  .composite([
    { input: celestialHero, left: 0, top: 0, blend: "over" },
    { input: victoryHero, left: 625, top: 0, blend: "screen" },
    { input: heroShade, left: 0, top: 0 },
  ])
  .webp({ quality: 84 })
  .toFile(path.join(OUT, "hero.webp"));

const davidRail = await sepia(raw.david, 390, 710, "cover", "north");
const eyeRail = await sepia(raw.eye, 390, 330, "cover", "centre");
const butterfly = await sepia(raw.butterfly, 170, 170, "contain", "centre");
const leftCopy = svgLayer(390, 1120, `
  <rect width="390" height="1120" fill="#070604" fill-opacity=".20"/>
  <rect x="25" y="345" width="300" height="150" fill="#080705" fill-opacity=".86"/>
  <text x="42" y="382" fill="#dfc7a3" font-size="16" letter-spacing="4" font-family="Georgia,serif">INTELLIGENCE</text>
  <text x="42" y="410" fill="#dfc7a3" font-size="16" letter-spacing="4" font-family="Georgia,serif">IN SERVICE</text>
  <text x="42" y="438" fill="#dfc7a3" font-size="16" letter-spacing="4" font-family="Georgia,serif">OF A MORE</text>
  <text x="42" y="466" fill="#dfc7a3" font-size="16" letter-spacing="4" font-family="Georgia,serif">HUMAN MIND.</text>
  <rect x="30" y="817" width="260" height="126" rx="3" fill="#c7a77c" fill-opacity=".90"/>
  <text x="54" y="854" fill="#211911" font-size="15" letter-spacing="4" font-family="Georgia,serif">CURIOSITY</text>
  <text x="54" y="882" fill="#211911" font-size="15" letter-spacing="4" font-family="Georgia,serif">CREATES</text>
  <text x="54" y="910" fill="#211911" font-size="15" letter-spacing="4" font-family="Georgia,serif">BETTER WORLDS.</text>
`);

await sharp({ create: { width: 390, height: 1120, channels: 4, background: "#070604" } })
  .composite([
    { input: davidRail, left: 0, top: 0, blend: "screen" },
    { input: butterfly, left: 155, top: 485, blend: "screen" },
    { input: eyeRail, left: 0, top: 610, blend: "screen" },
    { input: leftCopy, left: 0, top: 0 },
  ])
  .webp({ quality: 84 })
  .toFile(path.join(OUT, "left-rail.webp"));

const athenaRail = await sepia(raw.athena, 360, 650, "cover", "north");
const mapRail = await sepia(raw.celestial, 360, 1120, "cover", "centre");
const rightCopy = svgLayer(360, 1120, `
  <rect width="360" height="1120" fill="#070604" fill-opacity=".48"/>
  <rect x="44" y="130" width="258" height="132" fill="#c8aa7e" fill-opacity=".92"/>
  <text x="68" y="170" fill="#211911" font-size="15" letter-spacing="3.5" font-family="Georgia,serif">SAME QUESTIONS.</text>
  <text x="68" y="199" fill="#211911" font-size="15" letter-spacing="3.5" font-family="Georgia,serif">DEEPER ANSWERS.</text>
  <rect x="82" y="848" width="210" height="180" fill="#080705" fill-opacity=".88"/>
  <text x="106" y="890" fill="#d9c29f" font-size="15" letter-spacing="4" font-family="Georgia,serif">THINK</text>
  <text x="106" y="919" fill="#d9c29f" font-size="15" letter-spacing="4" font-family="Georgia,serif">BUILD</text>
  <text x="106" y="948" fill="#d9c29f" font-size="15" letter-spacing="4" font-family="Georgia,serif">DELEGATE</text>
  <text x="106" y="977" fill="#d9c29f" font-size="15" letter-spacing="4" font-family="Georgia,serif">REFINE</text>
  <text x="106" y="1006" fill="#d9c29f" font-size="15" letter-spacing="4" font-family="Georgia,serif">REPEAT</text>
`);

await sharp({ create: { width: 360, height: 1120, channels: 4, background: "#070604" } })
  .composite([
    { input: mapRail, left: 0, top: 0, blend: "screen" },
    { input: athenaRail, left: 0, top: 450, blend: "screen" },
    { input: rightCopy, left: 0, top: 0 },
  ])
  .webp({ quality: 84 })
  .toFile(path.join(OUT, "right-rail.webp"));

const eyeBottom = await sepia(raw.eye, 860, 180, "cover", "centre");
const mapBottom = await sepia(raw.celestial, 1200, 180, "cover", "south");
await sharp({ create: { width: 1200, height: 180, channels: 4, background: "#070604" } })
  .composite([
    { input: mapBottom, left: 0, top: 0, blend: "screen" },
    { input: eyeBottom, left: 170, top: 0, blend: "screen" },
    { input: svgLayer(1200, 180, `<rect width="1200" height="180" fill="#070604" fill-opacity=".28"/>`), left: 0, top: 0 },
  ])
  .webp({ quality: 80 })
  .toFile(path.join(OUT, "bottom-center.webp"));

await avatar("clio", raw.clio, "north");
await avatar("daedalus", raw.daedalus, "north");
await avatar("scribe", raw.scribe, "north");
await avatar("athena", raw.athena, "north");
await avatar("profile", raw.david, "north");

console.log(`[assets] generated collage assets in ${OUT}`);
