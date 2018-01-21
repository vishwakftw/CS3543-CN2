// Server for Simple Chat Application with two clients and Blocking capability
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAX_QUEUE 2
#define MAX_LEN 1000

int main(int argc, char* argv[])
{
    int server_socket, client_sockets[2];
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

    // Server will send the clients the send-first and receive-first privileges
    while((client_sockets[0] = accept(server_socket, (struct sockaddr *)&client[0], &len)) == -1);
    printf("Client 0 Connected. IP Address: %s\n", inet_ntoa(client[0].sin_addr));
    strcpy(message, "send_first");
    send(client_sockets[0], message, strlen(message), 0);
    while((client_sockets[1] = accept(server_socket, (struct sockaddr *)&client[1], &len)) == -1);
    printf("Client 1 Connected. IP Address: %s\n", inet_ntoa(client[1].sin_addr));
    strcpy(message, "recv_first");
    send(client_sockets[1], message, strlen(message), 0);

    int received, sent;
    while(1)  // Receive endlessly
    {
        received = recv(client_sockets[0], message, MAX_LEN, 0);
        if (received <= 0)  // Loop breaker, error checking
        {
            break;
        }
        sent = send(client_sockets[1], message, received, 0);
        received = recv(client_sockets[1], message, MAX_LEN, 0);
        if (received <= 0)  // Loop breaker, error checking
        {
            break;
        }
        sent = send(client_sockets[0], message, received, 0);
    }
    close(client_sockets[0]);
    close(client_sockets[1]);
    close(server_socket);
return 0;
}
