/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "share_drivers.h"

#define SHARE_DRIVER_ENTRY  0x5000
/* sec cipher */
#define UAPI_DRV_CIPHER_INIT                            (SHARE_DRIVER_ENTRY)
#define UAPI_DRV_CIPHER_DEINIT                          (UAPI_DRV_CIPHER_INIT + 0x4)
#define UAPI_DRV_CIPHER_CREATE                          (UAPI_DRV_CIPHER_DEINIT + 0x4)
#define UAPI_DRV_CIPHER_DESTROY                         (UAPI_DRV_CIPHER_CREATE + 0x4)
#define UAPI_DRV_CIPHER_DMA_COPY                        (UAPI_DRV_CIPHER_DESTROY + 0x4)
#define UAPI_DRV_CIPHER_SET_CONFIG                      (UAPI_DRV_CIPHER_DMA_COPY + 0x4)
#define UAPI_DRV_CIPHER_ATTACH                          (UAPI_DRV_CIPHER_SET_CONFIG + 0x4)
#define UAPI_DRV_CIPHER_DETACH                          (UAPI_DRV_CIPHER_ATTACH + 0x4)
#define UAPI_DRV_CIPHER_DECRYPT                         (UAPI_DRV_CIPHER_DETACH + 0x4)
#define UAPI_DRV_CIPHER_HASH_INIT                       (UAPI_DRV_CIPHER_DECRYPT + 0x4)
#define UAPI_DRV_CIPHER_HASH_UPDATE                     (UAPI_DRV_CIPHER_HASH_INIT + 0x4)
#define UAPI_DRV_CIPHER_HASH_FINAL                      (UAPI_DRV_CIPHER_HASH_UPDATE + 0x4)
#define UAPI_DRV_CIPHER_TRNG_GET_RANDOM                 (UAPI_DRV_CIPHER_HASH_FINAL + 0x4)
#define UAPI_DRV_CIPHER_REGISTER_CIPHER_WAIT_FUNC       (UAPI_DRV_CIPHER_TRNG_GET_RANDOM + 0x4)
/* sec keyslot */
#define UAPI_DRV_KEYSLOT_INIT                           (UAPI_DRV_CIPHER_REGISTER_CIPHER_WAIT_FUNC + 0x4)
#define UAPI_DRV_KEYSLOT_DEINIT                         (UAPI_DRV_KEYSLOT_INIT + 0x4)
#define UAPI_DRV_KEYSLOT_CREATE                         (UAPI_DRV_KEYSLOT_DEINIT + 0x4)
#define UAPI_DRV_KEYSLOT_DESTROY                        (UAPI_DRV_KEYSLOT_CREATE + 0x4)
/* sec klad */
#define UAPI_DRV_KLAD_CREATE                            (UAPI_DRV_KEYSLOT_DESTROY + 0x4)
#define UAPI_DRV_KLAD_DESTROY                           (UAPI_DRV_KLAD_CREATE + 0x4)
#define UAPI_DRV_KLAD_ATTACH                            (UAPI_DRV_KLAD_DESTROY + 0x4)
#define UAPI_DRV_KLAD_DETACH                            (UAPI_DRV_KLAD_ATTACH + 0x4)
#define UAPI_DRV_KLAD_SET_ATTR                          (UAPI_DRV_KLAD_DETACH + 0x4)
#define UAPI_DRV_KLAD_SET_SESSION_KEY                   (UAPI_DRV_KLAD_SET_ATTR + 0x4)
#define UAPI_DRV_KLAD_SET_CONTENT_KEY                   (UAPI_DRV_KLAD_SET_SESSION_KEY + 0x4)
#define UAPI_DRV_KLAD_ROOTKEY_DISABLE                   (UAPI_DRV_KLAD_SET_CONTENT_KEY + 0x4)
/* sec otp */
#define UAPI_DRV_OTP_READ_WORD                          (UAPI_DRV_KLAD_ROOTKEY_DISABLE + 0x4)
#define UAPI_DRV_OTP_READ_BYTE                          (UAPI_DRV_OTP_READ_WORD + 0x4)
#define UAPI_DRV_OTP_GET_DIE_ID                         (UAPI_DRV_OTP_READ_BYTE + 0x4)
/* sec pke */
#define UAPI_DRV_PKE_INIT                               (UAPI_DRV_OTP_GET_DIE_ID + 0x4)
#define UAPI_DRV_PKE_DEINIT                             (UAPI_DRV_PKE_INIT + 0x4)
#define UAPI_DRV_PKE_ECDSA_VERIFY                       (UAPI_DRV_PKE_DEINIT + 0x4)
#define UAPI_DRV_PKE_SM2_DSA_HASH                       (UAPI_DRV_PKE_ECDSA_VERIFY + 0x4)
/* sec register */
#define UAPI_DRV_REGISTER_FUNC                          (UAPI_DRV_PKE_SM2_DSA_HASH + 0x4)
/* mmc */
#define MMC_INIT					(UAPI_DRV_REGISTER_FUNC + 0x4)
#define MMC_READ                          		(MMC_INIT + 0x4)
#define IS_BOOTMODE					(MMC_READ + 0x4)
#define MMC_GET_CUR_MODE				(IS_BOOTMODE + 0x4)
#define MMC_SET_BOOTUP_MODE				(MMC_GET_CUR_MODE +0x4)
/* sdio */
#define COPY_FROM_SDIO					(MMC_SET_BOOTUP_MODE + 0x4)
#define SET_SDIO_POS					(COPY_FROM_SDIO + 0x4)
#define ENABLE_SDIO_DMA                                 (SET_SDIO_POS + 0x4)
/* usb */
#define SELF_USB_CHECK					(ENABLE_SDIO_DMA + 0x4)
#define COPY_FROM_USB					(SELF_USB_CHECK + 0x4)
#define USB3_DRIVER_INIT				(COPY_FROM_USB + 0x4)
#define SEND_TO_USB					(USB3_DRIVER_INIT + 0x4)
/* uart */
#define PL011_PUTC                                      (SEND_TO_USB + 0x4)
#define PL011_GETC                                      (PL011_PUTC + 0x4)
#define PL011_TSTC                                      (PL011_GETC + 0x4)
#define SERIAL_INIT                                     (PL011_TSTC + 0x4)
#define LOG_SERIAL_PUTS                         	(SERIAL_INIT + 0x4)
#define COPY_FROM_UART                                  (LOG_SERIAL_PUTS + 0x4)
/* timer */
#define TIMER_INIT                                      (COPY_FROM_UART + 0x4)
#define TIMER_DEINIT                                    (TIMER_INIT + 0x4)
#define TIMER_START                                     (TIMER_DEINIT + 0x4)
#define TIMER_GET_VAL                                   (TIMER_START + 0x4)
#define UDELAY                                          (TIMER_GET_VAL + 0x4)
#define MDELAY                         			(UDELAY + 0x4)
/* watchdog */
#define WATCHDOG_ENABLE					(MDELAY + 0x4)
#define WATCHDOG_DISABLE				(WATCHDOG_ENABLE + 0x4)
#define WATCHDOG_FEED					(WATCHDOG_DISABLE + 0x4)
/* sec mem opt */
#define MEMCMP_SS                                       (WATCHDOG_FEED + 0x4)
#define MEMCPY_SS                                       (MEMCMP_SS + 0x4)
#define MEMSET_SS                                       (MEMCPY_SS + 0x4)
#define MEMCPY_S                                        (MEMSET_SS + 0x4)
#define MEMSET_S                                        (MEMCPY_S + 0x4)
#define SAVE_CUR_POINT_SYSCNT				(MEMSET_S + 0x4)
#define SET_CLEAR_KEY						(SAVE_CUR_POINT_SYSCNT + 0x4)
#define SELF_SDIO_CHECK					(0x8a68)

