int main() {

	comando_t comando;

	unlink(pipe_name);

	int status = mkfifo(pipe_name, S_IWUSR);
	if (status != 0)
		perror("Erro a abrir o pipe terminal.");


	int pipe = open("i-banco-pipe",O_WRONLY);
	if (pipe == -1)
		perror("Erro a abrir o pipe.");


	while (1) {

		read(pipe, &comando, sizeof(comando_t));

	}

	unlink(pipe_name);
}

void processCommand(comando_t comando) {

	
}