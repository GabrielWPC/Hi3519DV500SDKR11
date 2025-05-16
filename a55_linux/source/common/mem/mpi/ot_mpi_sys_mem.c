/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_mpi_sys_mem.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "securec.h"
#include "ot_type.h"
#include "mmz.h"
#include "ot_common_sys_mem.h"
#include "ot_mpi_sys_mem_inner.h"
#include "mkp_mem.h"

static td_s32 g_mem_fd = -1;        /* mem fd for non-cache */
static td_s32 g_mem_cache_fd = -1;  /* mem fd for cache */

static pthread_mutex_t g_mem_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_mem_fd_mutex = PTHREAD_MUTEX_INITIALIZER;

#define OT_PAGE_SHIFT   12
#define OT_PAGE_SIZE    (1UL << OT_PAGE_SHIFT)
#define OT_PAGE_MASK    (~(OT_PAGE_SIZE - 1))

#define mem_mutex_lock()        (void)pthread_mutex_lock(&g_mem_mutex)
#define mem_mutex_unlock()      (void)pthread_mutex_unlock(&g_mem_mutex)
#define mem_fd_mutex_lock()     (void)pthread_mutex_lock(&g_mem_fd_mutex)
#define mem_fd_mutex_unlock()   (void)pthread_mutex_unlock(&g_mem_fd_mutex)

#define mem_check_null_ptr_return(ptr)                          \
    do {                                                        \
        if ((ptr) == NULL) {                                    \
            printf("func: %s null pointer!\n", __FUNCTION__);   \
            return OT_ERR_MEM_NULL_PTR;                         \
        }                                                       \
    } while (0)

#define mem_check_mmz_userdev_open_return()     \
    do {                                        \
        td_s32 ret_;                            \
        ret_ = mem_check_mmz_userdev_open();    \
        if (ret_ != TD_SUCCESS) {               \
            return ret_;                        \
        }                                       \
    } while (0)

static td_s32 mem_check_mmz_userdev_open(td_void)
{
    mem_fd_mutex_lock();
    if (g_mem_fd < 0) {
        g_mem_fd = open("/dev/mmz_userdev", O_RDWR | O_SYNC);
        if (g_mem_fd < 0) {
            mem_fd_mutex_unlock();
            perror("open mmz_userdev");
            return OT_ERR_MEM_NOT_READY;
        }
    }
    mem_fd_mutex_unlock();
    return TD_SUCCESS;
}

static td_s32 mem_check_mmz_userdev_cache_open(td_void)
{
    mem_fd_mutex_lock();
    if (g_mem_cache_fd < 0) {
        g_mem_cache_fd = open("/dev/mmz_userdev", O_RDWR);
        if (g_mem_cache_fd < 0) {
            mem_fd_mutex_unlock();
            perror("open mmz_userdev");
            return OT_ERR_MEM_NOT_READY;
        }
    }
    mem_fd_mutex_unlock();
    return TD_SUCCESS;
}

td_void *ot_mpi_sys_mmap(td_phys_addr_t phys_addr, td_u32 size)
{
    td_u32 diff;
    td_phys_addr_t page_phys;
    td_u8 *page_addr = TD_NULL;
    td_ulong page_size;

    /*
     * page_size will be 0 when u32size is 0 and diff is 0,
     * and then mmap will be error (error: invalid argument)
     */
    if ((size == 0) || (size >= 0xFFFFF000)) {
        printf("func: %s size should be in (0, 0xFFFFF000).\n", __FUNCTION__);
        return TD_NULL;
    }

    if (mem_check_mmz_userdev_open() != TD_SUCCESS) {
        return TD_NULL;
    }

    /* the mmap address should align with page */
    page_phys = phys_addr & OT_PAGE_MASK;
    /* the mmap size should be multiples of PAGE_SIZE */
    diff = (td_u32)(phys_addr & (OT_PAGE_SIZE - 1));
    page_size = ((size + diff - 1) & OT_PAGE_MASK) + OT_PAGE_SIZE;

    mem_mutex_lock();
#ifdef __LITEOS__
    page_addr = (td_u8 *)ioremap_nocache(page_phys, page_size);
#else
    page_addr = mmap((void *)0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mem_fd, (td_ulong)page_phys);
    if (page_addr == MAP_FAILED) {
        mem_mutex_unlock();
        perror("mmap error");
        return TD_NULL;
    }
#endif
    mem_mutex_unlock();
    return (td_void *)(page_addr + diff);
}

