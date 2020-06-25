#include "executar.h"

int size = 0;
pid_t pids[150];

void sigalrm_handler(int signum){ //Handler do SIGALRM
	for(int i = 0; i < size; i++){ //Percorre o array com os pids dos filhos e passa o sinal SIGKILL para todos.
		kill(pids[i],SIGKILL);
	}
	_exit(4); //Código 4 -> Terminado (por ultrapassar o tempo máximo de inatividade)
}

void sigkillall_handler(int signum){ //Handler do SIGUSR1
	//puts("cheguei aqui");
	for(int i = 0; i < size; i++){ //Percorre o array com os pids dos filhos e passa o sinal SIGKILL para todos.
		kill(pids[i],SIGKILL);
	}
	//puts("sigkillall_handler done");
	_exit(3); //Código 3 -> Terminado (forçado pelo cliente)
}

int separate(char* execute[10], char* command){ //Cria um array de strings com os comandos passados pelo cliente na execução de tarefas
	int arg = 0; //Numero de argumentos
	int i = 0;
	int j = 0;

	execute[arg] = malloc(sizeof(char)*30); //Aloca espaço para o o arg-1-ésimo comando a ser lido.
 
	while(command[i] != '\0'){ //Vamos percorrer a string command até ao final

		if(command[i] == ' ' && command[i+1] == '|' && command[i+2] == ' '){ //Caso encontre a sequência " | " divide os comandos separados por ela.
			execute[arg][j] = '\0';
			arg++;
			j = 0;
			i += 3;
			execute[arg] = malloc(sizeof(char)*30); //Aloca espaço para o o arg-1-ésimo comando a ser lido.
		} else {
			execute[arg][j] = command[i];
			j++;
			i++;
		}
	}
	execute[arg][j] = '\0';
	return (arg +1);
}

int exec_command(char* command){ //Executa um comando
	char* exec_args[20]; //na posicao 0 vai ter o comando, nas seguintes os argumentos
	char* string; //"buffer" do parse
	int exec_ret = 0; //valor de retorno do exec
	int i = 0; //Vaŕiável iterante

	string = strtok(command," "); //parse

	while(string != NULL){ //Parse
		exec_args[i] = string;
		string = strtok(NULL," ");
		i++;
	}

	exec_args[i] = NULL;
	exec_ret = execvp(exec_args[0], exec_args);
	return exec_ret;
}

int executar(char* command, int timeMax, int fdlidxw, int fdlogw){ //Executa uma tarefa no tempo máximo timeMax
	
	dup2(fdlogw,1);

	int pos = lseek(fdlogw, 0, SEEK_CUR);
	char* posNew = malloc(sizeof(char)*10);
	posNew = itoa(pos,posNew,10);
	strcat(posNew,"\n");
	write(fdlidxw,posNew,10);

	signal(SIGUSR1,sigkillall_handler);
	signal(SIGALRM,sigalrm_handler);

	pid_t pid;
	char* execute[20]; //comandos
	int arg; //nr de comandos

	arg = separate(execute,command);

	int p[arg-1][2]; //matriz com os file descriptors dos pipes
    		
 	//Cria os pipes conforme o número de comandos
	for(int i = 0; i < arg-1; i++){
		if(pipe(p[i]) == -1){
				perror("Pipe não foi criado");
				return -1;
			}
	}
	
	//Cria processos filhos que vão executar cada comando
	for(int i = 0; i < arg; i++){
		if(i == 0){ //1º comando a ser executado
			switch(pid = fork()){
				case -1:
					perror("Fork não foi criado.");
					break;
				case 0: //Código do processo filho (filho 0)
					close(p[i][0]);
					dup2(p[i][1],1);
					close(p[i][1]);
					exec_command(execute[i]);
					_exit(0);
				default:
					alarm(timeMax);
					pids[size] = pid;
					size++;
					close(p[i][1]);	
			}
		} else if (i == arg-1){ //último comando a ser executado
			switch(pid = fork()){
				case -1:
					perror("Fork não foi criado.");
					return -1;
				case 0:
					dup2(p[i-1][0],0);
					close(p[i-1][0]);
					exec_command(execute[i]);
					_exit(0);
				default:
					alarm(timeMax);
					pids[size] = pid;
					size++;
					close(p[i-1][0]);	
			}
		} else {
			switch(pid = fork()){
				case -1:
					perror("Fork não foi criado.");
					break;
				case 0:
					dup2(p[i-1][0],0);
                    close(p[i-1][0]);
					dup2(p[i][1],1);
                    close(p[i][1]);
					exec_command(execute[i]);
					_exit(0);
				default:
					alarm(timeMax);
					pids[size] = pid;
					size++;
					close(p[i-1][0]);
					close(p[i][1]);	
			}
		}
	}
	for(int i = 0; i < arg; i++){
		pid_t terminated_pid = wait(NULL);
	}
}