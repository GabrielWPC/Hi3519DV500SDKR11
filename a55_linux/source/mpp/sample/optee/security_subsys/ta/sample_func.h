/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_FUNC_H
#define SAMPLE_FUNC_H

#include "ot_type.h"

td_s32 sample_symc_clearkey(td_void);
td_s32 sample_symc_rootkey(td_void);
td_s32 sample_symc_mac(td_void);
td_s32 sample_hash(td_void);
td_s32 sample_pbkdf2(td_void);
td_s32 sample_trng(td_void);
td_s32 sample_rsa(td_void);
td_s32 sample_ecc(td_void);
td_s32 sample_sm4_clearkey(td_void);
td_s32 sample_sm3(td_void);
td_s32 sample_sm2(td_void);

#endif