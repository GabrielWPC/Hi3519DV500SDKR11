/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_AUDIO_H
#define OT_AUDIO_H


typedef struct audio_control_ops {
    int (*init)(void);
    int (*startup)(void);
    int (*shutdown)(void);
} audio_control_ops_st;

typedef struct ot_audio {
    struct audio_control_ops *mpi_ac_ops;
    int audioing;
} ot_audio;

/* audio control functions */
extern int ot_audio_register_mpi_ops(struct audio_control_ops *ac_ops);

extern int ot_audio_init(void);
extern int ot_audio_startup(void);
extern int ot_audio_shutdown(void);

#endif
