/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __OT_UVC_H__
#define __OT_UVC_H__

typedef struct ot_uvc {
    int (*init)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
} ot_uvc;

ot_uvc *get_ot_uvc(void);
void release_ot_uvc(ot_uvc *uvc);

#endif // __OT_UVC_H__
