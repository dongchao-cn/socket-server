/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <set>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include "base.h"
using namespace std;

extern const int BUF_SIZE;
extern const char SERVER_IP[];
extern const int SERVER_PORT;
extern const int BACKLOG;
extern const unsigned int MAX_CLIENT_NUM;

set<int> fd_sets;

void add_client(int client_sock)
{
    fd_sets.insert(client_sock);
    print_conn(client_sock);
    printf("now_client_num: %u\n", (unsigned int)fd_sets.size()-1);
}

void del_client(int client_sock)
{
    fd_sets.erase(client_sock);
    print_dconn(client_sock);
    close(client_sock);
    printf("now_client_num: %u\n", (unsigned int)fd_sets.size()-1);
}

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
    
    fd_sets.insert(listen_sock);
    fd_set fdsr;
    while (1)
    {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(listen_sock, &fdsr);

        // add active connection to fd set
        set<int>::iterator it;
        printf("%d\n", (int)(fd_sets.size()));
        for (it = fd_sets.begin(); it != fd_sets.end(); it++) 
            FD_SET(*it, &fdsr);

        int ret = select(*fd_sets.rbegin() + 1, &fdsr, NULL, NULL, NULL);

        if (ret <= 0) 
        {
            // timeout is error
            perror("select error");
            exit(1);
        }

        // check every fd in the set
        for (it = fd_sets.begin(); it != fd_sets.end(); it++)
        {
            if (FD_ISSET(*it, &fdsr))
            {
                if (*it == listen_sock)
                {
                    // new client
                    int client_sock = accept_socket(listen_sock);
                    if (client_sock == -1)
                    {
                        perror("accept failed");
                        continue;
                    }

                    // add to fd set
                    if (fd_sets.size()-1 < MAX_CLIENT_NUM)
                    {
                        add_client(client_sock);
                    }
                    else
                    {
                        too_many_clients(client_sock);
                    }
                }
                else
                {
                    // client echo message
                    char message[BUF_SIZE];
                    int read_size = recv(*it, message, BUF_SIZE, 0);

                    if (read_size <= 0) 
                    {        
                        // client close
                        FD_CLR(*it, &fdsr);
                        del_client(*it);
                    }
                    else
                    {
                        // receive data
                        print_recv(*it, message);
                        //Send the message back to client
                        send(*it, message, strlen(message)+1, 0);
                        print_send(*it, message);
                    }
                }
            }
        }
    }
    return 0;
}
