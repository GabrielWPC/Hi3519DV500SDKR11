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
#define MIPI_TX_READ_PROGRAM_NAME "./mipitx_read"
#define MIPI_TX_WRITE_PROGRAM_NAME "./mipitx_write"
#else
#define MIPI_TX_READ_PROGRAM_NAME "mipitx_read"
#define MIPI_TX_WRITE_PROGRAM_NAME "mipitx_write"
#endif

#define MIPI_TX_READ_USAGE_STR \
    "\nUsage: "MIPI_TX_READ_PROGRAM_NAME" [devno] [work_mode] [lp_clk_en] [datatype] [readparamnum] [sendparamnum] " \
        "[param0]...[param1]\n" \
    "datatype : DCS Data Type ,such as 0x14--generic read 1 param,0x24--generic read 2 param,\n" \
    "           to use 0x14 or 0x24,please refer to screen specification.\n" \
    "readparamnum: the num of data (byte) you want to get from the screen.\n" \
    "sendparamnum: the num of data (byte),you want to send to the screen.\n"

#define MIPI_TX_READ_USAGE_EG_STR \
    "   e.g : read 1 byte from screen, target register is 0xda\n" \
    "                "MIPI_TX_READ_PROGRAM_NAME" 0 0 1 0x14 0x1 0x1 0xda\n" \
    "   e.g : read 3 bytes from screen, target register is 0xa1, first set max return data num to 3 in step 1,\n" \
    "         then use 0x14 or 0x24(refer to screen) to get data, in step 2\n" \
    "             step 1 "MIPI_TX_WRITE_PROGRAM_NAME" 0 0 1 0x37 0x1 0x3\n" \
    "             step 2 "MIPI_TX_READ_PROGRAM_NAME" 0 0 1 0x14 0x3 0x1 0xa1\n"

static void usage(void)
{
    printf(MIPI_TX_READ_USAGE_STR \
        MIPI_TX_READ_USAGE_EG_STR);
}

#define INPUT_PARAM_NUM_BEFORE_SEND_DATA 7
#define MAX_SEND_DATA_NUM 2
#define MAX_READ_DATA_NUM 100
#define MIPI_TX_DEV_NAME "/dev/ot_mipi_tx"

