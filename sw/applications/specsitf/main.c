
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#define SPECSITF_START_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + 0x06000)

#define WORD_SIZE 4

#define DATA_SIZE 8

#define READ_ADDR_REG_OFFSET 0
#define WRITE_ADDR_REG_OFFSET 1
#define THRESHOLD_REG_OFFSET 2
#define DATA_SIZE_REG_OFFSET 3
#define CTRL_REG_OFFSET 4
#define STAT_REG_OFFSET 5

volatile static uint32_t *shm = (SPECSITF_START_ADDRESS);

int main(int argc, char *argv[]) {
  static uint32_t source_data[DATA_SIZE] __attribute__ ((aligned(WORD_SIZE)));
  static uint32_t copied_data[DATA_SIZE] __attribute__ ((aligned(WORD_SIZE)));

  uint32_t threshold = 20;

  /* populate source buffer */
  for (size_t i = 0; i < DATA_SIZE; i++)
    source_data[i] = i & 0x1 ? i*3 : i*2;

  /* set accelerator inputs */
  shm[READ_ADDR_REG_OFFSET] = &source_data[0];
  shm[WRITE_ADDR_REG_OFFSET] = &copied_data[0];
  shm[THRESHOLD_REG_OFFSET] = threshold;
  shm[DATA_SIZE_REG_OFFSET] = DATA_SIZE;
  
  /* assert CTRL, invoke accelerator */
  shm[CTRL_REG_OFFSET] = 1;

  /* poll until completion */
  while (shm[STAT_REG_OFFSET] != 2);

  /* deassert CTRL */
  shm[CTRL_REG_OFFSET] = 0;

  /* verify results */
  uint32_t errors = 0;
  
  for (size_t i = 0; i < DATA_SIZE; i++) {
    uint32_t expected_data = source_data[i] > threshold ? source_data[i] : threshold;

    if (copied_data[i] != expected_data) {
      errors++;

      printf("copied_data[%d] is %d, expected %d", i, copied_data[i], expected_data);
    }
  }

  if (errors == 0) {
    printf("Simple Accelerator Successful\n\r");

    return EXIT_SUCCESS;
  } else {
    printf("Simple Accelerator failure: %d errors out of %d data checked\n\r", errors, DATA_SIZE);
  }

  return EXIT_SUCCESS;
}
