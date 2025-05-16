/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>

#include "ot_osal.h"
#include "ot_sample_ist_mod_init.h"

static int sample_ist_module_init(void)
{
    return sample_ist_mod_init();
}

static void sample_ist_module_exit(void)
{
    sample_ist_mod_exit();
}

module_init(sample_ist_module_init);
module_exit(sample_ist_module_exit);

MODULE_DESCRIPTION("sample of isp sync task Driver");
MODULE_LICENSE("GPL");
