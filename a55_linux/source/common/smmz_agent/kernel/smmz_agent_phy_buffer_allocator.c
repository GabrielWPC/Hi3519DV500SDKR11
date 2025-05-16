/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "smmz_agent_phy_buffer_allocator.h"

#include <asm/cacheflush.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/securec.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "smmz_agent_private.h"

#define MMZ_DBG_LEVEL 0x0
#define MMZ_INFO_LEVEL 0x1

#define OT_MMB_MAP2KERN (1 << 0)
#define OT_MMB_MAP2KERN_CACHED (1 << 1)

#define mmz_trace(level, s, params...)                                             \
    do {                                                                           \
        if (level & MMZ_DBG_LEVEL)                                                 \
            printk(KERN_INFO "[%s, %d]: " s "\n", __FUNCTION__, __LINE__, params); \
    } while (0)

#define mmz_align2(x, g) (((g) == 0) ? (0) : ((((x) + (g)-1) / (g)) * (g)))
#define mmz_grain_align(x) mmz_align2(x, MMZ_GRAIN)
#define mmz_length2grain(len) (mmz_grain_align(len) / MMZ_GRAIN)

const long long g_max_malloc_size = 0x40000000UL;

static void mmb_list_lock(mmz_t *zone)
{
    struct sec_mmz_info *mmz = container_of(zone, typeof(*mmz), mmz);
    spin_lock(&mmz->lock);
}

static void mmb_list_unlock(mmz_t *zone)
{
    struct sec_mmz_info *mmz = container_of(zone, typeof(*mmz), mmz);
    spin_unlock(&mmz->lock);
}

static int calculate_fixed_region(unsigned long end, unsigned long start, unsigned long *fixed_start,
                                  unsigned long *fixed_len, unsigned long len)
{
    unsigned long blank_len;
    int ret = 0;

    blank_len = end - start;
    if ((blank_len < *fixed_len) && (blank_len >= len)) {
        *fixed_len = blank_len;
        *fixed_start = start;
        mmz_trace(MMZ_INFO_LEVEL, "fixed_region: start=0x%08lX, len=%luKB\n", *fixed_start, *fixed_len / SZ_1K);
        ret = 1;
    }
    return ret;
}

static unsigned long find_fixed_region(unsigned long *region_len, mmz_t *mmz, unsigned long size, unsigned long align)
{
    unsigned long start, len;
    unsigned long fixed_start = 0;
    unsigned long fixed_len = -1;
    mmb_t *p = NULL;

    align = mmz_grain_align(align);
    if (align == 0) {
        align = MMZ_GRAIN;
    }

    start = mmz_align2(mmz->phys_start, align);
    len = mmz_grain_align(size);

    list_for_each_entry(p, &mmz->mmb_list, list)
    {
        mmb_t *next = NULL;

        mmz_trace(MMZ_INFO_LEVEL, "p->phys_addr=0x%08lX p->length = %luKB \t", p->phys_addr, p->length / SZ_1K);

        next = list_entry(p->list.next, typeof(*p), list);
        mmz_trace(MMZ_INFO_LEVEL, ",next = 0x%08lX\n\n", next->phys_addr);

        /*
         * if p is the first entry or not.
         */
        if (list_first_entry(&mmz->mmb_list, typeof(*p), list) == p) {
            (void)calculate_fixed_region(p->phys_addr, start, &fixed_start, &fixed_len, len);
        }

        start = mmz_align2((p->phys_addr + p->length), align);

        /* if aglin is larger than mmz->nbytes, it would trigger the BUG_ON */
        /* if we have to alloc after the last node.  */
        if (list_is_last(&p->list, &mmz->mmb_list)) {
            if (calculate_fixed_region(mmz->phys_start + mmz->nbytes, start, &fixed_start, &fixed_len, len) == 1) {
                break;
            } else {
                if (fixed_len != (unsigned long)-1) {
                    goto out;
                }
                fixed_start = 0;
                mmz_trace(MMZ_INFO_LEVEL, "fixed_region: start=0x%08lX, len=%luKB\n", fixed_start, fixed_len / SZ_1K);
                goto out;
            }
        }

        /* blank is too small */
        if ((start + len) > next->phys_addr) {
            mmz_trace(MMZ_INFO_LEVEL, "start=0x%08lX ,len=%lu,next=0x%08lX\n", start, len, next->phys_addr);
            continue;
        }

        (void)calculate_fixed_region(next->phys_addr, start, &fixed_start, &fixed_len, len);
    }

    if ((mmz_grain_align(start + len) <= (mmz->phys_start + mmz->nbytes)) && (start >= mmz->phys_start) &&
        (start < (mmz->phys_start + mmz->nbytes))) {
        fixed_start = start;
    } else {
        fixed_start = 0;
    }

    mmz_trace(MMZ_INFO_LEVEL, "fixed_region: start=0x%08lX, len=%luKB\n", fixed_start, len / SZ_1K);
out:
    *region_len = len;
    return fixed_start;
}

