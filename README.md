# Nodus

**Nodus** é um painel local e leve para organizar, observar e futuramente controlar múltiplos agentes de IA em um único lugar.

O nome vem do latim *nodus* — **nó, ligação, ponto de conexão** — refletindo a ideia central do projeto: diversos agentes conectados por uma interface única.

## Estado atual

Versão atual: **0.2.1 — protótipo nativo em C / Win32**.

Nesta fase, o foco é construir uma base extremamente leve para Windows antes de adicionar integrações reais com agentes e shells locais.

### Já implementado

- Aplicação nativa em **C** usando **Win32 API**.
- Interface sem Electron, React, WebView, HTML ou JavaScript.
- Estética de **colagem digital surrealista dark**.
- Navegação principal em abas horizontais.
- Abas `Overview`, `Agents`, `Terminal`, `Tasks`, `Activity` e `Settings`.
- Painel e terminal em modo de simulação para validar a experiência antes da integração real.
- Instalador por script que baixa temporariamente um compilador C portátil, compila o aplicativo, cria os atalhos e remove a toolchain temporária.
- Correção de compatibilidade do instalador com LLVM-MinGW para coordenadas de mouse sem depender de `windowsx.h`.

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
- sistema de Mission Control para distribuir um objetivo entre vários agentes.

## Direção visual

A identidade do Nodus evita a estética genérica de dashboards futuristas. A interface segue uma linguagem de **colagem surrealista clássica e dark**, inspirada em gravuras antigas, anatomia, olhos, esculturas, papel envelhecido e composições editoriais.

A prioridade é manter a arte marcante sem sacrificar legibilidade ou desempenho.

## Princípios do projeto

- **Leve:** C nativo e poucas dependências.
- **Local-first:** pensado inicialmente para uso pessoal no próprio computador.
- **Observável:** cada agente deve deixar claro o que está fazendo.
- **Controlável:** o usuário continua sendo a autoridade sobre execução e permissões.
- **Modular:** novas integrações podem ser adicionadas sem transformar o painel em uma aplicação pesada.

## Changelog

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
