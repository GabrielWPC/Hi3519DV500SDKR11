/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include "ot_audio.h"

static struct ot_audio g_ot_audio = {
    .mpi_ac_ops = NULL,
    .audioing = 0,
};

static ot_audio *get_ot_audio(void)
{
    return &g_ot_audio;
}

int ot_audio_init(void)
{
    if (get_ot_audio()->mpi_ac_ops && get_ot_audio()->mpi_ac_ops->init) {
        return get_ot_audio()->mpi_ac_ops->init();
    }

    return 0;
}

int ot_audio_startup(void)
{
    int ret = 0;

    if (get_ot_audio()->mpi_ac_ops && get_ot_audio()->mpi_ac_ops->startup) {
        ret = get_ot_audio()->mpi_ac_ops->startup();
    }

    if (ret == 0) {
        get_ot_audio()->audioing = 1;
    }
    return ret;
}

int ot_audio_shutdown(void)
{
    int ret = 0;

    if (get_ot_audio()->mpi_ac_ops && get_ot_audio()->mpi_ac_ops->shutdown) {
        ret = get_ot_audio()->mpi_ac_ops->shutdown();
    }

    if (ret == 0) {
        get_ot_audio()->audioing = 0;
    }
    return ret;
}

int ot_audio_register_mpi_ops(struct audio_control_ops *ac_ops)
{
    if (ac_ops != NULL) {
        get_ot_audio()->mpi_ac_ops = ac_ops;
    }

    return 0;
}
