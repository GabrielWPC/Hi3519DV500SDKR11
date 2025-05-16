/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "smmz_agent_misc_dev.h"

#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mman.h>
#include <linux/securec.h>

#include "linux/dma-buf.h"
#include "smmz_agent_dma_buf.h"
#include "smmz_agent_private.h"

static int smmb_alloc_fd(const char *name, ssize_t size)
{
    struct dma_buf *dma_buf;
    int ret;
    dma_buf = smmz_dmabuf_alloc(name, size);
    if (dma_buf == NULL) {
        pr_err("smmz agent alloc dmabuf error.\n");
        return -EFAULT;
    }

    ret = dma_buf_fd(dma_buf, O_CLOEXEC);
    if (ret < 0) {
        dma_buf_put(dma_buf);
        pr_err("smmz agent alloc dmabuf error.\n");
        return ret;
    }
    return ret;
}

static int smmb_alloc_fd_npuclass(int npuclass, ssize_t size)
{
    char *mmz_name = smmz_agent_name_map(npuclass);
    pr_devel("func %s: mmz name is %s.\n", __func__, mmz_name);
    if (mmz_name)
        return smmb_alloc_fd(mmz_name, size);
    return -1;
}

static ssize_t smmb_ownership_get_phy_start(int buf_fd)
{
    int buf_id = get_smmb_buf_id(buf_fd);
    struct smmb_ownership_info *ownership = get_ownershipinfo(buf_id);

    if (ownership == NULL)
        return 0;

    return ownership->smmb->mmb.phys_addr;
}

ssize_t smmb_ownership_get_size(int buf_fd)
{
    int buf_id = get_smmb_buf_id(buf_fd);
    struct smmb_ownership_info *ownership = get_ownershipinfo(buf_id);
    if (ownership == NULL)
        return 0;

    return ownership->smmb->mmb.length;
}

long smmz_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct smmz_agent_args args = { 0 };
    int ret;
    if (_IOC_TYPE(cmd) == 'm') {
        pr_devel("kernel:%d,\n", _IOC_SIZE(cmd));

        if ((_IOC_SIZE(cmd) > sizeof(args)) || (arg == 0)) {
            pr_err("_IOC_SIZE(cmd)=%d\n", _IOC_SIZE(cmd));
            return -EINVAL;
        }

        if (copy_from_user(&args, (void *)(uintptr_t)arg, _IOC_SIZE(cmd))) {
            pr_err("\nmmz_userdev_ioctl: copy_from_user error.\n");
            return -EFAULT;
        }

        switch (_IOC_NR(cmd)) {
            case _IOC_NR(IOC_SMMZ_AGENT_ALLOC_SMMZ):
                ret = smmb_alloc_fd(args.mmz_name, args.size);
                if (ret < 0) {
                    pr_err("smmb alloc error\n");
                    return -EFAULT;
                }
                return ret;

            case _IOC_NR(IOC_SMMZ_AGENT_ALLOC_SMMZ_NPUCLASS):
                ret = smmb_alloc_fd_npuclass(args.npuclass, args.size);
                if (ret < 0) {
                    pr_err("smmb alloc error\n");
                    return -EFAULT;
                }
                return ret;

            case _IOC_NR(IOC_SMMZ_AGENT_GET_SIZE):
                args.size = smmb_ownership_get_size(args.buf_fd);
                if (args.size != 0) {
                    if (copy_to_user((void *)(uintptr_t)arg, &args, _IOC_SIZE(cmd)))
                        return -EFAULT;
                } else {
                    return -EFAULT;
                }
                break;

            case _IOC_NR(IOC_SMMZ_AGENT_GET_PHY_ADDR):
                args.phy_addr = smmb_ownership_get_phy_start(args.buf_fd);
                if (args.phy_addr != 0) {
                    if (copy_to_user((void *)(uintptr_t)arg, &args, _IOC_SIZE(cmd))) {
                        pr_err("smmz agent copy to user error.\n");
                        return -EFAULT;
                    }
                } else {
                    return -EFAULT;
                }
                break;
            default:
                return -EFAULT;
        }
        return 0;
    }

    return -EINVAL;
}

static const struct file_operations g_smmz_agent_userdev_fops = {
    .owner = THIS_MODULE,
#ifdef CONFIG_COMPAT
    .compat_ioctl = smmz_ioctl,
#endif
    .unlocked_ioctl = smmz_ioctl,
};

static struct miscdevice g_smmz_agent_userdev = { .minor = MISC_DYNAMIC_MINOR,
                                                  .fops = &g_smmz_agent_userdev_fops,
                                                  .name = "smmz_agent_userdev" };

int smmz_agent_userdev_init(void)
{
    int ret;
    ret = misc_register(&g_smmz_agent_userdev);
    if (ret) {
        pr_err("register mmz dev failure!\n");
        return -1;
    }

    return 0;
}

void smmz_agent_userdev_deinit(void)
{
    misc_deregister(&g_smmz_agent_userdev);
}
