/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef SOC_KLAD_API_H
#define SOC_KLAD_API_H

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */

#define EXT_DRV_KDF_SALT_LEN 32
#define EXT_DRV_KDF_PASSWORD_LEN 32

#define EXT_DRV_KDF_OTP_KEY_IOT    (0x02000000)
#define IOT_OTP_KEY(n)  (EXT_DRV_KDF_OTP_KEY_IOT | (n))

#define EXT_DRV_KDF_OTP_KEY_MRK1  IOT_OTP_KEY(0x00)
#define EXT_DRV_KDF_OTP_KEY_USK   IOT_OTP_KEY(0x01)
#define EXT_DRV_KDF_OTP_KEY_RUSK  IOT_OTP_KEY(0x02)

#define EXT_DRV_KLAD_ROOTKEY_IOT (0x03000000)
#define IOT_ROOT_KEY(n)  (EXT_DRV_KLAD_ROOTKEY_IOT | (n))

/* SBRK0, TEE Global Boot Root Key0, delivered from MRK1, soc_tee_enable and Hash of TEE Root Public Key */
#define EXT_DRV_KLAD_KEY_TYPE_SBRK0  IOT_ROOT_KEY(0x00)
/* SBRK1, TEE Global Boot Root Key1, delivered from MRK1, soc_tee_enable and Hash of TEE Root Public Key */
#define EXT_DRV_KLAD_KEY_TYPE_SBRK1  IOT_ROOT_KEY(0x01)
/* SBRK2, TEE Global Boot Root Key2, delivered from MRK1, soc_tee_enable and Hash of TEE Root Public Key */
#define EXT_DRV_KLAD_KEY_TYPE_SBRK2  IOT_ROOT_KEY(0x02)
#define EXT_DRV_KLAD_KEY_TYPE_ABRK0  IOT_ROOT_KEY(0x03) /* ABRK0, REE Global Boot Root Key0, delivered from MRK1 */
#define EXT_DRV_KLAD_KEY_TYPE_ABRK1  IOT_ROOT_KEY(0x04) /* ABRK1, REE Global Boot Root Key1, delivered from MRK1 */
#define EXT_DRV_KLAD_KEY_TYPE_ABRK2  IOT_ROOT_KEY(0x05) /* ABRK2, REE Global Boot Root Key2, delivered from MRK1 */
/* DRK0, TEE unique data decryption root key, delivered from USK */
#define EXT_DRV_KLAD_KEY_TYPE_DRK0   IOT_ROOT_KEY(0x06)
/* DRK1, REE unique data decryption root key, delivered from USK */
#define EXT_DRV_KLAD_KEY_TYPE_DRK1   IOT_ROOT_KEY(0x07)
/* RDRK0, TEE random data decryption root key, delivered from RUSK */
#define EXT_DRV_KLAD_KEY_TYPE_RDRK0  IOT_ROOT_KEY(0x08)
/* RDRK1, REE random data decryption root key, delivered from RUSK */
#define EXT_DRV_KLAD_KEY_TYPE_RDRK1  IOT_ROOT_KEY(0x09)
#define EXT_DRV_KLAD_KEY_TYPE_PSK    IOT_ROOT_KEY(0x0A) /* PSK, delivered from MRK1 */
/* ODRK0, TEE flash online decryptin root key, delivered from MRK0/1, soc_tee_enable and Hash of TEE Root Public Key */
#define EXT_DRV_KLAD_KEY_TYPE_ODRK0  IOT_ROOT_KEY(0x0B)
/* OARK0  TEE flash online authentication root key, delivered from MRK0/1,
    soc_tee_enable and Hash of TEE Root Public Key */
#define EXT_DRV_KLAD_KEY_TYPE_OARK0  IOT_ROOT_KEY(0x0C)
/* ODRK1, REE flash online decryption root key, delivered from MRK1 */
#define EXT_DRV_KLAD_KEY_TYPE_ODRK1  IOT_ROOT_KEY(0x0D)
typedef enum {
	EXT_DRV_KLAD_HMAC_SHA1 = 0x20, /* Do Not Recommend to Use */
	EXT_DRV_KLAD_HMAC_SHA224,      /* Software Kdf not support */
	EXT_DRV_KLAD_HMAC_SHA256,
	EXT_DRV_KLAD_HMAC_SHA384,
	EXT_DRV_KLAD_HMAC_SHA512,
	EXT_DRV_KLAD_HMAC_SM3 = 0x30,
	EXT_DRV_KLAD_HMAC_MAX,
	EXT_DRV_KLAD_HMAC_INVALID = 0xffffffff,
} ext_drv_klad_hmac_type;

