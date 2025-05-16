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

#ifndef ARGPARSER_H__
#define ARGPARSER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ERR_ARG_WRONG_FMT         2   /* format error */
#define ERR_ARG_UNKOWN_ARG        3 /* Undefined arg */
#define ERR_ARG_NOT_DEFINE_NO_OPT 4/* Undefined NO_OPT Type */
#define ERR_ARG_MULTI_NO_OPT      5/* Undefined NO_OPT Type */

enum ARG_TYPE {
	ARG_TYPE_NO_OPT   =  0, /* lack of opt, only single one arg */
	ARG_TYPE_CHAR     =  1, /* char */
	ARG_TYPE_STRING   =  2, /* string */
	ARG_TYPE_INT      =  3, /* integer */
	ARG_TYPE_FLOAT    =  4, /* float */
	ARG_TYPE_SINGLE   =  5, /* single, no parm */
	ARG_TYPE_HELP     =  6,
	ARG_TYPE_END
};

typedef struct arg_opt_stru {
	char *sz_opt;
	char type;
	char is_set; /* 1: be set, 0 not set */
	char resv;
	char *p_help_msg;
	void *p_value;
} arg_opt_t;

extern int arg_parser(int argc, char **argv, arg_opt_t *opts);
extern void arg_print_help(const arg_opt_t *opts);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* ARGPARSER_H__ */
