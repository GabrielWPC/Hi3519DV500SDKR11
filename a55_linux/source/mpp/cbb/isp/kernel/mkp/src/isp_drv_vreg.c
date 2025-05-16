/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_drv_vreg.h"
#include "ot_osal.h"
#include "mm_ext.h"
#include "isp_vreg.h"
#include "securec.h"
#include "osal_ioctl.h"
#include "isp_drv_define.h"
static drv_vreg_args g_vreg[OT_ISP_MAX_PIPE_NUM][OT_ISP_VREG_MAX_NUM] = {0};
drv_vreg_args *vreg_drv_search(ot_vi_pipe vi_pipe, td_phys_addr_t base_addr)
{
    td_s32  i;

    for (i = 0; i < OT_ISP_VREG_MAX_NUM; i++) {
        if ((g_vreg[vi_pipe][i].phy_addr != 0) &&
            (base_addr == g_vreg[vi_pipe][i].base_addr)) {
            return &g_vreg[vi_pipe][i];
        }
    }

    return TD_NULL;
}

drv_vreg_args *vreg_drv_query(td_phys_addr_t base_addr)
{
    td_s32  i, j;

    for (j = 0; j < OT_ISP_MAX_PIPE_NUM; j++) {
        for (i = 0; i < OT_ISP_VREG_MAX_NUM; i++) {
            if ((g_vreg[j][i].phy_addr != 0) &&
                (base_addr == g_vreg[j][i].base_addr)) {
                return &g_vreg[j][i];
            }
        }
    }

    return TD_NULL;
}

static td_s32 vreg_get_unused_idx(ot_vi_pipe vi_pipe, td_phys_addr_t base_addr,
    td_s32 *idx, drv_vreg_args **vreg_args)
{
    td_s32 i;
    drv_vreg_args *vreg_args_temp;
    vreg_args_temp = vreg_drv_search(vi_pipe, base_addr);
    if (vreg_args_temp != TD_NULL) {
        isp_err_trace("The vreg of base_addr 0x%lx has registered!\n", (td_ulong)base_addr);
        return TD_FAILURE;
    }

    /* search pos */
    for (i = 0; i < OT_ISP_VREG_MAX_NUM; i++) {
        if (g_vreg[vi_pipe][i].phy_addr == 0 && g_vreg[vi_pipe][i].used == TD_FALSE) {
            vreg_args_temp = &g_vreg[vi_pipe][i];
            break;
        }
    }

    if (vreg_args_temp == TD_NULL) {
        isp_err_trace("The vreg is too many, can't register!\n");
        return TD_FAILURE;
    }
    vreg_args_temp->used = TD_TRUE;
    *idx = i;
    *vreg_args = vreg_args_temp;
    return TD_SUCCESS;
}

td_s32 vreg_drv_init(ot_vi_pipe vi_pipe, td_phys_addr_t base_addr, td_u64 size)
{
    td_s32 ret, idx;
    td_phys_addr_t phy_addr;
    td_u8 *vir_addr = TD_NULL;
    td_char name[MAX_MMZ_NAME_LEN] = {0};
    drv_vreg_args *vreg_args = TD_NULL;
    mm_malloc_param malloc_param = {0};
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags = 0;

    isp_check_pipe_return(vi_pipe);

    /* check param */
    if (size == 0) {
        isp_err_trace("The vreg's size is 0!\n");
        return OT_ERR_ISP_ILLEGAL_PARAM;
    }
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);

    ret = vreg_get_unused_idx(vi_pipe, base_addr, &idx, &vreg_args);
    if (ret != TD_SUCCESS) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        return ret;
    }
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    /* Mmz malloc memory */
    ret =  snprintf_s(name, sizeof(name), sizeof(name) - 1, "isp[%d].vreg[%d]", vi_pipe, idx);
    if (ret < 0) {
        goto err_ret;
    }
    malloc_param.buf_name = name;
    malloc_param.size = size;
    malloc_param.kernel_only = TD_FALSE;
    ret = cmpi_mmz_malloc_cached(&malloc_param, &phy_addr, (td_void **)&vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("alloc virt regs buf err\n");
        ret = OT_ERR_ISP_NOMEM;
        goto err_ret;
    }

    (td_void)memset_s(vir_addr, size, 0, size);

    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    vreg_args->phy_addr  = phy_addr;
    vreg_args->virt_addr   = (td_void *)vir_addr;
    vreg_args->base_addr = base_addr;
    vreg_args->size = size;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    return TD_SUCCESS;
err_ret:
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    vreg_args->used = TD_FALSE;
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
    return ret;
}

td_s32 vreg_drv_exit(ot_vi_pipe vi_pipe, td_phys_addr_t base_addr)
{
    td_void *virt_addr = TD_NULL;
    drv_vreg_args *vreg_args = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags = 0;
    td_phys_addr_t phy_addr;

    phy_addr = 0;
    isp_check_pipe_return(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);
    osal_spin_lock_irqsave(isp_spin_lock, &flags);
    vreg_args = vreg_drv_search(vi_pipe, base_addr);
    if (vreg_args == TD_NULL) {
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        isp_warn_trace("The vreg of base_addr 0x%lx has not registered!\n", (td_ulong)base_addr);
        return TD_FAILURE;
    }

    if (vreg_args->phy_addr != 0) {
        phy_addr = vreg_args->phy_addr;
        virt_addr = vreg_args->virt_addr;
        vreg_args->phy_addr  = 0;
        vreg_args->size     = 0;
        vreg_args->base_addr = 0;
        vreg_args->virt_addr   = TD_NULL;
        vreg_args->used = TD_FALSE;
    }
    osal_spin_unlock_irqrestore(isp_spin_lock, &flags);

    if (phy_addr != 0) {
        cmpi_mmz_free(phy_addr, virt_addr);
    }

    return TD_SUCCESS;
}

