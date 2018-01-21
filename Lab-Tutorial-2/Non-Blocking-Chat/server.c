// Server for Echo System
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define MAX_QUEUE 2
#define MAX_LEN 1000

int client_sockets[2];

void *c2_to_c1()
{
    char message[MAX_LEN];
    int received, sent;
    while(1)
    {
        received = recv(client_sockets[1], message, MAX_LEN, 0);
        if (received <= 0)
        {
            return ;
        }        
        sent = send(client_sockets[0], message, received, 0);
    }
}

int main(int argc, char* argv[])
{
    int server_socket;
    struct sockaddr_in server, client[2];
    char message[MAX_LEN];
    int PORT = atoi(argv[1]);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }
    server.sin_family = AF_INET;  // used for TCP
    server.sin_port = htons(PORT);  // host to network byte ordering to maintain consistency
    server.sin_addr.s_addr = INADDR_ANY;  // no specific IP
    bzero(&server.sin_zero, 8);  // Fill empty fields with 0

    printf("Server information: IP Address: %s, Port Number: %d\n", inet_ntoa(server.sin_addr), PORT);

    // Bind socket to PORT
    socklen_t len = sizeof(struct sockaddr_in);
    if (bind(server_socket, (struct sockaddr *)&server, len) == -1)
    {
        printf("Socket Binding Error\n");
        exit(1);
    }

    // Now socket will listen
    if (listen(server_socket, MAX_QUEUE) == -1)
    {
        printf("Socket Listen Error\n");
        exit(1);
    }

    while((client_sockets[0] = accept(server_socket, (struct sockaddr *)&client[0], &len)) == -1);
    printf("Client 0 Connected. IP Address: %s\n", inet_ntoa(client[0].sin_addr));
    while((client_sockets[1] = accept(server_socket, (struct sockaddr *)&client[1], &len)) == -1);
    printf("Client 1 Connected. IP Address: %s\n", inet_ntoa(client[1].sin_addr));

    int received, sent;
    pthread_t c2_to_c1_thread;
    pthread_create(&c2_to_c1_thread, NULL, c2_to_c1, NULL);
    while(1)  // Receive endlessly
    {
        received = recv(client_sockets[0], message, MAX_LEN, 0);
        if (received <= 0)
        {
            break;
        }
        sent = send(client_sockets[1], message, received, 0);
    }
    pthread_join(c2_to_c1_thread, NULL);
    close(client_sockets[0]);
    close(client_sockets[1]);
    close(server_socket);
return 0;
}
