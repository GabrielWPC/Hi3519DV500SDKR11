/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef OOB_CONFIGH
#define OOB_CONFIGH

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#define _2K	2048
#define _4K	4096
#define _16K 0x4000

#define OOB_LEN_ECC_16BIT1K_2KPAGE		8
#define OOB_LEN_ECC_16BIT1K_4KPAGE		16
#define OOB_LEN_NORMAL				32

#define EB_BYTE0_OFFSET		2
#define EB_BYTE1_OFFSET		1

#define MAX_PAGE_SIZE	16384	/* must be greater than or equal to the actual pagesize size of the device */
#define MAX_OOB_SIZE	2000    /* must be greater than or equal to the actual OOB area size of the device */

#define NANDIP_V100    0x100
#define NANDIP_V300    0x300
#define NANDIP_V301    0x301
#define NANDIP_V504    0x504
#define NANDIP_V610    0x610

/* this follow must be consistent with fastboot !!! */
enum ecc_type {
	ET_ECC_ONE    = 0x00,
	ET_ECC_8BIT1K  = 0x01,
	ET_ECC_16BIT1K = 0x02,
	ET_ECC_24BIT1K = 0x03,
	ET_ECC_28BIT1K = 0x04,
	ET_ECC_40BIT1K = 0x05,
	ET_ECC_64BIT1K = 0x06,
	ET_ECC_LAST    = 0x07,
};

enum page_type {
	PT_PAGESIZE_2K    = 0x00,
	PT_PAGESIZE_4K    = 0x01,
	PT_PAGESIZE_8K    = 0x02,
	PT_PAGESIZE_16K   = 0x03,
	PT_PAGESIZE_LAST  = 0x04,
};

enum blocksize_type {
	PT_BLOCKSIZE_64    = 64,
	PT_BLOCKSIZE_128   = 128,
	PT_BLOCKSIZE_256   = 256,
	PT_BLOCKSIZE_512   = 512,
	PT_BLOCKSIZE_LAST  = 0x04,
};

struct nand_oob_free {
	unsigned long offset;
	unsigned long length;
};

struct oob_info {
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int oobsize;
	struct nand_oob_free *freeoob;
	int random;
};
struct reverse_pagesize_ecctype_sector {
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int reverse_sec;
};

struct reverse_blocksize_sector {
	unsigned int nandip;
	enum blocksize_type blocksize;
	unsigned int reverse_sec;
};
struct oobuse_info {
	unsigned int nandip;
	enum page_type pagetype;
	enum ecc_type ecctype;
	unsigned int oobuse;
};

struct reverse_pagesize_ecctype_sector *get_pagesize_ecctype_reverse_sector(enum page_type pagetype,
		enum ecc_type ecctype);
struct reverse_blocksize_sector *get_blocksize_reverse_sector(unsigned int blocksize);

EXTERN_C struct oob_info *get_n_oob_info(int selectindex);

struct oob_info *get_oob_info(enum page_type pagetype,
		enum ecc_type ecctype);

EXTERN_C char *get_ecctype_str(enum ecc_type ecctype);

EXTERN_C char *get_pagesize_str(enum page_type pagetype);

EXTERN_C unsigned int get_pagesize(enum page_type pagetype);

extern const char *nand_controller_version;

/* oob_config_v100.c func */
struct oobuse_info *get_oobuse_info(enum page_type pagetype,
		enum ecc_type ecctype);

#endif /* OOB_CONFIGH */
