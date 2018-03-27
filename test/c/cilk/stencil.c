#include <stdio.h>
#include <cilk/cilk.h>
#include <stdlib.h>

#define NROWS (4)
#define NCOLS (4)
#define NBRROWS (1)
#define NBRCOLS (1)
#define NWORKERS (4)
#define NSTEPS (5)
#define FILTCOLS (1 + NBRROWS * 2)
#define FILTROWS (1 + NBRCOLS * 2)


void print (int x[]) {
  printf("------------------------------------------------------------------------------------------------\n");
  for (int i = 0; i < NROWS; i ++ ) {
    for (int j = 0; j < NCOLS; j ++) {
      printf("\t%d", x[i * NCOLS + j]);
    }
    printf("\n");
  }
}


void stencil (int in[], int out[]) {
  cilk_for (int pos = 0; pos < NROWS * NCOLS; pos ++) {
    int i = pos / NCOLS;
    int j = pos % NCOLS;
    for (int nr = -NBRROWS; nr <= NBRROWS; nr ++) {
      for (int nc = -NBRCOLS; nc <= NBRCOLS; nc ++) {
	int row = i + nr;
	int col = j + nc;
	if ((row >= 0) && (row < NROWS)) {
	  if ((col >=0) && (col < NCOLS)) {
	    out[i * NCOLS + j] += in[row * NCOLS + col];
	  }
	}
      }
    }
    out[i * NCOLS + j] = (out[i * NCOLS + j] + (FILTROWS * FILTCOLS)) /
      (FILTROWS * FILTCOLS);
  }
}


int main () {
  //initialize data
  int buf0[NROWS * NCOLS];
  int buf1[NROWS * NCOLS];
  for (int i = 0; i < NROWS; i++) {
    for (int j = 0; j < NCOLS; j ++) {
      buf0[i * NCOLS + j] = rand() % 10;
      buf1[i * NCOLS + j] = 0;
    }
  }
  print(buf0);
  for (int i = 0; i < NSTEPS; i ++) {
    if (i % 2) stencil(buf1, buf0);
    else stencil(buf0, buf1);
  }
  return 0;
}




