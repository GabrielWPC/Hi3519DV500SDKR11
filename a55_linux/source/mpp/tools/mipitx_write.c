/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "securec.h"

#include "ot_common.h"
#include "ot_mipi_tx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#ifndef __LITEOS__
#define MIPI_TX_WRITE_PROGRAM_NAME "./mipitx_write"
#else
#define MIPI_TX_WRITE_PROGRAM_NAME "mipitx_write"
#endif

#define MIPI_TX_WRITE_USAGE_STR \
    "\nUsage: "MIPI_TX_WRITE_PROGRAM_NAME" [devno] [work_mode] [lp_clk_en] [datatype] [paramnum] " \
        "[param0][param1]...[paramn]\n" \
    "datatype : DCS Data Type, such as 0x05--DSC write, 0x23--generic short write, 0x29--generic long write\n" \
    "           to use 0x05 or others to write, please refer to screen specification.\n" \
    "paramnum : the total num of param.(unit:byte)\n"

#define MIPI_TX_WRITE_USAGE_EG_STR \
    "   e.g : "MIPI_TX_WRITE_PROGRAM_NAME" 0 0 1 0x05 0x1 0x11\n" \
    "   e.g : "MIPI_TX_WRITE_PROGRAM_NAME" 0 0 1 0x23 0x2 0x12 0x34\n" \
    "   e.g : "MIPI_TX_WRITE_PROGRAM_NAME" 0 0 1 0x29 0x3 0x12 0x34 0x56\n"

#define MIPI_TX_WRITE_TIPS \
    "\tTo see more usage, please enter: "MIPI_TX_WRITE_PROGRAM_NAME" -h\n\n"

static void usage(void)
{
    printf(MIPI_TX_WRITE_USAGE_STR \
        MIPI_TX_WRITE_USAGE_EG_STR);
}

#define INPUT_PARAM_NUM_BEFORE_SEND_DATA 6
#define MAX_SEND_DATA_NUM 28
#define MIPI_TX_DEV_NAME "/dev/ot_mipi_tx"

