This document applies only to socket boards. The usage method is as follows:
1. switch to dmic route
	You need to manually configure the DIP switches SW4200 and SW9300:
	1) SW4200 bit[1] set to 1
	2) SW4200 bit[3] set to 1
	3) SW9300 bit[1] set to 1
	4) SW9300 bit[2] set to 1

2. connect external dmic board
	On the socket board, you need connect the external dmic board to J15.

3. config pin_mux of dmic
	The pin_mux should be config to the dmic function. As follows：
	3.1 update the file: smp/a55_linux/source/interdrv/sysconfig/pin_mux.h
		#define DMIC_EN 1

	3.2 enter the directory and compile:
		cd smp/a55_linux/source/interdrv;
		make clean; make

4. On the socket board, you can't use both the dmic and the i2s at the same time.