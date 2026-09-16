# Nodus

**Nodus** é um aplicativo desktop local para organizar e controlar agentes, processos e tarefas em uma única interface.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão**.

## Versão atual

**0.6.0 — Local Orchestrator**

A versão 0.6 transforma o Nodus de um dashboard visual em uma ferramenta local utilizável. A interface continua usando a direção editorial clássica/surrealista aprovada, mas agora os dados principais podem vir de processos, tarefas e eventos reais do computador.

Stack principal:

- **Tauri 2** para o aplicativo desktop;
- **Rust** para processos, SQLite, arquivos, permissões, tray e integração com Windows;
- **React + TypeScript** para a interface;
- **Vite** para o frontend;
- **SQLite local** para persistência;
- **WebP locais** para a composição visual.

## O que já funciona

### Agentes locais

É possível cadastrar um agente informando:

- nome;
- função;
- descrição;
- comando de inicialização;
- workspace;
- provider opcional;
- modelo opcional.

Cada agente pode ser aberto em uma tela própria com:

- **Start**;
- **Stop**;
- **Restart**;
- **Open workspace**;
- **Edit**;
- **Delete**;
- PID e estado do processo;
- Terminal;
- Logs;
- Tasks;
- Memory;
- Settings.

O Nodus inicia agentes como processos filhos locais e mantém o handle deles enquanto a sessão do aplicativo estiver aberta.

### Terminal por agente

Cada agente possui uma área de terminal que executa comandos no workspace configurado e mostra:

- stdout;
- stderr;
- exit code;
- registro da ação em Activity.

O terminal atual executa comandos individuais no shell do sistema. Ele **ainda não é um PTY persistente completo** como Windows Terminal ou PowerShell interativo; esse é um próximo passo específico da arquitetura de terminal.

### Tarefas persistentes

As tarefas são armazenadas em SQLite e sobrevivem ao fechamento do aplicativo.

Cada tarefa pode guardar:

- título;
- agente responsável;
- prioridade;
- status;
- data de criação;
- início;
- conclusão;
- notas.

Estados atuais:

- `Queued`
- `Running`
- `Completed`
- `Failed`

### Activity real

O Nodus registra eventos locais como:

- agente criado ou alterado;
- start/stop/restart;
- comando executado;
- workspace aberto;
- arquivos enviados a um agente;
- tarefa criada, alterada ou removida;
- erros registrados pelo fluxo controlado pelo Nodus.

### Métricas reais

O Overview usa dados locais para mostrar:

- agentes online;
- total de agentes;
- tarefas em execução;
- ações do dia;
- erros do dia.

### Command Palette

Use:

```text
Ctrl + K
```

A palette permite navegar e executar ações rápidas, por exemplo:

- abrir uma aba;
- criar agente;
- criar tarefa;
- abrir agente;
- iniciar agente;
- parar agente;
- reiniciar agente;
- localizar tarefas.

Também existem atalhos:

```text
Ctrl + 1 .. Ctrl + 6
```

para alternar entre as abas principais.

### Permissões por agente

Cada agente possui controles persistentes para:

- Files;
- Terminal;
- Browser;
- Git;
- Network.

Valores possíveis:

- `Ask`
- `Allow`
- `Deny`

Nesta versão, as permissões **Terminal** e **Files** já bloqueiam/confirmam ações executadas pelo backend do Nodus. Browser, Git e Network já são persistidos e aparecem na interface, mas só passam a ser aplicados quando existirem motores de ação próprios do Nodus para essas categorias.

### Memory por agente

Cada agente possui uma memória local simples em formato chave/valor armazenada no SQLite.

Exemplo:

```text
project = Nodus
branch = main
objective = Implement local orchestration
```

Ela é indicada para fatos estáveis, contexto de projeto e configurações simples. Não é uma memória vetorial/semântica.

### Drag and drop de arquivos

No desktop, ao abrir um agente, arquivos podem ser arrastados para a janela.

Com permissão de Files, o Nodus copia arquivos válidos para:

```text
<workspace>\.nodus\inbox\
```

