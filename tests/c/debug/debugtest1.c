#include <stdio.h> 
#define N 2
//complexity level 8/10

//store happens so loops are parallel
void debugtest1(int mat1[N][N], int mat2[N][N], int res[N][N] ) 
{ 
    int i, j, k; 
    for (i = 0; i < N; i++) 
    { 
        for (j = 0; j < N; j++) 
        { 
            res[i][j] = 0; 
            for (k = 0; k < N; k++) 
                res[i][j] += mat1[i][k]*mat2[k][j]; 
        } 
    } 
} 
  
int main() 
{ 
    int mat1[N][N]={0,1,2,3};
    int mat2[N][N]={1,4,2,5};
    int res[N][N]; // To store result 
    int i, j; 
    debugtest1(mat1, mat2, res); 
  
    printf("Result matrix is \n"); 
    for (i = 0; i < N; i++) 
    { 
        for (j = 0; j < N; j++) 
           printf("%d ", res[i][j]); 
        printf("\n"); 
    } 
  
    return 0; 
} 