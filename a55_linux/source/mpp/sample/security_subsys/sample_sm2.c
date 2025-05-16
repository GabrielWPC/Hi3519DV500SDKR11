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

#define TEST_DATA_LEN   32

/* ECC data. */
typedef struct {
    const td_char *ecc_name;
    drv_pke_ecc_curve_type curve_type;
    td_u32 key_len;
    td_u32 hash_len;
} ecc_data;

typedef struct {
    const td_char *ecc_name;
    drv_pke_ecc_curve_type curve_type;
    td_u32 key_len;
    td_u32 plain_data_len;
    td_u32 cipher_data_len;
} ecc_crypto_data;

static ecc_data g_sm2_data[] = {
    {
        .ecc_name = "sm2", .key_len = ECC_SM2_KEY_LENGTH,
        .curve_type = DRV_PKE_ECC_TYPE_SM2, .hash_len = SM3_HASH_LEN
    }
};

static ecc_crypto_data g_sm2_crypto_data[] = {
    {
        .ecc_name = "sm2-crypto", .key_len = ECC_SM2_KEY_LENGTH, .curve_type = DRV_PKE_ECC_TYPE_SM2,
        .plain_data_len = TEST_DATA_LEN, .cipher_data_len = TEST_DATA_LEN + SM2_CIPHER_ADD_LENGTH
    }
};

/* SM2 generate key */
static td_s32 run_sm2_generate_key_sample(ecc_data *data)
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

/* SM2 ecdh */
static td_s32 run_sm2_ecdh_sample(ecc_data *data)
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

