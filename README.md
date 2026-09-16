# Nodus

**Nodus** é um aplicativo desktop local para organizar, observar e futuramente controlar múltiplos agentes de IA em uma única interface.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão**.

## Versão atual

**0.5.0 — Tauri desktop redesign**

Esta passa a ser a versão oficial do projeto.

A implementação antiga em C/Win32 foi removida do fluxo principal. O Nodus agora usa:

- **Tauri 2** para o aplicativo desktop;
- **React + TypeScript** para a interface;
- **Vite** para o frontend;
- **Rust** para o núcleo desktop;
- **WebP locais e otimizados** para a composição visual.

## Direção visual

O design segue a referência aprovada: uma interface escura editorial com colagem clássica/surrealista, mantendo o centro funcional e a arte mais rica nas bordas.

Elementos principais:

- fundo preto/carvão;
- dourado, ocre, marfim e sépia;
- esculturas e gravuras clássicas;
- mapas e diagramas celestes;
- papel envelhecido e recortes editoriais;
- tipografia serifada de destaque;
- header próprio, sem barra branca padrão do Windows;
- hero artístico;
- métricas compactas;
- cards de agentes;
- tabela de tarefas;
- painel de atividade.

Os assets visuais agora são arquivos reais do projeto, em vez de esculturas desenhadas com primitivas GDI.

## Abas

- `Overview`
- `Agents`
- `Terminal`
- `Tasks`
- `Activity`
- `Settings`

## Rodar em desenvolvimento

Pré-requisitos:

- Node.js 20+
- npm
- Rust + Cargo
- dependências do Tauri para Windows

Depois:

```bash
npm install
npm run desktop
```

No Windows, o Vite ignora `src-tauri/target/**` durante o modo desenvolvimento para não tentar observar executáveis temporários bloqueados pelo Cargo.

O projeto inclui `src-tauri/icons/icon.ico`, gerado a partir do PNG da marca Nodus com resoluções de 16, 24, 32, 48, 64, 128 e 256 px. O arquivo é necessário para o Tauri gerar os recursos nativos do Windows. `src-tauri/icons/icon.png` guarda a imagem-base usada para o ícone.

## Build Windows

Use:

```bat
build.bat
```

ou diretamente:

```bash
npm install
npm run desktop:build
```

Saídas principais:

```text
src-tauri\target\release\nodus.exe
src-tauri\target\release\bundle\nsis\
```

O instalador oficial passa a ser o pacote NSIS gerado pelo próprio Tauri.

## GitHub Actions

Cada atualização relevante do projeto também atualiza este README.

O workflow de Windows:

1. instala Node;
2. instala Rust;
3. instala as dependências do frontend;
4. compila o Tauri;
5. gera o executável e instalador;
6. publica um artifact para teste.

## Estrutura

```text
Nodus/
├─ .github/
│  └─ workflows/
│     └─ build-windows.yml
├─ docs/
├─ public/
│  └─ assets/
├─ scripts/
├─ src/
│  ├─ App.tsx
│  ├─ main.tsx
│  ├─ styles.css
│  └─ hero.css
├─ src-tauri/
│  ├─ icons/
│  │  └─ icon.ico
│  ├─ src/
│  ├─ Cargo.toml
│  └─ tauri.conf.json
├─ build.bat
├─ package.json
├─ tsconfig.json
└─ vite.config.ts
```

## Objetivo funcional

Depois que a interface estiver visualmente fechada, o Nodus evolui para um verdadeiro orquestrador local:

- execução real de processos;
- integração com agentes locais e externos;
- terminal por agente;
- start, stop e restart;
- workspaces individuais;
- tarefas persistentes;
- histórico de execução;
- métricas reais;
- comunicação entre agentes;
- permissões por agente;
- memória e configuração locais;
- distribuição de objetivos entre vários agentes.

## Princípios

- **Leve:** sem Electron.
- **Local-first:** pensado primeiro para uso pessoal no próprio computador.
- **Visual:** design forte sem sacrificar legibilidade.
- **Controlável:** o usuário continua sendo a autoridade sobre ações e permissões.
- **Modular:** integrações futuras não devem transformar o app em um sistema pesado.

## Changelog

### 0.5.0

- Migração de C/Win32 para **Tauri 2 + React + TypeScript + Rust**.
- Antiga implementação Win32 retirada do fluxo oficial.
- Assets clássicos reais adicionados ao projeto.
- Removido o antigo instalador manual em PowerShell/BAT.
- Instalador oficial passa a ser o NSIS gerado pelo Tauri.
- Header desktop customizado mantido.
- Overview reconstruído com hero, métricas, agents, tasks e activity.
- Abas secundárias preservadas.
- Pipeline Windows refeito para Tauri.
- Build sem dependência de download de assets em tempo de compilação.
- Vite deixa de observar `src-tauri/target/**`, evitando `EBUSY` com executáveis temporários do Cargo no Windows.
- Corrigido o `src-tauri/icons/icon.ico` inválido que causava `failed to parse icon ... failed to fill whole buffer` no build Windows. O ícone agora contém sete resoluções e tem o PNG da marca como imagem-base.
- Reconstruídos os WebP inválidos do hero e das laterais e incluído `bottom-center.webp`, antes ausente. A verificação de assets agora rejeita arquivos WebP ausentes ou com estrutura inválida antes do build.
- README consolidado para a nova arquitetura.

### 0.4.x e anteriores

Versões históricas baseadas em C/Win32. Não são mais a implementação oficial.

---

> *Nodus*: nó, vínculo, conexão.

