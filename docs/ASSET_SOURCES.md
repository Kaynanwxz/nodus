# Nodus — Visual Asset Provenance

## Estado atual

Os assets visuais usados na versão atual do Nodus foram preparados a partir da **referência visual fornecida durante o desenvolvimento do projeto** para reproduzir com fidelidade a direção de arte aprovada.

Eles estão versionados em:

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

O script:

```text
scripts/fetch-assets.mjs
```

não baixa mais imagens externas. Na versão atual ele apenas valida se os assets necessários estão presentes e utilizáveis antes do build.

## Importante para distribuição pública/comercial

A presença destes arquivos no repositório **não deve ser interpretada como uma declaração de domínio público, CC0 ou transferência de direitos sobre a arte de referência**.

Antes de distribuir o Nodus comercialmente ou reutilizar essa composição fora do contexto de desenvolvimento/mockup, revise a procedência e os direitos aplicáveis à referência visual e, se necessário, substitua os recortes por arte própria ou por material com licença adequada.

## Direção de arte a preservar em substituições futuras

Caso os assets sejam substituídos, a identidade visual aprovada deve manter:

- colagem clássica/surrealista;
- esculturas e gravuras;
- diagramas e mapas celestes;
- papel envelhecido;
- preto/carvão, dourado, marfim e sépia;
- composição editorial, sem estética neon/cyberpunk;
- arte mais expressiva nas bordas e interface funcional mais calma no centro.
