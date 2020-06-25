#include "argus.h"

#define MAX_LINE_SIZE 1024

int main(int argc, char* argv[]){
	
	char buffer[MAX_LINE_SIZE]; //Buffer para leitura do stdin
	int bytes_lidos; //Número de bytes lidos do stdin
	int fd; //File descriptor do pipe onde o cliente vai escrever
	int fdr; //File descriptor do pipe onde o cliente vai ler

	fdr = open("pipe_servidor_cliente",O_RDONLY); //FIFO entre o cliente e o servidor, no qual o cliente vai ler informação
	
	fd = open("pipe_cliente_servidor", O_WRONLY); //FIFO entre o cliente e o servidor, no qual o cliente vai escrever informação 
	
	if(fork() == 0){ //É criado um filho que vai ler a informação escrita pelo servidor no FIFO fdr e escreve para o client
		while((bytes_lidos = read(fdr,buffer,MAX_LINE_SIZE)) > 0){
			write(1,buffer,bytes_lidos);
		}
		_exit(0);
	}

	if(argc == 2){ //Caso o buffer lido seja de apenas 2 caracteres, quer dizer que não tem argumentos e será apenas do tipo -x em que x é uma das opções do programa quando o cliente executa com argumentos
		buffer [0] = '\0';
		strcat(buffer,argv[1]); //argv[1] é o comando passado pelo cliente

		write(fd,buffer,strlen(buffer)); //Escreve o buffer no fifo do qual o servidor vai ler

		memset(buffer,0,strlen(buffer)); //Limpa o espaço de memória das strings usadas

		memset(buffer,0,strlen(buffer)); //Limpa o espaço de memória das strings usadas

	}else if(argc > 1){ //Se for mais do que um argumento (maior que 2 na verdade)

		buffer[0] = '\0';				//Cria uma string do formato -> "comando 'args'"
		strcat(buffer,argv[1]);
		strcat(buffer," '");
		strcat(buffer,argv[2]);
		strcat(buffer,"'");
		strcat(buffer,"\n");

		write(fd,buffer,strlen(buffer)); //Escreve o buffer no fifo do qual o servidor vai ler
		
		memset(buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas/

		memset(buffer,0,strlen(buffer)); //Limpa o espaço de memória das strings usadas

	} else { //Sem argumentos
		char* cmd = "argus$ "; //Prompt escolhido
		write(1,cmd,strlen(cmd)); //Imprime o shell prompt

		while((bytes_lidos = read(0,buffer,MAX_LINE_SIZE)) > 0){ //Lê o input do client do stdin e escreve-o no buffer
			if(write(fd,buffer,bytes_lidos) == -1){ //Escreve o buffer no FIFO fd
				perror("write");
			}

			memset(buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas

			sleep(1); //Evita que o write execute antes da hora 
			write(1,cmd,strlen(cmd)); //Imprime o shell prompt

			memset(buffer,0,MAX_LINE_SIZE); //Limpa o espaço de memória das strings usadas
		
		}
	}
	
	close(fd); //Fechar o file descriptor do FIFO
	return 0;
}
