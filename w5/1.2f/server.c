#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>



#define PORT 8080
#define BUFFER_SIZE 1000
#define MAX_CLIENTS 10
#define MAX_NAMELEN 20

int main() {
    int sockfd, client_sock, max_fd, activity, i, valread, sd, new_socket;
    int clients[MAX_CLIENTS]; // Array of client sockets
    char client_usernames[MAX_CLIENTS][MAX_NAMELEN]; // Array of client usernames
    struct sockaddr_in server_addr;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    char username[MAX_NAMELEN];
    struct timeval timeout;

    // Initialize all client_sockets[] and client_usernames[] to 0 (empty)
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = 0;
        memset(client_usernames[i], 0, MAX_NAMELEN);
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
            sd = clients[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_fd) max_fd = sd;
        }

        // Timeout setting: 5 seconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Wait for an activity on one of the sockets
        activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select");
        }

        // Incoming connection on master socket
        if (FD_ISSET(sockfd, &read_fds)) {
            if ((new_socket = accept(sockfd, NULL, NULL)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Receive the username from the client
            int username_len = recv(new_socket, username, sizeof(username) - 1, 0);
            if (username_len <= 0) {
                perror("recv username");
                close(new_socket);
                continue;
            }
            username[username_len] = '\0';
            printf("%s joined the chat room\n", username);

            // Add new socket to clients array
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = new_socket;
                    strncpy(client_usernames[i], username, MAX_NAMELEN - 1);
                    break;
                }
            }

            // Broadcast join message to other clients
            char join_message[BUFFER_SIZE];
            snprintf(join_message, sizeof(join_message), "%s joined the chat room\n", username);
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != 0 && clients[i] != new_socket) {
                    send(clients[i], join_message, strlen(join_message), 0);
                }
            }
        }

        // IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i];
            if (FD_ISSET(sd, &read_fds)) {
                // Check if it was for closing
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // A client disconnected
                    getpeername(sd, (struct sockaddr *)&server_addr, (socklen_t *)&server_addr);
                    printf("%s disconnected\n", client_usernames[i]);

                    // Notify other clients
                    char leave_message[BUFFER_SIZE];
                    snprintf(leave_message, sizeof(leave_message), "%s left the chat room\n", client_usernames[i]);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j] != 0 && clients[j] != sd) {
                            send(clients[j], leave_message, strlen(leave_message), 0);
                        }
                    }

                    // Close the socket and mark it as 0
                    close(sd);
                    clients[i] = 0;
                    memset(client_usernames[i], 0, MAX_NAMELEN);
                } else {
                    // Broadcast message to other clients
                    buffer[valread] = '\0';
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j] != 0 && clients[j] != sd) {
                            send(clients[j], buffer, strlen(buffer), 0);
                        }
                    }
                    printf("%s: %s", client_usernames[i], buffer);
                }
            }
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}
