/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "base.h"
using namespace std;

extern const int BUF_SIZE;
extern const char SERVER_IP[];
extern const int SERVER_PORT;
extern const int BACKLOG;

int main(int argc, char *argv[])
{
    //Create socket
    int listen_sock;
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        perror("Could not create socket");
        return 1;
    }
    
    //Set SO_REUSEADDR
    int ret;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(ret)) == -1) {
        perror("setsockopt");
        return 1;
    }

    //Prepare the sockaddr_in structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    //Bind
    if( bind(listen_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    
    //Listen
    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("listen error");
        return 1;
    }
    
    //Accept and incoming connection
    print("Server started success, listen at %s:%d\n", SERVER_IP, SERVER_PORT);
    socklen_t addr_size = sizeof(sockaddr_in);
    while (1)
    {
        //accept connection from an incoming client
        int client_sock;
        struct sockaddr_in client;
        client_sock = accept(listen_sock, (sockaddr *)&client, &addr_size);
        if (client_sock < 0)
        {
            perror("accept failed");
            return 1;
        }
        print("[Conn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        
        //Receive a message from client
        int read_size;
        char message[BUF_SIZE];
        while( (read_size = recv(client_sock, message, BUF_SIZE, 0)) > 0 )
        {
            print("[Recv %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
            //Send the message back to client
            send(client_sock, message, strlen(message)+1, 0);
            print("[Send %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
        }
        
        if(read_size == 0)
            print("[DConn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        else if(read_size == -1)
            perror("recv failed");
    }
    return 0;
}
