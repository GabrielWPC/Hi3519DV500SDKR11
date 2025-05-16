/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ot_mpi_sys_mem.h"
#include "ot_common_isp.h"
#include "isp_drv_vreg.h"
#include "isp_main.h"
#include "isp_vreg.h"

typedef struct {
    td_bool used;
    td_u64  size;
    td_u64  base_addr;
    td_u64  phy_addr;
    td_u64  virt_addr;
} vreg_args;

typedef struct {
    td_u64  phy_addr;
    td_void  *virt_addr;
} ot_vreg_addr;

typedef struct {
    ot_vreg_addr slave_time_addr;
    ot_vreg_addr slave_sync_addr;
    ot_vreg_addr slave_reg_addr[CAP_SLAVE_MAX_NUM];
    ot_vreg_addr isp_fe_reg_addr[OT_ISP_MAX_FE_PIPE_NUM];
    ot_vreg_addr vicap_ch_reg_addr[OT_ISP_MAX_PHY_PIPE_NUM];
    ot_vreg_addr isp_be_reg_addr[ISP_MAX_BE_NUM];
    ot_vreg_addr isp_hdr_reg_addr[ISP_MAX_BE_NUM];
    ot_vreg_addr viproc_reg_addr[ISP_MAX_BE_NUM];
    ot_vreg_addr isp_vreg_addr[OT_ISP_MAX_PIPE_NUM];
    ot_vreg_addr ae_vreg_addr[MAX_ALG_LIB_VREG_NUM];
    ot_vreg_addr awb_vreg_addr[MAX_ALG_LIB_VREG_NUM];
    ot_vreg_addr af_vreg_addr[MAX_ALG_LIB_VREG_NUM];
} ot_vreg;

static ot_vreg g_ot_vreg = {{ 0 }};

td_s32 g_vreg_fd[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = -1};

static td_s32 vreg_check_open(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    if (g_vreg_fd[vi_pipe] <= 0) {
        g_vreg_fd[vi_pipe] = open("/dev/isp_dev", O_RDONLY, S_IRUSR);
        if (g_vreg_fd[vi_pipe] < 0) {
            perror("open isp device error!\n");
            return TD_FAILURE;
        }
        ret = ioctl(g_vreg_fd[vi_pipe], VREG_DRV_FD, &vi_pipe);
        if (ret != TD_SUCCESS) {
            close(g_vreg_fd[vi_pipe]);
            g_vreg_fd[vi_pipe] = -1;
            return ret;
        }
    }
    return TD_SUCCESS;
}

td_void vreg_check_close(ot_vi_pipe vi_pipe)
{
    if (g_vreg_fd[vi_pipe] > 0) {
        td_s32 args = -1;
        if (ioctl(g_vreg_fd[vi_pipe], VREG_DRV_FD, &args)) {
            isp_err_trace("close isp pipe:%d set fd:%d error!\n", vi_pipe, g_vreg_fd[vi_pipe]);
        }
        close(g_vreg_fd[vi_pipe]);
        g_vreg_fd[vi_pipe] = -1;
    }
    return;
}

static td_s32 g_s_mem_dev = -1;

static td_void *isp_reg_io_mmap(td_u64 phy_addr, td_u32 size)
{
    td_u32 diff;
    td_u64 page_phy;
    td_u8 *page_addr = TD_NULL;
    td_ulong  page_size;

    if (g_s_mem_dev <= 0) {
        g_s_mem_dev = open("/dev/isp_dev", O_RDWR | O_SYNC);
        if (g_s_mem_dev < 0) {
            perror("Open dev/isp error");
            return TD_NULL;
        }
    }

    /*
     * PageSize will be 0 when u32size is 0 and diff is 0,
     * and then mmap will be error (error: Invalid argument)
     */
    if (!size) {
        isp_err_trace("size can't be 0. error\n");
        return TD_NULL;
    }
    /* The mmap address should align with page */
    page_phy = phy_addr & 0xfffffffffffff000ULL;
    diff     = phy_addr - page_phy;

    /* The mmap size should be multiples of 1024 */
    page_size = ((size + diff - 1) & 0xfffff000UL) + 0x1000;

    page_addr = mmap((void *)0, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_s_mem_dev, page_phy);
    if (page_addr == MAP_FAILED) {
        perror("mmap error");
        return TD_NULL;
    }

    return (td_void *)(page_addr + diff);
}