typedef struct {
	ext_drv_klad_hmac_type  hmac_type;
	td_u8 *password;   /* Suggest use secure password, not use like all 0xff or 0x00... */
	td_u32 plen;       /* not more than block size sha1/sha256/sm3 algorithm block size is 64 bytes,
                          sha512/sha384 algorithm block size is 128 bytes */
	td_u8 *salt;       /* Suggest use secure salt, not use like all 0xff or 0x00... */
	td_u32 slen;       /* sha1/sha256/sm3 not more than block size - 13, sha512/sha384 not more than block size - 21 */
	td_u16 count;      /* The count range is between 1~0xffff, but suggest more than 1000 */
} ext_drv_kdf_pbkdf2_param;

/* Define the key security attribute. */
typedef enum {
	EXT_DRV_KLAD_SEC_DISABLE = 0,
	EXT_DRV_KLAD_SEC_ENABLE,
	EXT_DRV_KLAD_SEC_MAX,
	EXT_DRV_KLAD_SEC_INVALID = 0xffffffff,
} ext_drv_klad_sec;

/* Define the keyladder level. */
typedef enum {
	EXT_DRV_KLAD_LEVEL1 = 0,
	EXT_DRV_KLAD_LEVEL_MAX,
	EXT_DRV_KLAD_LEVEL_INVALID = 0xffffffff,
} ext_drv_klad_level;

/* Define the keyladder algorithm. */
typedef enum {
	EXT_DRV_KLAD_ALG_TYPE_TDES = 0, /* Do Not Recommend to Use */
	EXT_DRV_KLAD_ALG_TYPE_AES,
	EXT_DRV_KLAD_ALG_TYPE_SM4,
	EXT_DRV_KLAD_ALG_TYPE_MAX,
	EXT_DRV_KLAD_ALG_TYPE_INVALID = 0xffffffff,
} ext_drv_klad_alg_type;

/* Define the algorithm of crypto engine. */
typedef enum {
	EXT_DRV_CRYPTO_ALG_AES = 0x20,
	EXT_DRV_CRYPTO_ALG_LEA = 0x40,        /* Flash online decryption and Cipher offline encryption */
	EXT_DRV_CRYPTO_ALG_SM4 = 0x50,
	EXT_DRV_CRYPTO_ALG_TDES = 0x70,       /* Do Not Recommend to Use */
	EXT_DRV_CRYPTO_ALG_HMAC_SHA1 = 0xA0,  /* Do Not Recommend to Use */
	EXT_DRV_CRYPTO_ALG_HMAC_SHA2 = 0xA1,
	EXT_DRV_CRYPTO_ALG_HMAC_SM3 = 0xA2,
	EXT_DRV_CRYPTO_ALG_MAX,
	EXT_DRV_CRYPTO_ALG_INVALID = 0xffffffff,
} ext_drv_crypto_alg;

/* Define the destination type of keyladder. */
typedef enum {
	EXT_DRV_KLAD_DEST_TYPE_MCIPHER = 0x00,   /* keyslot for mcipher */
	EXT_DRV_KLAD_DEST_TYPE_HMAC,             /* keyslot for hmac */
	EXT_DRV_KLAD_DEST_TYPE_FLASH,            /* flash controller, for flash on line decryption and authentication */
	EXT_DRV_KLAD_DEST_TYPE_MAX,
	EXT_DRV_KLAD_DEST_TYPE_INVALID = 0xffffffff,
} ext_drv_klad_dest_type;

/* Define the flash online decryption key type it's valid for non-TEE platform,  it's will be ignored on TEE platform */
typedef enum {
	EXT_DRV_KLAD_FLASH_KEY_TYPE_REE_DEC = 0x00,  /* REE flash online decryption key */
	EXT_DRV_KLAD_FLASH_KEY_TYPE_TEE_DEC,         /* TEE flash online decryption key */
	EXT_DRV_KLAD_FLASH_KEY_TYPE_TEE_AUT,         /* TEE flash online authentication key */
	EXT_DRV_KLAD_FLASH_KEY_TYPE_MAX,
	EXT_DRV_KLAD_FLASH_KEY_TYPE_INVALID = 0xffffffff,
} ext_drv_klad_flash_key_type;

/* * Key parity attribute, valid when key length is not more than 128bit */
typedef enum {
	EXT_DRV_KLAD_KEY_EVEN =  0x0,   /* *< even key  */
	EXT_DRV_KLAD_KEY_ODD  =  0x1,   /* *< odd key */
	EXT_DRV_KLAD_KEY_PARITY_MAX,
	EXT_DRV_KLAD_KEY_PARITY_INVALID = 0xffffffff,
} ext_drv_klad_key_parity;

/* Define the structure of keyladder configuration. */
typedef struct {
	td_u32 rootkey_type;  /* Keyladder rootkey type, EXT_DRV_KLAD_KEY_TYPE_xxx. */
} ext_drv_klad_config;

