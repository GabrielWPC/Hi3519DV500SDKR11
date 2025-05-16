/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
#include <stdio.h>
#include "securec.h"
#include "oob_config.h"
#include "nandc_ecc.h"

struct lv2_ecc_low_8bit_data {
	unsigned char ecc_p4ol;
	unsigned char ecc_p4el;
	unsigned char ecc_p2ol;
	unsigned char ecc_p2el;
	unsigned char ecc_p1ol;
	unsigned char ecc_p1el;
};

struct lv2_ecc_high_8bit_data {
	unsigned char ecc_p4oh;
	unsigned char ecc_p4eh;
	unsigned char ecc_p2oh;
	unsigned char ecc_p2eh;
	unsigned char ecc_p1oh;
	unsigned char ecc_p1eh;
};

struct lv3_ecc_data {
	unsigned char ecc_p8o;
	unsigned char ecc_p8e;
	unsigned char ecc_p4o;
	unsigned char ecc_p4e;
	unsigned char ecc_p2o;
	unsigned char ecc_p2e;
	unsigned char ecc_p1o;
	unsigned char ecc_p1e;
};

unsigned int lfsr_len;
unsigned char lfsr_poly[MAX_LFSR_BITS];
unsigned char lfsr_value[MAX_LFSR_BITS];

static void ecc_calculate(struct lv3_ecc_data *lv3ecc_data, const unsigned char *ecc_data_in, unsigned char *ecc_pr)
{
	unsigned char ecc_l1[ECC_L1_LEN];
	unsigned char ecc_h1[ECC_H1_LEN];
	struct lv2_ecc_low_8bit_data lv2ecc_l;
	struct lv2_ecc_high_8bit_data lv2ecc_h;

	ecc_l1[0] = ecc_data_in[0] ^ ecc_data_in[1]; /* 0 0 1: Array index */
	ecc_l1[1] = ecc_data_in[0] ^ ecc_data_in[2]; /* 1 0 2: Array index */
	ecc_l1[2] = ecc_data_in[1] ^ ecc_data_in[3]; /* 2 1 3: Array index */
	ecc_l1[3] = ecc_data_in[2] ^ ecc_data_in[3]; /* 3 2 3: Array index */
	ecc_l1[4] = ecc_data_in[4] ^ ecc_data_in[5]; /* 4 4 5: Array index */
	ecc_l1[5] = ecc_data_in[4] ^ ecc_data_in[6]; /* 5 4 6: Array index */
	ecc_l1[6] = ecc_data_in[5] ^ ecc_data_in[7]; /* 6 5 7: Array index */
	ecc_l1[7] = ecc_data_in[6] ^ ecc_data_in[7]; /* 7 6 7: Array index */

	/* level one ECC calculate for high 8bit */

	ecc_h1[0] = ecc_data_in[8] ^ ecc_data_in[9];   /* 0 8 9: Array index */
	ecc_h1[1] = ecc_data_in[8] ^ ecc_data_in[10];  /* 1 8 10: Array index */
	ecc_h1[2] = ecc_data_in[9] ^ ecc_data_in[11];  /* 2 9 11: Array index */
	ecc_h1[3] = ecc_data_in[10] ^ ecc_data_in[11]; /* 3 10 11: Array index */
	ecc_h1[4] = ecc_data_in[12] ^ ecc_data_in[13]; /* 4 12 13: Array index */
	ecc_h1[5] = ecc_data_in[12] ^ ecc_data_in[14]; /* 5 12 14: Array index */
	ecc_h1[6] = ecc_data_in[13] ^ ecc_data_in[15]; /* 6 13 15: Array index */
	ecc_h1[7] = ecc_data_in[14] ^ ecc_data_in[15]; /* 7 14 15: Array index */

	/* level two ECC calculate for low 8bit */
	lv2ecc_l.ecc_p4ol = ecc_l1[1] ^ ecc_l1[2]; /* 1 2: Array index */
	lv2ecc_l.ecc_p2ol = ecc_l1[0] ^ ecc_l1[4]; /* 0 4: Array index */
	lv2ecc_l.ecc_p1ol = ecc_l1[1] ^ ecc_l1[5]; /* 1 5: Array index */
	lv2ecc_l.ecc_p1el = ecc_l1[2] ^ ecc_l1[6]; /* 2 6: Array index */
	lv2ecc_l.ecc_p2el = ecc_l1[3] ^ ecc_l1[7]; /* 3 7: Array index */
	lv2ecc_l.ecc_p4el = ecc_l1[5] ^ ecc_l1[6]; /* 5 6: Array index */

	/* level two ECC calculate for high 8bit */
	lv2ecc_h.ecc_p4oh = ecc_h1[1] ^ ecc_h1[2]; /* 1 2: Array index */
	lv2ecc_h.ecc_p2oh = ecc_h1[0] ^ ecc_h1[4]; /* 0 4: Array index */
	lv2ecc_h.ecc_p1oh = ecc_h1[1] ^ ecc_h1[5]; /* 1 5: Array index */
	lv2ecc_h.ecc_p1eh = ecc_h1[2] ^ ecc_h1[6]; /* 2 6: Array index */
	lv2ecc_h.ecc_p2eh = ecc_h1[3] ^ ecc_h1[7]; /* 3 7: Array index */
	lv2ecc_h.ecc_p4eh = ecc_h1[5] ^ ecc_h1[6]; /* 5 6: Array index */

	/* level three ECC calculate */
	lv3ecc_data->ecc_p1o = lv2ecc_l.ecc_p1ol ^ lv2ecc_h.ecc_p1oh;
	lv3ecc_data->ecc_p1e = lv2ecc_l.ecc_p1el ^ lv2ecc_h.ecc_p1eh;
	lv3ecc_data->ecc_p2o = lv2ecc_l.ecc_p2ol ^ lv2ecc_h.ecc_p2oh;
	lv3ecc_data->ecc_p2e = lv2ecc_l.ecc_p2el ^ lv2ecc_h.ecc_p2eh;
	lv3ecc_data->ecc_p4o = lv2ecc_l.ecc_p4ol ^ lv2ecc_h.ecc_p4oh;
	lv3ecc_data->ecc_p4e = lv2ecc_l.ecc_p4el ^ lv2ecc_h.ecc_p4eh;
	lv3ecc_data->ecc_p8o = lv2ecc_l.ecc_p2ol ^ lv2ecc_l.ecc_p2el;
	lv3ecc_data->ecc_p8e = lv2ecc_h.ecc_p2oh ^ lv2ecc_h.ecc_p2eh;

	/* line ECC calculate */
	*ecc_pr  = lv3ecc_data->ecc_p8o ^ lv3ecc_data->ecc_p8e;
	return;
}


