/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_CTRL_H
#define OT_CTRL_H

#include <stdint.h>

// CTRL id
#define OT_XUID_SET_RESET 0x01
#define OT_XUID_SET_STREAM 0x02
#define OT_XUID_SET_RESOLUTION 0x03
#define OT_XUID_SET_IFRAME 0x04
#define OT_XUID_SET_BITRATE 0x05
#define OT_XUID_UPDATE_SYSTEM 0x06

typedef struct _uvcx_base_ctrl_t {
    uint16_t selector;
    uint16_t index;
} __attribute__((__packed__)) uvcx_base_ctrl_t;


typedef struct uvcx_camera_stream_t {
    uint16_t format;
    uint16_t resolution;
} __attribute__((__packed__)) uvcx_camera_stream_t;


#endif
