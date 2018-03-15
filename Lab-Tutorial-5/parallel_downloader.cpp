// HTTP parallel download application using TCP Sockets in C++
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
#include<thread>
#include<ctime>

#define MAX_LEN 1500

using namespace std;
using namespace chrono;

struct addrinfo hints, *res;  // HINTS and res addrinfo structs for resolving

void get_ranges(string file_name, string host_name, int start, int end)
{
    int client_fd;

    // Create socket for each thread
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

    // Now send a GET Request to get data
    string range = "Range: bytes=";
    range.append(to_string(start));
    range.append("-");
    range.append(to_string(end));

    string header = "GET ";
    header.append(file_name);
    header.append(" HTTP/1.1\r\nHost: ");
    header.append(host_name);
    header.append("\r\n");
    header.append(range);
    header.append("\r\n\r\n");
    if (send(client_fd, header.c_str(), header.length(), 0) == -1)
    {
        cout<<"GET Request failed"<<endl;
    }

    // Receive data from the GET Request
    // Store in separate file
    ofstream temp_file;
    string temp_file_name = "temp_file-";
    temp_file_name.append(range);
    temp_file_name.append(".pdf");
    temp_file.open(temp_file_name.c_str(), ios::binary | ios::out);
    int content_size = end - start + 1;
    int total_received = 0;

    char reply[MAX_LEN];
    while(total_received < content_size)
    {
        int received = recv(client_fd, reply, sizeof(reply) - 1, 0);
        if (received < 0)
        {
            cout<<"GET Received failed"<<endl;
            exit(1);
        }

        int start = 0;
        if (total_received == 0)
        {
            start = string(reply).find("\r\n\r\n") + 4;
        }

        // To avoid misplacement of characters
        for (int i = start; i < received; i++)
        {
            temp_file<<reply[i];
        }
        total_received += received;
    }
    temp_file.close();
    close(client_fd);
    cout<<"Thread for start="<<start<<" and end="<<end<<" has completed"<<endl;
}

int main(int argc, char *argv[])
{
    int client_fd;  // Client side socket descriptor

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
    close(client_fd);

    // Substitute for REGEX
    string reply_string(reply);
    int index = reply_string.find(" ", reply_string.find("Content-Length"));
    int content_size = atoi(reply_string.substr(index, reply_string.find("\n", index) - index).c_str());
    cout<<"Content size parsed: "<<content_size<<endl;

    // Time calculation begin
    auto start = high_resolution_clock::now();

    int n_threads = atoi(argv[4]);
    int max_chunk = content_size / n_threads + 1;

    thread get_threads[n_threads];
    for (int i = 0; i < n_threads; i++)
    {
        int beginning = i * max_chunk;
        int ending = (i + 1) * max_chunk - 1;
        if (ending > content_size)
        {
            ending = content_size - 1;
        }
        cout<<"Launching thread for start="<<beginning<<" and end="<<ending<<endl;
        get_threads[i] = thread(get_ranges, string(argv[2]), string(argv[1]), beginning, ending);
    }
    for (int i = 0; i < n_threads; i++)
    {
        get_threads[i].join();
    }

    // Time calculation end
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(end - start);

    cout<<"Time taken for download: "<<dur.count()<<" milliseconds"<<endl;

    // Receive the data from the reply
    ofstream output_file("downloaded.pdf", ios_base::binary);

    // Accumulate all the temp_files
    for (int i = 0; i < n_threads; i++)
    {
        int beginning = i * max_chunk;
        int ending = (i + 1) * max_chunk - 1;
        if (ending > content_size)
        {
            ending = content_size - 1;
        }
        string temp_file_name = "temp_file-Range: bytes=";
        temp_file_name.append(to_string(beginning));        
        temp_file_name.append("-");
        temp_file_name.append(to_string(ending));
        temp_file_name.append(".pdf");

        ifstream input_file(temp_file_name.c_str(), ios_base::binary);
        output_file << input_file.rdbuf();
        remove(temp_file_name.c_str());
    }
    output_file.close();
return 0;
}
