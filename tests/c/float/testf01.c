#include <stdio.h>

float testf01(float a, float b){
    float sum = 0.0f;
    if(a / 2.0f > 2.1f)
        sum = a + b;
    return sum;
}

int main(){
    float a = 5.0f;
    float b = 3.4f;
    float sum = testf01(a,b);
    printf("%f\n", sum);
    return 0;
}
