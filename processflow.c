#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "processflow.h"

int main(int argc, char **argv) {
    Task tasks [MAX_TASKS];
    int numTasks = 0;
    
    FILE *input = stdin;
    int interativo = 1;

    if (argc > 2) {
        fprintf(stderr, "Uso: ./processflow [workflowFile]\n");
        return 1;
    }
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Erro ao abrir workflow.\n");
            return 1;
        }
        interativo = 0;
    }

    char linha[1024];
    while (1) {
        if (interativo) {
            printf("processflow> ");
            fflush(stdout);
        }
        if (!fgets(linha, sizeof(linha), input)) break;
        linha[strcspn(linha, "\n")] = '\0';

        if (strlen(linha) == 0) continue;
        if (!interativo) printf("%s\n", linha);

        if (strcmp(linha, "exit") == 0) break;

        // tokenização
        char *tokens[MAX_TOKENS];
        int n = 0;

        char *palavra = strtok(linha, " \t");
        while (palavra != NULL && n < MAX_TOKENS - 1) {
            tokens[n++] = palavra;
            palavra = strtok(NULL, " \t");
        }
        tokens[n] = NULL;
        if (n == 0) continue;

        // task
        if (strcmp(tokens[0], "task") == 0) {  
            if (n < 3) {
                fprintf(stderr, "Uso: task <nome> <programa> <...>\n");
                continue;
            }
            if (numTasks == MAX_TASKS) {
                fprintf(stderr, "Erro: limite de tasks atingido\n");
                continue;
            }

            int existe = 0;
            for (int i = 0; i < numTasks; i++) {
                if (strcmp(tokens[1], tasks[i].nome) == 0) {
                    existe = 1;
                    break;
                }   
            }
            if (existe) {
                fprintf(stderr, "Erro: task %s já existe\n", tokens[1]);
                continue;
            }
            
            Task *t = &tasks[numTasks];
            t->nome = strdup(tokens[1]);
            t->programa = strdup(tokens[2]);
            t->args[0] = strdup(tokens[2]);
            int j = 1;
            for (int i = 3; i < n; i++) {
                t->args[j++] = strdup(tokens[i]);
            }
            t->args[j] = NULL;
            numTasks++;
        }

        //run
        if (strcmp(tokens[0], "run") == 0) {
            int pipes[MAX_TASKS - 1][2];

            if (n < 2) {
                fprintf(stderr, "Uso: run <tarefa> | run <modo> <tarefa1> <...>\n");
                continue;
            }

            char *modo;
            int start;
            if (strcmp(tokens[1], "sequential") == 0 || strcmp(tokens[1], "parallel") == 0 || strcmp(tokens[1], "pipe") == 0) {
                modo = tokens[1];
                start = 2;
                if (n < 3) {
                    fprintf(stderr, "Uso: run <modo> <tarefa1> <...>\n");
                    continue;
                }
            } else {
                modo = "sequential";   
                start = 1;
            }

            pid_t pids[MAX_TASKS];  
            int k = 0;              
            int status;            

            if (strcmp(modo, "pipe") == 0) {
                for (int i = 0; i < n - start - 1; i++) {
                    pipe(pipes[i]);
                }
            }

            for (int i = start; i < n; i++) {
                Task *t = NULL;
                for (int j = 0; j < numTasks; j++) {
                    if (strcmp(tokens[i], tasks[j].nome) == 0) {
                        t = &tasks[j];
                        break;
                    }
                }
                if (t == NULL) {
                    fprintf(stderr, "Erro: tarefa '%s' não existe.\n", tokens[i]);
                    continue;
                }
                
                pid_t pid = fork();
                if (pid < 0) {
                    fprintf(stderr, "Erro: fork falhou.\n");
                    continue;
                }
                else if (pid == 0) {
                    if (strcmp(modo, "pipe") == 0) {
                        int pos = i - start;
                        if (pos > 0) {
                            dup2(pipes[pos - 1][0], 0);
                        }
                        if (pos < n - start - 1) {
                            dup2(pipes[pos][1], 1);
                        }
                        for (int p = 0; p < n - start - 1; p++) {
                            close(pipes[p][0]);
                            close(pipes[p][1]);
                        }
                    }
                    execvp(t->programa, t->args);
                    perror("Erro: exec falhou.");
                    _exit(1);
                }
                else if (pid > 0) {
                    if (strcmp(modo, "sequential") == 0) {
                        waitpid(pid, &status, 0);
                    }
                    else if (strcmp(modo, "parallel") == 0 || strcmp(modo, "pipe") == 0) {
                        pids[k++] = pid;
                    }
                }
            }
            if (strcmp(modo, "pipe") == 0) {
                for (int p = 0; p < n - start - 1; p++) {
                    close(pipes[p][0]);
                    close(pipes[p][1]);
                }
            }
            if (strcmp(modo, "parallel") == 0 || strcmp(modo, "pipe") == 0) {
                for (int i = 0; i < k; i++) {
                    waitpid(pids[i], &status, 0);
                }
            }
        }

        //input
        if (strcmp(tokens[0], "input") == 0) {
            if (n < 3) {
                fprintf(stderr, "Uso: input <tarefa> <arquivo>\n");
                continue;
            }
            
            Task *t = NULL;
            for (int j = 0; j < numTasks; j++) {
                if (strcmp(tokens[1], tasks[j].nome) == 0) {
                    t = &tasks[j];
                    t->inputFile = strdup(tokens[2]);
                    break;
                }
            }
            if (t == NULL) {
                fprintf(stderr, "Erro: tarefa não existe.\n");
                continue;
            }
        }

        //output
        if (strcmp(tokens[0], "output") == 0) {
            if (n < 3) {
                fprintf(stderr, "Uso: output <tarefa> <arquivo>\n");
                continue;
            }
            
            Task *t = NULL;
            for (int j = 0; j < numTasks; j++) {
                if (strcmp(tokens[1], tasks[j].nome) == 0) {
                    t = &tasks[j];
                    t->outputFile = strdup(tokens[2]);
                    t->append = 0;
                    break;
                }
            }
            if (t == NULL) {
                fprintf(stderr, "Erro: tarefa não existe.\n");
                continue;
            }
        }

        //append
        if (strcmp(tokens[0], "append") == 0) {
            if (n < 3) {
                fprintf(stderr, "Uso: append <tarefa> <arquivo>\n");
                continue;
            }
            
            Task *t = NULL;
            for (int j = 0; j < numTasks; j++) {
                if (strcmp(tokens[1], tasks[j].nome) == 0) {
                    t = &tasks[j];
                    t->outputFile = strdup(tokens[2]);
                    t->append = 1;
                    break;
                }
            }
            if (t == NULL) {
                fprintf(stderr, "Erro: tarefa não existe.\n");
                continue;
            }
        }
    }
    if (argc == 2) fclose(input);
    return 0;
}