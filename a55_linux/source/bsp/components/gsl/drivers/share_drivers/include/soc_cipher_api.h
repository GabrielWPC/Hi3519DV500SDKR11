/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef SOC_CIPHER_API_H
#define SOC_CIPHER_API_H

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */

#define EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES 16
#define EXT_DRV_CIPHER_SM4_IV_LEN_IN_BYTES 16
#define EXT_DRV_CIPHER_TDES_IV_LEN_IN_BYTES 8
#define EXT_DRV_CIPHER_LEA_IV_LEN_IN_BYTES 16
#define EXT_DRV_CIPHER_AES_BLOCK_SIZE 16

typedef enum {
	EXT_DRV_CIPHER_LPC_SYMC_LOW_POWER,
	EXT_DRV_CIPHER_LPC_HASH_LOW_POWER,
	EXT_DRV_CIPHER_LPC_INVALID = 0xffffffff,
} ext_drv_cipher_lpc;

typedef enum {
	EXT_DRV_CIPHER_DMA_MODE_ADDR_INCREASED = 0x00,
	EXT_DRV_CIPHER_DMA_MODE_ADDR_FIXED,
	EXT_DRV_CIPHER_DMA_MODE_MAX,
	EXT_DRV_CIPHER_DMA_MODE_INVALID = 0xffffffff,
} ext_drv_cipher_dma_mode;

typedef enum {
	EXT_DRV_CIPHER_BUF_NONSECURE = 0x00,
	EXT_DRV_CIPHER_BUF_SECURE,
	EXT_DRV_CIPHER_BUF_INVALID = 0xffffffff,
} ext_drv_cipher_buffer_secure;

/* * Key length */
typedef enum {
	EXT_DRV_CIPHER_KEY_64BIT =  0x0, /* *< 64-bit key for the DES algorithm */
	EXT_DRV_CIPHER_KEY_128BIT = 0x1, /* *< 128-bit key for the AES/LEA/SM4/TDES 2key algorithm */
	EXT_DRV_CIPHER_KEY_192BIT = 0x2, /* *< 192-bit key for the AES/LEA/TDES 3key algorithm */
	EXT_DRV_CIPHER_KEY_256BIT = 0x3, /* *< 256-bit key for the AES/LEA algorithm */
	EXT_DRV_CIPHER_KEY_LENGTH_MAX,
	EXT_DRV_CIPHER_KEY_LENGTH_INVALID = 0xffffffff,
} ext_drv_cipher_key_length;

/* * Key parity attribute, valid when key length is not more than 128bit */
typedef enum {
	EXT_DRV_CIPHER_KEY_EVEN =  0x0,   /* *< even key  */
	EXT_DRV_CIPHER_KEY_ODD  =  0x1,   /* *< odd key */
	EXT_DRV_CIPHER_KEY_PARITY_MAX,
	EXT_DRV_CIPHER_KEY_PARITY_INVALID = 0xffffffff,
} ext_drv_cipher_key_parity;

/* * Cipher bit width */
typedef enum {
	EXT_DRV_CIPHER_BIT_WIDTH_1BIT = 0x0, /* *< 1-bit width */
	EXT_DRV_CIPHER_BIT_WIDTH_8BIT = 0x1, /* *< 8-bit width */
	EXT_DRV_CIPHER_BIT_WIDTH_64BIT = 0x2, /* *< 64-bit width */
	EXT_DRV_CIPHER_BIT_WIDTH_128BIT = 0x3, /* *< 128-bit width */
	EXT_DRV_CIPHER_BIT_WIDTH_MAX,
	EXT_DRV_CIPHER_BIT_WIDTH_INVALID = 0xffffffff,
} ext_drv_cipher_bit_width;

typedef enum {
	EXT_DRV_CIPHER_DFA_ENABLE = 0x05,
	EXT_DRV_CIPHER_DFA_DISABLE = 0x0a,
	EXT_DRV_CIPHER_DFA_INVALID = 0xffffffff,
} ext_drv_cipher_dfa;

