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
#include <unistd.h>
#include <string.h>
#include "securec.h"
#include "bsp_type.h"
#include "bsp_config.h"
#include "bsp_dbg.h"
#include "cmdshell.h"
#include "argparser.h"
#include "btools.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_PATH_LEN 1024
#define BTOOLS_NAME "btools"
#define BTOOLS_VERSION "ver0.0.1_20121120"
#define	LOG_MSG_LEN 2048

#define OPT_PRINT_HELP 0
#define OTP_INSTALL_TOOLS 1
#define OTP_UNINSTALL_TOOLS 2
#define OTP_PRINT_VERSION 3

static arg_opt_t btools_opts[] = {
	{
		"h", ARG_TYPE_SINGLE, FALSE, 0,
		"\tprint help msg\n", NULL
	},

	{
		"i", ARG_TYPE_SINGLE, FALSE, 0,
		"\tinstall board tools\n", NULL
	},

	{
		"u", ARG_TYPE_SINGLE, FALSE, 0,
		"\tuninstall board tools\n", NULL
	},

	{
		"V", ARG_TYPE_SINGLE, FALSE, 0,
		"\tprint version\n", NULL
	},
	// end add

	{
		"END", ARG_TYPE_END, FALSE, 0,
		"\tEND\n", NULL
	}
};


static cmd_shell_t g_btools_cmds[] = {
	{ "bspmd",     CMD_ENABLE,  bspmd,     "memory display (8bit)" },
	{ "bspmd.l",   CMD_ENABLE,  bspmd_l,   "memory display (32bit)" },
	{ "bspmm",     CMD_ENABLE,  bspmm,     "memory modify" },
	{ "bspddrs",   CMD_ENABLE,  bspddrs,   "ddr statistic" },
	{ "i2c_read",  CMD_ENABLE,  i2c_read,  "i2c device read" },
	{ "i2c_write", CMD_ENABLE,  i2c_write, "i2c device read" },
	{ "ssp_read",  CMD_ENABLE,  ssp_read,  "ssp device read" },
	{ "ssp_write", CMD_ENABLE,  ssp_write, "ssp device read" },
	{ NULL,        CMD_DISABLE, NULL,       0 },
};

static void print_help_message(void)
{
	int i = 0;
	arg_print_help(btools_opts);

	printf("------------------------------------------------------\n");
	while (g_btools_cmds[i].cmdstr != NULL) {
		if (g_btools_cmds[i].is_enable == CMD_ENABLE)
			printf(" %-10s : %s\n", g_btools_cmds[i].cmdstr, g_btools_cmds[i].helpstr);
		i++;
	}
}


static void print_version_message(void)
{
	printf("*** Board tools : %s *** \n", BTOOLS_VERSION);
}

/* btools install */
static void do_cmd_install(void)
{
	int i = 0;
	while (g_btools_cmds[i].cmdstr) {
		int ret;
		write_log_info("%d:installing <%s>.\n", i, g_btools_cmds[i].cmdstr);
		ret = symlink(BTOOLS_NAME, g_btools_cmds[i].cmdstr);
		if (ret == EEXIST)
			write_log_info(" <%s> exist.\n", g_btools_cmds[i].cmdstr);
		else if (ret != 0)
			perror(" install error!");
		i++;
	}
}

/* btools uninstall */
static void do_cmd_uninstall(void)
{
	char s_real_file[MAX_PATH_LEN];
	int i = 0;
	while (g_btools_cmds[i].cmdstr) {
		int len_fn;
		write_log_info("%d:uninstalling <%s>.\n", i, g_btools_cmds[i].cmdstr);
		if (memset_s(s_real_file, MAX_PATH_LEN, 0, MAX_PATH_LEN) != EOK)
			return;
		len_fn = readlink(g_btools_cmds[i].cmdstr, s_real_file, MAX_PATH_LEN);
		if (len_fn <= 0) {
			perror(" read link error");
			i++;
			continue;
		}
		if (len_fn >= MAX_PATH_LEN) {
			write_log_error(" buffer is too small, %d\n", len_fn);
			i++;
			continue;
		}
		s_real_file[len_fn] = 0;
		if (strcmp(s_real_file, BTOOLS_NAME) != 0) {
			write_log_info(" %s not link to me(%s)\n", s_real_file, BTOOLS_NAME);
			i++;
			continue;
		}
		if (unlink(g_btools_cmds[i].cmdstr))
			perror(" uninstall error");
		i++;
	}
}

static int run_btools(int argc, char *argv[])
{
	if (arg_parser(argc, argv, btools_opts) != TD_SUCCESS) {
		print_help_message();
		return TD_SUCCESS;
	}

	if (btools_opts[OPT_PRINT_HELP].is_set)
		print_help_message();
	else if (btools_opts[OTP_INSTALL_TOOLS].is_set)
		do_cmd_install();
	else if (btools_opts[OTP_UNINSTALL_TOOLS].is_set)
		do_cmd_uninstall();
	else if (btools_opts[OTP_PRINT_VERSION].is_set)
		print_version_message();
	else
		print_help_message();

	return TD_SUCCESS;
}

int main(int argc, char *argv[])
{
	int bsp_do;
	char *p_cmd_str = argv[0];
	char *p_tmp = NULL;

	if (log_create(LOG_LEVEL_DEBUG, LOG_MSG_LEN) != TD_SUCCESS) {
		printf("create logqueue error.\n");
		return -1;
	}

	for (p_tmp = p_cmd_str; *p_tmp != '\0';) {
		if (*p_tmp++ == '/')
			p_cmd_str = p_tmp;
	}

	if (strcmp(p_cmd_str, BTOOLS_NAME) == 0)
		return run_btools(argc, argv);

	print_version_message();
	bsp_do = cmd_shell_run(argc, argv, (cmd_shell_t *)g_btools_cmds);
	if (bsp_do != TD_SUCCESS)
		printf("\ndo errro\n");
	printf("[END]\n");
	fflush(stdout);
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

