/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

/* Image id definition */
#define TEE_FLASH_RPK_IMG_ID          0x4B96B43C
#define REE_FLASH_RPK_IMG_ID          0x4BA5C34B
#define TP_FLASH_RPK_IMG_ID           0x4B2D4BC3
#define CFCT_KEY_IMG_ID               0x4BD2F01E
#define CFCT_TABLE_IMG_ID             0x4BD2F02D
#define GSL_KEY_AREA_IMG_ID           0x4BB4D25A

#define REE_KEY_AREA_IMG_ID           0x4B1E3C1E
#define PARAMS_INFO_IMG_ID            0x4B87A52D
#define UBOOT_INFO_IMG_ID             0x4BF01E2D

#define TEE_KEY_AREA_IMG_ID           0x4BE10F1E
#define TEE_CODE_INFO_IMG_ID          0x4BE10F2D

/* structure version */
#define STRUCTURE_VERSION               0x00010000

/* Algorithm select definition */
#define ALG_SEL_ECC256                  0x2A13C812
#define ALG_SEL_SM2                     0x2A13C823
#define ECC_CURVE_RFC5639               0x2A13C812

/* Maintenance flag definition */
#define MAINTENACE_MODE_ENABLE          0x3C7896E1

/* Image encrypt flag */
#define CODE_ENC_DISABLE                0x3C7896E1

/* Compress flag definition */
#define APP_COMPRESS_ENABLE             0x3C7896E1

#define DIE_ID_LEN                      16
#define HASH_LEN                        32
#define MX_2							2
#define SIG_LEN                         64
#define ECC_256_KEY_LEN                 64
#define PROTECT_KEY_LEN                 16
#define IV_LEN                          16
#define PARAMS_STRUCTURE_HEAD_LEN       15

#define MAX_PARAMS_NUM                  8
/* Flash rootkey definition */
#define FLSH_ROOTKEY_VALID              0x5
#define FLSH_ROOTKEY_INVALID            0xA

#define MAX_DDR_PARA_NUM                8
#define SIZE_1K                         1024
#define SIZE_15K                        (15 * 1024)
#define SHA_256_LEN 0x20 // 0X20 BYTES = 256 Bits
#define ECC_KEY_OFFSET 32

#define INVALID_PAM_HASH_VAL	0xff

/* for uart or usb driver recive data check */
/* ree key(fix 0x100) + param info(fix 0x200) + fill(fix 0x100) + param(max 15k) */
#define FIRST_RECIVE_MAX_LEN	0x4000
/* uboot info(fix 0x200) + uboot code(max 1M) */
#define SECOND_RECIVE_MAX_LEN	0x100000

/* TEE/REE/TP root public key area, size is 0x80 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;  /* currently version is 0x00010000 */
	unsigned int      structure_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned char     reserved[32]; /* 32 bytes reserved */
	unsigned char     root_key_area[ECC_256_KEY_LEN];
} flash_root_public_key_s;

/* Key area, size is 0x100 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	unsigned int      debug_rusk_enable;
	unsigned char     reserved[52]; /* 52 bytes reserved */
	unsigned char     ext_pulic_key_area[ECC_256_KEY_LEN];
	unsigned char     signature[SIG_LEN];
} gsl_key_area_s;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	unsigned int      debug_rusk_enable;
	unsigned char     reserved[52]; /* 56 bytes reserved */
	unsigned char     ext_pulic_key_area[ECC_256_KEY_LEN];
	unsigned char     signature[SIG_LEN];
} tp_key_area_s;


typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	unsigned int      debug_rusk_enable;
	unsigned char     reserved[52]; /* 52 bytes reserved */
	unsigned char     ext_pulic_key_area[ECC_256_KEY_LEN];
	unsigned char     signature[SIG_LEN];
} ree_key_area_s;

/* Code area info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      code_area_addr;
	unsigned int      code_area_len;
	unsigned char     code_area_hash[HASH_LEN];
	unsigned int      code_enc_flag;
	unsigned char     protection_key_l1[PROTECT_KEY_LEN];
	unsigned char     protection_key_l2[PROTECT_KEY_LEN];
	unsigned char     iv[IV_LEN];
	unsigned int      code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      code_uncompress_len;
	unsigned int      text_segment_size;
	unsigned char     reserved[248];  /* 248 byte is reserved */
	unsigned char     sig_code_info[SIG_LEN];
	unsigned char     sig_code_info_ext[SIG_LEN];
} gsl_code_info;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      para_area_addr;
	unsigned int      para_area_len;
	unsigned int      para_area_num;
	unsigned char     para_area_hash[MAX_PARAMS_NUM][HASH_LEN];
	unsigned char     board_index_hash_table[MAX_PARAMS_NUM];
	unsigned char     reserved[76];  /* 248 byte is reserved */
	unsigned char     signature[SIG_LEN];
	unsigned char     signature_ext[SIG_LEN];
} para_area_info;

typedef struct {
	unsigned int      code_enc_flag;
	unsigned char     protection_key_l1[PROTECT_KEY_LEN];
	unsigned char     protection_key_l2[PROTECT_KEY_LEN];
	unsigned char     iv[IV_LEN];
} para_enc_info;

/* Code area info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      code_area_addr;
	unsigned int      code_area_len;
	unsigned char     code_area_hash[HASH_LEN];
	para_enc_info     enc_info;
	unsigned int      code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      code_uncompress_len;
	unsigned int      text_segment_size;
	unsigned int      uboot_entry_point;
	unsigned char     reserved[244];  /* 248 byte is reserved */
	unsigned char     signature[SIG_LEN];
	unsigned char     signature_ext[SIG_LEN];
} uboot_code_info;

typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      key_owner_id;
	unsigned int      key_id;
	unsigned int      key_alg;            /* 0x2A13C812: ECC256;  0x2A13C823: SM2 */
	unsigned int      ecc_curve_type;     /* 0x2A13C812: RFC 5639, BrainpoolP256r1 */
	unsigned int      key_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      maintenance_mode;   /* 0x3C7896E1: enable */
	unsigned char     die_id[DIE_ID_LEN];
	unsigned int      debug_rusk_enable;
	unsigned char     reserved[52]; /* 52 bytes reserved */
	unsigned char     ext_pulic_key_area[ECC_256_KEY_LEN];
	unsigned char     signature[SIG_LEN];
} tee_key_area_s;

/* tee info, size is 0x200 */
typedef struct {
	unsigned int      img_id;
	unsigned int      structure_version;
	unsigned int      structure_length;
	unsigned int      signature_length;
	unsigned int      version_ext;
	unsigned int      mask_version_ext;
	unsigned int      msid_ext;
	unsigned int      mask_msid_ext;
	unsigned int      tee_code_area_addr;
	unsigned int      tee_code_area_len;
	unsigned char     tee_code_area_hash[HASH_LEN];
	para_enc_info     enc_info;
	unsigned int      tee_code_compress_flag; /* 0x3C7896E1: is compressed */
	unsigned int      tee_code_uncompress_len;
	unsigned int      tee_text_segment_size;
	unsigned int      tee_secure_ddr_size;
	unsigned int      atf_code_area_addr;
	unsigned int      atf_code_area_len;
	unsigned char     atf_code_area_hash[HASH_LEN];
	unsigned int      atf_code_compress_flag;
	unsigned int      atf_code_uncompresse_len;
	unsigned int      atf_text_segment_size;
	unsigned char     reserved[0xc0];  /* 0xc0 byte is reserved */
	unsigned char     signature[SIG_LEN];
	unsigned char     signature_ext[SIG_LEN];
} tee_code_info;

/* Boot Image BackUp */
typedef struct {
	int enable;
	unsigned int offset_times;
	unsigned int offset_addr;
} backup_img_params_s;
typedef struct aapcs64_params {
	size_t arg0;
	size_t arg1;
	size_t arg2;
	size_t arg3;
	size_t arg4;
	size_t arg5;
	size_t arg6;
	size_t arg7;
} aapcs64_params_t;

typedef struct param_header {
	u8 type;       /* type of the structure */
	u8 version;    /* version of this structure */
	u16 size;      /* size of this structure in bytes */
	u32 attr;      /* attributes: unused bits SBZ */
} param_header_t;

typedef struct entry_point_info {
	param_header_t h;
	size_t pc;
	u32 spsr;
	aapcs64_params_t args;
} entry_point_info_t;

int get_ree_key_and_paras_info_from_device(const backup_img_params_s *backup_params, u32 channel_type);
int get_paras_data_from_flash(const backup_img_params_s *backup_params, u32 board_index, u32 channel_type);
int get_uboot_info_from_flash(const backup_img_params_s *backup_params, u32 channel_type);
int get_uboot_code_from_flash(const backup_img_params_s *backup_params, u32 channel_type);
