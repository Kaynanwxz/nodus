# Nodus

**Nodus** é um painel local e leve para organizar, observar e futuramente controlar múltiplos agentes de IA em um único lugar.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão** — refletindo a ideia central do projeto: diversos agentes conectados por uma interface única.

## Estado atual

Versão atual: **0.2.2 — direção visual aprovada / protótipo nativo em C + Win32**.

Nesta fase, o foco é manter uma base extremamente leve para Windows e consolidar a interface antes de adicionar integrações reais com agentes e shells locais.

### Já implementado

- Aplicação nativa em **C** usando **Win32 API**.
- Interface sem Electron, React, WebView, HTML ou JavaScript.
- Navegação principal em abas horizontais.
- Abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`.
- Painel e terminal em modo de simulação para validar a experiência antes da integração real.
- Instalador por script que baixa temporariamente um compilador C portátil, compila o aplicativo, cria os atalhos e remove a toolchain temporária.
- Correção de compatibilidade do instalador com LLVM-MinGW para coordenadas de mouse sem depender de `windowsx.h`.

## Direção visual aprovada

A identidade definitiva do Nodus segue uma linguagem de **colagem digital surrealista clássica dark**.

A referência aprovada combina:

- fundo preto e carvão;
- esculturas clássicas e gravuras antigas;
- olhos, mãos, anjos e elementos anatômicos;
- mapas, diagramas celestes e manuscritos;
- papel envelhecido, recortes e sobreposições de colagem;
- detalhes em dourado/ocre, marfim e sépia;
- tipografia serifada editorial;
- painéis escuros limpos sobre a arte;
- navegação horizontal no topo;
- cabeçalho superior **customizado pelo próprio Nodus**, sem a barra branca padrão do Windows.

O objetivo é parecer uma peça editorial/galeria surrealista que também funciona como uma ferramenta séria de controle de agentes — e não um dashboard futurista/cyberpunk genérico.

A especificação detalhada está em [`docs/DESIGN.md`](docs/DESIGN.md).

## Instalação

No Windows 64-bit:

1. Baixe ou clone este repositório.
2. Execute `INSTALAR_NODUS.bat`.
3. Aguarde a compilação e instalação automáticas.
4. O **Nodus** será aberto e um atalho será criado na Área de Trabalho e no Menu Iniciar.

Não é necessário ter Visual Studio ou MinGW previamente instalado.

### Pasta de instalação

```text
%LOCALAPPDATA%\Nodus
```

### Desinstalação

Depois de instalado, execute:

```text
%LOCALAPPDATA%\Nodus\DESINSTALAR.bat
```

## Como receber as próximas atualizações

### Opção recomendada — ZIP pronto

Para o seu uso, o fluxo mais simples será:

1. Eu atualizo o projeto e o README no GitHub.
2. Nas versões que já estiverem utilizáveis, entrego também um **ZIP pronto**.
3. Você baixa, extrai e executa a nova versão.

Além disso, o repositório possui um workflow do GitHub Actions que gera automaticamente um pacote Windows a cada atualização enviada para a branch `main`.

No GitHub, você pode abrir **Actions → Build Nodus for Windows → execução mais recente → Artifacts** e baixar `Nodus-Windows`.

### Opção Git

Se em algum momento quiser manter uma cópia clonada no PC:

```text
git pull origin main
```

Isso atualiza a pasta local com a versão mais recente do código, mas para o seu caso o ZIP pronto continua sendo a alternativa com menos trabalho.

## Build manual

Se você já tiver MSVC ou MinGW-w64 configurado, execute:

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

- aplicar no aplicativo nativo a direção visual aprovada;
- substituir a barra de título branca padrão do Windows por uma janela customizada/frameless;
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
- sistema de Mission Control para distribuir um objetivo entre vários agentes.

## Princípios do projeto

- **Leve:** C nativo e poucas dependências.
- **Local-first:** pensado inicialmente para uso pessoal no próprio computador.
- **Observável:** cada agente deve deixar claro o que está fazendo.
- **Controlável:** o usuário continua sendo a autoridade sobre execução e permissões.
- **Modular:** novas integrações podem ser adicionadas sem transformar o painel em uma aplicação pesada.
- **Identidade própria:** interface artística marcante sem abrir mão de usabilidade.

## Changelog

### 0.2.2

- Direção visual principal aprovada.
- Colagem surrealista clássica dark definida como identidade oficial.
- Mantida navegação horizontal no topo.
- Definido que a barra branca nativa do Windows deve ser removida e substituída por cabeçalho customizado integrado ao layout.
- Criada documentação de design em `docs/DESIGN.md`.
- Adicionado fluxo de build automático para gerar ZIP Windows via GitHub Actions.
- README atualizado com o fluxo recomendado de atualizações.

### 0.2.1

- Projeto renomeado oficialmente para **Nodus**.
- Branding antigo do Agent Control removido.
- Executável renomeado para `Nodus.exe`.
- Diretório de instalação alterado para `%LOCALAPPDATA%\Nodus`.
- Atalhos e desinstalador atualizados para Nodus.
- README migrado e atualizado com visão, instalação, arquitetura atual e roadmap.
- Mantida a correção de compilação do LLVM-MinGW para `GET_X_LPARAM` / `GET_Y_LPARAM`.

---

> *Nodus*: nó, vínculo, conexão.
