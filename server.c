#include "server.h"


// default: cache on
static volatile int usecache = 1;    // turn off cache: proxy_server -nocache

// access control for clients
sem_t semaphore;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile sig_atomic_t sigint_received = 0;

void shutdown_handler(int signo) {
    (void)signo;  // discard

    sigint_received = 1;
}


void handle_http_get_req(int client_socket_fd, char* client_req, ssize_t client_req_nbytes, char* client_req_url) {
    ssize_t bytes_transmitted = client_req_nbytes;


    // parse client's req for the "Host:" field
    char* client_req_host_base   = strstr(client_req, "Host:") + 6;
    int   client_req_host_nbytes = strstr(client_req_host_base, "\r\n") - client_req_host_base;
    char* client_req_host        = malloc(client_req_host_nbytes + 1);  // alloc mem to store the "Host:" value
    memset(client_req_host, 0, client_req_host_nbytes + 1);
    strncpy(client_req_host, client_req_host_base, client_req_host_nbytes);

    printf("    host:    [%s]\t(%d)\n", client_req_host, client_socket_fd);


    // get target host addr from name
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo* target_host_addrinfo = NULL;

    if(getaddrinfo(client_req_host, NULL, &hints, &target_host_addrinfo) != 0) {
        printf("ERR: failed to lookup target host\t(%d)\n", client_socket_fd);
        free(client_req_host);
        return;
    }

    // free up buffer for Host value after use
    free(client_req_host);


    // create socket for the target connection
    int target_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(target_socket_fd == -1) {
        printf("ERR: failed to create target socket\t(%d)\n", client_socket_fd);
        return;
    }

    // create new socket addr for the target
    struct sockaddr_in target_socket_addr;
    target_socket_addr.sin_family      = AF_INET;
    target_socket_addr.sin_port        = htons(80);
    target_socket_addr.sin_addr.s_addr = ((struct sockaddr_in*)target_host_addrinfo->ai_addr)->sin_addr.s_addr;


    // connect to target
    if(connect(target_socket_fd, (struct sockaddr*)&target_socket_addr, sizeof(target_socket_addr)) == -1) {
        printf("    ERR: failed to connect to target\t(%d)\n", client_socket_fd);
        return;
    } else {
        printf("    OK: connected to the target @ [%s]\t(%d)\n",
        inet_ntoa(((struct sockaddr_in*)target_host_addrinfo->ai_addr)->sin_addr), client_socket_fd);
    }

    freeaddrinfo(target_host_addrinfo);


    // alloc response buffer
    char* target_res_buffer = malloc(MAX_RES_BUFF_SZ);
    memset(target_res_buffer, 0, MAX_RES_BUFF_SZ);


    // alloc cache buffer if allowed
    int   discard_cache_buffer = 0;  // discard caching if the response is > MAX_CACHE_RES_SIZE
    char* cache_buffer = NULL;
    if(usecache) {
        cache_buffer = malloc(MAX_CACHE_RES_SIZE);
        memset(cache_buffer, 0, MAX_CACHE_RES_SIZE);
    }


    // forward client's req to the target
    ssize_t n;
    if((n = send(target_socket_fd, client_req, client_req_nbytes, 0)) == -1) {
        printf("    ERR: transmission failure\t(%d)\n", client_socket_fd);
        close(target_socket_fd);
        free(target_res_buffer);
        return;
    }
    bytes_transmitted += n;


    ssize_t cache_buffer_offset = 0;
    ssize_t nbytes  = 0;  // num bytes received
    while((nbytes = recv(target_socket_fd, target_res_buffer, MAX_RES_BUFF_SZ, 0)) > 0) {
        // send it to the client
        if((n = send(client_socket_fd, target_res_buffer, nbytes, 0)) == -1) {
            printf("    ERR: transmission failure\t(%d)\n", client_socket_fd);
            close(target_socket_fd);
            free(target_res_buffer);
            return;
        }

        // store the response in cache if within the size constraints
        if((usecache) && (discard_cache_buffer == 0)) {
            if((cache_buffer_offset + nbytes) <= MAX_CACHE_RES_SIZE) {
                memcpy(cache_buffer + cache_buffer_offset, target_res_buffer, nbytes);
                cache_buffer_offset += nbytes;
            } else {
                discard_cache_buffer = 1;
            }
        }

        memset(target_res_buffer, 0, nbytes);  // reset target buffer for next chunk transmission

        bytes_transmitted += (nbytes + n);
    }

    // close connection to target
    close(target_socket_fd);

    printf("    OK: %ld bytes transmitted\t(%d)\n", bytes_transmitted, client_socket_fd);


    if(usecache) {
        // attempt to cache the response
        if(discard_cache_buffer == 0) {
            pthread_mutex_lock(&mutex);
            int updated = cache_upsert(client_req_url, cache_buffer, &cache_buffer_offset);  // cache_buffer_offset is the total response size in bytes
            pthread_mutex_unlock(&mutex);

            if(updated) {
                printf("    OK: updated the cache [%ld bytes]\t(%d)\n", cache_buffer_offset, client_socket_fd);
            } else {
                printf("    OK: cached the response [%ld bytes]\t(%d)\n", cache_buffer_offset, client_socket_fd);
            }
        } else {
            printf("    [response not cached]\t(%d)\n", client_socket_fd);
        }

        // free up the cache buffer
        free(cache_buffer);
    }

    // free up response buffer
    free(target_res_buffer);
}

