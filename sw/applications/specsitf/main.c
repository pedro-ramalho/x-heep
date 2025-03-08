#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#define SPECS_PERIPH_START_ADDRESS (EXT_PERIPHERAL_START_ADDRESS + 0x05000)
#define SPECS_PERIPH_SIZE 0x0100
#define SPECS_PERIPH_END_ADDRESS                                               \
  (SPECS_PERIPH_START_ADDRESS + SPECS_PERIPH_SIZE)

int main(int argc, char *argv[]) {
  /* write something to stdout */
  printf("hello world sw!\n");

  volatile static uint32_t *specs_cmem_ptr = (SPECS_PERIPH_START_ADDRESS);
  specs_cmem_ptr[0] = 0x02;

  return EXIT_SUCCESS;
}
