// Client for Echo system
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
    const char *exit_str = "Exit";
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
        message[strlen(message) - 1] = '\0';
        if (strcmp(message, exit_str) == 0)  // This is the breaking condition
        {
            break;
        }
        int sent = send(client_socket, message, strlen(message), 0);
        printf("Sent \"%s\"", message);
        int received = recv(client_socket, message, MAX_LEN, 0);
        message[received] = '\0';
        printf("Received \"%s\"", message);
    }
    close(client_socket);
return 0;
}
