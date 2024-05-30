#include "server.h"

void handle_client_conn(int client_socket) {
    char buffer[4096];
    memset(&buffer, 0, sizeof(buffer));

    // read client req
    size_t bytes_read = read(client_socket, buffer, sizeof(buffer));
    buffer[bytes_read] = '\0';

    //printf("%s", buffer);

    write(client_socket, "http://www.google.com/", 23);
    close(client_socket);
}
int main() {
    int socket_endp = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_endp == -1) {
        printf("ERR SOCK\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port        = htons(SRV_PORT);

    if(bind(socket_endp, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("ERR BIND\n");
        return 1;
    }

    if(listen(socket_endp, MAX_CLIENTS) == -1) {
        printf("ERR LISTEN\n");
    }

    printf("listening\n");

    // accept loop
    for(;;) {
        printf("meow\n");
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);

        int client_socket = accept(socket_endp, (struct sockaddr*)&client_addr, &client_addr_len);
        if(client_socket == -1) {
            printf("ERR CLIENT SOCK\n");
            continue;
        }
        printf("ACCEPT: %d", client_socket);

        handle_client_conn(client_socket);
        close(socket_endp);
        break;
    }


    close(socket_endp);
    printf("closed\n");

    return 0;
}

