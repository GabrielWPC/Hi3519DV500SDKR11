/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"

#define SAMPLE_VO_VB_NUM 8
#define SAMPLE_VO_DEV_FRAME_RATE 60
static volatile sig_atomic_t g_vo_sig_flag = 0;

#define USLEEP_6000   6000
#define USLEEP_50     50
#define USLEEP_100000 100000
#define USLEEP_60000  60000
#define CMD_COUNT_1200X1920 304
static mipi_tx_cmd_info g_cmd_info_1200x1920[CMD_COUNT_1200X1920] = {
    /* {devno work_mode lp_clk_en data_type cmd_size cmd}, usleep_value */
    {{0, 0, 0, 0x23, 0x00B0, NULL}, USLEEP_6000},
    {{0, 0, 0, 0x23, 0x05B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE5B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x52B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x05C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x85D9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x55C2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x88B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0BB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x8BBA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1ABF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0FC0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0CC2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x02C3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0CC4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x02C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x01B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26E0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26E1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00DC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00DD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26CC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26CD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x03D2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x03D3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04E6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04E7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x09C4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x09C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0AD8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0AD9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0BC2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0BC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0CD6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0CD7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x05C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x05C1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x06D4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x06D5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08DE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08DF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x02B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0FC1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1AC2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2BC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38C4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x39C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38C7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x36C8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x34C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x35CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x36CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x39CC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2DCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2DCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2CCF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07D0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00D2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0FD3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1AD4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2BD5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38D6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x39D7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38D8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x38D9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x36DA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x34DB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x35DC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x36DD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x39DE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2DDF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2DE0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2CE1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07E2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x03B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0BC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00E7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2AC5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x2ADE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x43CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xC0E4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0DE5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x06B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xA5B8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xA5C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0FC7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x32D5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00B8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00BC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x07B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x09B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x13B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x23B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4BB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6DB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD8B9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x17BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93BB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1BBC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1FBD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9BBE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x19BF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x57C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93C1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAEC2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xCAC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD7C4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE5C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF3C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF9C7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x10B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x23B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4BB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6DB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD8B9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x17BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93BB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1CBC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x20BD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9DBE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1CBF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x5BC0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x96C1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xB1C2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xCDC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDAC4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE7C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF4C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFAC7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x09B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0DB3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x22B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4CB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6EB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9DB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDAB9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x19BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9ABB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x25BC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x29BD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xA7BE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26BF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x63C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CC1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xB6C2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD1C3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDCC4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE9C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF5C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFAC7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0AB0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x09B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x13B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x23B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4BB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6DB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD8B9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x17BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93BB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1BBC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1FBD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9BBE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x19BF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x57C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93C1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAEC2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xCAC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD7C4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE5C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF3C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF9C7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0BB0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x10B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x23B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4BB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6DB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD8B9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x17BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x93BB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1CBC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x20BD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9DBE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x1CBF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x5BC0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x96C1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xB1C2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xCDC3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDAC4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE7C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF4C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFAC7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0CB0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0DB3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x22B4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x37B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x4CB6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6EB7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9DB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDAB9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x19BA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9ABB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x25BC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x29BD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xA7BE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x26BF, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x63C0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x9CC1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xB6C2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD1C3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xDCC4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE9C5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF5C6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFAC7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFCC8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00C9, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00CA, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x16CB, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xAFCC, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCD, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFCE, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x00B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x08B3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x04B0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFFB8, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x02B5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x01B6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x6FB1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xC8F0, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xD6F1, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xE0F2, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xF2F3, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFBF4, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFDF5, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFDF6, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0xFEF7, NULL}, USLEEP_50},
    {{0, 0, 0, 0x23, 0x0011, NULL}, USLEEP_100000},
    {{0, 0, 0, 0x23, 0x0029, NULL}, USLEEP_60000},
};

