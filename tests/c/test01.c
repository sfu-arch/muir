#include <stdio.h>

unsigned test01(unsigned a, unsigned b){
    return  a * b;
}

int main(){
    unsigned a = 5;
    unsigned b = 3;
    unsigned prod;

    prod = test01(a,b);
    printf("prod=%u\n", prod);
    return 0;
}
