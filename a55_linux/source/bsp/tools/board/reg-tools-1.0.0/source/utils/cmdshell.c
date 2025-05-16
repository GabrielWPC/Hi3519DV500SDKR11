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

#include <stdio.h>
#include "bsp_config.h"
#include "bsp_type.h"
#include "bsp_dbg.h"
#include "securec.h"
#include "cmdshell.h"

#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cpluscplus */

static const cmd_shell_t *cmd_shell_search(const char *cmdstr, const cmd_shell_t *p_cmds)
{
	if (p_cmds == NULL || cmdstr == NULL)
		return NULL;

	size_t len_cmd_str = strlen(cmdstr);
	if (len_cmd_str == 0) {
		write_log_debug1("cmdstr is zero length string \n");
		return NULL;
	}

	/**/
	int i = 0;
	while (p_cmds[i].cmdstr) {
		if (strcmp(p_cmds[i].cmdstr, cmdstr) == 0)
			return (&p_cmds[i]);
		i++;
	}
	write_log_debug1("cann't find cmd: %s \n", cmdstr);
	return NULL;
}

int cmd_shell_run(int argc, char *argv[], const cmd_shell_t *p_cmds)
{
	const cmd_shell_t *p_cmd = NULL;
	char *p_cmd_str = NULL;
	char *p_tmp = NULL;

	if (argv == NULL || p_cmds == NULL)
		return TD_FAILURE;

	p_cmd_str = argv[0];

	for (p_tmp = p_cmd_str; *p_tmp != '\0';) {
		if (*p_tmp++ == '/')
			p_cmd_str = p_tmp;
	}
	write_log_debug("cmdstr:%s\n", p_cmd_str);

	p_cmd = cmd_shell_search(p_cmd_str, (cmd_shell_t *)p_cmds);
	if (p_cmd == NULL) {
		write_log_debug("cann't find cmdstr:%s\n", argv[0]);
		return TD_FAILURE;
	} else {
		if (p_cmd->p_func == NULL)
			return ERR_CMD_SHELL_NULL_FP;
		return ((int)p_cmd->p_func(argc, argv));
	}

	return TD_SUCCESS;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
