/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstdlib>
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

void add_client(int client_sock)
{
    pthread_spin_lock(&now_client_num_lock);
    now_client_num++;
    pthread_spin_unlock(&now_client_num_lock);
    print_conn(client_sock);
    printf("now_client_num: %u\n", now_client_num);
}

void del_client(int client_sock)
{
    pthread_spin_lock(&now_client_num_lock);
    now_client_num--;
    pthread_spin_unlock(&now_client_num_lock);
    print_dconn(client_sock);
    close(client_sock);
    printf("now_client_num: %u\n", now_client_num);
}

epoll_event *events = new epoll_event[MAX_CLIENT_NUM]; // alloc in heap

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
    
    // create epoll
    int epollfd = epoll_create(MAX_CLIENT_NUM);
    if (epollfd == -1) {
        perror("epoll create error");
        exit(1);
    }

    // add listen_sock to epoll
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl add listen_sock error");
        exit(1);
    }

    // start success
    print_start();
    
    pthread_spin_init(&now_client_num_lock, 0);
    while(1) 
    {
        int nfds = epoll_wait(epollfd, events, MAX_CLIENT_NUM, -1);
        if (nfds == -1) {
            perror("epoll_pwait");
            exit(1);
        }

        for (int i = 0; i < nfds; ++i) 
        {
            if (events[i].data.fd == listen_sock) 
            {
                // new client
                int client_sock = accept_socket(listen_sock);
                if (client_sock == -1)
                {
                    perror("accept failed");
                    continue;
                }

                if (now_client_num < MAX_CLIENT_NUM)
                {
                    ev.events = EPOLLIN;
                    ev.data.fd = client_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &ev) == -1) {
                        perror("epoll_ctl add client_sock error");
                        close(client_sock);
                        continue;
                    }

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
                int read_size = recv(events[i].data.fd, message, BUF_SIZE, 0);

                if (read_size <= 0) 
                {        
                    // client close
                    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                        perror("epoll_ctl del error");
                    }
                    del_client(events[i].data.fd);
                }
                else
                {        
                    // receive data
                    print_recv(events[i].data.fd, message);
                    //Send the message back to client
                    send(events[i].data.fd, message, strlen(message)+1, 0);
                    print_send(events[i].data.fd, message);
                }
            }
        }
    }
}
