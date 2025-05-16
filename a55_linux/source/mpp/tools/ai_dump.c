/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include "securec.h"
#include "ot_common.h"
#include "ot_common_aio.h"
#include "ss_mpi_audio.h"

#define SAVE_FILE_DEFAULT_SIZE 1024
#define SAVE_FILE_DEFAULT_NAME "default"

#define AI_PATH_NAME_MAXLEN 256
#define AI_DUMP_ARG_NUMBER_BASE 10

#define AI_DUMP_ARG_FILE_SIZE_INDEX 3
#define AI_DUMP_ARG_SAMPLE_RATE_INDEX 4

#define audio_value_between(x, min, max) (((x) >= (min)) && ((x) < (max)))

static ot_audio_save_file_info g_save_file_info = {0};
static ot_audio_dev g_ai_dev_id = 0;
static ot_ai_chn g_ai_chn = 0;
static ot_audio_sample_rate g_in_sample_rate = OT_AUDIO_SAMPLE_RATE_16000;
static ot_audio_sample_rate g_out_sample_rate = OT_AUDIO_SAMPLE_RATE_16000;

static volatile sig_atomic_t g_signal_flag = 0;

#ifndef __LITEOS__
static FILE *g_cur_path_fp = TD_NULL;

static td_void tool_audio_handle_sig(td_s32 signo)
{
    if ((signo == SIGINT) || (signo == SIGTERM)) {
        g_signal_flag = 1;
    }
}

static td_void ai_dump_sig_proc(td_void)
{
    g_save_file_info.cfg = TD_FALSE;
    ss_mpi_ai_save_file(g_ai_dev_id, g_ai_chn, &g_save_file_info);

    if (g_cur_path_fp != TD_NULL) {
        pclose(g_cur_path_fp);
        g_cur_path_fp = TD_NULL;
    }

    printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
}
#else
static td_void ai_dump_sig_proc(td_void)
{
    return;
}
#endif

static td_void tool_audio_usage(td_void)
{
    printf("\n*************************************************\n"
           "Usage: ./ai_dump <dev> <chn> [size] [out_sample_rate]\n"
           "1)dev: ai device id.\n"
           "2)chn: ai channel id.\n"
           "3)size: file size(KB).\n"
           "default:1024\n"
           "4)out_sample_rate: output sample rate(Hz).\n"
           "range:8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000\n"
           "\n"
           "Note:\n"
           "1)path for saving is current path, name for saving is 'default'.\n"
           "2)need to assign out_sample_rate manual when enable resample.\n"
           "\n*************************************************\n");
}

static td_bool audio_check_sample_rate(ot_audio_sample_rate sample_rate)
{
    if ((sample_rate != OT_AUDIO_SAMPLE_RATE_8000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_12000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_11025) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_16000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_22050) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_24000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_32000) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_44100) &&
        (sample_rate != OT_AUDIO_SAMPLE_RATE_48000)) {
        return TD_FALSE;
    }

    return TD_TRUE;
}

static td_s32 ai_dump_set_dev(int argc, char *argv[])
{
    td_char *end_ptr = TD_NULL;

    g_ai_dev_id = strtol(argv[1], &end_ptr, 10); /* 10:base, argv[1]:dev id */
    if (*end_ptr != '\0') {
        tool_audio_usage();
        return -1;
    }
    if (!audio_value_between(g_ai_dev_id, 0, OT_AI_DEV_MAX_NUM)) {
        printf("dev id must be [0,%d)!!!!\n\n", OT_AI_DEV_MAX_NUM);
        return -1;
    }

    return 0;
}

static td_s32 ai_dump_set_chn(int argc, char *argv[])
{
    td_char *end_ptr = TD_NULL;

    g_ai_chn = strtol(argv[2], &end_ptr, 10); /* 10:base, argv[2]:chn id */
    if (*end_ptr != '\0') {
        tool_audio_usage();
        return -1;
    }
    if (!audio_value_between(g_ai_chn, 0, OT_AIO_MAX_CHN_NUM)) {
        printf("chn id must be [0,%d)!!!!\n\n", OT_AIO_MAX_CHN_NUM);
        return -1;
    }

    return 0;
}

static td_s32 ai_dump_set_file_size(int argc, char *argv[])
{
    td_slong size = SAVE_FILE_DEFAULT_SIZE;
    td_char *end_ptr = TD_NULL;

    if (argc >= (AI_DUMP_ARG_FILE_SIZE_INDEX + 1)) {
        errno = 0;
        size = strtol(argv[AI_DUMP_ARG_FILE_SIZE_INDEX], &end_ptr, AI_DUMP_ARG_NUMBER_BASE);
        if ((end_ptr == argv[AI_DUMP_ARG_FILE_SIZE_INDEX]) || (*end_ptr != '\0')) {
            printf("file size invalid!!!\n");
            return -1;
        } else if (((size == LONG_MIN) || (size == LONG_MAX)) && (errno == ERANGE)) {
            printf("file size range overflow!!!\n");
            return -1;
        } else if (!audio_value_between(size, 1, 10 * 1024 + 1)) { /* 10*1024+1:max file size */
            printf("file size must be [1, 10*1024]!!!!\n\n");
            return -1;
        }
    }
    g_save_file_info.file_size = size;

    return 0;
}

static td_s32 ai_dump_set_sample_rate(int argc, char *argv[])
{
    ot_aio_attr ai_attr = { 0 };
    td_s32 ret;
    td_slong result;
    td_char *end_ptr = TD_NULL;

    ret = ss_mpi_ai_get_pub_attr(g_ai_dev_id, &ai_attr);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_ai_get_pub_attr fail, ret:0x%x\n", ret);
        return -1;
    }
    g_in_sample_rate = ai_attr.sample_rate;
    if (argc >= AI_DUMP_ARG_SAMPLE_RATE_INDEX + 1) {
        errno = 0;
        result = strtol(argv[AI_DUMP_ARG_SAMPLE_RATE_INDEX], &end_ptr, AI_DUMP_ARG_NUMBER_BASE);
        if ((end_ptr == argv[AI_DUMP_ARG_SAMPLE_RATE_INDEX]) || (*end_ptr != '\0')) {
            printf("out_sample_rate param invalid!!!\n");
            return -1;
        } else if (((result == LONG_MIN) || (result == LONG_MAX)) && (errno == ERANGE)) {
            printf("out_sample_rate overflow!!!\n");
            return -1;
        } else if (!audio_check_sample_rate((ot_audio_sample_rate)result)) {
            printf("out_sample_rate must be [8000, 48000]!!!!\n\n");
            return -1;
        }
        g_out_sample_rate = (ot_audio_sample_rate)result;
    } else {
        g_out_sample_rate = ai_attr.sample_rate;
    }

    return 0;
}

static td_s32 ai_dump_set_file_path(td_void)
{
    td_s32 ret;
    td_char cur_path[OT_MAX_AUDIO_FILE_PATH_LEN + 2]; /* 2: '\n'+'\0' */

#ifndef __LITEOS__
    g_cur_path_fp = popen("pwd", "r");
    if (g_cur_path_fp == TD_NULL) {
        return -1;
    }

    if (fgets(cur_path, sizeof(cur_path), g_cur_path_fp) == TD_NULL) {
        pclose(g_cur_path_fp);
        g_cur_path_fp = TD_NULL;
        return -1;
    }

    if (!audio_value_between(strlen(cur_path), 1, OT_MAX_AUDIO_FILE_PATH_LEN)) {
        printf("path length must be [1,256]!!!!\n\n");
        pclose(g_cur_path_fp);
        g_cur_path_fp = TD_NULL;
        return -1;
    }
    cur_path[strlen(cur_path) - 1] = '/'; /* replace '\n' with '/' */
    pclose(g_cur_path_fp);
    g_cur_path_fp = TD_NULL;
#else
    cur_path[0] = '.';
    cur_path[1] = '/';
    cur_path[2] = '\0'; /* 2: index */
#endif

    ret = memcpy_s(g_save_file_info.file_path, OT_MAX_AUDIO_FILE_PATH_LEN - 1,
                   cur_path, strlen(cur_path) + 1);
    if (ret != EOK) {
        printf("file_path memcpy_s fail, ret: 0x%x.\n\n", ret);
        return -1;
    }

    return 0;
}

static td_s32 ai_dump_set_file_name(td_void)
{
    td_s32 ret;

    ret = memcpy_s(g_save_file_info.file_name, OT_MAX_AUDIO_FILE_NAME_LEN - 1,
        SAVE_FILE_DEFAULT_NAME, strlen(SAVE_FILE_DEFAULT_NAME) + 1);
    if (ret != EOK) {
        printf("file_name memcpy_s fail, ret: 0x%x.\n\n", ret);
        return -1;
    }

    return 0;
}

static td_s32 ai_dump_check_and_set(int argc, char *argv[])
{
    td_s32 ret;

    if (argc < 3 || argc > 5) { /* 3,5: min argc, max argc  */
        tool_audio_usage();
        return -1;
    }
    errno = 0;

    if (!strncmp(argv[1], "-h", strlen("-h"))) {
        tool_audio_usage();
        return -1;
    } else {
        ret = ai_dump_set_dev(argc, argv);
        if (ret != 0) {
            return ret;
        }
    }

    ret = ai_dump_set_chn(argc, argv);
    if (ret != 0) {
        return ret;
    }

    ret = ai_dump_set_file_size(argc, argv);
    if (ret != 0) {
        return ret;
    }

    ret = ai_dump_set_sample_rate(argc, argv);
    if (ret != 0) {
        return ret;
    }

    ret = ai_dump_set_file_path();
    if (ret != 0) {
        return ret;
    }

    ret = ai_dump_set_file_name();
    if (ret != 0) {
        return ret;
    }

    g_save_file_info.cfg = TD_TRUE;

    return 0;
}

