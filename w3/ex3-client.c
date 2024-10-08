#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = "Xin chào từ client!";
    char buffer[BUFFER_SIZE] = {0};
    char server_ip[INET_ADDRSTRLEN];

    // Nhập địa chỉ IP của server từ người dùng
    printf("Nhập địa chỉ IP của server: ");
    scanf("%s", server_ip);

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Cấu hình địa chỉ server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP từ chuỗi sang dạng nhị phân
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Gửi thông điệp tới server
    send(sock, message, strlen(message), 0);
    printf("Thông điệp đã được gửi\n");

    // Nhận phản hồi từ server
    read(sock, buffer, BUFFER_SIZE);
    printf("Phản hồi từ server: %s\n", buffer);

    close(sock);
    return 0;
}
