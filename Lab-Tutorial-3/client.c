#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<stdbool.h>

#define MAX_LEN 1000

typedef struct
{
    char data[MAX_LEN];
    char checksum;
    int seq_no;
    bool ack;
    bool last_packet;
}packet;

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in server, client;
    int server_PORT = atoi(argv[1]);
    int client_PORT = atoi(argv[2]);

    char message_send[MAX_LEN];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }
    client.sin_family = AF_INET;  // used for UDP
    client.sin_port = htons(client_PORT);  // host to network byte ordering to maintain consistency
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&client.sin_zero, 8);  // Fill empty fields with 0

    // Bind socket to PORT
    socklen_t len = sizeof(struct sockaddr_in);
    if (bind(client_socket, (struct sockaddr *)&client, len) == -1)
    {
        printf("Socket Binding Error\n");
        exit(1);
    }

    // Server details established
    server.sin_family = AF_INET;
    server.sin_port = htons(server_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Now we need to send messages to server
    int received, sent;
    packet cur_packet_send, cur_packet_recv;
    while(1)
    {
        received = recvfrom(client_socket, &cur_packet_recv, sizeof(cur_packet_recv), 0, (struct sockaddr *)&server, sizeof(server));
        if (strlen(cur_packet_recv.data) != cur_packet_recv.checksum)
        {
            break;
        }
        printf("%s", cur_packet_recv.data);
        if (cur_packet_recv.last_packet == true)
        {
            break;
        }
    }
    close(client_socket);
return 0;
}
