
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#define SPECSITF_START_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + 0x06000)
#define SPECSITF_SIZE 0x0100
#define SPECSITF_END_ADDRESS (SPECSITF_START_ADDRESS + SPECSITF_SIZE)

#define INPUT_REG1 0
#define INPUT_REG2 1
#define OUTPUT_REG1 2
#define CTRL_REG 3
#define ST_REG 4

volatile static uint32_t *shm = (SPECSITF_START_ADDRESS);

int main(int argc, char *argv[]) {
  shm[INPUT_REG1] = 2;
  shm[INPUT_REG2] = 2;

  shm[CTRL_REG] = 1;

  printf("BEFORE WHILE LOOP, ST_REG: %d\n", shm[ST_REG]);
  while ((shm[ST_REG] & 2) != 0) {
    printf("IN WHILE LOOP, ST_REG: %d\n", shm[ST_REG]);
  }

  printf("after while loop\n");
  uint32_t result = shm[OUTPUT_REG1];

  shm[CTRL_REG] = 0;

  printf("Result: %d\n", result);

  return EXIT_SUCCESS;
}