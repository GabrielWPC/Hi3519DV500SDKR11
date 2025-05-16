/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <assert.h>
#include "securec.h"

#include "sample_utils.h"
#include "ss_mpi_cipher.h"
#include "ss_mpi_km.h"
#include "ss_mpi_otp.h"
#include "sample_func.h"

#define QUANTILE_TWO       2
#define QUANTILE_THREE     3
#define DATA_SIZE        512

typedef struct {
    const td_char *hash_name;
    td_u8 message[DATA_SIZE];
    td_u32 message_length;
    crypto_hash_attr hash_attr;
    td_u32 hash_length;
} hash_data_t;

typedef struct {
    const td_char *hash_name;
    td_u8 key[HMAC_KEY_LEN];
    td_u32 key_length;
    td_u8 message[DATA_SIZE];
    td_u32 message_length;
    crypto_hash_attr hash_attr;
    td_u32 hash_length;
    km_klad_hmac_type klad_hmac_type;
} hmac_data_t;

static hash_data_t g_hash_data[] = {
    {
        .hash_name = "hash-sm3", .message_length = DATA_SIZE, .hash_length = SM3_HASH_LEN,
        .hash_attr = {
            .hash_type = CRYPTO_HASH_TYPE_SM3,
            .is_long_term = TD_FALSE
        }
    }
};

static hmac_data_t g_hmac_data[] = {
    {
        .hash_name = "hmac-sm3", .message_length = DATA_SIZE,
        .hash_length = SM3_HASH_LEN, .key_length = HMAC_KEY_LEN,
        .hash_attr = {
            .hash_type = CRYPTO_HASH_TYPE_HMAC_SM3,
            .is_long_term = TD_FALSE
        },
        .klad_hmac_type = KM_KLAD_HMAC_SM3
    }
};

static td_s32 hash_set_clear_key(crypto_handle keyslot_handle, td_u8 *key, td_u32 keylen,
    km_klad_hmac_type klad_hmac_type)
{
    td_s32 ret = TD_SUCCESS;
    crypto_handle klad_handle = 0;
    td_u32 offset = 0x12;
    td_u8 tee_enable = 0;
    km_klad_attr klad_attr = {
        .key_cfg = {
            .engine = KM_CRYPTO_ALG_HMAC_SM3,
            .decrypt_support = TD_TRUE,
            .encrypt_support = TD_TRUE
        }
    };
    km_klad_clear_key clear_key = {
        .hmac_type = klad_hmac_type,
        .key = key,
        .key_size = keylen
    };
    (td_void)ss_mpi_otp_init();
    (td_void)ss_mpi_otp_read_byte(offset, &tee_enable);
    (td_void)ss_mpi_otp_deinit();
    if (tee_enable == 0x42) {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_ENABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_TRUE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_FALSE;
    } else {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_DISABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_TRUE;
    }

    /* 1. klad create handle */
    sample_chk_expr_return(ss_mpi_klad_create(&klad_handle), TD_SUCCESS);

    /* 2. klad set attr for clear key */
    sample_chk_expr_goto(ss_mpi_klad_set_attr(klad_handle, &klad_attr), TD_SUCCESS, __KLAD_DESTORY__);

    /* 3. attach klad handle & kslot handle */
    sample_chk_expr_goto(ss_mpi_klad_attach(klad_handle, KM_KLAD_DEST_TYPE_HMAC, keyslot_handle),
        TD_SUCCESS, __KLAD_DESTORY__);

    /* 4. set clear key */
    sample_chk_expr_goto(ss_mpi_klad_set_clear_key(klad_handle, &clear_key), TD_SUCCESS, __KLAD_DETACH__);

__KLAD_DETACH__:
    ss_mpi_klad_detach(klad_handle, KM_KLAD_DEST_TYPE_HMAC, keyslot_handle);
__KLAD_DESTORY__:
    ss_mpi_klad_destroy(klad_handle);
    return ret;
}

