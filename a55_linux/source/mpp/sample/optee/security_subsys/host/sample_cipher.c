/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

#include "security_subsys_ta.h"

enum TEST_ERRNO {
    TEST_RET_SUCCESS            = 0,
    TEST_RET_FAILURE            = 0xffffffff,
};

static TEEC_Context g_tee_ctx = {0};
static TEEC_Session g_tee_sess = {0};

/*
* Condition Check.
*/
#define ca_test_chk_ret(cond, ret) do {    \
    if (cond) {                         \
        printf(#cond);                \
        printf("\n");                 \
        return ret;                     \
    }                                   \
} while (0)

#define ca_test_chk_ret_print(cond, ret, real_ret) do {    \
    if (cond) {                         \
        printf(#cond);                \
        printf("\n");                 \
        printf("ret is 0x%x\n", real_ret);        \
        return ret;                     \
    }                                   \
} while (0)

#define ca_test_chk_goto(cond, label) do { \
    if (cond) {                         \
        printf(#cond);                \
        printf("\n");                 \
        goto label;                     \
    }                                   \
} while (0)

int tee_env_init(void)
{
    unsigned int err_origin = 0;
    TEEC_UUID uuid = TA_SECURITY_SUBSYS_UUID;
    /* Initialize a Context Connecting to the TEE. */
    ca_test_chk_ret(TEEC_InitializeContext(NULL, &g_tee_ctx) != TEEC_SUCCESS, 1);

    /* Open a Session to the Cipher Test TA. */
    ca_test_chk_goto(TEEC_OpenSession(&g_tee_ctx, &g_tee_sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin)
        != TEEC_SUCCESS, error_exit);

    return TEST_RET_SUCCESS;
error_exit:
    TEEC_FinalizeContext(&g_tee_ctx);
    return TEST_RET_FAILURE;
}

static int send_cmd_to_tee(unsigned int sample_cmd)
{
    unsigned int ret = TEST_RET_SUCCESS;
    unsigned int err_origin = 0;
    TEEC_Operation op = {0};

    op.started = 1;
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    ca_test_chk_ret((ret = TEEC_InvokeCommand(&g_tee_sess, sample_cmd, &op, &err_origin)) != TEEC_SUCCESS,
        TEST_RET_FAILURE);

    return TEST_RET_SUCCESS;
}

int tee_env_deinit(void)
{
    TEEC_CloseSession(&g_tee_sess);
    TEEC_FinalizeContext(&g_tee_ctx);

    return 0;
}

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

int main(int argc, char **argv)
{
    unsigned int func_num;

    if (argc != 2) { /* 2: 2 arg num */
        sample_cipher_usage();
        return TEST_RET_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) {  /* 2: 2 chars */
        sample_cipher_usage();
        return TEST_RET_FAILURE;
    }

    ca_test_chk_ret(tee_env_init() != TEST_RET_SUCCESS, TEST_RET_FAILURE);

    func_num = strtol(argv[1], NULL, 0);
    if (func_num >= TA_CIPHER_SAMPLE_CMD_COUNT) {
        printf("the index is invalid!: %u\n", func_num);
        sample_cipher_usage();
        tee_env_deinit();
        return TEST_RET_FAILURE;
    }
    send_cmd_to_tee(func_num);

    tee_env_deinit();

    return TEST_RET_SUCCESS;
}