/*
 * enc_data_last[24] initial value 0xFFFFFF ;
 * [  23    22    21     20   19    18   17    16   15    14  ]
 * P2048 P2048' P1024 P1024' P512 P512' P256 P256' P128 P128'
 * [ 13  12   11  10   9   8    7  6  5   4  3   2  1   0 ]
 * P64 P64' P32 P32' P16 P16' P8 P8' P4 P4' P2 P2' P1 P1'
 * addr said how many bytes
 */
static void make_ecc_1bit(unsigned int addr, unsigned char data_in, unsigned char *enc_data,
		   const unsigned char *enc_data_last)
{
	unsigned int i;
	/* ecc_data_in to ECC calculate module */
	unsigned char nxt_ecc_reg[ENC_DATA_LEN];
	struct lv3_ecc_data lv3ecc_data;
	unsigned char ecc_pr;
	unsigned char ecc_data_in[ECC_DATA_IN_LEN] = {0};
	unsigned char addr_in[ADDR_IN_LEN] = {0};
	for (i = 0; i < (ECC_DATA_IN_LEN / 2); i++) /* div 2:first 8 bytes init */
		ecc_data_in[i] = (data_in >> i) & 0x1;

	for (i = 0; i < ADDR_IN_LEN; i++)
		addr_in[i] = (addr >> i) & 0x1;

	ecc_calculate(&lv3ecc_data, ecc_data_in, &ecc_pr);

	/* generate all row and column ecc result for one read/write cylce nxt_ecc_reg[23:0] = 24'h0; */
	nxt_ecc_reg[0] = lv3ecc_data.ecc_p1o; /* 0: Array index */
	nxt_ecc_reg[1] = lv3ecc_data.ecc_p1e; /* 1: Array index */
	nxt_ecc_reg[2] = lv3ecc_data.ecc_p2o; /* 2: Array index */
	nxt_ecc_reg[3] = lv3ecc_data.ecc_p2e; /* 3: Array index */
	nxt_ecc_reg[4] = lv3ecc_data.ecc_p4o; /* 4: Array index */
	nxt_ecc_reg[5] = lv3ecc_data.ecc_p4e; /* 5: Array index */

	nxt_ecc_reg[6] = (~addr_in[0]) & ecc_pr; /* 6 0: Array index */
	nxt_ecc_reg[7] =   addr_in[0]  & ecc_pr; /* 7 0: Array index */

	nxt_ecc_reg[8]  = (~addr_in[1]) & ecc_pr; /* 8 1: Array index */
	nxt_ecc_reg[9]  =   addr_in[1]  & ecc_pr; /* 9 1: Array index */
	nxt_ecc_reg[10] = (~addr_in[2]) & ecc_pr; /* 10 2: Array index */
	nxt_ecc_reg[11] =   addr_in[2]  & ecc_pr; /* 11 2: Array index */
	nxt_ecc_reg[12] = (~addr_in[3]) & ecc_pr; /* 12 3: Array index */
	nxt_ecc_reg[13] =   addr_in[3]  & ecc_pr; /* 13 3: Array index */
	nxt_ecc_reg[14] = (~addr_in[4]) & ecc_pr; /* 14 4: Array index */
	nxt_ecc_reg[15] =   addr_in[4]  & ecc_pr; /* 15 4: Array index */
	nxt_ecc_reg[16] = (~addr_in[5]) & ecc_pr; /* 16 5: Array index */
	nxt_ecc_reg[17] =   addr_in[5]  & ecc_pr; /* 17 5: Array index */
	nxt_ecc_reg[18] = (~addr_in[6]) & ecc_pr; /* 18 6: Array index */
	nxt_ecc_reg[19] =   addr_in[6]  & ecc_pr; /* 19 6: Array index */
	nxt_ecc_reg[20] = (~addr_in[7]) & ecc_pr; /* 20 7: Array index */
	nxt_ecc_reg[21] =   addr_in[7]  & ecc_pr; /* 21 7: Array index */
	nxt_ecc_reg[22] = (~addr_in[8]) & ecc_pr; /* 22 8 : Array index */
	nxt_ecc_reg[23] =   addr_in[8]  & ecc_pr; /* 23 8: Array index */

	for (i = 0; i < ENC_DATA_LEN; i++)
		enc_data[i] = enc_data_last[i] ^ nxt_ecc_reg[i];
	return;
}

