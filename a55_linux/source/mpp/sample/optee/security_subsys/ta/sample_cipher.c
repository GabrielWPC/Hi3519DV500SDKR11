/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "security_subsys_ta.h"
#include "ot_mpi_cipher.h"
#include "sample_utils.h"
#include "sample_func.h"

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
    DMSG("has been called");
    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
    DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
        TEE_Param __maybe_unused params[4],
        void __maybe_unused **sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE);

    DMSG("has been called");

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    /* Unused parameters */
    (void)&params;
    (void)&sess_ctx;

    /* If return value != TEE_SUCCESS the session will not be created. */
    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
    (void)&sess_ctx; /* Unused parameter */
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
            uint32_t cmd_id,
            uint32_t param_types, TEE_Param params[4])
{
    /* All data in the sample is randomly generated. You need to replace the data_init function in each .c file. */
    TEE_Result ret = TEE_SUCCESS;
    (void)&sess_ctx; /* Unused parameter */
    (void)&params; /* Unused parameter */
    if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE)) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    switch (cmd_id) {
        case TEST_SYMC_CLEARKEY:
            ret = (TEE_Result)sample_symc_clearkey();
            break;
        case TEST_SYMC_ROOTKEY:
            ret = (TEE_Result)sample_symc_rootkey();
            break;
        case TEST_SYMC_MAC:
            ret = (TEE_Result)sample_symc_mac();
            break;
        case TEST_HASH:
            ret = (TEE_Result)sample_hash();
            break;
        case TEST_PBKDF2:
            ret = (TEE_Result)sample_pbkdf2();
            break;
        case TEST_TRNG:
            ret = (TEE_Result)sample_trng();
            break;
        case TEST_RSA:
            ret = (TEE_Result)sample_rsa();
            break;
        case TEST_ECC:
            ret = (TEE_Result)sample_ecc();
            break;
        case TEST_SM4:
            ret = (TEE_Result)sample_sm4_clearkey();
            break;
        case TEST_SM3:
            ret = (TEE_Result)sample_sm3();
            break;
        case TEST_SM2:
            ret = (TEE_Result)sample_sm2();
            break;
        default:
            return TEE_ERROR_BAD_PARAMETERS;
    }

    return ret;
}