td_void *ot_mpi_sys_mmap_cached(td_phys_addr_t phys_addr, td_u32 size)
{
    td_u32 diff;
    td_phys_addr_t page_phys;
    td_ulong page_size;
    td_u8 *page_addr = TD_NULL;

    /*
     * page_size will be 0 when u32size is 0 and diff is 0,
     * and then mmap will be error (error: invalid argument)
     */
    if ((size == 0) || (size >= 0xFFFFF000)) {
        printf("func: %s size should be in (0, 0xFFFFF000).\n", __FUNCTION__);
        return TD_NULL;
    }

    if (mem_check_mmz_userdev_cache_open() != TD_SUCCESS) {
        return TD_NULL;
    }

    /* the mmap address should align with page */
    page_phys = phys_addr & OT_PAGE_MASK;
    /* the mmap size should be multiples of PAGE_SIZE */
    diff = (td_u32)(phys_addr & (OT_PAGE_SIZE - 1));
    page_size = ((size + diff - 1) & OT_PAGE_MASK) + OT_PAGE_SIZE;

    mem_mutex_lock();
#ifdef __LITEOS__
    page_addr = (td_u8 *)ioremap_cached(page_phys, page_size);
#else
    page_addr = mmap((void *)0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_mem_cache_fd, (td_ulong)page_phys);
    if (page_addr == MAP_FAILED) {
        mem_mutex_unlock();
        perror("mmap error");
        return TD_NULL;
    }
#endif
    mem_mutex_unlock();
    return (td_void *)(page_addr + diff);
}

td_s32 ot_mpi_sys_munmap(const td_void *virt_addr, td_u32 size)
{
    td_phys_addr_t page_addr;
    td_u32 page_size;
    td_u32 diff;

    mem_check_null_ptr_return(virt_addr);

    page_addr = (td_phys_addr_t)(((td_uintptr_t)virt_addr) & OT_PAGE_MASK);
    diff = (td_uintptr_t)virt_addr & (OT_PAGE_SIZE - 1);
    page_size = ((size + diff - 1) & OT_PAGE_MASK) + OT_PAGE_SIZE;
    return munmap((td_void *)(td_uintptr_t)page_addr, page_size);
}

/* alloc mmz memory in user context */
td_s32 ot_mpi_sys_mmz_alloc(td_phys_addr_t *phys_addr, td_void **virt_addr,
    const td_char *mmb, const td_char *zone, td_u32 len)
{
    td_s32 ret;
    td_void *mapped = TD_NULL;

    mem_check_null_ptr_return(phys_addr);
    mem_check_null_ptr_return(virt_addr);

    ret = ot_mpi_sys_mmz_alloc_only(phys_addr, zone, mmb, len);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mapped = ot_mpi_sys_mmz_remap_nocache(*phys_addr, len);
    if (mapped == TD_NULL) {
        (td_void)ot_mpi_sys_mmz_free_only(*phys_addr, TD_NULL);
        return OT_ERR_MEM_BUSY;
    }

    *virt_addr = mapped;
    return TD_SUCCESS;
}

/* alloc mmz memory with cache in user context */
td_s32 ot_mpi_sys_mmz_alloc_cached(td_phys_addr_t *phys_addr, td_void **virt_addr,
    const td_char *mmb, const td_char *zone, td_u32 len)
{
    td_s32 ret;
    td_void *mapped = TD_NULL;

    mem_check_null_ptr_return(phys_addr);
    mem_check_null_ptr_return(virt_addr);

    ret = ot_mpi_sys_mmz_alloc_only(phys_addr, zone, mmb, len);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mapped = ot_mpi_sys_mmz_remap_cached(*phys_addr, len);
    if (mapped == TD_NULL) {
        (td_void)ot_mpi_sys_mmz_free_only(*phys_addr, TD_NULL);
        return OT_ERR_MEM_BUSY;
    }

    *virt_addr = mapped;
    return TD_SUCCESS;
}