td_s32 vreg_drv_get_addr(td_phys_addr_t base_addr, td_phys_addr_t *phy_addr)
{
    td_s32 ret;
    drv_vreg_args *vreg_args = TD_NULL;

    vreg_args = vreg_drv_query(base_addr);
    if (vreg_args == TD_NULL) {
        isp_warn_trace("The vreg of base_addr 0x%lx has not registered!\n", (td_ulong)base_addr);
        return TD_FAILURE;
    }
    ret = ot_mmz_check_phys_addr(vreg_args->phy_addr, vreg_args->size);
    if (ret != TD_SUCCESS) {
        isp_err_trace("The vreg of base_addr 0x%lx check error\n", (td_ulong)base_addr);
        return TD_FAILURE;
    }

    *phy_addr = vreg_args->phy_addr;

    return TD_SUCCESS;
}

td_s32 vreg_drv_release_all(ot_vi_pipe vi_pipe)
{
    td_s32  i;
    td_void *virt_addr = TD_NULL;
    osal_spinlock *isp_spin_lock = TD_NULL;
    unsigned long flags = 0;
    td_phys_addr_t phy_addr;

    phy_addr = 0;
    isp_check_pipe_return(vi_pipe);
    isp_spin_lock = isp_drv_get_lock(vi_pipe);

    for (i = 0; i < OT_ISP_VREG_MAX_NUM; i++) {
        osal_spin_lock_irqsave(isp_spin_lock, &flags);
        if (g_vreg[vi_pipe][i].phy_addr != 0) {
            phy_addr = g_vreg[vi_pipe][i].phy_addr;
            virt_addr = g_vreg[vi_pipe][i].virt_addr;
            g_vreg[vi_pipe][i].phy_addr  = 0;
            g_vreg[vi_pipe][i].base_addr = 0;
            g_vreg[vi_pipe][i].size     = 0;
            g_vreg[vi_pipe][i].virt_addr   = TD_NULL;
            g_vreg[vi_pipe][i].used   = TD_FALSE;
        }
        osal_spin_unlock_irqrestore(isp_spin_lock, &flags);
        if (phy_addr != 0) {
            cmpi_mmz_free(phy_addr, virt_addr);
            phy_addr = 0;
        }
    }

    return TD_SUCCESS;
}

td_s32 vreg_drv_check_permission(ot_vi_pipe vi_pipe)
{
    td_s32  i;
    td_s32 ret;
    isp_check_pipe_return(vi_pipe);

    for (i = 0; i < OT_ISP_VREG_MAX_NUM; i++) {
        if (g_vreg[vi_pipe][i].phy_addr != 0) {
            ret = ot_mmz_check_phys_addr(g_vreg[vi_pipe][i].phy_addr, g_vreg[vi_pipe][i].size);
            if (ret != TD_SUCCESS) {
                isp_err_trace("vreg addr check permission error\n");
                return TD_FAILURE;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_ioctl_set_vreg_fd(unsigned int cmd, void *arg, void *private_data)
{
    ot_unused(cmd);

    isp_check_pointer_return(arg);
    isp_check_pointer_return(private_data);
    *((td_u32 *)(private_data)) = *(td_u32 *)arg;

    return TD_SUCCESS;
}
/* malloc memory for vregs, and record information in kernel. */
td_s32 isp_ioctl_init_vreg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    unsigned int *argp = (unsigned int *)arg;
    drv_vreg_args *vreg_args = TD_NULL;

    ot_unused(cmd);
    isp_check_pointer_return(arg);

    vreg_args = (drv_vreg_args *)argp;

    return vreg_drv_init(vi_pipe, vreg_args->base_addr, vreg_args->size);
}
/* free the memory of vregs, and clean information in kernel. */
td_s32 isp_ioctl_vreg_exit(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    unsigned int *argp = (unsigned int *)arg;
    drv_vreg_args *vreg_args = TD_NULL;

    ot_unused(cmd);
    isp_check_pointer_return(arg);

    vreg_args = (drv_vreg_args *)argp;

    return vreg_drv_exit(vi_pipe, vreg_args->base_addr);
}
/* free the memory of vregs, and clean information in kernel. */
td_s32 isp_ioctl_release_vreg(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    unsigned int *argp = (unsigned int *)arg;
    drv_vreg_args *vreg_args = TD_NULL;

    ot_unused(cmd);
    isp_check_pointer_return(arg);

    vreg_args = (drv_vreg_args *)argp;

    if ((vreg_args->size == ISP_VREG_SIZE) && (vreg_args->base_addr == ISP_VREG_BASE)) {
        vreg_drv_release_all(vi_pipe);
    }

    return TD_SUCCESS;
}
/* get the mapping relation between vreg addr and physical addr. */
td_s32 isp_ioctl_get_vreg_addr(unsigned int cmd, void *arg, void *private_data)
{
    unsigned int *argp = (unsigned int *)arg;
    drv_vreg_args *vreg_args = TD_NULL;

    ot_unused(cmd);
    ot_unused(private_data);
    isp_check_pointer_return(arg);

    vreg_args = (drv_vreg_args *)argp;

    return vreg_drv_get_addr(vreg_args->base_addr, &vreg_args->phy_addr);
}

td_s32 isp_ioctl_check_permission(unsigned int cmd, void *arg, void *private_data)
{
    ot_vi_pipe vi_pipe = isp_get_pipe(private_data);
    isp_check_pipe_return(vi_pipe);
    ot_unused(cmd);
    ot_unused(arg);

    return vreg_drv_check_permission(vi_pipe);
}

