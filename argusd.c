#include "argus.h"

Hist historico = NULL; //Vai manter os histórico das tarefas
int timeMax = 0; //Vai ser onde vai ser guardado o tempo maximo de execução definido pelo cliente
int timeInat = 0; //Vai ser onde vai ser guardado o tempo maximo de inatividade definido pelo lciente
pid_t pidG = 0; //pid do filho que mede o tempo de inatividade

#define MAX_LINE_SIZE 1024

void sigterm_handler(int signum){ //Handler do SIGUSR2
	/*puts("Sending SIGUSR1");
	char* pidSS = malloc(sizeof(char)*10); 
	itoa(pidG,pidSS,10);
	puts(pidSS);*/

	kill(pidG,SIGUSR1);
	//puts("sigterm_handler done");
	_exit(3); //Código 3 -> Terminado (forçado pelo cliente)
}

void sigalrm2_handler(int signum){
	kill(pidG,SIGUSR1);
	_exit(2); //Código 2 -> Terminado (por ultrapassar o tempo máximo de execução)
}

void sigchild_handler(int signum){ //Handler do sigchild
	int s; //Vamos usar esta variável para guardar o exit status to processo_filho que terminou.
	pid_t pid; //Vamos usar esta variável para guardar o pid do processo_filho que terminou.
	pid = wait(&s); //Guarda o pid e o exit status do processo_filho que terminou.
	historico = atualizaTask(pid,WEXITSTATUS(s),historico); //atualiza o estado da tarefa correspondente ao pid dado para o estado lido pelo WEXITSTATUS(s).
}

char* parse(char *aux){ //Dada uma string que começa e acaba com "'", retira-os e retorna a string resultante.
	int size = strlen(aux);
	char* ret = malloc(sizeof(char) * size);
	int i;
	int j = 0;
	int flag = 0; //Numero de apostrofes encontrado até ao momento
	for(i = 0; i < strlen(aux) && j < size && flag != 2; i++){
		if(aux[i] == 39 && flag == 1){ //Caso em que vai encontrar o segundo apostrofe
			ret[j] = '\0'; //Põe um fim à string
			j = size+1; //Quebra o variante do ciclo
			flag++; //Quebra o variante do ciclo
		}
		else if(aux[i] == 39){ //Caso em que encontra o primeiro apóstrofe
			flag++; //Regista que vimos um apóstrofe
		}
		else{ //Parte em que copia o conteúdo da String dada para a String nova
			ret[j] = aux[i];
			j++;
		}
	}
	
	return ret; //String arg sem os apostrofes
}

int	hasChar(char* s, char c){ //Dada uma string e um char e verifica se a string contém esse mesmo char
	for(int i = 0; i < strlen(s); i++){
		if(s[i] == c){
			return 1;
		}
	}
	return 0;
}

char* generateTask(char* s, int nr){ //Cria a string informativa da criação de uma nova tarefa de numero 'nr'.
	char* num = malloc(sizeof(char) * 2);
	itoa(nr,num,10);
	char* new = malloc(sizeof(strlen(s)) + strlen(num) + 1);
	strcpy(new,s);
	strcat(new,num);
	strcat(new,"\n");
	return new;
}

