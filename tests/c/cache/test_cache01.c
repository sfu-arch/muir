#include <stdio.h>

unsigned test_cache01(unsigned *a, unsigned *b, unsigned read) {

  unsigned s1 = 1;
  unsigned s2 = 2;
  unsigned s3 = 3;
  unsigned s4 = 4;
  unsigned s5 = 5;
  unsigned s6 = 6;
  unsigned s7 = 7;
  unsigned s8 = 8;

  if (read) {
    s1 = 2 * a[0];
    s2 = 4 * a[1];
    s3 = 6 * a[2];
    s4 = 8 * a[3];

    s5 = 2 * b[0];
    s6 = 4 * b[1];
    s7 = 6 * b[2];
    s8 = 8 * b[3];
  } else {
    a[0] = s1;
    a[1] = s2;
    a[3] = s3;
    a[4] = s4;

    b[0] = s5;
    b[1] = s6;
    b[2] = s7;
    b[3] = s8;
  }
  return s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8;
}

int main(int argc, char **argv) {
  unsigned a[4] = {10, 20, 30, 40};
  unsigned b[4] = {50, 60, 70, 80};
  printf("Result: %d\n", test_cache01(a, b, 1));
  for (unsigned i = 0; i < 4; i++) 
    printf("%d, ", a[i]);
  for (unsigned i = 0; i < 4; i++)
    printf("%d, ", b[i]);
  /*printf("a[%d] = %u\n",i, a[i]);*/
  return (0);
}