static const sample_vo_cfg g_vo_rgb_8bit_config = {
    .vo_dev = SAMPLE_VO_DEV_UHD,
    .vo_layer = SAMPLE_VO_LAYER_VHD0,
    .vo_intf_type = OT_VO_INTF_RGB_8BIT,
    .intf_sync = OT_VO_OUT_USER,
    .bg_color = COLOR_RGB_BLACK,
    .pix_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    .disp_rect = {0, 0, 320, 240},
    .image_size = {320, 240},
    .vo_part_mode = OT_VO_PARTITION_MODE_SINGLE,
    .dis_buf_len = 3, /* 3: def buf len for single */
    .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
    .vo_mode = VO_MODE_1MUX,
    .compress_mode = OT_COMPRESS_MODE_NONE,

    .sync_info = {0, 1, 1, 240, 21, 6, 320, 71, 110, 1, 240, 14, 9, 10, 2, 0, 1, 0},
    .user_sync = {
        .auto_user_sync_info.pixel_clk = 5400000,
        .clk_reverse_en = TD_TRUE,
        .op_mode = OT_OP_MODE_AUTO,
    },
    .dev_frame_rate = SAMPLE_VO_DEV_FRAME_RATE,
    .vo_sharpen = {
        .enable = TD_TRUE,
        .peak_ratio = 255,
    },
    .hor_split_en = TD_FALSE,
};

/* VO: USER 1200x1920_60, TX: USER 1200x1920 */
static const sample_vo_mipi_tx_cfg g_vo_tx_cfg_1200x1920_user = {
    .vo_config = {
        .vo_dev = SAMPLE_VO_DEV_UHD,
        .vo_layer = SAMPLE_VO_LAYER_VHD0,
        .vo_intf_type = OT_VO_INTF_MIPI,
        .intf_sync = OT_VO_OUT_USER,
        .bg_color = COLOR_RGB_BLACK,
        .pix_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        .disp_rect = {0, 0, 1200, 1920},
        .image_size = {1200, 1920},
        .vo_part_mode = OT_VO_PARTITION_MODE_SINGLE,
        .dis_buf_len = 3, /* 3: def buf len for single */
        .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
        .vo_mode = VO_MODE_1MUX,
        .compress_mode = OT_COMPRESS_MODE_NONE,

        .sync_info = {0, 1, 1, 1920, 12, 15, 1200, 104, 60, 1, 540, 9, 7, 24, 2, 0, 0, 0},
        .user_sync = {
            .manual_user_sync_info.user_sync_attr = {
                .clk_src = OT_VO_CLK_SRC_PLL,
                .vo_pll = { /* if mipitx, set it by pixel clk and div mode */
                    .fb_div = 80, /* 80 fb div */
                    .frac = 0,
                    .ref_div = 1, /* 1 ref div */
                    .post_div1 = 6, /* 6 post div1 */
                    .post_div2 = 2, /* 2 post div2 */
                },
            },
            .manual_user_sync_info.pre_div = 1, /* if mipitx, set it by pixel clk */
            .manual_user_sync_info.dev_div = 1, /* if rgb, set it by serial mode */
            .clk_reverse_en = TD_FALSE,
            .op_mode = OT_OP_MODE_MANUAL,
        },
        .dev_frame_rate = SAMPLE_VO_DEV_FRAME_RATE,
    },
    .tx_config = {
        /* for combo dev config */
        .intf_sync = OT_MIPI_TX_OUT_USER,

        /* for screen cmd */
        .cmd_count = CMD_COUNT_1200X1920,
        .cmd_info = g_cmd_info_1200x1920,

        /* for user sync */
        .combo_dev_cfg = {
            .devno = 0,
            .lane_id = {0, 1, 2, 3},
            .out_mode = OUT_MODE_DSI_VIDEO,
            .out_format = OUT_FORMAT_RGB_24BIT,
            .video_mode =  BURST_MODE,
            .sync_info = {
                .hpw = 24, /* 24 pixel */
                .hbp = 80, /* 80 pixel */
                .hact = 1200, /* 1200 pixel */
                .hfp = 60, /* 60 pixel */
                .vpw = 2, /* 2 line */
                .vbp = 10, /* 10 line */
                .vact = 1920, /* 1920 line */
                .vfp = 15, /* 15 line */
            },
            .phy_data_rate = 999, /* 999 Mbps */
            .pixel_clk = 160000, /* 160000 KHz */
        },
    },
};

