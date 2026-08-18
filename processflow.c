#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            t->programa =strdup(tokens[2]);
            t->args[0] = strdup(tokens[2]);
            int j = 1;
            for (int i = 3; i < n; i++) {
                t->args[j++] = strdup(tokens[i]);
            }
            t->args[j] = NULL;
            numTasks++;
        }
    }

    if (argc == 2) fclose(input);
    return 0;
}