#include "address_map_arm_brl4.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define FPGA
#define TIME

#ifdef FPGA
#include <pthread.h>
pthread_t fpga_thread;
pthread_mutex_t mem_lock;
#endif

#define STAT_ADDRESS 0x00000800
#define CNT_ADDRESS  0x00000000
#define ACC_INIT     0x00000002
#define ACC_START    0x00000001
#define ACC_BUSY     0x00000000
#define ACC_READY     0x00000002

#define ACC_ARG0  0x8
#define ACC_ARG1  0xC
#define ACC_RET   0x8

#define DONE_MASK  0x00000001
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

//void add_accel(unsigned int *a, unsigned int *b, unsigned *c) 
void *add_accel(unsigned int *vptr_fpga) 
{ 

  unsigned int arg0 = 0;
  unsigned int arg1 = 0;
  unsigned int result = 0;

  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + STAT_ADDRESS) = *(vptr_fpga + STAT_ADDRESS) | ACC_READY; // Set status register busy
  pthread_mutex_unlock(&mem_lock);

  // Step 1) Look for init bit to start read input registers

  // Need to check if Accelerator is ready before seting start bit
  // Polling ready bit until the value becomes equal to 1
  unsigned int init = 0;
  do {
    init = *(vptr_fpga) & ACC_INIT;
  }while(init != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga)  = *(vptr_fpga) ^ ACC_INIT;  //unset init bit
  pthread_mutex_unlock(&mem_lock);


  arg0 = *(vptr_fpga + ACC_ARG0); // read Arg0 = 1
  arg1 = *(vptr_fpga + ACC_ARG1); // read Arg1 = 4

  // Step 2) Look for start bit to start computation
  unsigned int start = 0;
  do {
    start = *(vptr_fpga) & ACC_START;
  }while(start != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga) = *(vptr_fpga) ^ ACC_START; //Unset start bit
  pthread_mutex_unlock(&mem_lock);

  *(vptr_fpga + STAT_ADDRESS) = *(vptr_fpga + STAT_ADDRESS) | ACC_BUSY; // Set status register busy

  result = arg0 + arg1;
  *(vptr_fpga + STAT_ADDRESS + ACC_RET) = result;

  *(vptr_fpga + STAT_ADDRESS) = *(vptr_fpga + STAT_ADDRESS) | DONE_MASK; //Set done bit

  return NULL;
}

int main(void) { 

  // === FPGA PTR ===
  volatile unsigned int *vptr_fpga = NULL;

  // === get FPGA addresses ===
  // Open /dev/mem
  int fd;
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

  //This part only exist to simulate FPGA
  #ifdef FPGA
  if (pthread_mutex_init(&mem_lock, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }

  int err = pthread_create(&fpga_thread, NULL, &add_accel, vptr_fpga);
  if (err != 0)
    printf("\ncan't create fpga thread:[%s]", strerror(err));

  pthread_join(fpga_thread, NULL);
  #endif

  // Start initializing argument values
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga) = *(vptr_fpga) | ACC_INIT; // Set init bit
  pthread_mutex_unlock(&mem_lock);

  *(vptr_fpga + ACC_ARG0) = 1; // Set Arg0 = 1
  *(vptr_fpga + ACC_ARG1) = 4; // Set Arg1 = 4

  // Need to check if Accelerator is ready before seting start bit
  // Polling ready bit until the value becomes equal to 1
  unsigned int ready = 0;
  do {
    ready = *(vptr_fpga + STAT_ADDRESS) & READY_MASK;
  }while(ready != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + STAT_ADDRESS) = *(vptr_fpga + STAT_ADDRESS) ^ READY_MASK;  //Unset ready bit

  *(vptr_fpga) = *(vptr_fpga) | ACC_START; // Set start bit
  pthread_mutex_unlock(&mem_lock);

  // Start polling done bit
  unsigned int done = 0;
  do {
    done = *(vptr_fpga + STAT_ADDRESS) & DONE_MASK;
  } while (done != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + STAT_ADDRESS) = *(vptr_fpga + STAT_ADDRESS) ^ DONE_MASK; //unset done mask
  pthread_mutex_unlock(&mem_lock);

  // Check the result
  unsigned int accl_result = *(vptr_fpga + STAT_ADDRESS + ACC_RET);

  printf("Accel result: %d\n", accl_result);

  return 0;
}
