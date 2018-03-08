// HTTP serial download application using TCP Sockets in C++
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/time.h>
#include<netdb.h>
#include<vector>

#define MAX_LEN 8192

using namespace std;

int main(int argc, char *argv[])
{
    int client_fd;  // Client side socket descriptor
    struct addrinfo hints, *res;  // HINTS and res addrinfo structs for resolving

    // Set fields to 0
    memset(&hints, 0, sizeof(hints));

    // Add TCP settings
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(argv[1], "80", &hints, &res);

    // Create socket
    if ((client_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        cout<<"Socket Creation Error"<<endl;
        exit(1);
    }

    // Connect to URL
    if (connect(client_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        cout<<"Connection Error"<<endl;
        exit(1);   
    }

    string header = "HEAD ";
    header.append(string(argv[2]));
    header.append(" HTTP/1.1\r\nHost: ");
    header.append(string(argv[1]));
    header.append("\r\n\r\n");

    cout<<"HEAD Request made: "<<header<<endl;

    if (send(client_fd, header.c_str(), header.length(), 0) == -1)
    {
        cout<<"HEAD Request failed"<<endl;
        exit(1);
    }

    char reply[MAX_LEN];
    int received = recv(client_fd, reply, sizeof(reply) - 1, 0);
    if (received < 0)
    {
        cout<<"HEAD Receive failed"<<endl;
        exit(1);
    }
    reply[received] = '\0';
    cout<<reply<<endl;

    // Substitute for REGEX
    string reply_string(reply);
    int index = reply_string.find(" ", reply_string.find("Content-Length"));
    int content_size = atoi(reply_string.substr(index, reply_string.find("\n", index) - index).c_str());
    cout<<"Content size parsed: "<<content_size<<endl;

    // Now send a GET Request to get data
    header = "GET ";
    header.append(string(argv[2]));
    header.append(" HTTP/1.1\r\nHost: ");
    header.append(string(argv[1]));
    header.append("\r\n\r\n");
    cout<<header<<endl;
    if (send(client_fd, header.c_str(), header.length(), 0) == -1)
    {
        cout<<"GET Request failed"<<endl;
    }

    // Receive the data from the reply
    ofstream output_file;
    output_file.open(argv[3], ios::binary | ios::out);

    int total_received = 0;
    while(total_received < content_size)
    {
        received = recv(client_fd, reply, sizeof(reply) - 1, 0);
        if (received < 0)
        {
            cout<<"GET Received failed"<<endl;
            exit(1);
        }

        int start = string(reply).find("%PDF");
        if (total_received != 0)
        {
            start = 0;
        }
        // To avoid misplacement of characters
        for (int i = start; i < received; i++)
        {
            output_file<<reply[i];
        }
        total_received += received;
    }
    close(client_fd);
return 0;
}
