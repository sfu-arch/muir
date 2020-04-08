#include <stdio.h>

#define DATA_SIZE 24

void test14(unsigned *a, unsigned n) {
  unsigned temp[DATA_SIZE];
  for (int i = 0; i < DATA_SIZE; i++)
      temp[i] = i;
  for (int i = 0; i < DATA_SIZE; i++)
      a[i] = temp[i] * n;


}

int main() {
  unsigned a[DATA_SIZE];
  for (int i = 0; i < DATA_SIZE; i++) {
    a[i] = 0;
  }
  test14(a, DATA_SIZE);
  printf("Test14 output: \n");
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("%d, " ,a[i]);
  }
  printf("\n");
  return 0;
}
