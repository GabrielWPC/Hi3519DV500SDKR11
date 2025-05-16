/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include "oob_config.h"


#define NULL    (void *)0

const char *nand_controller_version = "Flash Memory Controller V100";

struct nand_oob_free oobfree_def[] = {
	{2, 30}, {0, 0},
};

struct nand_oob_free oobfree_def_2k16bit[] = {
	{2, 6}, {0, 0},
};

struct nand_oob_free oobfree_def_4k16bit[] = {
	{2, 14}, {0, 0},
};

static struct oob_info fmc100_oob_info[] = {
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_8BIT1K,   64, oobfree_def, 0},
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_16BIT1K,  64, oobfree_def_2k16bit, 0},
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_24BIT1K, 128, oobfree_def, 0},

	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_8BIT1K,  128, oobfree_def, 0},
	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_16BIT1K, 128, oobfree_def_4k16bit, 0},
	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_24BIT1K, 200, oobfree_def, 0},

	{0},
};

static struct oobuse_info fmc100_oobuse_info[] = {
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_8BIT1K,     60},
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_16BIT1K,    64},
	{NANDIP_V100, PT_PAGESIZE_2K, ET_ECC_24BIT1K,   116},

	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_8BIT1K,     88},
	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_16BIT1K,   128},
	{NANDIP_V100, PT_PAGESIZE_4K, ET_ECC_24BIT1K,   200},

	{0},
};

static struct reverse_pagesize_ecctype_sector fmc100_page_ecc_reverse_sector[] = {
	{NANDIP_V100, PT_PAGESIZE_2K,  ET_ECC_8BIT1K,    0},
	{NANDIP_V100, PT_PAGESIZE_2K,  ET_ECC_24BIT1K,   1},
	{NANDIP_V100, PT_PAGESIZE_4K,  ET_ECC_8BIT1K,    2},
	{NANDIP_V100, PT_PAGESIZE_4K,  ET_ECC_24BIT1K,   3},

	{0},
};

static struct reverse_blocksize_sector fmc100_blocksize_reverse_sector[] = {
	{NANDIP_V100, PT_BLOCKSIZE_64,     0},
	{NANDIP_V100, PT_BLOCKSIZE_128,    1},
	{NANDIP_V100, PT_BLOCKSIZE_256,    2},
	{NANDIP_V100, PT_BLOCKSIZE_512,    3},

	{0},
};

struct oob_info *get_n_oob_info(int selectindex)
{
	struct oob_info *info = fmc100_oob_info;
	int index = 0;

	for (; info->freeoob; info++, index++) {
		if (index == selectindex)
			return info;
	}

	return 0;
}

struct oob_info *get_oob_info(enum page_type pagetype,
			      enum ecc_type ecctype)
{
	struct oob_info *info = fmc100_oob_info;

	for (; info->freeoob; info++) {
		if (info->ecctype == ecctype
		    && info->pagetype == pagetype)
			return info;
	}

	return NULL;
}

struct oobuse_info *get_oobuse_info(enum page_type pagetype,
				    enum ecc_type ecctype)
{
	struct oobuse_info *info = fmc100_oobuse_info;

	for (; info->nandip; info++) {
		if (info->ecctype == ecctype
		    && info->pagetype == pagetype)
			return info;
	}

	return NULL;
}

struct reverse_pagesize_ecctype_sector *get_pagesize_ecctype_reverse_sector(enum page_type pagetype,
		enum ecc_type ecctype)
{
	struct reverse_pagesize_ecctype_sector *sector = fmc100_page_ecc_reverse_sector;

	for (; sector->nandip; sector++) {
		if (sector->ecctype == ecctype
		    && sector->pagetype == pagetype)
			return sector;
	}

	return NULL;
}

struct reverse_blocksize_sector *get_blocksize_reverse_sector(unsigned int blocksize)
{
	struct reverse_blocksize_sector *sector = fmc100_blocksize_reverse_sector;

	for (; sector->nandip; sector++) {
		if (sector->blocksize == blocksize)
			return sector;
	}

	return NULL;
}

char *get_ecctype_str(enum ecc_type ecctype)
{
	char *ecctype_str[ET_ECC_LAST + 1] = { (char *)"None", (char *)"8bit/1K",
					(char *)"16bit/1K", (char *)"24bits/1K",
					(char *)"28bits/1K",
					(char *)"40bits/1K", (char *)"64bits/1K"};
	if (ecctype < ET_ECC_8BIT1K || ecctype > ET_ECC_LAST)
		ecctype = ET_ECC_LAST;
	return ecctype_str[ecctype];
}

char *get_pagesize_str(enum page_type pagetype)
{
	char *pagesize_str[PT_PAGESIZE_LAST + 1] = {(char *)"2K",
						    (char *)"4K", (char *)"8K",
						    (char *)"16K", (char *)"unknown"};
	if (pagetype < PT_PAGESIZE_2K || pagetype >= PT_PAGESIZE_LAST)
		pagetype = PT_PAGESIZE_LAST;
	return pagesize_str[pagetype];
}

unsigned int get_pagesize(enum page_type pagetype)
{
	unsigned int pagesize[] = {2048, 4096, 8192, 16384, 0};
	return pagesize[pagetype];
}
