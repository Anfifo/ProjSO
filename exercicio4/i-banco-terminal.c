#include <stdio.h>
#include "inputProcessing.h"


int i_banco_pipe;

char pipe_name[20];



void addToPipe(comando_t comando){
	comando.pid = getpid();

	if (write(i_banco_pipe, &comando, sizeof(comando_t)) == -1)
		perror("Erro na escrita no i-banco-pipe.");


	int fd = open(pipe_name,O_RDONLY);
	if (fd < 0)
		perror("Erro ao abrir o pipe de retorno.");


	char buffer[100];
	read(fd, buffer, 100);
	printf("%s\n", buffer);

}

void processInput(){

	char *args[MAXARGS + 1];
	char buffer[BUFFER_SIZE]; 


	while (1) {
		int numargs;
		numargs = readLineArguments(args, MAXARGS+1, buffer, BUFFER_SIZE);


		/* 
		 * Sair / Sair agora
		 * termina o programa apos todos os comandos  no cmd_buffer
		 * sejam concluidos pelos threads e que estes tenham terminado
		 *
		 * no caso de EOF (end of file) do stdin ou COMANDO_SAIR
		 * arg1(optional) char* COMANDO_SAIR_AGORA
		 * com arg1 manda os processos filhos terminarem
		 * sem arg1 COMANDO_SAIR_AGORA espera que os filhos terminem  
		 */
		if ( numargs < 0 ||
			(numargs > 0 && 
			(strcmp(args[0], COMANDO_SAIR) == 0) )){
			
			comando_t comando;

			if (numargs > 2 || numargs < 1) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SAIR);
				continue;
			}

			comando.operacao = OPERACAO_SAIR;


			if (numargs == 2 && (strcmp(args[1], COMANDO_SAIR_AGORA) == 0) ){	
				comando.operacao = OPERACAO_SAIR_AGORA;
			} 

			addToPipe(comando);
		}

		else if (numargs < 0 || 
				(numargs > 0 && 
				(strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0 ))) {

			printf("i-banco-terminal vai terminar.\n");
			sleep(2);
			return;
		}
	


		else if (numargs == 0)
			/* Nenhum argumento; ignora e volta a pedir */
			continue;
	

		/* 
		 * Debitar 
		 * arg 1 int - idConta1
		 * arg 2 int - valor
		 * adiciona a ao cmd_buffer a OPERACAO_DEBITAR  
		 */
		else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
			   continue;
			}

			comando_t comando;

			comando.operacao = OPERACAO_DEBITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToPipe(comando);
		}

		/*
		 * Creditar 
		 * arg 1 int - idConta1
		 * arg 2 int - valor
		 * adiciona ao cmd_buffer a OPERACAO_CREDITAR
		 */
		else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
			if (numargs < 3) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
				continue;
			}

			comando_t comando;

			comando.operacao = OPERACAO_CREDITAR;
			comando.idConta1 = atoi(args[1]);
			comando.valor = atoi(args[2]);

			addToPipe(comando);
		}


		/*
		 * Ler Saldo
		 * arg 1 int - idConta1
		 * adicionar ao cmd_buffer a OPERACAO_LER_SALDO
		 */
		else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
			if (numargs < 2) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
				continue;
			}
			
			comando_t comando;

			comando.operacao = OPERACAO_LER_SALDO;
			comando.idConta1 = atoi(args[1]);

			addToPipe(comando);
		}

		

		/* 
		 * Simular 
		 * arg 1 int - nr_de_anos
		 * faz a simulacao dos nr_de_anos sob as contas existentes
		 *
		 * para nao ocorrer erros, espera que nao haja nenhuma thread a bloquear
		 */
		else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
			
			if (numargs != 2 || atoi(args[1]) < 0) {
				printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_SIMULAR);
				continue;
			}
			comando_t comando;

			comando.operacao = OPERACAO_SIMULAR;
			comando.valor = atoi(args[1]);

			addToPipe(comando);
		}


		/*
		 * Transferir
		 * arg 1 int - idConta Origem
		 * arg 2 int - idConta Destino
		 * arg 3 int - valor 
		 * transfere valor de conta origem para conta destino
		 */
		else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {

			if (numargs != 4) {
				printf("%s: Sintaxe inválida, tente de novo. \n", COMANDO_TRANSFERIR);
				continue;
			}

			comando_t comando;

			int id1 = atoi(args[1]);
			int id2 = atoi(args[2]);
			int valor = atoi(args[3]);

			if (id1 == id2 || id1 < 0 || id2 < 0 || valor <= 0){
				printf("%s: Sintaxe inválida, tente de novo. \n", COMANDO_TRANSFERIR);
				continue;
			}

			comando.operacao = OPERACAO_TRANSFERIR;
			comando.idConta1 = id1;
			comando.idConta2 = id2;
			comando.valor = valor;

			addToPipe(comando);
		}


		else {
			printf("Comando desconhecido. Tente de novo.\n");
		}
	}
}

int main() {

	printf("Bem vinda/o ao i-banco-terminal.\n\n");

	i_banco_pipe = open("i-banco-pipe",O_WRONLY);
	if (i_banco_pipe == -1)
		perror("Erro a abrir o pipe.");

	printf("- Connection Successful -\n\n");
	
	snprintf(pipe_name, 50, "%d", getpid());

	unlink(pipe_name);


	int status = mkfifo(pipe_name, S_IRWXU);
	if (status != 0)
		perror("Erro a abrir o pipe terminal.");


	processInput();
	
	unlink(pipe_name);
	
	close(i_banco_pipe);

	return 0;
}