static inline td_bool isp_check_slave_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base(base_addr, SLAVE_REG_BASE, SLAVE_MODE_REG_END);
}

static inline td_bool isp_check_slave_mode_time_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base(base_addr, SLAVE_MODE_REG_BASE, SLAVE_MODE_REG_BASE);
}
static inline td_bool isp_check_slave_sync_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base(base_addr, SLAVE_MODE_REG_SYNC, SLAVE_MODE_REG_SYNC);
}

static inline td_bool isp_check_vicap_ch_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_vicap_ch_reg_base(base_addr), VICAP_CH_REG_BASE,
        isp_vicap_ch_reg_base(OT_ISP_MAX_PHY_PIPE_NUM));
}

static inline td_bool isp_check_isp_fe_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_isp_reg_base(base_addr), FE_REG_BASE,
        isp_fe_reg_base(OT_ISP_MAX_FE_PIPE_NUM));
}

static inline td_bool isp_check_isp_be_base(td_u32 base_addr)
{
    return (td_bool)(isp_check_reg_base(get_isp_reg_base(base_addr), BE_REG_BASE, BE_REG_END) &&
        ((get_isp_reg_base(base_addr)) == (isp_be_reg_base(isp_get_be_id(base_addr)))));
}

static inline td_bool isp_check_viproc_base(td_u32 base_addr)
{
    return (td_bool)(isp_check_reg_base_ex(get_viproc_reg_base(base_addr), VIPROC_REG_BASE, VIPROC_REG_MAP_END) &&
        ((get_viproc_reg_base(base_addr)) == (isp_viproc_reg_base(isp_get_viproc_id(base_addr)))));
}

static inline td_bool vreg_check_isp_vreg_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_isp_vreg_base(base_addr), ISP_VREG_BASE,
        isp_vir_reg_base(OT_ISP_MAX_PIPE_NUM));
}

static inline td_bool vreg_check_ae_vreg_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_3a_vreg_base(base_addr), AE_VREG_BASE, ae_lib_vreg_base(ALG_LIB_MAX_NUM));
}

static inline td_bool vreg_check_awb_vreg_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_3a_vreg_base(base_addr), AWB_VREG_BASE,
        awb_lib_vreg_base(ALG_LIB_MAX_NUM));
}

static inline td_bool vreg_check_af_vreg_base(td_u32 base_addr)
{
    return (td_bool)isp_check_reg_base_ex(get_3a_vreg_base(base_addr), AF_VREG_BASE, af_lib_vreg_base(ALG_LIB_MAX_NUM));
}

#define vreg_munmap_virtaddr(virt_addr, size) \
    do { \
        if ((virt_addr) != TD_NULL) { \
            ot_mpi_sys_munmap((virt_addr), (size)); \
        } \
    }while (0)

static ot_vreg_addr *vreg_match(td_u32 base_addr)
{
    if (vreg_check_isp_vreg_base(base_addr)) {
        return &g_ot_vreg.isp_vreg_addr[isp_get_vreg_id(base_addr)];
    }

    if (vreg_check_ae_vreg_base(base_addr)) {
        return &g_ot_vreg.ae_vreg_addr[isp_get_ae_id(base_addr)];
    }

    if (vreg_check_awb_vreg_base(base_addr)) {
        return &g_ot_vreg.awb_vreg_addr[isp_get_awb_id(base_addr)];
    }

    if (vreg_check_af_vreg_base(base_addr)) {
        return &g_ot_vreg.af_vreg_addr[isp_get_af_id(base_addr)];
    }

    return TD_NULL;
}

static inline td_u32 vreg_base_align(td_u32 base_addr)
{
    if (vreg_check_ae_vreg_base(base_addr)) {
        return (base_addr & 0xFFFFE000);
    } else if (vreg_check_awb_vreg_base(base_addr) || vreg_check_af_vreg_base(base_addr)) {
        return (base_addr & 0xFFFFF000);
    } else {
        return (base_addr & 0xFFFE0000);
    }
}

