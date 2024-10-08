#include <stdio.h>


typedef struct student {
    char name[20];
    int eng;
    int math;
    int phys;
    double mean; 
} STUDENT;

void calculate_mean(STUDENT *s) {
    s->mean = (s->eng + s->math + s->phys) / 3.0;
}

char get_grade(double mean) {
    if (mean >= 90) return 'S';
    else if (mean >= 80) return 'A';
    else if (mean >= 70) return 'B';
    else if (mean >= 60) return 'C';
    else return 'D';
}

int main() {
    
    STUDENT data[] = {
        {"Tuan", 82, 72, 58, 0.0},
        {"Nam", 77, 82, 79, 0.0},
        {"Khanh", 52, 62, 39, 0.0},
        {"Phuong", 61, 82, 88, 0.0}
    };

   
    STUDENT *p;
    int n = sizeof(data) / sizeof(data[0]);  


    for (p = data; p < data + n; p++) {
        calculate_mean(p);  
        printf("Sinh vien: %s\n", p->name);
        printf("Diem: Eng = %d, Math = %d, Phys = %d\n", p->eng, p->math, p->phys);
        printf("Diem trung binh: %.2f\n", p->mean);
        printf("Xep loai: %c\n", get_grade(p->mean));
        printf("\n");
    }

    return 0;
}
