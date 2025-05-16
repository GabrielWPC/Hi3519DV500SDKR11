/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_UAC_H
#define OT_UAC_H

typedef struct ot_uac {
    int (*init)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
} ot_uac;

ot_uac *get_ot_uac(void);
void release_ot_uac(ot_uac *uvc);

#endif