/* sec cipher */
td_s32 uapi_drv_cipher_init(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_INIT;
	return ((td_s32(*)(void))((uintptr_t)entry))();
}

td_s32 uapi_drv_cipher_deinit(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_DEINIT;
	return ((td_s32(*)(void))((uintptr_t)entry))();
}

td_s32 uapi_drv_cipher_create(td_handle *hcipher, const ext_drv_cipher_attr *cipher_attr, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_CREATE;
	return ((td_s32(*)(td_handle *, const ext_drv_cipher_attr *, const uintptr_t))((uintptr_t)entry))
		   (hcipher, cipher_attr, check_word);
}

td_s32 uapi_drv_cipher_destroy(const td_handle hcipher)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_DESTROY;
	return ((td_s32(*)(td_handle))((uintptr_t)entry))(hcipher);
}

td_s32 uapi_drv_cipher_dma_copy(const ext_drv_cipher_buf_attr *dest, const ext_drv_cipher_buf_attr *src,
								const td_u32 length, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_DMA_COPY;
	return ((td_s32(*)(const ext_drv_cipher_buf_attr *, const ext_drv_cipher_buf_attr *,
					   const td_u32, const uintptr_t))((uintptr_t)entry))(dest, src, length, check_word);
}

td_s32 uapi_drv_cipher_set_config(const td_handle hcipher, const ext_drv_cipher_config *ctrl,
								  const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_SET_CONFIG;
	return ((td_s32(*)(const td_handle, const ext_drv_cipher_config *, const uintptr_t))((uintptr_t)entry))
		   (hcipher, ctrl, check_word);
}