/* * Cipher encryption/decryption type selecting */
typedef enum {
	EXT_DRV_CIPHER_TYPE_NORMAL = 0x0, /* Create normal channel for DMA copy or encrypt/decrypt */
	EXT_DRV_CIPHER_TYPE_REG,          /* Create register channel for encrypt/decrypt */
	EXT_DRV_CIPHER_TYPE_MAX,
	EXT_DRV_CIPHER_TYPE_INVALID = 0xffffffff,
} ext_drv_cipher_type;

typedef enum {
	EXT_DRV_CIPHER_LONG_TERM_LOCK = 0x0, /* *< Create long term lock mode */
	EXT_DRV_CIPHER_SHORT_TERM_LOCK,      /* *< Create short term lock mode */
	EXT_DRV_CIPHER_LOCK_MODE_MAX,
	EXT_DRV_CIPHER_LOCK_MODE_INVALID = 0xffffffff,
} ext_drv_lock_mode;

/* * Cipher encrypt algorithm */
typedef enum {
	EXT_DRV_CIPHER_ALG_TDES = 0x0, /* *< TDES algorithm is not secure, and we advise not to use it. */
	EXT_DRV_CIPHER_ALG_AES = 0x1,  /* *< Advanced encryption standard (AES) algorithm */
	EXT_DRV_CIPHER_ALG_SM4 = 0x2,  /* *< SM4 algorithm */
	EXT_DRV_CIPHER_ALG_LEA = 0x3,  /* *< LEA algorithm for flash online decryption and Cipher offline encryption */
	EXT_DRV_CIPHER_ALG_MAX,
	EXT_DRV_CIPHER_ALG_INVALID = 0xffffffff,
} ext_drv_cipher_alg;

/* * cipher work mode */
typedef enum {
	EXT_DRV_CIPHER_WORK_MODE_ECB,     /* *<Electronic codebook (ECB) mode, ECB has been considered insecure
                                         and it is recommended not to use it. */
	EXT_DRV_CIPHER_WORK_MODE_CBC,     /* *<Cipher block chaining (CBC) mode */
	EXT_DRV_CIPHER_WORK_MODE_CFB,     /* *<Cipher feedback (CFB) mode */
	EXT_DRV_CIPHER_WORK_MODE_OFB,     /* *<Output feedback (OFB) mode */
	EXT_DRV_CIPHER_WORK_MODE_CTR,     /* *<Counter (CTR) mode */
	EXT_DRV_CIPHER_WORK_MODE_CCM,     /* *<Counter (CCM) mode */
	EXT_DRV_CIPHER_WORK_MODE_GCM,     /* *<Counter (GCM) mode */
	EXT_DRV_CIPHER_WORK_MODE_CBC_MAC, /* *<AES-CBC-MAC */
	EXT_DRV_CIPHER_WORK_MODE_CMAC,    /* *<AES-CMAC */
	EXT_DRV_CIPHER_WORK_MODE_MAX,
	EXT_DRV_CIPHER_WORK_MODE_INVALID = 0xffffffff,
} ext_drv_cipher_work_mode;

/* * Hash algrithm type */
typedef enum {
	EXT_DRV_CIPHER_HASH_TYPE_SHA1 = 0x00, /* SHA1 is not secure, and we advise not to use it. */
	EXT_DRV_CIPHER_HASH_TYPE_SHA224,
	EXT_DRV_CIPHER_HASH_TYPE_SHA256,
	EXT_DRV_CIPHER_HASH_TYPE_SHA384,
	EXT_DRV_CIPHER_HASH_TYPE_SHA512,
	EXT_DRV_CIPHER_HASH_TYPE_SM3 = 0x10,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SHA1 = 0x20,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SHA224,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SHA256,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SHA384,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SHA512,
	EXT_DRV_CIPHER_HASH_TYPE_HMAC_SM3 = 0x30,
	EXT_DRV_CIPHER_HASH_TYPE_MAX,
	EXT_DRV_CIPHER_HASH_TYPE_INVALID = 0xffffffff,
} ext_drv_cipher_hash_type;

typedef struct {
	ext_drv_cipher_hash_type hash_type;
	td_handle keyslot; // valid when do hmac calculation
} ext_drv_cipher_hash_attr;

#define EXT_DRV_HASH_RESULT_SIZE_MAX 64 // for SHA-512
#define EXT_DRV_HASH_BLOCK_SIZE_MAX 128 // for SHA-512

