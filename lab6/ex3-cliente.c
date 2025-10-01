#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_SERVIDOR "servidor_fifo"

typedef struct
{
  pid_t pid_cliente;
  char texto[256];
} Mensagem;

int main()
{
  int fd_servidor_escrita, fd_cliente_leitura;
  Mensagem msg;
  char fifo_cliente_nome[50];

  msg.pid_cliente = getpid();
  sprintf(fifo_cliente_nome, "cliente_%d_fifo", msg.pid_cliente);

  unlink(fifo_cliente_nome);
  if (mkfifo(fifo_cliente_nome, S_IRUSR | S_IWUSR) == -1)
  {
    perror("mkfifo cliente");
    return 1;
  }

  fd_servidor_escrita = open(FIFO_SERVIDOR, O_WRONLY);
  if (fd_servidor_escrita == -1)
  {
    perror("Servidor nao esta em execucao");
    unlink(fifo_cliente_nome);
    return 1;
  }

  printf("Cliente conectado. Digite uma mensagem ou pressione Ctrl+D para encerrar.\n");

  while (1)
  {
    printf("> ");
    if (fgets(msg.texto, sizeof(msg.texto), stdin) == NULL)
    {
      printf("\nDetectado Ctrl+D. ");
      break;
    }
    msg.texto[strcspn(msg.texto, "\n")] = 0;

    write(fd_servidor_escrita, &msg, sizeof(Mensagem));

    fd_cliente_leitura = open(fifo_cliente_nome, O_RDONLY);
    if (fd_cliente_leitura == -1)
    {
      perror("open cliente fifo leitura");
      break;
    }

    if (read(fd_cliente_leitura, &msg, sizeof(Mensagem)) > 0)
    {
      printf("Servidor: %s\n", msg.texto);
    }
    else
    {
      printf("Erro ao ler resposta do servidor.\n");
      close(fd_cliente_leitura);
      break;
    }

    close(fd_cliente_leitura);
  }

  printf("Encerrando cliente.\n");
  close(fd_servidor_escrita);
  unlink(fifo_cliente_nome);

  return 0;
}