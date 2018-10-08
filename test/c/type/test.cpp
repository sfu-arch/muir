#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_SIZE 16
#define TILE 4
#define NTiles INPUT_SIZE/TILE

#define Tile(N) \
typedef struct block {                                                  \
    int a[N][N];                                                               \
    void initrange() {                                                         \
      for (int i = 0; i < N; i++)                                           \
        for (int j = 0; j < N; j++)                                         \
          a[i][j] = i*N+j;                                                  \
    };      \
    void print() {                                                             \
    for (int i = 0; i < N; i++) {                                           \
      for (int j = 0; j < N; j++) {                                         \
        printf("%d,",a[i][j]);                                                 \
      }                                                                        \
      printf("\n");                                                            \
    }                                                                         \
} ;                                                                          \
} block;

#define Tile_T block
#define Tile_Ptr block *

Tile(TILE)

    Tile_T A[NTiles][NTiles],
    B[NTiles][NTiles], C[NTiles][NTiles];



Tile_T *addTile(Tile_T *left, Tile_T *right) {
  Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));
  for (int i = 0; i < TILE; i++)
    for (int j = 0; j < TILE; j++)
      m->a[i][j] = left->a[i][j] + right->a[i][j];
  return m;
}


void storeTile(Tile_T *data, Tile_T *address) {

  for (int i=0; i < TILE; i++)
    for(int j=0; j < TILE; j++)
      address->a[i][j] = data->a[i][j];
  free(data);
  return;
}

void kernel(Tile_T A[][NTiles], Tile_T B[][NTiles], Tile_T C[][NTiles]) {
  for (int i = 0; i < NTiles; i++) {
    for (int j = 0; j < NTiles; j++) {
      storeTile(addTile(&A[i][j], &B[i][j]), &C[i][j]);
    }
  }
  return;
}

int main() {

    for (int i = 0; i < NTiles; i++) {
      for (int j = 0; j < NTiles; j++) {
        A[i][j].initrange();
        B[i][j].initrange();
      }
    }

    kernel(A,B,C);

    for (int i = 0; i < NTiles; i++) {
      for (int j = 0; j < NTiles; j++) {
        C[i][j].print();
      }
    }
    return 0;
}
