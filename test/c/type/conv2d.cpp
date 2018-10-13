#define INPUT_SIZE 4
//  Padding by two rows and columns to handle boundary conditions.
#define TILE INPUT_SIZE+2


#include "tile.h"
#include "vector.h"
#include <cassert>
Tile_T in;
Tile_T out;
int filter[3][3] =
{
    {5,5,5},
    {5,5,5},
    {5,5,5}
};


void conv2d(int image[][INPUT_SIZE+2], int outimage[][INPUT_SIZE+2], int convfilter[3][3]) {
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        /* code */
        for (size_t j = 0; j < INPUT_SIZE; j++) {

            Vec_T* filterrow0 = loadVec(&convfilter[0][0]);
            Vec_T* filterrow1 = loadVec(&convfilter[1][0]);
            Vec_T* filterrow2 = loadVec(&convfilter[2][0]);

            Vec_T* row0 = loadVec(&image[i-1][j]);
            Vec_T* row1 = loadVec(&image[i][j]);
            Vec_T* row2 = loadVec(&image[i+1][j]);

            int dotsum1 = redaddVec(mulVec(filterrow0,row0));
            int dotsum2 = redaddVec(mulVec(filterrow1,row1));
            int dotsum3 = redaddVec(mulVec(filterrow2,row2));

            outimage[i+1][j+1] = dotsum1 + dotsum2 + dotsum3;
        }
    }
}

int main(int argc, char const *argv[]) {
    in.initval(0);
    for (int i = 1; i <= INPUT_SIZE; i++)
        for (int j = 1; j <= INPUT_SIZE; j++)
            in.a[i][j] = 10;

    printf("Input image \n");
    in.print();
    conv2d(in.a,out.a,filter);
    printf("Output image \n");
    out.print();
    return 0;
}
