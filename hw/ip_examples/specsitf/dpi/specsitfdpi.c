#include "specsitfdpi.h"
#include <backend/cipc_zmq.h>
#include <cipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#define SPECSITF_ADDRESS "tcp://*:5555"                                                     
#define SPECSITF_BUFFER_SIZE 1024

static cipc **specsitf_server = NULL;

void specsitf_comm_init() {
  specsitf_server = (cipc**)malloc(sizeof(cipc*));
  if (!specsitf_server) {
    printf("Failed to allocate memory for server!\n");
    
    exit(1);
  }

  *specsitf_server = cipc_create(CIPC_PROTOCOL_ZMQ);
  if (!(*specsitf_server)) {
    printf("Failed to create server!\n");

    exit(1);
  }

  cipc_zmq_config *config = cipc_zmq_config_rep(SPECSITF_ADDRESS);
  if ((*specsitf_server)->init (&(*specsitf_server)->context, config) != CIPC_OK) {
    printf("Failed to initialize server!\n");
    
    specsitf_comm_free();

    exit(1);
  }

  printf("Communication initialized successfully.\n");
}


void specsitf_comm_free() {
  if (specsitf_server) {
    if (*specsitf_server) {
      cipc_free(*specsitf_server);
    }

    free(specsitf_server);
    
    specsitf_server = NULL;
  }
}

void specsitf_comm_send(int int_reg_1_i, int int_reg_2_i) {
  char buffer[SPECSITF_BUFFER_SIZE] = {0};
  size_t length = snprintf(buffer, sizeof(buffer), "%d %d", int_reg_1_i, int_reg_2_i);

  if ((*specsitf_server)->send( (*specsitf_server)->context, buffer, length) != CIPC_OK) {
    printf("Failed to send reply!\n");
  } else {
    printf("Sent: %d %d\n", int_reg_1_i, int_reg_2_i);
  }
} 

void specsitf_comm_recv(int *int_reg_1_o, int *int_reg_2_o) {
  char buffer[SPECSITF_BUFFER_SIZE] = {0};

  if ((*specsitf_server)->recv( (*specsitf_server)->context, buffer, sizeof(buffer) ) != CIPC_OK) {
    printf("Failed to receive message!\n");
  }
  
  if (sscanf(buffer, "%d %d", int_reg_1_o, int_reg_2_o) != 2) {
    printf("Failed to parse received data!\n");

    int_reg_1_o = 0;
    int_reg_2_o = 0;
  }
}
