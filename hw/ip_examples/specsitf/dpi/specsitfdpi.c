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
  specsitf_server = (cipc **)malloc(sizeof(cipc *));
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

  if ((*specsitf_server)->init(&(*specsitf_server)->context, config) !=
      CIPC_OK) {
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

void specsitf_comm_send(int addr, int ctrl, int obi_gnt, int obi_rvalid, int obi_rdata) {
    char buffer[SPECSITF_BUFFER_SIZE] = {0};
    size_t length = snprintf(buffer, sizeof(buffer),
                             "addr=%d ctrl=%d obi_gnt=%d obi_rvalid=%d obi_rdata=%d",
                             addr, ctrl, obi_gnt, obi_rvalid, obi_rdata);

    if ((*specsitf_server)->send((*specsitf_server)->context, buffer, length) != CIPC_OK) {
        printf("DPI-C Send: Failed to send reply!\n");
    } else {
        printf("DPI-C Send: %s\n", buffer); 
    }
}

void specsitf_comm_recv(int *st, int *mem_req_type, int *mem_req_addr, int *mem_req_wdata) {
    char buffer[SPECSITF_BUFFER_SIZE] = { 0 };

    if ((*specsitf_server)->recv((*specsitf_server)->context, buffer, sizeof(buffer)) != CIPC_OK)
        printf("DPI-C Recv: Failed to receive message!\n");

    if (sscanf(buffer, "st=%d mem_req_type=%d mem_req_addr=%d mem_req_wdata=%d",
               st, mem_req_type, mem_req_addr, mem_req_wdata) != 4) {
        printf("DPI-C Recv: Failed to parse received data! Buffer: %s\n", buffer); 
        *st = 0;
        *mem_req_type = 0;
        *mem_req_addr = 0;
        *mem_req_wdata = 0;
    } else {
        printf("DPI-C Recv: %s\n", buffer); 
    }
}
