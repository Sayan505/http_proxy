#pragma once


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


void* client_handler(void* __client_socket_fd);
void  handle_http_get_req(int client_socket_fd, char* client_req_buffer, ssize_t client_req_nbytes);


#define SRV_PORT    ((short)(5555))
#define MAX_CLIENTS ((int)(64))
#define MAX_BUFF_SZ ((size_t)(4096))

