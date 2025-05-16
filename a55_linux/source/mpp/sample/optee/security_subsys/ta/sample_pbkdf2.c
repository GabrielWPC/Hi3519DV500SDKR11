/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "securec.h"

#include "sample_utils.h"
#include "ot_mpi_cipher.h"
#include "sample_func.h"

#define PBKDF2_SHA256_OUT_KEY_LEN    32
#define PBKDF2_SHA384_OUT_KEY_LEN    48
#define PBKDF2_SHA512_OUT_KEY_LEN    64
#define PBKDF2_MAX_OUT_KEY_LEN       64
#define ALG_ITERATION_NUM          5220
#define PASSWORD_LEN                 60
#define SALT_LEN                     40

typedef struct {
    const td_char *pbkdf2_name;
    td_u32 out_key_length;
    crypto_kdf_pbkdf2_param param;
} pbkdf2_data_t;

static pbkdf2_data_t g_pbkdf2_data[] = {
    {
        .pbkdf2_name = "PBKDF2-HMAC-SHA256",
        .out_key_length = PBKDF2_SHA256_OUT_KEY_LEN,
        .param = {
            .hash_type = CRYPTO_HASH_TYPE_HMAC_SHA256,
            .plen = PASSWORD_LEN,
            .slen = SALT_LEN,
            .count = ALG_ITERATION_NUM
        }
    },
    {
        .pbkdf2_name = "PBKDF2-HMAC-SHA384",
        .out_key_length = PBKDF2_SHA384_OUT_KEY_LEN,
        .param = {
            .hash_type = CRYPTO_HASH_TYPE_HMAC_SHA384,
            .plen = PASSWORD_LEN,
            .slen = SALT_LEN,
            .count = ALG_ITERATION_NUM
        }
    },
    {
        .pbkdf2_name = "PBKDF2-HMAC-SHA512",
        .out_key_length = PBKDF2_SHA512_OUT_KEY_LEN,
        .param = {
            .hash_type = CRYPTO_HASH_TYPE_HMAC_SHA512,
            .plen = PASSWORD_LEN,
            .slen = SALT_LEN,
            .count = ALG_ITERATION_NUM
        }
    },
};

/* run pbkdf2 sample */
static td_s32 run_pbkdf2_sample(pbkdf2_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 out_key[PBKDF2_MAX_OUT_KEY_LEN] = {0};

    data->param.password = (td_u8 *)malloc(data->param.plen);
    if (data->param.password == TD_NULL) {
        sample_err("malloc failed\n");
        return TD_FAILURE;
    }
    data->param.salt = (td_u8 *)malloc(data->param.slen);
    if (data->param.salt == TD_NULL) {
        free(data->param.password);
        sample_err("malloc failed\n");
        return TD_FAILURE;
    }

    sample_chk_expr_goto(get_random_data(data->param.password, sizeof(data->param.plen)), TD_SUCCESS, __EXIT_FREE__);
    sample_chk_expr_goto(get_random_data(data->param.salt, sizeof(data->param.slen)), TD_SUCCESS, __EXIT_FREE__);

    /* 1. pbkdf2 calculate */
    sample_chk_expr_goto(ot_mpi_cipher_pbkdf2(&data->param, out_key, data->out_key_length), TD_SUCCESS, __EXIT_FREE__);

__EXIT_FREE__:
    memset_s(data->param.password, data->param.plen, 0, data->param.plen);
    memset_s(data->param.salt, data->param.slen, 0, data->param.slen);
    memset_s(out_key, PBKDF2_MAX_OUT_KEY_LEN, 0, PBKDF2_MAX_OUT_KEY_LEN);
    free(data->param.password);
    free(data->param.salt);
    data->param.password = TD_NULL;
    data->param.salt = TD_NULL;
    return ret;
}

static td_s32 sample_pbkdf2_cal(td_void)
{
    td_u32 i;
    td_s32 ret;
    td_u32 num = (td_u32)(sizeof(g_pbkdf2_data) / sizeof(g_pbkdf2_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_pbkdf2_sample(&g_pbkdf2_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test pbkdf2 %s failed ************\n", g_pbkdf2_data[i].pbkdf2_name);
            return ret;
        }
        sample_log("************ test pbkdf2 %s success ************\n", g_pbkdf2_data[i].pbkdf2_name);
    }
    return TD_SUCCESS;
}

td_s32 sample_pbkdf2(td_void)
{
    td_s32 ret;
    sample_log("************ test pbkdf2 ************\n");
    ret = sample_pbkdf2_cal();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    sample_log("************ test pbkdf2 succeed ************\n");
    return TD_SUCCESS;
}
