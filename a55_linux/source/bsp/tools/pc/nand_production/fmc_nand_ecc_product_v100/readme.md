# Compile the nand_product tool.
	The compilation of the nand_product tool depends on the libsecurec-pc.a security function library. Therefore, you need to compile the libsecurec-pc.a static library before compiling the nand_product tool. For details, see the following steps:

	a. Go to the bsp/componment directory and decompress the secure function library.
		cd bsp/componment/secure_c/src

	b. Compile the secure function library and copy it to the nand_production/fmc_nand_ecc_product_v100 directory.

		CC=gcc make lib CHECK_OPTION=check
		cp ../lib/libsecurec.a ../../../tools/pc/nand_production/fmc_nand_ecc_product_v100/libsecurec-pc.a

	c. Compile the nand_product tool.
		cd ../../../tools/pc/nand_production/fmc_nand_ecc_product_v100
		make

# Usage of nand_product
	When the nand pagesize less than 8K, the Randomizer select 0, others select 1;
When make the uboot image, the Save Pin Mode select 1, ECC type 16bit/1K select 0, and others select 0;

Usage:
./nand_product      inputfile       outputfile      pagetype        ecctype oobsize yaffs   randomizer      pagenum/block   save_pin:
For example:             |               |               |               |       |       |       |               |               |
./nand_product      in_2k4b.yaffs   out_2k4b.yaffs   0               1       64      1       0               64              0
Page type Page size:
Input file:
Output file:
Pagetype:
0        2KB
1        4KB
ECC type ECC size:
1        4bit/512B
2        16bit/1K
3        24bit/1K
Chip OOB size:
yaffs2 image format:
0        NO
1        YES
Randomizer:
0        randomizer_disabled
1        randomizer_enabled
Pages_per_block:
64       64pages/block
128      128pages/block
Save Pin Mode:
0        disable
1        enable
