#include "server.h"


void handle_http_get_req(int client_socket_fd, char* client_req, ssize_t client_req_nbytes) {
    ssize_t bytes_transmitted = client_req_nbytes;


    // parse client's req for the "Host:" field
    char* client_req_host_base   = strstr(client_req, "Host:") + 6;
    int   client_req_host_nbytes = strstr(client_req_host_base, "\r\n") - client_req_host_base;
    char* client_req_host        = malloc(client_req_host_nbytes + 1);  // alloc mem to store the "Host:" value
    memset(client_req_host, 0, client_req_host_nbytes + 1);
    strncpy(client_req_host, client_req_host_base, client_req_host_nbytes);

    printf("    url:    [%s]\n", client_req_host);


    // get target host addr from name
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_CANONNAME;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo* target_host_addrinfo = NULL;

    if(getaddrinfo(client_req_host, NULL, &hints, &target_host_addrinfo) != 0) {
        printf("ERR: failed to lookup target host\n");
        free(client_req_host);
        return;
    }

    // free up buffer for Host value after use
    free(client_req_host);


    // create socket for the target connection
    int target_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(target_socket_fd == -1) {
        printf("ERR: failed to create target socket\n");
        return;
    }

    // create new socket addr for the target
    struct sockaddr_in target_socket_addr;
    target_socket_addr.sin_family      = AF_INET;
    target_socket_addr.sin_port        = htons(80);
    target_socket_addr.sin_addr.s_addr = ((struct sockaddr_in*)target_host_addrinfo->ai_addr)->sin_addr.s_addr;


    // connect to target
    if(connect(target_socket_fd, (struct sockaddr*)&target_socket_addr, sizeof(target_socket_addr)) == -1) {
        printf("    ERR: failed to connect to target\n");
        return;
    } else {
        printf("    OK: connected to the target @ [%s]\n", inet_ntoa(((struct sockaddr_in*)target_host_addrinfo->ai_addr)->sin_addr));
    }

    freeaddrinfo(target_host_addrinfo);


    // alloc response buffer
    char* target_res_buffer = malloc(MAX_BUFF_SZ);
    memset(target_res_buffer, 0, MAX_BUFF_SZ);


    // forward client's req to the target
    ssize_t n;
    if((n = send(target_socket_fd, client_req, client_req_nbytes, 0)) == -1) {
        printf("    ERR: transmission failure\n");
        close(target_socket_fd);
        free(target_res_buffer);
        return;
    }
    bytes_transmitted += n;


    ssize_t nbytes;  // recv n bytes from target
    while((nbytes = recv(target_socket_fd, target_res_buffer, MAX_BUFF_SZ, 0)) > 0) {
        // send it to the client
        if((n = send(client_socket_fd, target_res_buffer, nbytes, 0)) == -1) {
            printf("    ERR: transmission failure\n");
            close(target_socket_fd);
            free(target_res_buffer);
            return;
        }
        memset(target_res_buffer, 0, MAX_BUFF_SZ);             // reset buffer for next packet
        
        bytes_transmitted += (nbytes + n);
    }

    // close connection to target
    close(target_socket_fd);

    printf("    OK: %ld bytes transmitted\n", bytes_transmitted);

    // free up response buffer
    free(target_res_buffer);
}

void client_handler(int client_socket_fd) {
    // alloc buffer to receive client's req
    char* client_req_buffer = malloc(MAX_BUFF_SZ);
    memset(client_req_buffer, 0, MAX_BUFF_SZ);


    // recv client's req
    ssize_t client_req_nbytes = recv(client_socket_fd, client_req_buffer, MAX_BUFF_SZ, 0);


    // parse client's req to find the HTTP Method
    if(strncmp(client_req_buffer, "GET", 3) == 0) {
        // HTTP GET Method
        printf("    method: [GET]\n");
        handle_http_get_req(client_socket_fd, client_req_buffer, client_req_nbytes);
    }else {
        printf("    ERR: unsupported method\n");
        const char* msg = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 41\r\nContent-Type: text/html\r\n\r\n<html><h1>501 Not Implemented</h1></html>";
        send(client_socket_fd, msg, strlen(msg), 0);
    }


    // free up client req buffer
    free(client_req_buffer);
}


int main() {
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


    // keep trying to accept incoming connections
    while(1) {
        struct sockaddr_in client_sock_addr;
        socklen_t c = sizeof(client_sock_addr);

        int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_sock_addr, &c);
        if(client_socket_fd == -1) {
            printf("ERR: failed to accept an incoming connection\n");
            continue;  // try for next req on failure
        }


        // serve client
        printf("OK: connection accepted from [%s]\n", inet_ntoa(client_sock_addr.sin_addr));
        client_handler(client_socket_fd);


        // clean up client side
        close(client_socket_fd);
    }


    // clean up server side
    close(server_socket_fd);

    return 0;
}

