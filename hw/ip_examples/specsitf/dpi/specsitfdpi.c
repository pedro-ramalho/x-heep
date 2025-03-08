
#include "specsitfdpi.h"

#ifdef __linux__
#include <pty.h>
#elif __APPLE__
#include <util.h>
#endif

/*#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>*/
#include <stdio.h>
/*#include <stdlib.h>
#include <string.h>
#include <unistd.h>*/

void test_specsitf(int data) {
  printf("Writing to file.\n");

  FILE *file = fopen("/home/pedro-ramalho/peripheral_output.txt", "a");

  if (file == NULL) {
    fprintf(stderr, "Error: could not open file for writing.\n");

    return;
  }

  fprintf(file, "Hello %d\n", data);

  fclose(file);
}
