#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define return_defer(value) do { result = (value); goto defer; } while (0)

int main(void) {
    int result;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        fprintf(stderr, "ERROR: could not create socket: %s\n", strerror(errno));
        return_defer(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "ERROR: could not bind address to socket: %s\n", strerror(errno));
        return_defer(1);
    }


    if (listen(socket_fd, 1) == -1) {
        fprintf(stderr, "ERROR: could not listen to socket: %s\n", strerror(errno));
        return_defer(1);
    }

    // TODO: Use threads to support more than one connection at a time

    char buffer[4096];
    for (;;) {
        int client_fd = accept(socket_fd, NULL, NULL);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: could not accept connection: %s\n", strerror(errno));
            continue;
        }

        printf("Connection accepted!\n");

        int n = 0;
        do {
            n = read(client_fd, buffer, sizeof(buffer));
            if (n > 0) write(client_fd, buffer, n);
        } while (n > 0);
        if (n == -1) {
            fprintf(stderr, "ERROR: could not read from client socket: %s\n", strerror(errno));
            goto close_client;
        }

    close_client:
        printf("Closing connection!\n");
        if (close(client_fd) == -1) {
            fprintf(stderr, "ERROR: could not close client socket: %s\n", strerror(errno));
            continue;
        }
    }


defer:
    if (close(socket_fd) == -1) {
        fprintf(stderr, "ERROR: could not close socket: %s", strerror(errno));
        result = 1;
    }
    return result;
}