/* SM2 sign */
static td_s32 run_sm2_sign_sample(ecc_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_bool is_on_curve = TD_FALSE;
    td_u8 d[MAX_ECC_LENGTH] = {0};
    td_u8 x[MAX_ECC_LENGTH] = {0};
    td_u8 y[MAX_ECC_LENGTH] = {0};
    td_u8 r[MAX_ECC_LENGTH] = {0};
    td_u8 s[MAX_ECC_LENGTH] = {0};
    td_u8 input_msg[MAX_ECC_LENGTH] = {0};
    td_u8 sm2_id_buf[MAX_ECC_LENGTH] = {0};
    td_u8 *input_hash_data;

    sample_chk_expr_return(get_random_data(input_msg, sizeof(input_msg)), TD_SUCCESS);
    sample_chk_expr_return(get_random_data(sm2_id_buf, sizeof(sm2_id_buf)), TD_SUCCESS);

    input_hash_data = malloc(data->key_len);
    sample_chk_expr_return(input_hash_data != TD_NULL, TD_TRUE);

    drv_pke_data output_priv_key = {
        .data = d,
    };
    drv_pke_ecc_point output_pub_key = {
        .x = x,
        .y = y,
    };
    drv_pke_msg msg = {
        .data = input_msg,
        .length = MAX_ECC_LENGTH,
        .buf_sec = DRV_PKE_BUF_NONSECURE
    };
    drv_pke_data input_hash = {
        .data = input_hash_data,
        .length = data->hash_len
    };
    drv_pke_ecc_sig output_sig = {
        .r = r,
        .s = s,
    };
    drv_pke_data sm2_id = {
        .data = sm2_id_buf,
        .length = MAX_ECC_LENGTH
    };

    output_priv_key.length = data->key_len;
    output_pub_key.length = data->key_len;
    output_sig.length = data->key_len;

    /* 1. pke init */
    sample_chk_expr_goto(ss_mpi_cipher_pke_init(), TD_SUCCESS, __EXIT_FREE__);

    /* 2. get key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &output_priv_key, &output_pub_key),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 3. Check Dot */
    sample_chk_expr_goto(ss_mpi_cipher_pke_check_dot_on_curve(data->curve_type, &output_pub_key, &is_on_curve),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 4. DSA Hash */
    sample_chk_expr_goto(ss_mpi_cipher_pke_sm2_dsa_hash(&sm2_id, &output_pub_key, &msg, &input_hash),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 4. ECC Sign. */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecdsa_sign(data->curve_type, &output_priv_key, &input_hash, &output_sig),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 5. ECC Verify. */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecdsa_verify(data->curve_type, &output_pub_key,
        &input_hash, &output_sig), TD_SUCCESS, __PKE_DEINIT__);

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
    memset_s(d, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
__EXIT_FREE__:
    free(input_hash_data);
    input_hash_data = TD_NULL;
    return ret;
}

/* SM2 crypto */
static td_s32 run_sm2_crypto_sample(ecc_crypto_data *data)
{
    td_s32 ret = TD_SUCCESS;
    td_bool is_on_curve = TD_FALSE;
    td_u8 d[MAX_ECC_LENGTH] = {0};
    td_u8 x[MAX_ECC_LENGTH] = {0};
    td_u8 y[MAX_ECC_LENGTH] = {0};
    td_u8 *plain_data;
    td_u8 *cipher_data;
    td_u8 *check_data;

    plain_data = malloc(data->plain_data_len);
    sample_chk_expr_return(plain_data != TD_NULL, TD_TRUE);
    cipher_data = malloc(data->cipher_data_len);
    sample_chk_expr_goto(cipher_data != TD_NULL, TD_TRUE, __EXIT_FREE_PLAIN__);
    check_data = malloc(data->plain_data_len);
    sample_chk_expr_goto(plain_data != TD_NULL, TD_TRUE, __EXIT_FREE_CIPHER__);

    sample_chk_expr_goto(get_random_data(plain_data, data->plain_data_len), TD_SUCCESS, __EXIT_FREE_CHECK__);

    drv_pke_data plain_text = {
        .length = data->plain_data_len,
        .data = plain_data
    };
    drv_pke_data cipher_text = {
        .length = data->cipher_data_len,
        .data = cipher_data
    };
    drv_pke_data check_text = {
        .length = data->plain_data_len,
        .data = check_data
    };
    drv_pke_data priv_key = {
        .data = d,
        .length = data->key_len
    };
    drv_pke_ecc_point pub_key = {
        .x = x,
        .y = y,
        .length = data->key_len
    };

    /* 1. Pke Init */
    sample_chk_expr_goto(ss_mpi_cipher_pke_init(), TD_SUCCESS, __EXIT_FREE_CHECK__);

    /* 2. Generate Key */
    sample_chk_expr_goto(ss_mpi_cipher_pke_ecc_gen_key(data->curve_type, TD_NULL, &priv_key, &pub_key),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 3. Check Dot */
    sample_chk_expr_goto(ss_mpi_cipher_pke_check_dot_on_curve(data->curve_type, &pub_key, &is_on_curve),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 4. SM2 Public Encrypt */
    sample_chk_expr_goto(ss_mpi_cipher_pke_sm2_public_encrypt(&pub_key, &plain_text, &cipher_text),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 5. SM2 Private Decrypt */
    sample_chk_expr_goto(ss_mpi_cipher_pke_sm2_private_decrypt(&priv_key, &cipher_text, &check_text),
        TD_SUCCESS, __PKE_DEINIT__);

    /* 6. Compare Result */
    ret = memcmp(plain_text.data, check_text.data, data->plain_data_len);

__PKE_DEINIT__:
    ss_mpi_cipher_pke_deinit();
    memset_s(d, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(x, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
    memset_s(y, MAX_ECC_LENGTH, 0, MAX_ECC_LENGTH);
__EXIT_FREE_CHECK__:
    free(check_data);
    check_data = TD_NULL;
__EXIT_FREE_CIPHER__:
    free(cipher_data);
    cipher_data = TD_NULL;
__EXIT_FREE_PLAIN__:
    free(plain_data);
    plain_data = TD_NULL;
    return ret;
}

static td_s32 sample_sm2_cal(td_void)
{
    td_u32 i;
    td_s32 ret;
    td_u32 num = (td_u32)(sizeof(g_sm2_data) / sizeof(g_sm2_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_sm2_generate_key_sample(&g_sm2_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test generate key %s failed ************\n", g_sm2_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test generate key %s success ************\n", g_sm2_data[i].ecc_name);
    }
    for (i = 0; i < num; i++) {
        ret = run_sm2_ecdh_sample(&g_sm2_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test ECDH %s failed ************\n", g_sm2_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test ECDH %s success ************\n", g_sm2_data[i].ecc_name);
    }
    for (i = 0; i < num; i++) {
        ret = run_sm2_sign_sample(&g_sm2_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s sign & verify failed ************\n", g_sm2_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test %s sign & verify success ************\n", g_sm2_data[i].ecc_name);
    }
    num = (td_u32)(sizeof(g_sm2_crypto_data) / sizeof(g_sm2_crypto_data[0]));
    for (i = 0; i < num; i++) {
        ret = run_sm2_crypto_sample(&g_sm2_crypto_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s crypto failed ************\n", g_sm2_crypto_data[i].ecc_name);
            return ret;
        }
        sample_log("************ test %s crypto success ************\n", g_sm2_crypto_data[i].ecc_name);
    }
    return TD_SUCCESS;
}

td_s32 sample_sm2(td_void)
{
    td_s32 ret;
    sample_log("************ test SM2 ************\n");
    ret = sample_sm2_cal();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    sample_log("************ test SM2 succeed ************\n");
    return TD_SUCCESS;
}
