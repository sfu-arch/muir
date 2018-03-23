#include <stdio.h>
#include <cilk/cilk.h>

/*
  -- From PBBS breadthFirstSearch/graphData/unsymmetrizeEdgeArray.C

  parallel_for(long i=0;i<G.nonZeros;i++) {
    if(E[i].u > E[i].v) swap(E[i].u,E[i].v);
  }
 */

typedef struct Edge {
  int u;
  int v;
} edge_t;


__attribute__((always_inline))
inline void swap(int *a, int *b) {
  unsigned c;
  c = *a;
  *a = *b;
  *b = c;
}

int cilk_for_test11(edge_t *E, int n) {
  //int cilk_for_test11(int E[][2], int n) {
  cilk_for (int i = 0; i < n; ++i) {
    if (E[i].u > E[i].v) swap(&(E[i].u),&(E[i].v));
    //if (E[i][0] > E[i][1]) swap(&(E[i][0]),&(E[i][1]));
  }
  return 1;
}

int main() {

  edge_t E[]= {{.u = 0,.v = 1},
	      {.u = 4,.v = 2},
	      {.u = 1,.v = 2},
	      {.u = 3,.v = 7}};
  /*

  int E[4][2]= {{0,1},
	      {4,2},
	      {1,2},
	      {3,7}};
  */
  int result = cilk_for_test11(E,4);
  printf("%d\n",result);
}

