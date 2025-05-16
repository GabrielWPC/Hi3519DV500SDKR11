/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#ifndef _SMMZ_AGENT_DMA_BUF_H
#define _SMMZ_AGENT_DMA_BUF_H

#include "linux/types.h"

struct dma_buf *smmz_dmabuf_alloc(const char *mmz_name, ssize_t size);
int get_smmb_buf_id(int fd);

#endif  // !_SMMZ_AGENT_DMA_BUF_H
