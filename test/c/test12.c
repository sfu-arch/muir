#include <stdio.h>

unsigned test12_inner(unsigned j) {
  unsigned foo = j;
  for(unsigned k = 0; k < 5; ++k){
     foo++;
  }
  return foo;
}

unsigned test12(unsigned j) {
  unsigned foo = j;
  for (unsigned i = 0; i < 5; ++i) {
    foo += test12_inner(foo);
  }
  return foo;
}

int main() {
  int result = test12(10);
  printf("%d\n",result);
}
