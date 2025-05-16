/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "platform.h"
#include "lib.h"
#include "../drivers/otp/otp.h"
#include "share_drivers.h"
#include "flash_map.h"
#include "common.h"
#include "securecutil.h"
#include "err_print.h"

td_u8 hash_verify_buf[SHA_256_LEN];

typedef struct {
	ext_drv_cipher_dfa dfa;
	ext_drv_cipher_work_mode work_mode; /* < Operating mode */
} dec_parm;

static const u8 rom_ecc_p [32] __attribute__((aligned(8))) = {
	0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC, 0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x72,
	0x6E, 0x3B, 0xF6, 0x23, 0xD5, 0x26, 0x20, 0x28, 0x20, 0x13, 0x48, 0x1D, 0x1F, 0x6E, 0x53, 0x77
};
static const u8 rom_ecc_a [32] __attribute__((aligned(8))) = {
	0x7D, 0x5A, 0x09, 0x75, 0xFC, 0x2C, 0x30, 0x57, 0xEE, 0xF6, 0x75, 0x30, 0x41, 0x7A, 0xFF, 0xE7,
	0xFB, 0x80, 0x55, 0xC1, 0x26, 0xDC, 0x5C, 0x6C, 0xE9, 0x4A, 0x4B, 0x44, 0xF3, 0x30, 0xB5, 0xD9
};
static const u8 rom_ecc_b [32]__attribute__((aligned(8))) = {
	0x26, 0xDC, 0x5C, 0x6C, 0xE9, 0x4A, 0x4B, 0x44, 0xF3, 0x30, 0xB5, 0xD9, 0xBB, 0xD7, 0x7C, 0xBF,
	0x95, 0x84, 0x16, 0x29, 0x5C, 0xF7, 0xE1, 0xCE, 0x6B, 0xCC, 0xDC, 0x18, 0xFF, 0x8C, 0x07, 0xB6
};
static const u8 rom_ecc_gx [32]__attribute__((aligned(8))) = {
	0x8B, 0xD2, 0xAE, 0xB9, 0xCB, 0x7E, 0x57, 0xCB, 0x2C, 0x4B, 0x48, 0x2F, 0xFC, 0x81, 0xB7, 0xAF,
	0xB9, 0xDE, 0x27, 0xE1, 0xE3, 0xBD, 0x23, 0xC2, 0x3A, 0x44, 0x53, 0xBD, 0x9A, 0xCE, 0x32, 0x62
};
static const u8 rom_ecc_gy [32]__attribute__((aligned(8))) = {
	0x54, 0x7E, 0xF8, 0x35, 0xC3, 0xDA, 0xC4, 0xFD, 0x97, 0xF8, 0x46, 0x1A, 0x14, 0x61, 0x1D, 0xC9,
	0xC2, 0x77, 0x45, 0x13, 0x2D, 0xED, 0x8E, 0x54, 0x5C, 0x1D, 0x54, 0xC7, 0x2F, 0x04, 0x69, 0x97
};
static const u8 rom_ecc_n [32] __attribute__((aligned(8))) = {
	0xA9, 0xFB, 0x57, 0xDB, 0xA1, 0xEE, 0xA9, 0xBC, 0x3E, 0x66, 0x0A, 0x90, 0x9D, 0x83, 0x8D, 0x71,
	0x8C, 0x39, 0x7A, 0xA3, 0xB5, 0x61, 0xA6, 0xF7, 0x90, 0x1E, 0x0E, 0x82, 0x97, 0x48, 0x56, 0xA7
};

static const ext_drv_pke_ecc_curve g_rom_ecc = {rom_ecc_p, rom_ecc_a, rom_ecc_b, rom_ecc_gx,
												rom_ecc_gy, rom_ecc_n, 1, EXT_DRV_PKE_LEN_256, EXT_DRV_PKE_ECC_TYPE_RFC5639
											   };

