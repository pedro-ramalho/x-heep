#include <zmq.h>

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
#include <cstring>
/*#include <stdlib.h>
#include <string.h>
#include <unistd.h>*/


void test_specsitf(int data) {
  printf("Hello %d\n", data);

  void *context = zmq_ctx_new();
  if (!context) {
    fprintf(stderr, "Failed to create ZMQ context.\n");

    return;
  }

  void *socket = zmq_socket(context, ZMQ_REP);
  if (!socket) {
    fprintf(stderr, "Failed to create ZMQ socket.\n");

    zmq_ctx_destroy(context);

    return;
  }

  if (zmq_bind(socket, "tcp://*:5555") != 0) {
    fprintf(stderr, "Failed to bind ZMQ socket.\n");

    zmq_close(socket);
    zmq_ctx_destroy(context);

    return;
  }

  printf("Hardware peripheral (REP server) is running and waiting for messages...\n");

  char buffer[256];

  int length = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);

  if (length == -1) {
    fprintf(stderr, "Failed to receive message.\n");
  } else {
    buffer[length] = '\0';

    printf("Received number: %s\n", buffer);

    const char *reply = "ACK";

    zmq_send(socket, reply, strlen(reply), 0);
  }

  zmq_close(socket);
  zmq_ctx_destroy(context);
}