static void ecc_1bit_gen(const unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	unsigned int i;
	unsigned char enc_data_last[ENC_DATA_LEN] = {0x1};
	unsigned char enc_data[ENC_DATA_LEN] = {0x0};

	if (memset_s(enc_data_last, ENC_DATA_LEN, 0x1, ENC_DATA_LEN))
		return;

	for (i = 0; i < len; i++) {
		make_ecc_1bit(i, data[i], enc_data, enc_data_last);
		memcpy_s(enc_data_last, ENC_DATA_LEN, enc_data, ENC_DATA_LEN);
	}

	if (len == USER_DATA_LEN_16) {
		enc_data[14] = 0x1; /* 14: Array index */
		enc_data[15] = 0x1; /* 15: Array index */
	}

	if (len >= USER_DATA_LEN_256) {
		for (i = 0; i < 3; i++) /* 3 cycle: enc_data[0] ~ ecc_data[ENC_DATA_LEN - 1] */
			/* 2 - i and i * 8 + 1:Array index */
			ecc_code[2 - i] = enc_data[i * 8]  + (enc_data[i * 8 + 1] << 1) +
					  /* i * 8 + 2 and i * 8 + 3:Array index */
					  (enc_data[i * 8 + 2] << 2) + (enc_data[i * 8 + 3] << 3) +
					  /* i * 8 + 4 and i * 8 + 5:Array index */
					  (enc_data[i * 8 + 4] << 4) + (enc_data[i * 8 + 5] << 5) +
					  /* i * 8 + 6 and i * 8 + 7:Array index */
					  (enc_data[i * 8 + 6] << 6) + (enc_data[i * 8 + 7] << 7);
	} else {
		for (i = 0; i < 2; i++) /* 2 cycle: enc_data[0] ~ ecc_data[(ENC_DATA_LEN / 2) - 1] */
			/* 1 - i and i * 8: Array index */
			ecc_code[1 - i] = enc_data[i * 8]  + (enc_data[i * 8 + 1] << 1) +
					  /* i * 8 + 2 and i * 8 + 3:Array index */
					  (enc_data[i * 8 + 2] << 2) + (enc_data[i * 8 + 3] << 3) +
					  /* i * 8 + 4 and i * 8 + 5:Array index */
					  (enc_data[i * 8 + 4] << 4) + (enc_data[i * 8 + 5] << 5) +
					  /* i * 8 + 6 and i * 8 + 7:Array index */
					  (enc_data[i * 8 + 6] << 6) + (enc_data[i * 8 + 7] << 7);
	}
}