static td_void klad_set_attr(ext_drv_klad_attr *attr, ext_drv_klad_content_key *content_key,
							 ext_drv_klad_session_key *session_key, const unsigned char *key, unsigned int rootkey_type)
{
	otp_bit_aligned_lockable otp_value;
	unsigned int engine_alg, klad_alg;
	if ((attr == TD_NULL) || (content_key == TD_NULL) ||
			(session_key == TD_NULL) || (key == TD_NULL))
		return;
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	if (otp_value.bits.scs_alg_sel == 1) {
		engine_alg = EXT_DRV_CRYPTO_ALG_SM4;
		klad_alg = EXT_DRV_KLAD_ALG_TYPE_SM4;
	} else {
		engine_alg = EXT_DRV_CRYPTO_ALG_AES;
		klad_alg = EXT_DRV_KLAD_ALG_TYPE_AES;
	}
	attr->klad_cfg.rootkey_type = rootkey_type;
	attr->key_cfg.engine = engine_alg;
	attr->key_cfg.decrypt_support = 0x1;
	attr->key_cfg.encrypt_support = 0;
	attr->key_sec_cfg.key_sec = EXT_DRV_KLAD_SEC_ENABLE;
	attr->key_sec_cfg.dest_buf_non_sec_support = 0x1;
	attr->key_sec_cfg.dest_buf_sec_support = 0x1;
	attr->key_sec_cfg.master_only_enable = 0x1;
	attr->key_sec_cfg.src_buf_non_sec_support = 0x1;
	attr->key_sec_cfg.src_buf_sec_support = 0x1;
	session_key->level = EXT_DRV_KLAD_LEVEL1;
	session_key->alg = klad_alg;
	session_key->key_size = PROTECT_KEY_LEN;
	session_key->key = (unsigned char *)key;
	content_key->alg = klad_alg;
	content_key->key_size = PROTECT_KEY_LEN;
	content_key->key = (unsigned char *)(key + PROTECT_KEY_LEN);
	content_key->key_parity = EXT_DRV_KLAD_KEY_EVEN;
}

