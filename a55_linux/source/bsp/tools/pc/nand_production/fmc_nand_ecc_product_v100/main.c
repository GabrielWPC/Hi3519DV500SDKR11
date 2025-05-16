/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "securec.h"
#include "nandc_ecc.h"
#include "oob_config.h"

#define REVERSE_DATA_LENTH         32
#define DECIMAL 10

struct parm {
	FILE *infile;
	FILE *outfile;
	struct oob_info *info;
	struct oobuse_info *oobuseinfo;
	char *infilepath;
	char *outfilepath;

	unsigned int infilesize;
	unsigned int pagetype;
	unsigned int ecctype;
	unsigned int yaffs;
	unsigned int random;
	unsigned int pagenum;
	unsigned int savepin;

	unsigned int page_ecc_reverse_sector;
	unsigned int block_reverse_sector;
	unsigned int page0_reverse_flag;
	unsigned int page1_reverse_flag;
	/* oobsize: the actual size of the OOB area of the device
	 * oobuse: the size of the OOB zone actually used by logic
	 * info->oobsize: the oobsize,nandcv610 reserved for software use
	 * in the original image is 32 bytes, and if it is a yaffs image,
	 * it needs to be aligned with the pagesize+info->oobsize.
	 */
	unsigned int oobsize;
	unsigned int oobuse;
	unsigned int pagesize;
	unsigned int pageindex;
	unsigned int pagecount;
	unsigned int page_no_total_block;
	unsigned int page_no_incl_data;
	unsigned int page_no_fill_null;

	/* for nandcv610/nandcv620/fmcv100 province pin mode */
	struct reverse_pagesize_ecctype_sector *reverse_page_ecc_sector;
	struct reverse_blocksize_sector *reverse_block_sector;
};

typedef struct parm user_parm;

static int reverse_buf(unsigned char *buf, unsigned int buflen)
{
	unsigned int i;
	for (i = 0; i < buflen; i++)
		buf[i] = ~buf[i];

	return 0;
}

static void help_info_print(char * const *argv)
{
	printf("Usage:\n%s\tinputfile\toutputfile\t"
	       "pagetype\tecctype\toobsize\tyaffs\trandomizer\tpagenum/block\t"
	       "save_pin:\n"
	       "For example:\t\t |\t\t |\t\t |\t\t |\t |\t |\t |\t\t |\t\t |\n"
	       "%s\tin_2k4b.yaffs\tout_2k4b.yaffs\t"
	       " 0\t\t 1\t 64\t 1\t 0\t\t 64\t\t 0\n"
	       "Page type Page size:\n"
	       "Input file:\n"
	       "Output file:\n"
	       "Pagetype:\n"
	       "0        2KB\n"
	       "1        4KB\n"
	       "ECC type ECC size:\n"
	       "1        4bit/512B\n"
	       "2        16bit/1K\n"
	       "3        24bit/1K\n"
	       "Chip OOB size:\n"
	       "yaffs2 image format:\n"
	       "0	 NO\n"
	       "1	 YES\n"
	       "Randomizer:\n"
	       "0        randomizer_disabled\n"
	       "1        randomizer_enabled\n"
	       "Pages_per_block:\n"
	       "64       64pages/block\n"
	       "128      128pages/block\n"
	       "Save Pin Mode:\n"
	       "0	 disable\n"
	       "1	 enable\n", argv[0], argv[0]);
}

static void get_input_data(user_parm *data, char **argv)
{
	/* argv[n]: input parameter of user */
	data->infilepath = argv[1]; /* source file path */
	data->outfilepath = argv[2]; /* 2: Second command line argument,destination file path */
	data->pagetype = (unsigned int)strtol(argv[3], NULL, DECIMAL); /* 3: page size type */
	data->ecctype = (unsigned int)strtol(argv[4], NULL, DECIMAL); /* 4: ecc level */
	data->oobsize = (unsigned int)strtol(argv[5], NULL, DECIMAL); /* 5: nand controler
							best oob size for chip OOB area */
	data->yaffs = (unsigned int)strtol(argv[6], NULL, DECIMAL); /* 6: yaffs mirror or not */
	data->random = (unsigned int)strtol(argv[7], NULL, DECIMAL); /* 7: user data ramdom to nand or not */
	data->pagenum = (unsigned int)strtol(argv[8], NULL, DECIMAL); /* 8: nand page number of one block */
	data->savepin = (unsigned int)strtol(argv[9], NULL, DECIMAL); /* 9: save pin mode,
										only for nandcv620/fmcv100 */

	printf("pagetype=%x,ecctype=%x,oobsize=%x,yaffs=%x,randomizer=%x,pagenum=%x,savepin=%x,infilepath=%s,outfilepath=%s\n",
	       data->pagetype, data->ecctype, data->oobsize, data->yaffs, data->random, data->pagenum,
	       data->savepin,
	       data->infilepath, data->outfilepath);
}

