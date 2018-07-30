#include <stdio.h>
/*#include <cilk/cilk.h>*/
#include <stdlib.h>
/*#include "roi.h"*/

#define N (3 * 5)
#define M (5)
#define MIN(X, Y) (X < Y? X : Y)
#define MAX(X, Y) (X > Y? X : Y) 


void print (int x[]) {

	printf("------------------------------------------------------------------------------------------------\n");

	for (int i = 0; i < N; i ++ ) {
		printf("\t%d", x[i]);
	}
	printf("\n");

}//print

void work (int x[], int taskid, int stage) {

	//printf("task = %d, stage = %d\n", taskid, stage);
	//printf("\nworker id = %d", __cilkrts_get_worker_number ());
	int idx = stage * 3;

	x[idx + 1] = (x[idx] + x[idx + 1] + x[idx + 2] + 2) / 3;
	x[idx + 2] = (x[idx + 1] + x[idx + 2] + x[idx + 3] + 2) / 3;
	x[idx + 3] = (x[idx + 2] + x[idx + 3] + x[idx + 4] + 2) / 3;

}//work



void sor (int x[], int ntasks, int size) {

    /*__app_roi_begin();*/
	int nstages = size / 3;
	

	for (int i = 0; i < nstages + ntasks - 1; i ++) {

		int laststage = MIN(i, nstages - 1);
		int firststage = MAX(laststage - i, 0);


		int firsttask = MIN(ntasks - 1, i - laststage);
		int lasttask = MIN(ntasks - 1, i - firststage);
		for(int taskid = firsttask; taskid <= lasttask; taskid ++) {

			

		
			int stage = i - taskid;
			int idx = stage * 3;

			x[idx + 1] = (x[idx] + x[idx + 1] + x[idx + 2] + 2) / 3;
			x[idx + 2] = (x[idx + 1] + x[idx + 2] + x[idx + 3] + 2) / 3;
			x[idx + 3] = (x[idx + 2] + x[idx + 3] + x[idx + 4] + 2) / 3;

			

		}
        /*__app_roi_end();*/
        //for each parallel stage

		//cilk_sync;
		//print (x);
		


	}//for each task and stage

}//sor




void sor_seq (int x[], int ntasks, int size) {

	for (int i = 0; i < ntasks; i ++) {

		for (int j = 0; j < size; j ++) {
			x[j+1] = (x[j] + x[j + 1] + x[j + 2] + 2) / 3; 

		}

		print (x);
	}//for each task

	


}//sor_seq





int main () {


	int a[N + 2];
	for (int i = 0; i < N; i++) {
		a[i] = rand() % 10;
	}
	a[N] = a[N + 1] = 0;

	print(a);
	//sor_seq(a, M, N);
	sor(a, M, N);
	print (a);


	return 0;
}