int main(int argc, char* argv[]){
	
	int fd; //File descriptor do cliente para o servidor
	int fdr; //File descriptor do servidor para o cliente
	char buffer[MAX_LINE_SIZE]; //Buffer para a leitura e escrita nos pipes
	int bytes_lidos; //Nr de bytes_lidos em cada read
	char* exec; //Esta string vai ser o comando a executar
	char* aux; //Esta string vai ser os argumentos rodeados de apostrofes
	char* command; //Esta string vai ser os argumentos
	int tasknr = 1; //Contador de tarefas no servidor
	char* newTask = "nova tarefa #"; //String auxiliar da string abaixo
	char* task; //String informativa da criação de novas tarefas
	pid_t pid; //pid usado para os forks
	int i; //variável usada para iterar ciclos

	signal(SIGCHLD, sigchild_handler); //Atribuição do sigchild_handler ao sinal SIGCHILD
	signal(SIGALRM, sigalrm2_handler); //Atribuição do sigalrm_handler para o tempo maximo de execução
	signal(SIGUSR2, sigterm_handler); //Atribuição do sigterm_handler para terminar uma tarefa
	signal(SIGUSR1, sigkillall_handler); //Atribuição do sigkillall_handler ao sinal SIGUSR1, passado nos handlers acima

	int fdlidxw = open("log.idx", O_CREAT | O_TRUNC | O_WRONLY, 0666);
	int fdlidxr = open("log.idx", O_RDONLY, 0666);

	int fdlogw = open("log", O_CREAT | O_TRUNC | O_WRONLY, 0666);
	int fdlogr = open("log", O_RDONLY, 0666);

	if((fdr = open("pipe_servidor_cliente",O_WRONLY)) == -1){ //Abre o FIFO entre o servidor e o cliente para servir apenas de escrita
		perror("Open");
	}

	while(1){
		if((fd = open("pipe_cliente_servidor", O_RDONLY)) == -1){ //Abre o FIFO entre o cliente e o servidor para servir apenas de leitura
			perror("Open");
		}

		while((bytes_lidos = read(fd,buffer,MAX_LINE_SIZE)) > 0){ //Lê a info passada pelo cliente através do FIFO

			/* Caso o buffer lido seja de apenas 2 caracteres, quer dizer que não tem argumentos e será apenas do tipo -x em que x é uma das opções do programa quando o cliente executa com argumentos
			 * Caso o buffer lido tenha um espaço, quer dizer que tem argumentos.
			 * Se não se verificar nenhuma das duas quer dizer que o buffer tem apenas uma palavra que é uma opção do argus.
			 */
			if(hasChar(buffer,' ')){

				exec = strtok(buffer, " "); //Separa o comando dos argumentos
			
			}else if(strlen(buffer) == 2){

				exec = malloc((sizeof(char) * 2));
				for(i = 0; i < strlen(buffer); i++){
					exec[i] = buffer[i];
				}
				exec[i] = '\0';
			} else {
		
				exec = malloc((sizeof(char) * strlen(buffer)) -1);
				for(i = 0; i < strlen(buffer)-1; i++){
					exec[i] = buffer[i];
				}
				exec[i] = '\0';
			}
			
			if((strcmp(exec,"executar") == 0) || (strcmp(exec,"-e") == 0)){ //As duas opções possíveis para a execução de tarefas
				aux = strtok(NULL,"\0"); //Comando que o user quer executar
			
				command = parse(aux); //Tira os parenteses do comando

				if((pid = fork()) == 0){ //Criamos um filho que vai executar esse mesmo comando
					signal(SIGCHLD, SIG_DFL);
					if((pidG = fork()) == 0){
						executar(command, timeInat, fdlidxw, fdlogw);	//Execução do comando

						_exit(1); //Quando o filho acaba de executar o comando, dá exit com o código 1 (corresponde a "tarefa concluída")
					}
					if(timeMax != 0){
						alarm(timeMax);	
					}
					int statSon;
					wait(&statSon);
					_exit(WEXITSTATUS(statSon));
				}

				historico = addTask(pid,tasknr,command,historico); //adiciona a Task ao historico
				
				task = generateTask(newTask,tasknr); //Criação da string que vai ser passada ao client
				tasknr++;							 // a avisar da criação da nova tarefa
				write(fdr,task,strlen(task));

				//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
   	 			memset(command,0,strlen(command));
    			memset(aux,0,strlen(aux));

			} else if((strcmp(exec,"listar") == 0) || (strcmp(exec,"-l") == 0)){
				
				printListar(fdr,historico); //Apresenta a listagem das tasks em execução ao client

				//Limpa o espaço de memória das strings usadas
				memset(buffer,0,MAX_LINE_SIZE);

    		} else if((strcmp(exec,"historico") == 0) || (strcmp(exec,"-r") == 0)){

    			printHist(fdr,historico); //Apresenta o histórico ao cliente

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);

    		} else if((strcmp(exec,"tempo-execucao") == 0) || (strcmp(exec,"-m") == 0)){

    			aux = strtok(NULL,"\0"); //tempo que o user quer definir como tempo maximo de execução
    			command = parse(aux); //tira os apostrofes do tempo
    			timeMax = atoi(command); //passa a string para um int

    			//Criação de uma string que informa o cliente da definição do tempo máximo escolhido
    			char* tempoExec = malloc(sizeof(char) * 128);
    			strcpy(tempoExec,"Tempo de execução definido para ");
    			strcat(tempoExec,command);
    			strcat(tempoExec," segundos.\n");
    			write(fdr,tempoExec,strlen(tempoExec));

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
   	 			memset(command,0,strlen(command));
    			memset(aux,0,strlen(aux));

    		} else if((strcmp(exec,"terminar") == 0) || (strcmp(exec,"-t") == 0)){

    			aux = strtok(NULL,"\0"); //numero da tarefa que o cliente quer terminar
    			command = parse(aux); //tira os apostrofes do numero
    			int tarefa = atoi(command); //passa a string para um int
    			int res = taskExists(historico,tarefa);
    			if(res == 1){ //Se a tarefa existir no historico
    				
    				pid = getPidHist(historico,tarefa); //Devolve o pid que realiza a tarefa.
    				kill(pid,SIGUSR2); //Envia o sinal SIGUSR2 (definido no executar.c) para o pid que realiza a tarefa.
    				
    			} else if(res == 0){ //se a tarefa não existir

    				//Controlo de erros, avisa o cliente que a tarefa pedida não existe, logo não pode ser terminada	
    				char* warning = "A tarefa não existe.\n";
    				write(fdr,warning,strlen(warning));

    			} else if (res == 2) { //se a tarefa existir mas já não estiver em execução

    				//Controlo de erros, avisa o cliente que a tarefa pedida já tinha terminado, logo não pode ser terminada novamente
    				char* warning = "A tarefa já tinha terminado.\n";
    				write(fdr,warning,strlen(warning));
    			}

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
   	 			memset(command,0,strlen(command));
    			memset(aux,0,strlen(aux));

    		} else if(strcmp(exec,"ajuda") == 0){
    			char* tempoInatividade = "tempo-inatividade 'segs'\n";
    			char* tempoExec = "tempo-execucao 'segs'\n";
    			char* executarS = "executar 'p1 | p2 ... | pn'\n";
    			char* termS = "terminar 'tarefa'\n";
    			char* listarS = "listar\n";
    			char* histS = "historico\n";
    			char* outS = "output 'tarefa'\n";
    			write(fdr,tempoInatividade,strlen(tempoInatividade));
    			write(fdr,tempoExec,strlen(tempoExec));
    			write(fdr,executarS,strlen(executarS));
    			write(fdr,termS,strlen(termS));
    			write(fdr,listarS,strlen(listarS));
    			write(fdr,histS,strlen(histS));
    			write(fdr,outS,strlen(outS));
    		} else if((strcmp(exec,"tempo-inatividade") == 0) || (strcmp(exec,"-i") == 0)){


    			aux = strtok(NULL,"\0"); //tempo que o user quer definir como tempo maximo de execução
    			command = parse(aux); //tira os apostrofes do tempo
    			timeInat = atoi(command); //passa a string para um int

    			//Criação de uma string que informa o cliente da definição do tempo máximo escolhido
    			char* tempoExec = malloc(sizeof(char) * 128);
    			strcpy(tempoExec,"Tempo de inatividade definido para ");
    			strcat(tempoExec,command);
    			strcat(tempoExec," segundos.\n");
    			write(fdr,tempoExec,strlen(tempoExec));

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
   	 			memset(command,0,strlen(command));
    			memset(aux,0,strlen(aux));

    		} else if(strcmp(exec,"output") == 0){
    			aux = strtok(NULL,"\0"); //tempo que o user quer definir como tempo maximo de execução
    			command = parse(aux); //tira os apostrofes do tempo
    			int out = atoi(command); //passa a string para um int

    			int posFile = (out-1) * 10; //calcula a posição no log.idx
    			lseek(fdlidxr, posFile, SEEK_SET);
    			char* leitura = malloc(sizeof(char) * 10);
    			read(fdlidxr,leitura,10);
    			int posI = atoi(leitura);
    			memset(leitura,0,10);

    			read(fdlidxr,leitura,10);
    			int posF = atoi(leitura);
    			if(posF == 0){
    				posF = lseek(fdlogr,0,SEEK_END);
    			}
    			int sizeOfOut = posF - posI;

    			lseek(fdlogr,posI,SEEK_SET);
    			char* resultado = malloc(sizeof(char) * 4096);
    			bytes_lidos = read(fdlogr,resultado,sizeOfOut);

    			write(fdr,resultado,bytes_lidos);

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
   	 			memset(command,0,strlen(command));
    			memset(aux,0,strlen(aux));
    			memset(leitura,0,10);
    			memset(resultado,0,4096);

    		} else {
    			char* aviso = "Opção inválida. Escreva 'ajuda' para ver a lista de comandos disponíveis.\n";
    			write(fdr,aviso,strlen(aviso));

    			//Limpa o espaço de memória das strings usadas
    			memset(buffer,0,MAX_LINE_SIZE);
    		}
		}
		close(fd); //Fecha o pipe
		close(fdr);
	}
	return 0;
}	