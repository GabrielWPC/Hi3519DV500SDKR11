/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "types.h"
#include "common.h"
#include "lib.h"
#include "platform.h"
#include "otp.h"
#include "flash_map.h"

unsigned int is_ree_verify_enable()
{
	otp_byte_aligned_lockable_1 otp_value_lockable_1;
	otp_value_lockable_1.u32 = reg_get(OTP_BYTE_ALIGNED_LOCKABLE_1);
	if (otp_value_lockable_1.bits.ree_verify_enable == 0x42)
		return AUTH_FAILURE;
	return AUTH_SUCCESS;
}

unsigned int is_tp_verify_enable()
{
	otp_byte_aligned_lockable_1 otp_value_lockable_1;
	otp_value_lockable_1.u32 = reg_get(OTP_BYTE_ALIGNED_LOCKABLE_1);
	if (otp_value_lockable_1.bits.tp_verify_enable == 0x42)
		return AUTH_FAILURE;
	return AUTH_SUCCESS;
}

unsigned int is_tee_verify_enable()
{
	otp_byte_aligned_lockable_1 otp_value_lockable_1;
	otp_value_lockable_1.u32 = reg_get(OTP_BYTE_ALIGNED_LOCKABLE_1);
	if (otp_value_lockable_1.bits.tee_verify_enable == 0x42)
		return AUTH_FAILURE;
	return AUTH_SUCCESS;
}

unsigned int is_soc_tee_enable()
{
	otp_byte_aligned_lockable_0 otp_value_lockable_0;
	otp_value_lockable_0.u32 = reg_get(OTP_BYTE_ALIGNED_LOCKABLE_0);
	if (otp_value_lockable_0.bits.soc_tee_enable == 0x42)
		return AUTH_FAILURE;
	return AUTH_SUCCESS;
}

u32 is_sec_dbg_enable(void)
{
	otp_shadowed_oneway_0 otp_value_oneway_0;
	otp_value_oneway_0.u32 = reg_get(OTP_SHADOWED_ONEWAY_0);
	if (otp_value_oneway_0.bits.sec_boot_dbg_disable != 0x42)
		return AUTH_FAILURE;
	else
		return AUTH_SUCCESS;
}

unsigned int is_sec_dbg_lv_enable(void)
{
	otp_shadowed_oneway_0 otp_value_oneway_0;
	otp_value_oneway_0.u32 = reg_get(OTP_SHADOWED_ONEWAY_0);
	if (otp_value_oneway_0.bits.sec_boot_dbg_lv != 0)
		return AUTH_FAILURE;
	else
		return AUTH_SUCCESS;
}

unsigned int is_tee_dec_en_enable(void)
{
	otp_4bit_aligned_lockable otp_value_4bit;
	otp_value_4bit.u32 = reg_get(OTP_4BIT_ALIGNED_LOCKABLE);
	if (otp_value_4bit.bits.tee_dec_enable == 0)
		return AUTH_FAILURE;
	else
		return AUTH_SUCCESS;
}


unsigned int is_ree_boot_dec_en_enable(void)
{
	otp_4bit_aligned_lockable otp_value_4bit;
	otp_value_4bit.u32 = reg_get(OTP_4BIT_ALIGNED_LOCKABLE);
	if (otp_value_4bit.bits.ree_dec_enable == 0)
		return AUTH_FAILURE;
	else
		return AUTH_SUCCESS;
}

u32 opt_get_boot_backup_enable(void)
{
	otp_bit_aligned_lockable otp_value;
	otp_value.u32 = reg_get(OTP_BIT_ALIGNED_LOCKABLE);
	return otp_value.bits.boot_backup_enable;
}