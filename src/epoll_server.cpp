/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "base.h"
using namespace std;

extern const int BUF_SIZE;
extern const char SERVER_IP[];
extern const int SERVER_PORT;
extern const int BACKLOG;
extern const unsigned int MAX_CLIENT_NUM;

int now_client_num = 0;
pthread_spinlock_t now_client_num_lock;

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
    
    // create epoll
    int epollfd = epoll_create(MAX_CLIENT_NUM);
    if (epollfd == -1) {
        perror("epoll create error");
        return 1;
    }

    // add listen_sock to epoll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl add listen_sock error");
        return 1;
    }

    epoll_event events[MAX_CLIENT_NUM];
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in client;
    pthread_spin_init(&now_client_num_lock, 0);
    print("Server started success, listen at %s:%d\n", SERVER_IP, SERVER_PORT);
    while(1) 
    {
        int nfds = epoll_wait(epollfd, events, MAX_CLIENT_NUM, -1);
        if (nfds == -1) {
            perror("epoll_pwait");
            return 1;
        }

        for (int i = 0; i < nfds; ++i) 
        {
            if (events[i].data.fd == listen_sock) 
            {
                // accept new client
                int new_fd = accept(listen_sock, (struct sockaddr *)&client, &addr_size);
                if (now_client_num < MAX_CLIENT_NUM)
                {
                    if (new_fd == -1) {
                        perror("accept failed");
                        close(new_fd);
                        continue;
                    }

                    // add now_client_num
                    pthread_spin_lock(&now_client_num_lock);
                    now_client_num++;
                    pthread_spin_unlock(&now_client_num_lock);

                    ev.events = EPOLLIN | EPOLLERR;
                    ev.data.fd = new_fd;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, new_fd, &ev) == -1) {
                        perror("epoll_ctl new_fd error");
                        // plus now_client_num
                        pthread_spin_lock(&now_client_num_lock);
                        now_client_num--;
                        pthread_spin_unlock(&now_client_num_lock);
                        close(new_fd);
                        continue;
                    }
                    print("[Conn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    printf("now_client_num: %u\n", now_client_num);
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
                // echo socket
                char message[BUF_SIZE];

                int read_size = recv(events[i].data.fd, message, BUF_SIZE, 0);

                if (getpeername(events[i].data.fd, (struct sockaddr*)&client, &addr_size) != 0)
                {
                    perror("getpeername Error!");
                    continue;
                }

                if (read_size <= 0) 
                {        
                    // client close
                    print("[DConn]: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                        perror("epoll_ctl del error");
                        continue;
                    }

                    // plus now_client_num
                    pthread_spin_lock(&now_client_num_lock);
                    now_client_num--;
                    pthread_spin_unlock(&now_client_num_lock);
                    printf("now_client_num: %u\n", now_client_num);
                }
                else
                {        
                    // receive data
                    print("[Recv %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
                    //Send the message back to client
                    send(events[i].data.fd, message, strlen(message)+1, 0);
                    print("[Send %s:%d]: %s\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
                }
            }
        }
    }
    return 0;
}
