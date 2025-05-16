/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include "log.h"
#include "frame_cache.h"
#include "ot_camera.h"
#include "sample_comm.h"
#include "sample_venc.h"
#include "uvc_media.h"
#include "uvc.h"

#define SAMPLE_UVC_MAX_STREAM_NAME_LEN 64

#define SAMPLE_RETURN_CONTINUE  1
#define SAMPLE_RETURN_BREAK     2

static ot_payload_type change_to_mpp_format(uint32_t fcc)
{
    ot_payload_type t;

    switch (fcc) {
        case VIDEO_IMG_FORMAT_YUYV:
        case VIDEO_IMG_FORMAT_NV12:
        case VIDEO_IMG_FORMAT_NV21:
        case VIDEO_IMG_FORMAT_MJPEG:
            t = OT_PT_MJPEG;
            break;

        case VIDEO_IMG_FORMAT_H264:
            t = OT_PT_H264;
            break;

        case VIDEO_IMG_FORMAT_H265:
            t = OT_PT_H265;
            break;

        default:
            t = OT_PT_MJPEG;
            break;
    }

    return t;
}

static ot_pic_size change_to_mpp_wh(encoder_property *p)
{
    unsigned int width, height;
    if (p == NULL) {
        return PIC_720P;
    }
    width = p->width;
    height = p->height;

    switch (width) {
        case 640:           /* 640: width */
            if (height == 480) { /* 480: height */
                return PIC_640X480;
            } else {
                return PIC_360P;   /* 640 x 360 */
            }
        case 1280:          /* 1280: width */
            return PIC_720P;
        case 1920:          /* 1920: width */
            return PIC_1080P;
        case 3840:          /* 3840: width */
            return PIC_3840X2160;
        default:
            return PIC_720P;
    }
}

static td_void set_config_format(ot_payload_type *format, int idx)
{
    encoder_property property;
    sample_uvc_get_encoder_property(&property);

    format[idx] = change_to_mpp_format(property.format);
}

static td_void set_config_wh(ot_pic_size *wh, int idx)
{
    encoder_property property;
    sample_uvc_get_encoder_property(&property);

    wh[idx] = change_to_mpp_wh(&property);
}

td_void set_user_config_format(ot_payload_type *format, ot_pic_size *wh, td_s32 *c)
{
    if (format == NULL || wh == NULL || c == NULL) {
        err("format or wh, c is NULL\n");
        return;
    }
    set_config_format(format, 0);
    set_config_wh(wh, 0);
    *c = 1;
}

int is_channel_yuv(int channel)
{
    encoder_property property;
    sample_uvc_get_encoder_property(&property);

    if ((channel == 1) &&
        ((property.format == VIDEO_IMG_FORMAT_YUYV) ||
        (property.format == VIDEO_IMG_FORMAT_NV12) ||
        (property.format == VIDEO_IMG_FORMAT_NV21))) {
        return 1;
    }

    return 0;
}

