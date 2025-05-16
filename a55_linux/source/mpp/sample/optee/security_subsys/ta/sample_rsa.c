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

/* RSA Crypto. */
typedef struct {
    const td_char *rsa_name;
    drv_pke_rsa_scheme scheme;
    drv_pke_hash_type hash_type;
    td_u32 key_len;
    td_u32 input_data_len;
} rsa_crypto_data;

/* RSA Sign&Verify. */
typedef struct {
    const td_char *rsa_name;
    drv_pke_rsa_scheme scheme;
    drv_pke_hash_type hash_type;
    td_u32 key_len;
    td_u32 hash_len;
} rsa_sign_data;

/* hash type map */
#define HASH_TYPE_MAP_SIZE    3

typedef struct {
    crypto_hash_type cipher_hash_type;
    drv_pke_hash_type pke_hash_type;
} hash_type_pair;

static hash_type_pair g_hash_type_map[] = {{CRYPTO_HASH_TYPE_SHA256, DRV_PKE_HASH_TYPE_SHA256},
    {CRYPTO_HASH_TYPE_SHA384, DRV_PKE_HASH_TYPE_SHA384}, {CRYPTO_HASH_TYPE_SHA512, DRV_PKE_HASH_TYPE_SHA512}};

static rsa_crypto_data g_rsa_crypto_data[] = {
    {
        .rsa_name = "RSA-CRYPTO-V21-SHA256", .scheme = DRV_PKE_RSA_SCHEME_PKCS1_V21,
        .hash_type = DRV_PKE_HASH_TYPE_SHA256, .key_len = RSA_4096_KEY_LEN, .input_data_len = RSA_TEST_DATA_LEN
    },
    {
        .rsa_name = "RSA-CRYPTO-V21-SHA384", .scheme = DRV_PKE_RSA_SCHEME_PKCS1_V21,
        .hash_type = DRV_PKE_HASH_TYPE_SHA384, .key_len = RSA_4096_KEY_LEN, .input_data_len = RSA_TEST_DATA_LEN
    },
    {
        .rsa_name = "RSA-CRYPTO-V21-SHA512", .scheme = DRV_PKE_RSA_SCHEME_PKCS1_V21,
        .hash_type = DRV_PKE_HASH_TYPE_SHA512, .key_len = RSA_4096_KEY_LEN, .input_data_len = RSA_TEST_DATA_LEN
    }
};

static rsa_sign_data g_rsa_sign_data[] = {
    {
        .rsa_name = "RSA-SIGN-3072-PSS-SHA384", .scheme = DRV_PKE_RSA_SCHEME_PKCS1_V21,
        .hash_type = DRV_PKE_HASH_TYPE_SHA384, .key_len = RSA_3072_KEY_LEN, .hash_len = SHA384_HASH_LEN
    },
    {
        .rsa_name = "RSA-SIGN-4096-PSS-SHA512", .scheme = DRV_PKE_RSA_SCHEME_PKCS1_V21,
        .hash_type = DRV_PKE_HASH_TYPE_SHA512, .key_len = RSA_4096_KEY_LEN, .hash_len = SHA512_HASH_LEN
    },
};

/* run rsa crypto sample */
static td_s32 run_rsa_crypto_sample(rsa_crypto_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 encrypt_buffer[RSA_MAX_DATA_LEN] = {0};
    td_u8 decrypt_buffer[RSA_MAX_DATA_LEN] = {0};
    td_u8 result_buffer[RSA_MAX_DATA_LEN] = {0};
    drv_pke_rsa_priv_key priv_key = {0};
    drv_pke_rsa_pub_key pub_key = {0};
    drv_pke_data input = {0};
    drv_pke_data output = {0};

    input.data = encrypt_buffer;
    input.length = data->input_data_len;
    output.data = decrypt_buffer;
    output.length = data->key_len;

    sample_chk_expr_return(get_random_data(encrypt_buffer, sizeof(encrypt_buffer)), TD_SUCCESS);
    sample_chk_expr_return(get_rsa_key(&pub_key, &priv_key, data->key_len), TD_SUCCESS);

    /* 1. pke init */
    sample_chk_expr_goto(ot_mpi_cipher_pke_init(), TD_SUCCESS, __DESTROY_KEY__);

    /* 2. public key encrypt */
    sample_chk_expr_goto(ot_mpi_cipher_pke_rsa_public_encrypt(data->scheme, data->hash_type, &pub_key, &input,
        TD_NULL, &output), TD_SUCCESS, __PKE_DEINIT__);

    input.data = decrypt_buffer;
    input.length = data->key_len;
    output.data = result_buffer;
    output.length = data->key_len;

    /* 3. private key decrypt */
    sample_chk_expr_goto(ot_mpi_cipher_pke_rsa_private_decrypt(data->scheme, data->hash_type, &priv_key, &input,
        TD_NULL, &output), TD_SUCCESS, __PKE_DEINIT__);

    /* 4. compare the result */
    sample_chk_expr_goto_with_ret(memcmp(encrypt_buffer, result_buffer, data->input_data_len), 0, ret, TD_FAILURE,
        __PKE_DEINIT__);

__PKE_DEINIT__:
    ot_mpi_cipher_pke_deinit();
__DESTROY_KEY__:
    destroy_rsa_key(&pub_key, &priv_key, data->key_len);
    return ret;
}

