/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef __ERR_PRINTF_H__
#define __ERR_PRINTF_H__

#include <types.h>

#define STACK_CHK_FAIL                            62
#define GET_DATA_CHANNEL_TYPE_UNKNOW              63
#define GSL_SDIO_CHECK_ERROR                      64
#define GET_REE_KEY_AND_PARAM_FROM_UART_ERR       65
#define GET_REE_KEY_AND_PARAM_FROM_USB_ERR        66
#define FIRST_RECV_LEN_CHECK_ERR                  67
#define SECOND_RECV_LEN_CHECK_ERR                 68

#define VERIFY_IMG_ID_ERR                         69

#define DRV_CIPHER_CREATE_ERR                     70
#define DRV_KEYSLOT_INIT_ERR                      71
#define DRV_KEYSLOT_CREATE_ERR                    72
#define DRV_CIPHER_ATTACH_ERR                     73
#define IV_BUFF_ERR                               74
#define IV_MEMCPY_ERR                             75
#define DRV_CIPHER_SET_CONFIG_ERR                 76
#define CIPHER_SET_CONFIG_ERR                     77
#define KLAD_SET_CONTENT_KEY_ERR                  78
#define READ_CIPHER_DATA_ERR                      79
#define DEC_CODE_ERR                              80

#define MEMSET_ERR                                81
#define CALC_HASH_ERR                             82
#define DRV_PKE_ECDSA_VERIFY_ERR                  83
#define VERIFY_SIGN_CMP_ERR                       84
#define HASH_CMP_FAIL                             85

#define REE_FLASH_ROOTKEY_STATUS_INVALID          86
#define UART_BUSY_WATE_TIMEOUT_ERR                87
#define GPLL_CONFIG_ERR                           88

#define DRV_PKE_INIT_ERR                          89
#define SECURE_VERIFY_SIGN_ERR                    90
#define DRV_PKE_DEINIT_ERR                        91
#define TP_SECURE_VERIFY_ERR                      92
#define SECURE_VERIFY_ERR                         93

#define DRV_OTP_READ_WORD_ERR                     94
#define VERSION_CMP_BIT_ERR                       95
#define VERSION_CMP_COUNT_ERR                     96
#define VERIFY_VERSION_ERR                        97

#define VERIFY_MSID_ERR                           98

#define DRV_OTP_GET_DIE_ID_ERR                    99
#define MAINTENACE_MODE_DIE_ID_CMP_FAIL           100

#define PARA_AREA_NUM_ERR                         101
#define PARA_AREA_LEN_ERR                         102

#define HASH_STORE_TO_LPDS_CPY_ERR                103

#define HANDLE_REE_KEY_ERR                        104

#define HANDLE_PARAM_AREA_INFO_ERR                105

#define HANDLE_PARAM_AREA_ERR                     106

#define LPDS_ALL_HASH_CMP_FAIL                    107

#define TEE_SECURE_DDR_ADDR_SIZE_ERR              108
#define TZASC_BYPASS_DISABLE_ERR                  109
#define DDR_CONFIG_ERR                            110

#define GET_UBOOT_INFO_AND_CODE_FROM_UART_ERR     111

#define GET_UBOOT_INFO_AND_CODE_FROM_USB_ERR      112

#define HANDLE_UBOOT_CODE_INFO_ERR                113

#define HANDLE_UBOOT_CODE_ERR                     114

#define HANDLE_TEE_KEY_ERR                        115

#define HANDLE_TEE_INFO_ERR                       116

#define HANDLE_ATF_CODE_ERR                       117

#define HANDLE_TEE_CODE_ERR                       118

void err_print(u8 err_idx);

/* output error information */
#define MAX_STR_LEN	                          3
#define DIV_100		                          100
#define DIV_10		                          10
#define MAX_ERR_BITS	                          128

#define ERR_PRINT_DELAY_MS	                  10

#endif