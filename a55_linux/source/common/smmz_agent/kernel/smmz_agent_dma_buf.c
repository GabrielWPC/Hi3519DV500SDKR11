/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <linux/dma-buf.h>
#include <linux/slab.h>

#include "linux/err.h"
#include "smmz_agent_private.h"

static void smmz_buf_release(struct dma_buf *dmabuf)
{
    int buf_id;
    if (IS_ERR_OR_NULL(dmabuf)) {
        pr_err("Resource leakage may occur in the smmz_agent.\n");
        return;
    }
    buf_id = (int)(long)dmabuf->priv;
    ownership_release(buf_id);
}

struct sg_table *smmb_map_dma_buf(struct dma_buf_attachment *attachment, enum dma_data_direction direction)
{
    return NULL;
}

void smmb_unmap_dma_buf(struct dma_buf_attachment *attachment, struct sg_table *sgt, enum dma_data_direction direction)
{
    return;
}

static const struct dma_buf_ops smmz_dma_buf_ops = {
    .map_dma_buf = smmb_map_dma_buf,
    .unmap_dma_buf = smmb_unmap_dma_buf,
    .release = smmz_buf_release,
};

struct dma_buf *smmz_dmabuf_alloc(const char *mmz_name, ssize_t size)
{
    struct dma_buf *dmabuf;
    int buf_id;
    DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

    buf_id = smmb_alloc_ownership(mmz_name, size);
    if (buf_id == -1) {
        pr_err("smmz alloc error\n");
        return NULL;
    }

    exp_info.ops = &smmz_dma_buf_ops;
    exp_info.size = size;
    exp_info.flags = O_CLOEXEC;
    exp_info.priv = (void *)(long)buf_id;

    dmabuf = dma_buf_export(&exp_info);
    if (IS_ERR_OR_NULL(dmabuf)) {
        ownership_release(buf_id);
        return NULL;
    }

    return dmabuf;
}

int get_smmb_buf_id(int fd)
{
    struct dma_buf *dma_buf = dma_buf_get(fd);
    int ret;

    if (IS_ERR_OR_NULL(dma_buf)) {
        pr_err("error: dma_buf get error.\n");
        return -1;
    }

    ret = (int)(long)dma_buf->priv;
    dma_buf_put(dma_buf);
    return ret;
}
