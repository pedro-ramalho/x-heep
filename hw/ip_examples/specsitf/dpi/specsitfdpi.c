#include "specsitfdpi.h"
#include "backend/cipc_zmq.h"
#include <cipc.h>
#include <msgpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#define MESSAGE_ROOT_MAP_NUM_ENTRIES 3
#define MESSAGE_MANDATORY_FIELD_CONTROL "control"
#define MESSAGE_MANDATORY_FIELD_STATUS "status"
#define MESSAGE_MANDATORY_FIELD_DATA "data"
#define MESSAGE_MANDATORY_FIELD_MEMORY "memory"

#define SPECSITF_ADDRESS "tcp://localhost:5556"
#define SPECSITF_BUFFER_SIZE 1024
#define SPECSITF_LOG_ENABLE 1

/* server instance */
static cipc **specsitf_server = NULL;

/* message buffer & packer */
static msgpack_sbuffer sbuf;
static msgpack_packer pk;

static void pack_map(size_t entries) { msgpack_pack_map(&pk, entries); }

static void pack_nested_map(char *k, size_t entries) {
  msgpack_pack_str(&pk, strlen(k));
  msgpack_pack_str_body(&pk, k, strlen(k));
  msgpack_pack_map(&pk, entries);
}

static void pack_kv_u32(char *k, int v) {
  msgpack_pack_str(&pk, strlen(k));
  msgpack_pack_str_body(&pk, k, strlen(k));
  msgpack_pack_uint32(&pk, v);
}

static int fetch_ui32(msgpack_object *obj, uint32_t default_value) {
  if (obj == NULL)
    return default_value;

  if (obj->type == MSGPACK_OBJECT_POSITIVE_INTEGER ||
      obj->type == MSGPACK_OBJECT_NEGATIVE_INTEGER) {
    return (uint32_t)obj->via.u64;
  }

  return default_value;
}

static msgpack_object *get_map_value_by_key(msgpack_object *map_obj,
                                            const char *key) {
  if (map_obj->type != MSGPACK_OBJECT_MAP) {
    return NULL;
  }
  for (size_t i = 0; i < map_obj->via.map.size; ++i) {
    msgpack_object k = map_obj->via.map.ptr[i].key;
    if (k.type == MSGPACK_OBJECT_STR && k.via.str.size == strlen(key) &&
        strncmp(k.via.str.ptr, key, k.via.str.size) == 0) {
      return &(map_obj->via.map.ptr[i].val);
    }
  }
  return NULL;
}

void specsitf_comm_init() {
  specsitf_server = (cipc **)malloc(sizeof(cipc *));
  if (!specsitf_server) {
#if SPECSITF_LOG_ENABLE
    printf("Failed to allocate memory for server!\n");
#endif

    exit(EXIT_FAILURE);
  }

  *specsitf_server = cipc_create(CIPC_PROTOCOL_ZMQ);
  if (!(*specsitf_server)) {
#if SPECSITF_LOG_ENABLE
    printf("Failed to create server!\n");
#endif

    exit(EXIT_FAILURE);
  }

  cipc_zmq_config *config = cipc_zmq_config_req(SPECSITF_ADDRESS);

  if ((*specsitf_server)->init(&(*specsitf_server)->context, config) !=
      CIPC_OK) {

#if SPECSITF_LOG_ENABLE
    printf("Failed to initialize server!\n");
#endif

    specsitf_comm_free();

    exit(EXIT_FAILURE);
  }

  msgpack_sbuffer_init(&sbuf);
  msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
}

void specsitf_comm_free() {
  if (specsitf_server) {
    if (*specsitf_server)
      cipc_free(*specsitf_server);

    free(specsitf_server);

    specsitf_server = NULL;
  }

  msgpack_sbuffer_destroy(&sbuf);
}

