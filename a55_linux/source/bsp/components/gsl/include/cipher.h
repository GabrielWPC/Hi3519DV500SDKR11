/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __CIPHER_H_
#define __CIPHER_H_
#include "common.h"
#include "soc_pke_api.h"


int decrypt_data(u32 rootkey_type, const para_enc_info *enc_info, u8 *code_dest, u8 *code_src,
				 u32 code_len);
td_s32 verify_signature(const ext_data *data, const ext_drv_pke_ecc_point *pub_key, const ext_pke_sig *sign,
						const ext_data *v, td_u32 check_word);
int calc_hash(u32 src_addr, u32 src_len, u8 *data_hash, u32 data_hash_len, u32 check_word);
int dma_copy(uintptr_t dest, u32 count, uintptr_t src);
void store_hash_to_lpds(td_u8 *hash_addr);
#endif /* __CIPHER_H_ */
