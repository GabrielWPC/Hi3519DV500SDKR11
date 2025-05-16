/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef SOC_OTP_API_H
#define SOC_OTP_API_H

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */

/**
\brief  otp word read.
\attention \n
N/A

\param[in] addr: read address(0x00 ~ 0x1ff), requires 4 bytes align.
\param[out] data: read word buffer(data is put in little endian).
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_read_word(const td_u32 addr, td_u32 *data, const uintptr_t check_word);

/**
\brief  otp byte read.
\attention \n
N/A

\param[in] addr: read address(0x00 ~ 0x1ff).
\param[out] data: read byte buffer(data is put in little endian).
\param[in] chech_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_read_byte(const td_u32 addr, td_u8 *data, const uintptr_t check_word);

/**
\brief  otp byte write.
\attention \n
N/A

\param[in] addr: write address(0x00 ~ 0x1ff).
\param[in] data: write data.
\param[in] chech_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_write_byte(const td_u32 addr, const td_u8 data, const uintptr_t check_word);

/**
\brief get 16-bytes DIE_ID.
\attention \n
N/A

\param[out] die_id: output buffer for DIE_ID got.
\param[in] len: length of die_id, should not less than 16 bytes.
\param[in] chech_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_get_die_id(td_u8 *die_id, const td_u32 len, const uintptr_t check_word);

/**
\brief reload the allowed OTP memory data into shadow register.
\attention \n
This function can always update MRK1, USK and RUSK to KDF.
But for other OTP values, the update function can only work
when OTP reload is enabled.

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_update(void);

/**
\brief  enable OTP auto-standby mode.
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_standby_enable(void);

/**
\brief  disable OTP auto-standby mode.
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_otp_standby_disable(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
