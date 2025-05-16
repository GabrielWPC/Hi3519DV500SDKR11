/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include "securec.h"

#include "sample_utils.h"
#include "ss_mpi_cipher.h"
#include "sample_func.h"

/* ECC data. */
typedef struct {
    const td_char *ecc_name;
    drv_pke_ecc_curve_type curve_type;
    td_u32 key_len;
    td_u32 hash_len;
    crypto_hash_type hash_type;
} ecc_data;

static ecc_data g_ecdsa_data[] = {
    {
        .ecc_name = "brainpoolP256r1-sha256", .key_len = ECC_256_KEY_LENGTH,
        .curve_type = DRV_PKE_ECC_TYPE_RFC5639_P256, .hash_len = SHA256_HASH_LEN, .hash_type = CRYPTO_HASH_TYPE_SHA256
    },
    {
        .ecc_name = "brainpoolP384r1-sha384", .key_len = ECC_384_KEY_LENGTH,
        .curve_type = DRV_PKE_ECC_TYPE_RFC5639_P384, .hash_len = SHA384_HASH_LEN, .hash_type = CRYPTO_HASH_TYPE_SHA384
    },
    {
        .ecc_name = "brainpoolP512r1-sha512", .key_len = ECC_512_KEY_LENGTH,
        .curve_type = DRV_PKE_ECC_TYPE_RFC5639_P512, .hash_len = SHA512_HASH_LEN, .hash_type = CRYPTO_HASH_TYPE_SHA512
    }
};

static ecc_data g_eddsa_data[] = {
    {
        .ecc_name = "RFC8032", .key_len = ECC_256_KEY_LENGTH,
        .curve_type = DRV_PKE_ECC_TYPE_RFC8032, .hash_len = SHA256_HASH_LEN
    }
};

