// Client for simple chat application with non-blocking capabilities 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define MAX_LEN 1000

// This is the threading function for the thread spawned by the client process
// Only for receiving
void *receiver(void *rec_client)
{
    char message_recv[MAX_LEN];
    int *rec_fd = (int *)rec_client;
    int received;
    while(1)
    {
        received = recv(*rec_fd, message_recv, MAX_LEN, 0);
        message_recv[received - 1] = '\0';
        printf("Received: %s\n", message_recv);
        if (strcmp(message_recv, "Bye") == 0)
        {
            return ;
        }
    }
}

int main(int argc, char* argv[])
{
    int client_socket;
    struct sockaddr_in server;
    char message_send[MAX_LEN];
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

    int sent;
    pthread_t receiver_thread; // Create the thread
    pthread_create(&receiver_thread, NULL, receiver, &client_socket);
    while(1)  // Keep connection open endlessly
    {
        fgets(message_send, MAX_LEN, stdin);  // Standard input (this allows strings with spaces also)
        sent = send(client_socket, message_send, strlen(message_send), 0);
        printf("Sent: %s", message_send);
        if (strcmp(message_send, "Bye\n") == 0)  // This is the breaking condition
        {
            break;
        }
    }
    pthread_join(receiver_thread, NULL);  // Wait for completion of the parent process
    close(client_socket);
return 0;
}
