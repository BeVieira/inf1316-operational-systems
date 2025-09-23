#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int pipefd[2];
	pid_t pid;
	char buffer[100];
	int bytes_read;

	if (pipe(pipefd) == -1) {
		perror("pipe");
		return 1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return 1;
	}

	if (pid == 0)
	{
		close(pipefd[0]);
		write(pipefd[1], "Salve paizao!\n", 13);
		close(pipefd[1]);
		return 0;
	}

	close(pipefd[1]);
	bytes_read = read(pipefd[0], buffer, 100);
	printf("Pai lendo bytes do filho: %d\n", bytes_read);
	printf("Filhao: %s\n", buffer);
	close(pipefd[0]);

	return 0;
}