static int get_sector_number(user_parm *data)
{
	if (data->savepin) {
		data->reverse_page_ecc_sector = get_pagesize_ecctype_reverse_sector(data->pagetype, data->ecctype);
		/* for nandc v610/v620 and fmcv100 province pin mode, info->oobsize = 32 */
		if (data->reverse_page_ecc_sector == NULL) {
			fprintf(stderr, "get reverse sector failed.\n");
			return -EINVAL;
		}
		data->page_ecc_reverse_sector = data->reverse_page_ecc_sector->reverse_sec;
		data->reverse_block_sector = get_blocksize_reverse_sector(data->pagenum);
		/* for nandc v610/v620 and fmcv100 province pin mode, info->oobsize = 32 */
		if (data->reverse_block_sector == NULL) {
			fprintf(stderr, "get block sector failed.\n");
			return -EINVAL;
		}
		data->block_reverse_sector = data->reverse_block_sector->reverse_sec;
	}
	if (data->oobsize < data->info->oobsize) {
		fprintf(stderr, "Chip OOB size too small.\n");
		return -EINVAL;
	}

	data->pagesize = get_pagesize(data->pagetype);
	printf("pagesize=%x\n", data->pagesize);
	return 0;
}

static int data_init(user_parm *data, int argc, char **argv)
{
	int ret = 0;

	data->pageindex = 0;
	if (argc != 10) { /* fixed number of user input parameter is 10 */
		help_info_print(argv);
		return -EINVAL;
	}
	get_input_data(data, argv);

	data->page0_reverse_flag = 0;
	data->page1_reverse_flag = 0;

	data->info = get_oob_info(data->pagetype, data->ecctype);
	/* for nandc v610/v620/ and fmcv100, info->oobsize = 32 */
	if (data->info == NULL) {
		fprintf(stderr, "Not support this parameter page: %x ecc: %x\n",
			data->pagetype, data->ecctype);
		return -EINVAL;
	}
	/* For Nandcv610/nandcv620/fmcv100 province pin mode, get sector numbers that require data to be inverted */
	ret = get_sector_number(data);
	if (ret != 0)
		return ret;

	data->infile = fopen(data->infilepath, "rb");
	if (data->infile == NULL) {
		fprintf(stderr, "Could not open input file %s\n", data->infilepath);
		return errno;
	}
	return ret;
}

static int get_input_file_pagenum(user_parm *data)
{
	int ret;
	ret = fseek(data->infile, 0, SEEK_END);
	if (ret != 0) {
		fprintf(stderr, "seek end fail!");
		return -EINVAL;
	}
	data->infilesize = (unsigned int)ftell(data->infile);
	ret = fseek(data->infile, 0, SEEK_SET);
	if (ret != 0) {
		fprintf(stderr, "seek set fail!");
		return -EINVAL;
	}
	printf("infilesize=0x%x\n", data->infilesize);
	if (data->yaffs) {
		if ((data->infilesize == 0) || ((data->infilesize % (data->pagesize + data->info->oobsize)) != 0)) {
			fprintf(stderr, "Input file length is not page + oob aligned."
				"infilesize=%x, pagesize=%x oobsize=%x\n",
				data->infilesize, data->pagesize, data->info->oobsize);
			return -EINVAL;
		}
		data->pagecount = data->infilesize / (data->pagesize + data->info->oobsize);
	} else {
		data->pagecount = (data->infilesize + data->pagesize - 1) / data->pagesize;
	}
	data->page_no_incl_data = data->pagecount;

	return 0;
}

