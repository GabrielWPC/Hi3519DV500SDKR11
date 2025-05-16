/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef SOC_COMMON_API_H
#define SOC_COMMON_API_H

#include <serial_pl011.h>
#include "td_type.h"
#include "soc_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* __cplusplus */
#define SHA_256_LEN 0x20 // 0X20 BYTES = 256 Bits
#define EXT_DRV_WORD_WIDTH 4

typedef void *(*func_malloc)(const td_ulong size);
typedef void (*func_free)(void *ptr);
// typedef void (*func_serial_putc)(const td_char c);
typedef s32(*func_serial_putc)(uart_num uart_base, const s8 c);
typedef td_u32(*func_msleep)(const td_u32 ticks);
typedef void (*func_cache_flush)(uintptr_t base_addr, td_u32 size);
typedef void (*func_cache_inv)(uintptr_t base_addr, td_u32 size);

/* * struct of registered function */
typedef struct {
	func_malloc malloc;            /* Mandatory function */
	func_free free;                /* Mandatory function */
	func_serial_putc serial_putc;  /* optional function */
	func_msleep delay_ticks;       /* delay by ticks (1ms), optional */
	func_cache_flush cache_flush;  /* cache flush function */
	func_cache_inv cache_inv;      /* cache inv function */
} ext_drv_func;

#define EXT_DRV_MID_IOT_TEE_CPU          (1 << 0)
#define EXT_DRV_MID_IOT_CIPHER_TEE       (1 << 1)   /* Cipher cannel locked by TEE CPU */
#define EXT_DRV_MID_IOT_CIPHER_ACPU      (1 << 2)  /* Cipher cannel locked by ACPU */
#define EXT_DRV_MID_IOT_CIPHER_PCPU      (1 << 3)  /* Cipher cannel locked by PCPU */
#define EXT_DRV_MID_IOT_CIPHER_AIDSP     (1 << 4) /* Cipher cannel locked by AIDSP */
#define EXT_DRV_MID_IOT_HASH_TEE         (1 << 5)     /* Cipher cannel locked by TEE CPU */
#define EXT_DRV_MID_IOT_HASH_ACPU        (1 << 6)    /* Cipher cannel locked by ACPU */
#define EXT_DRV_MID_IOT_HASH_PCPU        (1 << 7)    /* Cipher cannel locked by PCPU */
#define EXT_DRV_MID_IOT_HASH_AIDSP       (1 << 8)   /* Cipher cannel locked by AIDSP */
#define EXT_DRV_MID_IOT_AIDSP            (1 << 9)
#define EXT_DRV_MID_IOT_ACPU             (1 << 10)
#define EXT_DRV_MID_IOT_EMMC             (1 << 11)
#define EXT_DRV_MID_IOT_SDIO             (1 << 12)
#define EXT_DRV_MID_IOT_USB              (1 << 13)
#define EXT_DRV_MID_IOT_TDE              (1 << 14) /* TDE */
#define EXT_DRV_MID_IOT_PCPU             (1 << 15)
#define EXT_DRV_MID_IOT_LMI              (1 << 16)
#define EXT_DRV_MID_IOT_PSDIO            (1 << 17)
#define EXT_DRV_MID_IOT_SFC              (1 << 18)
#define EXT_DRV_MID_IOT_NAND             (1 << 19)
#define EXT_DRV_MID_IOT_VDP              (1 << 20)
#define EXT_DRV_MID_IOT_AUDIO_CGRA       (1 << 21)
#define EXT_DRV_MID_IOT_AUDIO_AIAO       (1 << 22)
#define EXT_DRV_MID_IOT_AUDIO_MAD        (1 << 23)
#define EXT_DRV_MID_IOT_LPMCU            (1 << 24)
#define EXT_DRV_MID_IOT_IOMCU            (1 << 25)

/* RAPC: RAM access permission controller */
typedef struct {
	td_u32 start_addr; /* RAPC region_n start address, should align to 256bytes, means the lowest 8bit are 0 */
	td_u32 end_addr;   /* RAPC region_n end address, should align to 256bytes, means the lowest 8bit are 0 */
	td_u32 read_mid;   /* Define which module/master can read the region, each bit represents a module/master */
	td_u32 write_mid;  /* Define which module/master can write the region, each bit represents a module/master */
	td_bool enable;    /* RAPC region_n enable/disable, default is disable */
} ext_drv_rapc_config;

typedef enum {
	EXT_DRV_FAPC_MODE_NORMAL = 0,   /* Normal mode, SFC will bypass the data */
	EXT_DRV_FAPC_MODE_DEC = 2,      /* Decryption mode, SFC will decrypt the data */
	EXT_DRV_FAPC_MODE_DEC_AUTH = 3, /* Decryption and authentication mode, SFC will decrypt and verify the data */
	EXT_DRV_FAPC_MODE_MAX,
	EXT_DRV_FAPC_MODE_INVALID = 0xffffffff,
} ext_drv_fapc_mode;

