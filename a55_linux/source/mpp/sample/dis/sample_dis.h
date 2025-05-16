/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_SAMPLE_DIS_H
#define OT_SAMPLE_DIS_H

#include "ot_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

td_bool sample_dis_get_save_stream(void);
td_void sample_dis_set_save_stream(td_bool save_stream);
td_bool sample_dis_get_send_data_statue(td_void);
td_void sample_dis_set_send_data_statue(td_bool send_data);
td_void sample_dis_set_venc_chn_size(ot_size img_size);
td_void sample_dis_pause(td_char *s);
td_s32 sample_dis_start_sample(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg, ot_size *img_size);
td_s32 sample_dis_stop_sample(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg);
td_void sample_dis_stop_sample_without_sys_exit(sample_vi_cfg *vi_cfg, sample_vo_cfg *vo_cfg);

#ifdef __cplusplus
}
#endif /* End of __cplusplus */

#endif /* OT_SAMPLE_DIS_H */