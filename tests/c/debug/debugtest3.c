#include <stdio.h>
//complexity 2/10
// simple branch

int debugtest3(int a, int b){
    int res = 0;
    a = a *2;
    res = a/b;
    
    return res;
}

int main(){
    int a = 12;
    int b = 7;
    int res = debugtest3(a,b);
    return 0;
}