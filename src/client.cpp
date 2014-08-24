/*
    C++ ECHO socket client
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "base.h"
using namespace std;

extern const int BUF_SIZE;
extern const char SERVER_IP[];
extern const int SERVER_PORT;

extern const unsigned int MAX_CLIENT_NUM;
extern const int CLIENT_INTERVAL;

int main(int argc, char *argv[])
{
    //Create socket
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("could not create socket");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
 
    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect failed");
        exit(1);
    }
    
    //keep communicating with server
    int count = 0;
    while(1)
    {
        char message[BUF_SIZE];
        sprintf(message, "%d", count++);
        
        //Send count
        print("[Send]: %s\n", message);
        if( send(sock, message, strlen(message)+1, 0) < 0)
        {
            perror("send failed");
            exit(1);
        }

        //Receive a reply from the server
        if( recv(sock, message, BUF_SIZE, 0) < 0)
        {
            perror("recv failed");
            exit(1);
        }
        
        print("[Recv]: %s\n", message);
        sleep(CLIENT_INTERVAL);
    }
    
    close(sock);
    exit(0);
}
