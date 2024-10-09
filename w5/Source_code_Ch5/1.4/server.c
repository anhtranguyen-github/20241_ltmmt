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
#include <time.h>

#define PORT 8081
#define BUFFER_SIZE 1000
#define MAX_CLIENTS 10
#define MAX_NAMELEN 20

int sockfd;
int clients[MAX_CLIENTS];
char client_usernames[MAX_CLIENTS][MAX_NAMELEN];
struct sockaddr_in server_addr;
char buffer[BUFFER_SIZE];
volatile sig_atomic_t stop_server = 0; // Flag to stop server

// Signal handler to catch SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    stop_server = 1;  // Set flag to stop server
}

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

// Close all client connections
void close_all_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0) {
            close(clients[i]);
        }
    }
}

// I/O Multiplexing using select()
void select_server() {
    fd_set read_fds;
    int max_fd, activity;

    while (!stop_server) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) FD_SET(clients[i], &read_fds);
            if (clients[i] > max_fd) max_fd = clients[i];
        }

        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select");
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            handle_new_connection();
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(clients[i], &read_fds)) {
                if (client_usernames[i][0] == 0) {
                    handle_username(i);
                } else {
                    handle_client_message(i);
                }
            }
        }
    }
    close_all_clients();
}

// I/O Multiplexing using pselect()
void pselect_server() {
    fd_set read_fds;
    int max_fd, activity;
    struct timespec timeout = {5, 0};  // Timeout of 5 seconds
    sigset_t empty_mask;

    sigemptyset(&empty_mask);

    while (!stop_server) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_fd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) FD_SET(clients[i], &read_fds);
            if (clients[i] > max_fd) max_fd = clients[i];
        }

        activity = pselect(max_fd + 1, &read_fds, NULL, NULL, &timeout, &empty_mask);

        if ((activity < 0) && (errno != EINTR)) {
            perror("pselect");
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            handle_new_connection();
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(clients[i], &read_fds)) {
                if (client_usernames[i][0] == 0) {
                    handle_username(i);
                } else {
                    handle_client_message(i);
                }
            }
        }
    }
    close_all_clients();
}

// I/O Multiplexing using poll()
void poll_server() {
    struct pollfd poll_fds[MAX_CLIENTS + 1];
    int nfds;

    while (!stop_server) {
        poll_fds[0].fd = sockfd;
        poll_fds[0].events = POLLIN;
        nfds = 1;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != 0) {
                poll_fds[nfds].fd = clients[i];
                poll_fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        int poll_result = poll(poll_fds, nfds, -1);

        if (poll_result < 0 && errno != EINTR) {
            perror("poll");
        }

        if (poll_fds[0].revents & POLLIN) {
            handle_new_connection();
        }

        for (int i = 1; i < nfds; i++) {
            if (poll_fds[i].revents & POLLIN) {
                int client_index = i - 1;
                if (client_usernames[client_index][0] == 0) {
                    handle_username(client_index);
                } else {
                    handle_client_message(client_index);
                }
            }
        }
    }
    close_all_clients();
}

// Main function
int main(int argc, char *argv[]) {
    initialize_clients();
    setup_server();

    // Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    if (argc > 1 && strcmp(argv[1], "select") == 0) {
        select_server();
    } else if (argc > 1 && strcmp(argv[1], "pselect") == 0) {
        pselect_server();
    } else if (argc > 1 && strcmp(argv[1], "poll") == 0) {
        poll_server();
    } else {
        printf("Usage: %s [select|pselect|poll]\n", argv[0]);
    }

    close(sockfd);
    printf("Server closed.\n");
    return 0;
}
