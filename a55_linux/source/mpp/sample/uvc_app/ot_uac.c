/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_uac.h"
#include "ot_audio.h"

static int uac_init(void)
{
    return 0;
}

static int uac_open(void)
{
    return 0;
}

static int uac_close(void)
{
    ot_audio_shutdown();
    return 0;
}

static int uac_run(void)
{
    ot_audio_init();
    ot_audio_startup();

    return 0;
}

/* ---------------------------------------------------------------------- */

static ot_uac g_ot_uac = {
    .init = uac_init,
    .open = uac_open,
    .close = uac_close,
    .run = uac_run,
};

ot_uac *get_ot_uac(void)
{
    return &g_ot_uac;
}

void release_ot_uac(ot_uac *uvc) {}