struct alloc_result {
    unsigned long size;
    unsigned long start;
};

struct alloc_result mmb_alloc(unsigned long size, mmz_t *mmz)
{
    unsigned long align = mmz->block_align;
    unsigned long size_aligned = mmz_align2(size, align);
    struct alloc_result result = { 0 };
    result.start = find_fixed_region(&result.size, mmz, size_aligned, align);
    return result;
}

int private_mmb_alloc(unsigned long size, mmz_t *mmz, mmb_t *mmb)
{
    struct alloc_result result;
    mmb_t *p;

    mmb_list_lock(mmz);
    result = mmb_alloc(size, mmz);
    if (result.size < size) {
        mmb_list_unlock(mmb->zone);
        return -EFAULT;
    }

    mmb->zone = mmz;
    mmb->phys_addr = result.start;
    mmb->length = result.size;
    /* add mmb sorted */
    list_for_each_entry(p, &mmb->zone->mmb_list, list)
    {
        if (mmb->phys_addr < p->phys_addr) {
            break;
        }

        if (mmb->phys_addr == p->phys_addr) {
            mmb_list_unlock(mmb->zone);
            return -EFAULT;
        }
    }

    list_add(&mmb->list, p->list.prev);
    mmb_list_unlock(mmb->zone);
    return 0;
}

static void private_mmb_free(mmb_t *mmb)
{
    if (mmb->flags & OT_MMB_MAP2KERN_CACHED) {
#ifdef CONFIG_64BIT
        __flush_dcache_area((void *)mmb->kvirt, (size_t)mmb->length);
#else
        __cpuc_flush_dcache_area((void *)mmb->kvirt, (size_t)mmb->length);
        outer_flush_range(mmb->phys_addr, mmb->phys_addr + mmb->length);
#endif
    }

    mmb_list_lock(mmb->zone);
    list_del(&mmb->list);
    mmb_list_unlock(mmb->zone);
}

static int mmz_allocator_setopt(struct mmz_allocator *allocator)
{
    if (allocator == NULL) {
        pr_err("error parameters\n");
        return -ENOMEM;
    }

    allocator->mmb_alloc = private_mmb_alloc;
    allocator->mmb_free = private_mmb_free;

    return 0;
}

static int smmz_agent_zone_selector_regist(struct smmz_agent_allocator *allocator,
                                           struct smmz_agent_zone_selector *selector)
{
    if (allocator == NULL || selector == NULL || selector->zone_select_func == NULL) {
        pr_err("zone selector parameter is incorrect\n");
        return -ENOMEM;
    }

    allocator->zone_selector = selector;
    return 0;
}

int smmz_agent_allocator_init(struct smmz_agent_zone_selector *selector, struct smmz_agent_allocator *allocator)
{
    if (allocator == NULL) {
        pr_err("mmz alloc error\n");
        return -ENOMEM;
    }

    if (mmz_allocator_setopt(&allocator->allocator))
        return -ENOMEM;

    if (smmz_agent_zone_selector_regist(allocator, selector))
        return -ENOMEM;

    return 0;
}

bool smmz_agent_allocator_inited(struct smmz_agent_allocator *allocator)
{
    return allocator != NULL && allocator->zone_selector != NULL && allocator->allocator.mmb_alloc != NULL &&
           allocator->allocator.mmb_free != NULL;
}

