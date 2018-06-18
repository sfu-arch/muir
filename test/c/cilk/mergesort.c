#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#define LOOP_SIZE 1000000
#define TIME
#define N 100

double timespec_to_ms(struct timespec *ts)
{
  return ts->tv_sec*1000.0 + ts->tv_nsec/1000000.0;
}

typedef struct continue_struct {
  unsigned *A;
  unsigned *B;
  unsigned iBegin;
  unsigned iMiddle;
  unsigned iEnd;
} continue_t;

// Left source half is A[ iBegin:iMiddle-1].
// Right source half is A[iMiddle:iEnd-1   ].
// Result is            B[ iBegin:iEnd-1   ].
void mergesort_merge(continue_t *p)
{
  unsigned *A = p->A;
  unsigned *B = p->B;
  unsigned iBegin = p->iBegin;
  unsigned iMiddle = p->iMiddle;
  unsigned iEnd = p->iEnd;

  unsigned i = p->iBegin, j = p->iMiddle;
 
  // While there are elements in the left or right runs...
  for (unsigned k = iBegin; k < iEnd; k++) {
    // If left run head exists and is <= existing right run head.
    if (i < iMiddle && (j >= iEnd || A[i] <= A[j])) {
      B[k] = A[i];
      i = i + 1;
    } else {
      B[k] = A[j];
      j = j + 1;
    }
  }
}

// Sort the given run of array A[] using array B[] as a source.
// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
void mergesort(unsigned B[], unsigned iBegin, unsigned iEnd, unsigned A[])
{
  continue_t p;
  p.A = B; // Note: swap of A<->B is intentional
  p.B = A;
  p.iBegin = iBegin;
  p.iEnd = iEnd;
  
  if(iEnd - iBegin < 2)                     // if run size == 1
    return;                                 //   consider it sorted
  // split the run longer than 1 item into halves
  unsigned iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
  p.iMiddle = iMiddle;
  // recursively sort both runs from array A[] into B[]
  cilk_spawn mergesort(A, iBegin,  iMiddle, B);  // sort the left  run
  cilk_spawn mergesort(A, iMiddle,    iEnd, B);  // sort the right run
  cilk_sync;
  // merge the resulting runs from array B[] into A[]
  mergesort_merge(&p);
}

int main(int argc, char *argv[])
{
  unsigned n = N;
  unsigned i;
  unsigned* A;
  unsigned* B;

#ifdef TIME
  double time_ms;
  float *time_ns;
  float avg_time = 0.f;
  struct timespec start_time, end_time;
  time_ns = malloc(LOOP_SIZE * sizeof(float));
#endif

      // If we've got a parameter, assume it's the number of workers to be used
  if (argc > 1)
    {
      // Values less than 1, or parameters that aren't numbers aren't allowed
      if (atoi(argv[1]) < 1)
        {
	  printf("Usage: fib [workers]\n");
	  return 1;
        }

      // Set the number of workers to be used
#ifdef TIME      
      __cilkrts_set_param("nworkers", argv[1]);
#endif
    }

  A = (unsigned*) malloc(n * sizeof(unsigned));
  B = (unsigned*) malloc(n * sizeof(unsigned));

  for(int l = 0; l<LOOP_SIZE;l++) {
    
    srandom(368406);
    for (i=0; i<n; i++) {
      A[i] = random() %100 ;
      B[i] = A[i];
      A[i] = B[i]; // Both A & B should be in cache now
    }

#ifdef TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
#endif  

    mergesort(B, 0, n, A);   // sort data from B[] into A[]

#ifdef TIME
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    time_ms = timespec_to_ms(&end_time) - timespec_to_ms(&start_time);
    time_ns[l] = time_ms * 1000000;
    //    printf("%f\n", time_ns[l]);
#endif
  }

#ifdef TIME
  for(int l = 100; l<LOOP_SIZE;l++) {
    avg_time += time_ns[l];
  }
  avg_time /= (LOOP_SIZE-100);
  
  printf("Calculated in %.3f ns using %d workers.\n",
	 avg_time, __cilkrts_get_nworkers());
#endif

  free(B);
  free(A);
  return 0;
}
