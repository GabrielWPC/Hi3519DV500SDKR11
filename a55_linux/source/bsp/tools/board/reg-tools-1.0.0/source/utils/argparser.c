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

#include "securec.h"
#include "bsp_config.h"
#include "bsp_dbg.h"
#include "bsp_type.h"
#include "argparser.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cpluscplus */


char arg_type_str[ARG_TYPE_END + 1][8] = {
	{"NO_OPT"},
	{"CHAR"},
	{"STRING"},
	{"INT"},
	{"FLOAT"},
	{"SINGLE"},
	{"HELP"},
	{"END"}
};

static int arg_set_value(arg_opt_t *p_opt, const char *sz_argv, int num)
{
	if (p_opt == NULL || p_opt->p_value == NULL || sz_argv == NULL)
		return TD_FAILURE;

	switch (p_opt->type) {
	case ARG_TYPE_INT:
		*((int *)p_opt->p_value) = atoi(sz_argv);
		p_opt->is_set = TRUE;
		break;
	case ARG_TYPE_FLOAT:
		*((float *)p_opt->p_value) = (float)atof(sz_argv);
		p_opt->is_set = TRUE;
		break;
	case ARG_TYPE_CHAR:
		*((char *)p_opt->p_value) = sz_argv[0];
		p_opt->is_set = TRUE;
		break;
	case ARG_TYPE_NO_OPT:
	case ARG_TYPE_STRING:
		if (strcpy_s((char *)p_opt->p_value, num, sz_argv) == EOK) {
			p_opt->is_set = TRUE;
		} else {
			write_log_error("error sz_argv.\n");
			return ERR_ARG_UNKOWN_ARG;
		}

		break;
	case ARG_TYPE_END:
		p_opt->is_set = FALSE;
		break;
	default:
		p_opt->is_set = FALSE;
		write_log_error("error opt type: %u\n", p_opt->type);
		return ERR_ARG_UNKOWN_ARG;
	}
	return TD_SUCCESS;
}

static arg_opt_t *arg_find_opt_str(arg_opt_t *opts, const char *str_opt)
{
	int i = 0;
	while (opts[i].type < ARG_TYPE_END) {
		if (strcmp(opts[i].sz_opt, str_opt) == 0)
			return &opts[i];
		i++;
	}
	return NULL;
}

static arg_opt_t *arg_find_opt_no_arg(arg_opt_t *opts)
{
	int i = 0;
	while (opts[i].type < ARG_TYPE_END) {
		if (opts[i].type == ARG_TYPE_NO_OPT)
			return &opts[i];
		i++;
	}
	return NULL;
}

int arg_parser(int argc, char **argv, arg_opt_t *opts)
{
	int ret;
	int i;
	char *p_curr_arg = NULL;
	arg_opt_t *p_curr_opt = NULL;

	unsigned char b_has_parse_arg = FALSE;
	int   i_no_opt_cnt = 0;

	if (argv == NULL || opts == NULL)
		return TD_FAILURE;

	if (argc == 1)
		return TD_SUCCESS;

	// argv[0] is execute command
	for (i = 1; i < argc; i++) {
		p_curr_arg = argv[i];

		if (*p_curr_arg == '/' || *p_curr_arg == '-') {
			p_curr_arg++;

			/* the argv[n] as '-' */
			if (*p_curr_arg == 0) {
				write_log_debug("Wrong Args x: %s\n", argv[i]);
				return ERR_ARG_WRONG_FMT;
			}
			p_curr_opt = arg_find_opt_str(opts, p_curr_arg);
			if (p_curr_opt == NULL) {
				write_log_debug("Wrong Args y: %s\n", argv[i]);
				return ERR_ARG_WRONG_FMT;
			}
			if (p_curr_opt->type == ARG_TYPE_SINGLE) {
				b_has_parse_arg = FALSE;
				p_curr_opt->is_set = TRUE;
			} else {
				b_has_parse_arg = TRUE;
			}
			continue;
		} else if (b_has_parse_arg == FALSE) {
			p_curr_opt = arg_find_opt_no_arg(opts);
			if (p_curr_opt == NULL) {
				write_log_debug("Wrong Args z: %s\n", argv[i]);
				return ERR_ARG_NOT_DEFINE_NO_OPT;
			}
			b_has_parse_arg = TRUE;
			i_no_opt_cnt++;
			if (i_no_opt_cnt > 1)
				return ERR_ARG_MULTI_NO_OPT;
		}

		if (b_has_parse_arg == TRUE) {
			ret = arg_set_value(p_curr_opt, argv[i], i);
			if (ret != TD_SUCCESS)
				return ret;
			b_has_parse_arg = FALSE;
		}
	}
	return TD_SUCCESS;
}

/* return opts[] */
void arg_print_help(const arg_opt_t *opts)
{
	int i = 0;
	if (opts == NULL)
		return;
	while (opts[i].type < ARG_TYPE_END) {
		write_log_normal("-%s\n%s", opts[i].sz_opt, opts[i].p_help_msg);
		(void)fflush(stdout);
		i++;
	}
}

#ifdef __cplusplus
}
#endif /* __cpluscplus */

