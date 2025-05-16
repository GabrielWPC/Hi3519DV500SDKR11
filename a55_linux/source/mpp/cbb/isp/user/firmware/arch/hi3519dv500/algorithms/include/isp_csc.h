/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef ISP_CSC_H
#define ISP_CSC_H

#include "ot_type.h"
#include "ot_common_isp.h"
#include "mkp_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    td_bool update;
    ot_isp_csc_attr mpi_cfg;
} isp_csc;

td_void isp_csc_read_ext_regs(ot_vi_pipe vi_pipe, isp_csc *csc);
td_void isp_csc_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg);
td_s32  isp_csc_reg_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg, isp_csc *csc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif