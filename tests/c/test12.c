#include <stdio.h>

int test17(unsigned int a, unsigned int b, unsigned c){
    unsigned int sum = 0;
    if(a / 2 == 4){
            sum = a + b + c;
        if(a / 3 == 1)
            sum = a + b + sum * c;
        else
            sum = a * b + sum * c;
    }else{
            sum = a - b + c;
        if(a / 5 == 0)
            sum = a - b * sum + c;
        else
    sum = a * 12 * sum + c;
    }
    return sum;
}

int main(){
    unsigned int a = 8;
    unsigned int b = 3;
    unsigned int c = 6;
    unsigned int sum = test17(a,b,c);
    printf("Result: %d\n", sum);
    return 0;
}
