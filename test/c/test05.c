#include <stdio.h>


unsigned test05(unsigned *a) {
  for (unsigned i = 0; i < 5; ++i) {
    a[i+5] = 2*a[i];
  }
  return a[9];
}

int main() {
  /*unsigned a[10] = {0,1,2,3,4,0,0,0,0,0};*/
  unsigned a[10] = {1,2,3,4,5,6,7,8,9,10};
  test05(a);
  printf("a[9] = %u\n", a[9]);
  return(0);
}
