/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "sample_utils.h"
#include "ot_mpi_cipher.h"
#include "sample_func.h"

#define GENERATE_TIMES  10

td_s32 sample_trng(td_void)
{
    sample_log("************ test trng ************\n");
    td_s32 ret;
    td_s32 index;
    td_u32 random_number = 0;

    /* 1. cipher init */
    for (index = 0; index < GENERATE_TIMES; index++) {
        ret = ot_mpi_cipher_trng_get_random(&random_number);
        if (ret != TD_SUCCESS) {
            sample_err("ot_mpi_cipher_trng_get_random failed!\n");
            goto __EXIT__;
        }
    }
    sample_log("************ test trng success ************\n");
__EXIT__:
    return ret;
}