static td_s32 check_param_num(int argc, char *argv[])
{
    if (argc == 1) {
        printf(MIPI_TX_WRITE_TIPS);
        return TD_FAILURE;
    }

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 chars */
            usage();
            return TD_FAILURE;
        }
    }

    if (argc < INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("\t input not enough,use "MIPI_TX_WRITE_PROGRAM_NAME" -h to get help\n\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

#define mipitx_write_parse_check_return(format, info...) \
    do { \
        if (errno != 0) { \
            printf("parse "format" failed.\n", ##info); \
            return TD_FAILURE; \
        } \
    } while (0)

static td_s32 mipitx_write_parse(int argc, char *argv[], cmd_info_t *cmd_info, td_s16 *send_param_num)
{
    cmd_info->devno = (td_u32)strtol(argv[1], NULL, 0); /* 1st arg */
    mipitx_write_parse_check_return("devno");

    cmd_info->work_mode = (td_u32)strtol(argv[2], NULL, 0); /* 2nd arg */
    mipitx_write_parse_check_return("work_mode");

    cmd_info->lp_clk_en = (td_u8)strtol(argv[3], NULL, 0); /* 3rd arg */
    mipitx_write_parse_check_return("lp_clk_en");

    cmd_info->data_type = (td_u16)strtol(argv[4], NULL, 0); /* 4th arg */
    mipitx_write_parse_check_return("data_type");

    *send_param_num = (td_s16)strtol(argv[5], NULL, 0); /* 5th arg */
    mipitx_write_parse_check_return("send_param_num");

    return TD_SUCCESS;
}

static td_s32 mipitx_write_check_arg(int argc, const cmd_info_t *cmd_info, td_s16 send_param_num)
{
    if (cmd_info->devno != 0) {
        printf("devno(%u) input err\n", cmd_info->devno);
        return TD_FAILURE;
    }

    if ((cmd_info->work_mode != MIPI_TX_WORK_MODE_LP) &&
        (cmd_info->work_mode != MIPI_TX_WORK_MODE_HS)) {
        printf("work_mode(%u) input err\n", cmd_info->work_mode);
        return TD_FAILURE;
    }

    if ((cmd_info->lp_clk_en != 0) && (cmd_info->lp_clk_en != 1)) {
        printf("lp_clk_en(%u) input err\n", cmd_info->lp_clk_en);
        return TD_FAILURE;
    }

    if (cmd_info->data_type <= 0) {
        printf("data_type(%u) input err\n", cmd_info->data_type);
        return TD_FAILURE;
    }

    if ((send_param_num < 0) || (send_param_num > MAX_SEND_DATA_NUM)) {
        printf(MIPI_TX_WRITE_PROGRAM_NAME" only support send_param_num range [0, %d]\n", MAX_SEND_DATA_NUM);
        return TD_FAILURE;
    }

    if (argc < send_param_num + INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("please input enough param\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static unsigned char g_send_data[MAX_SEND_DATA_NUM];
static td_void mipitx_write_init_write_data(td_void)
{
    (td_void)memset_s(g_send_data, sizeof(g_send_data), 0x0, sizeof(g_send_data));
}

static td_s32 mipitx_write_set_cmd(int argc, char *argv[], cmd_info_t *cmd_info, td_s16 send_param_num)
{
    td_s32 i;
    td_ulong low_byte, high_byte;

    mipitx_write_init_write_data();

    if (send_param_num <= 2) {  /* 2 param */
        if (send_param_num == 0) {
            cmd_info->cmd_size = 0;
        }

        if (send_param_num == 1) {
            cmd_info->cmd_size = (td_s16)strtol(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
            mipitx_write_parse_check_return("1byte cmd_size");
        }

        if (send_param_num == 2) { /* 2 param */
            low_byte = strtoul(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
            mipitx_write_parse_check_return("low_byte cmd_size");
            high_byte = strtoul(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA + 1], NULL, 0) << 8; /* 8bit */
            mipitx_write_parse_check_return("high_byte cmd_size");
            cmd_info->cmd_size = (unsigned short)(low_byte | (high_byte));
        }

        cmd_info->cmd = NULL;
    } else {
        cmd_info->cmd = g_send_data;
        cmd_info->cmd_size = (unsigned short)send_param_num;

        for (i = 0; i < send_param_num; i++) {
            g_send_data[i] = (unsigned char)strtol((argv[i + INPUT_PARAM_NUM_BEFORE_SEND_DATA]), NULL, 0);
            mipitx_write_parse_check_return("send_data[%d]", i);
        }
    }

    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 mipi_tx_write(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    td_s32 fd;
    cmd_info_t cmd_info = {0};
    td_s16 send_param_num;

    send_param_num = -1;
    ret = check_param_num(argc, argv);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    (td_void)memset_s(&cmd_info, sizeof(cmd_info_t), 0x0, sizeof(cmd_info_t));
    ret = mipitx_write_parse(argc, argv, &cmd_info, &send_param_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    ret = mipitx_write_check_arg(argc, &cmd_info, send_param_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    ret = mipitx_write_set_cmd(argc, argv, &cmd_info, send_param_num);
    if (ret != TD_SUCCESS) {
        mipitx_write_init_write_data();
        return TD_FAILURE;
    }

    fd = open(MIPI_TX_DEV_NAME, O_RDONLY);
    if (fd < 0) {
        printf("open %s failed\n", MIPI_TX_DEV_NAME);
        return TD_FAILURE;
    }

    ret = ioctl(fd, OT_MIPI_TX_SET_CMD, &cmd_info);
    if (ret != TD_SUCCESS) {
        printf("MIPI_TX SET CMD failed\n");
        close(fd);
        return TD_FAILURE;
    }
    close(fd);
    printf("write end\n");
    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
