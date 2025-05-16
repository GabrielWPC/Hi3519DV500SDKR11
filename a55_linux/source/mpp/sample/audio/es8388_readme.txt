This document applies only to socket boards. The usage method is as follows:
1. switch to i2s route
	You need to manually configure the DIP switches SW4200 and SW9300:
	1) SW4200 bit[1] set to 1
	2) SW4200 bit[3] set to 1
	3) SW9300 bit[1] set to 0
	4) SW9300 bit[2] set to 0

2. enable I2S0
	The data transmission interface of the ES8388 is I2S0, it need to enable I2S0. As follows：
	2.1 update the file: smp/a55_linux/source/interdrv/sysconfig/pin_mux.h
		#define I2S0_EN 1

	2.2 enter the directory and compile:
		cd smp/a55_linux/source/interdrv;
		make clean; make

3. On the socket board, you need to transfer the board parameter when running the load3519dv500 script. As follows：
    ./load3519dv500 -board sck

4. insmod ES8388's driver on board.
	cd smp/a55_linux/source/out/ko;
	chmod +x extdrv/ot_es8388.ko;
	insmod extdrv/ot_es8388.ko

5. connect external codec ES8388.
	5.1 set ACODEC_TYPE to ACODEC_TYPE_ES8388, update the file: smp/a55_linux/source/mpp/sample/Makefile.param
		ACODEC_TYPE ?= ACODEC_TYPE_ES8388
		#ACODEC_TYPE ?= ACODEC_TYPE_INNER

	5.2 enter the directory and compile:
		cd smp/a55_linux/source/mpp/sample/audio;
		make clean; make

6. On the socket board, you can't use both the dmic and the i2s at the same time.