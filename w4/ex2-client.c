#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char answer[10];

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP từ chuỗi thành định dạng nhị phân
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    // Kết nối đến server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Nhận câu hỏi từ server, gửi câu trả lời, và nhận kết quả
    for (int i = 0; i < 10; i++) {
        // Nhận câu hỏi từ server
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s", buffer); // In câu hỏi ra màn hình

        // Nhập câu trả lời từ người dùng
        printf("Nhập câu trả lời của bạn (A/B/C/D): ");
        fgets(answer, sizeof(answer), stdin);

        // Gửi câu trả lời đến server
        send(sock, answer, strlen(answer), 0);
    }

    // Nhận kết quả cuối cùng từ server
    memset(buffer, 0, BUFFER_SIZE);
    read(sock, buffer, BUFFER_SIZE);
    printf("\n%s\n", buffer); // In kết quả ra màn hình

    // Đóng kết nối
    close(sock);
    return 0;
}