static td_s32 tool_audio_create_file(ot_audio_dev ai_dev, ot_ai_chn ai_chn,
    const ot_audio_save_file_info *save_file_info, ot_audio_sample_rate in_sample_rate,
    ot_audio_sample_rate out_sample_rate)
{
    td_s32 ret;
    td_char sin_file_path[AI_PATH_NAME_MAXLEN + 1];
    td_char sou_file_path[AI_PATH_NAME_MAXLEN + 1];
    td_char rin_file_path[AI_PATH_NAME_MAXLEN + 1];
    FILE *fd_temp = TD_NULL;

    ret = snprintf_s(sin_file_path, AI_PATH_NAME_MAXLEN + 1, AI_PATH_NAME_MAXLEN,
        "%s/sin_ai_dev%d_chn%d_%uk_%s.pcm", save_file_info->file_path, ai_dev, ai_chn,
        in_sample_rate / 1000, save_file_info->file_name); /* 1000: kHz */
    if (ret <= EOK) {
        printf("sin file path snprintf_s fail, ret:0x%x\n", ret);
        return -1;
    }

    ret = snprintf_s(sou_file_path, AI_PATH_NAME_MAXLEN + 1, AI_PATH_NAME_MAXLEN,
        "%s/sou_ai_dev%d_chn%d_%uk_%s.pcm", save_file_info->file_path, ai_dev, ai_chn,
        out_sample_rate / 1000, save_file_info->file_name); /* 1000: kHz */
    if (ret <= EOK) {
        printf("sou file path snprintf_s fail, ret:0x%x\n", ret);
        return -1;
    }

    ret = snprintf_s(rin_file_path, AI_PATH_NAME_MAXLEN + 1, AI_PATH_NAME_MAXLEN,
        "%s/rin_ai_dev%d_chn%d_%uk_%s.pcm", save_file_info->file_path, ai_dev, ai_chn,
        in_sample_rate / 1000, save_file_info->file_name); /* 1000: kHz */
    if (ret <= EOK) {
        printf("rin file path snprintf_s fail, ret:0x%x\n", ret);
        return -1;
    }

    fd_temp = fopen(sin_file_path, "wb+");
    if (fd_temp == TD_NULL) {
        printf("fopen sin file fail.\n");
        return -1;
    }
    (td_void)fclose(fd_temp);

    fd_temp = fopen(sou_file_path, "wb+");
    if (fd_temp == TD_NULL) {
        printf("fopen sou file fail.\n");
        return -1;
    }
    (td_void)fclose(fd_temp);

    fd_temp = fopen(rin_file_path, "wb+");
    if (fd_temp == TD_NULL) {
        printf("fopen rin file fail.\n");
        return -1;
    }
    (td_void)fclose(fd_temp);

    return 0;
}

#ifdef __LITEOS__
td_s32 ai_dump(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    ot_audio_file_status file_status = {0};
    td_s32 ret;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");

    ret = ai_dump_check_and_set(argc, argv);
    if (ret != 0) {
        return -1;
    }

    ret = tool_audio_create_file(g_ai_dev_id, g_ai_chn, &g_save_file_info,
        g_in_sample_rate, g_out_sample_rate);
    if (ret != 0) {
        return -1;
    }

#ifndef __LITEOS__
    (td_void)signal(SIGINT, tool_audio_handle_sig);
    (td_void)signal(SIGTERM, tool_audio_handle_sig);
#endif

    printf("file path:%s, file name:%s, file size:%u*1024\n\n", g_save_file_info.file_path,
        g_save_file_info.file_name, g_save_file_info.file_size);

    ret = ss_mpi_ai_save_file(g_ai_dev_id, g_ai_chn, &g_save_file_info);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_ai_save_file() error, ret=%x!!!!\n\n", ret);
        return -1;
    }

    printf("saving file now, please wait.\n");

    do {
        ret = ss_mpi_ai_query_file_status(g_ai_dev_id, g_ai_chn, &file_status);
        if (g_signal_flag != 0) {
            ai_dump_sig_proc();
            break;
        } else if ((ret != TD_SUCCESS) || (!file_status.saving)) {
            break;
        }

        usleep(200000); /* 200000 us */
    } while (file_status.saving);

    printf("file saving is finished.\n");

    return TD_SUCCESS;
}