static inline td_u32 vreg_size_align(td_u32 size)
{
    return (ALG_LIB_VREG_SIZE * ((size + ALG_LIB_VREG_SIZE - 1) / ALG_LIB_VREG_SIZE));
}

td_s32 vreg_check_permission(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);

    ret = vreg_check_open(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("vreg[%d] open failed\n", vi_pipe);
        return ret;
    }

    ret = ioctl(g_vreg_fd[vi_pipe], VREG_DRV_CHECK_PERMISSION);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 vreg_init(ot_vi_pipe vi_pipe, td_u32 base_addr, td_u32 size)
{
    vreg_args vreg = { 0 };
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);

    ret = vreg_check_open(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("vreg[%d] open failed\n", vi_pipe);
        return ret;
    }

    if (base_addr != vreg_base_align(base_addr)) {
        isp_err_trace("base_addr align error\n");
        return TD_FAILURE;
    }

    /* malloc vreg's phyaddr in kernel */
    vreg.base_addr = (td_u64)vreg_base_align(base_addr);
    vreg.size = (td_u64)vreg_size_align(size);

    ret = ioctl(g_vreg_fd[vi_pipe], VREG_DRV_INIT, &vreg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 vreg_exit(ot_vi_pipe vi_pipe, td_u32 base_addr, td_u32 size)
{
    ot_vreg_addr *v_reg = TD_NULL;
    vreg_args vreg;
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);

    ret = vreg_check_open(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("vreg[%d] open failed\n", vi_pipe);
        return ret;
    }

    if (base_addr != vreg_base_align(base_addr)) {
        isp_err_trace("base_addr align error\n");
        return TD_FAILURE;
    }

    /* check base */
    v_reg = vreg_match(vreg_base_align(base_addr));
    if (v_reg == TD_NULL) {
        return TD_FAILURE;
    }

    if (v_reg->virt_addr != TD_NULL) {
        /* munmap virtaddr */
        vreg_munmap_virtaddr(v_reg->virt_addr, vreg_size_align(size));
        v_reg->virt_addr  = TD_NULL;
        v_reg->phy_addr = 0;
    }

    /* release the buf in the kernel */
    vreg.base_addr = (td_u64)vreg_base_align(base_addr);
    ret = ioctl(g_vreg_fd[vi_pipe], VREG_DRV_EXIT, &vreg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void vreg_fe_munmap(td_u32 base_addr, td_u32 size)
{
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_isp_fe_base(base_addr)) {
        isp_err_trace("get fe_reg base vir addr error:0x%x\n", base_addr);
        return;
    }

    v_reg = &g_ot_vreg.isp_fe_reg_addr[isp_get_fe_id(base_addr)];
    if (v_reg == TD_NULL) {
        // not find addr, means not mapped, no need to unmap
        return;
    }

    if (v_reg->virt_addr != TD_NULL) {
        /* munmap virtaddr */
        vreg_munmap_virtaddr(v_reg->virt_addr, vreg_size_align(size));
        v_reg->virt_addr = TD_NULL;
        v_reg->phy_addr = 0;
    }

    return;
}

static td_void vreg_vicap_chn_munmap(td_u32 base_addr, td_u32 size)
{
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_vicap_ch_base(base_addr)) {
        isp_err_trace("get vicap_chn_reg base vir addr error:0x%x\n", base_addr);
        return;
    }

    v_reg = &g_ot_vreg.vicap_ch_reg_addr[isp_get_vicap_ch_id(base_addr)];
    if (v_reg == TD_NULL) {
        // not find addr, means not mapped, no need to unmap
        return;
    }

    if (v_reg->virt_addr != TD_NULL) {
        /* munmap virtaddr */
        vreg_munmap_virtaddr(v_reg->virt_addr, vreg_size_align(size));
        v_reg->phy_addr = 0;
        v_reg->virt_addr = TD_NULL;
    }

    return;
}

