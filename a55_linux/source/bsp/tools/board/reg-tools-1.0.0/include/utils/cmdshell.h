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

#ifndef CMDSHELL_H__
#define CMDSHELL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ERR_CMD_SHELL_NULL_CMD 2
#define ERR_CMD_SHELL_NULL_FP  3

typedef int(*cmd_func)(int, char **);

#define CMD_ENABLE  1
#define CMD_DISABLE 0

typedef struct cmdshell_stru {
	char        *cmdstr;
	unsigned int is_enable;
	cmd_func     p_func;
	char        *helpstr;
} cmd_shell_t;

int cmd_shell_run(int argc, char *argv[], const cmd_shell_t *p_cmds);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CMDSHELL_H__ */

