#include <stdio.h>
//complexity 6/10
// loop with loop carried dependency
int debugtest5(int a, int b){
    int sum = 0;
    int i;
    int n = 12;
    for(i = 0 ; i < n; i++){
        sum += (sum*b)+a;
    }
    return sum;
}

int main(){
    int a = 8;
    int b = 5;
    int sum = debugtest5(a,b);
    return 0;
}