td_void vreg_munmap_reg_specific(ot_vi_pipe vi_pipe)
{
    /* just unmap fe and chn reg_base_addr, other reg base addrs are not related to the pipe id when they are mapped */
    /* The logic register address of E is not mapped */
    /* to prevent non-mutual errors when one pipe is unmapping while another pipe is accessing the virtual address */
    /* other reg base addr wait the system to recycle. */
    if (vi_pipe >= 0 && vi_pipe < OT_ISP_MAX_FE_PIPE_NUM) {
        vreg_fe_munmap(isp_fe_reg_base(vi_pipe), FE_REG_SIZE_ALIGN);
    }
    if (vi_pipe >= 0 && vi_pipe < OT_ISP_MAX_PHY_PIPE_NUM) {
        vreg_vicap_chn_munmap(isp_vicap_ch_reg_base(vi_pipe), VICAP_CH_REG_SIZE_ALIGN);
    }
}

td_s32 vreg_release_all(ot_vi_pipe vi_pipe)
{
    vreg_args vreg = { 0 };
    td_s32 ret = TD_SUCCESS;

    isp_check_pipe_return(vi_pipe);

    ret = vreg_check_open(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("vreg[%d] open failed\n", vi_pipe);
        return ret;
    }

    vreg.base_addr = ISP_VREG_BASE;
    vreg.size      = ISP_VREG_SIZE;

    /* release all buf in the kernel */
    ret = ioctl(g_vreg_fd[vi_pipe], VREG_DRV_RELEASE_ALL, &vreg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg_munmap_reg_specific(vi_pipe);

    return TD_SUCCESS;
}

static td_s32 isp_virt_vreg_map(td_u32 base_addr, ot_vreg_addr *v_reg)
{
    vreg_args vreg;
    td_u32 base, size;
    td_s32 ret = TD_SUCCESS;

    ret = vreg_check_open(0);
    if (ret != TD_SUCCESS) {
        isp_err_trace("vreg[%d] open failed\n", 0);
        return ret;
    }

    if (vreg_check_isp_vreg_base(base_addr)) {
        base = get_isp_vreg_base(base_addr);
        size = ISP_VREG_SIZE;
    } else if (vreg_check_ae_vreg_base(base_addr)) {
        base = get_ae_id_vreg_base(base_addr);
        size = AE_VREG_SIZE;
    } else if (vreg_check_awb_vreg_base(base_addr)) {
        base = get_awb_id_vreg_base(base_addr);
        size = AWB_VREG_SIZE;
    } else {
        base = get_af_id_vreg_base(base_addr);
        size = ALG_LIB_VREG_SIZE;
    }

    vreg.base_addr = (td_u64)base;
    ret = ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    v_reg->phy_addr = vreg.phy_addr;

    /* Mmap virtaddr */
    v_reg->virt_addr = ot_mpi_sys_mmap_cached(v_reg->phy_addr, size);

    return TD_SUCCESS;
}

td_void *isp_get_be_reg_virt_addr_base(td_u32 base_addr)
{
    td_u32 size;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_isp_be_base(base_addr)) {
        isp_err_trace("get be_reg base vir addr error:0x%x\n", base_addr);
        return TD_NULL;
    }

    v_reg = &g_ot_vreg.isp_be_reg_addr[isp_get_be_id(base_addr)];
    if (v_reg->virt_addr != TD_NULL) {
        return v_reg->virt_addr;
    }
    v_reg->phy_addr = isp_be_reg_base(isp_get_be_id(base_addr));
    size = VI_ISP_BE_REG_SIZE;
    v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);

    return v_reg->virt_addr;
}

td_void *isp_get_viproc_reg_virt_addr_base(td_u32 base_addr)
{
    td_u32 size;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_viproc_base(base_addr)) {
        isp_err_trace("get viproc_reg base vir addr error:0x%x\n", base_addr);
        return TD_NULL;
    }

    v_reg = &g_ot_vreg.viproc_reg_addr[isp_get_viproc_id(base_addr)];
    if (v_reg->virt_addr != TD_NULL) {
        return v_reg->virt_addr;
    }
    v_reg->phy_addr = isp_viproc_reg_base(isp_get_viproc_id(base_addr));
    size = VIPROC_REG_SIZE;
    v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);

    return v_reg->virt_addr;
}

