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
int block_client_count;
int block_file;

// Função para criar o socket do servidor.
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

    // Configura o socket para deixar outros socketes conectarem na mesma porta - setsockopt()
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt()");
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

// Função para definir a banda de transferência do cliente.
int client_bandwidth(char *ip)
{
    FILE *file;
    while (block_file == 1)
    {
        sleep(1);
    }
    
    block_file = 1;
    if((file = fopen("clients_bandwidth.txt", "r")) == NULL)
    {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    // Cada linha tem o formato: IP|BANDWIDTH
    char line[BUFFERSIZE];
    char *client_ip;
    int bandwidth;
    
    while(fgets(line, BUFFERSIZE, file) != NULL)
    {
        client_ip = strtok(line, "|");
        bandwidth = atol(strtok(NULL, "|"));
        if(strcmp(client_ip, ip) == 0)
        {
            fclose(file);
            block_file = 0;
            return bandwidth;
        }
    }
    fclose(file);
    block_file = 0;
    
    // Se não encontrar o IP na lista de conhecidos, limita para 1000 kbps
    return 1000000;
}

// Função para criar o socket do cliente.
char *accept_client_connection(int server_socket)
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

    if (http_version == 1.1) // Configura o socket do cliente para aceitar pacotes keep-alive - setsockopt()
    {
        int keepalive = 1;
        if (setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) < 0)
        {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        }  
    }

    // Busca a largura de banda do cliente - client_bandwidth()
    int bandwidth = client_bandwidth(inet_ntoa(client_addr.sin_addr));
    char *client_socket_str = (char *)malloc(sizeof(int));
    sprintf(client_socket_str, "%d|", client_socket);
    char *bandwidth_str = (char *)malloc(sizeof(long int));
    sprintf(bandwidth_str, "%d|", bandwidth);
    char *client_socket_bandwidth = (char *)malloc(strlen(client_socket_str) + strlen(bandwidth_str) + 1);
    strcpy(client_socket_bandwidth, client_socket_str);
    strcat(client_socket_bandwidth, bandwidth_str);

    return client_socket_bandwidth;
}

// Função para receber o pedido do cliente.
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

// Função para enviar o arquivo requisitado pelo cliente em um taxa controlada.
int sendall(int s, char *buf, long int *len, char *path, long int client_bandwidth)
{
    long int total = 0;        // Quantidade de bytes enviados
    long int n;
    long int bytes_left = *len; // Quantidade de bytes a serem enviados
    int tries = 0;

    printf("\nEnviando o arquivo %s - SIZE -> %ld\n", path, *len);
    
    while(total < *len) {
        
        sleep(1);
        //Controlando a quantidade de bytes a serem enviados
        if (bytes_left > client_bandwidth) {
            n = send(s, buf + total, client_bandwidth, 0);
        } else {
            n = send(s, buf + total, bytes_left, 0);
        }
        printf("\nEnviando %ld bytes\n", n);

        if (n < 0 && tries < 5)
        {
            printf("\nErro ao enviar bytes, realizando o reenvio\n");
            tries++;
        }else if (n < 0 && tries >= 5){
            break;
        }else{
            total += n;
            bytes_left -= n;
            tries = 0;
            printf("Faltam %ld bytes\n", bytes_left);
        }
    }

    *len = total; // Retorna a quantidade de bytes enviados

    return n<0?-1:0; // Retorna 0 se não houve erro e -1 se houve erro
} 

// Função para definir qual o tipo de arquivo será enviado.
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

// Função para tratar o pedido do cliente.
void handle_request(char *request, int client_socket, long int client_bandwidth)
{
    // Separa o arquivo solicitado do pedido do cliente - strtok()
    char *token = strtok(request, "/"); //Pega o primeiro token do buffer
    char *method = token; //Define o método como o primeiro token
    token = strtok(NULL, " "); //Pega o segundo token do buffer
    char *path = token; //Define o caminho como o segundo token

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
        if (sendall(client_socket, buffer, &length, path, client_bandwidth) != 0)
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

// Função para fechar o socket do cliente.
void close_client_socket(int client_socket)
{
    // Fecha o socket do cliente - close()
    if (close(client_socket) < 0)
    {
        perror("close()");
        exit(EXIT_FAILURE);
    }

    while(block_client_count == 1)
    {
        sleep(1);
    }
    block_client_count = 1;
    printf("\nClientes conectados: %d\n", --client_count);
    block_client_count = 0;
}

// Função que define como a comunicação entre o cliente e o servidor será feita.
void http_execution(int client_socket, long int client_bandwidth)
{
    // HTTP/1.0
    if (http_version == 1.0)
    {
        char *request = receive_request(client_socket);
        handle_request(request, client_socket, client_bandwidth);
        free(request);

        close_client_socket(client_socket);
    }
    // HTTP/1.1
    else if (http_version == 1.1)
    {
        // Controlar ociosidade do cliente até 10 s - Se não receber nada, fechar o socket
        time_t start_time = time(NULL);

        while (time(NULL) - start_time < 10)
        {
            char *request = receive_request(client_socket);
            
            if (strstr(request, "GET") != NULL)
            {
                handle_request(request, client_socket, client_bandwidth);
                free(request);
                start_time = time(NULL);
            }
        }

        close_client_socket(client_socket);
    }
}

// Função que cada thread executa.
void *thread_execution(void *arg)
{
    char *client_str = (char *)arg;

    char *token = strtok(client_str, "|"); //Pega o primeiro token do buffer
    int client_socket = atoi(token); //Define o socket como o primeiro token
    token = strtok(NULL, "|"); //Pega o segundo token do buffer
    int client_bandwidth = atoi(token); //Define o bandwidth como o segundo token

    pthread_detach(pthread_self());

    http_execution(client_socket, client_bandwidth);
}

// Função para criar as threads e executar os pedidos do cliente.
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

        // Cria um socket do cliente - accept_client_connection()
        char *client_socket_str = accept_client_connection(server_socket);

        while(block_client_count == 1)
        {
            sleep(1);
        }
        block_client_count = 1;
        client_count ++;
        block_client_count = 0;

        printf("\nClientes conectados: %d\n", client_count);

        // Cria uma thread para o cliente - pthread_create()
        pthread_create(&threads[client_count - 1], NULL, thread_execution, client_socket_str);
    }
}

// Função para validar se foram passados os argumentos corretos.
int valida_argv(char *argv[]){
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL){
        printf("\nArgumentos inválidos\n");
        printf("\n./http_server <http_version> <port> <max_clients>\n");
        return 0;
    }
    return 1;
}

// Função principal do servidor.
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

    struct sockaddr_in server_addr;
    int server_socket = create_server_socket(&server_addr, port);

    in_thread(server_socket);

    close(server_socket);

    return 0;
}