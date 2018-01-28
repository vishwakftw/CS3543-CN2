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
    int server_socket;
    struct sockaddr_in server, client;
    int server_PORT = atoi(argv[1]);
    int client_PORT = atoi(argv[2]);

    char message_send[MAX_LEN];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }
    server.sin_family = AF_INET;  // used for UDP
    server.sin_port = htons(server_PORT);  // host to network byte ordering to maintain consistency
    server.sin_addr.s_addr = inet_addr("127.0.0.1");  // no specific IP
    bzero(&server.sin_zero, 8);  // Fill empty fields with 0

    printf("Server information: IP Address: %s, Port Number: %d\n", inet_ntoa(server.sin_addr), server_PORT);

    // Bind socket to PORT
    socklen_t len = sizeof(struct sockaddr_in);
    if (bind(server_socket, (struct sockaddr *)&server, len) == -1)
    {
        printf("Socket Binding Error\n");
        exit(1);
    }

    // Client details established
    client.sin_family = AF_INET;
    client.sin_port = htons(client_PORT);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");  // testing from local, hence this will be the same

    // Now we need to send messages to client
    int received, sent;
    packet cur_packet_send, cur_packet_recv;
    int counter = 0;
    while(1)
    {
        fgets(message_send, MAX_LEN, stdin);
        strcpy(cur_packet_send.data, message_send);
        cur_packet_send.seq_no = counter;
        cur_packet_send.ack = false;
        cur_packet_send.last_packet = false;
        cur_packet_send.checksum = strlen(message_send);
        if (fgets(message_send, MAX_LEN, stdin) == 0)
        {
            cur_packet_send.last_packet = true;
        }

        sent = sendto(server_socket, &cur_packet_send, sizeof(cur_packet_send), 0, (struct sockaddr *)&client, sizeof(client));
        printf("Sent packet with seq.number = %d\n", cur_packet_send.seq_no);
        counter += 1;

        if (cur_packet_send.last_packet == true)
        {
            break;
        }
    }
    close(server_socket);
return 0;
}
