#include <stdio.h>

int test06(unsigned int a, unsigned int b){
    int sum[2];
    sum[0] = a;
    sum[1] = b;
    return sum[0] + sum[1];
}

int main(){
    unsigned a = 5;
    unsigned b = 3;
    unsigned sum = test06(a,b);
    printf("sum=%u\n", sum);
    return 0;
}
