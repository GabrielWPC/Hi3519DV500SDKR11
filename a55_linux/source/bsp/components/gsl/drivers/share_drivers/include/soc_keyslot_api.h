/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef SOC_KEYSLOT_API_H
#define SOC_KEYSLOT_API_H

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* ************************** Structure Definition *************************** */
/* \addtogroup      KEYSLOT  */
/* @{ */ /* * <!-- [KEYSLOT] */

/* Define the keyslot type. */
typedef enum {
	EXT_DRV_KEYSLOT_TYPE_MCIPHER = 0x00,
	EXT_DRV_KEYSLOT_TYPE_HMAC,
	EXT_DRV_KEYSLOT_TYPE_MAX,
	EXT_DRV_KEYSLOT_TYPE_INVALID = 0xffffffff,
} ext_drv_keyslot_type;

/* @} */ /* <!-- ==== Structure Definition end ==== */

/* ****************************** API declaration **************************** */
/* \addtogroup      KEYSLOT  */
/* @{ */ /* * <!-- [KEYSLOT] */

/*
\brief Initializes the keyslot module.
\attention \n
Before calling any other api in keyslot, you must call this function first.
\param  N/A
\retval EXT_SEC_SUCCESS
\retval EXT_SEC_FAILURE

\see \n
N/A
*/
td_s32 uapi_drv_keyslot_init(td_void);

/*
\brief Deinitializes the keyslot module.
\attention \n
Call the keyslot deinitialization API to destroy all keyslot instances locked by the Current CPU.
\param  N/A
\retval EXT_SEC_SUCCESS              Success
\retval EXT_SEC_FAILURE              Failure
\retval EXT_ERR_KM_ENV_NOT_READY     Keyslot not initialized.
\retval EXT_ERR_KM_KSLOT_BUSY        Keyslot busy.
\retval EXT_ERR_KM_TIMEOUT           Timeout.

\see \n
N/A
*/
td_s32 uapi_drv_keyslot_deinit(td_void);

/*
\brief Creates a keyslot instance.
\param[in] keyslot_type The keyslot type that created the instance.
\param[out] hkeyslot Pointer to the handle of an allocated instance.
\retval EXT_SEC_SUCCESS              Success
\retval EXT_SEC_FAILURE              Failure
\retval EXT_ERR_KM_ENV_NOT_READY     Keyslot not initialized.
\retval EXT_ERR_KM_INVALID_PARAMETER Invalid param.
\retval EXT_ERR_KM_KSLOT_BUSY        Keyslot busy.

\see \n
N/A
*/
td_s32 uapi_drv_keyslot_create(td_handle *hkeyslot, ext_drv_keyslot_type keyslot_type);

/*
\brief Destroys a keyslot instance.
\attention \n
N/A
\param[in] hkeyslot instance handle.
\retval EXT_SEC_SUCCESS              Success
\retval EXT_SEC_FAILURE              Failure
\retval EXT_ERR_KM_ENV_NOT_READY     Keyslot not initialized.
\retval EXT_ERR_KM_INVALID_PARAMETER Invalid param.
\retval EXT_ERR_KM_KSLOT_BUSY        Keyslot busy.
\retval EXT_ERR_KM_TIMEOUT           Timeout.

\see \n
N/A
*/
td_s32 uapi_drv_keyslot_destroy(td_handle hkeyslot);

/* @} */ /* <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI_KEYSLOT_API_H */