/* Define the structure of content key security configurations. */
/* when cipher work mode is CBC_MAC, dest_buf_sec_support and dest_buf_non_sec_support cannot be both false */
typedef struct {
	ext_drv_klad_sec key_sec; /* Secure key can only be used by TEE CPU and AIDSP locked cipher and hash channel */
	/* Only the cipher or hash channel which is locked by same CPU as keyladder can use this key,
	valid only for TEE CPU and AIDSP */
	td_bool master_only_enable;
	td_bool dest_buf_sec_support;     /* The destination buffer of target engine can be secure. */
	td_bool dest_buf_non_sec_support; /* The destination buffer of target engine can be non-secure. */
	td_bool src_buf_sec_support;      /* The source buffer of target engine can be secure. */
	td_bool src_buf_non_sec_support;  /* The source buffer of target engine can be non-secure. */
} ext_drv_klad_key_secure_config;

/* Define the structure of content key configurations. */
typedef struct {
	ext_drv_crypto_alg engine; /* The content key can be used for which algorithm of the crypto engine. */
	td_bool decrypt_support;  /* The content key can be used for decrypting. */
	td_bool encrypt_support;  /* The content key can be used for encrypting. */
} ext_drv_klad_key_config;

/* Structure of keyladder extend attributes. */
typedef struct {
	ext_drv_klad_config klad_cfg;          /* The keyladder configuration, valid for harware key. */
	ext_drv_klad_key_config key_cfg;       /* The content key configuration. */
	ext_drv_klad_key_secure_config key_sec_cfg; /* The content key security configuration. */
} ext_drv_klad_attr;

/* Structure of setting session key. */
typedef struct {
	ext_drv_klad_level level;              /* the level of session key. */
	ext_drv_klad_alg_type alg;             /* the algorithm used to decrypt session key. */
	td_u32 key_size;                      /* the size of session key, should be 16. */
	td_u8 *key;   /* the session key. */
} ext_drv_klad_session_key;

/* Structure of setting content key. */
typedef struct {
	ext_drv_klad_alg_type alg;             /* The algorithm of the content key. */
	td_u32 key_size;                      /* The size of content key, should be 16 or 32. */
	td_u8 *key;                           /* The content key. */
	ext_drv_klad_key_parity key_parity;    /* Odd or even key flag. */
} ext_drv_klad_content_key;

/* Structure of sending clear key. */
typedef struct {
	ext_drv_klad_hmac_type hmac_type;      /* hmac type, only send hmac key need to config. */
	td_u32 key_size;                      /* The size of content key, 16 or 32 for cipher,
                                             not more than block size for HMAC. */
	td_u8 *key;                           /* The content key. */
	ext_drv_klad_key_parity key_parity;    /* Odd or even key flag. */
} ext_drv_klad_clear_key;

/* API LIST */

/**
\brief  Reload (de-obfuscate) OTP key MRK1/USK/RUSK to kdf module.
\attention \n
If MRK1/USK/RUSK should be used immediately after they are burned into OTP, should:
Call uapi_drv_otp_update() first, then call uapi_drv_kdf_update()

\param[in] otp_key: please refer to EXT_DRV_KDF_OTP_KEY_xxx

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.
\retval ::EXT_ERR_KM_TIMEOUT            Timeout.

\see \n
uapi_drv_otp_update()
*/
td_s32 uapi_drv_kdf_update(const td_u32 otp_key);

