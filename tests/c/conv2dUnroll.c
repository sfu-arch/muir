// Copyright 2017 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
#include <stdio.h>
#include <stdint.h>

#define IMG_SIZE 10

static int32_t coeffs[IMG_SIZE/2 * IMG_SIZE/2] = {
  1, 4, 6, 4, 1,
  4,16,24,16, 4,
  6,24,36,24, 6,
  4,16,24,16, 4,
  1, 4, 6, 4, 1 };

static const int32_t check[IMG_SIZE * IMG_SIZE] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0, 22, 23, 24, 25, 26, 27,  0,  0,
  0,  0, 32, 33, 34, 35, 36, 37,  0,  0,
  0,  0, 42, 43, 44, 45, 46, 47,  0,  0,
  0,  0, 52, 53, 54, 55, 56, 57,  0,  0,
  0,  0, 62, 63, 64, 65, 66, 67,  0,  0,
  0,  0, 72, 73, 74, 75, 76, 77,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

static int32_t img_in[IMG_SIZE * IMG_SIZE];
static int32_t img_out[IMG_SIZE * IMG_SIZE];

void test_setup() {
  for (unsigned i = 0; i != IMG_SIZE*IMG_SIZE; i++)
    img_in[i] = i;
}

int test_check() {
    for(unsigned i = 0; i < IMG_SIZE; ++i){
        for(unsigned j = 0; j < IMG_SIZE; ++j){
            if(img_out[i*IMG_SIZE + j] != check[i*IMG_SIZE + j])
                return 0;
        }
    }
    return 1;
}

void conv2dUnroll(int32_t* mat, int32_t* res,
            const int32_t *coeffs,
            int W, int H, int K, uint32_t scf) {
  int R = K >> 1;
  int index = R * W;

  for(int j = R; j < H - R; j++) {
    for(int i = R; i < W - R; i++) {
      int index2 = index - R * W;
      int c = 0;
      int32_t val = 0;
      for(int y = 0; y <= 2*R; ++y) {
        val += coeffs[c++] * mat[index2 + i - 2];
        val += coeffs[c++] * mat[index2 + i -1];
        val += coeffs[c++] * mat[index2 + i];
        val += coeffs[c++] * mat[index2 + i + 1];
        val += coeffs[c++] * mat[index2 + i + 2];
        index2 += W;
      }
      res[index+i] = val >> scf;
    }
    index += W;
  }
}

int main(){
  test_setup();
  conv2dUnroll(img_in, img_out, coeffs, IMG_SIZE, IMG_SIZE, 5, 8);
  if(test_check())
      printf("OK!\n");
  else
      printf("WRONG!\n");

  return 0;

}
