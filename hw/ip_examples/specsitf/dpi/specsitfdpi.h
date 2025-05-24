#ifndef SPECSITF_DPI_H
#define SPECSITF_DPI_H

#ifdef __cplusplus
extern "C" {
#endif

void specsitf_comm_init();

void specsitf_comm_free();

void specsitf_comm_send(int reg1, int reg2, int ctrl);

void specsitf_comm_recv(int *reg3, int *st);

#ifdef __cplusplus
}
#endif

#endif // SPECSITF_DPI_H