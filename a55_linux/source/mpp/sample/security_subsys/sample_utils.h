/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_UTILS_H
#define SAMPLE_UTILS_H

#include <stdio.h>

#include "sample_log.h"
#include "ot_type.h"
#include "ss_mpi_sys.h"
#include "crypto_common_struct.h"
#include "crypto_pke_struct.h"

#define TEST_MULTI_PKG_NUM  16
#define IV_LEN              16
#define MAX_AAD_LEN         32
#define AES128_KEY_LEN      16
#define AES192_KEY_LEN      24
#define AES256_KEY_LEN      32
#define SM4_KEY_LEN         16
#define MAX_TAG_LEN         16
#define MAX_KEY_LEN         32
#define CCM_IV_LEN          11  /* for ccm, iv len is 7 ~ 13 */
#define GCM_IV_LEN          16  /* for gcm, iv len is 1 ~ 16 */
#define CCM_TAG_LEN         10  /* for ccm, tag len is {4, 6, 8, 10, 12, 14, 16} */
#define GCM_TAG_LEN         16  /* for gcm, tag len is 16 */
#define TEST_AAD_LEN        16
#define SESSION_KEY_LEN     16
#define CONTENT_KEY_LEN     16
#define DATA_LEN            16
#define AAD_LEN             16
#define TAG_LEN             16
#define SYMC_MAC_DATA_LEN   64
#define SYMC_MAC_LEN        16

#define SHA256_HASH_LEN     32
#define SHA384_HASH_LEN     48
#define SHA512_HASH_LEN     64
#define SM3_HASH_LEN        32
#define MAX_HASH_LEN        64
#define HMAC_KEY_LEN        32

#define MAX_SESSION_KEY     32
#define MAX_CONTENT_KEY     32
#define RSA_MAX_DATA_LEN    512
#define RSA_3072_KEY_LEN    384
#define RSA_4096_KEY_LEN    512
#define RSA_TEST_DATA_LEN   256

#define ECC_256_KEY_LENGTH     32
#define ECC_384_KEY_LENGTH     48
#define ECC_512_KEY_LENGTH     64
#define ECC_521_KEY_LENGTH     68
#define ECC_SM2_KEY_LENGTH     32
#define SM2_CIPHER_ADD_LENGTH  97
#define MAX_ECC_LENGTH         68

/**
* The function is used to generate random data for the following scenarios:
* 1. The key used in hmac.
* 2. The clear key and iv used in symmetric algorithm like AES.
* 3. The derivatin materials, session key, content key and iv used in klad.
*/
td_s32 get_random_data(td_u8 *buffer, td_u32 size);

td_void cipher_free(const crypto_buf_attr *buf_attr, const void *virt_addr);
td_s32 cipher_alloc(crypto_buf_attr *buf_attr, void **virt_addr, unsigned int size);

td_s32 get_rsa_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key, td_u32 key_len);
td_void destroy_rsa_key(drv_pke_rsa_pub_key *pub_key, drv_pke_rsa_priv_key *priv_key, td_u32 key_len);

#define to_be_processed(x) (td_void)(x)

#endif