#include "historico.h"

Hist atualizaTask(pid_t pid, int s, Hist h){ //Dado um pid, um estado de execução e o histórico, atualiza a tarefa desse pid para o estado recebido.
	Hist aux = h; //Variável de instância usada para percorrer o histórico.
	while(aux != NULL && aux->pid != pid){ //Procura pela tarefa com o pid recebido.
		aux = aux->prox; //Iteração numa lista ligada.
	}
	if(aux != NULL && aux->pid == pid){ //Encontrando a tarefa com o pid recebido.
		aux->exec = s;					//Mudando o seu estado de execução para o recebido nos args.
	}
}

char* itoa(int value, char* result, int base) { //Encontrei esta função para passar de um inteiro para string, looks good.
    if (base < 2 || base > 36) { *result = '\0'; return result; } //verifica a validade da base

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

Hist addTask(pid_t pid, int tasknr, char* task, Hist hist){ //Adiciona uma nova tarefa ao histórico
	Hist aux = malloc(sizeof(struct hist));

	aux->pid = pid;
	aux->exec = 0;
	aux->tasknr = tasknr;
	aux->task = malloc(sizeof(char) * strlen(task));
	strcpy(aux->task,task);
	aux->prox = hist;

	return aux;
}

void printListar(int fdr, Hist hist){ //Imprime a lista das tarefas em execução
	Hist aux = hist; //variável de instância usada para percorrer o hostórico sem alterar o original

	char* line = malloc(sizeof(char) * 128); //Vamos criar uma linha para escrever para o client
	char* num = malloc(sizeof(char) * 3); //Vamos guardar aqui o numero da task em string

	int execs = 0; //Vamos ter esta flag para verificar se no final de percorrer o histórico encontramos alguma tarefa que estava em execução ou não
	while(aux != NULL){ //Percorremos o histórico até ao final
		if(aux->exec == 0){ //Só nos interessam as tarefas do histórico que estão com o estado de execução 0
			
			//Cria a linha que vai ser escrita
			strcpy(line,"#");
			strcat(line,(itoa(aux->tasknr, num, 10)));
			strcat(line,": ");
			strcat(line,aux->task);
			strcat(line,"\n");
			
			//Escreve a linha para o pipe entre o servidor e o cliente para ser apresentada ao cliente
			write(fdr,line,strlen(line));
			
			//Limpa o espaço de memória das strings usadas
			memset(num,0,3);
			memset(line,0,128);

			//Aumenta a flag para sabermos que já vimos tarefas em execução
			execs++;
		}

		aux = aux->prox; //Iteração de uma lista ligada
	}
	if(execs == 0){ //Caso em que atingimos o final do histórico mas não encontramos ennhuma tarefa em execução
		char* new = "Nenhuma tarefa em execução.\n";
		write(fdr,new,strlen(new));
	}
}

void printHist(int fdr, Hist hist){ //Imprime o histórico das tarefas
	Hist aux = hist; //variável de instância usada para percorrer o hostórico sem alterar o original

	char* line = malloc(sizeof(char) * 128); //Vamos criar uma linha para escrever para o client
	char* num = malloc(sizeof(char) * 3); //Vamos guardar aqui o numero da task em string

	int execs = 0; 
	while(aux != NULL){ //Percorremos o histórico até ao final
		if(aux->exec > 0){ //Só nos interessam as tarefas do histórico que estão com o estado de execução maior que 0
			
			//Cria a linha que vai ser escrita
			strcpy(line,"#");
			strcat(line,(itoa(aux->tasknr, num, 10)));
			if(aux->exec == 1){
				strcat(line,", concluído: ");
			} else if(aux->exec == 2){
				strcat(line,", max execução: ");
			} else if(aux->exec == 3){
				strcat(line,", terminado: ");
			} else if(aux->exec == 4){
				strcat(line,", max inatividade: ");
			}
			strcat(line,aux->task);
			strcat(line,"\n");
			
			//Escreve a linha para o pipe entre o servidor e o cliente para ser apresentada ao cliente
			write(fdr,line,strlen(line));
			
			//Limpa o espaço de memória das strings usadas
			memset(num,0,3);
			memset(line,0,128);

			execs++;
		}
		
		aux = aux->prox; //Iteração de uma lista ligada
	}
	if(execs == 0){ //Caso em que atingimos o final do histórico mas não encontramos ennhuma tarefa em execução
		char* new = "Nenhuma tarefa no histórico.\n";
		write(fdr,new,strlen(new));
	}
}

int taskExists(Hist historico, int tarefa){ //Verifica se uma tarefa existe no histórico dado o seu numero de tarefa
	Hist aux = historico; //variável de instância usada para percorrer o hostórico sem alterar o original

	while(aux != NULL && aux->tasknr != tarefa){ //vamos percorrer o histórico até encontrarmos, ou o seu final ou a tarefa com o nr de tarefa dado como argumento
		aux = aux->prox; //Iteração de uma lista ligada
	}
	if(aux != NULL && aux->tasknr == tarefa && aux->exec == 0){ //Caso encontremos a tarefa que procuravamos retornamos 1
		return 1;
	} else if (aux != NULL && aux->tasknr == tarefa){
		return 2;
	}
	return 0;
}

pid_t getPidHist(Hist historico, int tarefa){ //Devolve o pid da tarefa que procuramos dado o seu nr de tarefa
	Hist aux = historico; //variável de instância usada para percorrer o hostórico sem alterar o original

	while(aux->tasknr != tarefa){ //Este método é executado depois de confirmar a existência da tarefa, então percorremos até a encontrar
		aux = aux->prox; //Iteração de uma lista ligada
	}
	if(aux->tasknr == tarefa){ //Quando encontramos, damos return ao pid que lhe é associado
		return aux->pid;
	}
}