/* VO: USER 3840x720_60, TX: USER 3840x720 */
static const sample_vo_mipi_tx_cfg g_vo_tx_cfg_3840x720_user = {
    .vo_config = {
        .vo_dev = SAMPLE_VO_DEV_UHD,
        .vo_layer = SAMPLE_VO_LAYER_VHD0,
        .vo_intf_type = OT_VO_INTF_MIPI,
        .intf_sync = OT_VO_OUT_USER,
        .bg_color = COLOR_RGB_BLACK,
        .pix_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        .disp_rect = {0, 0, 3840, 720},
        .image_size = {3840, 720},
        .vo_part_mode = OT_VO_PARTITION_MODE_SINGLE,
        .dis_buf_len = 3, /* 3: def buf len for single */
        .dst_dynamic_range = OT_DYNAMIC_RANGE_SDR8,
        .vo_mode = VO_MODE_1MUX,
        .compress_mode = OT_COMPRESS_MODE_NONE,

        .sync_info = {0, 1, 1, 720, 11, 5, 3840, 56, 52, 1, 1, 1, 1, 24, 8, 0, 0, 0},
        .user_sync = {
            .auto_user_sync_info.pixel_clk = 174344000,
            .clk_reverse_en = TD_FALSE,
            .op_mode = OT_OP_MODE_AUTO,
        },
        .dev_frame_rate = SAMPLE_VO_DEV_FRAME_RATE,
        .vo_sharpen = {
            .enable = TD_TRUE,
            .peak_ratio = 255,
        },
        .hor_split_en = TD_TRUE,
    },
    .tx_config = {
        /* for combo dev config */
        .intf_sync = OT_MIPI_TX_OUT_USER,

        /* for screen cmd */
        .cmd_count = 0,
        .cmd_info = TD_NULL,

        /* for user sync */
        .combo_dev_cfg = {
            .devno = 0,
            .lane_id = {0, 1, 2, 3},
            .out_mode = OUT_MODE_DSI_VIDEO,
            .out_format = OUT_FORMAT_RGB_24BIT,
            .video_mode =  BURST_MODE,
            .sync_info = {
                .vact = 720, /* 720 line */
                .vbp = 3, /* 3 line */
                .vfp = 5, /* 5 line */
                .hact = 3840, /* 3840 pixel */
                .hbp = 32, /* 32 pixel */
                .hfp = 52, /* 52 pixel */
                .hpw = 24, /* 24 pixel */
                .vpw = 2, /* 2 line */
            },
            .phy_data_rate = 1047, /* 1047 Mbps */
            .pixel_clk = 174344, /* 174344 KHz */
        },
    },
};

static void sample_vo_handle_sig(td_s32 signo)
{
    if ((signo == SIGINT) || (signo == SIGTERM)) {
        g_vo_sig_flag = 1;
    }
}

static td_void sample_vo_do_pause(td_void)
{
    if (g_vo_sig_flag == 1) {
        return;
    }

    sample_pause();

    if (g_vo_sig_flag == 1) {
        return;
    }
}

static td_void sample_vo_get_default_vb_config(ot_size *size, ot_vb_cfg *vb_cfg)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* 128 blks */

    buf_attr.width         = size->width;
    buf_attr.height        = size->height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.video_format  = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt  = 30; /* 30 blk */
}

#ifdef SAMPLE_MEM_SHARE_ENABLE
static td_void sample_init_mem_share(td_void)
{
    td_u32 i;
    ot_vb_common_pools_id pools_id = {0};

    if (ss_mpi_vb_get_common_pool_id(&pools_id) != TD_SUCCESS) {
        sample_print("get common pool_id failed!\n");
        return;
    }
    for (i = 0; i < pools_id.pool_cnt; ++i) {
        ss_mpi_vb_pool_share_all(pools_id.pool[i]);
    }
}
#endif

static td_s32 sample_vo_set_sys_cfg(ot_vi_vpss_mode_type mode_type, ot_vi_aiisp_mode aiisp_mode)
{
    td_s32 ret;
    ot_size size;
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config;

    size.width  = 3840; /* 3840 x 2160 */
    size.height = 2160; /* 3840 x 2160 */
    sample_vo_get_default_vb_config(&size, &vb_cfg);

    supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK;
    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, aiisp_mode);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_init_mem_share();
#endif
    return TD_SUCCESS;
}

