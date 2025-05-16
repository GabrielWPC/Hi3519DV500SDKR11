/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#ifndef NANDC_ECC_H
#define NANDC_ECC_H

#include "oob_config.h"

#define ECC_L1_LEN		8
#define ECC_H1_LEN		8
#define ENC_DATA_LEN 		24
#define ECC_DATA_IN_LEN		16
#define ADDR_IN_LEN		9


#define ECC_LEN_MULTY_8		8
#define ECC_LEN_MULTY_14	14

#define ECC_LEVLE_4BIT		8
#define ECC_LEN_4BIT		14

#define ECC_LEVLE_8BIT		16
#define ECC_LEN_8BIT		28

#define ECC_LEVLE_13BIT1K	13
#define ECC_LEN_13BIT		24

#define ECC_LEVLE_24BIT1K	24
#define ECC_LEN_24BIT1K	42

#define ECC_LEVLE_27BIT1K	27
#define ECC_LEN_27BIT1K		48

#define ECC_LEVLE_40BIT1K	40
#define ECC_LEN_40BIT1K		70

#define ECC_LEVLE_64BIT1K	64
#define ECC_LEN_64BIT1K		112

#define ECC_LEVLE_41BIT1K	41
#define ECC_LEN_41BIT1K		112

#define ECC_LEVLE_60BIT1K	60
#define ECC_LEN_60BIT1K		108

#define ECC_LEVLE_80BIT1K	80
#define ECC_LEN_80BIT1K		140

#define	TOTAL_NUMS_4K	3
#define	TOTAL_NUMS_8K	7
#define	TOTAL_NUMS_16K	14

#define BB_LEN			2

#define ECC_LEN_8BIT1K	14

#define DATA_LEN0_8BIT1K_2K	1040
#define DATA_LEN1_8BIT1K_2K	994
#define DATA_LEN2_8BIT1K_2K	14
#define CTRL_LEN_8BIT1K_2K	30

#define DATA_LEN0_8BIT1K_4K	1032
#define DATA_LEN1_8BIT1K_4K	958
#define DATA_LEN2_8BIT1K_4K	42
#define CTRL_LEN_8BIT1K_4K	30

#define ECC_LEN_16BIT1K	28

#define DATA_LEN0_16BIT1K_2K	1028
#define DATA_LEN1_16BIT1K_2K	992
#define DATA_LEN2_16BIT1K_2K	28
#define CTRL_LEN_16BIT1K_2K	6

#define DATA_LEN0_16BIT1K_4K	1028
#define DATA_LEN1_16BIT1K_4K	928
#define DATA_LEN2_16BIT1K_4K	84
#define CTRL_LEN_16BIT1K_4K	14

#define ECC_LEN_24BIT1K	42

#define DATA_LEN0_24BIT1K_2K	1040
#define DATA_LEN1_24BIT1K_2K	966
#define DATA_LEN2_24BIT1K_2K	42
#define CTRL_LEN_24BIT1K_2K	30

#define DATA_LEN0_24BIT1K_4K	1032
#define DATA_LEN1_24BIT1K_4K	874
#define DATA_LEN2_24BIT1K_4K	126
#define CTRL_LEN_24BIT1K_4K	30

#define DATA_LEN0_24BIT1K_8K	1028
#define DATA_LEN1_24BIT1K_8K	702
#define DATA_LEN2_24BIT1K_8K	294
#define CTRL_LEN_24BIT1K_8K	30

#define ECC_LEN_40BIT1K	70

#define DATA_LEN0_40BIT1K_8K	1028
#define DATA_LEN1_40BIT1K_8K	506
#define DATA_LEN2_40BIT1K_8K	490
#define CTRL_LEN_40BIT1K_8K	30

#define ECC_LEN0_40BIT1K_16K	14
#define ECC_LEN1_40BIT1K_16K	56
#define DATA_LEN0_40BIT1K_16K	1026
#define DATA_LEN1_40BIT1K_16K	994
#define CTRL_LEN_40BIT1K_16K	30

#define ECC_LEN_64BIT1K	112

#define DATA_LEN0_64BIT1K_8K	1028
#define DATA_LEN1_64BIT1K_8K	212
#define DATA_LEN2_64BIT1K_8K	784
#define CTRL_LEN_64BIT1K_8K	30

#define DATA_LEN0_64BIT1K_16K	1026
#define DATA_LEN1_64BIT1K_16K	452
#define DATA_LEN2_64BIT1K_16K	574
#define DATA_LEN3_64BIT1K_16K	994
#define CTRL_LEN_64BIT1K_16K	30

#define USER_DATA_LEN_16	16
#define USER_DATA_LEN_256	256

#define MAX_LFSR_BITS 		2048

int page_ecc_gen(unsigned char *pagebuf,
		  enum page_type pagetype,
		  enum ecc_type ecctype);
void page_random_gen(unsigned char *pagebuf,
			    enum page_type pagetype,
			    enum ecc_type ecctype,
			    unsigned int pageindex,
			    unsigned int oobsize);
#endif
