# Nodus

**Nodus** é um painel local e leve para organizar, observar e futuramente controlar múltiplos agentes de IA em um único lugar.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão** — refletindo a ideia central do projeto: diversos agentes conectados por uma interface única.

## Estado atual

Versão atual: **0.2.3 — shell nativo sem barra branca do Windows**.

Nesta fase, o foco continua sendo consolidar a experiência visual e manter uma base extremamente leve para Windows antes de adicionar integrações reais com agentes e shells locais.

### Já implementado

- Aplicação nativa em **C** usando **Win32 API**.
- Interface sem Electron, React, WebView, HTML ou JavaScript.
- Estética oficial de **colagem digital surrealista clássica dark**.
- Navegação principal em abas horizontais.
- Abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`.
- Painel e terminal em modo de simulação para validar a experiência antes da integração real.
- Instalador que baixa temporariamente um compilador C portátil, compila o aplicativo e instala no perfil do usuário.
- Build manual via MSVC ou MinGW-w64.
- Janela customizada sem a barra branca padrão do Windows.
- Área superior do próprio Nodus usada para mover a janela.
- Controles próprios de minimizar, maximizar/restaurar e fechar.
- Redimensionamento nativo preservado pelas bordas da janela.
- Campo de busca visual integrado ao cabeçalho.

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
- cabeçalho superior desenhado pelo próprio Nodus;
- **sem barra branca de título do Windows**.

O objetivo é parecer uma peça editorial/galeria surrealista que também funciona como uma ferramenta séria de controle de agentes — e não um dashboard futurista/cyberpunk genérico.

## Instalação

No Windows 64-bit:

1. Baixe o ZIP mais recente.
2. Extraia a pasta.
3. Execute `INSTALAR_NODUS.bat`.
4. Aguarde a compilação e instalação automáticas.
5. O Nodus será aberto e um atalho será criado.

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

O fluxo padrão deste projeto é:

1. alterações são enviadas para `Kaynanwxz/nodus`;
2. o `README.md` e o changelog são atualizados;
3. uma nova versão ZIP é entregue pronta para baixar.

O GitHub Actions também gera automaticamente o pacote Windows a cada atualização da branch `main`.

No GitHub:

```text
Actions
→ Build Nodus for Windows
→ execução mais recente
→ Artifacts
→ Nodus-Windows
```

### Usando Git localmente

Se quiser manter uma cópia clonada:

```text
git pull origin main
```

Para uso normal, o ZIP continua sendo o caminho recomendado.

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
│  ├─ main.c
│  └─ frame_override.h
├─ installer/
│  └─ install.ps1
├─ build.bat
├─ INSTALAR_NODUS.bat
└─ README.md
```

## Próximas etapas

A evolução planejada é transformar o Nodus de um painel visual em um verdadeiro **orquestrador local de agentes**:

- aproximar ainda mais a implementação nativa do mockup visual aprovado;
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

### 0.2.3

- Removida a barra branca padrão de título do Windows.
- Janela principal migrada para um frame customizado do Nodus.
- Cabeçalho escuro passou a funcionar também como área de arraste da janela.
- Adicionados controles próprios de minimizar, maximizar/restaurar e fechar.
- Mantido o redimensionamento nativo pelas bordas.
- Campo de busca visual integrado ao topo.
- Navegação horizontal mantida como parte do design aprovado.
- Workflow de build Windows ajustado para configuração mais confiável do MSVC.
- README atualizado.

### 0.2.2

- Direção visual principal aprovada.
- Colagem surrealista clássica dark definida como identidade oficial.
- Mantida navegação horizontal no topo.
- Definido que a barra branca nativa do Windows deveria ser removida.
- Criada documentação de design em `docs/DESIGN.md`.
- Adicionado fluxo de build automático para gerar ZIP Windows via GitHub Actions.

### 0.2.1

- Projeto renomeado oficialmente para **Nodus**.
- Branding antigo do Agent Control removido.
- Executável renomeado para `Nodus.exe`.
- Diretório de instalação alterado para `%LOCALAPPDATA%\Nodus`.
- Atalhos e desinstalador atualizados para Nodus.
- Mantida a correção de compilação do LLVM-MinGW para `GET_X_LPARAM` / `GET_Y_LPARAM`.

---

> *Nodus*: nó, vínculo, conexão.
