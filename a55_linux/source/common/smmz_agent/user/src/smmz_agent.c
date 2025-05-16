/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <fcntl.h>
#include <securec.h>
#include <smmz_agent.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#define SMMZ_AGENT_PUSDO_DEVICE_NODE_NAME "/dev/smmz_agent_userdev"

static int fd = -1;

int smmz_agent_init(void)
{
    if (fd == -1) {
        fd = open(SMMZ_AGENT_PUSDO_DEVICE_NODE_NAME, O_RDWR | O_SYNC);
    }

    return fd;
}

void smmz_agent_close(void)
{
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

int smmb_alloc_from_sec_zone(const char *name, ssize_t size)
{
    int ret = -1;
    struct smmz_agent_args args = { 0 };
    if (strcpy_s(args.mmz_name, OT_MMZ_NAME_LEN, name)) {
        printf("%s: strcpy error\n", __func__);
        return -1;
    }
    args.size = size;
    if (fd != -1) {
        ret = ioctl(fd, IOC_SMMZ_AGENT_ALLOC_SMMZ, &args);
    }

    if (ret < 0) {
        printf("system alloc mmz memory failed!\n");
        return 0;
    }

    return ret;
}

int smmb_alloc_from_sec_zone_npuclass(int npuclass, ssize_t size)
{
    int ret = -1;
    struct smmz_agent_args args = { 0 };
    args.npuclass = npuclass;
    args.size = size;
    if (fd != -1) {
        ret = ioctl(fd, IOC_SMMZ_AGENT_ALLOC_SMMZ_NPUCLASS, &args);
    }
    if (ret < 0) {
        printf("system alloc mmz memory failed!\n");
        return 0;
    }

    return ret;
}

unsigned long smmb_get_phy_addr(int buf_fd)
{
    int ret = -1;
    struct smmz_agent_args args = { 0 };
    args.buf_fd = buf_fd;
    if (fd != -1) {
        ret = ioctl(fd, IOC_SMMZ_AGENT_GET_PHY_ADDR, &args);
    }
    if (ret != 0) {
        printf("system alloc mmz memory failed!\n");
        return 0;
    }

    return args.phy_addr;
}

ssize_t smmb_get_size(int buf_fd)
{
    int ret = -1;
    struct smmz_agent_args args = { 0 };
    args.buf_fd = buf_fd;
    if (fd != -1) {
        ret = ioctl(fd, IOC_SMMZ_AGENT_GET_SIZE, &args);
    }

    if (ret != 0) {
        printf("system alloc mmz memory failed!\n");
        return 0;
    }

    return args.size;
}

int smmb_free(int buf_fd)
{
    return close(buf_fd);
}
