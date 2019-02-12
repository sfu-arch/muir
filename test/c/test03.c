#include <stdio.h>

int test03(unsigned int a, unsigned int b, int n){
    unsigned int sum = a;
    int i;
    for(i = 0 ; i < n; i++)
        sum = (sum+a)*b;
    return sum;
}

int main(){
    unsigned int a = 5;
    unsigned int b = 3;
    unsigned int sum = test03(a,b, 5);
    printf("sum=%u\n", sum);
    printf("sum=0x%.8X\n", sum);
    return 0;
}
