/*
 *
 * Copyright (c) 2012-2021 Hisilicon Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <unistd.h>
#include <stdint.h>
#include <linux/limits.h>

#include "securec.h"
#include "bsp_config.h"
#include "bsp_dbg.h"

#ifdef OS_LINUX
#include <fcntl.h>
#include <sys/mman.h>

typedef struct tag_mmap_node {
	unsigned long start_p;
	void *start_v;
	unsigned long length;
	unsigned long refcount;
	struct tag_mmap_node *next;
} tmmap_node_t;

tmmap_node_t *p_tmmap_node = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK (~(0xfff))

static int mem_fd = -1;

#endif


#ifdef LOGQUEUE
#include "bsp_message.h"
#endif

static void *check_mapped(unsigned long phy_addr, unsigned long size)
{
	tmmap_node_t *p_tmp = NULL;
	p_tmp = p_tmmap_node;
	while (p_tmp != NULL) {
		if ((phy_addr >= p_tmp->start_p) && ((phy_addr + size) <= (p_tmp->start_p + p_tmp->length))) {
			p_tmp->refcount++;   /* referrence count increase by 1  */
			return (void *)((uintptr_t)(p_tmp->start_v) + phy_addr - p_tmp->start_p);
		}
		p_tmp = p_tmp->next;
	}
	return NULL;
}

static void *do_map(unsigned long phy_addr, unsigned long size)
{
	const char dev[] = "/dev/mem";
	unsigned long phy_addr_in_page;
	unsigned long page_diff;
	unsigned long size_in_page;
	tmmap_node_t *p_tmp = NULL;
	tmmap_node_t *p_new = NULL;
	void *addr = NULL;

	/* if dev is not opened yet, open it */
	mem_fd = mem_fd < 0 ? open(dev, O_RDWR | O_SYNC) : mem_fd;
	if (mem_fd < 0) {
		write_log_error("memmap():open %s error!\n", dev);
		return NULL;
	}

	/* addr align in page_size(4K) */
	phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
	page_diff = phy_addr - phy_addr_in_page;

	/* size in page_size */
	size_in_page = ((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

	addr = mmap((void *)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phy_addr_in_page);
	if (addr == MAP_FAILED) {
		write_log_error("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
		return NULL;
	}

	/* add this mmap to MMAP Node */
	p_new = (tmmap_node_t *)malloc(sizeof(tmmap_node_t));
	if (p_new == NULL) {
		write_log_error("memmap():malloc new node failed!\n");
		munmap(addr, size_in_page);
		return NULL;
	}
	p_new->start_p = phy_addr_in_page;
	p_new->start_v = addr;
	p_new->length = size_in_page;
	p_new->refcount = 1;
	p_new->next = NULL;

	if (p_tmmap_node == NULL) {
		p_tmmap_node = p_new;
	} else {
		p_tmp = p_tmmap_node;
		while (p_tmp->next != NULL)
			p_tmp = p_tmp->next;
		p_tmp->next = p_new;
	}
	return (void *)((uintptr_t)addr + page_diff);
}

/* no need considering page_size of 4K */
void *memmap(unsigned long phy_addr, unsigned long size)
{
#ifndef OS_LINUX
	return (void *)phy_addr;
#else
	void *addr = NULL;
	if (size == 0) {
		write_log_error("memmap():size can't be zero!\n");
		return NULL;
	}

	/* check if the physical memory space have been mmaped */
	addr = check_mapped(phy_addr, size);
	if (addr != NULL)
		return addr;

	/* not mmaped yet */
	return do_map(phy_addr, size);

#endif
}

int memunmap(const void *addr_mapped)
{
	tmmap_node_t *p_pre = NULL;
	tmmap_node_t *p_tmp = NULL;

	if (p_tmmap_node == NULL) {
		write_log_error("memunmap(): address have not been mmaped!\n");
		return -1;
	}

	/* check if the physical memory space have been mmaped */
	p_tmp = p_tmmap_node;
	p_pre = p_tmmap_node;

	while (p_tmp != NULL) {
		if ((addr_mapped < p_tmp->start_v) ||
			(addr_mapped > (void*)((uintptr_t)(p_tmp->start_v) + p_tmp->length))) {
			p_pre = p_tmp;
			p_tmp = p_tmp->next;
			continue;
		}

		p_tmp->refcount--;   /* referrence count decrease by 1  */
		if (p_tmp->refcount > 0)
			return 0;

		write_log_info("memunmap(): map node will be remove!\n");
		/* delete this map node from pMMAPNode */
		if (p_tmp == p_tmmap_node)
			p_tmmap_node = NULL;
		else
			p_pre->next = p_tmp->next;
		/* munmap */
		if (munmap((void *)p_tmp->start_v, p_tmp->length) != 0)
			write_log_info("memunmap(): munmap failed!\n");
		free(p_tmp);
		return 0;
	}

	write_log_error("memunmap(): address have not been mmaped!\n");
	return -1;
}