static void inttolfsr(unsigned char *lfsr, unsigned int value)
{
	unsigned int i;
	for (i = 0; i <= lfsr_len; i++) {
		if (value & (1 << i))
			lfsr[i] = 1;
		else
			lfsr[i] = 0;
	}
}
static void strtolfsr(unsigned char *lfsr, const unsigned char *value)
{
	unsigned int i;
	unsigned char c;
	size_t len = strlen(value);

	for (i = 0; i < len; i++) {
		c = *(value + len - 1 - i);
		if (c == '1')
			lfsr[i] = 1;
		else
			lfsr[i] = 0;
	}
}

static void lfsr_init(unsigned int len, const unsigned char *poly, unsigned int value)
{
	lfsr_len = len;
	memset_s(lfsr_poly, MAX_LFSR_BITS, 0x00, MAX_LFSR_BITS);
	memset_s(lfsr_value, MAX_LFSR_BITS, 0x00, MAX_LFSR_BITS);
	strtolfsr(lfsr_poly, poly);
	inttolfsr(lfsr_value, value);
}

static void parity_lfsr_shift(unsigned char din)
{
	unsigned char feedback;
	int i;

	feedback = lfsr_value[lfsr_len - 1] ^ din;

	for (i = lfsr_len - 1; i > 0; i--)
		lfsr_value[i] = (feedback & lfsr_poly[i]) ^ lfsr_value[i - 1];

	lfsr_value[0] = (feedback & lfsr_poly[0]);
}

static void get_parity(unsigned char *parity)
{
	int i;
	unsigned int value;
	unsigned int shift;

	shift = 0;
	value = 0;
	for (i = lfsr_len - 1; i >= 0; i--) {
		value |= lfsr_value[i] << shift;
		shift++;
		if (shift == 8) { /* 8: ecc algorithm shift */
			*parity = value;
			parity++;
			shift = 0;
			value = 0;
		}
	}
}

static int ecc_parity_gen(const unsigned char *data, unsigned int bits, unsigned int ecc_level,
		   unsigned char *ecc_code)
{
	unsigned int i;
	unsigned char c;

	switch (ecc_level) {
	case ECC_LEVLE_4BIT:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_4BIT,
			"b111111100111101110010111111111100101001110000100001111000111011001011001"
			"1110001001110011110011010101110000101101", 0);
		break;
	case ECC_LEVLE_8BIT:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_8BIT,
			"b110011000010010000001100011100000100101010100011010010010001010011000010"
			"1010001010010001001000000101100101001111011111111101011111100011110000111"
			"1010101100110000100010011101001111011011000100110101010100000110111011011"
			"001111", 0);
		break;
	case ECC_LEVLE_24BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_24BIT1K,
			"b100011101001010011100000001001001000110110010000100111010010101101000101"
			"0010010101110010110100011110110111011001110100001001100011111110011100110"
			"0001110100011101000110100100110110000101101001000101000100100111010001110"
			"1000000100100001011011110100001010101101101110000010110100100110010010100"
			"110100011010101101011110101000011000011101111", 0);
		break;
	/* 28bit, todo */
	case ECC_LEVLE_40BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_40BIT1K,
			"b110000000111111110001001101100011010000011011100010111011001011001100001"
			"1001111100110010110100000100100101100111010101001111011011011110100111010"
			"1001111100100111111010100100111111011110001010011101111101100001111110101"
			"0100111001100100010101101010000010110011001101100100100101010100101000100"
			"0000000110000010001110111110111101110001111110011001110001010010110011110"
			"1100010111111010001000010000101011111110001011101111110111111010111011010"
			"0100100010011011111001100010100110110100101000001110110001010110111001001"
			"00101000000000001010011100111011110010110111000001", 0);
		break;
	case ECC_LEVLE_64BIT1K:
		lfsr_init(ECC_LEN_MULTY_14 * ECC_LEVLE_64BIT1K,
			"b111101111011000101001110011100110011011000000011110001111001111011101011"
			"0010111011010110000011111101011110100010100000010110110001100100100111011"
			"1010011000111100000011110001110010011001001101100011111000100010001010111"
			"0001010101000111110001000111010001001101000101000001100001011010001011110"
			"1000011111101010010000101110101101011101010101101000100010110110101101100"
			"1110111111100001010111101110010100000010000111111011001110001011001011100"
			"0000011000100000000000111010000100110011111100010000010100100010101010010"
			"1101001101011100110001000110100101101100110011111111110001110000111010000"
			"1001000110011011110101100001010101011000101010010101001111000011010000100"
			"0110101010100001010111010010100111011000110101011000010110111011000011110"
			"0110110011011101111001110000101010000111011110111110110001001000101011011"
			"0101100010001110000000101110100100010111000111111101001110000100000001011"
			"110100001000001010101", 0);
		break;
	default:
		return -1;
	}

	for (i = 0; i < bits; i++) {
		c = *(data + (i >> 3)); /* 3: ecc algorithm offset */
		c = (c >> (i & 0x7)) & 0x1;
		parity_lfsr_shift(c);
	}

	get_parity(ecc_code);

	return 0;
}