static td_s32 hash_set_root_key(crypto_handle keyslot_handle, td_u8 *skey, td_u8 *ckey, td_u32 slen, td_u32 clen)
{
    td_s32 ret = TD_SUCCESS;
    crypto_handle klad_handle = 0;
    td_u32 offset = 0x12;
    td_u8 tee_enable = 0;
    km_klad_attr klad_attr = {
        .klad_cfg = {
            .rootkey_type = KM_KLAD_KEY_TYPE_ABRK2
        },
        .key_cfg = {
            .engine = KM_CRYPTO_ALG_HMAC_SM3,
            .decrypt_support = TD_TRUE,
            .encrypt_support = TD_TRUE
        }
    };
    km_klad_session_key klad_session_key = {
        .level = KM_KLAD_LEVEL1,
        .alg = KM_KLAD_ALG_TYPE_AES,
        .key_size = slen,
        .key = skey
    };

    km_klad_content_key klad_content_key = {
        .alg = KM_KLAD_ALG_TYPE_AES,
        .key_size = clen,
        .key = ckey
    };

    (td_void)ss_mpi_otp_init();
    (td_void)ss_mpi_otp_read_byte(offset, &tee_enable);
    (td_void)ss_mpi_otp_deinit();
    if (tee_enable == 0x42) {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_ENABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_TRUE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_FALSE;
    } else {
        klad_attr.key_sec_cfg.key_sec = KM_KLAD_SEC_DISABLE;
        klad_attr.key_sec_cfg.master_only_enable = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.dest_buf_non_sec_support = TD_TRUE;
        klad_attr.key_sec_cfg.src_buf_sec_support = TD_FALSE;
        klad_attr.key_sec_cfg.src_buf_non_sec_support = TD_TRUE;
    }

    /* 1. klad create handle */
    sample_chk_expr_return(ss_mpi_klad_create(&klad_handle), TD_SUCCESS);

    /* 2. klad set attr for rootkey */
    sample_chk_expr_goto(ss_mpi_klad_set_attr(klad_handle, &klad_attr), TD_SUCCESS, __KLAD_DESTORY__);

    /* 3. attach klad handle & kslot handle */
    sample_chk_expr_goto(ss_mpi_klad_attach(klad_handle, KM_KLAD_DEST_TYPE_HMAC, keyslot_handle),
        TD_SUCCESS, __KLAD_DESTORY__);

    /* 4. set session key */
    sample_chk_expr_goto(ss_mpi_klad_set_session_key(klad_handle, &klad_session_key), TD_SUCCESS, __KLAD_DETACH__);

    /* 5. set content key */
    sample_chk_expr_goto(ss_mpi_klad_set_content_key(klad_handle, &klad_content_key), TD_SUCCESS, __KLAD_DETACH__);

__KLAD_DETACH__:
    ss_mpi_klad_detach(klad_handle, KM_KLAD_DEST_TYPE_HMAC, keyslot_handle);
__KLAD_DESTORY__:
    ss_mpi_klad_destroy(klad_handle);
    return ret;
}

