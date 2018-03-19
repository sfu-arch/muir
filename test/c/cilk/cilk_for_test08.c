#include <stdio.h>
#include <cilk/cilk.h>

const int rgb2xyz[3][3] = {{27030,23434, 11825},
			   {13937,46868,  4730},
			   { 1267, 7811, 62279}};

int cilk_for_test08(int rgb[][3], int xyz[][3]) {
  int r,g,b;
  cilk_for (int i = 0; i < 8; ++i) {
    r = rgb[i][0];
    g = rgb[i][1];
    b = rgb[i][2];
    xyz[i][0] = (rgb2xyz[0][0]*r + rgb2xyz[0][1]*g + rgb2xyz[0][2]*b) >> 16;
    xyz[i][1] = (rgb2xyz[1][0]*r + rgb2xyz[1][1]*g + rgb2xyz[1][2]*b) >> 16;
    xyz[i][2] = (rgb2xyz[2][0]*r + rgb2xyz[2][1]*g + rgb2xyz[2][2]*b) >> 16;
  }
  return 1;
}

int main() {
  int i;
  int rgb[8][3] = {{1,1,1},     {0,10,3},   {0,2,5}, 
		  {255,192,32}, {52,71,98}, {31,27,99},
		  {12,77,52},   {128,7,7}};
  int xyz[8][3] = {0};
  cilk_for_test08(rgb,xyz);
  for(i=0;i<8;i++) {
    printf("%d %d %d\n", xyz[i][0], xyz[i][1],xyz[i][2]);
  }

}

