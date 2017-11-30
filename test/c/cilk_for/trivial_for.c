#include <stdio.h>
#include <cilk/cilk.h>

unsigned trivial_for() {
  unsigned j = 0;
  //cilk_for (int i = 0; i < 5; ++i) {
  for (int i = 0; i < 5; ++i) {
    j=i-1;
  }
  return j;
}

int main() {
  trivial_for();
}
