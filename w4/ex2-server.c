#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define NUM_QUESTIONS 10
#define NUM_CHOICES 4

// Cấu trúc lưu trữ câu hỏi và các đáp án
typedef struct {
    char *question;
    char *choices[NUM_CHOICES];
    int correct_answer;  // Chỉ số của đáp án đúng
} Question;

// Mảng câu hỏi với các đáp án
Question questions[NUM_QUESTIONS] = {
    {
        "Đâu là ngôn ngữ lập trình phổ biến nhất?",
        {"A. Python", "B. C", "C. Java", "D. JavaScript"},
        0
    },
    {
        "Con trỏ trong C là gì?",
        {"A. Một biến", "B. Một hàm", "C. Một kiểu dữ liệu", "D. Một biến lưu địa chỉ bộ nhớ"},
        3
    },
    {
        "Hàm nào trong C để xuất ra màn hình?",
        {"A. printf()", "B. scanf()", "C. gets()", "D. puts()"},
        0
    },
    {
        "Đâu là cú pháp đúng để khai báo một biến nguyên trong C?",
        {"A. int a;", "B. integer a;", "C. float a;", "D. char a;"},
        0
    },
    {
        "Từ khóa nào dùng để định nghĩa một hằng số?",
        {"A. const", "B. static", "C. define", "D. var"},
        0
    },
    {
        "Vòng lặp nào không có điều kiện kiểm tra?",
        {"A. for", "B. while", "C. do-while", "D. if"},
        2
    },
    {
        "Đâu là từ khóa dùng để dừng vòng lặp?",
        {"A. continue", "B. stop", "C. break", "D. exit"},
        2
    },
    {
        "Từ khóa nào dùng để trả về giá trị trong hàm?",
        {"A. return", "B. break", "C. continue", "D. exit"},
        0
    },
    {
        "Hàm nào để lấy độ dài của chuỗi trong C?",
        {"A. strlen()", "B. sizeof()", "C. length()", "D. strlength()"},
        0
    },
    {
        "Kiểu dữ liệu nào trong C không có?",
        {"A. int", "B. string", "C. float", "D. char"},
        1
    }
};


// Hàm hoán đổi hai câu hỏi
void swap(Question *a, Question *b) {
    Question temp = *a;
    *a = *b;
    *b = temp;
}

// Hàm sắp xếp ngẫu nhiên các câu hỏi
void shuffleQuestions(Question *array, int n) {
    srand(time(0)); // Khởi tạo seed cho hàm rand()
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&array[i], &array[j]);
    }
}

// Hàm xử lý một client
void handle_client(int conn_fd) {
    char buffer[BUFFER_SIZE];
    int score = 0;    
    // Xáo trộn câu hỏi
    shuffleQuestions(questions, NUM_QUESTIONS);    
    // Gửi câu hỏi và nhận câu trả lời
    for (int i = 0; i < NUM_QUESTIONS; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, "%d. %s\n", i + 1, questions[i].question);        
        // Thêm các đáp án vào buffer
        for (int j = 0; j < NUM_CHOICES; j++) {
            sprintf(buffer + strlen(buffer), "%s\n", questions[i].choices[j]);
        }
        // Gửi câu hỏi tới client
        send(conn_fd, buffer, strlen(buffer), 0);
        // Nhận câu trả lời
        memset(buffer, 0, BUFFER_SIZE);
        recv(conn_fd, buffer, BUFFER_SIZE, 0);
        // Xử lý câu trả lời (chấp nhận A/a/1 là câu trả lời 1)
        char answer = tolower(buffer[0]);
        int answer_index;
        if (answer == 'a') answer_index = 0;
        else if (answer == 'b') answer_index = 1;
        else if (answer == 'c') answer_index = 2;
        else if (answer == 'd') answer_index = 3;
        else answer_index = -1; // Nếu không hợp lệ, bỏ qua
        // Phản hồi lại client về câu trả lời đúng hay sai
        memset(buffer, 0, BUFFER_SIZE);
        if (answer_index == questions[i].correct_answer) {
            sprintf(buffer, "Câu trả lời của bạn đúng!\n");
            score++;
        } else {
            sprintf(buffer, "Câu trả lời của bạn sai. Đáp án đúng là: %s\n",
                    questions[i].choices[questions[i].correct_answer]);
        }
        send(conn_fd, buffer, strlen(buffer), 0);
    }
    // Gửi kết quả cuối cùng cho client
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "Bạn đã trả lời đúng %d/%d câu hỏi.\n", score, NUM_QUESTIONS);
    send(conn_fd, buffer, strlen(buffer), 0);
    
    close(conn_fd);
}

int main() {
int listen_fd, conn_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    // Tạo socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket failed");
        exit(1);
    }

    // Đặt các thuộc tính cho địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket với địa chỉ server
    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(listen_fd);
        exit(1);
    }

    // Lắng nghe kết nối từ client
    if (listen(listen_fd, 5) == -1) {
        perror("Listen failed");
        close(listen_fd);
        exit(1);
    }

    printf("Server đang lắng nghe trên cổng %d...\n", PORT);

    while (1) {
        // Chấp nhận kết nối từ client
        if ((conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
            perror("Accept failed");
            continue;
        }

        // Tạo tiến trình con để xử lý client
        if (fork() == 0) {
            close(listen_fd); // Đóng listen_fd trong tiến trình con
            handle_client(conn_fd);
            exit(0);
        }

        // Đóng kết nối trong tiến trình cha
        close(conn_fd);
    }

    close(listen_fd);
    return 0;
}
