#include <stdio.h>

void test04(int a, int b){
while (a != b) {
    if (a > b)
      a -= b;
    else
      b -= a;
  } 
  return;
}

int main(){
    unsigned int a = 50;
    unsigned int b = 5;
    test04(a,b);
    return 0;
}
