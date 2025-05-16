/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef ISP_THERMO_H
#define ISP_THERMO_H

#include "ot_type.h"
#include "ot_common_isp.h"
#include "mkp_isp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

td_s32 isp_thermo_run_callback(ot_vi_pipe vi_pipe, const isp_lib_node *lib, const ot_isp_ae_info *ae_info,
    ot_isp_ae_result *ae_result, isp_reg_cfg *reg_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif