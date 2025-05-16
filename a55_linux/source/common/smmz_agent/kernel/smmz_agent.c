/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_dma.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/securec.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include "smmz_agent_export.h"
#include "smmz_agent_misc_dev.h"
#include "smmz_agent_phy_buffer_allocator.h"
#include "smmz_agent_private.h"

struct smmz_agent_driver_data g_driver_data;
DEFINE_MUTEX(g_driver_mutex);

static void smmz_agent_driver_data_lock(void)
{
    mutex_lock(&g_driver_mutex);
}

static void smmz_agent_driver_data_unlock(void)
{
    mutex_unlock(&g_driver_mutex);
}

static void ownership_list_lock(struct smmz_agent_ownership_list *list)
{
    mutex_lock(&list->mutex);
}

static void ownership_list_unlock(struct smmz_agent_ownership_list *list)
{
    mutex_unlock(&list->mutex);
}

struct smmb_ownership_info *private_get_ownershipinfo(struct smmz_agent_ownership_list *list, int buf_id)
{
    return idr_find(&list->ownership_ids, buf_id);
}

struct ownership_creat_args {
    struct smmb_info *sec_mmb;
};

static int ownership_info_init(struct smmb_ownership_info *gen, const struct ownership_creat_args *args,
                               struct smmz_agent_ownership_list *ownership_list)
{
    int ret;

    if (args->sec_mmb == NULL)
        return -1;

    gen->smmb = args->sec_mmb;

    ownership_list_lock(ownership_list);
    ret = idr_alloc(&ownership_list->ownership_ids, gen, 1, 0, GFP_KERNEL);
    if (ret <= 0) {
        ownership_list_unlock(ownership_list);
        return -1;
    }
    ownership_list_unlock(ownership_list);

    gen->buf_id = ret;
    return 0;
}

void ownership_release(int buf_id)
{
    struct smmz_agent_allocator *allocator;
    struct smmz_agent_ownership_list *list;
    struct smmb_ownership_info *ownership = get_ownershipinfo(buf_id);

    smmz_agent_driver_data_lock();
    allocator = g_driver_data.allocator;
    list = &g_driver_data.ownership_list;
    smmz_agent_driver_data_unlock();

    ownership_list_lock(list);
    idr_remove(&list->ownership_ids, buf_id);
    ownership_list_unlock(list);

    if (IS_ERR_OR_NULL(ownership)) {
        // Media memory leaks are better than system crashes.
        pr_err("Resource leakage may occur in the smmz_agent.\n");
        return;
    }
    smmb_put(ownership->smmb, allocator);
    kfree(ownership);
}

static void sec_mmb_ownership_list_init(struct smmz_agent_ownership_list *list)
{
    mutex_init(&list->mutex);
    idr_init(&list->ownership_ids);
}

static bool mmz_match(struct sec_mmz_info *mmz, struct sec_memory_zone_select_parameter para)
{
    if (strncmp(mmz->name, para.zone_name, MMZ_NAME_LEN)) {
        pr_err("%s, %s\n", mmz->name, para.zone_name);
        return false;
    }

    return true;
}

static mmz_t *sec_mmz_zone_selector_func(struct sec_memory_zone_select_parameter para,
                                         struct smmz_agent_zone_selector *selector)
{
    struct list_head *selected_list;
    struct sec_mmz_info *mmz;
    struct smmz_agent_zone_manager *manager;

    if (selector == NULL) {
        pr_err(KERN_ERR "%s error parameters\n", __func__);
        return NULL;
    }

    manager = container_of(selector, typeof(*manager), selector);
    selected_list = &manager->sdp_smmz_list;

    list_for_each_entry(mmz, selected_list, list)
    {
        if (mmz_match(mmz, para))
            return &mmz->mmz;
    }

    return NULL;
}

static void smmz_agent_zone_manager_init(struct smmz_agent_zone_manager *zone_manager)
{
    INIT_LIST_HEAD(&zone_manager->sdp_smmz_list);

    zone_manager->selector.zone_select_func = sec_mmz_zone_selector_func;
}

void smmz_agent_driver_data_init(struct smmz_agent_driver_data *drv_data)
{
    sec_mmb_ownership_list_init(&drv_data->ownership_list);
    smmz_agent_zone_manager_init(&drv_data->zone_manager);
}

static int add_to_mmz_name_map(struct smmz_agent_driver_data *drv_data, int npuclass, const char *name)
{
    if (strncpy_s(drv_data->mmz_name_map[npuclass], MMZ_NAME_LEN, name, strlen(name)) != EOK)
        return -1;

    return 0;
}

char *smmz_agent_name_map(int npuclass)
{
    if (npuclass >= NPU_CLASS_NUM_MAX || npuclass < 0) {
        pr_err("dts error: npuclass num %d >= NPU_CLASS_NUM_MAX\n", npuclass);
        return NULL;
    }

    if (strnlen(g_driver_data.mmz_name_map[npuclass], MMZ_NAME_LEN) == 0) {
        pr_err("args error: can not find match class for npuclass %d\n", npuclass);
        return NULL;
    }

    pr_devel("class %d: %s\n", npuclass, g_driver_data.mmz_name_map[npuclass]);
    return g_driver_data.mmz_name_map[npuclass];
}

static int smmz_agent_of_probe(struct smmz_agent_driver_data *drv_data, struct platform_device *dev)
{
    const char *zone_name, *secure_type;
    struct resource *res;
    const struct device_node *np;
    struct smmz_agent_zone_manager *zone_manager;
    phys_addr_t start, size;
    struct smmz_create_attr create_attr = { 0 };
    ot_mmz_create_attr *mmz_create_attr = &create_attr.mmz_create_attr;
    struct sec_mmz_info *mmz;
    u32 npuclass;

    if (drv_data == NULL || dev == NULL) {
        pr_err("%s - parameters error!\n", __FUNCTION__);
        return -ENOMEM;
    }

    np = dev->dev.of_node;
    zone_manager = &drv_data->zone_manager;

    if (of_property_read_string(np, "zone-name", &zone_name)) {
        pr_err("%s:%d - dts parsing failed, no mmz is created!\n", __FUNCTION__, __LINE__);
        return -ENODEV;
    }

    if (of_property_read_u32_index(np, "npuclass-num", 0, &npuclass) == 0) {
        if (add_to_mmz_name_map(drv_data, npuclass, zone_name) != 0) {
            pr_err("%s:%d - dts parsing failed, add to mmz name map failed!\n", __FUNCTION__, __LINE__);
            return -ENODEV;
        }
    }

    res = platform_get_resource(dev, IORESOURCE_MEM, 0);
    if (res == NULL) {
        pr_err("%s:%d - dts parsing failed, no mmz is created!\n", __FUNCTION__, __LINE__);
        return -ENODEV;
    }

    start = res->start;
    size = res->end - res->start + 1;

    if (of_property_read_string(np, "secure-node", &secure_type)) {
        pr_err("%s:%d - dts parsing failed, no mmz is created!\n", __FUNCTION__, __LINE__);
        return -ENODEV;
    }

    if (strncmp(secure_type, "okay", sizeof("okay")) != 0) {
        return -ENODEV;
    }

    create_attr.name = zone_name;
    create_attr.zone_manager = zone_manager;

    mmz_create_attr->block_align = MMZ_GRAIN;
    mmz_create_attr->phys_start = start;
    mmz_create_attr->nbytes = size;

    mmz = kzalloc(sizeof(*mmz), GFP_KERNEL);
    if (mmz == NULL) {
        return -ENODEV;
    }

    if (sec_mmz_init(create_attr, mmz)) {
        pr_err("%s:%d driver init error!!!\n", __FUNCTION__, __LINE__);
        kfree(mmz);
        return -ENODEV;
    }

    return 0;
}

static int __init smmz_agent_probe(struct platform_device *pdev)
{
    if (pdev == NULL) {
        pr_err("%s:%d: probe error.\n", __func__, __LINE__);
        return -ENOMEM;
    }

    if (smmz_agent_of_probe(&g_driver_data, pdev)) {
        pr_err("%s:%d: zone probe error.\n", __func__, __LINE__);
        return -ENOMEM;
    }

    return 0;
}

static int smmz_agent_remove(struct platform_device *pdev)
{
    return 0;
}

