
#include <stdio.h>

int sum(int n)
{
  int x;
  if (n < 1)
    return n;
  x = sum(n-1)+n;
  return x;
}

int main(int argc, char *argv[])
{
  int n = 7;

  // Sum integers from 1 to n
  int result = sum(n);

  // Display our results
  printf("Sum #%d is %d.\n", n, result);

  return 0;
}