td_s32 uapi_drv_cipher_attach(const td_handle hcipher, const td_handle hkeyslot, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_ATTACH;
	return ((td_s32(*)(const td_handle, const td_handle, const uintptr_t))((uintptr_t)entry))
		   (hcipher, hkeyslot, check_word);
}
td_s32 uapi_drv_cipher_detach(const td_handle hcipher, const td_handle hkeyslot, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_DETACH;
	return ((td_s32(*)(const td_handle, const td_handle, const uintptr_t))((uintptr_t)entry))
		   (hcipher, hkeyslot, check_word);
}

td_s32 uapi_drv_cipher_decrypt(const td_handle hcipher, const ext_drv_cipher_buf_attr *src_buf,
							   const ext_drv_cipher_buf_attr *dest_buf, const td_u32 length, \
							   const td_u32 timeout_ms, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_DECRYPT;
	return ((td_s32(*)(const td_handle, const ext_drv_cipher_buf_attr *,
					   const ext_drv_cipher_buf_attr *, const td_u32, const td_u32, const uintptr_t))((uintptr_t)entry))
		   (hcipher, src_buf, dest_buf, length, timeout_ms, check_word);
}
// int uapi_drv_cipher_get_tag()
td_s32 uapi_drv_cipher_hash_init(td_handle *hhash, const ext_drv_cipher_hash_attr *hash_attr,
								 const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_HASH_INIT;
	return ((td_s32(*)(td_handle *, const ext_drv_cipher_hash_attr *, uintptr_t))((uintptr_t)entry))
		   (hhash, hash_attr, check_word);
}

td_s32 uapi_drv_cipher_hash_update(td_handle hhash, const ext_drv_cipher_buf_attr *src_buf, const td_u32 len,
								   const td_u32 timeout_ms, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_HASH_UPDATE;
	return ((td_s32(*)(td_handle, const ext_drv_cipher_buf_attr *, const td_u32, const td_u32, const uintptr_t))
			((uintptr_t)entry))(hhash, src_buf, len, timeout_ms, check_word);
}

td_s32 uapi_drv_cipher_hash_final(td_handle hhash, td_u8 *out, td_u32 *out_len, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_HASH_FINAL;
	return ((td_s32(*)(td_handle hhash, td_u8 *, td_u32 *, const uintptr_t))((uintptr_t)entry))
		   (hhash, out, out_len, check_word);
}

td_s32 uapi_drv_cipher_trng_get_random(td_u32 *randnum)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_TRNG_GET_RANDOM;
	return ((td_s32(*)(td_u32 *))((uintptr_t)entry))(randnum);
}


td_s32 uapi_drv_cipher_register_cipher_wait_func(const td_void *wait,
												 osal_cipher_wait_timeout_interruptible wait_func, td_u32 timeout_ms, uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_CIPHER_REGISTER_CIPHER_WAIT_FUNC;
	return ((td_s32(*)(const td_void * wait, osal_cipher_wait_timeout_interruptible wait_func,
					   td_u32 timeout_ms, uintptr_t check_word))((uintptr_t)entry))
		   (wait, wait_func, timeout_ms, check_word);
}

/* sec keyslot */
td_s32 uapi_drv_keyslot_init(td_void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KEYSLOT_INIT;
	return ((td_s32(*)(td_void))((uintptr_t)entry))();
}

td_s32 uapi_drv_keyslot_deinit(td_void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KEYSLOT_DEINIT;
	return ((td_s32(*)(td_void))((uintptr_t)entry))();
}

