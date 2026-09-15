# Nodus

**Nodus** é um aplicativo desktop local para organizar, observar e futuramente controlar múltiplos agentes de IA em uma única superfície.

O nome vem do latim *nodus* — **nó, vínculo, ponto de conexão**.

## Estado atual

Versão em desenvolvimento: **0.5.0 — migração para Tauri + reconstrução visual de alta fidelidade**.

Branch atual:

```text
tauri-v0.5
```

A versão 0.5 abandona a limitação visual do renderer GDI/Win32 anterior e passa a usar:

- **Tauri 2** para o aplicativo desktop;
- **Rust** para o núcleo nativo;
- **React + TypeScript** para a interface;
- **Vite** para build do frontend;
- **CSS** para a composição editorial;
- assets locais otimizados em **WebP**;
- imagens clássicas/gravuras reais em vez de esculturas simplificadas desenhadas com polígonos.

O objetivo é manter o Nodus leve e local-first, mas permitir uma interface muito mais próxima da referência aprovada.

## Direção visual

A referência oficial continua sendo a composição dark clássica/surrealista aprovada em 15/09/2026.

Características obrigatórias:

- preto, carvão e sépia;
- dourado/ocre e marfim envelhecido;
- esculturas, bustos e gravuras reais;
- mapas e diagramas celestes;
- olhos, anatomia, borboletas e fragmentos editoriais;
- tipografia serifada de caráter editorial;
- recortes/papel envelhecido;
- dashboard denso sem aparência de SaaS genérico;
- arte concentrada nas bordas e hero, preservando legibilidade no centro;
- janela sem a barra branca padrão do Windows;
- navegação superior integrada ao design.

A 0.5 não tenta mais desenhar estátuas com primitivas geométricas. A arte é gerada a partir de imagens reais e empacotada com o aplicativo.

## Interface 0.5

### Overview

A tela principal contém:

- header próprio do Nodus;
- abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`;
- busca no header;
- relógio local;
- controles próprios de minimizar, maximizar e fechar;
- hero editorial com a frase `A MORE THOUGHTFUL TOMORROW, COMPOSED TODAY.`;
- colagens laterais com escultura, gravura, olho, mapa e fragmentos de texto;
- quatro métricas principais;
- cards de agentes;
- tabela compacta de tarefas;
- activity feed vertical.

### Páginas secundárias

Também existem shells visuais próprios para:

- Agents;
- Terminal;
- Tasks;
- Activity;
- Settings.

Os dados desta etapa ainda são demonstrativos. A prioridade da 0.5 é acertar primeiro o shell visual e a arquitetura do desktop app.

## Assets

Os assets visuais são gerados por:

```text
scripts/fetch-assets.mjs
```

O script baixa imagens clássicas de fontes públicas, converte para WebP, aplica tratamento sépia/dark e monta os elementos utilizados pelo Nodus.

Os arquivos finais são criados em:

```text
public/assets/
```

Essa pasta é gerada automaticamente e não precisa ser versionada.

As fontes e licenças utilizadas estão documentadas em:

```text
docs/ASSET_SOURCES.md
```

## Como rodar em desenvolvimento

### Requisitos

No Windows:

- Node.js 20 ou superior;
- npm;
- Rust/Cargo via rustup;
- Microsoft C++ Build Tools / WebView2 normalmente disponíveis no ambiente Tauri para Windows.

Depois:

```bash
npm install
npm run desktop
```

O primeiro start também gera os assets automaticamente.

## Build do executável

Use:

```text
build.bat
```

ou diretamente:

```bash
npm install
npm run desktop:build
```

O executável release fica em:

```text
src-tauri\target\release\nodus.exe
```

O instalador Windows gerado pelo Tauri/NSIS fica em:

```text
src-tauri\target\release\bundle\nsis\
```

## GitHub Actions

O workflow:

```text
.github/workflows/build-windows.yml
```

compila a versão Windows automaticamente na `main` e na branch `tauri-v0.5`.

O artifact produzido é:

```text
Nodus-Windows-v0.5.0.zip
```

Ele contém:

```text
Nodus.exe
Nodus-Setup-v0.5.0.exe
README.md
```

Para testar sem instalar, abra `Nodus.exe`.

Para instalar normalmente, execute `Nodus-Setup-v0.5.0.exe`.

## Como atualizar sua cópia local

Se você já clonou o projeto:

```bash
git fetch origin
git checkout tauri-v0.5
git pull origin tauri-v0.5
npm install
npm run desktop
```

## Estrutura atual

```text
Nodus/
├─ .github/
│  └─ workflows/
│     └─ build-windows.yml
├─ docs/
│  ├─ DESIGN.md
│  └─ ASSET_SOURCES.md
├─ scripts/
│  └─ fetch-assets.mjs
├─ src/
│  ├─ App.tsx
│  ├─ main.tsx
│  ├─ styles.css
│  ├─ hero.css
│  └─ main.c              # implementação Win32 antiga, mantida temporariamente como histórico
├─ src-tauri/
│  ├─ capabilities/
│  │  └─ default.json
│  ├─ src/
│  │  ├─ lib.rs
│  │  └─ main.rs
│  ├─ Cargo.toml
│  ├─ build.rs
│  └─ tauri.conf.json
├─ index.html
├─ package.json
├─ vite.config.ts
├─ build.bat
└─ README.md
```

## Arquitetura planejada

```text
React / TypeScript UI
        ↓
      Tauri
        ↓
       Rust
        ↓
 Agent processes / terminals / files / Git / local tools
        ↓
 SQLite + logs + workspaces
