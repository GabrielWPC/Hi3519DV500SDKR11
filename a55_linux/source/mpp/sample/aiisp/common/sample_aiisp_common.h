/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_AIISP_COMMON_H
#define SAMPLE_AIISP_COMMON_H

#include <signal.h>
#include "sample_comm.h"
#include "ot_sns_ctrl.h"
#include "ot_common_aiisp.h"
#include "ot_common_aibnr.h"
#include "ss_mpi_aibnr.h"
#include "ss_mpi_ae.h"

/* exps is true, return ret */
#define sample_aiisp_check_exps_return(exps) \
do { \
    if ((exps)) { \
        printf("realpath file failed\n"); \
        return TD_FAILURE; \
    } \
} while (0) \

td_void sample_aiisp_handle_sig(td_s32 signo);
td_void sample_get_char(td_char *s);
sig_atomic_t sample_aiisp_get_sig(td_void);
sample_sns_type sample_aiisp_get_wdr_sns_type(sample_sns_type sns_type);
td_s32 sample_aiisp_load_mem(ot_aiisp_mem_info *mem, td_char *model_file);
td_void sample_aiisp_unload_mem(ot_aiisp_mem_info *param_mem);
td_s32 sample_aiisp_sys_init(ot_size *in_size, td_u32 vb_cnt);
td_s32 sample_aiisp_start_vpss(ot_vpss_grp grp, ot_size *in_size);
td_s32 sample_aiisp_start_vo(sample_vo_mode vo_mode);
td_void sample_aiisp_stop_vo(td_void);
td_s32 sample_aiisp_start_venc(ot_venc_chn venc_chn[], td_u32 venc_chn_len, td_u32 chn_num, ot_size *size);
td_s32 sample_aiisp_start_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num, ot_size *in_size);
td_void sample_aiisp_stop_vpss(ot_vpss_grp grp);
td_void sample_aiisp_stop_venc(ot_venc_chn venc_chn[], td_u32 venc_chn_len, td_u32 chn_num);
td_void sample_aiisp_stop_venc_and_vo(ot_vpss_grp vpss_grp[], td_u32 grp_num);
td_s32 sample_aiisp_set_long_frame_mode(ot_vi_pipe vi_pipe, td_bool is_wdr_mode);
td_void sample_aiisp_get_default_cfg(sample_sns_type sns_type, ot_vi_pipe vi_pipe,
    ot_size *size, sample_vi_cfg *vi_cfg);
td_void sample_aiisp_sensor_cfg(ot_vi_pipe vi_pipe, sample_sns_type sns_type);

#endif
