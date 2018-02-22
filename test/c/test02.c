#include <stdio.h>

int test02(unsigned a, unsigned b){
    unsigned sum = 0;
    if(a / 2 == 4)
        sum = a + b;
    return sum;
}

int main(){
    unsigned a = 8;
    unsigned b = 3;
    unsigned sum = test02(a,b);
    printf("sum=%u\n", sum);
    return 0;
}