/* free mmz memory in user context */
td_s32 ot_mpi_sys_mmz_free(td_phys_addr_t phys_addr, const td_void *virt_addr)
{
    struct mmb_info mmi = {0};
    td_s32 ret;
    ot_unused(virt_addr);

    mem_check_mmz_userdev_open_return();

    mmi.phys_addr = (td_ulong)phys_addr;

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_USER_UNMAP, &mmi);
    if (ret != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system unmap mmz memory failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    ret = ioctl(g_mem_fd, IOC_MMB_FREE, &mmi);
    if (ret != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system free mmz memory failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    mem_mutex_unlock();
    return TD_SUCCESS;
}

/* flush cache */
td_s32 ot_mpi_sys_flush_cache(td_phys_addr_t phys_addr, td_void *virt_addr, td_u32 size)
{
    struct mmb_info mmi = {0};
    td_phys_addr_t page_phys;
    td_u32 page_size;
    td_u32 diff;
    td_void *virt_page_addr = TD_NULL;

    mem_check_null_ptr_return(virt_addr);

    if ((size == 0) || (phys_addr == 0)) {
        mem_err_trace("size and addr can't be 0.\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }

    mem_check_mmz_userdev_open_return();

    /* the mmap address should align with cache line, 64B */
    page_phys = phys_addr & (~(CACHE_LINE_SIZE - 1));
    diff = (td_u32)(phys_addr - page_phys);
    virt_page_addr = (td_u8 *)virt_addr - diff;

    /* the mmap size should be multiples of cache line SIZE */
    page_size = ((size + diff - 1) & (~(CACHE_LINE_SIZE - 1))) + CACHE_LINE_SIZE;

    mmi.phys_addr = page_phys;
    mmi.mapped = virt_page_addr;
    mmi.size = page_size;
    if (ioctl(g_mem_fd, IOC_MMB_SYS_FLUSH_CACHE, &mmi) != TD_SUCCESS) {
        mem_err_trace("mmb flush cache failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

/* get virtual meminfo according to virtual addr, should be in one process */
td_s32 ot_mpi_sys_get_virt_mem_info(const td_void *virt_addr, ot_sys_virt_mem_info *mem_info)
{
    struct mmb_info mmi = {0};
    td_s32 ret;

    mem_check_null_ptr_return(virt_addr);
    mem_check_null_ptr_return(mem_info);

    mem_check_mmz_userdev_open_return();

    mmi.mapped = (void *)virt_addr;

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_VIRT_GET_PHYS, &mmi);
    if (ret != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("Get virt mem info failed(ret:%d).\n", ret);
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    mem_mutex_unlock();

    mem_info->is_cached = mmi.phys_addr & 0x1;
    mem_info->phys_addr = mmi.phys_addr & (~0x1);
    return TD_SUCCESS;
}

td_s32 ot_mpi_sys_mmz_check_phy_addr(td_phys_addr_t phys_addr, td_ulong size)
{
    struct mmb_info mmi = {0};

    if ((size == 0) || (phys_addr == 0)) {
        mem_err_trace("size and addr can't be 0.\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }

    mem_check_mmz_userdev_open_return();

    mmi.phys_addr = phys_addr;
    mmi.size = size;
    if (ioctl(g_mem_fd, IOC_MMB_BASE_CHECK_ADDR, &mmi) != TD_SUCCESS) {
        mem_err_trace("system check mmz physical address failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

/* malloc mmz without mmap! */
td_s32 ot_mpi_sys_mmz_alloc_only(td_phys_addr_t *phys_addr, const td_char *mmz_name,
    const td_char *buf_name, td_ulong size)
{
    struct mmb_info mmi = {0};

    mem_check_null_ptr_return(phys_addr);
    mem_check_mmz_userdev_open_return();

    mmi.size = size;
    mmi.prot = PROT_READ | PROT_WRITE;
    mmi.flags = MAP_SHARED;
    if (buf_name != TD_NULL) {
        if (strncpy_s(mmi.mmb_name, sizeof(mmi.mmb_name), buf_name, sizeof(mmi.mmb_name) - 1) != EOK) {
            mem_err_trace("copy err!\n");
            return OT_ERR_MEM_ILLEGAL_PARAM;
        }
    }
    if (mmz_name != TD_NULL) {
        if (strncpy_s(mmi.mmz_name, sizeof(mmi.mmz_name), mmz_name, sizeof(mmi.mmz_name) - 1) != EOK) {
            mem_err_trace("copy err!\n");
            return OT_ERR_MEM_ILLEGAL_PARAM;
        }
    }

    mem_mutex_lock();
    if (ioctl(g_mem_fd, IOC_MMB_ALLOC, &mmi) != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system alloc mmz memory failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    mem_mutex_unlock();

    *phys_addr = mmi.phys_addr;
    return TD_SUCCESS;
}

td_s32 ot_mpi_sys_mmz_free_only(td_phys_addr_t phys_addr, const td_void *virt_addr)
{
    struct mmb_info mmi = {0};

    mem_check_mmz_userdev_open_return();

    mmi.phys_addr = (td_ulong)phys_addr;

    mem_mutex_lock();
    if (virt_addr != TD_NULL) {
        if (ioctl(g_mem_fd, IOC_MMB_USER_UNMAP, &mmi) != TD_SUCCESS) {
            mem_mutex_unlock();
            mem_err_trace("system unmap mmz memory failed!\n");
            return OT_ERR_MEM_ILLEGAL_PARAM;
        }
    }
    if (phys_addr != 0) {
        if (ioctl(g_mem_fd, IOC_MMB_FREE, &mmi) != TD_SUCCESS) {
            mem_mutex_unlock();
            mem_err_trace("system free mmz memory failed!\n");
            return OT_ERR_MEM_ILLEGAL_PARAM;
        }
    }
    mem_mutex_unlock();
    return TD_SUCCESS;
}

td_void *ot_mpi_sys_mmz_remap_nocache(td_phys_addr_t phys_addr, td_u32 size)
{
    struct mmb_info mmi = {0};
    td_s32 ret;

    if (mem_check_mmz_userdev_open() != TD_SUCCESS) {
        return TD_NULL;
    }

    mmi.size = size;
    mmi.phys_addr = phys_addr;
    mmi.prot = PROT_READ | PROT_WRITE;
    mmi.flags = MAP_SHARED;

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_USER_REMAP, &mmi);
    if (ret != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system remap mmz nocache failed!\n");
        return TD_NULL;
    }
    mem_mutex_unlock();

    return mmi.mapped;
}

td_void *ot_mpi_sys_mmz_remap_cached(td_phys_addr_t phys_addr, td_u32 size)
{
    struct mmb_info mmi = {0};
    td_s32 ret;

    if (mem_check_mmz_userdev_open() != TD_SUCCESS) {
        return TD_NULL;
    }

    mmi.size = size;
    mmi.phys_addr = phys_addr;
    mmi.prot = PROT_READ | PROT_WRITE;
    mmi.flags = MAP_SHARED;

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_USER_REMAP_CACHED, &mmi);
    if (ret != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system remap mmz cached failed!\n");
        return TD_NULL;
    }
    mem_mutex_unlock();

    return mmi.mapped;
}

td_s32 ot_mpi_sys_mmz_unmap(const td_void *virt_addr)
{
    struct mmb_info mmi = {0};

    mem_check_null_ptr_return(virt_addr);
    mem_check_mmz_userdev_open_return();

    mmi.mapped = (void *)virt_addr;

    mem_mutex_lock();
    if (ioctl(g_mem_fd, IOC_MMB_VIRT_GET_PHYS, &mmi) != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system get mmz phys_addr failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    if (ioctl(g_mem_fd, IOC_MMB_USER_UNMAP, &mmi) != TD_SUCCESS) {
        mem_mutex_unlock();
        mem_err_trace("system unmap mmz memory failed!\n");
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    mem_mutex_unlock();

    return TD_SUCCESS;
}

static td_s32 mpi_sys_mem_convert_mmz_errno(td_s32 mmz_errno)
{
    if (mmz_errno == 0) {
        return TD_SUCCESS;
    }
    /* errno is thread-local */
    if (errno == EPERM) {
        return OT_ERR_MEM_NOT_PERM;
    } else {
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
}

static td_s32 mpi_sys_mem_do_share(td_ulong cmd, const td_void *mem_handle, td_s32 pid, td_bool is_share_all)
{
    td_s32 ret;
    struct mmb_share_info info = {0};

    info.mem_handle = (void *)mem_handle;
    if (is_share_all == TD_FALSE) {
        info.shared_pid = pid;
    }

    mem_check_mmz_userdev_open_return();

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, cmd, &info);
    mem_mutex_unlock();
    return mpi_sys_mem_convert_mmz_errno(ret);
}

td_s32 ot_mpi_sys_mem_share(const td_void *mem_handle, td_s32 pid)
{
    td_s32 ret;

    mem_check_null_ptr_return(mem_handle);

    if (pid < 0) {
        mem_err_trace("invalid pid %d!\n", pid);
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    if (getpgid(pid) < 0) {
        mem_err_trace("the process (pid %d) does not exist!\n", pid);
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    ret = mpi_sys_mem_do_share(IOC_MMB_MEM_SHARE, mem_handle, pid, TD_FALSE);
    if (ret != TD_SUCCESS) {
        mem_err_trace("mem share failed!\n");
    }
    return ret;
}

td_s32 ot_mpi_sys_mem_unshare(const td_void *mem_handle, td_s32 pid)
{
    td_s32 ret;

    mem_check_null_ptr_return(mem_handle);

    if (pid < 0) {
        mem_err_trace("invalid pid %d!\n", pid);
        return OT_ERR_MEM_ILLEGAL_PARAM;
    }
    ret = mpi_sys_mem_do_share(IOC_MMB_MEM_UNSHARE, mem_handle, pid, TD_FALSE);
    if (ret != TD_SUCCESS) {
        mem_err_trace("mem unshare failed!\n");
    }
    return ret;
}

td_s32 ot_mpi_sys_mem_share_all(const td_void *mem_handle)
{
    td_s32 ret;

    mem_check_null_ptr_return(mem_handle);

    ret = mpi_sys_mem_do_share(IOC_MMB_MEM_SHARE_ALL, mem_handle, 0, TD_TRUE);
    if (ret != TD_SUCCESS) {
        mem_err_trace("mem share all failed!\n");
    }
    return ret;
}

td_s32 ot_mpi_sys_mem_unshare_all(const td_void *mem_handle)
{
    td_s32 ret;

    mem_check_null_ptr_return(mem_handle);

    ret = mpi_sys_mem_do_share(IOC_MMB_MEM_UNSHARE_ALL, mem_handle, 0, TD_TRUE);
    if (ret != TD_SUCCESS) {
        mem_err_trace("mem unshare all failed!\n");
    }
    return ret;
}

static td_void mpi_sys_mem_get_mem_info(const struct mmb_share_info *share_info, ot_sys_mem_info *mem_info)
{
    mem_info->phys_addr = share_info->phys_addr;
    mem_info->offset = share_info->offset;
    mem_info->mem_handle = share_info->mem_handle;
}

td_s32 ot_mpi_sys_get_mem_info_by_virt(const td_void *virt_addr, ot_sys_mem_info *mem_info)
{
    td_s32 ret;
    struct mmb_share_info info = {0};

    mem_check_null_ptr_return(virt_addr);
    mem_check_null_ptr_return(mem_info);

    info.virt_addr = (void *)virt_addr;

    mem_check_mmz_userdev_open_return();

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_VIRT_GET_SYS_MEM, &info);
    mem_mutex_unlock();
    ret = mpi_sys_mem_convert_mmz_errno(ret);
    if (ret != TD_SUCCESS) {
        mem_err_trace("get mem info by virt failed!\n");
        return ret;
    }
    mpi_sys_mem_get_mem_info(&info, mem_info);
    return TD_SUCCESS;
}

td_s32 ot_mpi_sys_get_mem_info_by_phys(td_phys_addr_t phys_addr, ot_sys_mem_info *mem_info)
{
    td_s32 ret;
    struct mmb_share_info info = {0};

    mem_check_null_ptr_return(mem_info);

    info.phys_addr = phys_addr;

    mem_check_mmz_userdev_open_return();

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_PHYS_GET_SYS_MEM, &info);
    mem_mutex_unlock();
    ret = mpi_sys_mem_convert_mmz_errno(ret);
    if (ret != TD_SUCCESS) {
        mem_err_trace("get mem info by phys failed!\n");
        return ret;
    }
    mpi_sys_mem_get_mem_info(&info, mem_info);
    return TD_SUCCESS;
}

td_s32 ot_mpi_sys_get_mem_info_by_handle(const td_void *mem_handle, ot_sys_mem_info *mem_info)
{
    td_s32 ret;
    struct mmb_share_info info = {0};

    mem_check_null_ptr_return(mem_handle);
    mem_check_null_ptr_return(mem_info);

    info.mem_handle = (void *)mem_handle;

    mem_check_mmz_userdev_open_return();

    mem_mutex_lock();
    ret = ioctl(g_mem_fd, IOC_MMB_HANDLE_GET_SYS_MEM, &info);
    mem_mutex_unlock();
    ret = mpi_sys_mem_convert_mmz_errno(ret);
    if (ret != TD_SUCCESS) {
        mem_err_trace("get mem info by handle failed!\n");
        return ret;
    }
    mpi_sys_mem_get_mem_info(&info, mem_info);
    return TD_SUCCESS;
}
