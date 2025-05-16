/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef SMMZ_AGENT_PRIVATE_H
#define SMMZ_AGENT_PRIVATE_H

#include <linux/idr.h>
#include <linux/kref.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include "smmz_agent_phy_buffer_allocator.h"

#define MMZ_GRAIN PAGE_SIZE

struct smmb_ownership_info {
    struct smmb_info *smmb;
    int buf_id;
};

struct smmz_agent_zone_manager {
    /* mmz dont support dynamic modification */
    struct smmz_agent_zone_selector selector;
    struct list_head sdp_smmz_list;
};

struct smmz_agent_ownership_list {
    struct idr ownership_ids;
    /* protect ids */
    struct mutex mutex;
};

#define NPU_CLASS_NUM_MAX 16

struct smmz_agent_driver_data {
    struct smmz_agent_allocator *allocator;
    struct smmz_agent_ownership_list ownership_list;
    struct smmz_agent_zone_manager zone_manager;
    char mmz_name_map[NPU_CLASS_NUM_MAX][MMZ_NAME_LEN];
};

int smmb_alloc_ownership(const char *mmz_name, ssize_t size);
char *smmz_agent_name_map(int npuclass);
struct smmb_ownership_info *get_ownershipinfo(int buf_id);
void ownership_release(int buf_id);
#endif
