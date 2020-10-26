#include <stdio.h>
//complexity 6/10
// loop with loop carried dependency
int debugtest6(int a, int b, int n, int m){
    int sum = 0;
    for(int i = 0 ; i < n; i++){
      for(int j = 0; j < m; j++){
        sum += (sum*b)+a;
      }
    }
    return sum;
}

int main(){
    int a = 8;
    int b = 5;
    int n = 10;
    int m = 20;
    int sum = debugtest6(a,b,n,m);
    return 0;
}
