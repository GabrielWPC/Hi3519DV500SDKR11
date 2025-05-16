/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef _SMMZ_AGENT_MISC_DEV_H
#define _SMMZ_AGENT_MISC_DEV_H

#include "smmz_agent_export.h"
#include <linux/types.h>

struct smmz_agent_args {
    char mmz_name[MMZ_NAME_LEN];
    int buf_fd;
    ssize_t size;
    void *vaddr;
    unsigned long phy_addr;
    int npuclass;
};

#define IOC_SMMZ_AGENT_ALLOC_SMMZ _IOWR('m', 11, struct smmz_agent_args)

#define IOC_SMMZ_AGENT_ALLOC_SMMZ_NPUCLASS _IOWR('m', 14, struct smmz_agent_args)

#define IOC_SMMZ_AGENT_GET_SIZE _IOWR('m', 25, struct smmz_agent_args)
#define IOC_SMMZ_AGENT_GET_PHY_ADDR _IOWR('m', 26, struct smmz_agent_args)

int smmz_agent_userdev_init(void);
void smmz_agent_userdev_deinit(void);

#endif
