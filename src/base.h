#ifndef _BASE_H
#define _BASE_H
#include <netinet/in.h>

int get_listen_socket();
void set_reuse_addr(int listen_sock);
void listen_socket_bind(int listen_sock);
void listen_socket_listen(int listen_sock);
int accept_socket(int listen_sock);
sockaddr_in get_client_addr(int sock);
void too_many_clients(int client_sock);

void print_start();
void print_conn(int client_sock);
void print_recv(int client_sock, char* message);
void print_send(int client_sock, char* message);
void print_dconn(int client_sock);

#ifdef DEBUG
#define print(fmt, arg...) printf(fmt, ##arg)
#else
#define print(fmt, arg...)
#endif

#endif