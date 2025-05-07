// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef SPECSITF_DPI_H_
#define SPECSITF_DPI_H_

#ifdef __cplusplus
extern "C" {
#endif

void specsitf_comm_init();

void specsitf_comm_free();

void specsitf_comm_send(int int_reg_1_i, int int_reg_2_i);

void specsitf_comm_recv(int *int_reg_1_o, int *int_reg_2_o);

#ifdef __cplusplus
}
#endif
#endif // SPECSITF_DPI_H_
