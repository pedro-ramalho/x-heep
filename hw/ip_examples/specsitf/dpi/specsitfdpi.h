#ifndef SPECSITF_DPI_H
#define SPECSITF_DPI_H

#ifdef __cplusplus
extern "C" {
#endif

void specsitf_comm_init();

void specsitf_comm_free();

void specsitf_comm_send(int addr, int ctrl, int obi_gnt, int obi_rvalid, int obi_rdata);

void specsitf_comm_recv(int *st, int *mem_req_type, int *mem_req_addr, int *mem_req_wdata);

#ifdef __cplusplus
}
#endif

#endif // SPECSITF_DPI_H