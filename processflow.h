#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#define MAX_TOKENS 104
#define MAX_TASKS 104
#define MAX_ARGS 104

typedef struct {
    char *nome;
    char *programa;
    char *args[MAX_ARGS];
    char *inputFile;
    char *outputFile;
    int append;
} Task;

extern Task tasks[MAX_TASKS];
extern int numTasks;

#endif