td_s32 uapi_drv_keyslot_create(td_handle *hkeyslot, ext_drv_keyslot_type keyslot_type)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KEYSLOT_CREATE;
	return ((td_s32(*)(td_handle *, ext_drv_keyslot_type))((uintptr_t)entry))(hkeyslot, keyslot_type);
}

td_s32 uapi_drv_keyslot_destroy(td_handle hkeyslot)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KEYSLOT_DESTROY;
	return ((td_s32(*)(td_handle))((uintptr_t)entry))(hkeyslot);
}

/* sec klad */
int uapi_drv_klad_create(td_handle *hklad)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_CREATE;
	return ((td_s32(*)(td_handle *))((uintptr_t)entry))(hklad);
}

td_s32 uapi_drv_klad_destroy(const td_handle hklad)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_DESTROY;
	return ((td_s32(*)(td_handle))((uintptr_t)entry))(hklad);
}

int uapi_drv_klad_attach(const td_handle hklad, const ext_drv_klad_dest_type dest, const td_handle hkeyslot,
						 const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_ATTACH;
	return ((td_s32(*)(const td_handle, const ext_drv_klad_dest_type, const td_handle, const uintptr_t))((uintptr_t)entry))
		   (hklad, dest, hkeyslot, check_word);
}

td_s32 uapi_drv_klad_detach(const td_handle hklad, const ext_drv_klad_dest_type dest, const td_handle hkeyslot,
							const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_DETACH;
	return ((td_s32(*)(const td_handle, const ext_drv_klad_dest_type, const td_handle, const uintptr_t))((uintptr_t)entry))
		   (hklad, dest, hkeyslot, check_word);
}

td_s32 uapi_drv_klad_set_attr(const td_handle hklad, const ext_drv_klad_attr *attr, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_SET_ATTR;
	return ((td_s32(*)(const td_handle, const ext_drv_klad_attr *, const uintptr_t))((uintptr_t)entry))
		   (hklad, attr, check_word);
}

td_s32 uapi_drv_klad_set_session_key(const td_handle hklad, const ext_drv_klad_session_key *key,
									 const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_SET_SESSION_KEY;
	return ((td_s32(*)(const td_handle, const ext_drv_klad_session_key *, const uintptr_t))((uintptr_t)entry))
		   (hklad, key, check_word);
}

td_s32 uapi_drv_klad_set_content_key(const td_handle hklad, const ext_drv_klad_content_key *key,
									 const td_u32 timeout_ms, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_SET_CONTENT_KEY;
	return ((td_s32(*)(const td_handle, const ext_drv_klad_content_key *, const td_u32, const uintptr_t))
			((uintptr_t)entry))(hklad, key, timeout_ms, check_word);
}

td_s32 uapi_drv_klad_set_clear_key(const td_handle hklad, const ext_drv_klad_clear_key *key,
								   const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)(SET_CLEAR_KEY);
	return ((td_s32(*)(const td_handle, const ext_drv_klad_clear_key *, const uintptr_t))((uintptr_t)entry))
		   (hklad, key, check_word);
}

td_s32 uapi_drv_klad_rootkey_disable(const unsigned int rootkey_type, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_KLAD_ROOTKEY_DISABLE;
	return ((td_s32(*)(const td_u32, const uintptr_t))((uintptr_t)entry))(rootkey_type, check_word);
}

/* sec otp */
td_s32 uapi_drv_otp_read_word(const td_u32 addr, td_u32 *data, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_OTP_READ_WORD;
	return ((td_s32(*)(const td_u32, td_u32 *, const uintptr_t))((uintptr_t)entry))(addr, data, check_word);
}
td_s32 uapi_drv_otp_read_byte(const td_u32 addr, td_u8 *data, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_OTP_READ_BYTE;
	return ((td_s32(*)(const td_u32, td_u8 *, const uintptr_t))((uintptr_t)entry))(addr, data, check_word);
}

td_s32 uapi_drv_otp_get_die_id(td_u8 *die_id, const td_u32 len, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_OTP_GET_DIE_ID;
	return ((td_s32(*)(td_u8 *, const td_u32, const uintptr_t))((uintptr_t)entry))(die_id, len, check_word);
}

