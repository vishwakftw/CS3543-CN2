#include<iostream>
#include<fstream>
#include<cstdlib>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<cstring>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/time.h>
#include<netdb.h>
#include<vector>
#include<thread>
#include<mutex>

#define MAX_LEN 1000
#define WINDOW_SIZE 4

using namespace std;

// Compute checksum
char compute_checksum(char cur_string[MAX_LEN])
{
    char checksum = cur_string[0];
    for (int i = 1; i < MAX_LEN; i++)
    {
        checksum = checksum ^ cur_string[i];
    }
return checksum;
}

// Packet structure
typedef struct packet
{
    char data[MAX_LEN];
    char checksum;
    int seq_no;
    int ack_no;
    int data_size;
    bool last_packet;
    packet ()
    {
        for (int i = 0; i < MAX_LEN; i++)
        {
            data[i] = '\0';
        }
        checksum = '\0';
        seq_no = 0;
        ack_no = 0;
        data_size = 0;
        last_packet = false;
    }
} packet;

int main(int argc, char *argv[])
{
    int client_socket;
    struct sockaddr_in server, client;
    int server_PORT = atoi(argv[1]);
    int client_PORT = atoi(argv[2]);

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

    ofstream output_file;
    output_file.open(argv[3], ios::binary | ios::out);

    packet *to_send = new packet;
    packet *to_recv = new packet;
    struct sockaddr recvfrom_addr;
    socklen_t recvfrom_addr_len = sizeof(recvfrom_addr);

    int expectedseqnum = 0;
    while(1)
    {
        recvfrom(client_socket, to_recv, sizeof(*to_recv), 0, &recvfrom_addr, &recvfrom_addr_len);
        if ((to_recv->checksum != compute_checksum(to_recv->data)) || (to_recv->seq_no != expectedseqnum))
        {
            cout<<"Incorrect packet sent: expectedseqnum = "<<expectedseqnum<<" versus actual = "<<to_recv->seq_no<<endl;
            to_send->ack_no = expectedseqnum;
            sendto(client_socket, to_send, sizeof(*to_send), 0, &recvfrom_addr, sizeof(recvfrom_addr));
            recvfrom(client_socket, to_recv, sizeof(*to_recv), 0, &recvfrom_addr, &recvfrom_addr_len);
        }
        else
        {
            cout<<"Received correct packet for sequence number = "<<to_recv->seq_no<<endl;
            to_send->ack_no = expectedseqnum++;
            sendto(client_socket, to_send, sizeof(*to_send), 0, &recvfrom_addr, sizeof(recvfrom_addr));

            // write character by character to avoid junk writing
            for (int i = 0; i < to_recv->data_size; i++)
            {
                output_file<<to_recv->data[i];
            }
            if (to_recv->last_packet)
            {
                break;
            }
        }
    }
    close(client_socket);
return 0;
}
