/** UNIPAMPA - Universidade Federal do Pampa
  * Curso: Engenharia de Computação
  * Disciplina: Redes de Computadores
  * Professor: Leonardo Bidesse Pinho
  * Alunos:
  *      - Lucas Vilanova Barcellos Matricula: 1902450007
  *      - Renato Sayyed de Souza   Matricula: 1901560699
  *      - Willian Silva Domingues  Matricula: 1901570513
  * Servidor HTTP Versão 1.0
**/

/*************************
 * BIBLIOTECAS - FUNCTIONS
 * # sys/socket.h - socket(int domain, int type, int protocol)
 * - domain: Domínio do protocolo (IPv4 ou IPv6)
 * - type: Tipo de socket.
 * - protocol: Protocolo utilizado.
 * # sys/socket.h - bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)
 * - sockfd: Descritor do socket.
 * - my_addr: Endereço do socket.
 * - addrlen: Tamanho do endereço do socket.
 * # sys/socket.h - listen(int sockfd, int backlog)
 * - sockfd: Descritor do socket.
 * - backlog: Número máximo de conexões pendentes.
 * # sys/socket.h - accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
 * - sockfd: Descritor do socket.
 * - addr: Endereço do socket.
 * - addrlen: Tamanho do endereço do socket.
 * # sys/socket.h - recv(int sockfd, void *buf, size_t len, int flags)
 * - sockfd: Descritor do socket.
 * - buf: Buffer de recebimento.
 * - len: Tamanho do buffer de recebimento.
 * - flags: Flags de recebimento. (-1 para erro, 0 para sucesso).
 * # sys/socket.h - send(int sockfd, const void *buf, size_t len, int flags)
 * - sockfd: Descritor do socket.
 * - buf: Buffer de envio.
 * - len: Tamanho do buffer de envio.
 * - flags: Flags de envio. (-1 para erro, 0 para sucesso).
 * # sys/socket.h - close(int sockfd)
 * - sockfd: Descritor do socket.
************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVERNAME "HTTP Server 1.0"
#define SERVERPORT (8100)
#define BUFFERSIZE (1024)
#define MAX_CLIENTS (10)

int main()
{
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    long addrlen = sizeof(server_addr);

    // Cria o socket do servidor - create()
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create()");
        exit(EXIT_FAILURE);
    }

    // Configuração do socket
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero)); //Zera o restante do struct
    server_addr.sin_family = AF_INET; //Define o protocolo de comunicação como IPv4
    server_addr.sin_port = htons(SERVERPORT); //Define a porta do servidor
    server_addr.sin_addr.s_addr = INADDR_ANY; //Define o IP do servidor

    // Associa o socket ao endereço do servidor - bind()
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    // Configura o socket para ouvir conexões - listen()
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen()");
        exit(EXIT_FAILURE);
    }

    //Informando por qual porta o servidor está escutando - printf()
    printf("%s listening on port [%d]\n", SERVERNAME, SERVERPORT);
    
    // Tratamento da conexão com o cliente
    while(1)
    {
        // Aceita a conexão com o cliente - accept()
        if ((new_socket = accept(server_socket, (struct sockaddr*)&server_addr, (socklen_t*)&addrlen)) < 0)
        {
            perror("accept()");
            exit(EXIT_FAILURE);
        }

        // Informando o cliente conectado - printf()
        printf("Cliente-IP conectado: %d\n", inet_ntoa(server_addr.sin_addr));

        // Recebe o pedido do cliente - recv()
        char buffer[BUFFERSIZE]; //Buffer de recebimento
        memset(buffer, '\0', BUFFERSIZE); //Zera o buffer
        if (recv(new_socket, buffer, BUFFERSIZE, 0) < 0) 
        {
            perror("recv()");
            exit(EXIT_FAILURE);
        }

        // Imprime o pedido do cliente - printf()
        printf("Request - %s\n", buffer);

        // Trata o buffer de recebimento - strtok()
        char *token = strtok(buffer, "/"); //Pega o primeiro token do buffer
        char *method = token; //Define o método como o primeiro token
        token = strtok(NULL, " "); //Pega o segundo token do buffer
        char *path = token; //Define o caminho como o segundo token
        //printf("Method - %s\n", method);
        //printf("Path - %s\n", path);

        // Tratamento do pedido do cliente
        if(strstr(path, ".html") || strstr(path, ".htm"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".gif"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: image/gif\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".jpg") || strstr(path, ".jpeg"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".png"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: image/png\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".au"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: audio/basic\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".wav"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: audio/wav\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".avi"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: video/x-msvideo\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".mp3"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: audio/mpeg\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        else if(strstr(path, ".ico"))
        {
            // Message-Body
            FILE *file;
            char *buffer;
            long length;
            if ((file = fopen(path, "rb")) == NULL)
            {
                send(new_socket, "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>", 65, 0);
                close(new_socket);
                continue;
            }
            fseek(file, 0, SEEK_END); //Vai para o final do arquivo
            length = ftell(file); //Pega o tamanho do arquivo
            fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
            buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
            fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
            fclose(file); //Fecha o arquivo

            // Envia o cabeçalho do pedido - send()
            char *header = "HTTP/1.0 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %ld\r\n\r\n";
            char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
            sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo

            send(new_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho do pedido
            free(header_buffer); //Libera o buffer de cabeçalho

            // Envia o conteúdo do pedido - send()
            send(new_socket, buffer, length, 0); //Envia o conteúdo do pedido
            free(buffer); //Libera o buffer de conteúdo
        }
        
        if (close(new_socket) < 0)
        {
            perror("close()");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}