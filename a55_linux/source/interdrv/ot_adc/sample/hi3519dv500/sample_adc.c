/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ot_adc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DEV_FILE "/dev/ot_lsadc"

typedef enum {
    LSADC_SCAN_MODE_SINGLE_STEP,
    LSADC_SCAN_MODE_CONTINUOUS,
    LSADC_SCAN_MODE_ERROR
} ot_lsadc_scan_mode;

int sample_lsadc_singlestep_scanmode(ot_lsadc_scan_mode adc_mode, int chn)
{
    int i, value;
    int fd  = -1;
    fd = open(DEV_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open file:%s\n", DEV_FILE);
        return -1;
    }

    if (ioctl(fd, LSADC_IOC_MODEL_SEL, &adc_mode) < 0) {
        fprintf(stderr, "adc model select error.\n");
        goto exit;
    }

    if (ioctl(fd, LSADC_IOC_CHN_ENABLE, &chn) < 0) {
        fprintf(stderr, "enable chn %d error.\n", chn);
        goto exit;
    }

    for (i = 0; i < 20; i++) { /* get 20 times */
        if (ioctl(fd, LSADC_IOC_START) < 0) {
            fprintf(stderr, "start lsadc error.\n");
            goto exit;
        }

        value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
        printf("get value:%d, chn[%d]\n", value, chn);
        sleep(1);

        if (ioctl(fd, LSADC_IOC_STOP) < 0) {
            fprintf(stderr, "stop lsadc error.\n");
        }
    }

exit:
    if (ioctl(fd, LSADC_IOC_STOP) < 0) {
        fprintf(stderr, "stop lsadc error.\n");
    }

    if (ioctl(fd, LSADC_IOC_CHN_DISABLE, &chn) < 0) {
        fprintf(stderr, "disable chn %d error.\n", chn);
    }

    close(fd);
    return 0;
}

int sample_lsadc_continuous_scanmode(ot_lsadc_scan_mode adc_mode, int chn)
{
    int i, value;
    int fd  = -1;
    fd = open(DEV_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open file:%s\n", DEV_FILE);
        return -1;
    }

    if (ioctl(fd, LSADC_IOC_MODEL_SEL, &adc_mode) < 0) {
        fprintf(stderr, "adc model select error.\n");
        goto exit;
    }

    if (ioctl(fd, LSADC_IOC_CHN_ENABLE, &chn) < 0) {
        fprintf(stderr, "enable chn %d error.\n", chn);
        goto exit;
    }

    if (ioctl(fd, LSADC_IOC_START) < 0) {
        fprintf(stderr, "start lsadc error.\n");
        goto exit;
    }

    for (i = 0; i < 20; i++) { /* get 20 times */
        value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
        printf("get value:%d,chn[%d]\n", value, chn);
        sleep(1);
    }

exit:
    if (ioctl(fd, LSADC_IOC_STOP) < 0) {
        fprintf(stderr, "stop lsadc error.\n");
    }

    if (ioctl(fd, LSADC_IOC_CHN_DISABLE, &chn) < 0) {
        fprintf(stderr, "disable chn %d error.\n", chn);
    }

    close(fd);
    return 0;
}

int main(int argc, char* argv[])
{
    ot_lsadc_scan_mode adc_mode;
    int chn;
    int ret;

    printf("select scan mode:\n");
    printf(" - [0] is single step scan mode\n");
    printf(" - [1] is Continuous scan mode\n");

    ret = getchar();
    if (ret != '0' && ret != '1') {
        printf("invalid scan mode: %c\n", ret < 0 ? '?' : (char)ret);
        return -1;
    }

    adc_mode = ret - '0';

    (void)getchar();
    printf("select ADC CHN:\n");
    printf(" - [0] is CHN[0]\n");
    printf(" - [1] is CHN[1]\n");
    printf(" - [2] is CHN[2]\n");
    printf(" - [3] is CHN[3]\n");

    ret = getchar();
    if (ret < '0' || ret > '3') { /* chn[0, 3] */
        printf("invalid ADC CHN: %c\n", ret < 0 ? '?' : (char)ret);
        return -1;
    }
    chn = ret - '0';
    switch (adc_mode) {
        case LSADC_SCAN_MODE_SINGLE_STEP:
            sample_lsadc_singlestep_scanmode(adc_mode, chn);
            break;
        case LSADC_SCAN_MODE_CONTINUOUS:
            sample_lsadc_continuous_scanmode(adc_mode, chn);
            break;
        default:
            printf("error scan mode\n");
            break;
    }
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