void* client_handler(void* __client_socket_fd) {
    sem_wait(&semaphore);

    int sval;
    sem_getvalue(&semaphore, &sval);
    printf("!number of clients: [%d/%d]\n", MAX_CLIENTS - sval, MAX_CLIENTS);


    int client_socket_fd = *(int*)__client_socket_fd;


    // alloc buffer to receive client's req
    char* client_req_buffer = malloc(MAX_REQ_BUFF_SZ);
    memset(client_req_buffer, 0, MAX_REQ_BUFF_SZ);


    // recv client's req
    ssize_t client_req_nbytes = recv(client_socket_fd, client_req_buffer, MAX_REQ_BUFF_SZ, 0);
    if(client_req_nbytes <= 0) {
        printf("    [client transmitted 0 bytes]\t(%d)\n", client_socket_fd);
        const char* msg = "HTTP/1.1 400 Bad Request\r\nContent-Length: 37\r\nContent-Type: text/html\r\n\r\n<html><h1>400 Bad Request</h1></html>";
        send(client_socket_fd, msg, strlen(msg), 0);

        shutdown(client_socket_fd, SHUT_RDWR);
        close(client_socket_fd);
        free(client_req_buffer);

        sem_post(&semaphore);

        return NULL;
    }


    // parse client's req to find the http method
    char* client_req_method_base   = client_req_buffer;
    int   client_req_method_nbytes = strchr(client_req_method_base, ' ') - client_req_method_base;
    char* client_req_method        = malloc(client_req_method_nbytes + 1);  // alloc mem to store the client's req method
    memset(client_req_method, 0, client_req_method_nbytes + 1);
    strncpy(client_req_method, client_req_method_base, client_req_method_nbytes);


    // parse client's req to find the URL
    char* client_req_url_base   = client_req_method_base + client_req_method_nbytes + 1;
    int   client_req_url_nbytes = strchr(client_req_url_base, ' ') - client_req_url_base;
    char* client_req_url        = malloc(client_req_url_nbytes + 1);  // alloc mem to store the client's req url
    memset(client_req_url, 0, client_req_url_nbytes + 1);
    strncpy(client_req_url, client_req_url_base, client_req_url_nbytes);


    // handle client request
    if(strcmp(client_req_method, "GET") == 0) {
        // HTTP GET Method
        printf("    method: [%s]\t(%d)\n", client_req_method, client_socket_fd);
        printf("    url:    [%s]\t(%d)\n", client_req_url,    client_socket_fd);

        // try to refer to the cache if allowed
        char*   cached_response        = NULL;
        ssize_t cached_response_nbytes = 0;
        if(usecache) {
            pthread_mutex_lock(&mutex);
            cached_response = cache_refer(client_req_url, &cached_response_nbytes);
            pthread_mutex_unlock(&mutex);
        }

        // and return the cached result if cache is hit
        if(cached_response) {
            // return cached response
            printf("    OK: retrieved response from cache [%ld bytes]\t(%d)\n", cached_response_nbytes, client_socket_fd);
            ssize_t nbytes_sent_to_client = send(client_socket_fd, cached_response, cached_response_nbytes, 0);
            printf("    OK: %ld bytes transmitted\t(%d)\n", client_req_nbytes + nbytes_sent_to_client, client_socket_fd);
        } else {
            // else fetch response from target server
            handle_http_get_req(client_socket_fd, client_req_buffer, client_req_nbytes, client_req_url);
        }
    } else {
        printf("    ERR: unsupported method [%s]\t(%d)\n", client_req_method, client_socket_fd);
        const char* msg = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 41\r\nContent-Type: text/html\r\n\r\n<html><h1>501 Not Implemented</h1></html>";
        send(client_socket_fd, msg, strlen(msg), 0);
    }


    // close client socket after transmission
    shutdown(client_socket_fd, SHUT_RDWR);
    close(client_socket_fd);

    // free up client req buffers
    free(client_req_buffer);
    free(client_req_method);
    free(client_req_url);


    sem_post(&semaphore);


    return NULL;
}


