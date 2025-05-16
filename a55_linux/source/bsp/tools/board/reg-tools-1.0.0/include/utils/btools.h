/*
 *
 * Copyright (c) 2012-2021 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef BTOOLS_H
#define BTOOLS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int bspmd_l(int argc, char *argv[]);

int bspmd(int argc, char *argv[]);

int bspmm(int argc, char *argv[]);

int bspddrs(int argc, char *argv[]);

int i2c_read(int argc, char *argv[]);

int i2c_write(int argc, char *argv[]);

int ssp_read(int argc, char *argv[]);

int ssp_write(int argc, char *argv[]);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* BTOOLS_H */
