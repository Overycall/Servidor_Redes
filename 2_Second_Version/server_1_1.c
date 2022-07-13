/** UNIPAMPA - Universidade Federal do Pampa
  * Curso: Engenharia de Computação
  * Disciplina: Redes de Computadores
  * Professor: Leonardo Bidesse Pinho
  * Alunos:
  *      - Lucas Vilanova Barcellos Matricula: 1902450007
  *      - Renato Sayyed de Souza   Matricula: 1901560699
  *      - Willian Silva Domingues  Matricula: 1901570513
  * Servidor HTTP Versão 1.1
**/

/*********
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
********/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define SERVERNAME "HTTP Server 1.1"
#define BUFFERSIZE (1024)
#define MAX_CLIENTS (10)

int create_server_socket(struct sockaddr_in *server_addr, int port)
{
    int server_socket;

    // Cria o socket do servidor - create()
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create()");
        exit(EXIT_FAILURE);
    }

    // Configuração do socket
    memset(server_addr->sin_zero, '\0', sizeof(server_addr->sin_zero)); //Zera o restante do struct
    server_addr->sin_family = AF_INET; //Define o protocolo de comunicação como IPv4
    server_addr->sin_port = htons(port); //Define a porta do servidor
    server_addr->sin_addr.s_addr = INADDR_ANY; //Define o IP do servidor

    // Associa o socket ao endereço do servidor - bind()
    if (bind(server_socket, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) < 0)
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
    printf("SERVER [%s] listening on port [%d]\n", SERVERNAME, port);

    return server_socket;
}

int accept_client_connection(int server_socket)
{
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    // Aceita a conexão do cliente - accept()
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
    {
        perror("accept()");
        exit(EXIT_FAILURE);
    }

    // Informando o IP do cliente - printf()
    printf("SERVER [%s] accepted connection from [%d]\n", SERVERNAME, inet_ntoa(client_addr.sin_addr));

    return client_socket;
}

char *receive_request(int client_socket)
{
    char *request = (char *)malloc(BUFFERSIZE * sizeof(char));

    // Recebe o pedido do cliente - recv()
    if ((recv(client_socket, request, BUFFERSIZE, 0)) < 0)
    {
        perror("recv()");
        exit(EXIT_FAILURE);
    }

    // Informando o pedido do cliente - printf()
    //printf("SERVER [%s] received request [%s]\n", SERVERNAME, request);

    return request;
}

//Definir o cabecalho para o pedido do cliente
char *define_header(char *path)
{
    char *header;

    if(strstr(path, ".html") || strstr(path, ".htm"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".jpg") || strstr(path, ".jpeg"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".png"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".gif"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".css"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".js"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".ico"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".mp3"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".mp4"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\nContent-Length: %ld\r\n\r\n";
    }
    else if(strstr(path, ".pdf"))
    {
        header = "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nContent-Length: %ld\r\n\r\n";
    }

    return header;
}

//Tratando o pedido do cliente
void handle_request(char *request, int client_socket)
{
    printf("SERVER [%s] handling request [%s]\n", SERVERNAME, request);
    // Separa o arquivo solicitado do pedido do cliente - strtok()
    char *token = strtok(request, "/"); //Pega o primeiro token do buffer
    char *method = token; //Define o método como o primeiro toke
    token = strtok(NULL, " "); //Pega o segundo token do buffer
    char *path = token; //Define o caminho como o segundo token
    printf("Method - %s\n", method);
    printf("Path - %s\n", path);

    FILE *file;

    if((file = fopen(path, "rb")) == NULL)
    {
        send(client_socket, "HTTP/1.1 404 Not Found\r\n", strlen("HTTP/1.1 404 Not Found\r\n"), 0);
    }
    else
    {
        char *buffer;
        long length;
        
        fseek(file, 0, SEEK_END); //Vai para o final do arquivo
        length = ftell(file); //Pega o tamanho do arquivo
        fseek(file, 0, SEEK_SET); //Volta para o início do arquivo
        buffer = malloc(length); //Aloca o buffer de tamanho igual ao tamanho do arquivo
        fread(buffer, 1, length, file); //Lê o arquivo e armazena no buffer
        fclose(file); //Fecha o arquivo
        
         // Envia o cabeçalho do pedido - send()
        char *header = define_header(path);
        char *header_buffer = malloc(strlen(header) + strlen(buffer)); //Aloca o buffer de cabeçalho
        sprintf(header_buffer, header, length); //Formata o cabeçalho com o tamanho do arquivo
        send(client_socket, header_buffer, strlen(header_buffer), 0); //Envia o cabeçalho
        free(header_buffer); //Libera o buffer de cabeçalho

        // Envia o conteúdo do arquivo - send()
        send(client_socket, buffer, length, 0); //Envia o conteúdo do arquivo
        free(buffer); //Libera o buffer de conteúdo
    }
}

//Fecha o socket do cliente
void close_client_socket(int client_socket)
{
    // Fecha o socket do cliente - close()
    if (close(client_socket) < 0)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }

    // Informando o fim da conexão // cliente ip - printf()
    printf("SERVER [%s] closed connection with client\n", SERVERNAME);
}

void execution_1_0(int client_socket)
{
    char *request = receive_request(client_socket);

    printf("SERVER [%s] received request [%s]\n", SERVERNAME, request);

    handle_request(request, client_socket);
    close_client_socket(client_socket);
}

void execution_1_1(int client_socket)
{
    char *request;
    long int time_init = time(NULL);

    do
    {
        request = receive_request(client_socket);
        
        printf("SERVER [%s] received request [%s]\n", SERVERNAME, request);

        if(strstr(request, "GET"))
        {
            handle_request(request, client_socket);
            time_init = time(NULL);
        }
        else
        {
            send(client_socket, "HTTP/1.1 400 Bad Request\r\n", strlen("HTTP/1.1 400 Bad Request\r\n"), 0);
        }
    } while ((time(NULL) - time_init) < 10);

    close_client_socket(client_socket);
}

static void *thread_execution(void *arg)
{
    int client_socket = (int)arg;

    pthread_detach(pthread_self());
    
    printf("SERVER [%s] started one thread\n", SERVERNAME);

    execution_1_1(client_socket);

    printf("SERVER [%s] finished one thread\n", SERVERNAME);
}

// in_thread - função que será executada em uma thread
void in_thread(int server_socket)
{
    pthread_t tid;
    
    // O loop abaixo é um loop infinito que aceita conexões de clientes
    // e cria uma thread para tratar o pedido do cliente

    while(1)
    {
        int client_socket = accept_client_connection(server_socket);
        pthread_create(&tid, NULL, (void *)thread_execution, (void *)client_socket);
    }
}

int main(int argc, char *agrv[])
{
    int porta = atoi(agrv[1]);
    struct sockaddr_in server_addr;
    int server_socket = create_server_socket(&server_addr, porta); //Descritor do socket do servidor

    in_thread(server_socket); //Executa a função in_thread()
    close(server_socket); //Fecha o socket do servidor
    return 0;
}