int main(int argc, char** argv) {
    if(argc > 1) {
        if(strcmp(argv[0], "-nocache") == 0) {
            usecache = 0;
        }
    }


    // init the semaphore with MAX_CLIENTS number of allowed acquisitions
    sem_init(&semaphore, 0, (unsigned int)MAX_CLIENTS);


    // create new socket for server
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd == -1) {
        printf("ERR: failed to create server socket\n");
        return 1;
    }


    // create new socket addr for server
    struct sockaddr_in server_socket_addr;
    memset(&server_socket_addr, 0, sizeof(server_socket_addr));

    server_socket_addr.sin_family      = AF_INET;
    server_socket_addr.sin_port        = htons(SRV_PORT);    // listening port
    server_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // accept conn from any addr


    // bind the server socket with the addr
    if(bind(server_socket_fd, (struct sockaddr*)&server_socket_addr, sizeof(server_socket_addr)) == -1) {
        printf("ERR: failed to bind server socket to address\n");
        return 1;
    }


    // start listening for incoming connections
    if(listen(server_socket_fd, MAX_CLIENTS) == -1) {
        printf("ERR: failed listen on server socket\n");
        return 1;
    }

    printf("OK: server is listening for incoming connections @ port %d\n", SRV_PORT);


    // rig shutdown handler for a graceful shutdown on CTRL+C
    struct sigaction shutdown_action;
    shutdown_action.sa_flags   = SA_INTERRUPT;
    shutdown_action.sa_handler = shutdown_handler;
    sigaction(SIGINT, &shutdown_action, NULL);


    // keep trying to accept incoming connections until SIGINT
    while(!sigint_received) {
        struct sockaddr_in client_sock_addr;
        socklen_t c = sizeof(client_sock_addr);

        int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_sock_addr, &c);
        if(client_socket_fd == -1) {
            continue;  // try for next req on failure
        }


        // serve client
        printf("OK: connection accepted from [%s]\t(%d)\n", inet_ntoa(client_sock_addr.sin_addr), client_socket_fd);
        pthread_t dummy_tid;
        pthread_create(&dummy_tid, NULL, client_handler, &client_socket_fd);
    }


    printf("[SHUTDOWN]\n");


    // close server socket
    close(server_socket_fd);

    // free the semaphore
    sem_destroy(&semaphore);

    return 0;
}