static void ecc_data_gen(unsigned char *data, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		data[i] = ~data[i];
}

static int ecc_4bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_4BIT, ecc_code);
	for (i = 0; i < ECC_LEN_4BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_8bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_8BIT, ecc_code);
	for (i = 0; i < ECC_LEN_8BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_13bit_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_13BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_13BIT; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_24bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_24BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_24BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_27bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_27BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_27BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_40bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_40BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_40BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_64bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_64BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_64BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_41bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_41BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_41BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_60bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_60BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_60BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];

	return ret;
}

static int ecc_80bit1k_gen(unsigned char *data, unsigned int len, unsigned char *ecc_code)
{
	int i, ret;
	ecc_data_gen(data, len);
	ret = ecc_parity_gen(data, len * ECC_LEN_MULTY_8, ECC_LEVLE_80BIT1K, ecc_code);
	for (i = 0; i < ECC_LEN_80BIT1K; i++)
		ecc_code[i] = ~ecc_code[i];
	return ret;
}

static int ecc_gen_8bit1k_2k(unsigned char *buf,
			     unsigned char *pagebuf,
			     struct oobuse_info *info,
			     unsigned char *ecc_buf,
			     unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf, DATA_LEN0_8BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
	/* 1040,1032.etc These means the number of the data for ecc */
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K, DATA_LEN1_8BIT1K_2K,
		     buf + DATA_LEN0_8BIT1K_2K, DATA_LEN1_8BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN,
		     ECC_LEN_8BIT1K, buf + DATA_LEN0_8BIT1K_2K + DATA_LEN1_8BIT1K_2K, DATA_LEN2_8BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN +
		     DATA_LEN2_8BIT1K_2K + ECC_LEN_8BIT1K, CTRL_LEN_8BIT1K_2K, buf + pagesize + BB_LEN,
		     CTRL_LEN_8BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_2K, DATA_LEN0_8BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_8BIT1K_2K + ECC_LEN_8BIT1K + DATA_LEN1_8BIT1K_2K + BB_LEN +
		     DATA_LEN2_8BIT1K_2K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
}

static int ecc_gen_8bit1k_4k(unsigned char *buf,
			     unsigned char *pagebuf,
			     struct oobuse_info *info,
			     unsigned char *ecc_buf,
			     unsigned int pagesize)
{
	unsigned int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;

	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * i, DATA_LEN0_8BIT1K_4K,
			     buf + DATA_LEN0_8BIT1K_4K * i, DATA_LEN0_8BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
			return -1;
		ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_4K * i, DATA_LEN0_8BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * i + DATA_LEN0_8BIT1K_4K,
			     ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_8BIT1K_4K,
		     buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_8BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN,
		     DATA_LEN2_8BIT1K_4K,
		     buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_8BIT1K_4K, DATA_LEN2_8BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN
		     + DATA_LEN2_8BIT1K_4K + ECC_LEN_8BIT1K, CTRL_LEN_8BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_8BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_8BIT1K, 0xFF, ECC_LEN_8BIT1K))
		return -1;
	ecc_4bit_gen(buf + DATA_LEN0_8BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_8BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_8BIT1K_4K + ECC_LEN_8BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_8BIT1K_4K + BB_LEN
		     + DATA_LEN2_8BIT1K_4K, ECC_LEN_8BIT1K, ecc_buf, ECC_LEN_8BIT1K))
		return -1;
}

static int ecc_gen_16bit1k_2k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize +
		     info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf, DATA_LEN0_16BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K, DATA_LEN1_16BIT1K_2K,
		     buf + DATA_LEN0_16BIT1K_2K, DATA_LEN1_16BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN,
		     DATA_LEN2_16BIT1K_2K, buf + DATA_LEN0_16BIT1K_2K + DATA_LEN1_16BIT1K_2K, DATA_LEN2_16BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN +
		     DATA_LEN2_16BIT1K_2K + ECC_LEN_16BIT1K, CTRL_LEN_16BIT1K_2K, buf + pagesize + BB_LEN,
		     CTRL_LEN_16BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_2K, DATA_LEN0_16BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_16BIT1K_2K + ECC_LEN_16BIT1K + DATA_LEN1_16BIT1K_2K + BB_LEN +
		     DATA_LEN2_16BIT1K_2K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
}

