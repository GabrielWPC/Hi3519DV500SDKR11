/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef MMZ_TEE_ALLOCATOR_H
#define MMZ_TEE_ALLOCATOR_H

#include <linux/err.h>
#include <linux/kref.h>
#include <linux/list.h>

#include "smmz_agent_export.h"

struct memory_zone {
    unsigned long phys_start;
    unsigned long nbytes;
    struct list_head mmb_list;
    struct list_head list;

    unsigned long block_align;
};
typedef struct memory_zone mmz_t;

struct memory_block {
    struct memory_zone *zone;
    struct list_head list;
    unsigned long phys_addr;

    void *kvirt;
    unsigned long length;
    unsigned long flags;
};
typedef struct memory_block mmb_t;

struct mmz_allocator {
    int (*mmb_alloc)(unsigned long size, mmz_t *, mmb_t *);
    void (*mmb_free)(mmb_t *mmb);
};

typedef struct {
    unsigned long phys_start;
    unsigned long nbytes;
    unsigned long block_align;
} ot_mmz_create_attr;

struct sec_mmz_info {
    char name[MMZ_NAME_LEN];
    /* list node on sdp_smmz_list */
    struct list_head list;

    mmz_t mmz;

    /* mmb_list protect lock */
    spinlock_t lock;
};

struct smmb_info {
    struct memory_block mmb;

    struct kref ref;
};

struct smmz_create_attr {
    ot_mmz_create_attr mmz_create_attr;
    const char *name;
    struct smmz_agent_zone_manager *zone_manager;
};

struct sec_memory_zone_select_parameter {
    char zone_name[MMZ_NAME_LEN];
};

struct smmz_agent_zone_selector {
    mmz_t *(*zone_select_func)(struct sec_memory_zone_select_parameter, struct smmz_agent_zone_selector *);
    bool (*mmb_in_sec_zone)(struct smmb_info *mmb, struct smmz_agent_zone_selector *);
};

struct smmz_agent_allocator {
    struct smmz_agent_zone_selector *zone_selector;
    struct mmz_allocator allocator;
};

int mmz_init(const ot_mmz_create_attr *mmz_create_attr, mmz_t *p);
int sec_mmz_init(const struct smmz_create_attr smmz_create_attr, struct sec_mmz_info *p);

int smmz_agent_allocator_init(struct smmz_agent_zone_selector *selector, struct smmz_agent_allocator *allocator);

struct smmb_info *smmb_phy_addr_alloc(unsigned long size, struct sec_memory_zone_select_parameter para,
                                      struct smmz_agent_allocator *allocator);

int smmb_put(struct smmb_info *mmb, struct smmz_agent_allocator *allocator);
int smmb_get(struct smmb_info *mmb, struct smmz_agent_allocator *allocator);
#endif
