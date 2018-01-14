// Client for simple chat application
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAX_LEN 1000

int main(int argc, char* argv[])
{
    int client_socket;
    struct sockaddr_in server;
    char message[MAX_LEN];
    int PORT = atoi(argv[2]);

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }
    server.sin_family = AF_INET;  // Used for TCP
    server.sin_port = htons(PORT);  // host to network byte ordering for consistency
    server.sin_addr.s_addr = inet_addr(argv[1]);  // server IP to be given
    bzero(&server.sin_zero, 8);  // fill empty fields with 0

    if ((connect(client_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
    {
        printf("Client Connection Error\n");
        exit(1);
    }

    while(1)  // Keep connection open endlessly
    {
        fgets(message, MAX_LEN, stdin);  // Standard input (this allows strings with spaces also)
        if (strcmp(message, "Bye\n") == 0)  // This is the breaking condition
        {
            break;
        }
        int sent = send(client_socket, message, strlen(message), 0);
        printf("Sent: %s\n", message);
        int received = recv(client_socket, message, MAX_LEN, 0);
        message[strlen(message)] = '\0';
        printf("Received: %s\n", message);
    }
    close(client_socket);
return 0;
}
