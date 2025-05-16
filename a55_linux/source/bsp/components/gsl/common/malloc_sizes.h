/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
/***********************************************************************************
 * [bootRam Space] [0x04020000 -> 0x04040000 - 1], len: 0x20000(128k).
 * [bss + stack]   [0x04020000 -> 0x04021a00 - 1], len: 0x1a00.
 * [rpk key area + CFCT + gsl key + tp key + gsl info] [0x04021a00 -> 0x04022e00 - 1], len: 0x1400.
 * [gsl code] [0x04022e00 -> 0x04032e00 - 1] max len: 0x10000(64k)
 * [malloc] [0x04033000 -> 0x0403b000 - 1] len: 0x8000
 * [ree key + parm info + patch + parm data] [0x0403b000 -> 0x0403f000 - 1] len: 0x100 + 0x200 + 0x100 + 15k = 0x4000
 * [left len]  0x1000
***********************************************************************************/
/* real  malloc len: 0x6FA0. */
cache(0x50,     2)  	/* usb  */
cache(0x100,    5)      /* hash */
cache(0x200,    4)      /* uart_ctx */
cache(0x300,    1)      /* aes/dma */
cache(0x800,    1)
cache(0xb00,    1)      /* usb device */
cache(0xc00,    1)      /* usb device */
cache(0x4000,   1)  	/* fat32 drv */
/**********************************************************************************/