typedef struct {
	ext_drv_cipher_hash_type hash_type;
	td_u32 length[2];                         /* message total length, length[0] is low address. */
	td_u8 state[EXT_DRV_HASH_RESULT_SIZE_MAX]; /* The intermediate digest state (IV). */
	td_u8 buffer[EXT_DRV_HASH_BLOCK_SIZE_MAX]; /* The data block being processed which include tail data,
                                                 tail data size is (total length % block_size). */
} ext_drv_cipher_hash_ctx;

typedef struct {
	ext_drv_cipher_type cipher_type;
	ext_drv_lock_mode lock_mode; /* long_term channel be locked when create channel, short term channel will be locked
                                   when encrypt/decrypt operation */
} ext_drv_cipher_attr;

typedef enum {
	EXT_DRV_CIPHER_IV_DO_NOT_CHANGE = 0,  /* *< IV donot change, cipher only set IV at the first time */
	EXT_DRV_CIPHER_IV_CHANGE_ONE_PKG = 1, /* *< Cipher set IV for the first package */
	EXT_DRV_CIPHER_IV_CHANGE_ALL_PKG = 2, /* *< Cipher set IV for each package */
	EXT_DRV_CIPHER_IV_CHANGE_MAX,
	EXT_DRV_CIPHER_IV_CHANGE_INVALID = 0xffffffff,
} ext_drv_cipher_iv_change_type;

typedef enum {
	EXT_DRV_CIPHER_GCM_IV_DO_NOT_CHANGE = 0, /* Data update for same hardware channel,
                                               the iv is maintained by hardware */
	EXT_DRV_CIPHER_GCM_IV_CHANGE_START = 1,  /* GCM start, iv and add are updated */
	EXT_DRV_CIPHER_GCM_IV_CHANGE_UPDATE = 2, /* GCM update, iv_ctr and iv_ghash are updated */
	EXT_DRV_CIPHER_GCM_IV_CHANGE_FINISH = 3, /* GCM get_tag, tag are calculated, can't encrypt and get_tag again */
	EXT_DRV_CIPHER_GCM_IV_CHANGE_MAX,
	EXT_DRV_CIPHER_GCM_IV_CHANGE_INVALID = 0xffffffff,
} ext_drv_cipher_gcm_iv_change_type;

typedef enum {
	EXT_DRV_CIPHER_CCM_IV_DO_NOT_CHANGE = 0, /* Data update for same hardware channel,
                                               the iv is maintained by hardware */
	EXT_DRV_CIPHER_CCM_IV_CHANGE_START = 1,  /* CCM start, iv and add are updated */
	EXT_DRV_CIPHER_CCM_IV_CHANGE_UPDATE = 2, /* CCM update, iv_ctr and iv_ghash are updated */
	EXT_DRV_CIPHER_CCM_IV_CHANGE_FINISH = 3, /* GCM get_tag, tag are calculated, can't encrypt and get_tag again */
	EXT_DRV_CIPHER_CCM_IV_CHANGE_MAX,
	EXT_DRV_CIPHER_CCM_IV_CHANGE_INVALID = 0xffffffff,
} ext_drv_cipher_ccm_iv_change_type;

/* * Structure of the cipher AES ECB/CBC/CTR/OFB/CFB/CBC-MAC/CMAC control information */
typedef struct {
	ext_drv_cipher_key_length key_len; /* < Key length */
	ext_drv_cipher_key_parity key_parity; /* <Key parity -- even/odd */
	ext_drv_cipher_bit_width bit_width; /* < Bit width for encryption or decryption, only used for CFB/OFB mode */
	ext_drv_cipher_iv_change_type iv_change_flag;
	td_u8 iv[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES]; /* < Initialization vector (IV) in/out for CBC/CTR, MAC out for
                                                    CBC-MAC */
} ext_drv_cipher_config_aes;

/* * Structure of the cipher AES CMAC final block information, if there is final block, this struct should be set */
typedef struct {
	td_u8 final_block_data[EXT_DRV_CIPHER_AES_BLOCK_SIZE];      /* !<  CMAC final block data */
	td_u32 final_block_len;                  /* CMAC final block data length, between 1 to 16 bytes */
	td_u32 finish_flag;                      /* CMAC finish flag, 1:final_block_len < 16, 0:final_block_len = 16 */
} ext_drv_cipher_cmac_final_block;