/* ecc generate key */
static td_s32 run_ecc_generate_key_sample(ecc_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 d[MAX_ECC_LENGTH] = {0};
    td_u8 x[MAX_ECC_LENGTH] = {0};
    td_u8 y[MAX_ECC_LENGTH] = {0};
    drv_pke_data priv_key = {
        .data = d,
    };
    drv_pke_ecc_point pub_key = {
        .x = x,
        .y = y,
    };
    td_bool is_on_curve = TD_FALSE;

    priv_key.length = data->key_len;
    pub_key.length = data->key_len;

    /* 1. pke init */
    sample_chk_expr_return(ss_mpi_cipher_pke_init(), TD_SUCCESS);

    /* 2. generate ecc key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &priv_key, &pub_key), TD_SUCCESS,
        __PKE_DEINIT__);

    /* 3. check if the dot is on the specific curve */
    sample_chk_expr_goto(ss_mpi_cipher_pke_check_dot_on_curve(data->curve_type, &pub_key, &is_on_curve), TD_SUCCESS,
        __PKE_DEINIT__);
    sample_chk_expr_goto_with_ret(is_on_curve == TD_TRUE, TD_TRUE, ret, TD_FAILURE, __PKE_DEINIT__);

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
    memset_s(d, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    return ret;
}

/* ecdh */
static td_s32 run_ecc_ecdh_sample(ecc_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_u32 length;
    td_u8 d1[MAX_ECC_LENGTH] = {0};
    td_u8 x1[MAX_ECC_LENGTH] = {0};
    td_u8 y1[MAX_ECC_LENGTH] = {0};
    td_u8 d2[MAX_ECC_LENGTH] = {0};
    td_u8 x2[MAX_ECC_LENGTH] = {0};
    td_u8 y2[MAX_ECC_LENGTH] = {0};
    td_u8 key1[MAX_ECC_LENGTH] = {0};
    td_u8 key2[MAX_ECC_LENGTH] = {0};

    drv_pke_data priv_key1 = {
        .data = d1,
    };
    drv_pke_ecc_point pub_key1 = {
        .x = x1,
        .y = y1,
    };
    drv_pke_data priv_key2 = {
        .data = d2,
    };
    drv_pke_ecc_point pub_key2 = {
        .x = x2,
        .y = y2,
    };
    drv_pke_data shared_key1 = {
        .data = key1,
        .length = MAX_ECC_LENGTH
    };
    drv_pke_data shared_key2 = {
        .data = key2,
        .length = MAX_ECC_LENGTH
    };

    length = data->key_len;
    priv_key1.length = length;
    pub_key1.length = length;
    priv_key2.length = length;
    pub_key2.length = length;
    shared_key1.length = length;
    shared_key2.length = length;

    /* 1. pke init */
    sample_chk_expr_return(ss_mpi_cipher_pke_init(), TD_SUCCESS);

    /* 2. get key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &priv_key1, &pub_key1), TD_SUCCESS,
        __PKE_DEINIT__);

    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &priv_key2, &pub_key2), TD_SUCCESS,
        __PKE_DEINIT__);

    /* 3. get ecdh key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_ecdh_key(data->curve_type, &pub_key2, &priv_key1, &shared_key1),
        TD_SUCCESS, __PKE_DEINIT__);

    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_ecdh_key(data->curve_type, &pub_key1, &priv_key2, &shared_key2),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 4. compare the key, keys should be consistent */
    sample_chk_expr_goto_with_ret(memcmp(key1, key2, length), 0, ret, TD_FAILURE, __PKE_DEINIT__);

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
    memset_s(d1, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x1, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y1, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(d2, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x2, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y2, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(key1, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(key2, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    return ret;
}

/* ecc sign */
static td_s32 run_ecc_sign_sample(ecc_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    td_bool is_on_curve = TD_FALSE;
    td_u8 d[MAX_ECC_LENGTH] = {0};
    td_u8 x[MAX_ECC_LENGTH] = {0};
    td_u8 y[MAX_ECC_LENGTH] = {0};
    td_u8 r[MAX_ECC_LENGTH] = {0};
    td_u8 s[MAX_ECC_LENGTH] = {0};
    td_u8 input_msg[MAX_ECC_LENGTH] = {0};
    td_u8 *input_hash_data;
    crypto_hash_attr hash_attr = {0};
    crypto_buf_attr src_buf = {0};
    td_u32 output_hash_len = 0;

    input_hash_data = malloc(data->key_len);
    sample_chk_expr_return(input_hash_data != TD_NULL, TD_TRUE);

    sample_chk_expr_goto(get_random_data(input_msg, sizeof(input_msg)), TD_SUCCESS, __EXIT_FREE__);

    drv_pke_data output_priv_key = {
        .data = d,
    };
    drv_pke_ecc_point output_pub_key = {
        .x = x,
        .y = y,
    };
    drv_pke_data input_hash = {
        .data = input_hash_data,
        .length = data->hash_len
    };
    drv_pke_ecc_sig output_sig = {
        .r = r,
        .s = s,
    };

    output_priv_key.length = data->key_len;
    output_pub_key.length = data->key_len;
    output_sig.length = data->key_len;

    /* 1. hash init */
    sample_chk_expr_goto(ss_mpi_cipher_hash_init(), TD_SUCCESS, __EXIT_FREE__);

    /* 2. hash create */
    hash_attr.is_long_term = TD_FALSE;
    hash_attr.hash_type = data->hash_type;
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &hash_attr), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. hash update */
    src_buf.virt_addr = input_msg;
    if (ss_mpi_cipher_hash_update(hash_handle, &src_buf, sizeof(input_msg)) != TD_SUCCESS) {
        ss_mpi_cipher_hash_destroy(hash_handle);
        goto __HASH_DEINIT__;
    }

    /* 4. hash finish */
    if (ss_mpi_cipher_hash_finish(hash_handle, input_hash_data, data->key_len, &output_hash_len) != TD_SUCCESS) {
        ss_mpi_cipher_hash_destroy(hash_handle);
        goto __HASH_DEINIT__;
    }

    /* 5. pke init */
    sample_chk_expr_goto(ss_mpi_cipher_pke_init(), TD_SUCCESS, __HASH_DEINIT__);

    /* 6. get key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &output_priv_key, &output_pub_key),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 7. Check Dot */
    sample_chk_expr_goto(ss_mpi_cipher_pke_check_dot_on_curve(data->curve_type, &output_pub_key, &is_on_curve),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 8. ECC Sign. */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecdsa_sign(data->curve_type, &output_priv_key, &input_hash, &output_sig),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 9. ECC Verify. */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecdsa_verify(data->curve_type, &output_pub_key,
        &input_hash, &output_sig), TD_SUCCESS, __PKE_DEINIT__);

    ss_mpi_cipher_pke_deinit();

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    memset_s(d, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
__EXIT_FREE__:
    free(input_hash_data);
    input_hash_data = TD_NULL;
    return ret;
}

/* RFC8032 eddsa sign */
static td_s32 run_eddsa_sign_sample(ecc_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 d[MAX_ECC_LENGTH] = {0};
    td_u8 x[MAX_ECC_LENGTH] = {0};
    td_u8 y[MAX_ECC_LENGTH] = {0};
    td_u8 r[MAX_ECC_LENGTH] = {0};
    td_u8 s[MAX_ECC_LENGTH] = {0};
    td_u8 message[MAX_HASH_LEN] = {0};

    sample_chk_expr_return(get_random_data(message, sizeof(message)), TD_SUCCESS);

    drv_pke_data priv_key = {
        .data = d,
    };
    drv_pke_ecc_point pub_key = {
        .x = x,
        .y = y,
    };
    drv_pke_msg msg = {
        .data = message,
        .length = MAX_HASH_LEN,
        .buf_sec = DRV_PKE_BUF_NONSECURE
    };
    drv_pke_ecc_sig output_sig = {
        .r = r,
        .s = s,
    };

    priv_key.length = data->key_len;
    pub_key.length = data->key_len;
    output_sig.length = data->key_len;

    /* 1. pke init */
    sample_chk_expr_return(ss_mpi_cipher_pke_init(), TD_SUCCESS);

    /* 2. get key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &priv_key, &pub_key),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 3. EDDSA sign */
    sample_chk_expr_goto(ss_mpi_cipher_pke_eddsa_sign(data->curve_type, &priv_key, &msg, &output_sig),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 4. EDDSA verify. */
    sample_chk_expr_goto(ss_mpi_cipher_pke_eddsa_verify(data->curve_type, &pub_key, &msg, &output_sig),
        TD_SUCCESS, __PKE_DEINIT__);

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
    memset_s(d, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    return ret;
}

static td_s32 sample_ecc_cal(td_void)
{
    td_u32 i;
    td_s32 ret;
    td_u32 num = (td_u32)(sizeof(g_ecdsa_data) / sizeof(g_ecdsa_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_ecc_generate_key_sample(&g_ecdsa_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test generate key %s failed ************\n", g_ecdsa_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test generate key %s success ************\n", g_ecdsa_data[i].ecc_name);
    }
    for (i = 0; i < num; i++) {
        ret = run_ecc_ecdh_sample(&g_ecdsa_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test ECDH %s failed ************\n", g_ecdsa_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test ECDH %s success ************\n", g_ecdsa_data[i].ecc_name);
    }
    for (i = 0; i < num; i++) {
        ret = run_ecc_sign_sample(&g_ecdsa_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test ECC %s sign & verify failed ************\n", g_ecdsa_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test ECC %s sign & verify success ************\n", g_ecdsa_data[i].ecc_name);
    }
    num = (td_u32)(sizeof(g_eddsa_data) / sizeof(g_eddsa_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_eddsa_sign_sample(&g_eddsa_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test EDDSA %s sign & verify failed ************\n", g_eddsa_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test EDDSA %s sign & verify success ************\n", g_eddsa_data[i].ecc_name);
    }
    return TD_SUCCESS;
}

td_s32 sample_ecc(td_void)
{
    td_s32 ret;
    sample_log("************ test ECC ************\n");
    ret = sample_ecc_cal();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    sample_log("************ test ECC succeed ************\n");
    return TD_SUCCESS;
}
