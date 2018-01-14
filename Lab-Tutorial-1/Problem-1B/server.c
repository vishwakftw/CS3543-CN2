// Server for simple Chat Application
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAX_QUEUE 10
#define MAX_LEN 1000

int main(int argc, char* argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server, client;
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

    // Listen endlessly until one client connects and wait for connection
    if((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
    {
        printf("Socket Accept Error\n");
        exit(1);
    }

    printf("Client Connected. IP Address: %s\n", inet_ntoa(client.sin_addr));

    int received, sent;
    while(1)  // Receive endlessly
    {
        received = recv(client_socket, message, MAX_LEN, 0);
        message[received] = '\0';
        printf("Received: %s\n", message);
        // If the server receives Bye, then break. If the client receives Bye, then client disconnects. This will cause received <= 0
        if (strcmp(message, "Bye\n") == 0 | received <= 0)
        {
            break;
        }
        fgets(message, MAX_LEN, stdin);
        sent = send(client_socket, message, strlen(message), 0);
        printf("Sent: %s\n", message);
    }
    printf("Client at IP Address: %s has disconnected\n", inet_ntoa(client.sin_addr));
    close(client_socket);
    close(server_socket);
return 0;
}
