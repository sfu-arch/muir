#include <stdio.h>

float test02(float a, float b){
    return a+b;
}

int main(){
    float a = 5.0f;
    float b = 3.4f;
    float sum = test01(a,b);
    printf("%f\n", sum);
    return 0;
}