/* run rsa sign & verify sample */
static td_s32 run_rsa_sign_sample(rsa_sign_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_s32 i;
    td_handle hash_handle;
    td_u8 input_msg[RSA_MAX_DATA_LEN] = {0};
    td_u8 *input_hash_data;
    td_u8 out_sign[RSA_MAX_DATA_LEN] = {0};
    drv_pke_rsa_priv_key priv_key = {0};
    drv_pke_rsa_pub_key pub_key = {0};
    drv_pke_data sign = {0};
    drv_pke_data input_hash = {0};
    crypto_hash_attr hash_attr = {0};
    crypto_buf_attr src_buf = {0};
    td_u32 output_hash_len = 0;

    input_hash_data = malloc(data->hash_len);
    sample_chk_expr_return(input_hash_data != TD_NULL, TD_TRUE);

    sign.data = out_sign;
    sign.length = data->key_len;
    input_hash.length = data->hash_len;
    input_hash.data = input_hash_data;

    sample_chk_expr_goto(get_random_data(input_msg, sizeof(input_msg)), TD_SUCCESS, __EXIT_FREE__);
    sample_chk_expr_goto(get_rsa_key(&pub_key, &priv_key, data->key_len), TD_SUCCESS, __EXIT_FREE__);

    /* 1. hash init */
    sample_chk_expr_goto(ot_mpi_cipher_hash_init(), TD_SUCCESS, __DESTROY_KEY__);

    /* 2. hash create */
    hash_attr.is_long_term = TD_FALSE;
    for (i = 0; i < HASH_TYPE_MAP_SIZE; ++i) {
        if (g_hash_type_map[i].pke_hash_type == data->hash_type) {
            hash_attr.hash_type = g_hash_type_map[i].cipher_hash_type;
        }
    }
    sample_chk_expr_goto(ot_mpi_cipher_hash_create(&hash_handle, &hash_attr), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. hash update */
    src_buf.virt_addr = input_msg;
    if (ot_mpi_cipher_hash_update(hash_handle, &src_buf, sizeof(input_msg)) != TD_SUCCESS) {
        ot_mpi_cipher_hash_destroy(hash_handle);
        goto __HASH_DEINIT__;
    }

    /* 4. hash finish */
    if (ot_mpi_cipher_hash_finish(hash_handle, input_hash_data, data->hash_len,  &output_hash_len) != TD_SUCCESS) {
        ot_mpi_cipher_hash_destroy(hash_handle);
        goto __HASH_DEINIT__;
    }

    /* 5. pke init */
    sample_chk_expr_goto(ot_mpi_cipher_pke_init(), TD_SUCCESS, __HASH_DEINIT__);

    /* 6. rsa sign */
    sample_chk_expr_goto(ot_mpi_cipher_pke_rsa_sign(&priv_key, data->scheme, data->hash_type, &input_hash, &sign),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 7. rsa verify */
    sample_chk_expr_goto(ot_mpi_cipher_pke_rsa_verify(&pub_key, data->scheme, data->hash_type, &input_hash, &sign),
        TD_SUCCESS, __PKE_DEINIT__);

__PKE_DEINIT__:
    ot_mpi_cipher_pke_deinit();
__HASH_DEINIT__:
    ot_mpi_cipher_hash_deinit();
__DESTROY_KEY__:
    destroy_rsa_key(&pub_key, &priv_key, data->key_len);
__EXIT_FREE__:
    free(input_hash_data);
    input_hash_data = TD_NULL;
    return ret;
}

static td_s32 sample_rsa_cal(td_void)
{
    td_u32 i;
    td_s32 ret;
    td_u32 num = (td_u32)(sizeof(g_rsa_crypto_data) / sizeof(g_rsa_crypto_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_rsa_crypto_sample(&g_rsa_crypto_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test RSA %s failed ************\n", g_rsa_crypto_data[i].rsa_name);
            return ret;
        }
        sample_log("************ test RSA %s crypto success ************\n", g_rsa_crypto_data[i].rsa_name);
    }
    num = (td_u32)(sizeof(g_rsa_sign_data) / sizeof(g_rsa_sign_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_rsa_sign_sample(&g_rsa_sign_data[i]);
        if (ret !=TD_SUCCESS) {
            sample_err("************ test RSA %s failed ************\n", g_rsa_sign_data[i].rsa_name);
            return ret;
        }
        sample_log("************ test RSA %s sign & verify success ************\n", g_rsa_sign_data[i].rsa_name);
    }
    return TD_SUCCESS;
}

td_s32 sample_rsa(td_void)
{
    td_s32 ret;
    sample_log("************ test RSA ************\n");
    ret = sample_rsa_cal();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    sample_log("************ test RSA succeed ************\n");
    return TD_SUCCESS;
}
