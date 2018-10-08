#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_SIZE 16
#define TILE 4
#define NTiles 4

#define Tile(N)            \
    typedef struct block { \
        int a[N][N];       \
    } block;

#define Tile_T block
#define Tile_Ptr block *

Tile(4);

Tile_T A[NTiles][NTiles], B[NTiles][NTiles], C[NTiles][NTiles];

Tile_T *addTile(Tile_T *left, Tile_T *right) {
    Tile_T *m = (Tile_T *)malloc(sizeof(Tile_T));

    for (int i = 0; i < NTiles; i++) {
    }
}

void storeTile(Tile_T *data, Tile_T *address) {
    // printf("Storing result");
    return;
}

void kernel(Tile_T A[][NTiles], Tile_T B[][NTiles], Tile_T C[][NTiles]) {
    for (int i = 0; i < INPUT_SIZE; i = i + NTiles) {
        for (int j = 0; j < INPUT_SIZE; j = j + NTiles) {
            // unsigned int left = (uintptr_t)(&A[i][j]);
            // unsigned int right = (uintptr_t)(&A[i][j]);
            storeTile(addTile(&A[i][j], &B[i][j]), &C[i][j]);
        }
    }
    return;
}

int main() {
    kernel(A, B, C);
    return 0;
}
