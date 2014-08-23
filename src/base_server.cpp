/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "base.h"
using namespace std;

extern const int BUF_SIZE;
extern const char SERVER_IP[];
extern const int SERVER_PORT;
extern const int BACKLOG;

int main(int argc, char *argv[])
{
    // create socket
    int listen_sock = get_listen_socket();
    
    // set reuse
    set_reuse_addr(listen_sock);

    // bind
    listen_socket_bind(listen_sock);
    
    // listen
    listen_socket_listen(listen_sock);

    // start success
    print_start();

    while (1)
    {
        //accept connection from an incoming client
        int client_sock = accept_socket(listen_sock);
        if (client_sock == -1)
        {
            perror("accept failed");
            continue;
        }

        print_conn(client_sock);
        
        //Receive a message from client
        int read_size;
        char message[BUF_SIZE];
        while( (read_size = recv(client_sock, message, BUF_SIZE, 0)) > 0 )
        {
            print_recv(client_sock, message);
            //Send the message back to client
            send(client_sock, message, strlen(message)+1, 0);
            print_send(client_sock, message);
        }
        print_dconn(client_sock);
        close(client_sock);
    }
}
