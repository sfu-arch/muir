#include <stdio.h>
#include <cilk/cilk.h>
#include <stdlib.h>


#define CHUNKSIZE (2)
#define N (CHUNKSIZE * 10)
#define QSIZE (100)
#define INVALID (-1)
#define EOT (-10) //end-of-tasks

int q[QSIZE];


void print (int x[]) {

	printf("------------------------------------------------------------------------------------------------\n");

	for (int i = 0; i < N; i ++ ) {
	  printf("\t%c", (char)x[i]);
	}
	printf("\n");

}//print

void S4 (int x[], int y[]) {


	unsigned iter = 0;
	
	
	while (q[iter % QSIZE] != EOT) {

		while (q[iter % QSIZE] == INVALID) {}

		unsigned pos = q[iter % QSIZE];
		y[pos] = x[pos];
		y[pos + 1] = x[pos + 1];

		q[iter % QSIZE] = INVALID;
		iter = iter + 1;

		//print(y);

	}//while not end-of-tasks


}//S4


void S3 (int chunk[], int pos, unsigned iter) {

	//compress with RLE
	chunk[1] = '2';


	//enqueue task of writing out
	q[iter % QSIZE] = pos;

}//S3


void S2 (int x[], int pos, unsigned iter) {

	int* chunk = &x[pos];


	//check for duplicates
	int is_dup = chunk[0] == chunk[1]? 1: 0;

	if (is_dup) cilk_spawn S3 (chunk, pos, iter);
	else {
		q[iter % QSIZE] = pos;
	}

}//S1



void dedup (int x[], int y[]) {

	int pos = 0;
	int outpos = 0;
	int done = 0;
	unsigned iter = 0;

	
	cilk_spawn S4 (x, y);

	while (x[pos] != 0) {
		

		cilk_spawn S2 (x, pos, iter);
	
		pos += CHUNKSIZE;

		iter ++;

	}//while not end of buffer

	q[iter % QSIZE] = EOT;

	

}//dedup













int main () {

	int a[N + 1];
	int out[N];

	a[0] = a[1] = 'a';
	for (int i = 2; i < N; i++) {
		a[i] = rand() % ('z' - 'a') + 'a' ;
	}

	a[N] = 0;

	for (int i = 0; i < QSIZE; i++) {
		q[i] = INVALID;
	}

	print (a);
	
	dedup (a, out);


	print (out);



	return 0;
}//main




