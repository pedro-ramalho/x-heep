
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#define SPECSITF_START_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + 0x06000)

#define ADDR_REG_OFFSET 0
#define CTRL_REG_OFFSET 1
#define STAT_REG_OFFSET 2

volatile static uint32_t *shm = (SPECSITF_START_ADDRESS);

static uint32_t test_addr __attribute__((aligned(4), section(".ram")));

int main(int argc, char *argv[]) {
  uint32_t expected_value = 42;

  /* set input address */
  shm[ADDR_REG_OFFSET] = (uint32_t)&test_addr;

  /* assert CTRL, invoke accelerator */
  shm[CTRL_REG_OFFSET] = 1;

  /* poll until completion */
  while (shm[STAT_REG_OFFSET] != 2);

  /* deassert CTRL */
  shm[CTRL_REG_OFFSET] = 0;

  if (expected_value == test_addr) 
    printf("%d,%d\n", expected_value, test_addr);

  return EXIT_SUCCESS;
}
