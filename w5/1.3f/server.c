#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1000
#define MAX_CLIENTS 10
#define MAX_NAMELEN 20

int sockfd;
int clients[MAX_CLIENTS];
char client_usernames[MAX_CLIENTS][MAX_NAMELEN];
struct sockaddr_in server_addr;
char buffer[BUFFER_SIZE];

// Initialize client arrays
void initialize_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = 0;
        memset(client_usernames[i], 0, MAX_NAMELEN);
    }
}

// Make socket non-blocking
void set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

// Send message to all clients except the sender
void broadcast_message(int sender, char *message) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0 && clients[i] != sender) {
            send(clients[i], message, strlen(message), 0);
        }
    }
}

// Handle a new client connection
void handle_new_connection() {
    int new_socket;
    char username[MAX_NAMELEN];
    
    if ((new_socket = accept(sockfd, NULL, NULL)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Set the new socket to non-blocking
    set_non_blocking(new_socket);

    // Add new socket to clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == 0) {
            clients[i] = new_socket;
            printf("New client connected, waiting for username...\n");
            break;
        }
    }
}

// Handle client disconnection
void handle_client_disconnection(int i) {
    printf("%s disconnected\n", client_usernames[i]);

    // Notify other clients
    char leave_message[BUFFER_SIZE];
    snprintf(leave_message, sizeof(leave_message), "%s left the chat room\n", client_usernames[i]);
    broadcast_message(clients[i], leave_message);

    // Close the socket and clear the client's entry
    close(clients[i]);
    clients[i] = 0;
    memset(client_usernames[i], 0, MAX_NAMELEN);
}

// Handle client messages
void handle_client_message(int i) {
    int valread;
    
    if ((valread = read(clients[i], buffer, BUFFER_SIZE)) == 0) {
        handle_client_disconnection(i);
    } else {
        buffer[valread] = '\0';
        printf("%s: %s", client_usernames[i], buffer);
        broadcast_message(clients[i], buffer);
    }
}

// Handle receiving the username
void handle_username(int i) {
    char username[MAX_NAMELEN];
    int username_len = recv(clients[i], username, sizeof(username) - 1, 0);
    
    if (username_len > 0) {
        username[username_len] = '\0';
        printf("%s joined the chat room\n", username);
        strncpy(client_usernames[i], username, MAX_NAMELEN - 1);

        // Broadcast join message to other clients
        char join_message[BUFFER_SIZE];
        snprintf(join_message, sizeof(join_message), "%s joined the chat room\n", username);
        broadcast_message(clients[i], join_message);
    } else if (username_len == 0) {
        // Client closed connection without sending a username
        handle_client_disconnection(i);
    }
}

// Server setup
void setup_server() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 3) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
}

// I/O Multiplexing using select()
void select_server() {
    fd_set read_fds;
    int max_fd, activity;

    while (1) {
        // Clear the socket set
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) FD_SET(clients[i], &read_fds);
            if (clients[i] > max_fd) max_fd = clients[i];
        }

        // Wait for activity on one of the sockets
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select");
        }

        // New connection
        if (FD_ISSET(sockfd, &read_fds)) {
            handle_new_connection();
        }

        // Check client sockets
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(clients[i], &read_fds)) {
                if (client_usernames[i][0] == 0) {
                    // Client hasn't sent a username yet
                    handle_username(i);
                } else {
                    // Client has already sent a username
                    handle_client_message(i);
                }
            }
        }
    }
}

// Main function
int main(int argc, char *argv[]) {
    initialize_clients();
    setup_server();

    // Choose one of the following I/O multiplexing methods
    if (argc > 1 && strcmp(argv[1], "select") == 0) {
        select_server();
    } else {
        printf("Usage: %s select\n", argv[0]);
    }

    // Clean up
    close(sockfd);
    return 0;
}