/* sec pke */
td_s32 uapi_drv_pke_init(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_PKE_INIT;
	return ((td_s32(*)(void))((uintptr_t)entry))();
}
td_s32 uapi_drv_pke_deinit(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_PKE_DEINIT;
	return ((td_s32(*)(void))((uintptr_t)entry))();
}

td_s32 uapi_drv_pke_ecdsa_verify(const ext_drv_pke_ecc_curve *ecc, const ext_drv_pke_ecc_point *pub_key,
								 const ext_drv_pke_data *hash, const ext_drv_pke_ecc_sig *sig, td_u8 *v, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_PKE_ECDSA_VERIFY;
	return ((td_s32(*)(const ext_drv_pke_ecc_curve *, const ext_drv_pke_ecc_point *,
					   const ext_drv_pke_data *, const ext_drv_pke_ecc_sig *, td_u8 *, const uintptr_t))
			((uintptr_t)entry))(ecc, pub_key, hash, sig, v, check_word);
}

td_s32 uapi_drv_pke_sm2_dsa_hash(const ext_drv_pke_data *sm2_id, const ext_drv_pke_ecc_point *pub_key,
								 const ext_drv_pke_msg *msg, ext_drv_pke_data *hash, const uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_PKE_SM2_DSA_HASH;
	return ((td_s32(*)(const ext_drv_pke_data *, const ext_drv_pke_ecc_point *,
					   const ext_drv_pke_msg *, ext_drv_pke_data *, const uintptr_t))((uintptr_t)entry))
		   (sm2_id, pub_key, msg, hash, check_word);
}

/* sec register */
td_void uapi_drv_register_func(const ext_drv_func *func_list)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UAPI_DRV_REGISTER_FUNC;
	((td_void(*)(const ext_drv_func *))((uintptr_t)entry))(func_list);
}
/* mmc */
int mmc_init(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MMC_INIT;
	return ((int (*)(void))((uintptr_t)entry))();
}

int mmc_read(void *ptr, size_t src, size_t size, size_t read_type)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MMC_READ;
	return ((int (*)(void *ptr, size_t src, size_t size, size_t read_type))((uintptr_t)entry))
		   (ptr, src, size, read_type);
}
unsigned int is_bootmode(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)IS_BOOTMODE;
	return ((unsigned int (*)(void))((uintptr_t)entry))();
}

emmc_mode_u mmc_get_cur_mode(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MMC_GET_CUR_MODE;
	return ((emmc_mode_u(*)(void))((uintptr_t)entry))();
}

void mmc_set_bootup_mode(sys_boot_u mode)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MMC_SET_BOOTUP_MODE;
	((void (*)(sys_boot_u mode))((uintptr_t)entry))(mode);
}

/* sdio */
int copy_from_sdio(void *buffer, unsigned long maxsize)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)COPY_FROM_SDIO;
	return ((int (*)(void *buffer, unsigned long maxsize))((uintptr_t)entry))
		   (buffer, maxsize);
}

void set_sdio_pos(unsigned long pos)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)SET_SDIO_POS;
	return ((void (*)(unsigned long pos))((uintptr_t)entry))
		   (pos);
}

void enable_sdio_dma(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)ENABLE_SDIO_DMA;
	((void (*)(void))((uintptr_t)entry))();
}
/* usb */
int self_usb_check(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)SELF_USB_CHECK;
	return ((int (*)(void))((uintptr_t)entry))();
}

int copy_from_usb(void *dest, size_t *count, u32 max_length)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)COPY_FROM_USB;
	return ((int (*)(void *dest, size_t *count, u32 max_length))((uintptr_t)entry))(dest, count, max_length);
}

void usb3_driver_init(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)USB3_DRIVER_INIT;
	((void (*)(void))((uintptr_t)entry))();
}

int send_to_usb(char *addr, size_t count)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)SEND_TO_USB;
	return ((int (*)(char *addr, size_t count))((uintptr_t)entry))(addr, count);
}


/* uart */
void pl011_putc(uart_num uart_base, s8 c)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)PL011_PUTC;
	((td_s32(*)(uart_num uart_base, s8 c))((uintptr_t)entry))(uart_base, c);
}

td_s32 pl011_getc(uart_num uart_base)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)PL011_GETC;
	return ((td_s32(*)(uart_num uart_base))((uintptr_t)entry))(uart_base);
}

