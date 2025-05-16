/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "svp_npu_ta.h"
#include "securec.h"
#include "tee_api_types.h"

TEE_Result TA_CreateEntryPoint(void)
{
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

static void TA_SetPubKey(void)
{
    EMSG("Error! Please implement this function to set pub key!!!\n");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
        TEE_Param __maybe_unused params[4],
        void __maybe_unused **sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE,
                           TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types) {
        return TEE_ERROR_BAD_PARAMETERS;
    }
    TA_SetPubKey();
    (void)&params;
    (void)&sess_ctx;
    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
    (void)&sess_ctx;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
            uint32_t cmd_id,
            uint32_t param_types, TEE_Param params[4])
{
    (void)&sess_ctx;
    return svp_npu_ta(cmd_id, params, param_types);
}