/* * Structure of the cipher AES CMAC control information */
typedef struct {
	ext_drv_cipher_key_length key_len; /* < Key length */
	ext_drv_cipher_key_parity key_parity; /* <Key parity -- even/odd */
	ext_drv_cipher_bit_width bit_width; /* < Bit width for encryption or decryption, only used for CFB/OFB mode */
	ext_drv_cipher_iv_change_type iv_change_flag;
	td_u8 iv[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES]; /* < Initialization vector (IV) MAC out for CMAC */
	ext_drv_cipher_cmac_final_block cmac_final_block; /* < CMAC final block info */
} ext_drv_cipher_config_aes_cmac;

/* * Structure of the cipher AES GCM control information */
typedef struct {
	ext_drv_cipher_key_length key_len;            /* *< Key length */
	ext_drv_cipher_key_parity key_parity;  /* <Key parity -- even/odd */
	ext_drv_cipher_gcm_iv_change_type iv_change_flag;
	td_u8 *iv;                                   /* *< Initialization vector (IV) */
	td_u32 iv_len;                               /* *< IV lenght for GCM */
	td_u8 *add;
	td_u32 add_len;              /* Associated data for GCM, it should be original length, shouldn't be padding */
	td_u32 data_len;             /* *if START, it should be original length, shouldn't be padding */
	td_u8 j0[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES];
	td_u8 iv_ctr[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES];   /* iv_in/iv_out for CTR calculation */
	td_u8 iv_ghash[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES]; /* iv_in/iv_out for GHASH calculation */
} ext_drv_cipher_config_aes_gcm;

/* * Structure of the cipher AES CCM control information */
typedef struct {
	ext_drv_cipher_key_length key_len;            /* *< Key length */
	ext_drv_cipher_key_parity key_parity;  /* <Key parity -- even/odd */
	ext_drv_cipher_ccm_iv_change_type iv_change_flag;
	td_u32 iv_len;                               /* *< IV lenght for CCM, which should be 7~13 */
	td_u8 *add;
	td_u32 add_len;                                /* *< Associated data for CCM */
	td_u8  tag_len;                                /* should be 4,6,8,10,12,14,16 */
	td_u32 total_len;
	td_u8 s0[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES];
	td_u8 iv_ctr[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES];   /* iv_in/iv_out for CTR calculation */
	td_u8 iv_mac[EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES];   /* iv_in/iv_out for CBC-MAC calculation */
} ext_drv_cipher_config_aes_ccm;

/* * Structure of the cipher TDES control information */
typedef struct {
	ext_drv_cipher_key_length key_len;  /* < Key length */
	ext_drv_cipher_key_parity key_parity;  /* <Key parity -- even/odd */
	ext_drv_cipher_bit_width bit_width; /* < Bit width for encryption or decryption, only used for CFB/OFB mode */
	ext_drv_cipher_iv_change_type iv_change_flag;
	td_u8 iv[EXT_DRV_CIPHER_TDES_IV_LEN_IN_BYTES]; /* < Initialization vector (IV), in/out for CBC */
} ext_drv_cipher_config_tdes;

/* * Structure of the cipher SM4 control information */
typedef struct {
	ext_drv_cipher_key_parity key_parity;  /* <Key parity -- even/odd */
	ext_drv_cipher_iv_change_type iv_change_flag;
	td_u8 iv[EXT_DRV_CIPHER_SM4_IV_LEN_IN_BYTES]; /* < Initialization vector (IV), in/out for CBC/CTR */
} ext_drv_cipher_config_sm4;

/* * Structure of the cipher LEA control information */
typedef struct {
	ext_drv_cipher_key_length key_len;  /* < Key length, only support 128/256bits, do not support 192bits */
	ext_drv_cipher_key_parity key_parity;  /* <Key parity -- even/odd */
	ext_drv_cipher_iv_change_type iv_change_flag;
	td_u8 iv[EXT_DRV_CIPHER_LEA_IV_LEN_IN_BYTES]; /* < Initialization vector (IV), in/out for CBC/CTR */
} ext_drv_cipher_config_lea;