int smmb_alloc_ownership(const char *mmz_name, ssize_t size)
{
    struct smmz_agent_ownership_list *ownership_list;
    struct sec_memory_zone_select_parameter zone_select_para = { 0 };
    struct ownership_creat_args args = { 0 };
    struct smmb_ownership_info *gen;
    struct smmz_agent_allocator *allocator;

    allocator = g_driver_data.allocator;
    ownership_list = &g_driver_data.ownership_list;

    if (strncpy_s(zone_select_para.zone_name, sizeof(zone_select_para.zone_name), mmz_name, strlen(mmz_name)) != EOK) {
        pr_err("func %s: copy zone name error.\n", __func__);
        return -1;
    }

    args.sec_mmb = smmb_phy_addr_alloc(size, zone_select_para, allocator);
    if (args.sec_mmb == NULL) {
        pr_err("func %s: allocate error.\n", __func__);
        return -1;
    }

    gen = kzalloc(sizeof(*gen), GFP_KERNEL);
    if (gen == NULL) {
        pr_err("func %s parameters error.\n", __func__);
        smmb_put(args.sec_mmb, allocator);
        return -1;
    }

    if (ownership_info_init(gen, &args, ownership_list) != 0) {
        pr_err("func %s ownership_info init error.\n", __func__);
        smmb_put(args.sec_mmb, allocator);
        kfree(gen);
        return -1;
    }

    return gen->buf_id;
}

phys_addr_t smmb_phyaddr(int buf_fd, pid_t pid)
{
    struct smmz_agent_ownership_list *ownership_list;
    struct smmb_ownership_info *ownership;
    int buf_id = get_smmb_buf_id(buf_fd);
    pr_devel("func %s: buf_id %d pid %d\n", __func__, buf_id, pid);

    ownership_list = &g_driver_data.ownership_list;
    ownership = private_get_ownershipinfo(ownership_list, buf_id);
    if (ownership == NULL) {
        pr_err("func %s:%d: The buf does not belong to this process.\n", __func__, __LINE__);
        return 0;
    }

    return ownership->smmb->mmb.phys_addr;
}

struct smmb_ownership_info *get_ownershipinfo(int buf_id)
{
    struct smmz_agent_ownership_list *ownership_list;
    struct smmb_ownership_info *ownership;

    ownership_list = &g_driver_data.ownership_list;
    ownership = private_get_ownershipinfo(ownership_list, buf_id);
    if (ownership == NULL) {
        pr_err("func %s:%d: The buf does not belong to this process.\n", __func__, __LINE__);
        return NULL;
    }
    return ownership;
}

static const struct of_device_id smmz_agent_match[] = {
    { .compatible = "vendor,memory_zone" },
    {},
};

static struct platform_driver smmz_agent_driver = {
    .remove = smmz_agent_remove,
    .driver =
        {
            .name = "smmz_agent",
            .of_match_table = smmz_agent_match,
        },
};

int __init smmz_agent_driver_init(void)
{
    struct smmz_agent_allocator *allocator;
    int ret = 0;

    memset_s(&g_driver_data, sizeof(g_driver_data), 0, sizeof(g_driver_data));
    smmz_agent_driver_data_init(&g_driver_data);

    allocator = kzalloc(sizeof(*allocator), GFP_KERNEL);
    if (allocator == NULL) {
        return -ENOMEM;
    }

    if (smmz_agent_allocator_init(&g_driver_data.zone_manager.selector, allocator)) {
        kfree(allocator);
        return -ENOMEM;
    }

    g_driver_data.allocator = allocator;

    ret = platform_driver_probe(&smmz_agent_driver, smmz_agent_probe);
    if (ret) {
        kfree(allocator);
        g_driver_data.allocator = NULL;
        return ret;
    }

    smmz_agent_userdev_init();
    return 0;
}

void __exit smmz_agent_driver_exit(void)
{
    struct smmz_agent_allocator *allocator = g_driver_data.allocator;
    struct sec_mmz_info *mmz, *tmp;
    struct list_head *mmz_list = &g_driver_data.zone_manager.sdp_smmz_list;
    smmz_agent_userdev_deinit();

    platform_driver_unregister(&smmz_agent_driver);

    list_for_each_entry_safe(mmz, tmp, mmz_list, list)
    {
        if (mmz != NULL)
            kfree(mmz);
    }

    if (allocator != NULL)
        kfree(allocator);
}
