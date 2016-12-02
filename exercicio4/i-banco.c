#include <stdio.h>
#include "commands.h"

/**
 * GRUPO 4
 * Andre Fonseca 84698
 * Isabel Dias 84726
 * Projecto de SO 
 * exercicio 4
 */


int running_flag;



int main() {

	/* inicializcoes */
	int status;
	comando_t comando;
	running_flag = 1;

	char pipe_name[] = "i-banco-pipe";

	initializeCommandResources();

	/* i-banco start */
	printf("Bem-vinda/o ao i-banco\n\n");


	if(unlink(pipe_name) != 0)
		if(errno != ENOENT)
			perror("initialization - error no unlink do default pipe");

	status = mkfifo(pipe_name, S_IRWXU);
	if (status != 0){
		perror("initialization - Erro a abrir o default pipe.");
		return 0;
	}


	int pipe = open(pipe_name,O_RDONLY);
	if (pipe == -1){
		perror("Erro a abrir o pipe.");
		return 0;
	}



	/* loop running program */
	while (running_flag) {
		if (read(pipe, &comando, sizeof(comando_t)) > 0)
			readCommand(comando);
	}



	closeCommandResources();

	if(close(pipe) != 0)
		perror("unable to close pipe");
	
	if (unlink(pipe_name) != 0)
		if(errno != ENOENT)
			perror("unable to unlink pipefile");


	return 0;
}