td_void *isp_get_vicap_chn_reg_virt_addr_base(td_u32 base_addr)
{
    td_u32 size;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_vicap_ch_base(base_addr)) {
        isp_err_trace("get vicap_chn_reg base vir addr error:0x%x\n", base_addr);
        return TD_NULL;
    }

    v_reg = &g_ot_vreg.vicap_ch_reg_addr[isp_get_vicap_ch_id(base_addr)];
    if (v_reg->virt_addr != TD_NULL) {
        return v_reg->virt_addr;
    }
    v_reg->phy_addr = isp_vicap_ch_reg_base(isp_get_vicap_ch_id(base_addr));
    size = VICAP_CH_REG_SIZE_ALIGN;
    v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);

    return v_reg->virt_addr;
}

td_void *isp_get_fe_reg_virt_addr_base(td_u32 base_addr)
{
    td_u32 size;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (!isp_check_isp_fe_base(base_addr)) {
        isp_err_trace("get fe_reg base vir addr error:0x%x\n", base_addr);
        return TD_NULL;
    }
    v_reg = &g_ot_vreg.isp_fe_reg_addr[isp_get_fe_id(base_addr)];
    if (v_reg->virt_addr != TD_NULL) {
        return v_reg->virt_addr;
    }
    v_reg->phy_addr = isp_fe_reg_base(isp_get_fe_id(base_addr));
    size = FE_REG_SIZE_ALIGN;
    v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);
    return v_reg->virt_addr;
}

td_void *vreg_get_virt_addr_base(td_u32 base_addr)
{
    td_s32 ret;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    v_reg = vreg_match(base_addr);
    if (v_reg == TD_NULL) {
        isp_err_trace("find vreg base addr failed:0x%x\n", base_addr);
        return v_reg;
    }

    if (v_reg->virt_addr != TD_NULL) {
        return v_reg->virt_addr;
    }

    /* get phyaddr first */
    ret = isp_virt_vreg_map(base_addr, v_reg);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return v_reg->virt_addr;
}

static td_u32 get_slave_addr_offset(td_u32 base_addr)
{
    td_u8 addr_id;

    addr_id = get_slave_id(base_addr);

    switch (addr_id) {
        case 0:                                                              /* case index 0 */
            return ((base_addr - cap_slave_reg_base(addr_id)) & 0xFF);
        case 1:                                                              /* case index 1 */
            return ((base_addr - cap_slave_reg_base(addr_id)) & 0xFF);
        default:
            isp_err_trace("slave_id is error:%u, not 0 or 1, only use 0 and 1, need to check slave_id\n", addr_id);
            return 0;
    }
}

#define ot_vreg_write_reg32(b, addr) *(addr) = (b)
#define ot_vreg_read_reg32(addr) *(addr)

static td_void *vreg_get_virt_addr(td_u32 base_addr)
{
    td_void *virt_addr = TD_NULL;

    virt_addr = vreg_get_virt_addr_base(base_addr);
    if (virt_addr == TD_NULL) {
        return virt_addr;
    }

    if ((vreg_check_isp_vreg_base(base_addr))) {
        return ((td_u8 *)virt_addr + (base_addr & 0x1FFFF));
    } else if (vreg_check_ae_vreg_base(base_addr)) {
        return ((td_u8 *)virt_addr + (base_addr & 0x1FFF));
    } else if (vreg_check_awb_vreg_base(base_addr)) {
        return ((td_u8 *)virt_addr + (base_addr & 0xFFF));
    } else if (vreg_check_af_vreg_base(base_addr)) {
        return ((td_u8 *)virt_addr + (base_addr & 0xFFF));
    } else {
        isp_err_trace("vreg get vir_addr error, base_addr:0x%x\n", base_addr);
        return virt_addr;
    }
}