```

A interface continua separada do núcleo funcional. Isso permite uma UI rica sem obrigar o core de agentes a rodar em JavaScript.

## Próximas etapas

Depois da aprovação visual da 0.5:

- processo real de criação/edição de agentes;
- start, stop e restart de processos;
- captura de `stdout` e `stderr`;
- terminal real por agente;
- workspaces locais;
- tarefas persistentes;
- métricas reais de CPU/memória;
- histórico de atividades;
- SQLite local;
- comunicação entre agentes;
- permissões por agente;
- integração com Git;
- Mission Control para distribuir objetivos.

## Princípios

- **Local-first:** o usuário continua no controle da execução.
- **Leve:** Tauri/WebView em vez de empacotar Chromium como Electron.
- **Visualmente fiel:** assets reais e composição editorial, não imitações geométricas de esculturas.
- **Observável:** agentes e tarefas devem mostrar claramente seu estado.
- **Modular:** UI, core e integrações permanecem separados.
- **Identidade própria:** o Nodus não deve parecer um dashboard SaaS/cyberpunk genérico.

## Changelog

### 0.5.0 — branch `tauri-v0.5`

- Projeto migrado para Tauri 2.
- Novo core desktop em Rust.
- Interface reconstruída em React + TypeScript + CSS.
- Vite adicionado ao pipeline.
- Janela Tauri frameless com controles próprios.
- Overview reconstruído conforme a referência aprovada.
- Hero editorial com tipografia sobre composição clássica real.
- Colagens laterais e inferiores baseadas em imagens reais.
- Gerador de assets em WebP com Sharp.
- Retratos clássicos individuais para agentes.
- Métricas, Agents, Tasks e Activity reorganizados de acordo com a referência.
- Páginas Agents, Terminal, Tasks, Activity e Settings recriadas no novo shell.
- Build local migrado de C/Win32 para Tauri.
- GitHub Actions migrado para Node + Rust + Tauri.
- Empacotamento Windows agora gera executável e instalador NSIS.
- README refeito para refletir a nova arquitetura.

### 0.4.0

- Última versão da tentativa de reproduzir a interface diretamente em C/Win32/GDI.
- Serviu para validar composição e comportamento da janela, mas a renderização artística foi limitada pela abordagem.

### 0.3.x e anteriores

- Primeiras versões nativas em C/Win32.
- Remoção da barra branca padrão do Windows.
- Definição da identidade clássica/surrealista dark.
- Projeto renomeado oficialmente para Nodus.

---

> *Nodus*: nó, vínculo, conexão.
