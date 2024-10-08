#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Hàm tự tạo tương tự sock_ntop
const char *sock_ntop(struct sockaddr_in *addr) {
    static char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), str, INET_ADDRSTRLEN);
    return str;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address, client_address;
    socklen_t addrlen = sizeof(client_address);
    char buffer[BUFFER_SIZE] = {0};

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Gắn socket với địa chỉ và port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Đang chờ kết nối...\n");

    // Chấp nhận kết nối từ client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, &addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Nhận thông điệp từ client
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Nhận được thông điệp từ client: %s\n", buffer);

    // Lấy địa chỉ IP của client
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Client kết nối từ IP: %s\n", client_ip);

    // Gửi lại địa chỉ IP của client cho client
    send(new_socket, client_ip, strlen(client_ip), 0);
    printf("Đã gửi địa chỉ IP của client: %s\n", client_ip);

    close(new_socket);
    close(server_fd);
    return 0;
}
