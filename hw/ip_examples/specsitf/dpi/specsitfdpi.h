#ifndef SPECSITF_DPI_H
#define SPECSITF_DPI_H

#ifdef __cplusplus
extern "C" {
#endif

void specsitf_comm_init();

void specsitf_comm_free();

void specsitf_comm_send(
	unsigned int read_address,
	unsigned int write_address,
	unsigned int threshold,
	unsigned int data_size, 
	unsigned int control,
	unsigned int read_req_gnt, unsigned int read_req_rvalid, unsigned int read_req_rdata,
	unsigned int write_req_gnt
);

void specsitf_comm_recv(
	unsigned int *status, 
	unsigned int *read_channel_req_type, unsigned int *read_channel_req_addr,
	unsigned int *write_channel_req_type, unsigned int *write_channel_req_addr, unsigned int *write_channel_req_data
);

#ifdef __cplusplus
}
#endif

#endif // SPECSITF_DPI_H
