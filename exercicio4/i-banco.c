#include "commands.h"



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


	unlink(pipe_name);

	status = mkfifo(pipe_name, S_IRWXU);
	if (status != 0)
		perror("Erro a abrir o pipe terminal.");


	int pipe = open(pipe_name,O_RDONLY);
	if (pipe == -1)
		perror("Erro a abrir o pipe.");


	while (running_flag) {
		if (read(pipe, &comando, sizeof(comando_t)) > 0)
			readCommand(comando);
	}


	closeCommandResources();

	close(pipe);
	unlink(pipe_name);

	return 0;
}





