#include <stdio.h>

int test06(unsigned a, unsigned b){
    unsigned alloc0[2];
    int alloc1[1];
    alloc0[0] = a;
    alloc0[1] = b;
    alloc1[0] = alloc0[0]+alloc0[1];
    return alloc1[0];
}

int main(){
    unsigned a = 5;
    unsigned b = 3;
    unsigned result = test06(a,b);
    printf("result=%u\n", result);
    return 0;
}