bool smmb_allocated(struct smmb_info *mmb, struct smmz_agent_allocator *allocator)
{
    mmb_t *p = NULL;

    list_for_each_entry(p, &mmb->mmb.zone->mmb_list, list)
    {
        if (p == &mmb->mmb)
            return true;
    }

    return false;
}

struct smmb_info *smmb_phy_addr_alloc(unsigned long size, struct sec_memory_zone_select_parameter para,
                                      struct smmz_agent_allocator *allocator)
{
    mmz_t *selected_zone = NULL;
    struct smmb_info *mmb;
    struct smmz_agent_zone_selector *selector;

    if (!smmz_agent_allocator_inited(allocator)) {
        pr_err("%s:%d mmz_alloc is not initialized.\n", __func__, __LINE__);
        return NULL;
    }

    selector = allocator->zone_selector;
    if ((size == 0) || (size > g_max_malloc_size)) {
        pr_err("%s:%d requese size = %lu, parameters error\n", __func__, __LINE__, size);
        return NULL;
    }

    mmb = kzalloc(sizeof(*mmb), GFP_KERNEL);
    if (mmb == NULL)
        return NULL;

    (void)memset_s(mmb, sizeof(*mmb), 0, sizeof(*mmb));

    size = mmz_grain_align(size);

    selected_zone = selector->zone_select_func(para, selector);
    if (selected_zone == NULL) {
        kfree(mmb);
        return NULL;
    }

    if (allocator->allocator.mmb_alloc(size, selected_zone, &mmb->mmb) != 0) {
        kfree(mmb);
        return NULL;
    }

    kref_init(&mmb->ref);
    return mmb;
}

int smmb_get(struct smmb_info *mmb, struct smmz_agent_allocator *allocator)
{
    if (!smmz_agent_allocator_inited(allocator) || !smmb_allocated(mmb, allocator))
        return -EFAULT;

    kref_get(&mmb->ref);
    return 0;
}

void smmb_release(struct kref *kref)
{
    struct smmb_info *mmb = container_of(kref, struct smmb_info, ref);
    private_mmb_free(&mmb->mmb);
    kfree(mmb);
}

int smmb_put(struct smmb_info *mmb, struct smmz_agent_allocator *allocator)
{
    if (!smmz_agent_allocator_inited(allocator) || !smmb_allocated(mmb, allocator))
        return -EFAULT;

    kref_put(&mmb->ref, smmb_release);
    return 0;
}

int mmz_init(const ot_mmz_create_attr *mmz_create_attr, mmz_t *p)
{
    if ((mmz_create_attr == NULL) || p == NULL) {
        pr_err(KERN_ERR "%s: 'name' or mmz_attr can not be zero!", __FUNCTION__);
        return -ENOMEM;
    }

    (void)memset_s(p, sizeof(mmz_t), 0, sizeof(mmz_t));

    p->phys_start = mmz_create_attr->phys_start;
    p->nbytes = mmz_create_attr->nbytes;
    p->block_align = mmz_grain_align(mmz_create_attr->block_align);

    INIT_LIST_HEAD(&p->mmb_list);
    return 0;
}

int sec_mmz_init(const struct smmz_create_attr smmz_create_attr, struct sec_mmz_info *p)
{
    const ot_mmz_create_attr *mmz_create_attr = &smmz_create_attr.mmz_create_attr;
    pr_devel("func: %s, mmz init\n"
             "mmz name: %s\n"
             "mmz start: %ld\n"
             "mmz size: %ld\n"
             "mmz type: %s\n",
             __func__, smmz_create_attr.name, mmz_create_attr->phys_start, mmz_create_attr->nbytes, "SDP_SMMZ");

    if (strncpy_s(p->name, MMZ_NAME_LEN, smmz_create_attr.name, MMZ_NAME_LEN - 1) != EOK) {
        pr_err("%s - strncpy_s failed!\n", __FUNCTION__);
        return -ENOMEM;
    }

    pr_devel("mmz name: %s\n", p->name);
    INIT_LIST_HEAD(&p->list);
    spin_lock_init(&p->lock);

    if (mmz_init(&smmz_create_attr.mmz_create_attr, &p->mmz)) {
        return -ENOMEM;
    }

    list_add_tail(&p->list, &smmz_create_attr.zone_manager->sdp_smmz_list);
    return 0;
}
