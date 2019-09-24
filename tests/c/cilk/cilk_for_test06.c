#include <stdio.h>
#include <cilk/cilk.h>

int cilk_for_test06(int p1[][2], int p2[][2], int *d) {
  int x0,x1,y0,y1,dist;
  cilk_for (int i = 0; i < 9; ++i) {
    x0 = p1[i][0];
    y0 = p1[i][1];
    x1 = p2[i][0];
    y1 = p2[i][1];
    dist = (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
    d[i] = dist;
  }
  return 1;
}

int main() {
  int i;
  int p1[9][2] = {{0,0}, {0,1}, {0,2}, 
		  {1,0}, {1,1}, {1,2},
		  {2,0}, {2,1}, {2,2}};
  int p2[9][2] = {{7,5}, {1,1}, {3,5},
		  {2,1}, {9,2}, {4,3},
		  {3,3}, {0,0}, {6,2}};
  int d[9] = {0};
  cilk_for_test06(p1,p2,d);
  for(i=0;i<9;i++) {
    printf("%d\n", d[i]);
  }

}

