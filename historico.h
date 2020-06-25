#ifndef historico_h
#define historico_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct hist{
	pid_t pid; //PID do processo que executou a tarefa
	int exec; //0 se em execução, 1 terminado, 2 se terminado por tempo de execução máximo excedido, 3 se forçado a fechar pelo cliente;
	int tasknr; //Numero da tarefa
	char* task; //Tarefa
	struct hist *prox; //O Histórico segue o modelo de uma stack em que quando algo é adicionado, é adicionado no início da stack. Assim a prox é a tarefa anterior (ou seja terá tasknr = taskr-1)
} *Hist;

Hist atualizaTask(pid_t pid, int s, Hist h);
char* itoa(int value, char* result, int base);
Hist addTask(pid_t pid, int tasknr, char* task, Hist hist);
void printListar(int fdr, Hist hist);
void printHist(int fdr, Hist hist);
int taskExists(Hist historico, int tarefa);
pid_t getPidHist(Hist historico, int tarefa);

#endif