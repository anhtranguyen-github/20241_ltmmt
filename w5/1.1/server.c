#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 100
#define MAX_CLIENTS 10

int main() {
    int sockfd, client_sock, max_fd, activity, i, valread, sd, new_socket;
    int clients[MAX_CLIENTS]; // Array of client sockets
    struct sockaddr_in server_addr;
    fd_set read_fds; // Set of socket descriptors
    char buffer[BUFFER_SIZE];
    char username[BUFFER_SIZE]; // Buffer to store the username
    struct timeval timeout;

    // Initialize all client_sockets[] to 0 (empty)
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = 0;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, 3) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Clear the socket set
        FD_ZERO(&read_fds);

        // Add master socket to set
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            // Socket descriptor
            sd = clients[i];

            // If valid socket descriptor then add to read list
            if (sd > 0) FD_SET(sd, &read_fds);

            // Highest file descriptor number, need it for the select function
            if (sd > max_fd) max_fd = sd;
        }

        // Timeout setting: 5 seconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Wait for an activity on one of the sockets, timeout is 5 seconds
        activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select");
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(sockfd, &read_fds)) {
            if ((new_socket = accept(sockfd, NULL, NULL)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Inform user of socket number
            printf("New connection, socket fd is %d\n", new_socket);

            // Receive the username from the client
            int username_len = recv(new_socket, username, sizeof(username) - 1, 0);
            if (username_len <= 0) {
                perror("recv username");
                close(new_socket);
                continue;
            }
            username[username_len] = '\0'; // Null-terminate the username
            printf("%s joined the chat room\n", username);

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                // If position is empty
                if (clients[i] == 0) {
                    clients[i] = new_socket;
                    printf("Adding to list of sockets at index %d\n", i);
                    break;
                }
            }
        }

        // Else, it's some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i];

            if (FD_ISSET(sd, &read_fds)) {
                // Check if it was for closing, and also read the incoming message
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Somebody disconnected, get their details and print
                    getpeername(sd, (struct sockaddr *)&server_addr, (socklen_t *)&server_addr);
                    printf("Host disconnected, socket fd %d\n", sd);

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    clients[i] = 0;
                } else {
                    // Null-terminate the buffer
                    buffer[valread] = '\0';

                    // Broadcast the message to other clients
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j] != 0 && clients[j] != sd) {
                            send(clients[j], buffer, strlen(buffer), 0);
                        }
                    }

                    printf("Received message: %s", buffer);
                }
            }
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}