static td_void *isp_get_slave_virt_addr_base(td_u32 base_addr)
{
    td_u32 size;
    ot_vreg_addr *v_reg = TD_NULL;

    /* check base */
    if (isp_check_slave_mode_time_base(base_addr)) {
        v_reg = &g_ot_vreg.slave_time_addr;
        if (v_reg->virt_addr != TD_NULL) {
            return v_reg->virt_addr;
        }
        v_reg->phy_addr = SLAVE_MODE_REG_BASE;
        size = SLAVE_MODE_TIME_ALIGN;
        v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);
    } else if (isp_check_slave_sync_base(base_addr)) {
        v_reg = &g_ot_vreg.slave_sync_addr;
        if (v_reg->virt_addr != TD_NULL) {
            return v_reg->virt_addr;
        }
        v_reg->phy_addr = SLAVE_MODE_REG_SYNC;
        size = SLAVE_MODE_SYNC_ALIGN;
        v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);
    } else if (isp_check_slave_base(base_addr)) {
        v_reg = &g_ot_vreg.slave_reg_addr[get_slave_id(base_addr)];
        if (v_reg->virt_addr != TD_NULL) {
            return v_reg->virt_addr;
        }
        v_reg->phy_addr = cap_slave_reg_base(get_slave_id(base_addr));
        size = SLAVE_MODE_ALIGN;
        v_reg->virt_addr = isp_reg_io_mmap(v_reg->phy_addr, size);
    } else {
        isp_err_trace("slave base addr:0x%x not find the slave base addr, error\n", base_addr);
        return TD_NULL;
    }

    return v_reg->virt_addr;
}

static td_void *isp_get_slave_virt_addr(td_u32 base_addr)
{
    td_void *virt_addr = TD_NULL;

    virt_addr = isp_get_slave_virt_addr_base(base_addr);
    if (virt_addr == TD_NULL) {
        return virt_addr;
    }

    if (isp_check_slave_mode_time_base(base_addr)) {
        return ((td_u8 *)virt_addr + (base_addr - SLAVE_MODE_REG_BASE));
    } else if (isp_check_slave_sync_base(base_addr)) {
        return ((td_u8 *)virt_addr + (base_addr - SLAVE_MODE_REG_SYNC));
    } else if (isp_check_slave_base(base_addr)) {
        return ((td_u8 *)virt_addr + get_slave_addr_offset(base_addr));
    } else {
        isp_err_trace("get slave vir addr error:0x%x\n", base_addr);
        return virt_addr;
    }
}

td_u32 io_read32_slave(td_u32 addr)
{
    td_void *virt_addr = TD_NULL;
    td_u32  *addr_tmp  = TD_NULL;
    td_u32  value;

    virt_addr = isp_get_slave_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_u32 *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    value = ot_vreg_read_reg32(addr_tmp);

    return value;
}

td_s32 io_write32_slave(td_u32 addr, td_u32 value)
{
    td_void *virt_addr = TD_NULL;
    td_u32  *addr_tmp = TD_NULL;

    virt_addr = isp_get_slave_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_u32 *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    ot_vreg_write_reg32(value, addr_tmp);

    return TD_SUCCESS;
}

td_u32 io_read32(td_u32 addr)
{
    td_void *virt_addr = TD_NULL;
    td_u32  *addr_tmp  = TD_NULL;
    td_u32  value;

    virt_addr = vreg_get_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_u32 *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    value = ot_vreg_read_reg32(addr_tmp);

    return value;
}

td_s32 io_read32_ex(td_u32 addr, td_u32 *value)
{
    td_void *virt_addr = TD_NULL;
    td_u32  *addr_tmp = TD_NULL;
    td_u32  value_tmp;
    isp_check_pointer_return(value);
    virt_addr = vreg_get_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return TD_FAILURE;
    }
    addr_tmp  = (td_u32 *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    value_tmp = ot_vreg_read_reg32(addr_tmp);
    *value = value_tmp;
    return TD_SUCCESS;
}

td_s32 io_write32(td_u32 addr, td_u32 value)
{
    td_void *virt_addr = TD_NULL;
    td_u32  *addr_tmp = TD_NULL;

    virt_addr = vreg_get_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_u32 *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    ot_vreg_write_reg32(value, addr_tmp);

    return TD_SUCCESS;
}

td_u8 io_read8(td_u32 addr)
{
    td_u32 value;

    value = io_read32(addr & IO_MASK_BIT32);

    return (value >> get_shift_bit(addr)) & 0xFF;
}

td_s32 io_write8(td_u32 addr, td_u32 value)
{
    td_u32  current;
    td_u32  current_mask;
    td_u32  value_tmp;
    td_s32  ret;

    current_mask = ~(0xFFU << get_shift_bit(addr));
    value_tmp    = (value & 0xFF) << get_shift_bit(addr);
    current = io_read32(addr & IO_MASK_BIT32);
    ret = io_write32((addr & IO_MASK_BIT32), value_tmp | (current & current_mask));

    return ret;
}