/**
\brief use kdf module to generate soft key.
\attention \n
N/A

\param[in] param: give password/salt and count data to set. (data->cnt != 0)
\param[out] out : generate soft key output, 32 bytes.
\param[in] out_len: describe the out buffer size.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_kdf_pbkdf2(const ext_drv_kdf_pbkdf2_param *param, td_u8 *out, const td_u32 out_len,
						   const uintptr_t check_word);

/**
\brief Cipher crate klad handle.
\attention \n
N/A
Before calling any other api in klad, you must call this function first.
\param[out] hklad : klad handle.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_klad_create(td_handle *hklad);

/**
\brief Cipher destroy klad handle.
\attention \n
N/A

\param[in] hklad : klad handle.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_destroy(const td_handle hklad);

/**
\brief Cipher attach klad handle.
\attention \n
N/A
\param[in] hklad : klad handle.
\param[in] dest : attach destination type of keyladder.
\param[in] hkeyslot : when destination is mcipher or hmac, hkeyslot is the handle of keyslot.
\param[in] check_word: the checksum of the parameters.
    when destination is flash controller, hkeyslot is ext_drv_klad_flash_key_type
    when destination are others, hkeyslot is ignored.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_attach(const td_handle hklad, const ext_drv_klad_dest_type dest, const td_handle hkeyslot,
							const uintptr_t check_word);

/**
\brief Cipher detach klad handle.
\attention \n
N/A
\param[in] hklad : klad handle.
\param[in] dest : detach destination type of keyladder.
\param[in] hkeyslot : when destination is mcipher or hmac, hkeyslot is the handle of keyslot.
\param[in] check_word: the checksum of the parameters.
    when destination is flash controller, hkeyslot is ext_drv_klad_flash_key_type
    when destination are others, hkeyslot is ignored.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_detach(const td_handle hklad, const ext_drv_klad_dest_type dest, const td_handle hkeyslot,
							const uintptr_t check_word);

/**
\brief Cipher set keyladder extend attributes.
\attention \n
N/A
\param[in] hklad : klad handle.
\param[in] attr : keyladder extend attributes.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_set_attr(const td_handle hklad, const ext_drv_klad_attr *attr, const uintptr_t check_word);

/**
\brief Cipher get keyladder attributes.
\attention \n
N/A
\param[in] hklad : klad handle.
\param[out] attr : keyladder extend attributes.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_get_attr(const td_handle hklad, ext_drv_klad_attr *attr, const uintptr_t check_word);

/**
\brief Cipher setting keyladder session key.
\attention \n
N/A
\param[in] hklad : klad handle.
\param[in] key : keyladder session key.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_set_session_key(const td_handle hklad, const ext_drv_klad_session_key *key,
									 const uintptr_t check_word);

/**
\brief Cipher setting keyladder content key.
\attention \n
    To avoid the RKP and keyladder be locked long time, HW KDF and keyladder calcation should be started after
content key is set. No need lock keyladder when send clear key.
\param[in] hklad : klad handle.
\param[in] key : keyladder content key.
\param[in] timeout_ms : keyladder timeout ms.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_set_content_key(const td_handle hklad, const ext_drv_klad_content_key *key,
									 const td_u32 timeout_ms, const uintptr_t check_word);

/**
\brief Cipher setting keyladder clear key.
\attention \n
    Support send clear key to keyslot of mcipher or hmac.
    If it's hmac key, the key size should not exceed block size.
    If the hmac key size exceed block size, the application should do hash first.
\param[in] hklad : klad handle.
\param[in] key : keyladder clear key.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_ENV_NOT_READY      Klad not initialized.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_set_clear_key(const td_handle hklad, const ext_drv_klad_clear_key *key,
								   const uintptr_t check_word);


/**
\brief Disable SBRK0, SBRK1, ABRK0.
\attention \n
    Disable SBRK0, SBRK1, ABRK0, only TEE CPU can disable SBRK0 and SBRK1.
    rootkey_type: please refer to EXT_DRV_KLAD_KEY_TYPE_xxx
\param[in] rootkey_type: the rootkey_type need to disable.
\param[in] check_word: the checksum of the parameters, check_word is inversion rootkey_type.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_rootkey_disable(const td_u32 rootkey_type, const uintptr_t check_word);

/**
 * Check the kdf/keyladder ready and the interrupt status corresponding to the channel.
 * param indicates the handle or ID of the channel at the ROM layer.
*/
typedef td_bool(*uapi_drv_kdf_wait_condition_func)(const td_void *param);

typedef td_bool(*uapi_drv_klad_wait_condition_func)(const td_void *param);

/**
 * param: provided by the ROM and used by the uapi_drv_xxx_wait_condition_func().
 * The channel handle or ID of the cipher/hash is transferred.
 * Waiting for the wait event queue, read_event()
*/
typedef td_s32(*osal_kdf_wait_timeout_interruptible)(const td_void *wait, uapi_drv_kdf_wait_condition_func func,
													 const td_void *param, const td_u32 timeout_ms);

typedef td_s32(*osal_klad_wait_timeout_interruptible)(const td_void *wait, uapi_drv_klad_wait_condition_func func,
													  const td_void *param, const td_u32 timeout_ms);

/**
\brief kdf register wait function.
\attention \n
    Wait func registration function provided by the ROM driver for the service driver.
\param[in] wait : wait event.
\param[in] wait_func : wait timeout function.
\param[in] timeout_ms : timeout.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_kdf_register_wait_func(const td_void *wait,
									   const osal_kdf_wait_timeout_interruptible wait_func, const td_u32 timeout_ms, const uintptr_t check_word);

/**
\brief klad register wait function.
\attention \n
    Wait func registration function provided by the ROM driver for the service driver.
\param[in] wait : wait event.
\param[in] wait_func : wait timeout function.
\param[in] timeout_ms : timeout.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_ERR_KM_INVALID_PARAMETER  Invalid param.

\see \n
N/A
*/
td_s32 uapi_drv_klad_register_wait_func(const td_void *wait,
										const osal_klad_wait_timeout_interruptible wait_func, const td_u32 timeout_ms, const uintptr_t check_word);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
