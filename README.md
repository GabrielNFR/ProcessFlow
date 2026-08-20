# ProcessFlow

**Orquestrador de processos em C.** O ProcessFlow cadastra "tarefas" (nome → programa + argumentos) e as executa criando **processos filhos** com `fork()`, `exec()`, `waitpid()`, `dup2()` e `pipe()` — sem delegar para nenhum shell.

Pense nele como um **maestro**: você registra as tarefas, decide como executá-las (uma a uma, em paralelo ou encadeadas por pipe), redireciona entrada/saída para arquivos e gerencia tudo a partir de um único processo pai.

## Sumário

1. [Compilação](#1-compilação)
2. [Execução](#2-execução)
3. [Comandos](#3-comandos)
4. [Exemplos](#4-exemplos)
5. [Tratamento de erros](#5-tratamento-de-erros)
6. [Estrutura do projeto](#6-estrutura-do-projeto)

## 1. Compilação

```bash
make clean
make
```

Gera o executável `processflow` (compilador: `clang`, com `-Wall -Wextra`).

## 2. Execução

### Modo interativo

```bash
./processflow
```

Mostra o prompt `processflow> ` e aguarda comandos. Encerra com `exit` ou Ctrl+D.

### Modo workflow

```bash
./processflow arquivo.pf
```

Lê os comandos de um arquivo `.pf`. Cada linha do arquivo é **impressa antes de ser executada** e o prompt não aparece. Se o arquivo não terminar com `exit`, o programa encerra normalmente no fim do arquivo.

Exemplo de `arquivo.pf`:

```
task listar /bin/ls -l
task ordenar /usr/bin/sort
run pipe listar ordenar
exit
```

## 3. Comandos

### `task <nome> <programa> [argumentos...]`

Cadastra uma tarefa. **Não executa nada** — apenas registra.

```
processflow> task listar /bin/ls -l
```

- O **nome é único**: se já existir uma tarefa com aquele nome, é exibido um erro e a definição original é mantida.
- Há um limite de tarefas (104).

### `run <tarefa>`

Executa uma **única tarefa**.

```
processflow> run listar
```

### `run sequential <tarefa1> <tarefa2> ...`

Executa as tarefas **uma após a outra** — a próxima só inicia quando a anterior termina.

```
processflow> run sequential listar ordenar
```

### `run parallel <tarefa1> <tarefa2> ...`

**Inicia todas ao mesmo tempo** e espera o grupo inteiro terminar (a ordem de término não importa).

```
processflow> run parallel dormir dormir dormir
```

### `run pipe <tarefa1> <tarefa2> ...`

Conecta a saída de uma tarefa na entrada da seguinte — como `|` no shell. Cada tarefa roda em um processo diferente.

```
processflow> run pipe listar ordenar contar
```

equivale a `ls -l | sort | wc -l`.

### `input <tarefa> <arquivo>`

Configura a tarefa para **ler a entrada de um arquivo** (como `<` no shell).

```
processflow> input ordenar nomes.txt
```

### `output <tarefa> <arquivo>`

Configura a tarefa para **escrever a saída em um arquivo**, sobrescrevendo o conteúdo existente (como `>` no shell).

```
processflow> output ordenar resultado.txt
```

### `append <tarefa> <arquivo>`

Igual ao `output`, mas **acrescenta** ao final do arquivo (como `>>` no shell).

```
processflow> append ordenar historico.txt
```

> **Importante:** `input`/`output`/`append` são **configurações** — não executam a tarefa. Os redirecionamentos são aplicados quando a tarefa é executada posteriormente por um `run`. Exemplo:
>
> ```
> processflow> task ordenar /usr/bin/sort
> processflow> input ordenar nomes.txt
> processflow> output ordenar resultado.txt
> processflow> run sequential ordenar
> ```

### `exit`

Encerra o ProcessFlow (nos dois modos).

## 4. Exemplos

### Exemplo 1 — básico

```
processflow> task listar /bin/ls -l
processflow> run listar
```

### Exemplo 2 — pipe

```
processflow> task listar /bin/ls -l
processflow> task ordenar /usr/bin/sort
processflow> task contar /usr/bin/wc -l
processflow> run pipe listar ordenar contar
```

A saída é um **único número**: a contagem de linhas do resultado de `ls -l | sort`.

### Exemplo 3 — redirecionamento

```
processflow> task ordenar /usr/bin/sort
processflow> input ordenar nomes.txt
processflow> output ordenar resultado.txt
processflow> run sequential ordenar
```

A tarefa `ordenar` lê de `nomes.txt` e escreve em `resultado.txt`.

### Exemplo 4 — workflow

```bash
echo "task listar /bin/ls -l" > exemplo.pf
echo "run sequential listar" >> exemplo.pf
echo "exit" >> exemplo.pf
./processflow exemplo.pf
```

## 5. Tratamento de erros

- **Erros fatais (encerram o programa):**
  - número incorreto de argumentos ao iniciar o ProcessFlow;
  - arquivo workflow inexistente ou que não pode ser aberto.

- **Erros não fatais (imprimem mensagem e continuam):**
  - tarefa informada não existe;
  - programa associado à tarefa não existe ou não pode ser executado;
  - arquivo de entrada ou saída não pode ser aberto;
  - job informado não existe;
  - diretório informado em `workdir` não existe.

- **Casos especiais tratados de forma coerente:**
  - linha de comando vazia no prompt;
  - múltiplos espaços em branco em uma linha;
  - workflow sem `exit` e Ctrl+D no modo interativo;
  - processos que terminam com código de saída diferente de zero;
  - processos em paralelo terminando em ordens diferentes.

## 6. Estrutura do projeto

```
processflow.h     → struct Task, constantes e declarações
processflow.c     → main, despacho de comandos e execução (fork/exec/wait/dup2/pipe)
Makefile          → compilação com clang
README.md         → este documento
Relatorio.md      → relatório de desenvolvimento
evidencias.log    → log de evidências (gerado com `script`)
teste.pf          → exemplo de arquivo workflow
```

## Em desenvolvimento

- `workdir <diretório>` — altera o diretório de trabalho das tarefas executadas posteriormente;
- `start <tarefa>` / `jobs` / `wait <jobId>` — execução em background, listagem de jobs e espera por um job específico.