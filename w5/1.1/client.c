#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 100
#define MAX_NAMELEN 20

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + MAX_NAMELEN + 3]; // Extra space for username and formatting
    char username[MAX_NAMELEN];

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

    
    send(sockfd, username, strlen(username), 0);
    printf("Username sent to the server: %s\n", username);
    
    
    while (1) {
        // Read input from the user
        printf("Enter message to send to server: ");
        fgets(buffer, sizeof(buffer), stdin);

        // Prepend username to the message
        snprintf(message, sizeof(message), "%s: %s", username, buffer);

        // Send the message with the username to the server
        send(sockfd, message, strlen(message), 0);
        
        // Optionally, you can receive response from the server (if implemented)
    }

    // Clean up
    close(sockfd);
    return 0;
}