static int ecc_gen_16bit1k_4k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * i, DATA_LEN0_16BIT1K_4K,
			     buf + DATA_LEN0_16BIT1K_4K * i, DATA_LEN0_16BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
			return -1;
		ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_4K * i, DATA_LEN0_16BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * i + DATA_LEN0_16BIT1K_4K,
			     ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_16BIT1K_4K,
		     buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_16BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN, DATA_LEN2_16BIT1K_4K,
		     buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_16BIT1K_4K, DATA_LEN2_16BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN + DATA_LEN2_16BIT1K_4K + ECC_LEN_16BIT1K, CTRL_LEN_16BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_16BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_16BIT1K, 0xFF, ECC_LEN_16BIT1K))
		return -1;
	ecc_8bit_gen(buf + DATA_LEN0_16BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_16BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_16BIT1K_4K + ECC_LEN_16BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_16BIT1K_4K +
		     BB_LEN + DATA_LEN2_16BIT1K_4K, ECC_LEN_16BIT1K, ecc_buf, ECC_LEN_16BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_2k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf, DATA_LEN0_24BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K, DATA_LEN1_24BIT1K_2K,
		     buf + DATA_LEN0_24BIT1K_2K, DATA_LEN1_24BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN,
		     DATA_LEN2_24BIT1K_2K, buf + DATA_LEN0_24BIT1K_2K + DATA_LEN1_24BIT1K_2K, DATA_LEN2_24BIT1K_2K))
		return -1;
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN +
		     DATA_LEN2_24BIT1K_2K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_2K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_2K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_2K, DATA_LEN0_24BIT1K_2K, ecc_buf);
	if (memcpy_s(pagebuf + DATA_LEN0_24BIT1K_2K + ECC_LEN_24BIT1K + DATA_LEN1_24BIT1K_2K + BB_LEN +
		     DATA_LEN2_24BIT1K_2K, DATA_LEN2_24BIT1K_2K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_4k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_4K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * i, DATA_LEN0_24BIT1K_4K,
			     buf + DATA_LEN0_24BIT1K_4K * i, DATA_LEN0_24BIT1K_4K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
			return -1;
		ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_4K * i, DATA_LEN0_24BIT1K_4K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * i + DATA_LEN0_24BIT1K_4K,
			     ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K,
		     DATA_LEN1_24BIT1K_4K,
		     buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN1_24BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN, DATA_LEN2_24BIT1K_4K,
		     buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K + DATA_LEN1_24BIT1K_4K, DATA_LEN2_24BIT1K_4K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN + DATA_LEN2_24BIT1K_4K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_4K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_4K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_4K * TOTAL_NUMS_4K, DATA_LEN0_24BIT1K_4K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_4K + ECC_LEN_24BIT1K) * TOTAL_NUMS_4K +
		     DATA_LEN1_24BIT1K_4K +
		     BB_LEN + DATA_LEN2_24BIT1K_4K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_24bit1k_8k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * i, DATA_LEN0_24BIT1K_8K,
			     buf + DATA_LEN0_24BIT1K_8K * i, DATA_LEN0_24BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
			return -1;
		ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_8K * i, DATA_LEN0_24BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * i + DATA_LEN0_24BIT1K_8K,
			     ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_24BIT1K_8K,
		     buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN1_24BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN, DATA_LEN2_24BIT1K_8K,
		     buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_24BIT1K_8K, DATA_LEN2_24BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN + DATA_LEN2_24BIT1K_8K + ECC_LEN_24BIT1K, CTRL_LEN_24BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_24BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_24BIT1K, 0xFF, ECC_LEN_24BIT1K))
		return -1;
	ecc_24bit1k_gen(buf + DATA_LEN0_24BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_24BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_24BIT1K_8K + ECC_LEN_24BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_24BIT1K_8K +
		     BB_LEN + DATA_LEN2_24BIT1K_8K, ECC_LEN_24BIT1K, ecc_buf, ECC_LEN_24BIT1K))
		return -1;
}

