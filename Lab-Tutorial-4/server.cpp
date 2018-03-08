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

// GBN ARQ required variables
int base = 0, nextseqnum = 0, n_packets;
vector<packet* > all_packets;

void make_packets_from_file(char *root_loc, int full_file_size)
{
    ifstream input_file;
    input_file.open(root_loc, ios::binary | ios::in);
    int counter = 0;
    while(!input_file.eof())
    {
        packet *cur_packet = new packet;
        cur_packet->seq_no = counter++;
        input_file.read(cur_packet->data, MAX_LEN);
        if (counter * MAX_LEN >= full_file_size)
        {
            cur_packet->last_packet = true;
            cur_packet->data_size = full_file_size - (counter - 1) * MAX_LEN;
        }
        else
        {
            cur_packet->data_size = MAX_LEN;
        }
        cur_packet->checksum = compute_checksum(cur_packet->data);
        all_packets.push_back(cur_packet);
    }
}

// Receiver Function and Networking variables
int server_socket;
struct sockaddr_in server, client;

void receiver_func()
{
    packet *ACK = new packet;
    packet *to_send;
    struct sockaddr recvfrom_addr;
    socklen_t recvfrom_addr_len = sizeof(recvfrom_addr);
    while(1)
    {
        if (recvfrom(server_socket, ACK, sizeof(*ACK), 0, &recvfrom_addr, &recvfrom_addr_len) < 0)
        {
            int _base = base, _nextseqnum = nextseqnum;
            cout<<"Resending from : "<<_base<<" to "<<_nextseqnum<<endl;
            for(int i = _base; i < _nextseqnum; i++)
            {
                to_send = all_packets[i];
                sendto(server_socket, to_send, sizeof(*to_send), 0, (struct sockaddr *)&client, sizeof((struct sockaddr *)&client));
            }
        }
        else
        {
            cout<<"Received ACK "<<ACK->ack_no<<endl;
            base = ACK->ack_no;
            if (base >= (n_packets - 1))
            {
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int server_PORT = atoi(argv[1]);
    int client_PORT = atoi(argv[2]);
    
    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Socket Creation Error\n");
        exit(1);
    }

    // Timeout setting
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
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
    bzero(&client.sin_zero, 8);  // Fill empty fields with 0

    // Get file details and make packets using the file
    struct stat details_file;
    stat(argv[3], &details_file);

    make_packets_from_file(argv[3], details_file.st_size);
    n_packets = all_packets.size();

    // Sending Loop
    packet *to_send;
    thread receiver_thread = thread(receiver_func);
    struct sockaddr* client_addr = (struct sockaddr*) (&client);
    while (base < n_packets - 1)
    {
        if ((nextseqnum < base + WINDOW_SIZE) && (nextseqnum < n_packets))
        {
            to_send = all_packets[nextseqnum];
            sendto(server_socket, to_send, sizeof(*to_send), 0, client_addr, sizeof(*client_addr));
            cout<<"Sending packet with sequence number : "<<to_send->seq_no<<endl;
            nextseqnum++;
        }
    }
    receiver_thread.join();
    close(server_socket);
return 0;
}