#ifdef SAMPLE_MEM_SHARE_ENABLE
static td_void sample_init_vo_mem_share(ot_vo_layer vo_layer)
{
    td_s32 ret;
    ot_video_frame_info frame_info;
    td_s32 milli_sec = 5000;
    ot_sys_mem_info mem_info;

    ret = ss_mpi_vo_get_screen_frame(vo_layer, &frame_info, milli_sec);
    if (ret != TD_SUCCESS) {
        sample_print("layer %d ss_mpi_vo_get_screen_frame fail for %#x!\n", vo_layer, ret);
        return;
    }

    ret = ss_mpi_sys_get_mem_info_by_phys(frame_info.video_frame.phys_addr[0], &mem_info);
    if (ret != TD_SUCCESS) {
        ss_mpi_vo_release_screen_frame(vo_layer, &frame_info);
        sample_print("layer %d ss_mpi_sys_get_mem_info_by_phys fail for %#x!\n", vo_layer, ret);
        return;
    }

    ret = ss_mpi_sys_mem_share_all(mem_info.mem_handle);
    if (ret != TD_SUCCESS) {
        ss_mpi_vo_release_screen_frame(vo_layer, &frame_info);
        sample_print("layer %d ss_mpi_sys_mem_share_all fail for %#x!\n", vo_layer, ret);
        return;
    }

    ss_mpi_vo_release_screen_frame(vo_layer, &frame_info);
}
#endif

static td_s32 start_vo_mipi_tx(const sample_vo_mipi_tx_cfg *vo_tx_cfg)
{
    td_s32 ret;
    const sample_vo_cfg *vo_config = &vo_tx_cfg->vo_config;
    const sample_mipi_tx_config *tx_config = &vo_tx_cfg->tx_config;

    ret = sample_comm_vo_start_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("start vo failed with 0x%x!\n", ret);
        return ret;
    }
    printf("start vo dhd%d.\n", vo_config->vo_dev);

#ifdef SAMPLE_MEM_SHARE_ENABLE
    sample_init_vo_mem_share(vo_config->vo_layer);
#endif
    if ((vo_config->vo_intf_type & OT_VO_INTF_MIPI) ||
        (vo_config->vo_intf_type & OT_VO_INTF_MIPI_SLAVE)) {
        ret = sample_comm_start_mipi_tx(tx_config);
        if (ret != TD_SUCCESS) {
            sample_print("start mipi tx failed with 0x%x!\n", ret);
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_void stop_vo_mipi_tx(const sample_vo_mipi_tx_cfg *vo_tx_cfg)
{
    td_s32 ret;
    const sample_vo_cfg *vo_config = &vo_tx_cfg->vo_config;

    sample_comm_stop_mipi_tx(vo_config->vo_intf_type);
    ret = sample_comm_vo_stop_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("stop vo failed with 0x%x!\n", ret);
    }
}

static td_void sample_vo_mipitx_do_stop(sample_vi_cfg *vi_cfg, const sample_vo_mipi_tx_cfg *vo_tx_cfg)
{
    stop_vo_mipi_tx(vo_tx_cfg);
    sample_comm_vi_stop_vi(vi_cfg);
    sample_comm_sys_exit();
}

static td_void sample_vo_do_stop(sample_vi_cfg *vi_cfg, const sample_vo_cfg *vo_config)
{
    td_s32 ret;

    ret = sample_comm_vo_stop_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("stop vo failed with 0x%x!\n", ret);
    }
    sample_comm_vi_stop_vi(vi_cfg);
    sample_comm_sys_exit();
}


static td_s32 sample_vo_mipi_tx(const sample_vo_mipi_tx_cfg *vo_tx_cfg)
{
    /* vi */
    td_s32 ret;
    sample_vi_cfg vi_cfg;

    /* vo */
    const sample_vo_cfg *vo_config = &vo_tx_cfg->vo_config;

    ret = sample_vo_set_sys_cfg(OT_VI_ONLINE_VPSS_OFFLINE, OT_VI_AIISP_MODE_DEFAULT);
    if (ret != TD_SUCCESS) {
        sample_print("start sys failed with 0x%x!\n", ret);
        return ret;
    }

    sample_comm_vi_get_default_vi_cfg(SENSOR0_TYPE, &vi_cfg);

    ret = sample_comm_vi_start_vi(&vi_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vi failed with 0x%x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }

    /* step2: start vo and mipi tx */
    ret = start_vo_mipi_tx(vo_tx_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start mipi tx 1080p failed with 0x%x!\n", ret);
        sample_comm_vi_stop_vi(&vi_cfg);
        sample_comm_sys_exit();
        return ret;
    }

    /* step3: bind vi and vo */
    ret = sample_comm_vi_bind_vo(0, 0, vo_config->vo_layer, 0);
    if (ret != TD_SUCCESS) {
        sample_print("bind vi and vo failed with 0x%x!\n", ret);
        sample_vo_mipitx_do_stop(&vi_cfg, vo_tx_cfg);
        return ret;
    }

    /* step4: do pause. */
    sample_vo_do_pause();

    /* step5: exit */
    ret = sample_comm_vi_un_bind_vo(0, 0, vo_config->vo_layer, 0);
    if (ret != TD_SUCCESS) {
        sample_print("vi unbind vo failed with 0x%x!\n", ret);
    }

    sample_vo_mipitx_do_stop(&vi_cfg, vo_tx_cfg);
    return ret;
}

