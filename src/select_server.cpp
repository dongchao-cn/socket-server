/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstring>
#include <cstdarg>
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
    if( bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
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
    
    fd_sets.insert(listen_sock);
    fd_set fdsr;
    while (1)
    {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(listen_sock, &fdsr);

        // add active connection to fd set
        set<int>::iterator it;
        for (it = fd_sets.begin(); it != fd_sets.end(); it++) 
            FD_SET(*it, &fdsr);

        ret = select(*fd_sets.rbegin() + 1, &fdsr, NULL, NULL, NULL);

        if (ret < 0) 
        {
            perror("select error");
            break;
        } 
        else if (ret == 0) 
        {
            perror("timeout\n");
            continue;
        }

        // check every fd in the set
        for (it = fd_sets.begin(); it != fd_sets.end(); it++)
        {
            if (FD_ISSET(*it, &fdsr))
            {
                if (*it == listen_sock)
                {
                    struct sockaddr_in client;
                    int new_fd = accept(listen_sock, (struct sockaddr *)&client, &addr_size);
                    if (new_fd <= 0) 
                    {
                        perror("accept new connection error");
                        continue;
                    }

                    // add to fd set
                    if (fd_sets.size()-1 < MAX_CLIENT_NUM)
                    {
                        fd_sets.insert(new_fd);
                        print("[Conn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                        printf("now_client_num: %u\n", (unsigned int)fd_sets.size()-1);
                    }
                    else
                    {
                        char message[] = "[ERR]too many clients!!!";
                        print("%s\n", message);
                        send(new_fd, message, strlen(message)+1, 0);
                        close(new_fd);
                    }

                }
                else
                {
                    char message[BUF_SIZE];
                    struct sockaddr_in client;

                    int read_size = recv(*it, message, BUF_SIZE, 0);

                    if (getpeername(*it, (struct sockaddr*)&client, &addr_size) != 0)
                    {
                        perror("getpeername Error!");
                        continue;
                    }

                    if (read_size <= 0) 
                    {        
                        // client close
                        print("[DConn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                        close(*it);
                        FD_CLR(*it, &fdsr);
                        fd_sets.erase(it);
                        printf("now_client_num: %u\n", (unsigned int)fd_sets.size()-1);
                    }
                    else
                    {        
                        // receive data
                        print("[Recv %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
                        //Send the message back to client
                        send(*it, message, strlen(message)+1, 0);
                        print("[Send %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
                    }
                }
            }
        }
    }
    return 0;
}
