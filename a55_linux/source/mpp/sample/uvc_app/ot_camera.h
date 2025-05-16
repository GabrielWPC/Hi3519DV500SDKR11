/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_CAMERA_H
#define OT_CAMERA_H

typedef enum ot_stream_type_e {
    OT_FORMAT_H264 = 0x1,
    OT_FORMAT_MJPEG = 0x2,
    OT_FORMAT_MJPG = 0x2,
    OT_FORMAT_YUV = 0x3,
    OT_FORMAT_YUV420 = 0x3
} ot_stream_type_e;

typedef enum ot_stream_resolution_e {
    OT_RESOLUTION_1080 = 0x1,
    OT_RESOLUTION_720 = 0x2,
    OT_RESOLUTION_360 = 0x3
} ot_stream_resolution_e;

typedef struct encoder_property {
    unsigned int format;
    unsigned int width;
    unsigned int height;
    unsigned int fps;
    unsigned char compsite;
} encoder_property;

#endif
