#define INPUT_SIZE 4
#define NTiles INPUT_SIZE / TILE

#include "tile.h"
#include <assert.h>

extern "C" {

Tile_T A[NTiles][NTiles], B[NTiles][NTiles], C[NTiles][NTiles];


// The HLS function
void matmul(Tile_T A[][NTiles], Tile_T B[][NTiles], Tile_T C[][NTiles]) {
  for (int i = 0; i < NTiles; i++) {
    for (int j = 0; j < NTiles; j++) {
        for (int k = 0; k < NTiles; k++) {
            Tile_T* a   = loadTile(&A[i][j]);
            Tile_T* b   = loadTile(&B[i][j]);
            Tile_T *mul = mulTile(a,b);
            storeTile(addTile(&C[i][j],mul),&C[i][j]);
            destroy(mul);
            destroy(a);
            destroy(b);
            }
        }
    }
  return;
}

int main(int argc, char const *argv[]) {

  assert(INPUT_SIZE%TILE == 0);
  // Initialize input matrix
  for (int i = 0; i < NTiles; i++) {
    for (int j = 0; j < NTiles; j++) {
      A[i][j].initrange();
      B[i][j].initrange();
      C[i][j].initval(0);
    }
  }

  // Invoke Kernel
  matmul(A, B, C);

  // Print result
  for (int i = 0; i < NTiles; i++) {
    for (int j = 0; j < NTiles; j++) {
      printf("[%d,%d]\n", i, j);
      C[i][j].print();
    }
  }

  return 0;
}
}
