#include <stdio.h>
#include <string.h> // memcpy

#define DATA_SIZE 24

void test13(float *a, float *b, int n) {
  for (int k = 0; k < n; k++) {
    b[k] = a[k] * 2;
  }
}

int main() {
  float a[DATA_SIZE];
  float b[DATA_SIZE];
  float sum = 0.0;
  for (int i = 0; i < DATA_SIZE; i++) {
    a[i] = (float)i;
    sum += (float)i;
    b[i] = 0;
  }
  for (int i = 0; i < DATA_SIZE; i++) {
    unsigned int ui;
    memcpy(&ui, &a[i], sizeof(ui));
    printf("0x%x, ", ui);
  }
  printf("\n");
  unsigned int ui;
  float mean = (float)(sum / DATA_SIZE);
  memcpy(&ui, &mean, sizeof(ui));
  printf("Mean: 0x%x \n", ui);
  test13(a, b, DATA_SIZE);
  for (int i = 0; i < DATA_SIZE; i++) {
    unsigned int ui;
    memcpy(&ui, &a[i], sizeof(ui));
    printf("0x%x, ", ui);
  }
  return (0);
}
