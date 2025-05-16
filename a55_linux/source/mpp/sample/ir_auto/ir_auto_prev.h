/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef _OT_IR_AUTO_PREV_H_
#define _OT_IR_AUTO_PREV_H_

typedef struct {
    sample_vi_cfg vi_config;
    sample_vo_cfg vo_config;
    ot_vo_dev vo_dev;
    ot_vo_chn vo_chn;
    ot_vi_pipe vi_pipe;
    ot_vi_chn vi_chn;
} ir_auto_prev;

#endif
