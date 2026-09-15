# Nodus — Asset Sources

A versão 0.5 usa imagens clássicas reais como matéria-prima visual. O build baixa as imagens abaixo, aplica tratamento sépia/dark e converte os resultados para WebP antes de empacotar o aplicativo.

A intenção é usar apenas material indicado como domínio público/CC0 nas páginas de origem. Antes de redistribuir comercialmente uma versão futura, revise novamente cada página de licença, pois metadados de acervo podem mudar.

## Fontes

### Michelangelo — David

Usado no rail esquerdo e no medallion/profile.

- Wikimedia Commons: `Michelangelo Buonarroti, Hoved fra statuen af David, , KAS2232, Statens Museum for Kunst.jpg`
- https://commons.wikimedia.org/wiki/File:Michelangelo_Buonarroti,_Hoved_fra_statuen_af_David,_,_KAS2232,_Statens_Museum_for_Kunst.jpg

### Winged Victory of Samothrace

Usado na composição do hero.

- Wikimedia Commons: `Louvre Victoire de Samothrace (Winged Victory) (9811971386).jpg`
- https://commons.wikimedia.org/wiki/File:Louvre_Victoire_de_Samothrace_(Winged_Victory)_(9811971386).jpg

### John Bevis — celestial map

Usado no hero, rail direito e composição inferior.

- Wikimedia Commons: `John Bevis - celestial map.jpg`
- https://commons.wikimedia.org/wiki/File:John_Bevis_-_celestial_map.jpg

### Eye illustration

Usado como gravura/fragmento visual.

- Wikimedia Commons: `Eye (1).jpg`
- https://commons.wikimedia.org/wiki/File:Eye_(1).jpg

### Butterfly

Usado como elemento de colagem no rail esquerdo.

- Wikimedia Commons: `Butterfly Image.jpg`
- https://commons.wikimedia.org/wiki/File:Butterfly_Image.jpg

### Clio portrait

- Wikimedia Commons: `Roman Marble Bust of Severan Woman, 220-230 AD (28702800686).jpg`
- https://commons.wikimedia.org/wiki/File:Roman_Marble_Bust_of_Severan_Woman,_220-230_AD_(28702800686).jpg

### Daedalus portrait

- Wikimedia Commons: `Ancient Greece Marble Bust (28738468135).jpg`
- https://commons.wikimedia.org/wiki/File:Ancient_Greece_Marble_Bust_(28738468135).jpg

### Scribe portrait

- Wikimedia Commons: `Roman Marble Bust of Woman, Trajan Era (28138662634).jpg`
- https://commons.wikimedia.org/wiki/File:Roman_Marble_Bust_of_Woman,_Trajan_Era_(28138662634).jpg

### Athena portrait

- Wikimedia Commons: `Bust of Athena Parthenos, Vatican Museums.jpg`
- https://commons.wikimedia.org/wiki/File:Bust_of_Athena_Parthenos,_Vatican_Museums.jpg

## Pipeline

O script responsável é:

```text
scripts/fetch-assets.mjs
```

Ele gera:

```text
public/assets/hero.webp
public/assets/left-rail.webp
public/assets/right-rail.webp
public/assets/bottom-center.webp
public/assets/clio.webp
public/assets/daedalus.webp
public/assets/scribe.webp
public/assets/athena.webp
public/assets/profile.webp
```

Os arquivos gerados não são versionados no Git; fazem parte do build.