static int set_oob_data(unsigned char *pagebuf, int buflen, user_parm *data)
{
	int ret;

	memset_s(pagebuf, (size_t)buflen, 0xFF, (size_t)buflen);
	if (data->yaffs == 0) {
		ret = (int)fread(pagebuf, 1, (size_t)data->pagesize, data->infile);
		(void)memset_s(pagebuf + data->pagesize, data->info->oobsize, 0xff, data->info->oobsize);
		/* for nandcv610/nandcv620 only, empty page flag. */
		if (data->ecctype != ET_ECC_16BIT1K) {
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE0_OFFSET) = 0;
		} else {
			if (data->pagesize == _2K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE0_OFFSET) = 0;
			} else if (data->pagesize == _4K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE0_OFFSET) = 0;
			}
		}
	} else {
		ret = (int)fread(pagebuf, 1, (size_t)(data->pagesize + data->info->oobsize), data->infile);
		/* for nandcv610/nandcv620 only, empty page flag. */
		*(pagebuf + data->pagesize + OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
		*(pagebuf + data->pagesize + OOB_LEN_NORMAL - EB_BYTE0_OFFSET) = 0;
	}

	return ret;
}

static int data_random_op(unsigned char *pagebuf, user_parm *data)
{
	/* randomzer enable. */
	/* 1. First you need to get the OOB size that the logic actually uses
	* 2. Second, the data is randomizer according to the OOB size actually used by logic.
	*/
	data->oobuseinfo = get_oobuse_info(data->pagetype, data->ecctype);
	if (data->oobsize < data->oobuseinfo->oobuse) {
		fprintf(stderr, "CHIP OOBSIZE too small.\n");
		return -EINVAL;
	}
	if (data->random)
		page_random_gen(pagebuf, data->pagetype, data->ecctype,
				data->pageindex % data->pagenum,
				data->oobuseinfo->oobuse);

	return 0;
}

static void save_pin_mode_op(unsigned char *pagebuf, user_parm *data)
{
	/* For nandcv610/nandcv620 province pin mode, Take the reverse for pag0
	* (pagesize and ECC combination) and Page1 (blocksize) sector data
	*/
	/* First, reverse the corresponding sector data of the page0. */
	if ((data->savepin != 0) && (data->page0_reverse_flag == 0)) {
		unsigned char *reverse_sector;
		reverse_sector = pagebuf + REVERSE_DATA_LENTH * data->page_ecc_reverse_sector;
		reverse_buf(reverse_sector, REVERSE_DATA_LENTH);
	}
	/* After the page0 data is inverted, the data of the page1 is inverted.  */
	if ((data->savepin != 0) && (data->page0_reverse_flag != 0) && (data->page1_reverse_flag == 0)) {
		unsigned char *reverse_sector;
		reverse_sector = pagebuf + REVERSE_DATA_LENTH * data->block_reverse_sector;
		reverse_buf(reverse_sector, REVERSE_DATA_LENTH);
		data->page1_reverse_flag = 1;
	}
	data->page0_reverse_flag = 1;
}

