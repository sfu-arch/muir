#include <stdio.h>
/*#include "roi.h"*/

int cilk_for_test06(int a[][5], int b[][5], int c[][5]) {
    /*__app_roi_begin();*/
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      c[i][j]=a[i][j]+b[i][j];
    }
  }
  /*__app_roi_end();*/

  return 1;
}

int main() {
  int i,j;
  int a[5][5] = {{ 1, 2, 3, 4, 5},
		 {11,12,13,14,15},
		 {21,22,23,24,25},
		 {31,32,33,34,35},
		 {41,42,43,44,45}};
  int b[5][5] = {{ 1, 2, 3, 4, 5},
		 {11,12,13,14,15},
		 {21,22,23,24,25},
		 {31,32,33,34,35},
		 {41,42,43,44,45}};
  int c[5][5] = {0};
  cilk_for_test06(a,b,c);
  for(i=0;i<5;i++) {
    for(j=0;j<5;j++) {
      printf("%d\n", c[i][j]);
    }
  }

}
