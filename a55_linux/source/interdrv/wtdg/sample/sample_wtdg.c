/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ot_wtdg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DEV_FILE "/dev/watchdog"

void print_help(void)
{
    (void)fprintf(stdout,
        "\n"
        "Usage: ./sample_wtdg [options] ...\n"
        "Options: \n"
        "    -s(set)  e.g. -s timeout 30\n"
        "    -g(get)  e.g. -g timeout\n"
        "    -f(feed) e.g. -f\n");
}

static int get_param_from_argv(const char *argv, long *val)
{
    char *end_ptr = NULL;
    long param;

    errno = 0;
    param = strtol(argv, &end_ptr, 10); /* base 10 */
    if ((end_ptr == argv) || (*end_ptr != '\0')) {
        fprintf(stderr, "[Error]%s(%d): invalid arg %s\n", __FUNCTION__, __LINE__, argv);
        return -1;
    } else if ((param == LONG_MIN || param == LONG_MAX) && (errno == ERANGE)) {
        fprintf(stderr, "[Error]%s(%d): invalid arg %s\n", __FUNCTION__, __LINE__, argv);
        return -1;
    }
    *val = param;
    return 0;
}

static int set_func_test(int fd, int argc, char *argv[])
{
    int ret;

    if (strcmp("timeout", argv[2]) == 0) { /* 2: The second argument */
        long timeout;

        if (get_param_from_argv(argv[3], &timeout) != 0) { /* 3: The third argument */
            return -1;
        }
        ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
        if (ret != 0) {
            fprintf(stderr, "[Error]%s(%d): ioctl ret =%d\n", __FUNCTION__, __LINE__, ret);
            return -1;
        }
    } else if (strcmp("option", argv[2]) == 0) { /* 2: The second argument */
        long  options;

        if (get_param_from_argv(argv[3], &options) != 0) { /* 3: The third argument */
            return -1;
        }
        printf("set new option :%ld \n", options);
        ret = ioctl(fd, WDIOC_SETOPTIONS, &options);
        if (ret != 0) {
            fprintf(stderr, "[Error]%s(%d): ioctl ret =%d\n", __FUNCTION__, __LINE__, ret);
            return -1;
        }
    } else {
        print_help();
    }
    return 0;
}

static int get_func_test(int fd, int argc, char *argv[])
{
    int ret;
    int timeout = 0;

    if (strcmp("timeout", argv[2]) == 0) { /* 2: The second argument */
        ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
        if (ret != 0) {
            fprintf(stderr, "[Error]%s(%d): ioctl ret =%d\n", __FUNCTION__, __LINE__, ret);
            return -1;
        } else {
            fprintf(stdout, "Resp: timeout=%d\n", timeout);
        }
    } else if (strcmp("option", argv[2]) == 0) { /* 2: The second argument */
        printf("Line:%d \n", __LINE__);
        ret = ioctl(fd, WDIOC_GETSTATUS, &timeout);
        if (ret != 0) {
            fprintf(stderr, "[Error]%s(%d): ioctl ret =%d\n", __FUNCTION__, __LINE__, ret);
            return -1;
        } else {
            fprintf(stdout, "Resp: option=%d\n", timeout);
        }
    } else {
        print_help();
    }
    return 0;
}

static int parse_args_and_proc(int fd, int argc, char *argv[])
{
    int ret = 0;

    if (argc == 1) {
        print_help();
        return -1;
    }
    int i = argc - 1;
    while (i > 0) {
        printf("%d: %s  ", i, argv[i]);
        i--;
    }
    printf("\n");
    if ((strcmp("-s", argv[1]) == 0) && (argc == 4)) { /* 4 args like './test -s timeout 30' */
        ret = set_func_test(fd, argc, argv);
        if (ret != 0) {
            return -1;
        }
    } else if ((strcmp("-g", argv[1]) == 0) && (argc == 3)) { /* 3 args like './test -g timeout' */
        ret = get_func_test(fd, argc, argv);
        if (ret != 0) {
            return -1;
        }
    } else if ((strcmp("-f", argv[1]) == 0) && (argc == 2)) { /* 2 args like './test -f' */
        ret = ioctl(fd, WDIOC_KEEPALIVE);
        if (ret != 0) {
            fprintf(stderr, "[Error]%s(%d): ioctl ret =%d\n", __FUNCTION__, __LINE__, ret);
            return -1;
        }
    } else {
        print_help();
    }
    return ret;
}

#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int fd  = open(DEV_FILE, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "fail to open file:%s\n", DEV_FILE);
        return -1;
    }

    parse_args_and_proc(fd, argc, argv);

    close(fd);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