static td_s32 run_hash_sample(hash_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 result_len;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, data->message_length), TD_SUCCESS,
        __HASH_DESTROY__);

    /* 4. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len), TD_SUCCESS,
        __HASH_DESTROY__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hmac_sample(hmac_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 result_len;

    data->hash_attr.is_keyslot = TD_FALSE;
    data->hash_attr.key = data->key;
    data->hash_attr.key_len = data->key_length;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, data->message_length), TD_SUCCESS,
        __HASH_DESTROY__);

    /* 4. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len), TD_SUCCESS,
        __HASH_DESTROY__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hmac_clear_key_sample(hmac_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle = 0;
    crypto_handle keyslot_handle = 0;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 result_len;

    data->hash_attr.is_keyslot = TD_TRUE;
    data->hash_attr.is_long_term = TD_TRUE;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. km init */
    sample_chk_expr_goto(ss_mpi_km_init(), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. create keyslot handle */
    sample_chk_expr_goto(ss_mpi_keyslot_create(&keyslot_handle, KM_KEYSLOT_TYPE_HMAC), TD_SUCCESS,
        __KM_DEINIT__);

    /* 4. set clear key */
    sample_chk_expr_goto(hash_set_clear_key(keyslot_handle, data->key, data->key_length, data->klad_hmac_type),
        TD_SUCCESS, __KEYSLOT_DESTROY__);

    /* 5. hash create */
    data->hash_attr.drv_keyslot_handle = (td_handle)keyslot_handle;
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __KEYSLOT_DESTROY__);

    /* 6. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, data->message_length), TD_SUCCESS,
        __HASH_DESTROY__);

    /* 7. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len), TD_SUCCESS,
        __HASH_DESTROY__);

    ss_mpi_keyslot_destroy(keyslot_handle);
    ss_mpi_km_deinit();
    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__KEYSLOT_DESTROY__:
    ss_mpi_keyslot_destroy(keyslot_handle);
__KM_DEINIT__:
    ss_mpi_km_deinit();
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hmac_root_key_sample(hmac_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle = 0;
    crypto_handle keyslot_handle = 0;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 result_len;
    td_u8 skey[AES128_KEY_LEN];
    td_u8 ckey[AES256_KEY_LEN];

    sample_chk_expr_return(get_random_data(skey, sizeof(skey)), TD_SUCCESS);
    sample_chk_expr_return(get_random_data(ckey, sizeof(ckey)), TD_SUCCESS);
    data->hash_attr.is_keyslot = TD_TRUE;
    data->hash_attr.is_long_term = TD_TRUE;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. km init */
    sample_chk_expr_goto(ss_mpi_km_init(), TD_SUCCESS, __HASH_DEINIT__);

    /* 3. create keyslot handle */
    sample_chk_expr_goto(ss_mpi_keyslot_create(&keyslot_handle, KM_KEYSLOT_TYPE_HMAC), TD_SUCCESS,
        __KM_DEINIT__);

    /* 4. set root key */
    sample_chk_expr_goto(hash_set_root_key(keyslot_handle, skey, ckey, sizeof(skey), sizeof(ckey)),
        TD_SUCCESS, __KEYSLOT_DESTROY__);

    /* 5. hash create */
    data->hash_attr.drv_keyslot_handle = (td_handle)keyslot_handle;
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __KEYSLOT_DESTROY__);

    /* 6. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, data->message_length), TD_SUCCESS,
        __HASH_DESTROY__);

    /* 7. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len), TD_SUCCESS,
        __HASH_DESTROY__);

    ss_mpi_keyslot_destroy(keyslot_handle);
    ss_mpi_km_deinit();
    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__KEYSLOT_DESTROY__:
    ss_mpi_keyslot_destroy(keyslot_handle);
__KM_DEINIT__:
    ss_mpi_km_deinit();
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hash_clone_sample(hash_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    td_handle hash_handle_clone;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u8 out_hash_clone[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    crypto_hash_clone_ctx clone_ctx = {0};
    td_u32 message_length = data->message_length;
    td_u32 result_len;
    td_u32 result_len_clone;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle_clone, &data->hash_attr), TD_SUCCESS, __HASH_DESTROY__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, message_length / QUANTILE_TWO), TD_SUCCESS,
        __HASH_CLONE_DESTROY__);

    /* 4. clone */
    /* 4.1. get context */
    sample_chk_expr_goto(ss_mpi_cipher_hash_get(hash_handle, &clone_ctx), TD_SUCCESS, __HASH_CLONE_DESTROY__);
    /* 4.2. set context */
    sample_chk_expr_goto(ss_mpi_cipher_hash_set(hash_handle_clone, &clone_ctx), TD_SUCCESS, __HASH_CLONE_DESTROY__);

    /* 5. clone continue update */
    src_buf.virt_addr = data->message + message_length / QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf,
        message_length - message_length / QUANTILE_TWO), TD_SUCCESS, __HASH_CLONE_DESTROY__);
    src_buf.virt_addr = data->message + message_length / QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_clone, &src_buf,
        message_length - message_length / QUANTILE_TWO), TD_SUCCESS, __HASH_CLONE_DESTROY__);

    /* 6. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle_clone, out_hash_clone, out_len, &result_len_clone),
        TD_SUCCESS, __HASH_CLONE_DESTROY__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len),
        TD_SUCCESS, __HASH_DESTROY__);

    sample_chk_expr_goto_with_ret(result_len == result_len_clone, TD_TRUE, ret, TD_FAILURE, __HASH_DEINIT__);
    sample_chk_expr_goto_with_ret(memcmp(out_hash, out_hash_clone, result_len),
        0, ret, TD_FAILURE, __HASH_DEINIT__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_CLONE_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle_clone);
__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hmac_clone_sample(hmac_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    td_handle hash_handle_clone;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u8 out_hash_clone[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    crypto_hash_clone_ctx clone_ctx = {0};
    td_u32 message_length = data->message_length;
    td_u32 result_len;
    td_u32 result_len_clone;

    data->hash_attr.is_keyslot = TD_FALSE;
    data->hash_attr.key = data->key;
    data->hash_attr.key_len = data->key_length;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle_clone, &data->hash_attr), TD_SUCCESS, __HASH_DESTROY__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, message_length / QUANTILE_TWO), TD_SUCCESS,
        __HASH_CLONE_DESTROY__);

    /* 4. clone */
    /* 4.1. get context */
    sample_chk_expr_goto(ss_mpi_cipher_hash_get(hash_handle, &clone_ctx), TD_SUCCESS, __HASH_CLONE_DESTROY__);
    /* 4.2. set context */
    sample_chk_expr_goto(ss_mpi_cipher_hash_set(hash_handle_clone, &clone_ctx), TD_SUCCESS, __HASH_CLONE_DESTROY__);

    /* 5. clone continue update */
    src_buf.virt_addr = data->message + message_length / QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf,
        message_length - message_length / QUANTILE_TWO), TD_SUCCESS, __HASH_CLONE_DESTROY__);
    src_buf.virt_addr = data->message + message_length / QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_clone, &src_buf,
        message_length - message_length / QUANTILE_TWO), TD_SUCCESS, __HASH_CLONE_DESTROY__);

    /* 6. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle_clone, out_hash_clone, out_len, &result_len_clone),
        TD_SUCCESS, __HASH_CLONE_DESTROY__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len),
        TD_SUCCESS, __HASH_DESTROY__);

    sample_chk_expr_goto_with_ret(result_len == result_len_clone, TD_TRUE, ret, TD_FAILURE, __HASH_DEINIT__);
    sample_chk_expr_goto_with_ret(memcmp(out_hash, out_hash_clone, result_len),
        0, ret, TD_FAILURE, __HASH_DEINIT__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_CLONE_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle_clone);
__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hash_multi_update_sample(hash_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    td_handle hash_handle_multi;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u8 out_hash_multi[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 message_length = data->message_length;
    td_u32 result_len;
    td_u32 result_len_multi;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle_multi, &data->hash_attr), TD_SUCCESS, __HASH_DESTROY__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, message_length), TD_SUCCESS,
        __HASH_MULTI_DESTROY__);

    /* 4. hash multi update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf, message_length / QUANTILE_THREE),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    src_buf.virt_addr = data->message + message_length / QUANTILE_THREE;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf, message_length / QUANTILE_THREE),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    src_buf.virt_addr = data->message + (message_length / QUANTILE_THREE) * QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf,
        message_length - (message_length / QUANTILE_THREE) * QUANTILE_TWO), TD_SUCCESS, __HASH_MULTI_DESTROY__);

    /* 5. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle_multi, out_hash_multi, out_len, &result_len_multi),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len),
        TD_SUCCESS, __HASH_DESTROY__);

    sample_chk_expr_goto_with_ret(result_len == result_len_multi, TD_TRUE, ret, TD_FAILURE, __HASH_DEINIT__);
    sample_chk_expr_goto_with_ret(memcmp(out_hash, out_hash_multi, result_len),
        0, ret, TD_FAILURE, __HASH_DEINIT__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_MULTI_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle_multi);
__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 run_hmac_multi_update_sample(hmac_data_t *data)
{
    td_s32 ret = TD_SUCCESS;
    td_handle hash_handle;
    td_handle hash_handle_multi;
    crypto_buf_attr src_buf = {0};
    td_u8 out_hash[MAX_HASH_LEN] = {0};
    td_u8 out_hash_multi[MAX_HASH_LEN] = {0};
    td_u32 out_len = MAX_HASH_LEN;
    td_u32 message_length = data->message_length;
    td_u32 result_len;
    td_u32 result_len_multi;

    data->hash_attr.is_keyslot = TD_FALSE;
    data->hash_attr.key = data->key;
    data->hash_attr.key_len = data->key_length;

    /* 1. hash init */
    sample_chk_expr_return(ss_mpi_cipher_hash_init(), TD_SUCCESS);

    /* 2. hash create */
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle, &data->hash_attr), TD_SUCCESS, __HASH_DEINIT__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_create(&hash_handle_multi, &data->hash_attr), TD_SUCCESS, __HASH_DESTROY__);

    /* 3. hash update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle, &src_buf, message_length), TD_SUCCESS,
        __HASH_MULTI_DESTROY__);

    /* 4. hash multi update */
    src_buf.virt_addr = data->message;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf, message_length / QUANTILE_THREE),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    src_buf.virt_addr = data->message + message_length / QUANTILE_THREE;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf, message_length / QUANTILE_THREE),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    src_buf.virt_addr = data->message + (message_length / QUANTILE_THREE) * QUANTILE_TWO;
    sample_chk_expr_goto(ss_mpi_cipher_hash_update(hash_handle_multi, &src_buf,
        message_length - (message_length / QUANTILE_THREE) * QUANTILE_TWO), TD_SUCCESS, __HASH_MULTI_DESTROY__);

    /* 5. hash finish */
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle_multi, out_hash_multi, out_len, &result_len_multi),
        TD_SUCCESS, __HASH_MULTI_DESTROY__);
    sample_chk_expr_goto(ss_mpi_cipher_hash_finish(hash_handle, out_hash, out_len, &result_len),
        TD_SUCCESS, __HASH_DESTROY__);

    sample_chk_expr_goto_with_ret(result_len == result_len_multi, TD_TRUE, ret, TD_FAILURE, __HASH_DEINIT__);
    sample_chk_expr_goto_with_ret(memcmp(out_hash, out_hash_multi, result_len),
        0, ret, TD_FAILURE, __HASH_DEINIT__);

    ss_mpi_cipher_hash_deinit();
    return ret;

