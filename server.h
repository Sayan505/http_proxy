#pragma once


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "lru_cache.h"


void  shutdown_handler(int signo);
void* client_handler(void* __client_socket_fd);
void  handle_http_get_req(int client_socket_fd, char* client_req_buffer, ssize_t client_req_nbytes, char* client_req_url);


#define SRV_PORT        ((short)(5555))
#define MAX_CLIENTS     ((int)(255))
#define MAX_REQ_BUFF_SZ ((int)(4096))
#define MAX_RES_BUFF_SZ ((int)(4096))

