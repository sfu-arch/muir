#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_SIZE 24

void test14_mul(unsigned *a, unsigned *b, unsigned *c) {
  for (int i = 0; i < DATA_SIZE; i++) {
    c[i] = a[i] * b[i];
  }
}

unsigned test14_reduce(unsigned *c) {
  int sum = 0;
  for (int i = 0; i < DATA_SIZE; i++) {
    sum += c[i];
  }
  return sum;
}

unsigned test14(unsigned *a, unsigned *b, unsigned *c) {
  int sum = 0;
  test14_mul(a, b, c);
  sum = test14_reduce(c);
  return sum;
}

int main() {

  /**
   * Random generator
   */
  srand(100);
  unsigned a[DATA_SIZE];
  unsigned b[DATA_SIZE];
  unsigned c[DATA_SIZE];
  for (int i = 0; i < DATA_SIZE; i++) {
    a[i] = (rand() % DATA_SIZE);
    b[i] = (rand() % DATA_SIZE);
    c[i] = 0;
  }
  unsigned sum = test14(a, b, c);
  printf("test14 output: %d\n", sum);
  return 0;
}
