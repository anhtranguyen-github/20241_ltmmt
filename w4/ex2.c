#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

// Hàm hiển thị câu hỏi
void displayQuestions(Question *questions, int num) {
    for (int i = 0; i < num; i++) {
        printf("Câu %d: %s\n", i + 1, questions[i].question);
        for (int j = 0; j < NUM_CHOICES; j++) {
            printf("%s\n", questions[i].choices[j]);
        }
        printf("\n");
    }
}

void checkAnswer(Question *question, char answer) {
    int answer_index = answer - 'A'; // Chuyển đổi ký tự thành chỉ số (A -> 0, B -> 1, C -> 2, D -> 3)
    if (answer_index == question->correct_answer) {
        printf("Đáp án đúng!\n");
    } else {
        printf("Đáp án sai. Đáp án đúng là: %s\n", question->choices[question->correct_answer]);
    }
}

int main() {
    // Trộn ngẫu nhiên các câu hỏi
    shuffleQuestions(questions, NUM_QUESTIONS);

    // Hiển thị 10 câu hỏi đã trộn
    displayQuestions(questions, NUM_QUESTIONS);

    // Xử lý đáp án cho từng câu hỏi
    char answer;
    for (int i = 0; i < NUM_QUESTIONS; i++) {
        printf("Nhập đáp án cho câu %d (A/B/C/D): ", i + 1);
        scanf(" %c", &answer); // Thêm khoảng trắng trước %c để loại bỏ ký tự newline
        checkAnswer(&questions[i], answer);
        printf("\n");
    }

    return 0;
}