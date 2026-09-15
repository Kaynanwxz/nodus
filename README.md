# Nodus

**Nodus** é um painel local e leve para organizar, observar e futuramente controlar múltiplos agentes de IA em um único lugar.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão** — refletindo a ideia central do projeto: diversos agentes conectados por uma interface única.

## Estado atual

Versão atual: **0.3.0 — redesign nativo em colagem surrealista dark**.

A 0.3.0 é uma reconstrução visual real da interface anterior. O layout deixou de ser apenas uma variação do protótipo antigo e agora usa uma composição mais próxima do mockup aprovado, mantendo o aplicativo em C/Win32 e sem adicionar frameworks pesados.

### Já implementado

- Aplicação nativa em **C** usando **Win32 API**.
- Sem Electron, React, WebView, HTML ou JavaScript.
- Janela principal sem a barra branca padrão do Windows.
- Cabeçalho próprio do Nodus, com controles de janela integrados.
- Navegação horizontal no topo.
- Abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`.
- Interface redesenhada com estética de **colagem digital surrealista clássica dark**.
- Recortes de papel, olhos, estátuas, diagramas celestes e composição editorial diretamente renderizados em GDI.
- Dashboard, agentes, terminal, tarefas, atividade e configurações com layouts próprios.
- Instalador simplificado que usa o `Nodus.exe` já compilado — o computador do usuário não precisa baixar compilador nem compilar nada.
- Atalhos na Área de Trabalho e Menu Iniciar.
- Desinstalador local.
- Build Windows automático via GitHub Actions.

## Direção visual aprovada

A identidade do Nodus segue uma linguagem de **colagem digital surrealista clássica dark**, sem estética futurista/cyberpunk genérica.

A referência aprovada combina:

- fundo preto e carvão;
- esculturas clássicas e gravuras antigas;
- olhos, anatomia e diagramas celestes;
- papel envelhecido e recortes irregulares;
- dourado/ocre, marfim, sépia e tons terrosos;
- tipografia editorial serifada;
- painéis escuros integrados à composição;
- navegação horizontal no topo;
- cabeçalho superior desenhado pelo próprio Nodus;
- **sem barra branca de título do Windows**.

## Instalação

No Windows 64-bit:

1. Baixe o ZIP mais recente.
2. Extraia o ZIP inteiro.
3. Execute `INSTALAR_NODUS.bat`.
4. O instalador copia a versão já compilada para o perfil do usuário.
5. O Nodus abre automaticamente e cria atalhos.

Não é necessário instalar Visual Studio, MinGW ou qualquer compilador.

### Pasta de instalação

```text
%LOCALAPPDATA%\Nodus
```

### Executar sem instalar

Também é possível simplesmente abrir:

```text
Nodus.exe
```

### Desinstalação

Depois de instalado, execute:

```text
%LOCALAPPDATA%\Nodus\DESINSTALAR_NODUS.bat
```

## Como receber as próximas atualizações

O fluxo padrão do projeto é:

1. alterações são enviadas para `Kaynanwxz/nodus`;
2. o `README.md` e o changelog são atualizados;
3. o GitHub Actions compila a versão Windows;
4. um novo ZIP pronto para uso é entregue.

Para uso normal, o ZIP é o caminho recomendado.

## Build manual

Se você já tiver MSVC ou MinGW-w64 configurado:

```text
build.bat
```

O executável será gerado em:

```text
bin\Nodus.exe
```

## Estrutura

```text
Nodus/
├─ .github/
│  └─ workflows/
│     └─ build-windows.yml
├─ docs/
│  └─ DESIGN.md
├─ src/
│  └─ main.c
├─ installer/
│  └─ install.ps1
├─ build.bat
├─ INSTALAR_NODUS.bat
└─ README.md
```

## Próximas etapas

A evolução planejada é transformar o Nodus de um painel visual em um verdadeiro **orquestrador local de agentes**:

- execução real de processos com `CreateProcess`;
- captura de `stdout` e `stderr` usando pipes;
- terminal local real por agente;
- criação, edição, start, stop e restart de agentes;
- workspaces individuais;
- histórico de execução;
- tarefas persistentes;
- métricas reais de CPU e memória;
- comunicação entre agentes;
- permissões por agente;
- armazenamento local de configuração e memória;
- Mission Control para distribuir um objetivo entre vários agentes.

## Princípios do projeto

- **Leve:** C nativo e poucas dependências.
- **Local-first:** pensado inicialmente para uso pessoal no próprio computador.
- **Observável:** cada agente deve deixar claro o que está fazendo.
- **Controlável:** o usuário continua sendo a autoridade sobre execução e permissões.
- **Modular:** novas integrações podem ser adicionadas sem transformar o painel em uma aplicação pesada.
- **Identidade própria:** interface artística marcante sem abrir mão de usabilidade.

## Changelog

### 0.3.0

- Interface principal redesenhada de verdade em C/Win32.
- Composição visual aproximada do mockup aprovado em vez de reaproveitar o painel antigo.
- Estética consolidada como colagem surrealista dark, sem elementos futuristas.
- Cabeçalho próprio integrado à janela, sem barra branca do Windows.
- Navegação horizontal mantida no topo.
- Overview reconstruído com hero central, collage strips, métricas e painéis de agentes/atividade.
- Abas Agents, Terminal, Tasks, Activity e Settings redesenhadas.
- `frame_override.h` removido; o frame agora faz parte diretamente da implementação principal.
- Instalador refeito para usar um executável já compilado.
- Removida a necessidade de baixar LLVM/MinGW durante a instalação.
- Pacote Windows agora inclui `Nodus.exe`, `INSTALAR_NODUS.bat`, `install.ps1` e desinstalador.
- Workflow de build atualizado para gerar `Nodus-Windows-v0.3.0.zip`.
- README e changelog atualizados.

### 0.2.3

- Primeira tentativa de remover a barra branca padrão do Windows.
- Frame customizado separado em `frame_override.h`.
- Build Windows automatizado validado no GitHub Actions.

### 0.2.2

- Direção visual principal aprovada.
- Colagem surrealista clássica dark definida como identidade oficial.
- Navegação horizontal no topo definida como padrão.

### 0.2.1

- Projeto renomeado oficialmente para **Nodus**.
- Executável renomeado para `Nodus.exe`.
- Diretório de instalação alterado para `%LOCALAPPDATA%\Nodus`.

---

> *Nodus*: nó, vínculo, conexão.