/* FAPC: Flash access permission controller */
typedef struct {
	td_u32 start_addr;     /* FAPC region_n start address, should align to 256bytes, means the lowest 8bit are 0 */
	td_u32 end_addr;       /* FAPC region_n end address, should align to 256bytes, means the lowest 8bit are 0 */
	td_u32 mac_addr;       /* Define the start address of MAC value if current region work in decrypt and verify mode */
	td_u32 read_mid;       /* Define which module/master can read the region, each bit represents a module/master */
	td_u32 write_mid;      /* Define which module/master can write the region, each bit represents a module/master */
	ext_drv_fapc_mode mode; /* SFC work mode, bypass/decrypt/decrypt and verify */
	td_bool enable;        /* RAPC region_n enable/disable, default is disable */
	td_bool lock;          /* The one way locker of FAPC region_n configuration,
                              if FAPC region_n configuration is locked, it cannot be unlock */
} ext_drv_fapc_config;

/* Define the flash online decryption iv type */
typedef enum {
	EXT_DRV_FAPC_IV_TYPE_ACPU = 0x00,  /* ACPU flash online decryption iv */
	EXT_DRV_FAPC_IV_TYPE_PCPU,         /* PCPU flash online decryption iv */
	EXT_DRV_FAPC_IV_TYPE_TEE,          /* TEE flash online decryption iv */
	EXT_DRV_FAPC_IV_TYPE_MAX,
	EXT_DRV_FAPC_IV_TYPE_INVALID = 0xffffffff,
} ext_drv_fapc_iv_type;

/**
\brief Register the os functions for driver code usage.
\attention Should call this API to register malloc and free function before call driver code.
N/A

\param[in] func: Function pointer structure.

\see \n
N/A
*/
void uapi_drv_register_func(const ext_drv_func *func_list);

/**
\brief Compare cs and ct.
\attention \n
N/A

\param[in] cs: Input for compare.
\param[in] ct: Input for compare.
\param[in] count: Number of bytes for compare.
\retval ::EXT_SEC_SUCCESS                    cs is equal to ct.
\retval ::EXT_SEC_FAILURE                    cs is not equal to ct.

\see \n
N/A
*/
td_s32 memcmp_ss(const td_void *cs, const td_void *ct, td_size_t count, uintptr_t check_word);

/**
\brief Set memory to specific byte.
\attention \n
N/A

\param[in] dest: The buffer to be set.
\param[in] dest_max: Maximum size of dest buffer. This parameter need to be a const value.
\param[in] c: The byte want to set to dest buffer.
\param[in] count: Number of bytes to set. count must <= dest_max.
\param[in] check_word: checksum of the parameters. check_word = (uintptr_t)dest ^ dest_max ^ c ^ count.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.

\see \n
N/A
*/
td_s32 memset_ss(td_void *dest, td_size_t dest_max, td_u8 c, td_size_t count, uintptr_t check_word);

/**
\brief Copy buffer src to buffer dest.
\attention \n
N/A

\param[in] dest: The buffer copy to.
\param[in] dest_max: Maximum size of dest buffer. This parameter need to be a const value.
\param[in] src: The buffer to be copied.
\param[in] count: Number of bytes to copy. count must <= dest_max.
\param[in] check_word: checksum of the parameters. check_word = (uintptr_t)dest ^ dest_max ^ (uintptr_t)src ^ count.
\retval ::EXT_SEC_SUCCESS                    Call this API succussful.
\retval ::EXT_SEC_FAILURE                    Call this API fails.

\see \n
N/A
*/
td_s32 memcpy_ss(td_void *dest, td_size_t dest_max, const td_void *src, td_size_t count, uintptr_t check_word);

/*
\brief  set RAPC configuration (RAM access permission controller).
\attention \n
N/A

\param[in] region: RAPC region, 0~3.
\param[in] config: configuration
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_rapc_set_config(td_u32 region, const ext_drv_rapc_config *config, const uintptr_t check_word);

/*
\brief get RAPC configuration (RAM access permission controller).
\attention \n
N/A

\param[in] region: RAPC region, 0~3.
\param[out] config: configuration
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_rapc_get_config(td_u32 region, ext_drv_rapc_config *config, const uintptr_t check_word);

/*
\brief  set FAPC configuration (Flash access permission controller).
\attention \n
N/A

\param[in] region: FAPC region, 0~11.
\param[in] config: configuration
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_fapc_set_config(td_u32 region, const ext_drv_fapc_config *config, const uintptr_t check_word);

/*
\brief get FAPC configuration (RAM access permission controller).
\attention \n
N/A

\param[in] region: FAPC region, 0~11.
\param[out] config: configuration
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_fapc_get_config(td_u32 region, ext_drv_fapc_config *config, const uintptr_t check_word);

/*
\brief set FAPC LEA iv for online decryption.
\attention \n
N/A

\param[in] type: FAPC iv type.
\param[in] iv: FAPC iv buffer address.
\param[in] length: FAPC iv length, should be 16.
\param[in] check_word: the checksum of the parameters.
\retval ::EXT_SEC_SUCCESS  Call this API succussful.
\retval ::EXT_SEC_FAILURE  Call this API fails.

\see \n
N/A
*/
td_s32 uapi_drv_fapc_set_iv(const ext_drv_fapc_iv_type type, const td_u8 *iv, const td_u32 length,
							const uintptr_t check_word);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */

#endif /* HI_COMMON_API_H */
