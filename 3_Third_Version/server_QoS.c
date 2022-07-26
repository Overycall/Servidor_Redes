/** UNIPAMPA - Universidade Federal do Pampa
  * Curso: Engenharia de Computação
  * Disciplina: Redes de Computadores
  * Professor: Leonardo Bidesse Pinho
  * Alunos:
  *      - Lucas Vilanova Barcellos Matricula: 1902450007
  *      - Renato Sayyed de Souza   Matricula: 1901560699
  *      - Willian Silva Domingues  Matricula: 1901570513
  * Servidor HTTP | 1.0 and 1.1 with QoS
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define SERVERNAME "http server"
#define BUFFERSIZE (1024)
double http_version;
int client_count;
int MAX_CLIENTS;
int *max_bandwidth;

int create_server_socket(struct sockaddr_in *server_addr, int port)
{
    int server_socket;

    // Cria o socket do servidor - create()
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("create()");
        exit(EXIT_FAILURE);
    }

    // Configura o socket do servidor para se manter aberto para mais de uma conexão - setsockopt()
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    if (http_version == 1.1) // Configura o socket do servidor para se manter vivo durante o tempo de espera - setsockopt()
    {
        int keepalive = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
        {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        }  
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

    // Mostra o IP e a porta do servidor - gethostbyname()
    struct hostent *host_info = gethostbyname("localhost");
    if (host_info == NULL)
    {
        perror("gethostbyname()");
        exit(EXIT_FAILURE);
    }
    printf("\nServidor rodando em %s:%d\n", inet_ntoa(*(struct in_addr *)host_info->h_addr), port);
    
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

    // Mostra o IP e a porta do cliente
    struct hostent *host_info = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr), AF_INET);
    if (host_info == NULL)
    {
        perror("gethostbyaddr()");
        exit(EXIT_FAILURE);
    }
    printf("\nCliente conectado em %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    if (http_version == 1.1) // Configura o socket do cliente para se manter vivo durante o tempo de espera - setsockopt()
    {
        int keepalive = 1;
        if (setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
        {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        }  
    }

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

    return request;
}

int sendall(int s, char *buf, long int *len, char *path)
{
    long int total = 0;        // how many bytes we've sent
    long int n;
    long int bytes_left = *len; // how many we have left to send
    int client_bandwidth = max_bandwidth[client_count]; 

    printf("\nEnviando o arquivo %s - SIZE -> %ld\n", path, *len);
    
    while(total < *len) {
        
        //Controlando a quantidade de bytes a serem enviados
        if (bytes_left > client_bandwidth) {
            n = send(s, buf + total, client_bandwidth, 0);
        } else {
            n = send(s, buf + total, bytes_left, 0);
        }

        if (n == -1) break;

        total += n;
        bytes_left -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

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

void handle_request(char *request, int client_socket)
{
    // Informar que o servidor está tratando a requisição - gethostbyname()
    struct hostent *host_info = gethostbyname("localhost");
    if (host_info == NULL)
    {
        perror("gethostbyname()");
        exit(EXIT_FAILURE);
    }
    printf("\nServidor tratando requisição de %s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr));

    // Separa o arquivo solicitado do pedido do cliente - strtok()
    char *token = strtok(request, "/"); //Pega o primeiro token do buffer
    char *method = token; //Define o método como o primeiro token
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

        // Envia o conteúdo do pedido - sendall()
        if (sendall(client_socket, buffer, &length, path) != 0)
        {
            perror("sendall");
            printf("\nArquivo não pode ser enviado por completo\n");
        }
        else
        {
            printf("\nArquivo enviado com sucesso\n");
        }	

        free(buffer); //Libera o buffer de conteúdo
    }
}

void close_client_socket(int client_socket)
{
    // Fecha o socket do cliente - close()
    if (close(client_socket) < 0)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }

    // Informa que o cliente foi desconectado - gethostbyname()
    struct hostent *host_info = gethostbyname("localhost");
    if (host_info == NULL)
    {
        perror("gethostbyname()");
        exit(EXIT_FAILURE);
    }
    printf("\nCliente desconectado de %s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr));
    printf("\nClientes conectados: %d\n", --client_count);

}

void http_execution(int client_socket)
{
    // HTTP/1.0
    if (http_version == 1.0)
    {
        char *request = receive_request(client_socket);
        handle_request(request, client_socket);
        free(request);

        close_client_socket(client_socket);
    }
    // HTTP/1.1
    else if (http_version == 1.1)
    {
        char *request = receive_request(client_socket);
        while (strcmp(request, "") != 0)
        {
            handle_request(request, client_socket);
            free(request);
            request = receive_request(client_socket);
        }

        close_client_socket(client_socket);
    }
}

void *thread_execution(void *arg)
{
    int client_socket = *(int *)arg;

    pthread_detach(pthread_self());

    http_execution(client_socket);
}

void in_thread(int server_socket)
{
    client_count = 0;

    pthread_t threads[MAX_CLIENTS];

    while(1)
    {
        while(client_count == MAX_CLIENTS)
        {
            sleep(1);
        }

        int client_socket = accept_client_connection(server_socket);
        client_count++;
        printf("\nClientes conectados: %d\n", client_count);

        if (client_count == 0)
            pthread_create(&threads[client_count], NULL, thread_execution, &client_socket);
        else
            pthread_create(&threads[client_count - 1], NULL, thread_execution, &client_socket);
    }
}

int valida_argv(char *argv[]){
    if(argv[1] == NULL){
        printf("A versão para o servidor HTTP não foi informada.\n");
        return 0;
    } else if(argv[2] == NULL){
        printf("A porta utilizada para conexão com o servidor não foi informada.\n");        
        return 0;
    } else if(argv[3] == NULL){
        printf("A quantidade máxima de clientes possíveis conectados ao servidor não foi informada\n");
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if(!valida_argv(argv)) return 0;

    // Define a versão do protocolo HTTP - 1.0 ou 1.1 vindo do argumento do programa
    http_version = atoi(argv[1]);
    if (http_version != 1.0 && http_version != 1.1)
    {
        printf("\nVersão do protocolo HTTP inválida!\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);

    // Define a quantidade de clientes que podem se conectar ao servidor - MAX_CLIENTS
    MAX_CLIENTS = atoi(argv[3]);

    // Criando o vetor para saber o bandwidth dos usuários - bandwidth
    max_bandwidth = malloc(MAX_CLIENTS * sizeof(int));
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        max_bandwidth[i] = 1000000;
    }

    struct sockaddr_in server_addr;
    int server_socket = create_server_socket(&server_addr, port);

    in_thread(server_socket);

    close(server_socket);

    return 0;
}