/* * Expand Structure of the cipher control information */
typedef struct {
	ext_drv_cipher_alg alg;             /* < Cipher algorithm */
	ext_drv_cipher_work_mode work_mode; /* < Operating mode */
	ext_drv_cipher_dfa dfa;             /* Against DFA attack enable flag (0xa: close DFA; 0x5:open DFA). */
	/* *< Parameter for special algorithm
	    for AES ECB/CBC/CTR/OFB/CFB/CBC-MAC, the pointer should point to ext_drv_cipher_config_aes;
	    for AES_CMAC, the pointer should point to ext_drv_cipher_config_aes_cmac;
	    for AES_GCM, the pointer should point to ext_drv_cipher_config_aes_gcm;
	    for AES_CCM, the pointer should point to ext_drv_cipher_config_aes_ccm;
	    for TDES, the pointer should point to ext_drv_cipher_config_tdes;
	    for SM4, the pointer should point to ext_drv_cipher_config_sm4;
	*/
	td_void *param;
} ext_drv_cipher_config;

typedef struct {
	td_u8 *address; /* physical address, should align to 4 bytes */
	ext_drv_cipher_buffer_secure buf_sec;
} ext_drv_cipher_buf_attr;

/*
\brief Cipher initialization. it will set CORE_LP_CTRL and SPACC_CALC_CRG_CFG to enable.
\       Enable symmetric and hash core low power control register;
\       Enable SPACC algorithm low power core clock gate;
\       Malloc cipher channel resource;
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_init(void);

/*
\brief Cipher deinitialization.
\      Free cipher channel resource
\attention \n
N/A

\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_deinit(void);

/*
\brief Create Cipher handle.
\      Find an idle cipher channel to create.
\attention \n
N/A
\param[out] hcipher: get a available cipher handle.
\param[in]  cipher_attr: set the cipher type and lock mode.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_create(td_handle *hcipher, const ext_drv_cipher_attr *cipher_attr, const uintptr_t check_word);

/*
\brief Destroy the cipher channel.
\attention \n
N/A
\param[in] hcipher: The cipher handle to be destroyed.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_destroy(const td_handle hcipher);

/*
\brief Cipher memory dma copy function.
\attention \n
N/A

\param[in] dest: provide destination data, contain address/buf_type/dma_mode.the address should align to 4 bytes.
\param[in] src: provide source data in, contain address/buf_type/dma_mode.the address should align to 4 bytes.
\param[in] length: copy byte length, align to byte.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_ENV_NOT_READY     Cipher not initialized.
\retval ::EXT_ERR_CIPHER_INVALID_PARAMETER Invalid param.
\retval ::EXT_ERR_CIPHER_ADDR_NOT_ALIGNED  Addr not aligne.
\retval ::EXT_ERR_CIPHER_BUSY              Cipher busy.
\retval ::EXT_ERR_CIPHER_TIMEOUT           Cipher wait timeout.
\Note: For dcache enable mode, the dest_buf->address and length should be aligned to 32 bytes.
\      Before call this API, you should do cache_flush src_buf->address.
\      After call this API, you should do cache_inv dest_buf->address.
\see \n
N/A
*/
td_s32 uapi_drv_cipher_dma_copy(const ext_drv_cipher_buf_attr *dest, const ext_drv_cipher_buf_attr *src,
								const td_u32 length, const uintptr_t check_word);

