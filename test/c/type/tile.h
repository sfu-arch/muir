#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

#define Tile(N)                                                                \
  typedef struct block {                                                       \
    int a[N][N];                                                               \
    void initrange() {                                                         \
      for (int i = 0; i < N; i++)                                              \
        for (int j = 0; j < N; j++)                                            \
          a[i][j] = i * N + j;                                                 \
    };                                                                         \
    void initval(int val) {                                                    \
      for (int i = 0; i < N; i++)                                              \
        for (int j = 0; j < N; j++)                                            \
          a[i][j] = val;                                                       \
    };                                                                         \
    void print() {                                                             \
      for (int i = 0; i < N; i++) {                                            \
        for (int j = 0; j < N; j++) {                                          \
          printf("%d \t", a[i][j]);                                            \
        }                                                                      \
        printf("\n");                                                          \
      }                                                                        \
    };                                                                         \
  } block

#define Tile_T block
#define Tile_Ptr block *

Tile(TILE);

// Blackbox operations
void destroy(Tile_T *node) { free(node); }
// Binary operations
Tile_T *addTile(Tile_T *left, Tile_T *right) {
  Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));
  for (int i = 0; i < TILE; i++)
    for (int j = 0; j < TILE; j++)
      m->a[i][j] = left->a[i][j] + right->a[i][j];
  return m;
}

Tile_T *subTile(Tile_T *left, Tile_T *right) {
  Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));
  for (int i = 0; i < TILE; i++)
    for (int j = 0; j < TILE; j++)
      m->a[i][j] = left->a[i][j] + right->a[i][j];
  return m;
}

Tile_T *mulTile(Tile_T *left, Tile_T *right) {
  Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));
  m->initval(0);
  for (int i = 0; i < TILE; i++)
    for (int j = 0; j < TILE; j++)
      for (int k = 0; k < TILE; k++)
        m->a[i][j] += left->a[i][k] * right->a[k][j];
  return m;
}

// loads and stores.
void storeTile(Tile_T *data, Tile_T *address) {

  for (int i = 0; i < TILE; i++)
    for (int j = 0; j < TILE; j++)
      address->a[i][j] = data->a[i][j];
  free(data);
  return;
}

Tile_T *loadTile(void *address) {
  Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));
  memcpy((void *)m, address, sizeof(Tile_T));
  return m;
}
}
