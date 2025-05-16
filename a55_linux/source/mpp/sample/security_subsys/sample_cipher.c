/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sample_utils.h"
#include "sample_func.h"

enum {
    TEST_SYMC_CLEARKEY = 1,
    TEST_SYMC_ROOTKEY,
    TEST_SYMC_MAC,
    TEST_HASH,
    TEST_PBKDF2,
    TEST_TRNG,
    TEST_RSA,
    TEST_ECC,
    TEST_SM4,
    TEST_SM3,
    TEST_SM2
};

static void sample_cipher_usage(void)
{
    printf("\n##########----Run a cipher sample:for example, ./sample_cipher 1----###############\n");
    printf("    You can selset a cipher sample to run as fllow:\n"
           "    [1] SYMC ClearKey\n"
           "    [2] SYMC RootKey\n"
           "    [3] SYMC MAC\n"
           "    [4] HASH & HMAC\n"
           "    [5] PBKDF2\n"
           "    [6] TRNG\n"
           "    [7] RSA\n"
           "    [8] ECC\n"
           "    [9] SM4\n"
           "    [10] SM3\n"
           "    [11] SM2\n");
}

#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char* argv[])
#else
td_s32 main(td_s32 argc, td_char* argv[])
#endif
{
    /* All data in the sample is randomly generated. You need to replace the data_init function in each .c file. */
    td_s32 ret;
    td_u32 func_num;

    if (argc != 2) { /* 2: 2 arg num */
        sample_cipher_usage();
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) {    /* 2: 2 chars */
        sample_cipher_usage();
        return TD_FAILURE;
    }

    func_num = strtol(argv[1], TD_NULL, 0);
    switch (func_num) {
        case TEST_SYMC_CLEARKEY:
            ret = sample_symc_clearkey();
            break;
        case TEST_SYMC_ROOTKEY:
            ret = sample_symc_rootkey();
            break;
        case TEST_SYMC_MAC:
            ret = sample_symc_mac();
            break;
        case TEST_HASH:
            ret = sample_hash();
            break;
        case TEST_PBKDF2:
            ret = sample_pbkdf2();
            break;
        case TEST_TRNG:
            ret = sample_trng();
            break;
        case TEST_RSA:
            ret = sample_rsa();
            break;
        case TEST_ECC:
            ret = sample_ecc();
            break;
        case TEST_SM4:
            ret = sample_sm4_clearkey();
            break;
        case TEST_SM3:
            ret = sample_sm3();
            break;
        case TEST_SM2:
            ret = sample_sm2();
            break;
        default:
            sample_log("the index is invalid!: %u\n", func_num);
            sample_cipher_usage();
            return TD_FAILURE;
    }
    if (ret != TD_SUCCESS) {
        sample_log("fund_id %u failed\n", func_num);
    }
#ifdef __LITEOS__
    return ret;
#else
    exit(ret);
#endif
}

