#include "address_map_arm_brl4.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define STAT_ADDRESS 0x00000800
#define CNT_ADDRESS 0x00000000
#define ACC_INIT 0x00000002
#define ACC_START 0x00000001

#define ACC_ARG0 = 0x8
#define ACC_ARG1 = 0xC
#define ACC_RET = 0x8

#define DONE_MASK = 0x00000001

#define CNT_ADDRESS 0x00000000
#define CNT_ADDRESS 0x00000000
#define CNT_ADDRESS 0x00000000

/*
 *
 *    ------------
 *   |            |
 *   |            |
 *   |            |
 *   |            |
 *   |            |
 *   |            |
 *   |            |
 *   |------------|
 *   |   | I | S  |
 *    ------------
 *
 */

void add_accel(unsigned int *a, unsigned int *b, unsigned *c) { *c = *a + *b; }

int main(void) {
  // === FPGA PTR ===
  volatile unsigned int *vptr_fpga = NULL;

  // === get FPGA addresses ===
  // Open /dev/mem
  if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
    printf("ERROR: could not open \"/dev/mem\"...\n");
    return (1);
  }

  // Get a virtual pointer to 4k window at FPGA address.
  vptr_fpga = mmap(NULL, (1 << 12), (PROT_READ | PROT_WRITE), MAP_SHARED, fd,
                   SDRAM_BASE);

  if (vptr_fpga == MAP_FAILED) {
    printf("ERROR: mmap() failed...\n");
    close(fd);
    return (1);
  }

  // Start initializing argument values
  *(vptr_fpga) = ACC_INIT; // Set init bit

  *(vptr_fpga + ACC_ARG0) = 1; // Set Arg0 = 1
  *(vptr_fpga + ACC_ARG1) = 4; // Set Arg1 = 4

  *(vptr_fpga) = ACC_START; // Set start bit

  // Start polling done bit
  unsigned int done = 0;
  do {
    done = *(vptr_fpga + STAT_ADDRESS) & DONE_MASK;
  } while (done != 1);

  // Check the result
  unsigned int accl_result = *(vptr_fpga + STAT_ADDRESS + ACC_RET);

  printf("Accel result: %d\n", accl_result);

  return 0;
}
