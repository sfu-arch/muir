#include <stdio.h>

<<<<<<< HEAD
unsigned test01(unsigned a, unsigned b){
    return  a * b;
}

int main(){
    unsigned a = 5;
    unsigned b = 3;
    unsigned prod;
=======
void* test01(int a, int b){
    return  Mat(&a , &b);
}

int main(){
    int a = 5;
    int b = 3;

    void* sum = test01(a,b);
>>>>>>> origin/newLib

    prod = test01(a,b);
    printf("prod=%u\n", prod);
    return 0;
}
