#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<stdbool.h>

#define MAX_LEN 200

char compute_checksum(char cur_string[], int size)
{
    int i;
    char checker = cur_string[0];
    for (i = 1; i < size; i++)
    {
        checker = checker ^ cur_string[i];
    }
return checker;
}

typedef struct
{
    char data[MAX_LEN];
    char checksum;
    int seq_no;
    bool ack;
    bool last_packet;
    int data_size;
}packet;

int main(int argc, char *argv[])
{
    int server_socket;
    struct sockaddr_in server, client;
    int server_PORT = atoi(argv[1]);
    int client_PORT = atoi(argv[2]);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = atoi(argv[4]);
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        printf("Socket timeout setting error\n");
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


    // Sending details
    FILE *file_ptr = fopen(argv[3], "rb");
    fseek(file_ptr, 0, SEEK_END);
    int total_size = ftell(file_ptr);
    rewind(file_ptr);

    // Create placeholders
    int received, sent;
    int counter = 0;
    int read_bytes = 0, total_sent = 0;

    // Sending loop
    while(1)
    {
        // Make packet
        char message_send[MAX_LEN];
        packet cur_packet_send, cur_packet_recv;
        read_bytes = fread(message_send, sizeof(char), MAX_LEN, file_ptr);
        strcpy(cur_packet_send.data, message_send);
        cur_packet_send.seq_no = counter;
        cur_packet_send.ack = false;
        cur_packet_send.last_packet = ((total_size - total_sent) <= MAX_LEN);
        cur_packet_send.checksum = compute_checksum(cur_packet_send.data, read_bytes);
        cur_packet_send.data_size = read_bytes;
    
        // Keep sending until you get the correct ack with the correct expected sequence number
        do
        {
            sent = sendto(server_socket, &cur_packet_send, sizeof(cur_packet_send), 0, (struct sockaddr *)&client, sizeof(client));    
            printf("Sending message of length: %d\n", read_bytes);
            received = recvfrom(server_socket, &cur_packet_recv, sizeof(cur_packet_recv), 0, (struct sockaddr *)&client, sizeof(client));
        } while (cur_packet_recv.seq_no != cur_packet_send.seq_no + 1);

        printf("Sent packet with sequence number = %d\n", cur_packet_send.seq_no);
        if (cur_packet_send.last_packet == true)
        {
            break;
        }
        counter += 1;
        total_sent += read_bytes;
    }
    close(server_socket);
return 0;
}