td_u16 io_read16(td_u32 addr)
{
    td_u32  value;

    value = io_read32(addr & IO_MASK_BIT32);

    return (value >> get_shift_bit(addr & 0xFFFFFFFE)) & 0xFFFF;
}

td_s32 io_write16(td_u32 addr, td_u32 value)
{
    td_u32  current;
    td_u32  current_mask;
    td_u32  value_tmp;
    td_s32  ret;

    current_mask = ~(0xFFFFU << get_shift_bit(addr & 0xFFFFFFFE));
    value_tmp    = (value & 0xFFFF) << get_shift_bit(addr & 0xFFFFFFFE);
    current = io_read32(addr & IO_MASK_BIT32);
    ret = io_write32((addr & IO_MASK_BIT32), value_tmp | (current & current_mask));

    return ret;
}

td_s32 io_write_double(td_u32 addr, td_double value)
{
    td_void *virt_addr = TD_NULL;
    td_double *addr_tmp = TD_NULL;

    virt_addr = vreg_get_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_double *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    *addr_tmp = value;

    return TD_SUCCESS;
}

td_double io_read_double(td_u32 addr)
{
    td_void *virt_addr = TD_NULL;
    td_double *addr_tmp  = TD_NULL;
    td_double  value;

    virt_addr = vreg_get_virt_addr(addr);
    if (virt_addr == TD_NULL) {
        return 0;
    }

    addr_tmp = (td_double *)(td_uintptr_t)((td_uintptr_t)virt_addr & IO_MASK_BITXX);
    value = *addr_tmp;

    return value;
}

td_s32 isp_mem_share_vreg(ot_vi_pipe vi_pipe, td_s32 pid, td_s32 ae_lib_id, td_s32 awb_lib_id)
{
    vreg_args vreg;
    td_s32 ret;

    vreg.base_addr = isp_vir_reg_base(vi_pipe);
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg)!= TD_SUCCESS) {
        isp_err_trace("get isp vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_ae_id_vreg_base(ae_lib_vreg_base((td_u32)ae_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get ae vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_awb_id_vreg_base(awb_lib_vreg_base((td_u32)awb_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get awb vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_mem_unshare_vreg(ot_vi_pipe vi_pipe, td_s32 pid, td_s32 ae_lib_id, td_s32 awb_lib_id)
{
    vreg_args vreg;
    td_s32 ret;

    vreg.base_addr = isp_vir_reg_base(vi_pipe);
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg)!= TD_SUCCESS) {
        isp_err_trace("get isp vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_ae_id_vreg_base(ae_lib_vreg_base((td_u32)ae_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get ae vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_awb_id_vreg_base(awb_lib_vreg_base((td_u32)awb_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get awb vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem(vreg.phy_addr, pid);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_mem_share_all_vreg(ot_vi_pipe vi_pipe, td_s32 ae_lib_id, td_s32 awb_lib_id)
{
    vreg_args vreg;
    td_s32 ret;

    vreg.base_addr = isp_vir_reg_base(vi_pipe);
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg)!= TD_SUCCESS) {
        isp_err_trace("get isp vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_ae_id_vreg_base(ae_lib_vreg_base((td_u32)ae_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get ae vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_awb_id_vreg_base(awb_lib_vreg_base((td_u32)awb_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get awb vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_share_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_mem_unshare_all_vreg(ot_vi_pipe vi_pipe, td_s32 ae_lib_id, td_s32 awb_lib_id)
{
    vreg_args vreg;
    td_s32 ret;

    vreg.base_addr = isp_vir_reg_base(vi_pipe);
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg)!= TD_SUCCESS) {
        isp_err_trace("get isp vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_ae_id_vreg_base(ae_lib_vreg_base((td_u32)ae_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get ae vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vreg.base_addr = get_awb_id_vreg_base(awb_lib_vreg_base((td_u32)awb_lib_id));
    if (ioctl(g_vreg_fd[0], VREG_DRV_GETADDR, &vreg) != TD_SUCCESS) {
        isp_err_trace("get awb vreg buffer info failed\n");
        return TD_FAILURE;
    }
    ret = isp_unshare_mem_all(vreg.phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}
