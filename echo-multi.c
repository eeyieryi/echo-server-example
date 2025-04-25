#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define MAX_CLIENTS 10

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

    struct pollfd pfds[MAX_CLIENTS + 1];
    pfds[0].fd = socket_fd;
    pfds[0].events = POLLIN;
    for (size_t i = 1; i <= MAX_CLIENTS; i++) {
        pfds[i].fd = -1;
    }

    char buffer[4096];
    for (;;) {
        int poll_count = poll(pfds, MAX_CLIENTS+1, -1);
        if (poll_count < 0) {
            fprintf(stderr, "ERROR: could not poll fds: %s\n", strerror(errno));
            return_defer(1);
        }

        if (pfds[0].revents & POLLIN) {
            int client_fd = accept(socket_fd, NULL, NULL);
            if (client_fd == -1) {
                fprintf(stderr, "ERROR: could not accept connection: %s\n", strerror(errno));
                continue;
            }

            for (size_t i = 1; i <= MAX_CLIENTS; i++) {
                if (pfds[i].fd == -1) {
                    pfds[i].fd = client_fd;
                    pfds[i].events = POLLIN;
                    printf("Connection accepted! fd: %d\n", pfds[i].fd);
                    break;
                }
            }
        }

        for (size_t i = 1; i <= MAX_CLIENTS; i++) {
            if (pfds[i].fd != -1 && (pfds[i].revents & POLLIN)) {
                int n = read(pfds[i].fd, buffer, sizeof(buffer));
                if (n <= 0) {
                    if (n == -1) {
                        fprintf(stderr, "ERROR: could not read from client socket: %s\n", strerror(errno));
                    }
                    printf("Closing connection! fd: %d\n", pfds[i].fd);
                    if (close(pfds[i].fd) == -1) {
                        fprintf(stderr, "ERROR: could not close client socket: %s\n", strerror(errno));
                        continue;
                    }
                    pfds[i].fd = -1;
                    pfds[i].events = 0;
                }
                write(pfds[i].fd, buffer, n);
            }
        }
    }


defer:
    if (close(socket_fd) == -1) {
        fprintf(stderr, "ERROR: could not close socket: %s", strerror(errno));
        result = 1;
    }
    return result;
}
