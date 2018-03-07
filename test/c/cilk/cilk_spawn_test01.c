#include <stdio.h>
#include <cilk/cilk.h>

int cilk_spawn_test01(int a, int b) {
  int x, y;
  cilk_spawn {
    x = a + 5;
  }
  y = b + 5;
  cilk_sync;
  return x + y;
}

int main() {
  int a = 5;
  int b = 8;
  int c;
  c = cilk_spawn_test01(a,b);
  printf("c=%d\n", c);

}
