#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Vec_T int
#define Vec_T_ptr int*
#define sizevec sizeof(int)*VECN

void printVec(Vec_T* base) {
    for(int i = 0; i < VECN; i++) {
    printf("Vec: %d,",base[i]);
    }
}


// Blackbox operations
void destroy(Vec_T *node) { free(node); }
// Binary operations
Vec_T *addVec(Vec_T *left, Vec_T *right) {
  Vec_T *m = (Vec_T *)malloc(sizeof(Vec_T));
  for (int i = 0; i < VECN; i++)
      m[i] = left[i] + right[i];
  return m;
}

Vec_T *subVec(Vec_T *left, Vec_T *right) {
  Vec_T *m = (Vec_T *)malloc(sizevec);
  for (int i = 0; i < VECN; i++)
     m[i] = left[i] - right[i];
  return m;
}

Vec_T *mulVec(Vec_T *left, Vec_T *right) {
  Vec_T *m = (Vec_T *)malloc(sizevec);
  for (int i = 0; i < VECN; i++)
        m[i] = left[i] * right[i];
  return m;
}

int redaddVec(Vec_T *left) {
  int m = 0;
  for (int i = 0; i < VECN; i++)
        m += left[i];
  return m;
}

Vec_T *padleftVec(Vec_T *left) {
  Vec_T *m = (Vec_T *)malloc(sizevec);
  for (int i = 1; i < VECN; i++)
        m[i] = left[i-1];
  m[0] = 0;
  return m;
}

Vec_T *padrightVec(Vec_T *left) {
  Vec_T *m = (Vec_T *)malloc(sizevec);
for (int i = 0; i < VECN-1; i++)
        m[i] = left[i-1];
  m[VECN-1] = 0;
  return m;
}



// loads and stores.
void storeVec(Vec_T *data, Vec_T *address) {

  for (int i = 0; i < VECN; i++)
      address[i] = data[i];
  free(data);
  return;
}

Vec_T *loadVec(void *address) {
  Vec_T *m = (Vec_T *)malloc(sizevec);
  memcpy((void *)m, address, sizevec);
  return m;
}
