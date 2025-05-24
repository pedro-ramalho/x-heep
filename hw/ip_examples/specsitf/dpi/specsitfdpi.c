#include "specsitfdpi.h"
#include "backend/cipc_zmq.h"
#include <cipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#define SPECSITF_ADDRESS "tcp://localhost:5556"
#define SPECSITF_BUFFER_SIZE 1024

static cipc **specsitf_server = NULL;

void specsitf_comm_init() {
  specsitf_server = (cipc**)malloc(sizeof(cipc*));
  if (!specsitf_server) {
    printf("Failed to allocate memory for server!\n");

    exit(EXIT_FAILURE);
  }

  *specsitf_server = cipc_create(CIPC_PROTOCOL_ZMQ);
  if (!(*specsitf_server)) {
    printf("Failed to create server!\n");

    exit(EXIT_FAILURE);
  }

  cipc_zmq_config *config = cipc_zmq_config_req(SPECSITF_ADDRESS);

  if ((*specsitf_server)->init(&(*specsitf_server)->context, config) != CIPC_OK) {
    printf("Failed to initialize server!\n");

    specsitf_comm_free();

    exit(EXIT_FAILURE);
  }
}

void specsitf_comm_free() {
  if (specsitf_server) {
    if (*specsitf_server)
      cipc_free(*specsitf_server);

    free(specsitf_server);

    specsitf_server = NULL;
  }
}

void specsitf_comm_send(int reg1, int reg2, int ctrl) {
  char buffer[SPECSITF_BUFFER_SIZE] = {0};

  size_t length = snprintf(buffer, sizeof(buffer), "reg1=%d reg2=%d ctrl=%d", reg1, reg2, ctrl);

  if ((*specsitf_server)->send((*specsitf_server)->context, buffer, length) != CIPC_OK) {
    printf("Failed to send reply!\n");
  } else {
    printf("Sent: reg1=%d reg2=%d ctrl=%d\n", reg1, reg2, ctrl);
  }
}

void specsitf_comm_recv(int *reg3, int *st) {
  char buffer[SPECSITF_BUFFER_SIZE] = { 0 };

  if ((*specsitf_server)->recv((*specsitf_server)->context, buffer, sizeof(buffer)) != CIPC_OK)
    printf("Failed to receive message!\n");

  if (sscanf(buffer, "reg3=%d st=%d", reg3, st) != 2)
    printf("Failed to parse received data!\n");
}