#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define FIFO_NOME "minhaFifoEx2"

int main()
{
  pid_t pid1, pid2;
  int fd;
  char buffer[1024];

  mkfifo(FIFO_NOME, S_IRUSR | S_IWUSR);

  pid1 = fork();
  if (pid1 == 0)
  {
    fd = open(FIFO_NOME, O_WRONLY);
    char msg[] = "Opa, meu amigo, eu sou o filho 1.\n";
    write(fd, msg, strlen(msg));
    close(fd);
    exit(0);
  }

  pid2 = fork();
  if (pid2 == 0)
  {
    fd = open(FIFO_NOME, O_WRONLY);
    char msg[] = "Fala, meu parceiro, eu sou o filho 2.\n";
    write(fd, msg, strlen(msg));
    close(fd);
    exit(0);
  }

  fd = open(FIFO_NOME, O_RDONLY);

  waitpid(pid1, NULL, 0);
  waitpid(pid2, NULL, 0);

  int bytes_lidos = read(fd, buffer, sizeof(buffer) - 1);
  buffer[bytes_lidos] = '\0';
  printf("Pai leu da FIFO o seguinte conteudo:\n%s", buffer);

  close(fd);
  unlink(FIFO_NOME);

  return 0;
}