td_s32 pl011_tstc(uart_num uart_base)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)PL011_TSTC;
	return ((td_s32(*)(uart_num uart_base))((uintptr_t)entry))(uart_base);
}
td_s32 serial_init(uart_num uart_base, const uart_cfg *cfg)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)SERIAL_INIT;
	return ((td_s32(*)(uart_num uart_base, const uart_cfg * cfg))((uintptr_t)entry))(uart_base, cfg);
}

void log_serial_puts(const s8 *s)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)LOG_SERIAL_PUTS;
	((void (*)(const s8 * s))((uintptr_t)entry))(s);
}

td_s32 copy_from_uart(void *dest, size_t *count, td_u32 max_length)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)COPY_FROM_UART;
	return ((td_s32(*)(void *dest, size_t *count, td_u32 max_length))((uintptr_t)entry))(dest, count, max_length);
}

/* timer */
void timer_init()
{
	td_u32 entry = *(td_u32 *)(uintptr_t)TIMER_INIT;
	((void (*)(void))((uintptr_t)entry))();
}

void timer_deinit()
{
	td_u32 entry = *(td_u32 *)(uintptr_t)TIMER_DEINIT;
	((void (*)(void))((uintptr_t)entry))();
}

void timer_start()
{
	td_u32 entry = *(td_u32 *)(uintptr_t)TIMER_START;
	((void (*)(void))((uintptr_t)entry))();
}

unsigned long timer_get_val()
{
	td_u32 entry = *(td_u32 *)(uintptr_t)TIMER_GET_VAL;
	return ((unsigned long (*)())((uintptr_t)entry))();
}

void udelay(unsigned long usec)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)UDELAY;
	((void (*)(unsigned long usec))((uintptr_t)entry))(usec);
}

void mdelay(unsigned long msec)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MDELAY;
	((void (*)(unsigned long msec))((uintptr_t)entry))(msec);
}

/* watchdog */
td_s32 watchdog_enable(td_u32 n, td_u32 timeout, td_u32 wdg_freq)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)WATCHDOG_ENABLE;
	return ((td_s32(*)(td_u32 n, td_u32 timeout, td_u32 wdg_freq))((uintptr_t)entry))(n, timeout, wdg_freq);
}

td_s32 watchdog_disable(td_u32 n)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)WATCHDOG_DISABLE;
	return ((td_s32(*)(td_u32 n))((uintptr_t)entry))(n);
}
td_s32 watchdog_feed(td_u32 n, td_u32 timeout)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)WATCHDOG_FEED;
	return ((td_s32(*)(td_u32 n, td_u32 timeout))((uintptr_t)entry))(n, timeout);
}
/* sec mem opt */
td_s32 memcmp_ss(const td_void *cs, const td_void *ct, td_size_t count, uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MEMCMP_SS;
	return ((td_s32(*)(const td_void *, const td_void *, td_size_t, uintptr_t))((uintptr_t)entry))
		   (cs, ct, count, check_word);
}

td_s32 memcpy_ss(td_void *dest, td_size_t dest_max, const td_void *src, td_size_t count, uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MEMCPY_SS;
	return ((td_s32(*)(td_void *, td_size_t, const td_void *, td_size_t, uintptr_t))((uintptr_t)entry))
		   (dest, dest_max, src, count, check_word);
}

td_s32 memset_ss(td_void *dest, td_size_t dest_max, td_u8 c, td_size_t count, uintptr_t check_word)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MEMSET_SS;
	return ((td_s32(*)(td_void *, td_size_t, td_u8, td_size_t, uintptr_t))((uintptr_t)entry))
		   (dest, dest_max, c, count, check_word);
}

errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MEMCPY_S;
	return ((errno_t(*)(void *, size_t, const void *, size_t))((uintptr_t)entry))
		   (dest, dest_max, src, count);
}

errno_t memset_s(void *dest, size_t dest_max, int c, size_t count)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)MEMSET_S;
	return ((errno_t(*)(void *, size_t, int, size_t))((uintptr_t)entry))
		   (dest, dest_max, c, count);
}

void save_cur_point_syscnt(void)
{
	td_u32 entry = *(td_u32 *)(uintptr_t)SAVE_CUR_POINT_SYSCNT;
	((void (*)(void))((uintptr_t)entry))();
}

int self_sdio_check(void)
{
	return ((int (*)(void))((uintptr_t)SELF_SDIO_CHECK))();
}