static td_s32 sample_vo_rgb(const sample_vo_cfg *vo_config)
{
    /* vi */
    td_s32 ret;
    sample_vi_cfg vi_cfg;

    ret = sample_vo_set_sys_cfg(OT_VI_ONLINE_VPSS_OFFLINE, OT_VI_AIISP_MODE_DEFAULT);
    if (ret != TD_SUCCESS) {
        sample_print("start sys failed with 0x%x!\n", ret);
        return ret;
    }

    sample_comm_vi_get_default_vi_cfg(SENSOR0_TYPE, &vi_cfg);

    ret = sample_comm_vi_start_vi(&vi_cfg);
    if (ret != TD_SUCCESS) {
        sample_print("start vi failed with 0x%x!\n", ret);
        sample_comm_sys_exit();
        return ret;
    }

    ret = sample_comm_vo_start_vo(vo_config);
    if (ret != TD_SUCCESS) {
        sample_print("start vo failed with 0x%x!\n", ret);
        sample_comm_vi_stop_vi(&vi_cfg);
        sample_comm_sys_exit();
        return ret;
    }
    /* step3: bind vi and vo */
    ret = sample_comm_vi_bind_vo(0, 0, vo_config->vo_layer, 0);
    if (ret != TD_SUCCESS) {
        sample_print("bind vi and vo failed with 0x%x!\n", ret);
        sample_vo_do_stop(&vi_cfg, vo_config);
        return ret;
    }

    /* step4: do pause. */
    sample_vo_do_pause();

    /* step5: exit */
    ret = sample_comm_vi_un_bind_vo(0, 0, vo_config->vo_layer, 0);
    if (ret != TD_SUCCESS) {
        sample_print("vi unbind vo failed with 0x%x!\n", ret);
    }

    sample_vo_do_stop(&vi_cfg, vo_config);
    return ret;
}

static td_s32 sample_vo_mipi_tx_1200x1920_user(td_void)
{
    return sample_vo_mipi_tx(&g_vo_tx_cfg_1200x1920_user);
}

static td_s32 sample_vo_mipi_tx_3840x720_user(td_void)
{
    return sample_vo_mipi_tx(&g_vo_tx_cfg_3840x720_user);
}

static td_s32 sample_vo_rgb_8bit_240x320_user(td_void)
{
    return sample_vo_rgb(&g_vo_rgb_8bit_config);
}

static void sample_vo_usage(const char *name)
{
    printf("usage : %s <index>\n", name);
    printf("index:\n");
    printf("\t0: vo dhd0 rgb 8bit                   USER    240x320@60 output.\n");
    printf("\t1: vo dhd0 mipi_tx                    USER    1200x1920@60 output.\n");
    printf("\t2: vo dhd0 mipi_tx one to dual-screen USER    3840x720@60 output.\n");
}

static td_s32 sample_vo_execute_case(td_char index)
{
    if (index == '0') {
        return sample_vo_rgb_8bit_240x320_user();
    } else if (index == '1') {
        return sample_vo_mipi_tx_1200x1920_user();
    } else if (index == '2') {
        return sample_vo_mipi_tx_3840x720_user();
    }

    sample_print("the index %c is invalid!\n", index);
    return TD_FAILURE;
}

td_s32 main(td_s32 argc, td_char *argv[])
{
    td_s32 ret;
    td_char index;

    if (argc != 2) { /* 2: 2 arg num */
        sample_vo_usage(argv[0]);
        return TD_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2: 2 chars */
        sample_vo_usage(argv[0]);
        return TD_SUCCESS;
    }

#ifndef __LITEOS__
    sample_sys_signal(sample_vo_handle_sig);
#endif

    if (strlen(argv[1]) > 1) {
        sample_vo_usage(argv[0]);
        return TD_FAILURE;
    }

    index = *argv[1];
    ret = sample_vo_execute_case(index);
    if ((ret == TD_SUCCESS) && (g_vo_sig_flag == 0)) {
        sample_print("sample_vo exit normally!\n");
    } else {
        sample_print("sample_vo exit abnormally!\n");
    }

    return ret;
}