static int ecc_gen_40bit1k_8k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * i, DATA_LEN0_40BIT1K_8K,
			     buf + DATA_LEN0_40BIT1K_8K * i, DATA_LEN0_40BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
			return -1;
		ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_8K * i, DATA_LEN0_40BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * i + DATA_LEN0_40BIT1K_8K,
			     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
			return -1;
	}
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_40BIT1K_8K,
		     buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN1_40BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN, DATA_LEN2_40BIT1K_8K,
		     buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_40BIT1K_8K, DATA_LEN2_40BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN + DATA_LEN2_40BIT1K_8K + ECC_LEN_40BIT1K, CTRL_LEN_40BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_40BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_40BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_8K + ECC_LEN_40BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_40BIT1K_8K +
		     BB_LEN + DATA_LEN2_40BIT1K_8K, ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
		return -1;
}

static int ecc_gen_40bit1k_16k(unsigned char *buf,
			       unsigned char *pagebuf,
			       struct oobuse_info *info,
			       unsigned char *ecc_buf,
			       unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_16K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * i, DATA_LEN0_40BIT1K_16K,
			     buf + DATA_LEN0_40BIT1K_16K * i, DATA_LEN0_40BIT1K_16K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
			return -1;
		ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * i, DATA_LEN0_40BIT1K_16K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * i + DATA_LEN0_40BIT1K_16K,
			     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K,
		     DATA_LEN0_40BIT1K_16K,
		     buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     ECC_LEN0_40BIT1K_16K, BB_LEN,
		     buf + pagesize, BB_LEN))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_40BIT1K_16K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K,
		     ECC_LEN0_40BIT1K_16K, ecc_buf, ECC_LEN0_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K, ECC_LEN1_40BIT1K_16K,
		     ecc_buf + TOTAL_NUMS_16K, ECC_LEN1_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K, DATA_LEN1_40BIT1K_16K,
		     buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_40BIT1K_16K, DATA_LEN1_40BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K + DATA_LEN1_40BIT1K_16K + ECC_LEN_40BIT1K,
		     CTRL_LEN_40BIT1K_16K, buf + pagesize + BB_LEN, CTRL_LEN_40BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_40BIT1K, 0xFF, ECC_LEN_40BIT1K))
		return -1;
	ecc_40bit1k_gen(buf + DATA_LEN0_40BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_40BIT1K_16K,
			DATA_LEN0_40BIT1K_16K,
			ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_40BIT1K_16K + ECC_LEN_40BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN0_40BIT1K_16K +
		     BB_LEN + ECC_LEN0_40BIT1K_16K + ECC_LEN1_40BIT1K_16K + DATA_LEN1_40BIT1K_16K,
		     ECC_LEN_40BIT1K, ecc_buf, ECC_LEN_40BIT1K))
		return -1;
}

static int ecc_gen_64bit1k_8k(unsigned char *buf,
			      unsigned char *pagebuf,
			      struct oobuse_info *info,
			      unsigned char *ecc_buf,
			      unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_8K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * i, DATA_LEN0_64BIT1K_8K,
			     buf + DATA_LEN0_64BIT1K_8K * i, DATA_LEN0_64BIT1K_8K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
			return -1;
		ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_8K * i, DATA_LEN0_64BIT1K_8K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * i + DATA_LEN0_64BIT1K_8K,
			     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K,
		     DATA_LEN1_64BIT1K_8K,
		     buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K, ECC_LEN_64BIT1K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN, DATA_LEN2_64BIT1K_8K,
		     buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K + DATA_LEN1_64BIT1K_8K, DATA_LEN2_64BIT1K_8K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN + DATA_LEN2_64BIT1K_8K + ECC_LEN_64BIT1K, CTRL_LEN_64BIT1K_8K,
		     buf + pagesize + BB_LEN, CTRL_LEN_64BIT1K_8K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_8K * TOTAL_NUMS_8K, DATA_LEN0_64BIT1K_8K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_8K + ECC_LEN_64BIT1K) * TOTAL_NUMS_8K +
		     DATA_LEN1_64BIT1K_8K +
		     BB_LEN + DATA_LEN2_64BIT1K_8K, ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;
}

