#define INPUT_SIZE 16
#define NTiles INPUT_SIZE / TILE

#include "tile.h"
#include <assert.h>


extern "C" {

Tile_T A[NTiles][NTiles], B[NTiles][NTiles], C[NTiles][NTiles];


// The HLS function
void matadd(Tile_T A[][NTiles], Tile_T B[][NTiles], Tile_T C[][NTiles]) {
  for (int i = 0; i < NTiles; i++) {
    for (int j = 0; j < NTiles; j++) {
      storeTile(addTile(&A[i][j], &B[i][j]), &C[i][j]);
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
    }
  }

  // Invoke Kernel
  matadd(A, B, C);

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