__HASH_MULTI_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle_multi);
__HASH_DESTROY__:
    ss_mpi_cipher_hash_destroy(hash_handle);
__HASH_DEINIT__:
    ss_mpi_cipher_hash_deinit();
    return ret;
}

static td_s32 sample_hash_cal(td_void)
{
    td_u32 i;
    td_s32 ret;
    td_u32 num = (td_u32)(sizeof(g_hash_data) / sizeof(g_hash_data[0]));
    sample_log("************ test sm3 data start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hash_sample(&g_hash_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s failed ************\n", g_hash_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s success ************\n", g_hash_data[i].hash_name);
    }
    sample_log("************ test sm3 data done ************\n");
    num = (td_u32)(sizeof(g_hmac_data) / sizeof(g_hmac_data[0]));
    sample_log("************ test hmac-sm3 data start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hmac_sample(&g_hmac_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s failed ************\n", g_hmac_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s success ************\n", g_hmac_data[i].hash_name);
    }
    sample_log("************ test hmac-sm3 data done ************\n");
    sample_log("************ test hmac-sm3 clear key start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hmac_clear_key_sample(&g_hmac_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s failed ************\n", g_hmac_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s success ************\n", g_hmac_data[i].hash_name);
    }
    sample_log("************ test hmac-sm3 clear key done ************\n");
    sample_log("************ test hmac-sm3 root key start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hmac_root_key_sample(&g_hmac_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s failed ************\n", g_hmac_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s success ************\n", g_hmac_data[i].hash_name);
    }
    sample_log("************ test hmac-sm3 root key done ************\n");
    num = (td_u32)(sizeof(g_hash_data) / sizeof(g_hash_data[0]));
    sample_log("************ test sm3 clone start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hash_clone_sample(&g_hash_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s clone failed ************\n", g_hash_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s clone success ************\n", g_hash_data[i].hash_name);
    }
    sample_log("************ test sm3 clone done ************\n");
    num = (td_u32)(sizeof(g_hmac_data) / sizeof(g_hmac_data[0]));
    sample_log("************ test hmac-sm3 clone start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hmac_clone_sample(&g_hmac_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s clone failed ************\n", g_hmac_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s clone success ************\n", g_hmac_data[i].hash_name);
    }
    sample_log("************ test hmac-sm3 clone done ************\n");
    num = (td_u32)(sizeof(g_hash_data) / sizeof(g_hash_data[0]));
    sample_log("************ test sm3 multi update start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hash_multi_update_sample(&g_hash_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s multi update failed ************\n", g_hash_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s multi update success ************\n", g_hash_data[i].hash_name);
    }
    sample_log("************ test sm3 multi update done ************\n");
    num = (td_u32)(sizeof(g_hmac_data) / sizeof(g_hmac_data[0]));
    sample_log("************ test hmac-sm3 multi update start ************\n");
    for (i = 0; i < num; i++) {
        ret = run_hmac_multi_update_sample(&g_hmac_data[i]);
        if (ret != TD_SUCCESS) {
            sample_err("************ test %s multi update failed ************\n", g_hmac_data[i].hash_name);
            return ret;
        }
        sample_log("************ test %s multi update success ************\n", g_hmac_data[i].hash_name);
    }
    sample_log("************ test hmac-sm3 multi update done ************\n");
    return TD_SUCCESS;
}