O evento também entra em Activity e a lista mais recente é registrada na memória do agente.

Diretórios inteiros ainda não são copiados por esse fluxo; atualmente ele aceita arquivos normais.

### Busca global

A busca do header filtra agentes e tarefas pelo conteúdo disponível na interface.

### Backup, export e import

Em Settings:

- **Create backup** cria uma cópia do banco SQLite no diretório local do Nodus;
- **Export JSON** exporta agentes, tarefas, permissões e memória;
- **Import JSON** recria a configuração exportada e remapeia IDs entre agentes e tarefas.

O import atual pede confirmação e substitui a configuração de agentes/tarefas existente.

### System tray

A versão desktop cria um ícone na bandeja do Windows com:

- Open Nodus;
- Hide to tray;
- Quit.

Também existe o botão **Hide to tray** em Settings.

### Iniciar com Windows

A opção **Start with Windows** usa a chave do usuário atual:

```text
HKCU\Software\Microsoft\Windows\CurrentVersion\Run
```

Nenhum privilégio de administrador é necessário para essa configuração.

### Notificações

Ao mudar tarefas para `Completed` ou `Failed`, a interface tenta emitir uma notificação usando o suporte de notificações do WebView/Windows quando permitido pelo sistema.

## Persistência local

O banco principal é criado no diretório de dados do aplicativo Tauri com o nome:

```text
nodus.db
```

Ele armazena:

- agentes;
- tarefas;
- activity;
- permissões;
- memória;
- settings preparados para expansão.

O SQLite usa WAL durante a execução. Antes de criar um backup, o Nodus solicita um checkpoint do WAL.

## Segurança e controle

O Nodus foi pensado para uso pessoal e local-first, mas os comandos configurados em agentes são **comandos reais do seu computador**.

Por isso:

- não cadastre comandos que você não entende;
- mantenha permissões em `Ask` quando quiser confirmação antes de Terminal ou Files;
- `Deny` bloqueia a ação controlada pelo Nodus;
- `Allow` executa sem nova confirmação;
- o Nodus não transforma um comando perigoso em seguro.

A interface não envia seus dados para um serviço cloud por padrão.

## Limitações atuais conhecidas

A versão 0.6 já é funcional, mas ainda há algumas limitações técnicas importantes:

1. O terminal executa comandos independentes e ainda não mantém uma sessão PTY interativa persistente.
2. Processos são controlados com handles mantidos pela instância atual do Nodus. O aplicativo ainda não reanexa automaticamente um processo iniciado por uma sessão anterior.
3. As permissões Browser/Git/Network estão prontas no modelo de dados, mas aguardam ações próprias dessas categorias para serem efetivamente aplicadas.
4. Drag and drop copia arquivos, não árvores completas de diretórios.
5. Notificações dependem do suporte/permissão do WebView e do Windows.
6. A memória atual é estruturada chave/valor, não uma base vetorial ou sistema de aprendizado automático.

## Direção visual

O design mantém a referência aprovada:

- fundo preto/carvão;
- dourado, ocre, marfim e sépia;
- esculturas e gravuras clássicas;
- mapas e diagramas celestes;
- papel envelhecido e recortes editoriais;
- tipografia serifada combinada com sans-serif discreta;
- header próprio sem a barra branca padrão do Windows;
- métricas abertas;
- cards leves;
- microinterações e transições suaves;
- scroll interno para evitar conteúdo cortado;
- suporte a `prefers-reduced-motion`.

Arquivos principais da camada visual:

```text
src/styles.css
src/hero.css
src/experience.css
src/functional.css
src/functional-overrides.css
```

## Abas

- `Overview`
- `Agents`
- `Terminal`
- `Tasks`
- `Activity`
- `Settings`

## Rodar somente a interface

Para testar design e fluxos sem compilar Rust/Tauri:

```bat
npm install
npm run dev
```

Depois abra:

```text
http://127.0.0.1:1420/
```

No navegador, o Nodus entra em **preview mode** e usa `localStorage` para simular/persistir agentes, tarefas, permissões e memória. Comandos não são executados de verdade nesse modo.

