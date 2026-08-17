#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 64

int main(int argc, char **argv) {
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

        char *tokens[MAX_TOKENS];
        int n = 0;

        char *palavra = strtok(linha, " \t");
        while (palavra != NULL && n < MAX_TOKENS - 1) {
            tokens[n++] = palavra;
            palavra = strtok(NULL, " \t");
        }
        tokens[n] = NULL;
    }

    if (argc == 2) fclose(input);
    return 0;
}