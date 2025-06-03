
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#define SPECSITF_START_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + 0x06000)

#define BUFFER_SIZE 4

#define ADDR_REG_OFFSET 0
#define CTRL_REG_OFFSET 1
#define STAT_REG_OFFSET 2

volatile static uint32_t *shm = (SPECSITF_START_ADDRESS);

int main(int argc, char *argv[]) {
  uint32_t expected_value = 42;

  static uint32_t buffer[BUFFER_SIZE];

  /* set input address */
  shm[ADDR_REG_OFFSET] = &buffer[0];

  /* assert CTRL, invoke accelerator */
  shm[CTRL_REG_OFFSET] = 1;

  /* poll until completion */
  while (shm[STAT_REG_OFFSET] != 2)
    ;

  /* deassert CTRL */
  shm[CTRL_REG_OFFSET] = 0;

  printf("%d,%d,%d,%d\n", buffer[0], buffer[1], buffer[2], buffer[3]);

  return EXIT_SUCCESS;
}