## Rodar o desktop real

Pré-requisitos no Windows:

- Node.js 20+;
- npm;
- Rust/Cargo via rustup;
- Microsoft C++ Build Tools/Windows SDK exigidos pelo Tauri;
- WebView2.

Execute:

```bat
git pull origin main
npm install
npm run desktop
```

## Build Windows

Use:

```bat
build.bat
```

ou:

```bat
npm run desktop:build
```

Saídas esperadas:

```text
src-tauri\target\release\nodus.exe
src-tauri\target\release\bundle\nsis\Nodus_0.6.0_x64-setup.exe
```

O GitHub Actions também gera:

```text
Nodus-Windows-v0.6.0.zip
```

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
│  ├─ nodus.ts
│  ├─ platform-api.ts
│  ├─ config-io.ts
│  ├─ main.tsx
│  ├─ styles.css
│  ├─ hero.css
│  ├─ experience.css
│  ├─ functional.css
│  └─ functional-overrides.css
├─ src-tauri/
│  ├─ capabilities/
│  ├─ icons/
│  ├─ src/
│  │  ├─ lib.rs
│  │  ├─ main.rs
│  │  └─ platform.rs
│  ├─ Cargo.toml
│  └─ tauri.conf.json
├─ build.bat
├─ package.json
├─ tsconfig.json
└─ vite.config.ts
```

## Próximas evoluções técnicas

Com a base funcional pronta, as evoluções de maior valor passam a ser:

- PTY persistente por agente;
- reconexão/descoberta de processos após reiniciar o Nodus;
- motores próprios para ações de Git, browser e network;
- fila/runner automático de tarefas;
- comunicação entre agentes;
- templates de agentes;
- memória semântica opcional;
- diretórios completos no drag and drop;
- política de permissões mais granular por comando/caminho;
- estatísticas de CPU/RAM por processo.

## Princípios

- **Leve:** sem Electron.
- **Local-first:** os dados principais ficam no computador do usuário.
- **Controlável:** Start/Stop/Restart e permissões continuam sob autoridade do usuário.
- **Persistente:** tarefas, activity, memória e configurações sobrevivem ao fechamento do app.
- **Visual:** o design forte não deve atrapalhar a leitura ou o trabalho.
- **Modular:** novos agentes e integrações devem entrar sem transformar o Nodus em um sistema pesado.

## Changelog

### 0.6.0

- Agentes reais com comando, workspace, PID e ciclo Start/Stop/Restart.
- Tela individual de agente com Terminal, Logs, Tasks, Memory e Settings.
- Terminal funcional para execução de comandos no workspace.
- SQLite local com WAL para agentes, tarefas, activity, permissões e memória.
- Tarefas persistentes com prioridade e estados de execução.
- Activity alimentado por ações reais do backend.
- Métricas reais no Overview.
- Command Palette com `Ctrl + K`.
- Atalhos `Ctrl + 1..6`.
- Busca global de agentes e tarefas.
- Permissões Ask/Allow/Deny por agente.
- Memory chave/valor por agente.
- Drag and drop de arquivos para `.nodus/inbox`.
- Backup SQLite.
- Export e import JSON.
- System tray no Windows.
- Start with Windows.
- Edição e remoção de agentes.
- Notificações de tarefa concluída/falha quando suportadas.
- Browser preview com persistência em localStorage para testar a interface sem compilar o desktop.
- Versão do pacote, Tauri, build.bat e pipeline Windows atualizadas para 0.6.0.

### 0.5.0

- Migração de C/Win32 para Tauri 2 + React + TypeScript + Rust.
- Redesign editorial clássico/surrealista.
- Assets locais em WebP.
- Header desktop customizado.
- Revisão de tipografia, espaçamento, transições e overflow.
- Pipeline Windows Tauri e instalador NSIS.

### 0.4.x e anteriores

Versões históricas baseadas em C/Win32. Não são mais a implementação oficial.

---

> *Nodus*: nó, vínculo, conexão.