static int ecc_gen_64bit1k_16k(unsigned char *buf,
			       unsigned char *pagebuf,
			       struct oobuse_info *info,
			       unsigned char *ecc_buf,
			       unsigned int pagesize)
{
	int i;

	if (memcpy_s(buf, MAX_PAGE_SIZE + MAX_OOB_SIZE, pagebuf, pagesize + info->oobuse))
		return -1;
	for (i = 0; i < TOTAL_NUMS_16K; i++) {
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * i, DATA_LEN0_64BIT1K_16K,
			     buf + DATA_LEN0_64BIT1K_16K * i, DATA_LEN0_64BIT1K_16K))
			return -1;
		if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
			return -1;
		ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * i, DATA_LEN0_64BIT1K_16K, ecc_buf);
		if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * i + DATA_LEN0_64BIT1K_16K,
			     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
			return -1;
	}

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K,
		     DATA_LEN1_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN1_64BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K,
		     BB_LEN, buf + pagesize, BB_LEN))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN, DATA_LEN2_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN1_64BIT1K_16K, DATA_LEN2_64BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K, DATA_LEN0_64BIT1K_16K, ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K, ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;

	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K, DATA_LEN3_64BIT1K_16K,
		     buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN1_64BIT1K_16K + DATA_LEN2_64BIT1K_16K,
		     DATA_LEN3_64BIT1K_16K))
		return -1;
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K + DATA_LEN3_64BIT1K_16K + ECC_LEN_64BIT1K,
		     CTRL_LEN_64BIT1K_16K, buf + pagesize + BB_LEN, CTRL_LEN_64BIT1K_16K))
		return -1;
	if (memset_s(ecc_buf, ECC_LEN_64BIT1K, 0xFF, ECC_LEN_64BIT1K))
		return -1;
	ecc_64bit1k_gen(buf + DATA_LEN0_64BIT1K_16K * TOTAL_NUMS_16K + DATA_LEN0_64BIT1K_16K,
			DATA_LEN0_64BIT1K_16K,
			ecc_buf);
	if (memcpy_s(pagebuf + (DATA_LEN0_64BIT1K_16K + ECC_LEN_64BIT1K) * TOTAL_NUMS_16K +
		     DATA_LEN1_64BIT1K_16K +
		     BB_LEN + DATA_LEN2_64BIT1K_16K + ECC_LEN_64BIT1K + DATA_LEN3_64BIT1K_16K,
		     ECC_LEN_64BIT1K, ecc_buf, ECC_LEN_64BIT1K))
		return -1;
}

static int ecc_gen_8bit1k(unsigned char *buf,
			  unsigned char *pagebuf,
			  struct oobuse_info *info,
			  enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_8BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_8bit1k_2k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_8bit1k_4k(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_16bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_16BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_16bit1k_2k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_16bit1k_4k(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_24bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_24BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_2K)
		ret = ecc_gen_24bit1k_2k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_4K)
		ret = ecc_gen_24bit1k_4k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_24bit1k_8k(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

static int ecc_gen_40bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_40BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_40bit1k_8k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_16K)
		ret = ecc_gen_40bit1k_16k(buf, pagebuf, info, ecc_buf, pagesize);
	return ret;
}

static int ecc_gen_64bit1k(unsigned char *buf,
			   unsigned char *pagebuf,
			   struct oobuse_info *info,
			   enum page_type pagetype)
{
	int ret = 0;
	unsigned char ecc_buf[ECC_LEN_64BIT1K];
	unsigned int pagesize;

	pagesize = get_pagesize(pagetype);

	if (pagetype == PT_PAGESIZE_8K)
		ret = ecc_gen_64bit1k_8k(buf, pagebuf, info, ecc_buf, pagesize);
	else if (pagetype == PT_PAGESIZE_16K)
		ret = ecc_gen_64bit1k_16k(buf, pagebuf, info, ecc_buf, pagesize);

	return ret;
}

int page_ecc_gen(unsigned char *pagebuf, enum page_type pagetype, enum ecc_type ecctype)
{
	unsigned char *buf = NULL;
	struct oobuse_info *info;
	int ret = 0;
	info = get_oobuse_info(pagetype, ecctype);
	if (info == NULL || pagebuf == NULL)
		return -1;
	buf = (unsigned char *)malloc(sizeof(unsigned char) * (MAX_PAGE_SIZE + MAX_OOB_SIZE));
	if (buf == NULL)
		return -1;
	switch (ecctype) {
	case ET_ECC_8BIT1K:
		ret = ecc_gen_8bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_16BIT1K:
		ret = ecc_gen_16bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_24BIT1K:
		ret = ecc_gen_24bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_40BIT1K:
		ret = ecc_gen_40bit1k(buf, pagebuf, info, pagetype);
		break;
	case ET_ECC_64BIT1K:
		ret = ecc_gen_64bit1k(buf, pagebuf, info, pagetype);
		break;
	default:
		printf("%s not support ecctype 0x%08x:\n", __FUNCTION__, ecctype);
		break;
	}
	free(buf);
	return ret;
}
