#include <stdio.h>
#include <cilk/cilk.h>
#include <stdlib.h>


//Data Type
#define TYPE int

//Algorithm Parameters
#define row_size (4)
#define col_size (4)
#define N (row_size*col_size)
#define block_size (2)
#define NUMOFBLOCKS (N/block_size/block_size)

void print (int x[]) {
  printf("------------------------------------------------------------------------------------------------\n");
  for (int i = 0; i < row_size; i ++ ) {
    for (int j = 0; j < col_size; j ++) {
      printf("\t%d", x[i * col_size + j]);
    }
    printf("\n");
  }
}


void bgemm(int m1[], int m2[], int prod[]){
    
    int i_row, k_row;
    TYPE temp_x, mul;

    loopjj:cilk_for (int jj = 0; jj < row_size; jj += block_size){
        loopkk:cilk_for (int kk = 0; kk < row_size; kk += block_size){
            loopi:cilk_for ( int i = 0; i < row_size; ++i){
		//printf("\nworker id = %d", __cilkrts_get_worker_number ());
                loopk:for (int k = 0; k < block_size; ++k){
                    i_row = i * row_size;
                    k_row = (k  + kk) * row_size;
                    temp_x = m1[i_row + k + kk];
                    loopj:for (int j = 0; j < block_size; ++j){
                        mul = temp_x * m2[k_row + j + jj];
                        prod[i_row + j + jj] += mul;
                    }
                }
            }
        }
    }
}







int main () {

	int a[N], b[N], c[N];

	for (int i = 0; i < N; i++) {

		a[i] = rand() % 10;
		b[i] = rand() % 10;		
		c[i] = 0;

	}

	print (a);

	print (b);
	bgemm(a, b, c);

	print(c);


	return 0;
}




