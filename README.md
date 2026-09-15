# Nodus

**Nodus** é um painel local e leve para organizar, observar e futuramente controlar múltiplos agentes de IA em um único lugar.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão** — refletindo a ideia central do projeto: diversos agentes conectados por uma interface única.

## Estado atual

Versão em desenvolvimento: **0.4.0 — reference-driven classical collage dashboard**.

A 0.4.0 reconstrói a interface principal a partir da referência visual aprovada em 15/09/2026, aproximando estrutura, hierarquia, densidade e composição do mockup escolhido sem abandonar a base nativa em C/Win32.

Esta versão está na branch:

```text
design-reference-v0.4
```

### Já implementado

- Aplicação nativa em **C** usando **Win32 API**.
- Sem Electron, React, WebView, HTML ou JavaScript.
- Janela principal sem a barra branca padrão do Windows.
- Cabeçalho próprio do Nodus com navegação integrada.
- Abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`.
- Overview refeito com composição muito mais próxima da referência aprovada.
- Hero editorial com headline serifada, escultura, olho, diagrama celeste, asa e fragmentos de papel.
- Métricas em quatro cards: agentes online, tarefas, ações e saúde do sistema.
- Cards compactos de agentes em linha.
- Lista de tarefas densa, no estilo da referência.
- Activity feed vertical integrado ao dashboard.
- Escala visual adaptativa para diferentes tamanhos de janela usando renderização off-screen e `StretchBlt`.
- Janela redimensionável com tamanho mínimo para preservar legibilidade.
- Build local corrigido: removida a dependência antiga de `frame_override.h`.
- Instalador atualizado para reconhecer `bin\Nodus.exe` quando o projeto foi compilado a partir do código-fonte.
- Build Windows automático via GitHub Actions na `main` e na branch `design-reference-v0.4`.

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

## O que mudou na 0.4.0

A versão anterior já apontava para a linguagem visual correta, mas ainda funcionava como um dashboard escuro genérico com alguns elementos de colagem. A 0.4.0 muda a composição inteira para seguir a referência aprovada:

- branding `NODUS` mais editorial;
- tagline `IDEAS. AGENTS. ACTIONS. TOGETHER.`;
- navegação central compacta;
- busca visual no header;
- relógio e medallion/profile no canto direito;
- grande hero antes dos dados;
- headline `A MORE THOUGHTFUL TOMORROW, COMPOSED TODAY.`;
- colagem decorativa concentrada nas bordas;
- área de dados mais limpa no centro;
- métricas compactas em uma única linha;
- quatro agent cards lado a lado;
- tasks e activity divididos como no mockup;
- melhor comportamento quando a janela é redimensionada.

## Instalação

Existem dois fluxos diferentes:

### 1. ZIP compilado do GitHub Actions — recomendado

Esse pacote já contém `Nodus.exe` e **não exige compilador**.

1. Abra a execução mais recente do workflow **Build Nodus for Windows**.
2. Baixe o artifact `Nodus-Windows-v0.4.0`.
3. Extraia o ZIP inteiro.
4. Execute `INSTALAR_NODUS.bat`.
5. O instalador copia o aplicativo para o perfil do usuário e cria os atalhos.

### 2. Código-fonte clonado do GitHub

O repositório não armazena `Nodus.exe` compilado. Se você clonou o projeto, primeiro execute:

```text
build.bat
```

Depois do build, o executável estará em:

```text
bin\Nodus.exe
```

A partir daí, `INSTALAR_NODUS.bat` também reconhece automaticamente esse executável e consegue instalar a versão compilada localmente.

Se `build.bat` disser que nenhum compilador foi encontrado, use o ZIP do GitHub Actions em vez do código-fonte.

### Pasta de instalação

```text
%LOCALAPPDATA%\Nodus
```

## Como testar a branch 0.4.0

Se você já clonou o projeto:

```bash
git fetch origin
git checkout design-reference-v0.4
git pull origin design-reference-v0.4
build.bat
```

Depois, para instalar a build local:

```text
INSTALAR_NODUS.bat
```

Para voltar à versão estável:

```bash
git checkout main
git pull origin main
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

## Próximas etapas de design

A 0.4.0 resolve primeiro a **estrutura visual**. Para chegar ainda mais perto da referência final, os próximos passos são:

- substituir parte dos desenhos GDI por assets locais otimizados de gravuras/esculturas;
- criar retratos individuais dos agentes;
- adicionar textura de papel e impressão com baixo custo de renderização;
- refinar proporções após teste em 1920×1080 e tablets;
- manter a região central legível mesmo com arte mais rica nas bordas;
- só então avançar nas funções reais de agentes.

## Próximas etapas funcionais

Depois que o shell visual for aprovado:

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

### 0.4.0 — branch `design-reference-v0.4`

- Overview reconstruído a partir da referência visual aprovada.
- Hierarquia do dashboard aproximada do mockup: hero → métricas → agents/tasks/activity.
- Header redesenhado com branding, navegação, busca, status, relógio e profile medallion.
- Hero editorial com composição clássica/surrealista desenhada em GDI.
- Agent cards compactos e task table adicionados ao Overview.
- Activity feed lateral alinhado à referência.
- Renderização base em 1440×900 escalada para o tamanho real da janela.
- Tamanho mínimo de janela definido em 1024×640.
- Abas secundárias atualizadas para compartilhar a nova identidade visual.
- `build.bat` corrigido para não tentar incluir o arquivo removido `frame_override.h`.
- `installer/install.ps1` agora detecta `bin\Nodus.exe` quando a aplicação foi compilada a partir do source checkout.
- Mensagem de erro do instalador agora diferencia código-fonte sem build de pacote ZIP incompleto.
- GitHub Actions agora gera `Nodus-Windows-v0.4.0.zip` também para `design-reference-v0.4`.
- README atualizado com o fluxo correto de build e instalação.

### 0.3.0

- Interface principal redesenhada em C/Win32.
- Estética consolidada como colagem surrealista dark.
- Cabeçalho próprio integrado à janela, sem barra branca do Windows.
- Overview com hero, métricas e painéis de agentes/atividade.
- Abas Agents, Terminal, Tasks, Activity e Settings redesenhadas.
- Instalador simplificado para usar executável já compilado.
- Workflow de build atualizado para gerar pacote Windows.

### 0.2.x

- Remoção da barra branca padrão do Windows.
- Direção visual principal aprovada.
- Projeto renomeado oficialmente para **Nodus**.

---

> *Nodus*: nó, vínculo, conexão.