#define mipitx_read_parse_check_return(format, info...) \
    do { \
        if (errno != 0) { \
            printf("parse "format" failed.\n", ##info); \
            return TD_FAILURE; \
        } \
    } while (0)

static td_s32 check_param_num(int argc, char *argv[])
{
    if (argc == 1) {
        printf("\tTo see more usage, please enter: "MIPI_TX_READ_PROGRAM_NAME" -h\n\n");
        return TD_FAILURE;
    }

    if (argc > 1) {
        if (!strncmp(argv[1], "-h", 2)) { /* 2 chars */
            usage();
            return TD_FAILURE;
        }
    }

    if (argc < INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("\t input not enough,use "MIPI_TX_READ_PROGRAM_NAME" -h to get help\n\n");
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 mipitx_read_parse(int argc, char *argv[], get_cmd_info_t *get_cmd_info,
    td_s16 *read_param_num, td_s32 *send_param_num)
{
    get_cmd_info->devno = (td_u32)strtol(argv[1], NULL, 0); /* 1st arg */
    mipitx_read_parse_check_return("devno");

    get_cmd_info->work_mode = (td_u32)strtol(argv[2], NULL, 0); /* 2nd arg */
    mipitx_read_parse_check_return("work_mode");

    get_cmd_info->lp_clk_en = (td_u8)strtol(argv[3], NULL, 0); /* 3rd arg */
    mipitx_read_parse_check_return("lp_clk_en");

    get_cmd_info->data_type = (td_u16)strtol(argv[4], NULL, 0); /* 4nd arg */
    mipitx_read_parse_check_return("data_type");

    *read_param_num = (td_u16)strtol(argv[5], NULL, 0); /* 5th arg */
    mipitx_read_parse_check_return("read_param_num");

    *send_param_num = (td_s32)strtol(argv[6], NULL, 0); /* 6th arg */
    mipitx_read_parse_check_return("send_param_num");
    return TD_SUCCESS;
}

static td_s32 mipitx_read_check_arg(int argc, const get_cmd_info_t *get_cmd_info, td_s32 send_param_num,
    td_s32 read_param_num)
{
    if (get_cmd_info->devno != 0) {
        printf("devno(%u) input err\n", get_cmd_info->devno);
        return TD_FAILURE;
    }

    if ((get_cmd_info->work_mode != MIPI_TX_WORK_MODE_LP) &&
        (get_cmd_info->work_mode != MIPI_TX_WORK_MODE_HS)) {
        printf("work_mode(%u) input err\n", get_cmd_info->work_mode);
        return TD_FAILURE;
    }

    if ((get_cmd_info->lp_clk_en != 0) && (get_cmd_info->lp_clk_en != 1)) {
        printf("lp_clk_en(%u) input err\n", get_cmd_info->lp_clk_en);
        return TD_FAILURE;
    }

    if (get_cmd_info->data_type <= 0) {
        printf("data_type(%u) input err\n", get_cmd_info->data_type);
        return TD_FAILURE;
    }

    if ((send_param_num < 0) || (send_param_num > MAX_SEND_DATA_NUM)) {
        printf(MIPI_TX_READ_PROGRAM_NAME" only support send_param_num range [0, %d]\n", MAX_SEND_DATA_NUM);
        return TD_FAILURE;
    }

    if (argc < send_param_num + INPUT_PARAM_NUM_BEFORE_SEND_DATA) {
        printf("please input enough param\n");
        return TD_FAILURE;
    }

    if ((read_param_num < 0) || (read_param_num > MAX_READ_DATA_NUM)) {
        printf(MIPI_TX_READ_PROGRAM_NAME" only support read_param_num range [0, %d]\n", MAX_READ_DATA_NUM);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 mipitx_read_set_cmd(char *argv[],
    get_cmd_info_t *get_cmd_info, td_s32 send_param_num)
{
    td_ulong low_byte, high_byte;

    if (send_param_num == 0) {
        get_cmd_info->data_param = 0;
    }

    if (send_param_num == 1) {
        get_cmd_info->data_param = (unsigned short)strtoul(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
        mipitx_read_parse_check_return("1byte send_param_num");
    }

    if (send_param_num == 2) {  /* 2 param */
        low_byte = (td_ulong)strtoul(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA], NULL, 0);
        mipitx_read_parse_check_return("low_byte send_param_num");
        high_byte = (td_ulong)strtoul(argv[INPUT_PARAM_NUM_BEFORE_SEND_DATA + 1], NULL, 0) << 8; /* 8 bit */
        mipitx_read_parse_check_return("high_byte send_param_num");
        get_cmd_info->data_param = (unsigned short)(low_byte | high_byte);
    }
    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 mipi_tx_read(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 ret;
    td_s32 i;
    td_s32 fd;
    unsigned char readdata[MAX_READ_DATA_NUM];
    get_cmd_info_t get_cmd_info = {0};
    td_s32 send_param_num;
    td_s16 read_param_num;
    send_param_num = -1;
    read_param_num = -1;

    ret = check_param_num(argc, argv);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    (td_void)memset_s(readdata, sizeof(readdata), 0x0, sizeof(readdata));
    ret = mipitx_read_parse(argc, argv, &get_cmd_info, &read_param_num, &send_param_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = mipitx_read_check_arg(argc, &get_cmd_info, send_param_num, read_param_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    ret = mipitx_read_set_cmd(argv, &get_cmd_info, send_param_num);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    fd = open(MIPI_TX_DEV_NAME, O_RDONLY);
    if (fd < 0) {
        printf("open %s dev failed\n", MIPI_TX_DEV_NAME);
        return TD_FAILURE;
    }

    get_cmd_info.devno = 0;
    get_cmd_info.get_data = readdata;
    get_cmd_info.get_data_size = (unsigned short)read_param_num;
    ret = ioctl(fd, OT_MIPI_TX_GET_CMD, &get_cmd_info);
    if (ret != TD_SUCCESS) {
        printf("MIPI_TX GET CMD failed, please check, may not use mipitx_set first\n");
        close(fd);
        return TD_FAILURE;
    }

    for (i = 0; i < read_param_num; i++) {
        printf("readdata[%d] = 0x%x\n", i, get_cmd_info.get_data[i]);
    }

    close(fd);
    printf("read end\n");
    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