/*
\brief Set cipher ctrl config.
\attention: for aes_ccm algorithm, the total_len should be equal to the src_len, otherwise the uapi_drv_cipher_get_tag
\ API will check it and return EXT_ERR_CIPHER_INVALID_PARAMETER. And, if you've created the cipher_handle of register
\ channel, you shouldn't set work_mode to be EXT_DRV_CIPHER_WORK_MODE_CCM here.
N/A
\param[in] hcipher: The cipher handle to be set config.
\param[in] ctrl: cipher ctrl config pointer, contain alg/dfa/interrupt_mode/work_mode/key_len/bit_width/iv.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_set_config(const td_handle hcipher, const ext_drv_cipher_config *ctrl,
								  const uintptr_t check_word);

/*
\brief Get cipher ctrl config.
\attention: before call this API, you need to have called the uapi_drv_cipher_set_config API, so that you can get the
\ right ctrl value, otherwise, the API will return EXT_ERR_CIPHER_ENV_NOT_READY.
N/A
\param[in] hcipher: The cipher handle to be get config.
\param[in] ctrl: cipher ctrl config pointer, contain alg/dfa/interrupt_mode/work_mode/key_len/bit_width/iv.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_get_config(const td_handle hcipher, ext_drv_cipher_config *ctrl, const uintptr_t check_word);

/*
\brief Attach cipher handle and keyslot.
\attention \n
N/A
\param[in] hcipher: The cipher handle to attach.
\param[in] hkeyslot: The keyslot created by keyladder.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_attach(const td_handle hcipher, const td_handle hkeyslot, const uintptr_t check_word);

/*
\brief Detach cipher handle and keyslot.
\attention \n
N/A
\param[in] hcipher: The cipher handle to detach.
\param[in] hkeyslot: The keyslot created by keyladder.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_detach(const td_handle hcipher, const td_handle hkeyslot, const uintptr_t check_word);

/*
\brief Cipher encrypt.
\attention: This API don't support CMAC & CBC_MAC work mode, for which you can call uapi_drv_cipher_mac_update API.
\ Before call this API, you need to have called the uapi_drv_cipher_set_config and uapi_drv_cipher_attach API.
N/A
\param[in] hcipher: The cipher handle to encrypt data.
\param[in] src_buf: source buffer of data in, the address should align to 4 bytes ((src_addr % 4) == 0).
\param[in] dest_buf: dest buffer of data out, the address should align to 4 bytes ((dest_addr % 4) == 0).
\param[in] length: length of the data, should align to block size (16 or 8) when update except last update.
\param[in] timeout_ms: timeout of cipher in ms. If timeout is 0,
\                      we will use the default value SYMC_WAIT_DONE_TIME_OUT_MS.
\param[in] check_word: the checksum of the parameters
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\Note: For dcache enable mode, the dest_buf->address and length should be aligned to 32 bytes.
\      Before call this API, you should do cache_flush src_buf->address.
\      After call this API, you should do cache_inv dest_buf->address.
\see \n N/A
*/
td_s32 uapi_drv_cipher_encrypt(const td_handle hcipher, const ext_drv_cipher_buf_attr *src_buf,
							   const ext_drv_cipher_buf_attr *dest_buf, const td_u32 length,
							   const td_u32 timeout_ms, const uintptr_t check_word);

/*
\brief Cipher decrypt.
\attention: This API don't support CMAC & CBC_MAC work mode and also don't support lea algorithm.
\ Before call this API, you need to have called the uapi_drv_cipher_set_config and uapi_drv_cipher_attach API.
N/A
\param[in] hcipher: The cipher handle to decrypt data.
\param[in] src_buf: source buffer of data in, the address should align to 4bytes ((src_addr % 4) == 0).
\param[in] dest_buf: dest buffer of data out, the address should align to 4 bytes ((dest_addr % 4) == 0).
\param[in] length: length of the data, should align to block size (16 or 8) when update except last update.
\param[in] timeout_ms: timeout of cipher in ms. If timeout is 0,
\                      we will use the default value SYMC_WAIT_DONE_TIME_OUT_MS.
\param[in] check_word: the checksum of the parameters
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\Note: For dcache enable mode, the dest_buf->address and length should be aligned to 32 bytes.
\      Before call this API, you should do cache_flush src_buf->address.
\      After call this API, you should do cache_inv dest_buf->address.
\see \n
N/A
*/
td_s32 uapi_drv_cipher_decrypt(const td_handle hcipher, const ext_drv_cipher_buf_attr *src_buf,
							   const ext_drv_cipher_buf_attr *dest_buf, const td_u32 length,
							   const td_u32 timeout_ms, const uintptr_t check_word);

