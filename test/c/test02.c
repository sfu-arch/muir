#include <stdio.h>

int test02(unsigned int a, unsigned int b){
    unsigned int sum = 0;
    if(a / 2 == 4)
        sum = a + b;
    return sum;
}

int main(){
    unsigned int a = 5;
    unsigned int b = 3;
    unsigned int sum = test02(a,b);
    printf("Result: %d\n", sum);
    return 0;
}
