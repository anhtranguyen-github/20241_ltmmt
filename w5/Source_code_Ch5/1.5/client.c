#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8081
#define BUFFER_SIZE 100
#define MAX_NAMELEN 20

int main() {
    int sockfd, max_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + MAX_NAMELEN + 3]; // Extra space for username and formatting
    char username[MAX_NAMELEN];
    fd_set read_fds; // For select()

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change this if needed

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    // Prompt the user to enter a username
    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);
    // Remove newline character from username if present
    username[strcspn(username, "\n")] = 0;

    // Send the username to the server
    send(sockfd, username, strlen(username), 0);
    printf("Username sent to the server: %s\n", username);

    // Set up for select() to monitor both stdin and the socket
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Monitor stdin (user input)
        FD_SET(sockfd, &read_fds);        // Monitor server socket

        // Determine the maximum file descriptor
        max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        // Use select() to wait for activity on either stdin or the socket
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break;
        }

        // Check if there is input from the user
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Read input from the user
            fgets(buffer, sizeof(buffer), stdin);

            // Prepend username to the message
            snprintf(message, sizeof(message), "%s: %s", username, buffer);

            // Send the message with the username to the server
            send(sockfd, message, strlen(message), 0);
        }

        // Check if there is a message from the server
        if (FD_ISSET(sockfd, &read_fds)) {
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                // Server closed the connection or error occurred
                if (bytes_received == 0) {
                    printf("Server disconnected.\n");
                } else {
                    perror("recv");
                }
                break;
            }
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("%s", buffer);
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}
