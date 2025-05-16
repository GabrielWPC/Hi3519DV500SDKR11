/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __MEDIA_VDEC_H__
#define __MEDIA_VDEC_H__

#include "sample_comm.h"

td_s32 sample_uvc_media_init(const td_char *type_name, td_u32 width, td_u32 height);
td_s32 sample_uvc_media_exit(td_void);
td_s32 sample_uvc_media_send_data(td_void *data, td_u32 size, td_u32 stride,
    const ot_size *pic_size, const td_char *type_name);
td_s32 sample_uvc_media_stop_receive_data(td_void);

#endif /* end of #ifndef __MEDIA_VDEC_H__ */
