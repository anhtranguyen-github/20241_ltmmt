#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Gán địa chỉ IP và cổng
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Liên kết socket với địa chỉ và cổng
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Chấp nhận kết nối từ client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Đọc dữ liệu từ client
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Received from client: %s\n", buffer);

    // Chuyển đổi xâu thành chữ hoa
    for (int i = 0; buffer[i]; i++) {
        buffer[i] = toupper(buffer[i]);
    }

    // Gửi phản hồi cho client
    send(new_socket, buffer, strlen(buffer), 0);
    printf("Capitalized message sent to client: %s\n", buffer);

    // Đóng kết nối
    close(new_socket);
    close(server_fd);

    return 0;
}