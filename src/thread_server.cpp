/*
    C++ ECHO socket server
*/
#include <cstdio>
#include <cstring>
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

unsigned int now_client_num = 0;
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

void* echo(void *ptr);

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

    pthread_spin_init(&now_client_num_lock, 0);

    while (1)
    {
        //accept connection from an incoming client
        int client_sock = accept_socket(listen_sock);
        if (client_sock == -1)
        {
            perror("accept failed");
            continue;
        }
        
        if (now_client_num < MAX_CLIENT_NUM)
        {
            // create new thread
            pthread_t thread;
            int *p_client_sock = new int;
            *p_client_sock = client_sock;
            if (pthread_create(&thread, NULL, echo, p_client_sock) != 0)
            {
                perror("create thread failed");
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
}

void* echo(void *ptr)
{
    int client_sock = *(int *)ptr;
    delete (int *)ptr;

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
    del_client(client_sock);
}