void specsitf_comm_send(unsigned int read_address, unsigned int write_address,
                        unsigned int threshold, unsigned int data_size, unsigned int control,
                        unsigned int read_req_gnt, unsigned int read_req_rvalid,
                        unsigned int read_req_rdata,
                        unsigned int write_req_gnt) {

  msgpack_sbuffer_clear(&sbuf);

  /* initialize the root map (control, data, memory) */
  pack_map(MESSAGE_ROOT_MAP_NUM_ENTRIES);

  /* pack control */
  pack_kv_u32(MESSAGE_MANDATORY_FIELD_CONTROL, control);

  /* pack data */
  pack_nested_map(MESSAGE_MANDATORY_FIELD_DATA, 4);
  pack_kv_u32("read_address", read_address);
  pack_kv_u32("write_address", write_address);
  pack_kv_u32("threshold", threshold);
  pack_kv_u32("data_size", data_size);

  /* pack memory */
  pack_nested_map(MESSAGE_MANDATORY_FIELD_MEMORY, 4);
  pack_kv_u32("read_req_gnt", read_req_gnt);
  pack_kv_u32("read_req_rvalid", read_req_rvalid);
  pack_kv_u32("read_req_rdata", read_req_rdata);
  pack_kv_u32("write_req_gnt", write_req_gnt);

#if SPECSITF_LOG_ENABLE
  printf("Sending data: read_req_gnt=%d read_req_valid=%d read_req_rdata=%d "
         "write_req_gnt=%d\n");
#endif

  cipc_err result =
      (*specsitf_server)
          ->send((*specsitf_server)->context, sbuf.data, sbuf.size);

#if SPECSITF_LOG_ENABLE
  if (result != CIPC_OK) {
    printf("Failed to send reply!\n");
  } else {
    /* try to display messages here */
  }
#endif
}

void specsitf_comm_recv(unsigned int *status,
                        unsigned int *read_channel_req_type,
                        unsigned int *read_channel_req_addr,
                        unsigned int *write_channel_req_type,
                        unsigned int *write_channel_req_addr,
                        unsigned int *write_channel_req_data) {
  char buffer[SPECSITF_BUFFER_SIZE] = {0};

  size_t received_length = 0;

  cipc_err result = (*specsitf_server)
                        ->recv((*specsitf_server)->context, buffer,
                               sizeof(buffer), &received_length);

#if SPECSITF_LOG_ENABLE
  if (result != CIPC_OK)
    printf("Failed to parse received data!\n");
#endif

  msgpack_unpacked msg;
  msgpack_unpacked_init(&msg);

  size_t offset = 0;
  if (!msgpack_unpack_next(&msg, buffer, received_length, &offset)) {
#if SPECSITF_LOG_ENABLE
    printf("RECV: Could not unpack next message!\n");
#endif

    msgpack_unpacked_destroy(&msg);
    *status = 0;

    *read_channel_req_type = 0;
    *read_channel_req_addr = 0;

    *write_channel_req_type = 0;
    *write_channel_req_addr = 0;
    *write_channel_req_data = 0;

    return;
  }

  msgpack_object root = msg.data;

  *status = fetch_ui32(get_map_value_by_key(&root, "status"), 0);

  *read_channel_req_type =
      fetch_ui32(get_map_value_by_key(&root, "read_channel_req_type"), 0);
  *read_channel_req_addr =
      fetch_ui32(get_map_value_by_key(&root, "read_channel_req_addr"), 0);

  *write_channel_req_type =
      fetch_ui32(get_map_value_by_key(&root, "write_channel_req_type"), 0);
  *write_channel_req_addr =
      fetch_ui32(get_map_value_by_key(&root, "write_channel_req_addr"), 0);
  *write_channel_req_data =
      fetch_ui32(get_map_value_by_key(&root, "write_channel_req_data"), 0);

#if SPECSITF_LOG_ENABLE
  printf("RECV: status=%d read_channel_req_type=%d read_channel_req_addr=%d "
         "write_channel_req_type=%d write_channel_req_addr=%d "
         "write_channel_req_data=%d\n",
         *status, *read_channel_req_type, *read_channel_req_addr,
         *write_channel_req_type, *write_channel_req_addr,
         *write_channel_req_data);
#endif

  msgpack_unpacked_destroy(&msg);
}
