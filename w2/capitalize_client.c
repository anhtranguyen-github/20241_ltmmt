#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP từ text sang binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address or address not supported\n");
        return -1;
    }

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    // Nhập chuỗi từ người dùng
    printf("Enter a string: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0;  // Xóa ký tự newline

    // Gửi chuỗi đến server
    send(sock, buffer, strlen(buffer), 0);
    printf("Message sent to server\n");

    // Nhận phản hồi từ server
    read(sock, buffer, BUFFER_SIZE);
    printf("Capitalized message from server: %s\n", buffer);

    // Đóng socket
    close(sock);

    return 0;
}