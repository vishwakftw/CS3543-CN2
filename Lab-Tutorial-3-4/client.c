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

#define MAX_LEN 200

char compute_checksum(char cur_string[])
{
    int i;
    char checker = 'a';
    for (i = 1; i < strlen(cur_string); i++)
    {
        if (i == 0)
        {
            checker = checker ^ cur_string[i];
        }
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

    // Now we need to receive the packets from the server
    int received, sent;
    bool flag;
    int counter = 0;
    FILE *file_ptr = fopen(argv[3], "wb");
    while(1)
    {
        packet cur_packet_send, cur_packet_recv;
        // receive now
        received = recvfrom(client_socket, &cur_packet_recv, sizeof(cur_packet_recv), 0, (struct sockaddr *)&server, sizeof(server));
        cur_packet_send.ack = true;  // Sending an ack
        if ((cur_packet_recv.checksum != compute_checksum(cur_packet_recv.data)) || (cur_packet_recv.seq_no != counter))
        {
            cur_packet_send.seq_no = cur_packet_recv.seq_no;    // if no correct checksum, then expected sequence number is the same
            flag = false;
        }
        else
        {
            cur_packet_send.seq_no = cur_packet_recv.seq_no + 1;    // else, increment expected sequence number
            printf("Received packet with sequence number %d\n", cur_packet_recv.seq_no);  // print message
            fwrite(cur_packet_recv.data, sizeof(char), cur_packet_recv.data_size, file_ptr);
            flag = cur_packet_recv.last_packet;
            counter += 1;
        }
        sent = sendto(client_socket, &cur_packet_send, sizeof(cur_packet_send), 0, (struct sockaddr *)&server, sizeof(server));  // send
        printf("Sent acknowledgement for received packet sequence number = %d\n", cur_packet_recv.seq_no);
        if (flag == true)
        {
            break;
        }
    }
    close(client_socket);
    fclose(file_ptr);
return 0;
}
