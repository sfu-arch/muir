#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_SIZE 2048

void vadd(unsigned *a, unsigned *b, unsigned *c) {
  for (unsigned i = 0; i < DATA_SIZE; ++i) {
    c[i] = a[i] + b[i];
  }
}


void init_array(unsigned *input){
    for(int i = 0; i < DATA_SIZE; i++){
        input[i] = rand() % DATA_SIZE;
    }
}

int main() {
  int i;
  unsigned a[DATA_SIZE] ;
  unsigned b[DATA_SIZE] ;
  unsigned c[DATA_SIZE] = {0};

  //Initlizing the arrays
  srand(time(0));
  init_array(a);
  init_array(b);

  vadd(a, b, c);

  for (i = 0; i < DATA_SIZE; i++) {
    printf("%d, ", c[i]);
  }
}
