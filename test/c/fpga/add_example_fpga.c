#include "address_map_arm_brl4.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define TIME

#define STAT_ADDRESS 0x00000800
#define CNT_ADDRESS 0x00000000
#define ACC_INIT 0x00000002
#define ACC_START 0x00000001
#define ACC_BUSY 0x00000000
#define ACC_READY 0x00000002

#define ACC_ARG0 0x8
#define ACC_ARG1 0xC
#define ACC_RET 0x8

#define DONE_MASK 0x00000001
#define READY_MASK 0x00000002

#define CNT_ADDRESS 0x00000000
#define CNT_ADDRESS 0x00000000
#define CNT_ADDRESS 0x00000000

/*
 *
 *              ------------------
 *             |                  |
 *             |------------------|
 * 0xC0000010  |      stat(2)     |
 *             |------------------|
 * 0xC000000C  |      stat(1)     |
 *             |------------------|
 * 0xC0000008  |      stat(0)     |
 *             |------------------|
 * 0xC0000004  |        RFU       |
 *             |------------------|
 * 0xC0000800  |           | R | D|
 *             |------------------|
 *             |                  |
 *             |      Unused      |
 *             |                  |
 *             |------------------|
 * 0xC0000000  |      ctrl(2)     |
 *             |------------------|
 * 0xC000000C  |      ctrl(1)     |
 *             |------------------|
 * 0xC0000008  |      ctrl(0)     |
 *             |------------------|
 * 0xC0000004  |       RFU        |
 *             |------------------|
 * 0xC0000000  |           | I | S|
 *              ------------------
 */

int main(void) {

  // === get FPGA addresses ===
  // Open /dev/mem
  int fd;

  // === FPGA PTR ===
  volatile unsigned int *vptr_fpga = NULL;

  if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
    printf("ERROR: could not open \"/dev/mem\"...\n");
    return (1);
  }

  // Get a virtual pointer to 4k window at FPGA address.
  vptr_fpga =
      mmap(NULL, 4096, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, SDRAM_BASE);

  if (vptr_fpga == MAP_FAILED) {
    printf("ERROR: mmap() failed...\n");
    close(fd);
    return (1);
  }

  // Start initializing argument values
  *(vptr_fpga + ACC_ARG0) = 1; // Set Arg0 = 1
  *(vptr_fpga + ACC_ARG1) = 4; // Set Arg1 = 4

  *(vptr_fpga) = *(vptr_fpga) | ACC_INIT; // Set init bit

  printf("[LOG] Host is initializing the accel\n");

  // Need to check if Accelerator is ready before seting start bit
  // Polling ready bit until the value becomes equal to 1
  unsigned int ready = 0;
  do {
    ready = *(vptr_fpga + (STAT_ADDRESS >> 2)) & READY_MASK;
  } while (ready != 2);
  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) ^ READY_MASK;
  // Unset ready bit
  *(vptr_fpga) = *(vptr_fpga) | ACC_START; // Set start bit

  printf("[LOG] Host is seting start bit\n");

  // Start polling done bit
  unsigned int done = 0;
  do {
    done = *(vptr_fpga + (STAT_ADDRESS >> 2)) & DONE_MASK;
  } while (done != 1);
  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) ^ DONE_MASK;
  // unset done mask
  printf("[LOG] Host is done\n");

  // Check the result
  unsigned int accl_result = *(vptr_fpga + (STAT_ADDRESS >> 2) + ACC_RET);

  printf("[LOG] Accel result: %d\n", accl_result);

  printf("[LOG] Finish execution\n");

  return 0;
}
