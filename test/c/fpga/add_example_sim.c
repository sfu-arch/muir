#include "address_map_arm_brl4.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>


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


pthread_t fpga_thread;
pthread_t host_thread;
pthread_mutex_t mem_lock;

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

void *create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_ANONYMOUS | MAP_SHARED;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

// void add_accel(unsigned int *a, unsigned int *b, unsigned *c)
void *add_accel(void *mem) {

  unsigned int arg0 = 0;
  unsigned int arg1 = 0;
  unsigned int result = 0;

  unsigned int *vptr_fpga = (unsigned int *)mem;

  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + (STAT_ADDRESS >> 2)) = *(vptr_fpga + (STAT_ADDRESS >> 2)) |
                                       ACC_READY; // Set status register busy
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Accel is ready\n");

  /*Step 1) Look for init bit to start read input registers*/

  /*Need to check if Accelerator is ready before seting start bit*/
  /*Polling ready bit until the value becomes equal to 1*/
  unsigned int init = 0;
  do {
    init = *(vptr_fpga)&ACC_INIT;
  } while (init != 2);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga) = *(vptr_fpga) ^ ACC_INIT; // unset init bit
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Accel is initializing arguments\n");

  arg0 = *(vptr_fpga + ACC_ARG0); // read Arg0 = 1
  arg1 = *(vptr_fpga + ACC_ARG1); // read Arg1 = 4

  printf("[LOG] accel -> arg0[%d] + arg1[%d]\n", arg0, arg1);

  // Step 2) Look for start bit to start computation
  unsigned int start = 0;
  do {
    start = *(vptr_fpga)&ACC_START;
  } while (start != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga) = *(vptr_fpga) ^ ACC_START; // Unset start bit
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Accel is started\n");

  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) | ACC_BUSY; // Set status register busy

  result = arg0 + arg1;
  *(vptr_fpga + (STAT_ADDRESS >> 2) + ACC_RET) = result;

  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) | DONE_MASK; // Set done bit

  return NULL;
}

void *host_exe(void *mem) {

  unsigned int *vptr_fpga = (unsigned int *)mem;
  // Start initializing argument values

  *(vptr_fpga + ACC_ARG0) = 1; // Set Arg0 = 1
  *(vptr_fpga + ACC_ARG1) = 4; // Set Arg1 = 4

  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga) = *(vptr_fpga) | ACC_INIT; // Set init bit
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Host is initializing the accel\n");

  // Need to check if Accelerator is ready before seting start bit
  // Polling ready bit until the value becomes equal to 1
  unsigned int ready = 0;
  do {
    ready = *(vptr_fpga + (STAT_ADDRESS >> 2)) & READY_MASK;
  } while (ready != 2);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) ^ READY_MASK;
  // Unset ready bit
  *(vptr_fpga) = *(vptr_fpga) | ACC_START; // Set start bit
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Host is seting start bit\n");

  // Start polling done bit
  unsigned int done = 0;
  do {
    done = *(vptr_fpga + (STAT_ADDRESS >> 2)) & DONE_MASK;
  } while (done != 1);
  pthread_mutex_lock(&mem_lock);
  *(vptr_fpga + (STAT_ADDRESS >> 2)) =
      *(vptr_fpga + (STAT_ADDRESS >> 2)) ^ DONE_MASK;
  // unset done mask
  pthread_mutex_unlock(&mem_lock);
  printf("[LOG] Host is done\n");

  // Check the result
  unsigned int accl_result = *(vptr_fpga + (STAT_ADDRESS >> 2) + ACC_RET);

  printf("[LOG] Accel result: %d\n", accl_result);

  return NULL;
}

int main(void) {

  // === FPGA PTR ===
  volatile unsigned int *vptr_fpga = NULL;

  vptr_fpga = (unsigned int *)create_shared_memory(1 << 12);

  // This part only exist to simulate FPGA
  if (pthread_mutex_init(&mem_lock, NULL) != 0) {
    printf("\n mutex init failed\n");
    return 1;
  }

  int err = 0;
  err = pthread_create(&fpga_thread, NULL, add_accel, (void *)vptr_fpga);
  if (err != 0)
    fprintf(stderr, "\ncan't create fpga thread: %d", err);

  err = pthread_create(&host_thread, NULL, host_exe, (void *)vptr_fpga);
  if (err != 0)
    fprintf(stderr, "\ncan't create host thread: %d", err);

  pthread_join(fpga_thread, NULL);
  pthread_join(host_thread, NULL);

  printf("[LOG] Finish execution\n");

  return 0;
}
