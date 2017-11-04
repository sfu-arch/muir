#include <stdio.h>

unsigned int_exp(unsigned m, unsigned exp ) {
  unsigned result = m;
  for (unsigned i = 0; i < exp; ++i) {
    m *= m;
  }
  return m;
}

int main() {
  unsigned f;
  f=int_exp(2,5);
}