/* *
\brief Get the tag data of CCM/GCM.
\attention: for aes_ccm algorithm, the API will check whether total_len is equal to the src_len, if not it will return
\ EXT_ERR_CIPHER_INVALID_PARAMETER. So when call uapi_drv_cipher_set_config API, make sure that the total_len is
\ equal to the src_len. And, Before call this API, you need to have also called the uapi_drv_cipher_encrypt API or
\ uapi_drv_cipher_decrypt API.
This API is used to get the tag data of CCM/GCM.
\param[in] cipher cipher handle
\param[out] tag tag data of CCM/GCM
\param[in/out] tag_len tag data length of CCM/GCM.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\retval ::EXT_ERR_CIPHER_ENV_NOT_READY  The cipher device is not initialized.
\retval ::EXT_ERR_CIPHER_INVALID_PARA  The parameter is invalid.
\see \n
N/A
*/
td_s32 uapi_drv_cipher_get_tag(td_handle hcipher, td_u8 *tag, td_u32 *tag_len, const uintptr_t check_word);

/**
\brief CBC-MAC and CMAC update.
\attention: Before call this API, you need to have called the uapi_drv_cipher_set_config and uapi_drv_cipher_attach API.
\param[in] src_buf: source buffer of data in, the address should align to 4bytes ((src_addr % 4) == 0).
\param[in] length: length of the data, should align to block size (16 or 8) except last update of CMAC.
\param[in] timeout_ms: timeout of cipher in ms. If timeout is 0,
\                      we will use the default value SYMC_WAIT_DONE_TIME_OUT_MS.
\param[in] check_word: the checksum of the parameters
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\Note: For dcache enable mode, before call this API, you should do cache_flush src_buf->address.
\see \n
N/A
*/
td_s32 uapi_drv_cipher_mac_update(const td_handle hcipher, const ext_drv_cipher_buf_attr *src_buf,
								  const td_u32 length, const td_u32 timeout_ms, const uintptr_t check_word);

/**
\brief CBC-MAC and CMAC finish.
\attention: Before call this API, you need to have called the uapi_drv_cipher_mac_update API.
N/A

\param[in/out] mac: buffer for mac value.
\param[in/out] length: length of the mac value, it's should be larget than 8 when CMAC.
\param[in] check_word: the checksum of the parameters
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\see \n
N/A
*/

td_s32 uapi_drv_cipher_mac_finish(const td_handle hcipher, td_u8 *mac, td_u32 *length, const uintptr_t check_word);

/**
\brief Cipher hash initialization.
\attention \n
\ There are only two physical hash channels.
\ For hmac channel, clear key or encrypted key should be set to hmac keyslot.
\ For hmac channel, when it is locked, it is unlocked after uapi_drv_cipher_hash_final is called.
\ For hmac channel, iv is maintained by hardware.
\ For hash channel, it is locked just before computation and unlocked after computation.
\ For hash channel, iv is maintained by driver, the output is used for the next iv. \n
N/A

\param[out] hhash: handle of hash channel.
\param[in] hash_attr: configuration of hash operation.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_hash_init(td_handle *hhash, const ext_drv_cipher_hash_attr *hash_attr,
								 const uintptr_t check_word);

/*
\brief Cipher hash update.
\attention \n
N/A
\param[in] hhash: handle of hash channel.
\param[in] src_buf: the address of hash data in, should align to 4 bytes.
\param[in] timeout_ms: timeout of hash in ms. If timeout is 0, we will use the default value HASH_TIME_OUT_MS.
\param[in] len: data length. should align to block size except last call.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.
\Note: For dcache enable mode, before call this API, you should do cache_flush src_buf->address.
\see \n
N/A
*/
td_s32 uapi_drv_cipher_hash_update(td_handle hhash, const ext_drv_cipher_buf_attr *src_buf, const td_u32 len,
								   const td_u32 timeout_ms, const uintptr_t check_word);

