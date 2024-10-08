#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();  // Tạo tiến trình con

    if (pid < 0) {
        // Nếu fork() trả về giá trị âm, có lỗi xảy ra
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Tiến trình con
        printf("Tiến trình con đang chạy với PID: %d\n", getpid());
        printf("Tiến trình con kết thúc\n");
    } else {
        // Tiến trình cha
        //printf("Tiến trình cha với PID: %d đã tạo tiến trình con với PID: %d\n", getpid(), pid);
        //sleep(30);  // Ngủ trong 30 giây
        //printf("Tiến trình cha kết thúc mà không chờ tiến trình con\n");
        wait(NULL);  // Chờ tiến trình con kết thúc  
        printf("Tiến trình cha đã thu gom tiến trình zombie.\n"); 
    }

    return 0;
}
