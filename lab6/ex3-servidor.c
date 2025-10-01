#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define FIFO_SERVIDOR "servidor_fifo"

typedef struct
{
  pid_t pid_cliente;
  char texto[256];
} Mensagem;

int main()
{
  int fd_leitura, fd_cliente;
  Mensagem msg;
  char fifo_cliente_nome[50];

  unlink(FIFO_SERVIDOR);
  if (mkfifo(FIFO_SERVIDOR, S_IRUSR | S_IWUSR) == -1)
  {
    perror("mkfifo");
    return 1;
  }

  printf("Servidor online. Use Ctrl+C para encerrar.\n");

  while (1)
  {
    fd_leitura = open(FIFO_SERVIDOR, O_RDONLY);
    if (fd_leitura == -1)
    {
      perror("open (leitura)");
      return 1;
    }

    printf("Aguardando cliente...\n");

    ssize_t bytes_lidos;
    while ((bytes_lidos = read(fd_leitura, &msg, sizeof(Mensagem))) > 0)
    {
      printf("Servidor recebeu de [%d]: %s\n", msg.pid_cliente, msg.texto);
      for (int i = 0; msg.texto[i]; i++)
      {
        msg.texto[i] = toupper(msg.texto[i]);
      }

      sprintf(fifo_cliente_nome, "cliente_%d_fifo", msg.pid_cliente);
      fd_cliente = open(fifo_cliente_nome, O_WRONLY);
      if (fd_cliente != -1)
      {
        write(fd_cliente, &msg, sizeof(Mensagem));
        close(fd_cliente);
      }
    }

    close(fd_leitura);
    printf("Cliente desconectado. Servidor reiniciando...\n");
  }

  return 0;
}