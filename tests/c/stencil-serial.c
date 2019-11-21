#include <stdio.h>
/*#include <cilk/cilk.h>*/
#include <stdlib.h>

/*#include "roi.h"*/

#define NROWS (4)
#define NCOLS                                                                  \
  (4) // NCOLS must be power of 2 to avoid use of MOD operator (unsupported)
#define NBRROWS (1)
#define NBRCOLS (1)
#define NWORKERS (4)
#define NSTEPS (5)
#define FILTCOLS (1 + NBRROWS * 2)
#define FILTROWS (1 + NBRCOLS * 2)
#define UNSIGNED

void print(unsigned x[]) {
  printf("---------------------------------------------------------------------"
         "---------------------------\n");
  for (int i = 0; i < NROWS; i++) {
    for (int j = 0; j < NCOLS; j++) {
      printf("\t%d", x[i * NCOLS + j]);
    }
    printf("\n");
  }
}

#ifdef UNSIGNED
void stencil_inner(unsigned in[], unsigned out[], unsigned i, unsigned j,
                   unsigned nr) {
  for (unsigned nc = 0; nc <= 2 * NBRCOLS; nc++) {
    unsigned row = i + nr - NBRROWS;
    unsigned col = j + nc - NBRCOLS;
    if ((row < NROWS)) {
      if ((col < NCOLS)) {
        out[i * NCOLS + j] += in[row * NCOLS + col];
      }
    }
  }
}

void stencil(unsigned in[], unsigned out[]) {
  /*__app_roi_begin();*/
  for (unsigned pos = 0; pos < NROWS * NCOLS; pos++) {
    unsigned i = pos / NCOLS;
    unsigned j = pos & (NCOLS - 1);
    for (unsigned nr = 0; nr <= 2 * NBRROWS; nr++) {
      for (unsigned nc = 0; nc <= 2 * NBRCOLS; nc++) {
        unsigned row = i + nr - NBRROWS;
        unsigned col = j + nc - NBRCOLS;
        if ((row < NROWS)) {
          if ((col < NCOLS)) {
            out[i * NCOLS + j] += in[row * NCOLS + col];
          }
        }
      }
    }

    /*__app_roi_end();*/
    out[i * NCOLS + j] =
        (out[i * NCOLS + j] + (FILTROWS * FILTCOLS)) / (FILTROWS * FILTCOLS);
  }
}

#else
void stencil(unsigned in[], unsigned out[]) {
  cilk_for(int pos = 0; pos < NROWS * NCOLS; pos++) {
    int i = pos / NCOLS;
    int j = pos & (NCOLS - 1);
    for (int nr = -NBRROWS; nr <= NBRROWS; nr++) {
      for (int nc = -NBRCOLS; nc <= NBRCOLS; nc++) {
        int row = i + nr;
        int col = j + nc;
        if ((row >= 0) && (row < NROWS)) {
          if ((col >= 0) && (col < NCOLS)) {
            out[i * NCOLS + j] += in[row * NCOLS + col];
          }
        }
      }
    }
    out[i * NCOLS + j] =
        (out[i * NCOLS + j] + (FILTROWS * FILTCOLS)) / (FILTROWS * FILTCOLS);
  }
}
#endif

int main() {
  // initialize data
  unsigned buf0[NROWS * NCOLS];
  unsigned buf1[NROWS * NCOLS];
  for (int i = 0; i < NROWS; i++) {
    for (int j = 0; j < NCOLS; j++) {
      buf0[i * NCOLS + j] = rand() % 10;
      buf1[i * NCOLS + j] = 0;
    }
  }
  print(buf0);
  for (int i = 0; i < NSTEPS; i++) {
    if (i % 2)
      stencil(buf1, buf0);
    else
      stencil(buf0, buf1);
  }
  print(buf1);
  return 0;
}