/*
\brief Get the final hash value, after calculate all of the data, call this API to get the final hash value and close
       the handle.If there is some reason need to interupt the calculation, this API should also be call to close the
       handle.

\attention \n
N/A

\param[in] handle:  Hash handle.
\param[out] out:  The final output hash value, and its length depends on hash type, it is 20 for sha1,
                       28 for sha224,32 for sha256 or sm3,48 for sha384,64 for sha512.
\param[in/out] out_len: the output buffer length.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_hash_final(td_handle hhash, td_u8 *out, td_u32 *out_len, const uintptr_t check_word);

/*
\brief get hash context.
\attention \n
Only for hash calculation, HMAC not support.
\param[in] hhash: handle of hash channel.
\param[out] ctx: the output context, please refer to uapi_drv_cipher_hash_ctx.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_hash_get(const td_handle hhash, ext_drv_cipher_hash_ctx *ctx, const uintptr_t check_word);

/*
\brief set hash context.
\attention \n
Only for hash calculation, HMAC not support.
\param[in] hhash: handle of hash channel.
\param[in] ctx: the input context, please refer to uapi_drv_cipher_hash_ctx.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_hash_set(const td_handle hhash, const ext_drv_cipher_hash_ctx *ctx, const uintptr_t check_word);

/*
\brief Cipher random number generate.
\attention \n
N/A

\param[out] randnum: random number get.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_trng_get_random(td_u32 *randnum);

/*
\brief Cipher sym low power contrl.
\        it will set KDF/keyladder/dma/aes/hash/milenage clock or clockgate to enable or disable at the same time.
\attention \n
N/A

\param[in] lpc: clock or clockgate want to set.
\param[in] enable: disable or enable. =TD_FALSE: disable, =TD_TRUE: enable.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_low_power_set(ext_drv_cipher_lpc lpc, td_bool enable, const uintptr_t check_word);

/*
\brief Cipher sym low power register status get.
\attention \n
N/A

\param[in] lpc: clock or clockgate register status want to get.
\param[out] enable: status get.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_low_power_get(ext_drv_cipher_lpc lpc, td_bool *enable, const uintptr_t check_word);

/**
\brief Type of cipher wait condition function provided by ROM
\attention \n
N/A

\param[in] param: params of wait condition func.
\retval ::TD_TRUE  wait condition reached.
\retval ::TD_FALSE  wait condition not reached.

\see \n
N/A
*/
typedef td_bool(*uapi_drv_cipher_wait_condition_func)(const td_void *param);

/**
\brief Type of cipher wait function provided by OS for ROM to call
\attention \n
N/A

\param[in] wait: wait queue.
\param[in] func: wait condition function.
\param[in] param: params of func.
\param[in] timeout_ms: timeout val in ms.
\retval ::not zero: wait event happened.
\retval ::zero: wait event timeout.

\see \n
N/A
*/
typedef td_s32(*osal_cipher_wait_timeout_interruptible)(const td_void *wait, uapi_drv_cipher_wait_condition_func func,
														const td_void *param, const td_u32 timeout_ms);

/**
\brief Register cipher wait_func callback.
\attention \n
This API is provided by ROM to register cipher wait_func callback for service.

\param[in] wait: wait event queue.
\param[in] wait_func: callback function.
\param[in] timeout_ms: timeout val in ms.
\param[in] check_word: checksum of params.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_register_cipher_wait_func(const td_void *wait,
												 osal_cipher_wait_timeout_interruptible wait_func, td_u32 timeout_ms, uintptr_t check_word);

/**
\brief Type of hash wait condition function provided by ROM
\attention \n
N/A

\param[in] param: params of wait condition func.
\retval ::TD_TRUE  wait condition reached.
\retval ::TD_FALSE  wait condition not reached.

\see \n
N/A
*/
typedef td_bool(*uapi_drv_hash_wait_condition_func)(const td_void *param);

/**
\brief Type of hash wait function provided by OS for ROM to call
\attention \n
N/A

\param[in] wait: wait queue.
\param[in] func: wait condition function.
\param[in] param: params of func.
\param[in] timeout_ms: timeout val in ms.
\retval ::not zero: wait event happened.
\retval ::zero: wait event timeout.

\see \n
N/A
*/
typedef td_s32(*osal_hash_wait_timeout_interruptible)(const td_void *wait, uapi_drv_hash_wait_condition_func func,
													  const td_void *param, const td_u32 timeout_ms);

/**
\brief Register hash wait_func callback.
\attention \n
This API is provided by ROM to register hash wait_func callback for service.

\param[in] wait: wait event queue.
\param[in] wait_func: callback function.
\param[in] timeout_ms: timeout val in ms.
\param[in] check_word: checksum of params.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_cipher_register_hash_wait_func(const td_void *wait,
											   osal_hash_wait_timeout_interruptible wait_func, td_u32 timeout_ms, uintptr_t check_word);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */

#endif