static int fill_null_page(unsigned char *pagebuf, unsigned int buflen, user_parm *data)
{
	int ret = 0;
	size_t len;

	data->page_no_total_block = data->page_no_incl_data & ~(data->pagenum - 1);
	data->page_no_total_block += ((data->page_no_incl_data & (data->pagenum - 1)) != 0) ? data->pagenum : 0;
	data->page_no_fill_null = data->page_no_total_block - data->page_no_incl_data;

	printf("Total block page number:%x\n", data->page_no_total_block);
	printf("Include data page number:%x\n", data->page_no_incl_data);
	printf("Need fill NULL page number:%x\n", data->page_no_fill_null);

	while (data->page_no_fill_null--) {
		memset_s(pagebuf, buflen, 0xff, buflen);

		if (data->yaffs == 0)
			memset_s(pagebuf + data->pagesize, data->info->oobsize, 0xff, data->info->oobsize);

		if (data->ecctype != ET_ECC_16BIT1K) {
			*(pagebuf + data->pagesize +
				OOB_LEN_NORMAL - EB_BYTE1_OFFSET) = 0;
			*(pagebuf + data->pagesize + OOB_LEN_NORMAL -
				EB_BYTE0_OFFSET) = 0;
		} else {
			if (data->pagesize == _2K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_2KPAGE - EB_BYTE0_OFFSET) = 0;
			} else if (data->pagesize == _4K) {
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE1_OFFSET) = 0;
				*(pagebuf + data->pagesize +
					OOB_LEN_ECC_16BIT1K_4KPAGE - EB_BYTE0_OFFSET) = 0;
			}
		}

		ret = page_ecc_gen(pagebuf, data->pagetype, data->ecctype);
		if (ret < 0) {
			fprintf(stderr, "page_ecc_gen error.\n");
			return ret;
		}

		len = fwrite(pagebuf, 1, (size_t)(data->pagesize + data->oobsize), data->outfile);
		if (len != (size_t)(data->pagesize + data->oobsize)) {
			fprintf(stderr, "Could not fill NULL page in output file %s\n", data->outfilepath);
			ret = errno;
			return ret;
		}
	}

	return ret;
}

static int fill_ecc_oob_to_page(unsigned char *pagebuf, int buflen, user_parm *data)
{
	int ret;
	size_t len;

	printf("pagecount=%x\n", data->pagecount);
	printf("outfilepath=%s\n", data->outfilepath);
	while (data->pagecount--) {
		ret = set_oob_data(pagebuf, buflen, data);
		if (ret == 0)
			break;

		if (ret < 0) {
			fprintf(stderr, "Could not read input file %s\n", data->infilepath);
			ret = errno;
			return ret;
		}

		ret = page_ecc_gen(pagebuf, data->pagetype, data->ecctype);
		if (ret < 0) {
			fprintf(stderr, "page_ecc_gen error.\n");
			return ret;
		}
		ret = data_random_op(pagebuf, data);
		if (ret)
			return ret;

		save_pin_mode_op(pagebuf, data);
		len = fwrite(pagebuf, 1, (size_t)(data->pagesize + data->oobsize), data->outfile);
		if (len != (size_t)(data->pagesize + data->oobsize)) {
			fprintf(stderr, "Could not write output file %s\n", data->outfilepath);
			ret = errno;
			return ret;
		}
		data->pageindex++;
	}
}

int main(int argc, char **argv)
{
	unsigned char *pagebuf = NULL;
	int ret;
	user_parm *data = NULL;

	data = malloc(sizeof(user_parm));
	if (data == NULL)
		return -ENOMEM;
	pagebuf = (unsigned char *)malloc(sizeof(unsigned char) * (MAX_PAGE_SIZE + MAX_OOB_SIZE));
	if (pagebuf == NULL) {
		ret = -ENOMEM;
		goto fail_free_data;
	}

	memset_s(data, sizeof(user_parm), 0, sizeof(user_parm));

	ret = data_init(data, argc, argv);
	if (ret)
		goto fail_close_file;

	ret = get_input_file_pagenum(data);
	if (ret)
		goto fail_close_file;

	data->outfile = fopen(data->outfilepath, "wb");
	if (data->outfile == NULL) {
		fprintf(stderr, "Could not open output file %s\n", data->outfilepath);
		ret = errno;
		goto fail_close_file;
	}
	ret = fill_ecc_oob_to_page(pagebuf, (MAX_PAGE_SIZE + MAX_OOB_SIZE), data);
	if (ret)
		goto fail_close_file;

	ret = fill_null_page(pagebuf, (MAX_PAGE_SIZE + MAX_OOB_SIZE), data);
	if (ret)
		goto fail_close_file;

	ret = 0;
	if ((data->savepin != 0) && ((data->page0_reverse_flag == 0) || (data->page1_reverse_flag == 0))) {
		printf("savepin mode: reverse data failed.\n");
		ret = -1;
	}

fail_close_file:
	if (data->infile)
		fclose(data->infile);
	if (data->outfile)
		fclose(data->outfile);
	free(pagebuf);
fail_free_data:
	free(data);
	return ret;
}