static td_s32 klad_set_content_key(td_handle key_slot, const unsigned char *key, unsigned int key_len,
								   unsigned int rootkey_type, unsigned int check_word)
{
	td_handle klad;
	ext_drv_klad_attr attr;
	ext_drv_klad_content_key content_key;
	ext_drv_klad_session_key session_key;
	volatile td_s32 ret = EXT_SEC_FAILURE;
	if ((key_len != (PROTECT_KEY_LEN + PROTECT_KEY_LEN)) || (key == TD_NULL))
		return EXT_SEC_FAILURE;
	/* set klad attribute */
	klad_set_attr(&attr, &content_key, &session_key, key, rootkey_type);
	if (uapi_drv_klad_create(&klad) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	if (uapi_drv_klad_attach(klad, EXT_DRV_KLAD_DEST_TYPE_MCIPHER, key_slot, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
		goto exit1;
	if (uapi_drv_klad_set_attr(klad, &attr, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
		goto exit2;
	/* level 1 */
	if (uapi_drv_klad_set_session_key(klad, &session_key, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
		goto exit2;
	/* level 2 */
	if (uapi_drv_klad_set_content_key(klad, &content_key, 0, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
		goto exit2;
	ret = EXT_SEC_SUCCESS;
exit2:
	if (uapi_drv_klad_detach(klad, EXT_DRV_KLAD_DEST_TYPE_MCIPHER, key_slot, NO_CHECK_WORD) != EXT_SEC_SUCCESS)
		ret = EXT_SEC_FAILURE;
exit1:
	/* destroy klad */
	if (uapi_drv_klad_destroy(klad) != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return ret;
}

static td_s32 read_cipher_data(td_handle cipher_handle, unsigned int start_addr, unsigned int offset,
							   unsigned char *buffer, unsigned int size)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	ext_drv_cipher_buf_attr src_buf;
	ext_drv_cipher_buf_attr dest_buf;
	src_buf.address = (unsigned char *)(uintptr_t)(start_addr + offset);
	src_buf.buf_sec = EXT_DRV_CIPHER_BUF_SECURE;
	dest_buf.address = buffer;
	dest_buf.buf_sec = EXT_DRV_CIPHER_BUF_SECURE;
	ret = uapi_drv_cipher_decrypt(cipher_handle, &src_buf, &dest_buf, size, 0, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

static td_s32 cipher_set_config(td_handle cipher_handle, const unsigned char *iv, unsigned int iv_len,
								dec_parm *dec_mode, unsigned int check_word)
{
	otp_bit_aligned_lockable otp_value;
	ext_drv_cipher_config_aes aes_config;
	ext_drv_cipher_config_sm4 sm4_config;
	ext_drv_cipher_config ctrl;
	unsigned char *iv_buff = TD_NULL;
	volatile td_s32 ret = EXT_SEC_FAILURE;
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	if (otp_value.bits.scs_alg_sel == 1) {
		sm4_config.iv_change_flag = EXT_DRV_CIPHER_IV_CHANGE_ONE_PKG;
		sm4_config.key_parity = EXT_DRV_CIPHER_KEY_EVEN;
		iv_buff = sm4_config.iv;
		ctrl.alg = EXT_DRV_CIPHER_ALG_SM4;
		ctrl.param = &sm4_config;
	} else {
		aes_config.key_len = EXT_DRV_CIPHER_KEY_128BIT;
		aes_config.bit_width = EXT_DRV_CIPHER_BIT_WIDTH_128BIT;
		aes_config.iv_change_flag = EXT_DRV_CIPHER_IV_CHANGE_ONE_PKG;
		aes_config.key_parity = EXT_DRV_CIPHER_KEY_EVEN;
		iv_buff = aes_config.iv;
		ctrl.alg = EXT_DRV_CIPHER_ALG_AES;
		ctrl.param = &aes_config;
	}
	if (iv_buff == TD_NULL) {
		err_print(IV_BUFF_ERR);
		return EXT_SEC_FAILURE;
	}
	ret = memcpy_ss(iv_buff, EXT_DRV_CIPHER_AES_IV_LEN_IN_BYTES, iv, iv_len, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(IV_MEMCPY_ERR);
		return EXT_SEC_FAILURE;
	}
	ctrl.dfa = dec_mode->dfa;
	ctrl.work_mode = dec_mode->work_mode;
	ret = uapi_drv_cipher_set_config(cipher_handle, &ctrl, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(DRV_CIPHER_SET_CONFIG_ERR);
		return EXT_SEC_FAILURE;
	}
	return EXT_SEC_SUCCESS;
}

int decrypt_data(u32 rootkey_type, const para_enc_info *enc_info, u8 *code_dest, u8 *code_src,
				 u32 code_len)
{
	td_s32 ret = EXT_SEC_FAILURE;
	td_handle cipher_handle, keyslot_handle;
	ext_drv_cipher_attr cipher_attr;
	dec_parm dec_mode;
	/* Creat cipher handle */
	cipher_attr.cipher_type = EXT_DRV_CIPHER_TYPE_NORMAL;
	cipher_attr.lock_mode = EXT_DRV_CIPHER_LONG_TERM_LOCK;
	if (uapi_drv_cipher_create(&cipher_handle, &cipher_attr, NO_CHECK_WORD) != EXT_SEC_SUCCESS) {
		err_print(DRV_CIPHER_CREATE_ERR);
		return EXT_SEC_FAILURE;
	}
	/* init keyslot */
	if (uapi_drv_keyslot_init() != EXT_SEC_SUCCESS) {
		(td_void)uapi_drv_cipher_destroy(cipher_handle);
		err_print(DRV_KEYSLOT_INIT_ERR);
		return EXT_SEC_FAILURE;
	}
	/* Creat keyslot */
	if (uapi_drv_keyslot_create(&keyslot_handle, EXT_DRV_KEYSLOT_TYPE_MCIPHER) != EXT_SEC_SUCCESS) {
		(td_void)uapi_drv_keyslot_deinit();
		(td_void)uapi_drv_cipher_destroy(cipher_handle);
		err_print(DRV_KEYSLOT_CREATE_ERR);
		return EXT_SEC_FAILURE;
	}
	/* Attach keyslot and cipher */
	ret = EXT_SEC_FAILURE;
	ret = uapi_drv_cipher_attach(cipher_handle, keyslot_handle, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		(td_void)uapi_drv_keyslot_deinit();
		(td_void)uapi_drv_cipher_destroy(cipher_handle);
		(td_void)uapi_drv_keyslot_destroy(keyslot_handle);
		err_print(DRV_CIPHER_ATTACH_ERR);
		return EXT_SEC_FAILURE;
	}
	/* Set cipher config */
	dec_mode.work_mode = EXT_DRV_CIPHER_WORK_MODE_CBC;
	dec_mode.dfa = EXT_DRV_CIPHER_DFA_ENABLE;
	ret = EXT_SEC_FAILURE;
	ret = cipher_set_config(cipher_handle, enc_info->iv, IV_LEN, &dec_mode, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(CIPHER_SET_CONFIG_ERR);
		goto done;
	}
	ret = EXT_SEC_FAILURE;
	ret = klad_set_content_key(keyslot_handle, enc_info->protection_key_l1,
							   PROTECT_KEY_LEN + PROTECT_KEY_LEN, rootkey_type, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(KLAD_SET_CONTENT_KEY_ERR);
		goto done;
	}
	ret = EXT_SEC_FAILURE;
	ret = read_cipher_data(cipher_handle, (uintptr_t)code_src, 0, code_dest, code_len);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(READ_CIPHER_DATA_ERR);
		goto done;
	}
done:
	(td_void)uapi_drv_cipher_detach(cipher_handle, keyslot_handle, NO_CHECK_WORD);
	(td_void)uapi_drv_keyslot_destroy(keyslot_handle);
	(td_void)uapi_drv_cipher_destroy(cipher_handle);
	(td_void)uapi_drv_keyslot_deinit();
	return ret;
}

static td_s32 hash_operation(const ext_drv_cipher_buf_attr *src_buf, td_u32 src_len, td_u8 *data_sha,
							 const ext_drv_cipher_hash_attr *hash_attr, td_u32 check_word)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	td_handle handle;
	td_u32 out_length = SHA_256_LEN;
	ret = EXT_SEC_FAILURE;
	ret = uapi_drv_cipher_hash_init(&handle, hash_attr, (uintptr_t)(&handle) ^ (uintptr_t)hash_attr);
	if (ret != EXT_SEC_SUCCESS)
		return ret;
	ret = EXT_SEC_FAILURE;
	ret = uapi_drv_cipher_hash_update(handle, src_buf, src_len, 0, handle ^ (uintptr_t)src_buf ^ src_len);
	(td_void)uapi_drv_cipher_hash_final(handle, data_sha, &out_length, NO_CHECK_WORD);
	return ret;
}

td_s32 calc_hash(td_u32 src_addr, td_u32 src_len, td_u8 *data_sha,
				 td_u32 data_sha_len, td_u32 check_word)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	ext_drv_cipher_hash_attr hash_attr;
	ext_drv_cipher_buf_attr src_buf;
	otp_bit_aligned_lockable otp_value;
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	if (otp_value.bits.scs_alg_sel == 1)
		hash_attr.hash_type = EXT_DRV_CIPHER_HASH_TYPE_SM3;
	else
		hash_attr.hash_type = EXT_DRV_CIPHER_HASH_TYPE_SHA256;
	hash_attr.keyslot = 0;
	src_buf.address = (u8 *)(uintptr_t)src_addr;
	src_buf.buf_sec = EXT_DRV_CIPHER_BUF_SECURE;
	ret = hash_operation(&src_buf, src_len, data_sha, &hash_attr, 0);
	return ret;
}

void store_hash_to_lpds(td_u8 *hash_addr)
{
	volatile td_s32 ret = EXT_SEC_FAILURE;
	ret = EXT_SEC_FAILURE;
	ret = memcpy_ss(hash_addr, SHA_256_LEN, hash_verify_buf, SHA_256_LEN, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(HASH_STORE_TO_LPDS_CPY_ERR);
		call_reset();
	}
	return;
}

static td_s32 store_to_hash_buf(td_u8 *hash_verify_buf, td_u8 *data_hash, td_u32 hash_len)
{
	td_s32 ret = EXT_SEC_FAILURE;
	ret =  memset_ss(hash_verify_buf, hash_len, 0, hash_len, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		return ret;
	ret = memcpy_ss(hash_verify_buf, hash_len, data_hash, hash_len, NO_CHECK_WORD);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	return EXT_SEC_SUCCESS;
}

td_s32 verify_signature(const ext_data *data, const ext_drv_pke_ecc_point *pub_key,
						const ext_pke_sig *sign, const ext_data *v, td_u32 check_word)
{
	td_s32 ret = EXT_SEC_FAILURE;
	td_u8 data_hash[SHA_256_LEN];
	td_u8 data_hash_sm3_out[SHA_256_LEN];
	ext_drv_pke_data hash;
	ext_drv_pke_ecc_curve curve;
	otp_bit_aligned_lockable otp_value;
	ext_drv_pke_data sm2_id;
	const ext_drv_pke_ecc_curve *curve_point = TD_NULL;
	ext_drv_pke_msg pke_msg;
	if ((v == TD_NULL) || (v->data == TD_NULL) || (v->length != SHA_256_LEN) || (data == TD_NULL))
		return EXT_SEC_FAILURE;
	/* Initialise hash arrays and structs */
	ret =  memset_ss(data_hash, SHA_256_LEN, 0x5a, SHA_256_LEN, 0);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = EXT_SEC_FAILURE;
	ret =  memset_ss(v->data, v->length, 0xa5, v->length, 0);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	hash.length = SHA_256_LEN;
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	if (otp_value.bits.scs_alg_sel == 1) {
		hash.data = data_hash_sm3_out; /* use for verify */
		sm2_id.data = (td_u8 *)EXT_SM2_ID;
		sm2_id.length = EXT_SM2_ID_LEN;
		curve.ecc_type = EXT_DRV_PKE_ECC_TYPE_SM2;
		curve_point = &curve;
		pke_msg.data = data->data;
		pke_msg.length = data->length;
		pke_msg.buf_sec = EXT_DRV_PKE_BUF_SECURE;
		ret = EXT_SEC_FAILURE;
		ret = uapi_drv_pke_sm2_dsa_hash(&sm2_id, pub_key, &pke_msg, &hash, 0);
		if (ret != EXT_SEC_SUCCESS)
			return EXT_SEC_FAILURE;
	} else {
		hash.data = data_hash; /* use for verify */
		curve_point = &g_rom_ecc;
	}
	/* data_hash: sm3 or hash256 result for lpds */
	ret = EXT_SEC_FAILURE;
	ret = calc_hash((uintptr_t)data->data, data->length, data_hash, SHA_256_LEN, 0);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(CALC_HASH_ERR);
		return ret;
	}
	ret = store_to_hash_buf(hash_verify_buf, data_hash, SHA_256_LEN);
	if (ret != EXT_SEC_SUCCESS)
		return EXT_SEC_FAILURE;
	ret = EXT_SEC_FAILURE;
	ret = uapi_drv_pke_ecdsa_verify(curve_point, pub_key, &hash, (ext_drv_pke_ecc_sig *)sign, v->data, 0);
	if (ret != EXT_SEC_SUCCESS) {
		err_print(DRV_PKE_ECDSA_VERIFY_ERR);
		return ret;
	}
	return ret;
}

int dma_copy(uintptr_t dest, u32 count, uintptr_t src)
{
	s32 ret;
	ext_drv_cipher_buf_attr dma_dest_buf;
	ext_drv_cipher_buf_attr dma_src_buf;
	dma_dest_buf.address = (u8 *)(dest);
	dma_dest_buf.buf_sec = EXT_DRV_CIPHER_BUF_SECURE;
	dma_src_buf.address = (u8 *)(src);
	dma_src_buf.buf_sec = EXT_DRV_CIPHER_BUF_SECURE;
	ret = uapi_drv_cipher_dma_copy(&dma_dest_buf, &dma_src_buf, count, NO_CHECK_WORD);
	return ret;
}