static td_s32 data_init(td_void)
{
    td_u32 i;
    td_u32 num = 0;
    /* 1. init g_hash_data */
    num = (td_u32)(sizeof(g_hash_data) / sizeof(g_hash_data[0]));
    for (i = 0; i < num; i++) {
        sample_chk_expr_return(get_random_data(g_hash_data[i].message, sizeof(g_hash_data[i].message)), TD_SUCCESS);
    }
    /* 2. init g_hmac_data */
    num = (td_u32)(sizeof(g_hmac_data) / sizeof(g_hmac_data[0]));
    for (i = 0; i < num; i++) {
        sample_chk_expr_return(get_random_data(g_hmac_data[i].message, sizeof(g_hmac_data[i].message)), TD_SUCCESS);
        sample_chk_expr_return(get_random_data(g_hmac_data[i].key, sizeof(g_hmac_data[i].key)), TD_SUCCESS);
    }
    return TD_SUCCESS;
}

static td_void data_deinit(td_void)
{
    td_u32 i;
    td_u32 num = 0;
    /* 1. clear the key in g_hmac_data */
    num = (td_u32)(sizeof(g_hmac_data) / sizeof(g_hmac_data[0]));
    for (i = 0; i < num; i++) {
        memset_s(g_hmac_data[i].key, sizeof(g_hmac_data[i].key), 0, sizeof(g_hmac_data[i].key));
    }
}

td_s32 sample_sm3(td_void)
{
    td_s32 ret;
    sample_chk_expr_return(data_init(), TD_SUCCESS);
    sample_log("************ test SM3 ************\n");
    ret = sample_hash_cal();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    sample_log("************ test SM3 succeed ************\n");
    data_deinit();
    return TD_SUCCESS;
}
