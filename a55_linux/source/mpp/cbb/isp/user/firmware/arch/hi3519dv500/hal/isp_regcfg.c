/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "isp_config.h"
#include "isp_lut_config.h"
#include "isp_ext_config.h"
#include "isp_main.h"
#include "isp_math_utils.h"
#include "ot_mpi_sys_mem.h"
#include "ot_math.h"
#include "valg_plat.h"
#include "isp_regcfg.h"
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
#include "isp_dump_dbg.h"
#endif
isp_be_buf            g_be_buf_ctx[OT_ISP_MAX_PIPE_NUM] = {{ 0 }};
isp_reg_cfg_attr     *g_reg_cfg_ctx[OT_ISP_MAX_PIPE_NUM] = { TD_NULL };
isp_be_lut_buf        g_be_lut_buf_ctx[OT_ISP_MAX_PIPE_NUM] = { 0 };
isp_ldci_read_stt_buf g_ldci_read_stt_buf_ctx[OT_ISP_MAX_PIPE_NUM] = { 0 };
isp_fe_lut2stt_attr   g_fe_lut_stt_buf_ctx[OT_ISP_MAX_FE_PIPE_NUM] = { 0 };

#define isp_regcfg_set_ctx(pipe, ctx) (g_reg_cfg_ctx[pipe] = (ctx))
#define isp_regcfg_reset_ctx(pipe) (g_reg_cfg_ctx[pipe] = TD_NULL)
#define be_reg_get_ctx(pipe, ctx) ctx = &g_be_buf_ctx[pipe]
#define be_lut_buf_get_ctx(pipe, ctx) ctx = &g_be_lut_buf_ctx[pipe]
#define fe_lut_stt_buf_get_ctx(pipe, ctx) ctx = &g_fe_lut_stt_buf_ctx[pipe]
#define ldci_buf_get_ctx(pipe, ctx) ctx = &g_ldci_read_stt_buf_ctx[pipe]
#define STT_LUT_CONFIG_TIMES 3

isp_reg_cfg_attr *isp_get_regcfg_ctx(ot_vi_pipe vi_pipe)
{
    return g_reg_cfg_ctx[vi_pipe];
}

td_s32 isp_ctrl_param_get(ot_isp_ctrl_param *ctrl_param)
{
    isp_check_pointer_return(ctrl_param);

    ctrl_param->quick_start_en = TD_FALSE;

    return TD_SUCCESS;
}

td_s32 isp_clut_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        return TD_SUCCESS;
    }
    ret = ioctl(isp_get_fd(vi_pipe), ISP_CLUT_BUF_INIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] clut buffer init failed\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void isp_clut_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if (isp_ctx->isp_yuv_mode == TD_TRUE) {
        return;
    }
    ret = ioctl(isp_get_fd(vi_pipe), ISP_CLUT_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] exit clut bufs failed\n", vi_pipe);
        return;
    }
}

td_s32 isp_update_fe_lut_stt_buf_ctx(ot_vi_pipe vi_pipe, td_u64 phy_addr)
{
    td_u8 i;
    size_t size;
    td_void *virt_addr = TD_NULL;
    isp_fe_lut2stt_attr *fe_lut_buf = TD_NULL;

    fe_lut_stt_buf_get_ctx(vi_pipe, fe_lut_buf);

    size = sizeof(isp_fe_lut_wstt_type);

    virt_addr = ot_mpi_sys_mmap(phy_addr, size * PING_PONG_NUM);
    if (virt_addr == TD_NULL) {
        isp_err_trace("Pipe:%d get fe lut stt bufs address failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < PING_PONG_NUM; i++) {
        fe_lut_buf->lut_stt_buf[i].phy_addr = phy_addr + i * size;
        fe_lut_buf->lut_stt_buf[i].vir_addr = (td_void *)((td_u8 *)virt_addr + i * size);
        fe_lut_buf->lut_stt_buf[i].size = size;
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_be_lut_stt_buf_ctx(ot_vi_pipe vi_pipe, td_u64 phy_addr)
{
    td_u8 i;
    size_t size;
    td_void *virt_addr = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        return TD_FAILURE;
    }

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    size = sizeof(isp_be_lut_wstt_type);

    virt_addr = ot_mpi_sys_mmap(phy_addr, size * 2 * ISP_MAX_BE_NUM); /* lut_stt_buf number 2  */
    if (virt_addr == TD_NULL) {
        isp_err_trace("Pipe:%d get be lut stt bufs address failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < ISP_MAX_BE_NUM; i++) {
        be_lut_buf->lut_stt_buf[i].phy_addr = phy_addr + 2 * i * size;                        /* phy_addr index 2  */
        be_lut_buf->lut_stt_buf[i].vir_addr = (td_void *)((td_u8 *)virt_addr + 2 * i * size); /* lut_stt_buf 2  */
        be_lut_buf->lut_stt_buf[i].size = size;
    }

    return TD_SUCCESS;
}


static td_s32 isp_update_be_pre_online_post_offline_lut_stt_buf_ctx(ot_vi_pipe vi_pipe, td_u64 phy_addr)
{
    td_u8 i;
    size_t size;
    td_void *virt_addr = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return TD_FAILURE;
    }

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    size = sizeof(isp_pre_be_lut_wstt_type);

    virt_addr = ot_mpi_sys_mmap(phy_addr, size * 2 * ISP_MAX_BE_NUM); /* lut_stt_buf number 2  */
    if (virt_addr == TD_NULL) {
        isp_err_trace("Pipe:%d get be lut stt bufs address failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < ISP_MAX_BE_NUM; i++) {
        be_lut_buf->lut_stt_buf[i].phy_addr = phy_addr + 2 * i * size;                        /* phy_addr index 2  */
        be_lut_buf->lut_stt_buf[i].vir_addr = (td_void *)((td_u8 *)virt_addr + 2 * i * size); /* lut_stt_buf 2  */
        be_lut_buf->lut_stt_buf[i].size = size;
    }

    return TD_SUCCESS;
}
static td_s32 isp_be_pre_online_post_offline_lut_buf_addr_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u64 phy_addr;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_PRE_LUT_STT_BUF_GET, &phy_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d get be lut2stt bufs address failed%x!\n", vi_pipe, ret);
        return ret;
    }

    ot_ext_system_be_lut_stt_buffer_high_addr_write(vi_pipe, (phy_addr >> 32)); /* shitf right  32 bit  */
    ot_ext_system_be_lut_stt_buffer_low_addr_write(vi_pipe, (phy_addr & 0xFFFFFFFF));

    ret = isp_update_be_pre_online_post_offline_lut_stt_buf_ctx(vi_pipe, phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_bool isp_fe_lut_stt_buf_addr_need_init(isp_usr_ctx *isp_ctx)
{
    td_u8 i;
    ot_vi_pipe vi_pipe_bind;
    td_u8 local_blc_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_LOCAL_BLC_ENABLE };

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        if (local_blc_en[vi_pipe_bind] == 1) {
            return TD_TRUE;
        }
    }

    return TD_FALSE;
}

static td_s32 isp_fe_lut_stt_buf_addr_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_bool need_init;
    td_u64 phys_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_no_fe_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    need_init = isp_fe_lut_stt_buf_addr_need_init(isp_ctx);
    if (need_init == TD_FALSE) {
        return TD_SUCCESS;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_FE_LUT_STT_BUF_GET, &phys_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d get fe lut2stt bufs address failed %x!\n", vi_pipe, ret);
        return ret;
    }

    ot_ext_system_fe_lut_stt_buffer_high_addr_write(vi_pipe, (phys_addr >> 32)); /* shitf right  32 bit  */
    ot_ext_system_fe_lut_stt_buffer_low_addr_write(vi_pipe, (phys_addr & 0xFFFFFFFF));

    ret = isp_update_fe_lut_stt_buf_ctx(vi_pipe, phys_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_fe_lut_buf_addr_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_fe_lut2stt_attr *fe_lut_buf = TD_NULL;

    isp_check_no_fe_pipe_return(vi_pipe);
    fe_lut_stt_buf_get_ctx(vi_pipe, fe_lut_buf);

    if (fe_lut_buf->lut_stt_buf[0].vir_addr != TD_NULL) {
        ot_mpi_sys_munmap(fe_lut_buf->lut_stt_buf[0].vir_addr, sizeof(isp_fe_lut_wstt_type) * PING_PONG_NUM);
        for (i = 0; i < PING_PONG_NUM; i++) {
            fe_lut_buf->lut_stt_buf[i].vir_addr = TD_NULL;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_be_lut_buf_addr_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return TD_SUCCESS;
    }

    if (is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        isp_be_pre_online_post_offline_lut_buf_addr_init(vi_pipe);
        return TD_SUCCESS;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_LUT_STT_BUF_GET, &phy_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d get be lut2stt bufs address failed%x!\n", vi_pipe, ret);
        return ret;
    }

    ot_ext_system_be_lut_stt_buffer_high_addr_write(vi_pipe, (phy_addr >> 32)); /* shitf right  32 bit  */
    ot_ext_system_be_lut_stt_buffer_low_addr_write(vi_pipe, (phy_addr & 0xFFFFFFFF));

    ret = isp_update_be_lut_stt_buf_ctx(vi_pipe, phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}


static td_s32 isp_be_lut_buf_addr_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return TD_SUCCESS;
    }
    if (is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        ot_mpi_sys_munmap(be_lut_buf->lut_stt_buf[0].vir_addr,
            sizeof(isp_pre_be_lut_wstt_type) * 2 * ISP_MAX_BE_NUM); /* lut buf number is 2 */
        for (i = 0; i < ISP_MAX_BE_NUM; i++) {
            be_lut_buf->lut_stt_buf[i].vir_addr = TD_NULL;
        }
        return TD_SUCCESS;
    }

    if (be_lut_buf->lut_stt_buf[0].vir_addr != TD_NULL) {
        ot_mpi_sys_munmap(be_lut_buf->lut_stt_buf[0].vir_addr,
            sizeof(isp_be_lut_wstt_type) * 2 * ISP_MAX_BE_NUM); /* lut buf number is 2 */
        for (i = 0; i < ISP_MAX_BE_NUM; i++) {
            be_lut_buf->lut_stt_buf[i].vir_addr = TD_NULL;
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_stt_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    if (ioctl(isp_get_fd(vi_pipe), ISP_STT_BUF_INIT) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] stt buffer init failed\n", vi_pipe);
        return TD_FAILURE;
    }

    ret = isp_be_lut_buf_addr_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] be lut2stt buffer address init failed\n", vi_pipe);
        goto fail0;
    }

    ret = isp_fe_lut_stt_buf_addr_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] fe lut2stt buffer address init failed\n", vi_pipe);
        goto fail1;
    }

    return TD_SUCCESS;

fail1:
    isp_be_lut_buf_addr_exit(vi_pipe);
fail0:
    isp_stt_buf_exit(vi_pipe);
    return TD_FAILURE;
}

td_void isp_stt_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = isp_fe_lut_buf_addr_exit(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] fe lut stt buffer exit failed\n", vi_pipe);
    }

    ret = isp_be_lut_buf_addr_exit(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] be lut stt buffer exit failed\n", vi_pipe);
    }

    if (ioctl(isp_get_fd(vi_pipe), ISP_STT_BUF_EXIT) != TD_SUCCESS) {
        isp_err_trace("exit stt bufs failed\n");
        return;
    }
}

td_s32 isp_stt_addr_init(ot_vi_pipe vi_pipe)
{
    if (ioctl(isp_get_fd(vi_pipe), ISP_STT_ADDR_INIT) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] stt address init failed\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 isp_update_ldci_read_stt_buf_ctx(ot_vi_pipe vi_pipe, td_u64 phy_addr)
{
    td_u8 i;
    size_t size;
    td_void *virt_addr = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ldci_read_stt_buf *ldci_read_stt_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->ldci_tpr_flt_en == TD_FALSE) {
        return TD_SUCCESS;
    }

    ldci_buf_get_ctx(vi_pipe, ldci_read_stt_buf);

    size = sizeof(isp_ldci_stat);
    virt_addr = ot_mpi_sys_mmap(phy_addr, size * ldci_read_stt_buf->buf_num);
    if (virt_addr == TD_NULL) {
        isp_err_trace("ISP[%d]:map ldci read stt buffer failed\n", vi_pipe);
        return TD_FAILURE;
    }

    for (i = 0; i < ldci_read_stt_buf->buf_num; i++) {
        ldci_read_stt_buf->read_buf[i].phy_addr = phy_addr + i * size;
        ldci_read_stt_buf->read_buf[i].vir_addr = (td_void *)((td_u8 *)virt_addr + i * size);
        ldci_read_stt_buf->read_buf[i].size = size;
    }

    return TD_SUCCESS;
}

td_s32 isp_ldci_read_stt_buf_addr_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u64 phy_addr;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ldci_read_stt_buf *ldci_read_stt_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->ldci_tpr_flt_en == TD_FALSE) {
        return TD_SUCCESS;
    }

    ldci_buf_get_ctx(vi_pipe, ldci_read_stt_buf);
    ret = ioctl(isp_get_fd(vi_pipe), ISP_LDCI_READ_STT_BUF_GET, ldci_read_stt_buf);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d]:Get ldci read stt buffer address failed\n", vi_pipe);
        return ret;
    }

    phy_addr = ldci_read_stt_buf->head_phy_addr;
    ot_ext_system_ldci_read_stt_buffer_low_addr_write(vi_pipe, phy_addr & 0xFFFFFFFF);
    ot_ext_system_ldci_read_stt_buffer_high_addr_write(vi_pipe, phy_addr >> 32); /* shitf right  32 bit  */

    ret = isp_update_ldci_read_stt_buf_ctx(vi_pipe, phy_addr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_ldci_read_stt_buf_addr_exit(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ldci_read_stt_buf *ldci_read_stt_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->ldci_tpr_flt_en == TD_FALSE) {
        return TD_SUCCESS;
    }

    ldci_buf_get_ctx(vi_pipe, ldci_read_stt_buf);

    if (ldci_read_stt_buf->read_buf[0].vir_addr != TD_NULL) {
        ot_mpi_sys_munmap(ldci_read_stt_buf->read_buf[0].vir_addr, sizeof(isp_ldci_stat) * ldci_read_stt_buf->buf_num);

        for (i = 0; i < OT_ISP_BE_BUF_NUM_MAX; i++) {
            ldci_read_stt_buf->read_buf[i].vir_addr = TD_NULL;
        }
    }

    return TD_SUCCESS;
}

td_s32 isp_ldci_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ot_isp_ctrl_param isp_ctrl_param = { 0 };
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_CTRL_PARAM, &isp_ctrl_param);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] get ctrlparam failed\n", vi_pipe);
        return ret;
    }

    isp_ctx->ldci_tpr_flt_en = isp_ctrl_param.ldci_tpr_flt_en;

    if ((isp_ctx->ldci_tpr_flt_en == TD_FALSE) && (is_online_mode(isp_ctx->block_attr.running_mode))) {
        return TD_SUCCESS;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_LDCI_BUF_INIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] ldci buffer init failed\n", vi_pipe);
        return TD_FAILURE;
    }

    if (isp_ctx->ldci_tpr_flt_en == TD_TRUE) {
        ret = isp_ldci_read_stt_buf_addr_init(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] ldci read stt buffer address init failed\n", vi_pipe);
            isp_ldci_buf_exit(vi_pipe);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

td_void isp_ldci_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if ((isp_ctx->ldci_tpr_flt_en == TD_FALSE) && is_online_mode(isp_ctx->block_attr.running_mode)) {
        return;
    }

    if (isp_ctx->ldci_tpr_flt_en == TD_TRUE) {
        ret = isp_ldci_read_stt_buf_addr_exit(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] exit readstt bufaddr failed\n", vi_pipe);
        }
    }

    if (ioctl(isp_get_fd(vi_pipe), ISP_LDCI_BUF_EXIT) != TD_SUCCESS) {
        isp_err_trace("ISP[%d] exit ldci bufs failed\n", vi_pipe);
        return;
    }
}

td_s32 isp_drc_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        return TD_SUCCESS;
    }
    ret = ioctl(isp_get_fd(vi_pipe), ISP_DRC_BUF_INIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] drc buffer init failed\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void isp_drc_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        return;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_DRC_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] exit drc bufs failed\n", vi_pipe);
        return;
    }
}

td_s32 isp_alg_stats_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = isp_ldci_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail00;
    }

    ret = isp_drc_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        goto fail01;
    }

    return TD_SUCCESS;

fail01:
    isp_ldci_buf_exit(vi_pipe);
fail00:
    return OT_ERR_ISP_NOT_INIT;
}

td_void isp_alg_stats_buf_exit(ot_vi_pipe vi_pipe)
{
    isp_drc_buf_exit(vi_pipe);
    isp_ldci_buf_exit(vi_pipe);
}

td_s32 isp_cfg_be_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    size_t be_buf_size;
    isp_mmz_buf_ex be_buf_info;
    isp_be_buf *be_buf = TD_NULL;

    isp_check_offline_mode_return(vi_pipe);
    be_reg_get_ctx(vi_pipe, be_buf);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_CFG_BUF_INIT, &be_buf_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d init be config bufs failed %x!\n", vi_pipe, ret);
        return ret;
    }
    be_buf->be_phy_addr = be_buf_info.phy_addr;
    be_buf->be_buf_size = be_buf_info.size; // all be_buf_size: size * be_buf_num
    be_buf->be_virt_addr = ot_mpi_sys_mmap_cached(be_buf->be_phy_addr, be_buf->be_buf_size);

    if (be_buf->be_virt_addr == TD_NULL) {
        isp_err_trace("Pipe:%d init be config bufs failed!\n", vi_pipe);
        ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_CFG_BUF_EXIT);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Pipe:%d exit be config bufs failed %x!\n", vi_pipe, ret);
            return ret;
        }

        return OT_ERR_ISP_NOMEM;
    }

    be_buf->be_wo_cfg_buf.phy_addr = be_buf->be_phy_addr;

    /* Get be buffer start address & size */
    be_buf_size = be_buf_info.size;
    ot_ext_system_be_buffer_address_high_write(vi_pipe, (be_buf->be_phy_addr >> 32)); /* shitf right 32 bit */
    ot_ext_system_be_buffer_address_low_write(vi_pipe, (be_buf->be_phy_addr & 0xFFFFFFFF));
    ot_ext_system_be_buffer_size_write(vi_pipe, be_buf_size);

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
td_s32 isp_detail_stats_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_not_offline_mode_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_DETAIL_STATS_BUF_INIT, &isp_ctx->detail_stats_info);
    if (ret != TD_SUCCESS) {
        return OT_ERR_ISP_NOT_INIT;
    }

    return TD_SUCCESS;
}

td_void isp_detail_stats_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    isp_check_not_offline_mode_void(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_DETAIL_STATS_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] exit detail_stats bufs failed\n", vi_pipe);
        return;
    }

    return;
}
#endif
static td_s8 isp_get_block_id_by_pipe(ot_vi_pipe vi_pipe)
{
    td_s8 block_id = 0;
    ot_unused(vi_pipe);
    return block_id;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_void isp_update_be_buf_extend_addr(isp_usr_ctx *isp_ctx, td_u8 *virt_addr)
{
    size_t offset;
    if (isp_ctx->detail_stats_info.stats_cfg.enable == TD_FALSE) {
        isp_ctx->isp_be_ae_virt_addr = TD_NULL;
        isp_ctx->isp_be_awb_virt_addr = TD_NULL;
        return;
    }
    offset = ISP_DETAIL_STATS_EXTEND_OFFSET;
    if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_ae) {
        isp_ctx->isp_be_ae_virt_addr = (td_void *)((td_u8 *)virt_addr + offset);
        offset += ISP_DETAIL_STATS_AE_CFG_SIZE;
    }
    if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_awb) {
        isp_ctx->isp_be_awb_virt_addr = (td_void *)((td_u8 *)virt_addr + offset);
        offset += ISP_DETAIL_STATS_AWB_CFG_SIZE;
    }
}
#endif
td_s32 isp_update_be_buf_addr(ot_vi_pipe vi_pipe, td_void *virt_addr)
{
    td_u16 i;
    size_t buf_size;
    td_u8 *head_virt = TD_NULL;
    isp_running_mode isp_runing_mode;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    isp_runing_mode = isp_ctx->block_attr.running_mode;
    buf_size = sizeof(isp_be_wo_reg_cfg) / OT_ISP_STRIPING_MAX_NUM;

    switch (isp_runing_mode) {
        case ISP_MODE_RUNNING_STRIPING:
            for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
                head_virt = (td_u8 *)virt_addr + i * buf_size;
                isp_ctx->isp_pre_be_virt_addr[i] = (td_void *)head_virt;
                isp_ctx->isp_post_be_virt_addr[i] = (td_void *)(head_virt + BE_OFFLINE_OFFSET);
                isp_ctx->pre_viproc_virt_addr[i] = (td_void *)(head_virt + VIPROC_OFFLINE_OFFSET);
                isp_ctx->post_viproc_virt_addr[i] = (td_void *)(head_virt + VIPROC_OFFLINE_OFFSET);
            }
            break;

        case ISP_MODE_RUNNING_OFFLINE:
            for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
                if (i == 0) {
                    isp_ctx->isp_pre_be_virt_addr[i] = virt_addr;
                    isp_ctx->isp_post_be_virt_addr[i] = (td_void *)((td_u8 *)virt_addr + BE_OFFLINE_OFFSET);
                    isp_ctx->pre_viproc_virt_addr[i] = (td_void *)((td_u8 *)virt_addr + VIPROC_OFFLINE_OFFSET);
                    isp_ctx->post_viproc_virt_addr[i] = (td_void *)((td_u8 *)virt_addr + VIPROC_OFFLINE_OFFSET);
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
                    isp_update_be_buf_extend_addr(isp_ctx, virt_addr);
#endif
                } else {
                    isp_ctx->isp_pre_be_virt_addr[i] = TD_NULL;
                    isp_ctx->isp_post_be_virt_addr[i] = TD_NULL;
                    isp_ctx->pre_viproc_virt_addr[i] = TD_NULL;
                    isp_ctx->post_viproc_virt_addr[i] = TD_NULL;
                }
            }
            break;

        case ISP_MODE_RUNNING_PRE_ONLINE_POST_OFFLINE:
            for (i = 0; i < OT_ISP_STRIPING_MAX_NUM; i++) {
                if (i == 0) {
                    isp_ctx->isp_pre_be_virt_addr[i] = isp_get_be_reg_virt_addr_base(isp_be_reg_base(i));
                    isp_ctx->isp_post_be_virt_addr[i] = (td_void *)((td_u8 *)virt_addr);
                    isp_ctx->pre_viproc_virt_addr[i] = isp_get_viproc_reg_virt_addr_base(isp_viproc_reg_base(i));
                    isp_ctx->post_viproc_virt_addr[i] = (td_void *)((td_u8 *)virt_addr + VIPROC_OFFLINE_OFFSET);
                } else {
                    isp_ctx->isp_pre_be_virt_addr[i] = TD_NULL;
                    isp_ctx->isp_post_be_virt_addr[i] = TD_NULL;
                    isp_ctx->pre_viproc_virt_addr[i] = TD_NULL;
                    isp_ctx->post_viproc_virt_addr[i] = TD_NULL;
                }
            }
            break;

        default:
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_cfg_be_buf_mmap(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u64 be_phy_addr;
    isp_be_buf *be_buf = TD_NULL;

    be_reg_get_ctx(vi_pipe, be_buf);

    be_phy_addr = be_buf->be_wo_cfg_buf.phy_addr;
    ot_ext_system_be_free_buffer_high_addr_write(vi_pipe, (be_phy_addr >> 32)); /* shitf right  32 bit  */
    ot_ext_system_be_free_buffer_low_addr_write(vi_pipe, (be_phy_addr & 0xFFFFFFFF));

    if (be_buf->be_virt_addr != TD_NULL) {
        be_buf->be_wo_cfg_buf.vir_addr =
            (td_void *)((td_u8 *)be_buf->be_virt_addr + (be_buf->be_wo_cfg_buf.phy_addr - be_buf->be_phy_addr));
    } else {
        be_buf->be_wo_cfg_buf.vir_addr = TD_NULL;
    }

    if (be_buf->be_wo_cfg_buf.vir_addr == TD_NULL) {
        return TD_FAILURE;
    }

    ret = isp_update_be_buf_addr(vi_pipe, be_buf->be_wo_cfg_buf.vir_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp update BE bufs failed %x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_get_be_buf_first(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_be_buf *be_buf = TD_NULL;

    isp_check_offline_mode_return(vi_pipe);
    be_reg_get_ctx(vi_pipe, be_buf);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_GET_BE_BUF_FIRST, &be_buf->be_wo_cfg_buf.phy_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d Get be free bufs failed %x!\n", vi_pipe, ret);
        return ret;
    }

    ret = isp_cfg_be_buf_mmap(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_cfg_be_buf_mmap failed %x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_get_be_free_buf(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_be_buf *be_buf = TD_NULL;

    isp_check_offline_mode_return(vi_pipe);
    be_reg_get_ctx(vi_pipe, be_buf);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_FREE_BUF_GET, &be_buf->be_wo_cfg_buf);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_cfg_be_buf_mmap(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_cfg_be_buf_mmap failed %x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_get_be_last_buf(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_be_buf *be_buf = TD_NULL;

    isp_check_offline_mode_return(vi_pipe);
    be_reg_get_ctx(vi_pipe, be_buf);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_LAST_BUF_GET, &be_buf->be_wo_cfg_buf.phy_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d Get be busy bufs failed %x!\n", vi_pipe, ret);
        return ret;
    }

    ret = isp_cfg_be_buf_mmap(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_cfg_be_buf_mmap failed %x!\n", vi_pipe, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void isp_cfg_be_buf_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_buf *be_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        return;
    }
    be_reg_get_ctx(vi_pipe, be_buf);

    if (be_buf->be_virt_addr != TD_NULL) {
        ot_mpi_sys_munmap(be_buf->be_virt_addr, be_buf->be_buf_size);
        be_buf->be_virt_addr = TD_NULL;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_CFG_BUF_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d exit be config bufs failed %x!\n", vi_pipe, ret);
        return;
    }
}

td_s32 isp_cfg_be_buf_ctl(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_be_buf *be_buf = TD_NULL;

    isp_check_offline_mode_return(vi_pipe);
    be_reg_get_ctx(vi_pipe, be_buf);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_CFG_BUF_CTL, &be_buf->be_wo_cfg_buf);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_set_cfg_be_buf_state(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx   = TD_NULL;
    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    isp_check_offline_mode_return(vi_pipe);
    if (isp_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        return TD_SUCCESS;
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_CFG_BUF_RUNNING);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

/* init isp be cfgs all buffer */
td_s32 isp_all_cfgs_be_buf_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    isp_check_offline_mode_return(vi_pipe);

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_ALL_BUF_INIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] init be all bufs Failed with ec %#x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_s32 isp_check_be_buf_extend_size(isp_usr_ctx *isp_ctx, size_t map_size)
{
    size_t extend_size = 0;
    if (isp_ctx->detail_stats_info.stats_cfg.enable) {
        if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_ae) {
            extend_size += sizeof(isp_be_ae_reg_type);
        }
        if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_awb) {
            extend_size += sizeof(isp_be_awb_reg_type);
        }
        extend_size = extend_size * isp_ctx->detail_stats_info.stats_cfg.col * isp_ctx->detail_stats_info.stats_cfg.row;

        if (map_size != (sizeof(isp_be_wo_reg_cfg) + extend_size)) {
            isp_err_trace("map_size:%lx is error, be_reg_size:%lx, one extend_size:%lx \n", map_size,
                sizeof(isp_be_wo_reg_cfg), extend_size);
            return TD_FAILURE;
        }
    } else {
        if (map_size != sizeof(isp_be_wo_reg_cfg)) {
            isp_err_trace("map_size:%lx is error,  one buf size:%lx \n", map_size, sizeof(isp_be_wo_reg_cfg));
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}
#endif
td_void *isp_vreg_cfg_buf_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev)
{
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    td_s32 ret;
#endif
    size_t size, map_size;
    td_u64 phy_addr_high;
    td_u64 phy_addr_temp;
    isp_be_buf *be_buf = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    be_reg_get_ctx(vi_pipe, be_buf);

    size = sizeof(isp_be_wo_reg_cfg) / OT_ISP_STRIPING_MAX_NUM;

    if (be_buf->be_wo_cfg_buf.vir_addr != TD_NULL) {
        return ((td_u8 *)be_buf->be_wo_cfg_buf.vir_addr + blk_dev * size);
    }

    phy_addr_high = (td_u64)ot_ext_system_be_free_buffer_high_addr_read(vi_pipe);
    phy_addr_temp = (td_u64)ot_ext_system_be_free_buffer_low_addr_read(vi_pipe);
    phy_addr_temp |= (phy_addr_high << 32); /* shitf left  32 bit  */
    map_size = (be_buf->be_buf_size / isp_ctx->be_buf_num); // one be_buf size
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    ret = isp_check_be_buf_extend_size(isp_ctx, map_size);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }
#endif
    be_buf->be_wo_cfg_buf.phy_addr = phy_addr_temp;
    be_buf->be_wo_cfg_buf.vir_addr = ot_mpi_sys_mmap_cached(be_buf->be_wo_cfg_buf.phy_addr, map_size);

    return ((td_u8 *)be_buf->be_wo_cfg_buf.vir_addr + blk_dev * size);
}

static td_s32 isp_get_reg_cfg_virt_addr_online(ot_vi_pipe vi_pipe, td_void *pre_be_virt_addr[],
    td_void *post_be_virt_addr[], td_void *pre_viproc_virt_addr[], td_void *post_viproc_virt_addr[])
{
    td_u8 k;
    td_s8 blk_dev;
    td_u8 block_id;

    blk_dev = isp_get_block_id_by_pipe(vi_pipe);
    if (blk_dev == -1) {
        isp_err_trace("ISP[%d] Online Mode Pipe Err!\n", vi_pipe);
        return TD_FAILURE;
    }

    block_id = (td_u8)blk_dev;
    for (k = 0; k < OT_ISP_STRIPING_MAX_NUM; k++) {
        if (k == 0) {
            pre_be_virt_addr[k] = isp_get_be_reg_virt_addr_base(isp_be_reg_base(block_id));
            post_be_virt_addr[k] = pre_be_virt_addr[k];
            pre_viproc_virt_addr[k] = isp_get_viproc_reg_virt_addr_base(isp_viproc_reg_base(block_id));
            post_viproc_virt_addr[k] = pre_viproc_virt_addr[k];
        } else {
            pre_be_virt_addr[k] = TD_NULL;
            post_be_virt_addr[k] = TD_NULL;
            pre_viproc_virt_addr[k] = TD_NULL;
            post_viproc_virt_addr[k] = TD_NULL;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_get_reg_cfg_virt_addr_offline(ot_vi_pipe vi_pipe, td_void *pre_be_virt_addr[],
    td_void *post_be_virt_addr[], td_void *pre_viproc_virt_addr[], td_void *post_viproc_virt_addr[])
{
    td_u8 k;
    td_void *head_virt = TD_NULL;
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
#endif
    for (k = 0; k < OT_ISP_STRIPING_MAX_NUM; k++) {
        if (k == 0) {
            head_virt = (td_u8 *)isp_vreg_cfg_buf_addr(vi_pipe, (ot_blk_dev)k);
            pre_be_virt_addr[k] = head_virt;
            post_be_virt_addr[k] = pre_be_virt_addr[k];
            pre_viproc_virt_addr[k] = (td_void *)((td_u8 *)head_virt + VIPROC_OFFLINE_OFFSET);
            post_viproc_virt_addr[k] = pre_viproc_virt_addr[k];
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
            isp_update_be_buf_extend_addr(isp_ctx, head_virt);
#endif
        } else {
            pre_be_virt_addr[k] = TD_NULL;
            post_be_virt_addr[k] = TD_NULL;
            pre_viproc_virt_addr[k] = TD_NULL;
            post_viproc_virt_addr[k] = TD_NULL;
        }
    }
}

static td_s32 isp_get_reg_cfg_virt_addr_pre_online_post_offline(ot_vi_pipe vi_pipe, td_void *pre_be_virt_addr[],
    td_void *post_be_virt_addr[], td_void *pre_viproc_virt_addr[], td_void *post_viproc_virt_addr[])
{
    td_u8 k;
    td_void *head_virt = TD_NULL;

    for (k = 0; k < OT_ISP_STRIPING_MAX_NUM; k++) {
        if (k == 0) {
            head_virt = (td_u8 *)isp_vreg_cfg_buf_addr(vi_pipe, (ot_blk_dev)k);
            pre_be_virt_addr[k] = isp_get_be_reg_virt_addr_base(isp_be_reg_base(k));
            pre_viproc_virt_addr[k] = isp_get_viproc_reg_virt_addr_base(isp_viproc_reg_base(k));
            post_be_virt_addr[k] = (td_void *)((td_u8 *)head_virt + BE_OFFLINE_OFFSET);
            post_viproc_virt_addr[k] = (td_void *)((td_u8 *)head_virt + VIPROC_OFFLINE_OFFSET);
        } else {
            pre_be_virt_addr[k] = TD_NULL;
            post_be_virt_addr[k] = TD_NULL;
            pre_viproc_virt_addr[k] = TD_NULL;
            post_viproc_virt_addr[k] = TD_NULL;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_get_reg_cfg_virt_addr_striping(ot_vi_pipe vi_pipe, td_void *pre_be_virt_addr[],
    td_void *post_be_virt_addr[], td_void *pre_viproc_virt_addr[], td_void *post_viproc_virt_addr[])
{
    td_u8 k;
    td_u8 *head_virt = TD_NULL;

    for (k = 0; k < OT_ISP_STRIPING_MAX_NUM; k++) {
        head_virt = (td_u8 *)isp_vreg_cfg_buf_addr(vi_pipe, (ot_blk_dev)k);

        pre_be_virt_addr[k] = (td_void *)head_virt;
        post_be_virt_addr[k] = pre_be_virt_addr[k];
        pre_viproc_virt_addr[k] = (td_void *)(head_virt + VIPROC_OFFLINE_OFFSET);
        post_viproc_virt_addr[k] = pre_viproc_virt_addr[k];
    }
}

td_s32 isp_get_reg_cfg_virt_addr(ot_vi_pipe vi_pipe, td_void *pre_be_virt_addr[], td_void *post_be_virt_addr[],
    td_void *pre_viproc_virt_addr[], td_void *post_viproc_virt_addr[])
{
    td_s32 ret;
    isp_working_mode isp_work_mode;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_WORK_MODE_GET, &isp_work_mode);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get isp work mode failed!\n");
        return ret;
    }

    switch (isp_work_mode.running_mode) {
        case ISP_MODE_RUNNING_ONLINE:
            ret = isp_get_reg_cfg_virt_addr_online(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
                post_viproc_virt_addr);
            if (ret != TD_SUCCESS) {
                return ret;
            }
            break;
        case ISP_MODE_RUNNING_OFFLINE:
            isp_get_reg_cfg_virt_addr_offline(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
                post_viproc_virt_addr);
            break;
        case ISP_MODE_RUNNING_STRIPING:
            isp_get_reg_cfg_virt_addr_striping(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
                post_viproc_virt_addr);
            break;
        case ISP_MODE_RUNNING_PRE_ONLINE_POST_OFFLINE:
            isp_get_reg_cfg_virt_addr_pre_online_post_offline(vi_pipe, pre_be_virt_addr, post_be_virt_addr,
                pre_viproc_virt_addr, post_viproc_virt_addr);
            break;
        default:
            isp_err_trace("ISP[%d] GetBe Running Mode Err!\n", vi_pipe);
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_be_vreg_addr_init(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    return isp_get_reg_cfg_virt_addr(vi_pipe, isp_ctx->isp_pre_be_virt_addr, isp_ctx->isp_post_be_virt_addr,
        isp_ctx->pre_viproc_virt_addr, isp_ctx->post_viproc_virt_addr);
}

td_void *isp_get_ldci_read_stt_vir_addr(ot_vi_pipe vi_pipe, td_u8 buf_idx)
{
    td_s32 ret;
    td_u64 phy_addr_high, phy_addr_tmp;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ldci_read_stt_buf *ldci_read_stt_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->ldci_tpr_flt_en == TD_FALSE) {
        return TD_NULL;
    }

    ldci_buf_get_ctx(vi_pipe, ldci_read_stt_buf);

    if (ldci_read_stt_buf->read_buf[buf_idx].vir_addr != TD_NULL) {
        return ldci_read_stt_buf->read_buf[buf_idx].vir_addr;
    }

    phy_addr_high = (td_u64)ot_ext_system_ldci_read_stt_buffer_high_addr_read(vi_pipe);
    phy_addr_tmp = (td_u64)ot_ext_system_ldci_read_stt_buffer_low_addr_read(vi_pipe);
    phy_addr_tmp |= (phy_addr_high << 32); /* shitf left  32 bit  */

    ret = isp_update_ldci_read_stt_buf_ctx(vi_pipe, phy_addr_tmp);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return ldci_read_stt_buf->read_buf[buf_idx].vir_addr;
}

td_void *isp_get_vicap_ch_virt_addr(ot_vi_pipe vi_pipe)
{
    return isp_get_vicap_chn_reg_virt_addr_base(isp_vicap_ch_reg_base(vi_pipe));
}

td_void *isp_get_fe_vir_addr(ot_vi_pipe vi_pipe)
{
    isp_check_fe_pipe_return_null(vi_pipe);

    return isp_get_fe_reg_virt_addr_base(isp_fe_reg_base(vi_pipe));
}

td_void *isp_get_fe_lut2stt_vir_addr(ot_vi_pipe vi_pipe, td_u8 buf_id)
{
    td_s32 ret;
    td_u64 phy_addr_high, phy_addr_temp;
    isp_fe_lut2stt_attr *fe_lut_buf = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_fe_pipe_return_null(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);
    fe_lut_stt_buf_get_ctx(vi_pipe, fe_lut_buf);

    if (isp_fe_lut_stt_buf_addr_need_init(isp_ctx) != TD_TRUE) {
        return TD_NULL;
    }

    if (fe_lut_buf->lut_stt_buf[buf_id].vir_addr != TD_NULL) {
        return (td_void *)((td_u8 *)fe_lut_buf->lut_stt_buf[buf_id].vir_addr);
    }

    phy_addr_high = (td_u64)ot_ext_system_fe_lut_stt_buffer_high_addr_read(vi_pipe);
    phy_addr_temp = (td_u64)ot_ext_system_fe_lut_stt_buffer_low_addr_read(vi_pipe);
    phy_addr_temp |= (phy_addr_high << 32); /* shitf left  32 bit  */

    ret = isp_update_fe_lut_stt_buf_ctx(vi_pipe, phy_addr_temp);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return (td_void *)((td_u8 *)fe_lut_buf->lut_stt_buf[buf_id].vir_addr);
}

td_void *isp_get_be_lut2stt_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev, td_u8 buf_id)
{
    td_s32 ret;
    size_t size;
    td_u64 phy_addr_high, phy_addr_temp;
    isp_be_lut_buf *be_lut_buf = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);
    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        return TD_NULL;
    }

    size = sizeof(isp_be_lut_wstt_type);

    if (be_lut_buf->lut_stt_buf[blk_dev].vir_addr != TD_NULL) {
        return (td_void *)((td_u8 *)be_lut_buf->lut_stt_buf[blk_dev].vir_addr + size * buf_id);
    }

    phy_addr_high = (td_u64)ot_ext_system_be_lut_stt_buffer_high_addr_read(vi_pipe);
    phy_addr_temp = (td_u64)ot_ext_system_be_lut_stt_buffer_low_addr_read(vi_pipe);
    phy_addr_temp |= (phy_addr_high << 32); /* shitf left  32 bit  */

    ret = isp_update_be_lut_stt_buf_ctx(vi_pipe, phy_addr_temp);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return (td_void *)((td_u8 *)be_lut_buf->lut_stt_buf[blk_dev].vir_addr + size * buf_id);
}

td_void *isp_get_pre_on_post_off_lut2stt_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev, td_u8 buf_id)
{
    td_s32 ret;
    size_t size;
    td_u64 phy_addr_high, phy_addr_temp;
    isp_be_lut_buf *be_lut_buf = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);
    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return TD_NULL;
    }

    size = sizeof(isp_pre_be_lut_wstt_type);

    if (be_lut_buf->lut_stt_buf[blk_dev].vir_addr != TD_NULL) {
        return (td_void *)((td_u8 *)be_lut_buf->lut_stt_buf[blk_dev].vir_addr + size * buf_id);
    }

    phy_addr_high = (td_u64)ot_ext_system_be_lut_stt_buffer_high_addr_read(vi_pipe);
    phy_addr_temp = (td_u64)ot_ext_system_be_lut_stt_buffer_low_addr_read(vi_pipe);
    phy_addr_temp |= (phy_addr_high << 32); /* shitf left  32 bit  */

    ret = isp_update_be_pre_online_post_offline_lut_stt_buf_ctx(vi_pipe, phy_addr_temp);
    if (ret != TD_SUCCESS) {
        return TD_NULL;
    }

    return (td_void *)((td_u8 *)be_lut_buf->lut_stt_buf[blk_dev].vir_addr + size * buf_id);
}

td_void *isp_get_post_be_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->isp_post_be_virt_addr[blk_dev] != TD_NULL) {
        return isp_ctx->isp_post_be_virt_addr[blk_dev];
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get Be CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(post_be_virt_addr[blk_dev]);

    return post_be_virt_addr[blk_dev];
}

td_void *isp_get_pre_be_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->isp_pre_be_virt_addr[blk_dev] != TD_NULL) {
        return isp_ctx->isp_pre_be_virt_addr[blk_dev];
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get Be CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(pre_be_virt_addr[blk_dev]);

    return pre_be_virt_addr[blk_dev];
}

td_void *isp_get_pre_vi_proc_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->pre_viproc_virt_addr[blk_dev] != TD_NULL) {
        return isp_ctx->pre_viproc_virt_addr[blk_dev];
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get viproc CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(pre_viproc_virt_addr[blk_dev]);

    return pre_viproc_virt_addr[blk_dev];
}

td_void *isp_get_post_vi_proc_vir_addr(ot_vi_pipe vi_pipe, ot_blk_dev blk_dev)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_pipe_valid_return(vi_pipe);
    isp_check_be_dev_return(blk_dev);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->post_viproc_virt_addr[blk_dev] != TD_NULL) {
        return isp_ctx->post_viproc_virt_addr[blk_dev];
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get viproc CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(post_viproc_virt_addr[blk_dev]);

    return post_viproc_virt_addr[blk_dev];
}

td_void *isp_get_be_ae_vir_addr(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_fe_pipe_return_null(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->isp_be_ae_virt_addr != TD_NULL) {
        return isp_ctx->isp_be_ae_virt_addr;
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get Be CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(isp_ctx->isp_be_ae_virt_addr);

    return isp_ctx->isp_be_ae_virt_addr;
}

td_void *isp_get_be_awb_vir_addr(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_void *pre_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_be_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *pre_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };
    td_void *post_viproc_virt_addr[OT_ISP_STRIPING_MAX_NUM] = { TD_NULL };

    isp_check_fe_pipe_return_null(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->isp_be_awb_virt_addr != TD_NULL) {
        return isp_ctx->isp_be_awb_virt_addr;
    }

    ret = isp_get_reg_cfg_virt_addr(vi_pipe, pre_be_virt_addr, post_be_virt_addr, pre_viproc_virt_addr,
        post_viproc_virt_addr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Get Be CfgAddr Failed!\n", vi_pipe);
        return TD_NULL;
    }

    isp_check_reg_nullptr_return(isp_ctx->isp_be_awb_virt_addr);

    return isp_ctx->isp_be_awb_virt_addr;
}


/* fe */
static td_s32 isp_lblc_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 buf_id, isp_vicap_ch_reg_type *vicap_ch_reg)
{
    td_u64 phys_addr;
    isp_fe_lut2stt_attr *fe_lut_buf = TD_NULL;

    fe_lut_stt_buf_get_ctx(vi_pipe, fe_lut_buf);

    phys_addr = fe_lut_buf->lut_stt_buf[buf_id].phy_addr;
    if (phys_addr == 0) {
        return TD_FAILURE;
    }

    isp_fe_lblc_lut_addr_low_write(vicap_ch_reg, get_low_addr(phys_addr));
    isp_fe_lblc_lut_addr_high_write(vicap_ch_reg, get_high_addr(phys_addr));

    return TD_SUCCESS;
}

/* pre be */
static td_s32 isp_dpc_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *pre_viproc)
{
    td_u64 phy_addr;
    size_t size;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    isp_dpc_lut_addr_low_write(pre_viproc, get_low_addr(phy_addr + ISP_DPC_LUT_WSTT_OFFSET + buf_id * size));
    isp_dpc_lut_addr_high_write(pre_viproc, get_high_addr(phy_addr + ISP_DPC_LUT_WSTT_OFFSET + buf_id * size));

    return TD_SUCCESS;
}

static td_s32 isp_bnr_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = ISP_BNR_LUT_WSTT_OFFSET + buf_id * size;
    isp_bnr_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_bnr_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));

    return TD_SUCCESS;
}

static td_s32 isp_lsc_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    isp_mlsc_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + ISP_MLSC_LUT_WSTT_OFFSET + buf_id * size));
    isp_mlsc_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + ISP_MLSC_LUT_WSTT_OFFSET + buf_id * size));

    return TD_SUCCESS;
}

/* post be */
static td_s32 isp_clut_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;
    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    offset = POST_BE_WSTT_OFFSET + ISP_CLUT_LUT_WSTT_OFFSET;
    isp_clut_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + offset + buf_id * size));
    isp_clut_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + offset + buf_id * size));
    isp_clut_lut_width_word_write(vi_proc_reg, OT_ISP_CLUT_LUT_WIDTH_WORD_DEFAULT);

    return TD_SUCCESS;
}

static td_s32 isp_gamma_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_GAMMA_LUT_WSTT_OFFSET + buf_id * size;
    isp_gamma_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_gamma_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));
    return TD_SUCCESS;
}
static td_s32 isp_dehaze_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id,
    isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_DEHAZE_LUT_WSTT_OFFSET + buf_id * size;
    isp_dehaze_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_dehaze_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));
    return TD_SUCCESS;
}

static td_s32 isp_sharpen_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id,
    isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_SHARPEN_LUT_WSTT_OFFSET + buf_id * size;
    isp_sharpen_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_sharpen_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));
    return TD_SUCCESS;
}

static td_s32 isp_ldci_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_LDCI_LUT_WSTT_OFFSET + buf_id * size;
    isp_ldci_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_ldci_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));

    return TD_SUCCESS;
}

static td_s32 isp_ca_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;
    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    offset = POST_BE_WSTT_OFFSET + ISP_CA_LUT_WSTT_OFFSET;
    isp_ca_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + offset + buf_id * size));
    isp_ca_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + offset + buf_id * size));
    isp_ca_lut_width_word_write(vi_proc_reg, OT_ISP_CA_LUT_WIDTH_WORD_DEFAULT);

    return TD_SUCCESS;
}

td_s32 isp_cc_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_CC_LUT_WSTT_OFFSET + buf_id * size;
    isp_cc_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_cc_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));

    return TD_SUCCESS;
}

td_s32 isp_dmnr_lut_wstt_addr_write(ot_vi_pipe vi_pipe, td_u8 i, td_u8 buf_id, isp_viproc_reg_type *vi_proc_reg)
{
    td_u64 phy_addr;
    size_t size, addr_offset;
    isp_be_lut_buf *be_lut_buf = TD_NULL;

    be_lut_buf_get_ctx(vi_pipe, be_lut_buf);

    phy_addr = be_lut_buf->lut_stt_buf[i].phy_addr;

    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    size = sizeof(isp_be_lut_wstt_type);
    addr_offset = POST_BE_WSTT_OFFSET + ISP_DMNR_LUT_WSTT_OFFSET + buf_id * size;
    isp_dmnr_lut_addr_low_write(vi_proc_reg, get_low_addr(phy_addr + addr_offset));
    isp_dmnr_lut_addr_high_write(vi_proc_reg, get_high_addr(phy_addr + addr_offset));

    return TD_SUCCESS;
}

static td_void isp_fe_nr_dgain_reg_config(ot_vi_pipe vi_pipe, isp_fe_reg_type *fe_reg,
    const isp_fe_dg_reg_cfg *fe_dg_reg_cfg)
{
    td_u8  fe_nr_dgain_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };
    if (fe_nr_dgain_en[vi_pipe] == 0) {
        return;
    }
    isp_fe_dg1_en_write(fe_reg, TD_TRUE);

    if (fe_dg_reg_cfg->dyna_reg_cfg.resh) {
        isp_fe_dg1_rgain_write(fe_reg, 0x100);
        isp_fe_dg1_grgain_write(fe_reg, 0x100);
        isp_fe_dg1_gbgain_write(fe_reg, 0x100);
        isp_fe_dg1_bgain_write(fe_reg, 0x100);
        isp_fe_dg1_clip_value_write(fe_reg, 0xFFFFF);
    }
}
static td_s32 isp_fe_dg_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u16 i;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_dg_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_check_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[0].fe_dg_reg_cfg.dyna_reg_cfg;

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        if (reg_cfg_info->cfg_key.bit1_fe_dg_cfg) {
            isp_fe_dg2_en_write(fe_reg, reg_cfg_info->alg_reg_cfg[0].fe_dg_reg_cfg.dg_en);

            if (dyna_reg_cfg->resh) {
                isp_fe_dg2_rgain_write(fe_reg, dyna_reg_cfg->gain_r[i]);
                isp_fe_dg2_grgain_write(fe_reg, dyna_reg_cfg->gain_gr[i]);
                isp_fe_dg2_gbgain_write(fe_reg, dyna_reg_cfg->gain_gb[i]);
                isp_fe_dg2_bgain_write(fe_reg, dyna_reg_cfg->gain_b[i]);
                isp_fe_dg2_clip_value_write(fe_reg, dyna_reg_cfg->clip_value);
            }
            isp_fe_nr_dgain_reg_config(vi_pipe_bind, fe_reg, &reg_cfg_info->alg_reg_cfg[0].fe_dg_reg_cfg);
        }
    }

    dyna_reg_cfg->resh = TD_FALSE;
    reg_cfg_info->cfg_key.bit1_fe_dg_cfg = 0;

    return TD_SUCCESS;
}

static td_void isp_fe_blc_static_reg_config(ot_vi_pipe vi_pipe_bind, isp_fe_reg_type *fe_reg,
                                            const isp_fe_blc_static_cfg *static_cfg)
{
    td_u8  af_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };
    td_u8  fe_nr_dg_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };
    if (static_cfg->resh_static) {
        isp_fe_dg2_blc_en_write(fe_reg, static_cfg->fe_dg_blc.blc_in, static_cfg->fe_dg_blc.blc_out);
        isp_fe_dg2_en_comp_write(fe_reg, static_cfg->fe_dg_blc.blc_compensation_en);
        isp_fe_dg2_ofs_shift_write(fe_reg, static_cfg->fe_dg_offset_shift);
        isp_fe_wb_blc_en_write(fe_reg, static_cfg->fe_wb_blc.blc_in, static_cfg->fe_wb_blc.blc_out);
        isp_fe_ae_blc_en_write(fe_reg, static_cfg->fe_ae_blc.blc_in);
        if (af_en[vi_pipe_bind] == 1) {
            isp_fe_af_offset_en_write(fe_reg, static_cfg->fe_af_blc.blc_in);
        }

        if (fe_nr_dg_en[vi_pipe_bind] == 1) {
            isp_fe_dg1_en_in_write(fe_reg,  static_cfg->fe_nr_dg_blc.blc_in);
            isp_fe_dg1_en_out_write(fe_reg, static_cfg->fe_nr_dg_blc.blc_out);
            isp_fe_dg1_en_comp_write(fe_reg, static_cfg->fe_nr_dg_blc.blc_compensation_en);
            isp_fe_dg1_ofs_shift_write(fe_reg, static_cfg->fe_nr_dg_offset_shift);
        }
    }
}

static td_void isp_fe_blc_dyna_reg_config(ot_vi_pipe vi_pipe_bind, td_u8 wdr_idx, isp_fe_reg_type *fe_reg,
                                          const isp_fe_blc_dyna_cfg *dyna_cfg)
{
    td_u8  af_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };
    td_u8  fe_nr_dg_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };

    isp_fe_glb_blc_mode_write(fe_reg, dyna_cfg->global_blc.blc_mode);
    isp_fe_glb_tg_mode_write(fe_reg, dyna_cfg->global_blc.target_mode);
    isp_fe_global_blc_write(fe_reg, dyna_cfg->global_blc.global_blc1[wdr_idx]);
    isp_fe_target_blc_write(fe_reg, dyna_cfg->global_blc.target_blc[wdr_idx]);
    isp_fe_blc1_en_write(fe_reg, dyna_cfg->fe_blc1_en);
    isp_fe_blc1_offset_write(fe_reg, dyna_cfg->fe_blc[wdr_idx].blc);
    if (fe_nr_dg_en[vi_pipe_bind] == 1) {
        isp_fe_dg1_ofsr_write(fe_reg, dyna_cfg->fe_nr_dg_blc[wdr_idx].blc[OT_ISP_CHN_R]);
        isp_fe_dg1_ofsgr_write(fe_reg, dyna_cfg->fe_nr_dg_blc[wdr_idx].blc[OT_ISP_CHN_GR]);
        isp_fe_dg1_ofsgb_write(fe_reg, dyna_cfg->fe_nr_dg_blc[wdr_idx].blc[OT_ISP_CHN_GB]);
        isp_fe_dg1_ofsb_write(fe_reg, dyna_cfg->fe_nr_dg_blc[wdr_idx].blc[OT_ISP_CHN_B]);
        isp_fe_dg1_real_offset_write(fe_reg, dyna_cfg->fe_nr_dg_real_blc[wdr_idx].blc);
    }

    isp_fe_dg2_offset_write(fe_reg, dyna_cfg->fe_dg_blc[wdr_idx].blc);
    isp_fe_dg2_real_offset_write(fe_reg, dyna_cfg->fe_dg_real_blc[wdr_idx].blc);
    isp_fe_wb_offset_write(fe_reg, dyna_cfg->fe_wb_blc[wdr_idx].blc);
    isp_fe_ae_offset_write(fe_reg, dyna_cfg->fe_ae_blc[wdr_idx].blc);
    if (af_en[vi_pipe_bind] == 1) {
        isp_fe_af_offset_write(fe_reg, dyna_cfg->fe_af_blc[wdr_idx].blc);
    }
}
static td_void isp_fe_blc_fix_update_dg_blc(ot_vi_pipe vi_pipe_bind, isp_fe_reg_type *fe_reg)
{
    td_bool fe_dg_blc_in, fe_dg_blc_out, fe_nr_dg_blc_in, fe_nr_dg_blc_out;
    td_u8 fe_nr_dg_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };

    fe_dg_blc_in = TD_FALSE;
    fe_dg_blc_out = TD_FALSE;
    fe_nr_dg_blc_in = TD_FALSE;
    fe_nr_dg_blc_out = TD_FALSE;
    isp_fe_dg2_blc_en_write(fe_reg, fe_dg_blc_in, fe_dg_blc_out);
    if (fe_nr_dg_en[vi_pipe_bind] == 1) {
        isp_fe_dg1_en_in_write(fe_reg,  fe_nr_dg_blc_in);
        isp_fe_dg1_en_out_write(fe_reg, fe_nr_dg_blc_out);
    }
}
static td_s32 isp_fe_blc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u16 i;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_blc_static_cfg *static_cfg = TD_NULL;
    isp_fe_blc_dyna_cfg *dyna_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    static_cfg = &reg_cfg_info->alg_reg_cfg[0].fe_blc_cfg.static_blc;
    dyna_cfg = &reg_cfg_info->alg_reg_cfg[0].fe_blc_cfg.dyna_blc;

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        if (reg_cfg_info->cfg_key.bit1_fe_blc_cfg) {
            isp_fe_blc_static_reg_config(vi_pipe_bind, fe_reg, static_cfg);
            if (isp_ctx->linkage.blc_fix.enable == TD_TRUE) {
                isp_fe_blc_fix_update_dg_blc(vi_pipe_bind, fe_reg);
            }
            if (reg_cfg_info->alg_reg_cfg[0].fe_blc_cfg.resh_dyna_init == TD_TRUE) {
                isp_fe_blc_dyna_reg_config(vi_pipe_bind, i, fe_reg, dyna_cfg);
            }
        }
    }

    static_cfg->resh_static = TD_FALSE;
    reg_cfg_info->alg_reg_cfg[0].fe_blc_cfg.resh_dyna_init = TD_FALSE;
    reg_cfg_info->cfg_key.bit1_fe_blc_cfg = 0;

    return TD_SUCCESS;
}

static td_s32 isp_aibnr_fpn_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u16 i;
    ot_vi_pipe vi_pipe_bind;
    td_u8 aibnr_fpn_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_AIBNR_FPN_ENABLE };
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_fpn_dyna_cfg *dyna_reg_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        if (aibnr_fpn_en[vi_pipe_bind] != 1) {
            continue;
        }
        if (reg_cfg_info->cfg_key.bit1_aibnr_fpn_cfg) {
            fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
            isp_check_pointer_return(fe_reg);
            dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[0].aibnr_fpn_reg_cfg.dyna_reg_cfg;
            isp_aibnr_fpn_overflowthr_write(fe_reg, dyna_reg_cfg->isp_fpn_overflow_thr);
            isp_aibnr_fpn_strength0_write(fe_reg, dyna_reg_cfg->isp_fpn_strength[0]);
        }
    }

    reg_cfg_info->cfg_key.bit1_aibnr_fpn_cfg = 0;

    return TD_SUCCESS;
}

static td_s32 isp_dynamic_blc_reg_config_all_wdr_pipe(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u16 i;
    ot_vi_pipe vi_pipe_bind;
    isp_dynamicblc_usr_cfg *usr_cfg = TD_NULL;
    isp_dynamicblc_static_cfg *static_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    static_cfg = &reg_cfg_info->alg_reg_cfg[0].dynamic_blc_reg_cfg.static_reg_cfg;
    usr_cfg = &reg_cfg_info->alg_reg_cfg[0].dynamic_blc_reg_cfg.usr_reg_cfg;

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        isp_fe_blc_dyn_en_write(fe_reg, reg_cfg_info->alg_reg_cfg[0].dynamic_blc_reg_cfg.dynablc_enable);

        if (reg_cfg_info->cfg_key.bit1_dynablc_cfg) {
            if (static_cfg->static_resh) {
                isp_fe_blc_dynamic_raw_array_write(fe_reg, static_cfg->dynablc_raw_array);
                isp_fe_blc_dynamic_dp_stat_en_write(fe_reg, static_cfg->dynablc_dpstat_en);
            }

            if (usr_cfg->usr_resh) {
                isp_fe_blc_dynamic_bis2x2pattern_write(fe_reg, usr_cfg->dynablc_pattern);
                isp_fe_blc_dynamic_start_xpos_write(fe_reg, usr_cfg->dynablc_start_xpos);
                isp_fe_blc_dynamic_start_ypos_write(fe_reg, usr_cfg->dynablc_start_ypos);
                isp_fe_blc_dynamic_end_xpos_write(fe_reg, usr_cfg->dynablc_end_xpos);
                isp_fe_blc_dynamic_end_ypos_write(fe_reg, usr_cfg->dynablc_end_ypos);
                isp_fe_blc_dynamic_hot_pxl_thd_write(fe_reg, usr_cfg->dynablc_high_threshold);
                isp_fe_blc_dynamic_dead_pxl_thd_write(fe_reg, usr_cfg->dynablc_low_threshold);
                isp_fe_blc_dynamic_bfilter_en_write(fe_reg, usr_cfg->bfilter_en);
                isp_fe_blc_dynamic_tar0_write(fe_reg, usr_cfg->dynablc_tar0);
                isp_fe_blc_dynamic_tar1_write(fe_reg, usr_cfg->dynablc_tar1);
            }
        }
    }

    static_cfg->static_resh = TD_FALSE;
    usr_cfg->usr_resh = TD_FALSE;
    reg_cfg_info->cfg_key.bit1_dynablc_cfg = 0;

    return TD_SUCCESS;
}

static td_s32 isp_stitch_dynamic_blc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u8 i;
    td_bool all_init = TD_TRUE;
    ot_vi_pipe stitch_pipe;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_usr_ctx *isp_stitch_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg_attr = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    for (i = 0; i < isp_ctx->stitch_attr.stitch_pipe_num; i++) {
        stitch_pipe = isp_ctx->stitch_attr.stitch_bind_id[i];
        isp_check_pipe_return(stitch_pipe);
        isp_get_ctx(stitch_pipe, isp_stitch_ctx);
        if (isp_stitch_ctx->para_rec.init != TD_TRUE) {
            all_init = TD_FALSE;
            break;
        }
    }

    if (all_init == TD_TRUE) {
        if (isp_ctx->stitch_attr.main_pipe == TD_TRUE) {
            for (i = 0; i < isp_ctx->stitch_attr.stitch_pipe_num; i++) {
                stitch_pipe = isp_ctx->stitch_attr.stitch_bind_id[i];
                isp_check_pipe_return(stitch_pipe);
                isp_regcfg_get_ctx(stitch_pipe, reg_cfg_attr);
                isp_check_pointer_return(reg_cfg_attr);
                isp_dynamic_blc_reg_config_all_wdr_pipe(stitch_pipe, &reg_cfg_attr->reg_cfg);
            }
        }
    } else {
        isp_dynamic_blc_reg_config_all_wdr_pipe(vi_pipe, reg_cfg_info);
    }

    return TD_SUCCESS;
}

static td_s32 isp_fe_dynamicblc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        return isp_stitch_dynamic_blc_reg_config(vi_pipe, reg_cfg_info);
    } else {
        return isp_dynamic_blc_reg_config_all_wdr_pipe(vi_pipe, reg_cfg_info);
    }
}

static td_void isp_fe_ae_weight_config(isp_fe_reg_type *fe_reg, isp_ae_dyna_cfg *dyna_reg_cfg)
{
    td_u16 j, k;
    td_u32 table_weight_tmp;
    td_u32 combin_weight = 0;
    td_u32 combin_weight_num = 0;

    isp_fe_ae_wei_waddr_write(fe_reg, 0);

    for (j = 0; j < OT_ISP_AE_ZONE_ROW; j++) {
        for (k = 0; k < OT_ISP_AE_ZONE_COLUMN; k++) {
            table_weight_tmp = (td_u32)dyna_reg_cfg->fe_weight_table[j][k];
            combin_weight |= (table_weight_tmp << (8 * combin_weight_num)); /* weightTmp shift left 8 */
            combin_weight_num++;

            if (combin_weight_num == OT_ISP_AE_WEI_COMBIN_COUNT) {
                isp_fe_ae_wei_wdata_write(fe_reg, combin_weight);
                combin_weight_num = 0;
                combin_weight = 0;
            }
        }
    }

    if ((combin_weight_num != OT_ISP_AE_WEI_COMBIN_COUNT) && (combin_weight_num != 0)) {
        isp_fe_ae_wei_wdata_write(fe_reg, combin_weight);
    }
}

static td_s32 isp_fe_ae_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_bool lut_update_en = TD_FALSE;
    td_u16 i, crop_width, crop_x;
    ot_vi_pipe vi_pipe_bind;
    isp_ae_static_cfg *static_reg = TD_NULL;
    isp_ae_dyna_cfg *dyna_reg = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    if (reg_cfg_info->cfg_key.bit1_ae_cfg1) {
        for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
            vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
            isp_check_no_fe_pipe_return(vi_pipe_bind);

            fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
            isp_check_pointer_return(fe_reg);
            /* ae fe static */
            static_reg = &reg_cfg_info->alg_reg_cfg[0].ae_reg_cfg.static_reg_cfg;
            dyna_reg = &reg_cfg_info->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg;

            isp_fe_ae_en_write(fe_reg, static_reg->fe_enable);
            crop_x = is_hrs_on(vi_pipe_bind) ? (static_reg->fe_crop_pos_x >> 1) : static_reg->fe_crop_pos_x;
            isp_fe_ae_crop_pos_write(fe_reg, crop_x, static_reg->fe_crop_pos_y);
            crop_width = is_hrs_on(vi_pipe_bind) ? (static_reg->fe_crop_out_width >> 1) : static_reg->fe_crop_out_width;
            isp_fe_ae_crop_outsize_write(fe_reg, crop_width - 1, static_reg->fe_crop_out_height - 1);

            /* ae fe dynamic */
            isp_fe_ae_zone_write(fe_reg, dyna_reg->fe_weight_table_width, dyna_reg->fe_weight_table_height);
            isp_fe_ae_skip_crg_write(fe_reg, dyna_reg->fe_hist_skip_x, dyna_reg->fe_hist_offset_x,
                dyna_reg->fe_hist_skip_y, dyna_reg->fe_hist_offset_y);
            isp_fe_ae_bitmove_write(fe_reg, dyna_reg->fe_bit_move, dyna_reg->fe_hist_gamma_mode,
                dyna_reg->fe_aver_gamma_mode);
            isp_fe_ae_gamma_limit_write(fe_reg, dyna_reg->fe_gamma_limit);
            isp_fe_ae_fourplanemode_write(fe_reg, dyna_reg->fe_four_plane_mode);

            isp_fe_ae_weight_config(fe_reg, dyna_reg);

            lut_update_en = dyna_reg->fe_weight_table_update;
        }
    }

    reg_cfg_info->alg_reg_cfg[0].fe_lut_update_cfg.ae1_lut_update = lut_update_en;
    return TD_SUCCESS;
}

static td_void isp_combine_ae_wgt_calc(isp_ae_dyna_cfg *dyna_reg_be_cfg)
{
    td_u16 j, k, m;
    td_u32 combin_weight, combin_weight_num;

    m = 0;
    combin_weight = 0;
    combin_weight_num = 0;

    for (j = 0; j < dyna_reg_be_cfg->be_weight_table_height; j++) {
        for (k = 0; k < dyna_reg_be_cfg->be_weight_table_width; k++) {
            combin_weight |= ((td_u32)dyna_reg_be_cfg->be_weight_table[j][k] << (8 * combin_weight_num)); /* 8 */
            combin_weight_num++;

            if (combin_weight_num == OT_ISP_AE_WEI_COMBIN_COUNT) {
                dyna_reg_be_cfg->combine_wgt[m++] = combin_weight;
                combin_weight_num = 0;
                combin_weight = 0;
            }
        }
    }

    if ((combin_weight_num != OT_ISP_AE_WEI_COMBIN_COUNT) && (combin_weight_num != 0)) {
        dyna_reg_be_cfg->combine_wgt[m++] = combin_weight;
    }
}

static td_void isp_ae_reg_write(isp_viproc_reg_type *viproc_reg, isp_be_reg_type *be_reg, isp_ae_reg_cfg *ae_reg_cfg)
{
    td_u8 m;
    isp_ae_static_cfg *static_reg_cfg = &ae_reg_cfg->static_reg_cfg;
    isp_ae_dyna_cfg *dyna_reg_cfg = &ae_reg_cfg->dyna_reg_cfg;

    isp_combine_ae_wgt_calc(dyna_reg_cfg);
    isp_ae_en_write(viproc_reg, static_reg_cfg->be_enable);

    isp_ae_crop_pos_x_write(be_reg, static_reg_cfg->be_crop_pos_x);
    isp_ae_crop_pos_y_write(be_reg, static_reg_cfg->be_crop_pos_y);
    isp_ae_crop_out_width_write(be_reg, static_reg_cfg->be_crop_out_width - 1);
    isp_ae_crop_out_height_write(be_reg, static_reg_cfg->be_crop_out_height - 1);
    /* ae be dynamic */
    isp_ae_sel_write(be_reg, dyna_reg_cfg->be_ae_sel);
    isp_ae_hnum_write(be_reg, dyna_reg_cfg->be_weight_table_width);
    isp_ae_vnum_write(be_reg, dyna_reg_cfg->be_weight_table_height);
    isp_ae_skip_x_write(be_reg, dyna_reg_cfg->be_hist_skip_x);
    isp_ae_offset_x_write(be_reg, dyna_reg_cfg->be_hist_offset_x);
    isp_ae_skip_y_write(be_reg, dyna_reg_cfg->be_hist_skip_y);
    isp_ae_offset_y_write(be_reg, dyna_reg_cfg->be_hist_offset_y);
    isp_ae_bitmove_write(be_reg, dyna_reg_cfg->be_bit_move);
    isp_ae_hist_gamma_mode_write(be_reg, dyna_reg_cfg->be_hist_gamma_mode);
    isp_ae_aver_gamma_mode_write(be_reg, dyna_reg_cfg->be_aver_gamma_mode);
    isp_ae_gamma_limit_write(be_reg, dyna_reg_cfg->be_gamma_limit);
    isp_ae_fourplanemode_write(be_reg, dyna_reg_cfg->be_four_plane_mode);
    if (dyna_reg_cfg->is_online == TD_TRUE) {
        isp_ae_wei_waddr_write(be_reg, 0);

        for (m = 0; m < 64; m++) { /* ae weight table number 64 */
            isp_ae_wei_wdata_write(be_reg, dyna_reg_cfg->combine_wgt[m]);
        }
    } else {
        isp_ae_weight_write(&be_reg->be_lut.be_apb_lut, dyna_reg_cfg->combine_wgt);
    }
}
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
static td_s32 isp_cfg_ae_to_extend_reg(ot_vi_pipe vi_pipe, td_u32 offset, td_u32 w, td_u32 h)
{
    td_s32 ret;
    td_u8 *be_ae_vir_addr = TD_NULL;
    isp_be_ae_reg_type *be_ae_reg = TD_NULL;
    td_u8 *be_vir_addr = TD_NULL;
    be_vir_addr = (td_u8 *)isp_get_pre_be_vir_addr(vi_pipe, 0); // one buf head addr
    isp_check_pointer_return(be_vir_addr);

    be_ae_vir_addr = (td_u8 *)isp_get_be_ae_vir_addr(vi_pipe); // extend_addr
    isp_check_pointer_return(be_ae_vir_addr);
    ret = memcpy_s(be_ae_vir_addr + offset, ISP_DETAIL_STATS_AE_CFG_SIZE,
        (be_vir_addr + ISP_BE4_CFG_LOAD_ADDR_OFFSET), ISP_DETAIL_STATS_AE_CFG_SIZE);
    isp_check_eok_return(ret, TD_FAILURE);

    be_ae_reg = (isp_be_ae_reg_type *)(be_ae_vir_addr + offset);
    isp_be_ae_crop_pos_x_write(be_ae_reg, 0);
    isp_be_ae_crop_pos_y_write(be_ae_reg, 0);
    isp_be_ae_crop_out_width_write(be_ae_reg, w - 1);
    isp_be_ae_crop_out_height_write(be_ae_reg, h - 1);

    return TD_SUCCESS;
}

static td_s32 isp_cfg_awb_to_extend_reg(ot_vi_pipe vi_pipe, td_u32 offset, td_u32 w, td_u32 h)
{
    td_s32 ret;
    isp_be_awb_reg_type *be_awb_reg = TD_NULL;
    td_u8 *be_awb_vir_addr = TD_NULL;
    td_u8 *be_vir_addr = TD_NULL;
    be_vir_addr = (td_u8 *)isp_get_pre_be_vir_addr(vi_pipe, 0); // one buf head addr
    isp_check_pointer_return(be_vir_addr);

    be_awb_vir_addr = (td_u8 *)isp_get_be_awb_vir_addr(vi_pipe);
    isp_check_pointer_return(be_awb_vir_addr);
    ret = memcpy_s(be_awb_vir_addr + offset, ISP_DETAIL_STATS_AWB_CFG_SIZE,
        (be_vir_addr + ISP_BE5_CFG_LOAD_ADDR_OFFSET), ISP_DETAIL_STATS_AWB_CFG_SIZE);
    isp_check_eok_return(ret, TD_FAILURE);

    be_awb_reg = (isp_be_awb_reg_type *)(be_awb_vir_addr + offset);
    isp_be_awb_crop_pos_x_write(be_awb_reg, 0);
    isp_be_awb_crop_pos_y_write(be_awb_reg, 0);
    isp_be_awb_crop_out_width_write(be_awb_reg, w - 1);
    isp_be_awb_crop_out_height_write(be_awb_reg, h - 1);
    isp_be_awb_hnum_write(be_awb_reg, OT_ISP_AWB_ZONE_ORIG_COLUMN);
    isp_be_awb_vnum_write(be_awb_reg, OT_ISP_AWB_ZONE_ORIG_ROW);

    return TD_SUCCESS;
}

static td_s32 isp_reg_copy_to_extend_reg(ot_vi_pipe vi_pipe)
{
    td_s32 i, j, row, col, ret;
    td_u32 offset, w, h, size;

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    ret = TD_SUCCESS;
    if (isp_ctx->detail_stats_info.stats_cfg.enable == TD_FALSE) {
        return TD_SUCCESS;
    }
    if (is_offline_mode(isp_ctx->block_attr.running_mode) != TD_TRUE) {
        isp_err_trace("detail_stats is enable, but running mode is not offline:%d\n", isp_ctx->block_attr.running_mode);
        return TD_FAILURE;
    }

    row = isp_ctx->detail_stats_info.stats_cfg.row;
    col = isp_ctx->detail_stats_info.stats_cfg.col;
    size = 0;
    if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_ae) {
        size += ISP_DETAIL_STATS_AE_CFG_SIZE;
    }
    if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_awb) {
        size += ISP_DETAIL_STATS_AWB_CFG_SIZE;
    }

    for (i = 0; i < row ; i++) {
        for (j = 0; j < col; j++) {
            w = isp_ctx->detail_stats_info.detail_size.split_width[j];
            h = isp_ctx->detail_stats_info.detail_size.split_height[i];

            offset = (j + i * col) * size;
            if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_ae) {
                ret += isp_cfg_ae_to_extend_reg(vi_pipe, offset, w, h);
            }
            if (isp_ctx->detail_stats_info.stats_cfg.ctrl.bit1_awb) {
                ret += isp_cfg_awb_to_extend_reg(vi_pipe, offset, w, h);
            }
        }
    }
    return ret;
}
#endif
static td_void isp_la_reg_config(isp_viproc_reg_type *viproc_reg, isp_be_reg_type *be_reg, isp_mg_reg_cfg *mg_reg_cfg)
{
    isp_mg_static_cfg *mg_static_reg_cfg = &mg_reg_cfg->static_reg_cfg;
    isp_mg_dyna_cfg *mg_dyna_reg_cfg = &mg_reg_cfg->dyna_reg_cfg;

    isp_la_crop_pos_x_write(be_reg, mg_static_reg_cfg->crop_pos_x);
    isp_la_crop_pos_y_write(be_reg, mg_static_reg_cfg->crop_pos_y);
    isp_la_crop_out_width_write(be_reg, mg_static_reg_cfg->crop_out_width - 1);
    isp_la_crop_out_height_write(be_reg, mg_static_reg_cfg->crop_out_height - 1);

    /* mg dynamic */
    isp_la_hnum_write(be_reg, mg_dyna_reg_cfg->zone_width);
    isp_la_vnum_write(be_reg, mg_dyna_reg_cfg->zone_height);
    isp_la_bitmove_write(be_reg, mg_dyna_reg_cfg->bit_move);
    isp_la_gamma_en_write(be_reg, mg_dyna_reg_cfg->gamma_mode);
    isp_la_gamma_limit_write(be_reg, mg_dyna_reg_cfg->gamma_limit);

    isp_la_en_write(viproc_reg, mg_static_reg_cfg->enable);
}

static td_s32 isp_ae_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool lut_update_en = TD_FALSE;
    td_bool offline_mode;
    isp_ae_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;
    isp_be_reg_type *be_reg = TD_NULL;

    dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg;
    offline_mode = (dyna_reg_cfg->is_online == TD_FALSE);
    if (reg_cfg_info->cfg_key.bit1_ae_cfg1) {
        if (dyna_reg_cfg->is_in_pre_be == TD_FALSE) {
            viproc_reg = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
            be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        } else {
            viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
            be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        }
        isp_check_pointer_return(viproc_reg);
        isp_check_pointer_return(be_reg);

        isp_ae_reg_write(viproc_reg, be_reg, &reg_cfg_info->alg_reg_cfg[i].ae_reg_cfg);

        /* la */
        if (reg_cfg_info->alg_reg_cfg[i].mg_reg_cfg.dyna_reg_cfg.is_in_pre_be == TD_FALSE) {
            viproc_reg = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
            be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        } else {
            viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
            be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        }

        isp_check_pointer_return(viproc_reg);
        isp_check_pointer_return(be_reg);

        isp_la_reg_config(viproc_reg, be_reg, &reg_cfg_info->alg_reg_cfg[i].mg_reg_cfg);

        lut_update_en = reg_cfg_info->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg.be_weight_table_update;
    }

    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.ae_lut_update = lut_update_en || offline_mode;

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_AF_SUPPORT
static td_void isp_fe_af_iir_gain_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_iirg0_0_write(fe_reg, af_reg_fe_cfg->iir_gain0_group0);
    isp_fe_af_iirg0_1_write(fe_reg, af_reg_fe_cfg->iir_gain0_group1);
    isp_fe_af_iirg1_0_write(fe_reg, af_reg_fe_cfg->iir_gain1_group0);
    isp_fe_af_iirg1_1_write(fe_reg, af_reg_fe_cfg->iir_gain1_group1);
    isp_fe_af_iirg2_0_write(fe_reg, af_reg_fe_cfg->iir_gain2_group0);
    isp_fe_af_iirg2_1_write(fe_reg, af_reg_fe_cfg->iir_gain2_group1);
    isp_fe_af_iirg3_0_write(fe_reg, af_reg_fe_cfg->iir_gain3_group0);
    isp_fe_af_iirg3_1_write(fe_reg, af_reg_fe_cfg->iir_gain3_group1);
    isp_fe_af_iirg4_0_write(fe_reg, af_reg_fe_cfg->iir_gain4_group0);
    isp_fe_af_iirg4_1_write(fe_reg, af_reg_fe_cfg->iir_gain4_group1);
    isp_fe_af_iirg5_0_write(fe_reg, af_reg_fe_cfg->iir_gain5_group0);
    isp_fe_af_iirg5_1_write(fe_reg, af_reg_fe_cfg->iir_gain5_group1);
    isp_fe_af_iirg6_0_write(fe_reg, af_reg_fe_cfg->iir_gain6_group0);
    isp_fe_af_iirg6_1_write(fe_reg, af_reg_fe_cfg->iir_gain6_group1);
}

static td_void isp_fe_af_iir_shift_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_iirshift0_0_write(fe_reg, af_reg_fe_cfg->iir0_shift_group0);
    isp_fe_af_iirshift0_1_write(fe_reg, af_reg_fe_cfg->iir1_shift_group0);
    isp_fe_af_iirshift0_2_write(fe_reg, af_reg_fe_cfg->iir2_shift_group0);
    isp_fe_af_iirshift0_3_write(fe_reg, af_reg_fe_cfg->iir3_shift_group0);
    isp_fe_af_iirshift1_0_write(fe_reg, af_reg_fe_cfg->iir0_shift_group1);
    isp_fe_af_iirshift1_1_write(fe_reg, af_reg_fe_cfg->iir1_shift_group1);
    isp_fe_af_iirshift1_2_write(fe_reg, af_reg_fe_cfg->iir2_shift_group1);
    isp_fe_af_iirshift1_3_write(fe_reg, af_reg_fe_cfg->iir3_shift_group1);
}

static td_void isp_fe_af_fir_gain_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_firh0_0_write(fe_reg, af_reg_fe_cfg->fir_h_gain0_group0);
    isp_fe_af_firh0_1_write(fe_reg, af_reg_fe_cfg->fir_h_gain0_group1);

    isp_fe_af_firh1_0_write(fe_reg, af_reg_fe_cfg->fir_h_gain1_group0);
    isp_fe_af_firh1_1_write(fe_reg, af_reg_fe_cfg->fir_h_gain1_group1);

    isp_fe_af_firh2_0_write(fe_reg, af_reg_fe_cfg->fir_h_gain2_group0);
    isp_fe_af_firh2_1_write(fe_reg, af_reg_fe_cfg->fir_h_gain2_group1);

    isp_fe_af_firh3_0_write(fe_reg, af_reg_fe_cfg->fir_h_gain3_group0);
    isp_fe_af_firh3_1_write(fe_reg, af_reg_fe_cfg->fir_h_gain3_group1);

    isp_fe_af_firh4_0_write(fe_reg, af_reg_fe_cfg->fir_h_gain4_group0);
    isp_fe_af_firh4_1_write(fe_reg, af_reg_fe_cfg->fir_h_gain4_group1);
}

static td_void isp_fe_af_crop_write(ot_vi_pipe vi_pipe, isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    td_u16 crop_width, crop_pos_x;

    isp_fe_af_crop_en_write(fe_reg, af_reg_fe_cfg->fe_crop_enable);
    if (af_reg_fe_cfg->fe_crop_enable) {
        crop_pos_x = is_hrs_on(vi_pipe) ? (af_reg_fe_cfg->fe_crop_pos_x >> 1) : af_reg_fe_cfg->fe_crop_pos_x;
        isp_fe_af_pos_x_write(fe_reg, crop_pos_x);
        isp_fe_af_pos_y_write(fe_reg, af_reg_fe_cfg->fe_crop_pos_y);

        crop_width = is_hrs_on(vi_pipe) ? (af_reg_fe_cfg->fe_crop_h_size >> 1) : af_reg_fe_cfg->fe_crop_h_size;
        crop_width = isp_alignup(crop_width, 8); /* align up 8 */
        isp_fe_af_crop_hsize_write(fe_reg, crop_width - 1);
        isp_fe_af_crop_vsize_write(fe_reg, af_reg_fe_cfg->fe_crop_v_size - 1);
    }
}

static td_void isp_fe_af_raw_cfg_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_raw_mode_write(fe_reg, af_reg_fe_cfg->raw_mode);
    isp_fe_af_gamma_write(fe_reg, af_reg_fe_cfg->gamma);
    isp_fe_af_bayer_mode_write(fe_reg, af_reg_fe_cfg->bayer_mode);
}

static td_void isp_fe_af_level_depend_gain_cfg_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_iir0_ldg_en_write(fe_reg, af_reg_fe_cfg->iir0_ldg_enable);
    isp_fe_af_iir_thre0_l_write(fe_reg, af_reg_fe_cfg->iir_thre0_low);
    isp_fe_af_iir_thre0_h_write(fe_reg, af_reg_fe_cfg->iir_thre0_high);
    isp_fe_af_iir_slope0_l_write(fe_reg, af_reg_fe_cfg->iir_slope0_low);
    isp_fe_af_iir_slope0_h_write(fe_reg, af_reg_fe_cfg->iir_slope0_high);
    isp_fe_af_iir_gain0_l_write(fe_reg, af_reg_fe_cfg->iir_gain0_low);
    isp_fe_af_iir_gain0_h_write(fe_reg, af_reg_fe_cfg->iir_gain0_high);

    isp_fe_af_iir1_ldg_en_write(fe_reg, af_reg_fe_cfg->iir1_ldg_enable);
    isp_fe_af_iir_thre1_l_write(fe_reg, af_reg_fe_cfg->iir_thre1_low);
    isp_fe_af_iir_thre1_h_write(fe_reg, af_reg_fe_cfg->iir_thre1_high);
    isp_fe_af_iir_slope1_l_write(fe_reg, af_reg_fe_cfg->iir_slope1_low);
    isp_fe_af_iir_slope1_h_write(fe_reg, af_reg_fe_cfg->iir_slope1_high);
    isp_fe_af_iir_gain1_l_write(fe_reg, af_reg_fe_cfg->iir_gain1_low);
    isp_fe_af_iir_gain1_h_write(fe_reg, af_reg_fe_cfg->iir_gain1_high);

    isp_fe_af_fir0_ldg_en_write(fe_reg, af_reg_fe_cfg->fir0_ldg_enable);
    isp_fe_af_fir_thre0_l_write(fe_reg, af_reg_fe_cfg->fir_thre0_low);
    isp_fe_af_fir_thre0_h_write(fe_reg, af_reg_fe_cfg->fir_thre0_high);
    isp_fe_af_fir_slope0_l_write(fe_reg, af_reg_fe_cfg->fir_slope0_low);
    isp_fe_af_fir_slope0_h_write(fe_reg, af_reg_fe_cfg->fir_slope0_high);
    isp_fe_af_fir_gain0_l_write(fe_reg, af_reg_fe_cfg->fir_gain0_low);
    isp_fe_af_fir_gain0_h_write(fe_reg, af_reg_fe_cfg->fir_gain0_high);

    isp_fe_af_fir1_ldg_en_write(fe_reg, af_reg_fe_cfg->fir1_ldg_enable);
    isp_fe_af_fir_thre1_l_write(fe_reg, af_reg_fe_cfg->fir_thre1_low);
    isp_fe_af_fir_thre1_h_write(fe_reg, af_reg_fe_cfg->fir_thre1_high);
    isp_fe_af_fir_slope1_l_write(fe_reg, af_reg_fe_cfg->fir_slope1_low);
    isp_fe_af_fir_slope1_h_write(fe_reg, af_reg_fe_cfg->fir_slope1_high);
    isp_fe_af_fir_gain1_l_write(fe_reg, af_reg_fe_cfg->fir_gain1_low);
    isp_fe_af_fir_gain1_h_write(fe_reg, af_reg_fe_cfg->fir_gain1_high);
}

static td_void isp_fe_af_coring_cfg_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_iir_thre0_c_write(fe_reg, af_reg_fe_cfg->iir_thre0_coring);
    isp_fe_af_iir_slope0_c_write(fe_reg, af_reg_fe_cfg->iir_slope0_coring);
    isp_fe_af_iir_peak0_c_write(fe_reg, af_reg_fe_cfg->iir_peak0_coring);

    isp_fe_af_iir_thre1_c_write(fe_reg, af_reg_fe_cfg->iir_thre1_coring);
    isp_fe_af_iir_slope1_c_write(fe_reg, af_reg_fe_cfg->iir_slope1_coring);
    isp_fe_af_iir_peak1_c_write(fe_reg, af_reg_fe_cfg->iir_peak1_coring);

    isp_fe_af_fir_thre0_c_write(fe_reg, af_reg_fe_cfg->fir_thre0_coring);
    isp_fe_af_fir_slope0_c_write(fe_reg, af_reg_fe_cfg->fir_slope0_coring);
    isp_fe_af_fir_peak0_c_write(fe_reg, af_reg_fe_cfg->fir_peak0_coring);

    isp_fe_af_fir_thre1_c_write(fe_reg, af_reg_fe_cfg->fir_thre1_coring);
    isp_fe_af_fir_slope1_c_write(fe_reg, af_reg_fe_cfg->fir_slope1_coring);
    isp_fe_af_fir_peak1_c_write(fe_reg, af_reg_fe_cfg->fir_peak1_coring);
}

static td_void isp_fe_af_output_shift_cfg_write(isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_acc_shift0_h_write(fe_reg, af_reg_fe_cfg->acc_shift0_h);
    isp_fe_af_acc_shift1_h_write(fe_reg, af_reg_fe_cfg->acc_shift1_h);
    isp_fe_af_acc_shift0_v_write(fe_reg, af_reg_fe_cfg->acc_shift0_v);
    isp_fe_af_acc_shift1_v_write(fe_reg, af_reg_fe_cfg->acc_shift1_v);
    isp_fe_af_acc_shift_y_write(fe_reg, af_reg_fe_cfg->acc_shift_y);
    isp_fe_af_cnt_shift_y_write(fe_reg, af_reg_fe_cfg->shift_count_y);
    isp_fe_af_cnt_shift0_h_write(fe_reg, 0x0);
    isp_fe_af_cnt_shift1_h_write(fe_reg, 0x0);
    isp_fe_af_cnt_shift0_v_write(fe_reg, ISP_AF_CNT_SHIFT0_V_DEFAULT);
    isp_fe_af_cnt_shift1_v_write(fe_reg, 0x0);
}

static td_void isp_fe_af_reg_write(ot_vi_pipe vi_pipe, isp_fe_reg_type *fe_reg, isp_af_reg_cfg *af_reg_fe_cfg)
{
    isp_fe_af_en_write(fe_reg, af_reg_fe_cfg->af_enable);

    isp_fe_af_lpf_en_write(fe_reg, af_reg_fe_cfg->lpf_enable);
    isp_fe_af_fir0_lpf_en_write(fe_reg, af_reg_fe_cfg->fir0_lpf_enable);
    isp_fe_af_fir1_lpf_en_write(fe_reg, af_reg_fe_cfg->fir1_lpf_enable);
    isp_fe_af_iir0_ds_en_write(fe_reg, af_reg_fe_cfg->iir0_ds_enable);
    isp_fe_af_iir1_ds_en_write(fe_reg, af_reg_fe_cfg->iir1_ds_enable);
    isp_fe_af_iir_dilate0_write(fe_reg, af_reg_fe_cfg->iir_dilate0);
    isp_fe_af_iir_dilate1_write(fe_reg, af_reg_fe_cfg->iir_dilate1);
    isp_fe_af_iirplg_0_write(fe_reg, af_reg_fe_cfg->iir_plg_group0);
    isp_fe_af_iirpls_0_write(fe_reg, af_reg_fe_cfg->iir_pls_group0);
    isp_fe_af_iirplg_1_write(fe_reg, af_reg_fe_cfg->iir_plg_group1);
    isp_fe_af_iirpls_1_write(fe_reg, af_reg_fe_cfg->iir_pls_group1);

    isp_fe_af_iir0_en0_write(fe_reg, af_reg_fe_cfg->iir0_enable0);
    isp_fe_af_iir0_en1_write(fe_reg, af_reg_fe_cfg->iir0_enable1);
    isp_fe_af_iir0_en2_write(fe_reg, af_reg_fe_cfg->iir0_enable2);
    isp_fe_af_iir1_en0_write(fe_reg, af_reg_fe_cfg->iir1_enable0);
    isp_fe_af_iir1_en1_write(fe_reg, af_reg_fe_cfg->iir1_enable1);
    isp_fe_af_iir1_en2_write(fe_reg, af_reg_fe_cfg->iir1_enable2);
    isp_fe_af_peak_mode_write(fe_reg, af_reg_fe_cfg->peak_mode);
    isp_fe_af_squ_mode_write(fe_reg, af_reg_fe_cfg->squ_mode);
    isp_fe_af_hnum_write(fe_reg, af_reg_fe_cfg->window_hnum);
    isp_fe_af_vnum_write(fe_reg, af_reg_fe_cfg->window_vnum);

    isp_fe_af_iir_gain_write(fe_reg, af_reg_fe_cfg);
    isp_fe_af_iir_shift_write(fe_reg, af_reg_fe_cfg);
    isp_fe_af_fir_gain_write(fe_reg, af_reg_fe_cfg);

    isp_fe_af_crop_write(vi_pipe, fe_reg, af_reg_fe_cfg);
    isp_fe_af_raw_cfg_write(fe_reg, af_reg_fe_cfg);

    /* AF BE pre median filter */
    isp_fe_af_mean_en_write(fe_reg, af_reg_fe_cfg->mean_enable);
    isp_fe_af_mean_thres_write(fe_reg, 0xFFFF - af_reg_fe_cfg->mean_thres);
    isp_fe_af_level_depend_gain_cfg_write(fe_reg, af_reg_fe_cfg);
    isp_fe_af_coring_cfg_write(fe_reg, af_reg_fe_cfg);

    /* high luma counter */
    isp_fe_af_highlight_write(fe_reg, af_reg_fe_cfg->highlight_thre);

    /* AF output shift */
    isp_fe_af_output_shift_cfg_write(fe_reg, af_reg_fe_cfg);
}
#endif
static td_s32 isp_fe_af_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_u8  af_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };

    td_u32 i;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    if (reg_cfg_info->cfg_key.bit1_af_fe_cfg) {
        for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
            vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
            isp_check_no_fe_pipe_return(vi_pipe_bind);
            if (af_en[vi_pipe_bind] != TD_TRUE) {
                continue;
            }
            fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
            isp_check_pointer_return(fe_reg);

            isp_fe_af_reg_write(vi_pipe_bind, fe_reg, &reg_cfg_info->alg_reg_cfg[0].fe_af_reg_cfg);
        }

        reg_cfg_info->cfg_key.bit1_af_fe_cfg = 0;
    }

#endif

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_AF_SUPPORT
static td_void isp_be_af_iir_gain_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_iirg0_0_write(be_reg, af_reg_be_cfg->iir_gain0_group0);
    isp_af_iirg0_1_write(be_reg, af_reg_be_cfg->iir_gain0_group1);
    isp_af_iirg1_0_write(be_reg, af_reg_be_cfg->iir_gain1_group0);
    isp_af_iirg1_1_write(be_reg, af_reg_be_cfg->iir_gain1_group1);
    isp_af_iirg2_0_write(be_reg, af_reg_be_cfg->iir_gain2_group0);
    isp_af_iirg2_1_write(be_reg, af_reg_be_cfg->iir_gain2_group1);
    isp_af_iirg3_0_write(be_reg, af_reg_be_cfg->iir_gain3_group0);
    isp_af_iirg3_1_write(be_reg, af_reg_be_cfg->iir_gain3_group1);
    isp_af_iirg4_0_write(be_reg, af_reg_be_cfg->iir_gain4_group0);
    isp_af_iirg4_1_write(be_reg, af_reg_be_cfg->iir_gain4_group1);
    isp_af_iirg5_0_write(be_reg, af_reg_be_cfg->iir_gain5_group0);
    isp_af_iirg5_1_write(be_reg, af_reg_be_cfg->iir_gain5_group1);
    isp_af_iirg6_0_write(be_reg, af_reg_be_cfg->iir_gain6_group0);
    isp_af_iirg6_1_write(be_reg, af_reg_be_cfg->iir_gain6_group1);
}

static td_void isp_be_af_iir_shift_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_iirshift0_0_write(be_reg, af_reg_be_cfg->iir0_shift_group0);
    isp_af_iirshift0_1_write(be_reg, af_reg_be_cfg->iir1_shift_group0);
    isp_af_iirshift0_2_write(be_reg, af_reg_be_cfg->iir2_shift_group0);
    isp_af_iirshift0_3_write(be_reg, af_reg_be_cfg->iir3_shift_group0);
    isp_af_iirshift1_0_write(be_reg, af_reg_be_cfg->iir0_shift_group1);
    isp_af_iirshift1_1_write(be_reg, af_reg_be_cfg->iir1_shift_group1);
    isp_af_iirshift1_2_write(be_reg, af_reg_be_cfg->iir2_shift_group1);
    isp_af_iirshift1_3_write(be_reg, af_reg_be_cfg->iir3_shift_group1);
}

static td_void isp_be_af_fir_gain_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_firh0_0_write(be_reg, af_reg_be_cfg->fir_h_gain0_group0);
    isp_af_firh0_1_write(be_reg, af_reg_be_cfg->fir_h_gain0_group1);

    isp_af_firh1_0_write(be_reg, af_reg_be_cfg->fir_h_gain1_group0);
    isp_af_firh1_1_write(be_reg, af_reg_be_cfg->fir_h_gain1_group1);

    isp_af_firh2_0_write(be_reg, af_reg_be_cfg->fir_h_gain2_group0);
    isp_af_firh2_1_write(be_reg, af_reg_be_cfg->fir_h_gain2_group1);

    isp_af_firh3_0_write(be_reg, af_reg_be_cfg->fir_h_gain3_group0);
    isp_af_firh3_1_write(be_reg, af_reg_be_cfg->fir_h_gain3_group1);

    isp_af_firh4_0_write(be_reg, af_reg_be_cfg->fir_h_gain4_group0);
    isp_af_firh4_1_write(be_reg, af_reg_be_cfg->fir_h_gain4_group1);
}

static td_void isp_be_af_crop_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_crop_en_write(be_reg, af_reg_be_cfg->crop_enable);
    if (af_reg_be_cfg->crop_enable) {
        isp_af_pos_x_write(be_reg, af_reg_be_cfg->crop_pos_x);
        isp_af_pos_y_write(be_reg, af_reg_be_cfg->crop_pos_y);
        isp_af_crop_hsize_write(be_reg, af_reg_be_cfg->crop_h_size - 1);
        isp_af_crop_vsize_write(be_reg, af_reg_be_cfg->crop_v_size - 1);
    }
}

static td_void isp_be_af_raw_cfg_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_sel_write(be_reg, af_reg_be_cfg->af_pos_sel);
    isp_af_raw_mode_write(be_reg, af_reg_be_cfg->raw_mode);
    isp_af_gamma_write(be_reg, af_reg_be_cfg->gamma);
    isp_af_bayer_mode_write(be_reg, af_reg_be_cfg->bayer_mode);
}

static td_void isp_be_af_level_depend_gain_cfg_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_iir0_ldg_en_write(be_reg, af_reg_be_cfg->iir0_ldg_enable);
    isp_af_iir_thre0_l_write(be_reg, af_reg_be_cfg->iir_thre0_low);
    isp_af_iir_thre0_h_write(be_reg, af_reg_be_cfg->iir_thre0_high);
    isp_af_iir_slope0_l_write(be_reg, af_reg_be_cfg->iir_slope0_low);
    isp_af_iir_slope0_h_write(be_reg, af_reg_be_cfg->iir_slope0_high);
    isp_af_iir_gain0_l_write(be_reg, af_reg_be_cfg->iir_gain0_low);
    isp_af_iir_gain0_h_write(be_reg, af_reg_be_cfg->iir_gain0_high);

    isp_af_iir1_ldg_en_write(be_reg, af_reg_be_cfg->iir1_ldg_enable);
    isp_af_iir_thre1_l_write(be_reg, af_reg_be_cfg->iir_thre1_low);
    isp_af_iir_thre1_h_write(be_reg, af_reg_be_cfg->iir_thre1_high);
    isp_af_iir_slope1_l_write(be_reg, af_reg_be_cfg->iir_slope1_low);
    isp_af_iir_slope1_h_write(be_reg, af_reg_be_cfg->iir_slope1_high);
    isp_af_iir_gain1_l_write(be_reg, af_reg_be_cfg->iir_gain1_low);
    isp_af_iir_gain1_h_write(be_reg, af_reg_be_cfg->iir_gain1_high);

    isp_af_fir0_ldg_en_write(be_reg, af_reg_be_cfg->fir0_ldg_enable);
    isp_af_fir_thre0_l_write(be_reg, af_reg_be_cfg->fir_thre0_low);
    isp_af_fir_thre0_h_write(be_reg, af_reg_be_cfg->fir_thre0_high);
    isp_af_fir_slope0_l_write(be_reg, af_reg_be_cfg->fir_slope0_low);
    isp_af_fir_slope0_h_write(be_reg, af_reg_be_cfg->fir_slope0_high);
    isp_af_fir_gain0_l_write(be_reg, af_reg_be_cfg->fir_gain0_low);
    isp_af_fir_gain0_h_write(be_reg, af_reg_be_cfg->fir_gain0_high);

    isp_af_fir1_ldg_en_write(be_reg, af_reg_be_cfg->fir1_ldg_enable);
    isp_af_fir_thre1_l_write(be_reg, af_reg_be_cfg->fir_thre1_low);
    isp_af_fir_thre1_h_write(be_reg, af_reg_be_cfg->fir_thre1_high);
    isp_af_fir_slope1_l_write(be_reg, af_reg_be_cfg->fir_slope1_low);
    isp_af_fir_slope1_h_write(be_reg, af_reg_be_cfg->fir_slope1_high);
    isp_af_fir_gain1_l_write(be_reg, af_reg_be_cfg->fir_gain1_low);
    isp_af_fir_gain1_h_write(be_reg, af_reg_be_cfg->fir_gain1_high);
}

static td_void isp_be_af_coring_cfg_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_iir_thre0_c_write(be_reg, af_reg_be_cfg->iir_thre0_coring);
    isp_af_iir_slope0_c_write(be_reg, af_reg_be_cfg->iir_slope0_coring);
    isp_af_iir_peak0_c_write(be_reg, af_reg_be_cfg->iir_peak0_coring);

    isp_af_iir_thre1_c_write(be_reg, af_reg_be_cfg->iir_thre1_coring);
    isp_af_iir_slope1_c_write(be_reg, af_reg_be_cfg->iir_slope1_coring);
    isp_af_iir_peak1_c_write(be_reg, af_reg_be_cfg->iir_peak1_coring);

    isp_af_fir_thre0_c_write(be_reg, af_reg_be_cfg->fir_thre0_coring);
    isp_af_fir_slope0_c_write(be_reg, af_reg_be_cfg->fir_slope0_coring);
    isp_af_fir_peak0_c_write(be_reg, af_reg_be_cfg->fir_peak0_coring);

    isp_af_fir_thre1_c_write(be_reg, af_reg_be_cfg->fir_thre1_coring);
    isp_af_fir_slope1_c_write(be_reg, af_reg_be_cfg->fir_slope1_coring);
    isp_af_fir_peak1_c_write(be_reg, af_reg_be_cfg->fir_peak1_coring);
}

static td_void isp_be_af_output_shift_cfg_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_acc_shift0_h_write(be_reg, af_reg_be_cfg->acc_shift0_h);
    isp_af_acc_shift1_h_write(be_reg, af_reg_be_cfg->acc_shift1_h);
    isp_af_acc_shift0_v_write(be_reg, af_reg_be_cfg->acc_shift0_v);
    isp_af_acc_shift1_v_write(be_reg, af_reg_be_cfg->acc_shift1_v);
    isp_af_acc_shift_y_write(be_reg, af_reg_be_cfg->acc_shift_y);
    isp_af_cnt_shift_y_write(be_reg, af_reg_be_cfg->shift_count_y);
    isp_af_cnt_shift0_v_write(be_reg, ISP_AF_CNT_SHIFT0_V_DEFAULT);
    isp_af_cnt_shift0_h_write(be_reg, 0x0);
    isp_af_cnt_shift1_h_write(be_reg, 0x0);
    isp_af_cnt_shift1_v_write(be_reg, 0x0);
}

static td_void isp_be_af_reg_write(isp_post_be_reg_type *be_reg, isp_af_reg_cfg *af_reg_be_cfg)
{
    isp_af_lpf_en_write(be_reg, af_reg_be_cfg->lpf_enable);
    isp_af_fir0_lpf_en_write(be_reg, af_reg_be_cfg->fir0_lpf_enable);
    isp_af_fir1_lpf_en_write(be_reg, af_reg_be_cfg->fir1_lpf_enable);
    isp_af_iir0_ds_en_write(be_reg, af_reg_be_cfg->iir0_ds_enable);
    isp_af_iir1_ds_en_write(be_reg, af_reg_be_cfg->iir1_ds_enable);
    isp_af_iir_dilate0_write(be_reg, af_reg_be_cfg->iir_dilate0);
    isp_af_iir_dilate1_write(be_reg, af_reg_be_cfg->iir_dilate1);
    isp_af_iirplg_0_write(be_reg, af_reg_be_cfg->iir_plg_group0);
    isp_af_iirpls_0_write(be_reg, af_reg_be_cfg->iir_pls_group0);
    isp_af_iirplg_1_write(be_reg, af_reg_be_cfg->iir_plg_group1);
    isp_af_iirpls_1_write(be_reg, af_reg_be_cfg->iir_pls_group1);

    isp_af_iir0_en0_write(be_reg, af_reg_be_cfg->iir0_enable0);
    isp_af_iir0_en1_write(be_reg, af_reg_be_cfg->iir0_enable1);
    isp_af_iir0_en2_write(be_reg, af_reg_be_cfg->iir0_enable2);
    isp_af_iir1_en0_write(be_reg, af_reg_be_cfg->iir1_enable0);
    isp_af_iir1_en1_write(be_reg, af_reg_be_cfg->iir1_enable1);
    isp_af_iir1_en2_write(be_reg, af_reg_be_cfg->iir1_enable2);
    isp_af_peak_mode_write(be_reg, af_reg_be_cfg->peak_mode);
    isp_af_squ_mode_write(be_reg, af_reg_be_cfg->squ_mode);
    isp_af_hnum_write(be_reg, af_reg_be_cfg->window_hnum);
    isp_af_vnum_write(be_reg, af_reg_be_cfg->window_vnum);

    isp_be_af_iir_gain_write(be_reg, af_reg_be_cfg);
    isp_be_af_iir_shift_write(be_reg, af_reg_be_cfg);
    isp_be_af_fir_gain_write(be_reg, af_reg_be_cfg);

    isp_be_af_crop_write(be_reg, af_reg_be_cfg);
    isp_be_af_raw_cfg_write(be_reg, af_reg_be_cfg);

    /* AF BE pre median filter */
    isp_af_mean_en_write(be_reg, af_reg_be_cfg->mean_enable);
    isp_af_mean_thres_write(be_reg, 0xFFFF - af_reg_be_cfg->mean_thres);
    isp_be_af_level_depend_gain_cfg_write(be_reg, af_reg_be_cfg); /* level depend gain */
    isp_be_af_coring_cfg_write(be_reg, af_reg_be_cfg);            /* AF BE coring */

    /* high luma counter */
    isp_af_highlight_write(be_reg, af_reg_be_cfg->highlight_thre);

    isp_be_af_output_shift_cfg_write(be_reg, af_reg_be_cfg);
}
#endif
static td_s32 isp_af_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
#ifdef CONFIG_OT_ISP_AF_SUPPORT
    td_bool offline_mode;
    td_bool usr_resh;
    td_bool idx_resh;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_af_reg_cfg *af_reg_be_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;

    af_reg_be_cfg = &reg_cfg_info->alg_reg_cfg[i].be_af_reg_cfg;
    offline_mode = (af_reg_be_cfg->be_is_online == TD_TRUE) ? (TD_FALSE) : (TD_TRUE);

    if (af_reg_be_cfg->is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        viproc_reg = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    }

    isp_check_pointer_return(be_reg);
    isp_check_pointer_return(viproc_reg);

    isp_af_en_write(viproc_reg, af_reg_be_cfg->af_enable);

    idx_resh = (isp_af_update_index_read(be_reg) != af_reg_be_cfg->update_index);
    usr_resh =
        (offline_mode) ? (reg_cfg_info->cfg_key.bit1_af_be_cfg && idx_resh) : (reg_cfg_info->cfg_key.bit1_af_be_cfg);

    if (usr_resh) {
        isp_af_update_index_write(be_reg, af_reg_be_cfg->update_index);

        isp_be_af_reg_write(be_reg, af_reg_be_cfg);

        reg_cfg_info->cfg_key.bit1_af_be_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
#endif

    return TD_SUCCESS;
}

static td_s32 isp_fe_awb_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u32 i;
    ot_vi_pipe vi_pipe_bind;
    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;
    isp_awb_reg_sta_cfg *awb_reg_sta_cfg = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);

        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);
        if (reg_cfg_info->cfg_key.bit1_awb_dyn_cfg) {
            awb_reg_dyn_cfg = &reg_cfg_info->alg_reg_cfg[0].awb_reg_cfg.awb_reg_dyn_cfg;
            isp_fe_wb_gain_write(fe_reg, awb_reg_dyn_cfg->fe_white_balance_gain);
            isp_fe_wb_en_write(fe_reg, awb_reg_dyn_cfg->fe_wb_work_en);
        }

        awb_reg_sta_cfg = &reg_cfg_info->alg_reg_cfg[0].awb_reg_cfg.awb_reg_sta_cfg;

        if (awb_reg_sta_cfg->fe_awb_sta_cfg) {
            awb_reg_sta_cfg = &reg_cfg_info->alg_reg_cfg[0].awb_reg_cfg.awb_reg_sta_cfg;
            isp_fe_wb_clip_value_write(fe_reg, awb_reg_sta_cfg->fe_clip_value);
        }
    }

    return TD_SUCCESS;
}

static td_void isp_awb_cc_set(isp_post_be_reg_type *be_reg, td_u16 *be_cc)
{
    td_u8 i;
    for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
        if ((be_cc[i] >> 12) < 0x8) {          /* valid bit 12, sign bit 0x8 */
            be_cc[i] = MIN2(be_cc[i], 0xFFF);  /* max positive value 0xFFF */
        } else {  /* valid bit 12, sign bit 0x8 */
            be_cc[i] = MIN2(be_cc[i], 0x8FFF); /* max negative value 0x8FFF */
        }
    }
    isp_cc_coef00_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[0]))); /* array index 0 */
    isp_cc_coef01_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[1]))); /* array index 1 */
    isp_cc_coef02_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[2]))); /* array index 2 */
    isp_cc_coef10_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[3]))); /* array index 3 */
    isp_cc_coef11_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[4]))); /* array index 4 */
    isp_cc_coef12_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[5]))); /* array index 5 */
    isp_cc_coef20_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[6]))); /* array index 6 */
    isp_cc_coef21_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[7]))); /* array index 7 */
    isp_cc_coef22_write(be_reg, ccm_convert(ccm_convert_pre(be_cc[8]))); /* array index 8 */
}

static td_void isp_awb_gain_set(isp_post_be_reg_type *be_reg, td_u32 *be_wb_gain)
{
    isp_wb_rgain_write(be_reg, be_wb_gain[0]);  /* array index 0 */
    isp_wb_grgain_write(be_reg, be_wb_gain[1]); /* array index 1 */
    isp_wb_gbgain_write(be_reg, be_wb_gain[2]); /* array index 2 */
    isp_wb_bgain_write(be_reg, be_wb_gain[3]);  /* array index 3 */
}

static td_void isp_awb_dyn_reg_config(isp_post_be_reg_type *be_reg, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg)
{
    isp_awb_threshold_max_write(be_reg, awb_reg_dyn_cfg->be_metering_white_level_awb);
    isp_awb_threshold_min_write(be_reg, awb_reg_dyn_cfg->be_metering_black_level_awb);
    isp_awb_cr_ref_max_write(be_reg, awb_reg_dyn_cfg->be_metering_cr_ref_max_awb);
    isp_awb_cr_ref_min_write(be_reg, awb_reg_dyn_cfg->be_metering_cr_ref_min_awb);
    isp_awb_cb_ref_max_write(be_reg, awb_reg_dyn_cfg->be_metering_cb_ref_max_awb);
    isp_awb_cb_ref_min_write(be_reg, awb_reg_dyn_cfg->be_metering_cb_ref_min_awb);

    isp_cc_r_gain_write(be_reg, awb_reg_dyn_cfg->be_cc_r_gain);
    isp_cc_g_gain_write(be_reg, awb_reg_dyn_cfg->be_cc_g_gain);
    isp_cc_b_gain_write(be_reg, awb_reg_dyn_cfg->be_cc_b_gain);

    isp_awb_crop_pos_x_write(be_reg, awb_reg_dyn_cfg->be_crop_pos_x);
    isp_awb_crop_pos_y_write(be_reg, awb_reg_dyn_cfg->be_crop_pos_y);
    isp_awb_crop_out_width_write(be_reg, awb_reg_dyn_cfg->be_crop_out_width - 1);
    isp_awb_crop_out_height_write(be_reg, awb_reg_dyn_cfg->be_crop_out_height - 1);
}

static td_void isp_awb_sta_reg_config(isp_post_be_reg_type *be_reg, isp_awb_reg_sta_cfg *awb_reg_sta_cfg)
{
    isp_awb_bitmove_write(be_reg, awb_reg_sta_cfg->be_awb_bitmove);
    isp_awb_stat_raddr_write(be_reg, awb_reg_sta_cfg->be_awb_stat_raddr);
    isp_cc_recover_en_write(be_reg, OT_ISP_CCM_RECOVER_EN_DEFAULT);
    isp_cc_luma_coefr_write(be_reg, OT_ISP_CCM_LUMA_COEFR_DEFAULT);
    isp_cc_luma_coefb_write(be_reg, OT_ISP_CCM_LUMA_COEFB_DEFAULT);
    isp_cc_luma_coefr_up_write(be_reg, OT_ISP_CCM_LUMA_COEFR_UP_DEFAULT);
    isp_cc_luma_coefb_up_write(be_reg, OT_ISP_CCM_LUMA_COEFB_UP_DEFAULT);
    isp_cc_soft_clip0_step_write(be_reg, OT_ISP_CCM_SOFT_CLIP0_STEP_DEFAULT);
    isp_cc_soft_clip1_step_write(be_reg, OT_ISP_CCM_SOFT_CLIP1_STEP_DEFAULT);
    isp_cc_darkprev_write(be_reg, OT_ISP_CCM_DARKPREV_DEFAULT);
    isp_cc_peaksupp_sat_write(be_reg, OT_ISP_CCM_PEAKSUPP_SAT_DEFAULT);
    isp_cc_peaksupp_max_write(be_reg, OT_ISP_CCM_PEAKSUPP_MAX_DEFAULT);

    isp_cc_in_dc0_write(be_reg, awb_reg_sta_cfg->be_cc_in_dc0);
    isp_cc_in_dc1_write(be_reg, awb_reg_sta_cfg->be_cc_in_dc1);
    isp_cc_in_dc2_write(be_reg, awb_reg_sta_cfg->be_cc_in_dc2);
    isp_cc_out_dc0_write(be_reg, awb_reg_sta_cfg->be_cc_out_dc0);
    isp_cc_out_dc1_write(be_reg, awb_reg_sta_cfg->be_cc_out_dc1);
    isp_cc_out_dc2_write(be_reg, awb_reg_sta_cfg->be_cc_out_dc2);
    isp_wb_clip_value_write(be_reg, awb_reg_sta_cfg->be_wb_clip_value);
    isp_awb_offset_comp_write(be_reg, awb_reg_sta_cfg->be_awb_offset_comp);
}

static td_void isp_awb_usr_reg_config(isp_post_be_reg_type *be_reg, isp_awb_reg_usr_cfg *awb_reg_usr_cfg)
{
    isp_awb_update_index_write(be_reg, awb_reg_usr_cfg->update_index);
    isp_awb_sel_write(be_reg, awb_reg_usr_cfg->be_awb_switch);
    isp_awb_hnum_write(be_reg, awb_reg_usr_cfg->be_zone_col);
    isp_awb_vnum_write(be_reg, awb_reg_usr_cfg->be_zone_row);
}

static td_void isp_awb_cc_and_gain_reg_config(ot_vi_pipe vi_pipe, isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg,
    isp_post_be_reg_type *be_reg)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    /* offline mode, cfg ccm/awb at sync param in vi module. */
    if (isp_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        isp_awb_cc_set(be_reg, awb_reg_dyn_cfg->be_color_matrix);
        isp_awb_gain_set(be_reg, awb_reg_dyn_cfg->be_white_balance_gain);
    }
}

static td_void isp_awb_reg_config_write(ot_vi_pipe vi_pipe, isp_be_reg_type *be_reg, isp_viproc_reg_type *viproc_reg,
    isp_awb_reg_cfg *awb_reg_cfg)
{
    td_bool offline_mode;
    td_bool idx_resh, usr_resh;
    isp_awb_reg_sta_cfg *awb_reg_sta_cfg = &awb_reg_cfg->awb_reg_sta_cfg;
    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = &awb_reg_cfg->awb_reg_dyn_cfg;
    isp_awb_reg_usr_cfg *awb_reg_usr_cfg = &awb_reg_cfg->awb_reg_usr_cfg;

    isp_awb_en_write(viproc_reg, awb_reg_sta_cfg->be_awb_work_en);
    isp_awb_cc_and_gain_reg_config(vi_pipe, awb_reg_dyn_cfg, be_reg);

    isp_awb_dyn_reg_config(be_reg, awb_reg_dyn_cfg);

    if (awb_reg_sta_cfg->be_awb_sta_cfg) {
        isp_awb_sta_reg_config(be_reg, awb_reg_sta_cfg);
        awb_reg_sta_cfg->be_awb_sta_cfg = 0;
    }

    offline_mode = (awb_reg_dyn_cfg->is_online == TD_TRUE) ? (TD_FALSE) : (TD_TRUE);
    idx_resh = (isp_awb_update_index_read(be_reg) != awb_reg_usr_cfg->update_index);
    usr_resh = (offline_mode) ? (awb_reg_usr_cfg->resh && idx_resh) : (awb_reg_usr_cfg->resh);

    if (usr_resh) {
        isp_awb_usr_reg_config(be_reg, awb_reg_usr_cfg);
        /* if online mode, resh=0; if offline mode, resh=1; but only index != will resh */
        awb_reg_usr_cfg->resh = offline_mode;
    }
}

static td_s32 isp_awb_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_awb_reg_dyn_cfg *awb_reg_dyn_cfg = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;
    isp_be_reg_type *be_reg = TD_NULL;

    awb_reg_dyn_cfg = &reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg;

    if (awb_reg_dyn_cfg->is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        viproc_reg = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    }
    isp_check_pointer_return(be_reg);
    isp_check_pointer_return(viproc_reg);

    isp_awb_reg_config_write(vi_pipe, be_reg, viproc_reg, &reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg);

    return TD_SUCCESS;
}

static td_void isp_sharpen_lumawgt_write(isp_post_be_reg_type *be_reg, td_u8 *luma_wgt)
{
    isp_sharpen_lumawgt0_write(be_reg, luma_wgt[0]);   /* array index 0 */
    isp_sharpen_lumawgt1_write(be_reg, luma_wgt[1]);   /* array index 1 */
    isp_sharpen_lumawgt2_write(be_reg, luma_wgt[2]);   /* array index 2 */
    isp_sharpen_lumawgt3_write(be_reg, luma_wgt[3]);   /* array index 3 */
    isp_sharpen_lumawgt4_write(be_reg, luma_wgt[4]);   /* array index 4 */
    isp_sharpen_lumawgt5_write(be_reg, luma_wgt[5]);   /* array index 5 */
    isp_sharpen_lumawgt6_write(be_reg, luma_wgt[6]);   /* array index 6 */
    isp_sharpen_lumawgt7_write(be_reg, luma_wgt[7]);   /* array index 7 */
    isp_sharpen_lumawgt8_write(be_reg, luma_wgt[8]);   /* array index 8 */
    isp_sharpen_lumawgt9_write(be_reg, luma_wgt[9]);   /* array index 9 */
    isp_sharpen_lumawgt10_write(be_reg, luma_wgt[10]); /* array index 10 */
    isp_sharpen_lumawgt11_write(be_reg, luma_wgt[11]); /* array index 11 */
    isp_sharpen_lumawgt12_write(be_reg, luma_wgt[12]); /* array index 12 */
    isp_sharpen_lumawgt13_write(be_reg, luma_wgt[13]); /* array index 13 */
    isp_sharpen_lumawgt14_write(be_reg, luma_wgt[14]); /* array index 14 */
    isp_sharpen_lumawgt15_write(be_reg, luma_wgt[15]); /* array index 15 */
    isp_sharpen_lumawgt16_write(be_reg, luma_wgt[16]); /* array index 16 */
    isp_sharpen_lumawgt17_write(be_reg, luma_wgt[17]); /* array index 17 */
    isp_sharpen_lumawgt18_write(be_reg, luma_wgt[18]); /* array index 18 */
    isp_sharpen_lumawgt19_write(be_reg, luma_wgt[19]); /* array index 19 */
    isp_sharpen_lumawgt20_write(be_reg, luma_wgt[20]); /* array index 20 */
    isp_sharpen_lumawgt21_write(be_reg, luma_wgt[21]); /* array index 21 */
    isp_sharpen_lumawgt22_write(be_reg, luma_wgt[22]); /* array index 22 */
    isp_sharpen_lumawgt23_write(be_reg, luma_wgt[23]); /* array index 23 */
    isp_sharpen_lumawgt24_write(be_reg, luma_wgt[24]); /* array index 24 */
    isp_sharpen_lumawgt25_write(be_reg, luma_wgt[25]); /* array index 25 */
    isp_sharpen_lumawgt26_write(be_reg, luma_wgt[26]); /* array index 26 */
    isp_sharpen_lumawgt27_write(be_reg, luma_wgt[27]); /* array index 27 */
    isp_sharpen_lumawgt28_write(be_reg, luma_wgt[28]); /* array index 28 */
    isp_sharpen_lumawgt29_write(be_reg, luma_wgt[29]); /* array index 29 */
    isp_sharpen_lumawgt30_write(be_reg, luma_wgt[30]); /* array index 30 */
    isp_sharpen_lumawgt31_write(be_reg, luma_wgt[31]); /* array index 31 */
}

static td_void isp_sharpen_mpi_dyna_reg_config(isp_post_be_reg_type *be_reg,
    isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_reg_cfg)
{
    isp_sharpen_detl_oshtmul_write(be_reg, mpi_dyna_reg_cfg->detail_osht_mul);
    isp_sharpen_detl_ushtmul_write(be_reg, mpi_dyna_reg_cfg->detail_usht_mul);
    isp_sharpen_omaxgain_write(be_reg, mpi_dyna_reg_cfg->o_max_gain);
    isp_sharpen_umaxgain_write(be_reg, mpi_dyna_reg_cfg->u_max_gain);
    isp_sharpen_skinmaxu_write(be_reg, mpi_dyna_reg_cfg->skin_max_u);
    isp_sharpen_skinminu_write(be_reg, mpi_dyna_reg_cfg->skin_min_u);
    isp_sharpen_skinmaxv_write(be_reg, mpi_dyna_reg_cfg->skin_max_v);
    isp_sharpen_skinminv_write(be_reg, mpi_dyna_reg_cfg->skin_min_v);
    isp_sharpen_chrgmfmul_write(be_reg, mpi_dyna_reg_cfg->chr_gmf_mul);
    isp_sharpen_chrgmul_write(be_reg, mpi_dyna_reg_cfg->chr_g_mul);
    isp_sharpen_chrggain_write(be_reg, mpi_dyna_reg_cfg->chr_g_gain);
    isp_sharpen_chrgmfgain_write(be_reg, mpi_dyna_reg_cfg->chr_gmf_gain);

    isp_sharpen_chrbgain_write(be_reg, mpi_dyna_reg_cfg->chr_b_gain);
    isp_sharpen_chrbmul_write(be_reg, mpi_dyna_reg_cfg->chr_b_mul);
    isp_sharpen_chrrgain_write(be_reg, mpi_dyna_reg_cfg->chr_r_gain);
    isp_sharpen_chrrmul_write(be_reg, mpi_dyna_reg_cfg->chr_r_mul);
    isp_sharpen_benchrctrl_write(be_reg, mpi_dyna_reg_cfg->en_chr_ctrl);
    isp_sharpen_bendetailctrl_write(be_reg, mpi_dyna_reg_cfg->en_detail_ctrl);
    isp_sharpen_benlumactrl_write(be_reg, mpi_dyna_reg_cfg->en_luma_ctrl);
    isp_sharpen_benshtctrlbyvar_write(be_reg, mpi_dyna_reg_cfg->en_sht_ctrl_by_var);
    isp_sharpen_benskinctrl_write(be_reg, mpi_dyna_reg_cfg->en_skin_ctrl);
    isp_sharpen_weakdetailadj_write(be_reg, mpi_dyna_reg_cfg->en_weak_detail_adj);
    isp_sharpen_dirdiffsft_write(be_reg, mpi_dyna_reg_cfg->dir_diff_sft);
    isp_sharpen_dirrt0_write(be_reg, mpi_dyna_reg_cfg->dir_rt[0]); /* array index 0 */
    isp_sharpen_dirrt1_write(be_reg, mpi_dyna_reg_cfg->dir_rt[1]); /* array index 1 */
    isp_sharpen_lumawgt_write(be_reg, mpi_dyna_reg_cfg->luma_wgt);

    isp_sharpen_osht_dtl_thd0_write(be_reg, mpi_dyna_reg_cfg->detail_osht_thr[0]); /* array index 0 */
    isp_sharpen_osht_dtl_thd1_write(be_reg, mpi_dyna_reg_cfg->detail_osht_thr[1]); /* array index 1 */
    isp_sharpen_osht_dtl_wgt_write(be_reg, mpi_dyna_reg_cfg->detail_osht_amt);
    isp_sharpen_shtvarthd1_write(be_reg, mpi_dyna_reg_cfg->sht_var_thd1);
    isp_sharpen_oshtamt_write(be_reg, mpi_dyna_reg_cfg->osht_amt);
    isp_sharpen_ushtamt_write(be_reg, mpi_dyna_reg_cfg->usht_amt);
    isp_sharpen_shtbldrt_write(be_reg, mpi_dyna_reg_cfg->sht_bld_rt);
    isp_sharpen_shtvarmul_write(be_reg, mpi_dyna_reg_cfg->sht_var_mul);
    isp_sharpen_skinedgemul_write(be_reg, mpi_dyna_reg_cfg->skin_edge_mul);
    isp_sharpen_skinedgewgt0_write(be_reg, mpi_dyna_reg_cfg->skin_edge_wgt[0]);
    isp_sharpen_skinedgewgt1_write(be_reg, mpi_dyna_reg_cfg->skin_edge_wgt[1]);
    isp_sharpen_usht_dtl_thd0_write(be_reg, mpi_dyna_reg_cfg->detail_usht_thr[0]);
    isp_sharpen_usht_dtl_thd1_write(be_reg, mpi_dyna_reg_cfg->detail_usht_thr[1]);
    isp_sharpen_usht_dtl_wgt_write(be_reg, mpi_dyna_reg_cfg->detail_usht_amt);
    isp_sharpen_weakdetailgain_write(be_reg, mpi_dyna_reg_cfg->weak_detail_gain);
}

static td_void isp_sharpen_def_dyna_reg_config(isp_post_be_reg_type *be_reg,
    isp_sharpen_default_dyna_reg_cfg *def_dyna_reg_cfg)
{
    /* sharpen default iso */
    isp_sharpen_mhfthdsftd_write(be_reg, def_dyna_reg_cfg->gain_thd_sft_d);
    isp_sharpen_mhfthdselud_write(be_reg, def_dyna_reg_cfg->gain_thd_sel_ud);
    isp_sharpen_mhfthdsftud_write(be_reg, def_dyna_reg_cfg->gain_thd_sft_ud);
    isp_sharpen_dirvarsft_write(be_reg, def_dyna_reg_cfg->dir_var_sft);
    isp_sharpen_shtvarwgt0_write(be_reg, def_dyna_reg_cfg->sht_var_wgt0);
    isp_sharpen_shtvardiffthd0_write(be_reg, def_dyna_reg_cfg->sht_var_diff_thd[0]);
    isp_sharpen_selpixwgt_write(be_reg, def_dyna_reg_cfg->sel_pix_wgt);
    isp_sharpen_shtvardiffthd1_write(be_reg, def_dyna_reg_cfg->sht_var_diff_thd[1]);
    isp_sharpen_shtvardiffwgt1_write(be_reg, def_dyna_reg_cfg->sht_var_diff_wgt1);
    isp_sharpen_shtvardiffmul_write(be_reg, def_dyna_reg_cfg->sht_var_diff_mul);
    isp_sharpen_rmfscale_write(be_reg, def_dyna_reg_cfg->rmf_gain_scale);
    isp_sharpen_bmfscale_write(be_reg, def_dyna_reg_cfg->bmf_gain_scale);
    isp_sharpen_dirrlythrlow_write(be_reg, def_dyna_reg_cfg->dir_rly_thr_low);
    isp_sharpen_dirrlythrhih_write(be_reg, def_dyna_reg_cfg->dir_rly_thrhigh);
}

static td_void isp_sharpen_dyna_reg_config(isp_post_be_reg_type *be_reg, isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_reg_cfg,
    isp_sharpen_default_dyna_reg_cfg *def_dyna_reg_cfg)
{
    isp_sharpen_mpi_dyna_reg_config(be_reg, mpi_dyna_reg_cfg);
    isp_sharpen_def_dyna_reg_config(be_reg, def_dyna_reg_cfg);
}

static td_void isp_sharpen_dhsfcoef_d0_write(isp_post_be_reg_type *be_reg, td_s8 *hsf_coef_d0)
{
    isp_sharpen_dhsfcoef0_0_write(be_reg, hsf_coef_d0[0]);   /* array index 0 */
    isp_sharpen_dhsfcoef0_1_write(be_reg, hsf_coef_d0[1]);   /* array index 1 */
    isp_sharpen_dhsfcoef0_2_write(be_reg, hsf_coef_d0[2]);   /* array index 2 */
    isp_sharpen_dhsfcoef0_3_write(be_reg, hsf_coef_d0[3]);   /* array index 3 */
    isp_sharpen_dhsfcoef0_4_write(be_reg, hsf_coef_d0[4]);   /* array index 4 */
    isp_sharpen_dhsfcoef0_5_write(be_reg, hsf_coef_d0[5]);   /* array index 5 */
    isp_sharpen_dhsfcoef0_6_write(be_reg, hsf_coef_d0[6]);   /* array index 6 */
    isp_sharpen_dhsfcoef0_7_write(be_reg, hsf_coef_d0[7]);   /* array index 7 */
    isp_sharpen_dhsfcoef0_8_write(be_reg, hsf_coef_d0[8]);   /* array index 8 */
    isp_sharpen_dhsfcoef0_9_write(be_reg, hsf_coef_d0[9]);   /* array index 9 */
    isp_sharpen_dhsfcoef0_10_write(be_reg, hsf_coef_d0[10]); /* array index 10 */
    isp_sharpen_dhsfcoef0_11_write(be_reg, hsf_coef_d0[11]); /* array index 11 */
    isp_sharpen_dhsfcoef0_12_write(be_reg, hsf_coef_d0[12]); /* array index 12 */
}

static td_void isp_sharpen_dhsfcoef_d1_write(isp_post_be_reg_type *be_reg, td_s8 *hsf_coef_d1)
{
    isp_sharpen_dhsfcoef1_0_write(be_reg, hsf_coef_d1[0]);   /* array index 0 */
    isp_sharpen_dhsfcoef1_1_write(be_reg, hsf_coef_d1[1]);   /* array index 1 */
    isp_sharpen_dhsfcoef1_2_write(be_reg, hsf_coef_d1[2]);   /* array index 2 */
    isp_sharpen_dhsfcoef1_3_write(be_reg, hsf_coef_d1[3]);   /* array index 3 */
    isp_sharpen_dhsfcoef1_4_write(be_reg, hsf_coef_d1[4]);   /* array index 4 */
    isp_sharpen_dhsfcoef1_5_write(be_reg, hsf_coef_d1[5]);   /* array index 5 */
    isp_sharpen_dhsfcoef1_6_write(be_reg, hsf_coef_d1[6]);   /* array index 6 */
    isp_sharpen_dhsfcoef1_7_write(be_reg, hsf_coef_d1[7]);   /* array index 7 */
    isp_sharpen_dhsfcoef1_8_write(be_reg, hsf_coef_d1[8]);   /* array index 8 */
    isp_sharpen_dhsfcoef1_9_write(be_reg, hsf_coef_d1[9]);   /* array index 9 */
    isp_sharpen_dhsfcoef1_10_write(be_reg, hsf_coef_d1[10]); /* array index 10 */
    isp_sharpen_dhsfcoef1_11_write(be_reg, hsf_coef_d1[11]); /* array index 11 */
    isp_sharpen_dhsfcoef1_12_write(be_reg, hsf_coef_d1[12]); /* array index 12 */
}

static td_void isp_sharpen_dhsfcoef_d2_write(isp_post_be_reg_type *be_reg, td_s8 *hsf_coef_d2)
{
    isp_sharpen_dhsfcoef2_0_write(be_reg, hsf_coef_d2[0]);   /* array index 0 */
    isp_sharpen_dhsfcoef2_1_write(be_reg, hsf_coef_d2[1]);   /* array index 1 */
    isp_sharpen_dhsfcoef2_2_write(be_reg, hsf_coef_d2[2]);   /* array index 2 */
    isp_sharpen_dhsfcoef2_3_write(be_reg, hsf_coef_d2[3]);   /* array index 3 */
    isp_sharpen_dhsfcoef2_4_write(be_reg, hsf_coef_d2[4]);   /* array index 4 */
    isp_sharpen_dhsfcoef2_5_write(be_reg, hsf_coef_d2[5]);   /* array index 5 */
    isp_sharpen_dhsfcoef2_6_write(be_reg, hsf_coef_d2[6]);   /* array index 6 */
    isp_sharpen_dhsfcoef2_7_write(be_reg, hsf_coef_d2[7]);   /* array index 7 */
    isp_sharpen_dhsfcoef2_8_write(be_reg, hsf_coef_d2[8]);   /* array index 8 */
    isp_sharpen_dhsfcoef2_9_write(be_reg, hsf_coef_d2[9]);   /* array index 9 */
    isp_sharpen_dhsfcoef2_10_write(be_reg, hsf_coef_d2[10]); /* array index 10 */
    isp_sharpen_dhsfcoef2_11_write(be_reg, hsf_coef_d2[11]); /* array index 11 */
    isp_sharpen_dhsfcoef2_12_write(be_reg, hsf_coef_d2[12]); /* array index 12 */
}

static td_void isp_sharpen_dhsfcoef_d3_write(isp_post_be_reg_type *be_reg, td_s8 *hsf_coef_d3)
{
    isp_sharpen_dhsfcoef3_0_write(be_reg, hsf_coef_d3[0]);   /* array index 0 */
    isp_sharpen_dhsfcoef3_1_write(be_reg, hsf_coef_d3[1]);   /* array index 1 */
    isp_sharpen_dhsfcoef3_2_write(be_reg, hsf_coef_d3[2]);   /* array index 2 */
    isp_sharpen_dhsfcoef3_3_write(be_reg, hsf_coef_d3[3]);   /* array index 3 */
    isp_sharpen_dhsfcoef3_4_write(be_reg, hsf_coef_d3[4]);   /* array index 4 */
    isp_sharpen_dhsfcoef3_5_write(be_reg, hsf_coef_d3[5]);   /* array index 5 */
    isp_sharpen_dhsfcoef3_6_write(be_reg, hsf_coef_d3[6]);   /* array index 6 */
    isp_sharpen_dhsfcoef3_7_write(be_reg, hsf_coef_d3[7]);   /* array index 7 */
    isp_sharpen_dhsfcoef3_8_write(be_reg, hsf_coef_d3[8]);   /* array index 8 */
    isp_sharpen_dhsfcoef3_9_write(be_reg, hsf_coef_d3[9]);   /* array index 9 */
    isp_sharpen_dhsfcoef3_10_write(be_reg, hsf_coef_d3[10]); /* array index 10 */
    isp_sharpen_dhsfcoef3_11_write(be_reg, hsf_coef_d3[11]); /* array index 11 */
    isp_sharpen_dhsfcoef3_12_write(be_reg, hsf_coef_d3[12]); /* array index 12 */
}

static td_void isp_sharpen_lmt_hf_write(isp_post_be_reg_type *be_reg, td_u8 *lmt_hf)
{
    isp_sharpen_lmthf0_write(be_reg, lmt_hf[0]); /* array index 0 */
    isp_sharpen_lmthf1_write(be_reg, lmt_hf[1]); /* array index 1 */
    isp_sharpen_lmthf2_write(be_reg, lmt_hf[2]); /* array index 2 */
    isp_sharpen_lmthf3_write(be_reg, lmt_hf[3]); /* array index 3 */
    isp_sharpen_lmthf4_write(be_reg, lmt_hf[4]); /* array index 4 */
    isp_sharpen_lmthf5_write(be_reg, lmt_hf[5]); /* array index 5 */
    isp_sharpen_lmthf6_write(be_reg, lmt_hf[6]); /* array index 6 */
    isp_sharpen_lmthf7_write(be_reg, lmt_hf[7]); /* array index 7 */
}

static td_void isp_sharpen_lmt_mf_write(isp_post_be_reg_type *be_reg, td_u8 *lmt_mf)
{
    isp_sharpen_lmtmf0_write(be_reg, lmt_mf[0]); /* array index 0 */
    isp_sharpen_lmtmf1_write(be_reg, lmt_mf[1]); /* array index 1 */
    isp_sharpen_lmtmf2_write(be_reg, lmt_mf[2]); /* array index 2 */
    isp_sharpen_lmtmf3_write(be_reg, lmt_mf[3]); /* array index 3 */
    isp_sharpen_lmtmf4_write(be_reg, lmt_mf[4]); /* array index 4 */
    isp_sharpen_lmtmf5_write(be_reg, lmt_mf[5]); /* array index 5 */
    isp_sharpen_lmtmf6_write(be_reg, lmt_mf[6]); /* array index 6 */
    isp_sharpen_lmtmf7_write(be_reg, lmt_mf[7]); /* array index 7 */
}

static td_void isp_sharpen_chr_write(isp_post_be_reg_type *be_reg, isp_sharpen_static_reg_cfg *static_reg_cfg)
{
    isp_sharpen_chrrsft0_write(be_reg, static_reg_cfg->chr_r_sft[0]); /* array index 0 */
    isp_sharpen_chrrsft1_write(be_reg, static_reg_cfg->chr_r_sft[1]); /* array index 1 */
    isp_sharpen_chrrsft2_write(be_reg, static_reg_cfg->chr_r_sft[2]); /* array index 2 */
    isp_sharpen_chrrsft3_write(be_reg, static_reg_cfg->chr_r_sft[3]); /* array index 3 */
    isp_sharpen_chrrvarshift_write(be_reg, static_reg_cfg->chr_r_var_sft);

    isp_sharpen_chrbsft0_write(be_reg, static_reg_cfg->chr_b_sft[0]); /* array index 0 */
    isp_sharpen_chrbsft1_write(be_reg, static_reg_cfg->chr_b_sft[1]); /* array index 1 */
    isp_sharpen_chrbsft2_write(be_reg, static_reg_cfg->chr_b_sft[2]); /* array index 2 */
    isp_sharpen_chrbsft3_write(be_reg, static_reg_cfg->chr_b_sft[3]); /* array index 3 */
    isp_sharpen_chrbvarshift_write(be_reg, static_reg_cfg->chr_b_var_sft);
    isp_sharpen_chrgsft0_write(be_reg, static_reg_cfg->chr_g_sft[0]); /* array index 0 */
    isp_sharpen_chrgsft1_write(be_reg, static_reg_cfg->chr_g_sft[1]); /* array index 1 */
    isp_sharpen_chrgsft2_write(be_reg, static_reg_cfg->chr_g_sft[2]); /* array index 2 */
    isp_sharpen_chrgsft3_write(be_reg, static_reg_cfg->chr_g_sft[3]); /* array index 3 */

    isp_sharpen_chrgori0_write(be_reg, static_reg_cfg->chr_g_ori_cb);
    isp_sharpen_chrgori1_write(be_reg, static_reg_cfg->chr_g_ori_cr);
    isp_sharpen_chrgthd0_write(be_reg, static_reg_cfg->chr_g_thd[0]); /* array index 0 */
    isp_sharpen_chrgthd1_write(be_reg, static_reg_cfg->chr_g_thd[1]); /* array index 1 */
    isp_sharpen_chrrori0_write(be_reg, static_reg_cfg->chr_r_ori_cb);
    isp_sharpen_chrrori1_write(be_reg, static_reg_cfg->chr_r_ori_cr);
    isp_sharpen_chrrthd0_write(be_reg, static_reg_cfg->chr_r_thd[0]); /* array index 0 */
    isp_sharpen_chrrthd1_write(be_reg, static_reg_cfg->chr_r_thd[1]); /* array index 1 */

    isp_sharpen_chrbori0_write(be_reg, static_reg_cfg->chr_b_ori_cb);
    isp_sharpen_chrbori1_write(be_reg, static_reg_cfg->chr_b_ori_cr);
    isp_sharpen_chrbthd0_write(be_reg, static_reg_cfg->chr_b_thd[0]); /* array index 0 */
    isp_sharpen_chrbthd1_write(be_reg, static_reg_cfg->chr_b_thd[1]); /* array index 1 */
}

static td_void isp_sharpen_static_reg_config(isp_post_be_reg_type *be_reg, isp_sharpen_static_reg_cfg *static_reg_cfg)
{
    isp_sharpen_skincntthd0_write(be_reg, static_reg_cfg->skin_cnt_thd[0]); /* array index 0 */
    isp_sharpen_skincntthd1_write(be_reg, static_reg_cfg->skin_cnt_thd[1]); /* array index 1 */
    isp_sharpen_skincntmul_write(be_reg, static_reg_cfg->skin_cnt_mul);
    isp_sharpen_dhsfcoef_d0_write(be_reg, static_reg_cfg->hsf_coef_d0);
    isp_sharpen_dhsfcoef_d1_write(be_reg, static_reg_cfg->hsf_coef_d1);
    isp_sharpen_dhsfcoef_d2_write(be_reg, static_reg_cfg->hsf_coef_d2);
    isp_sharpen_dhsfcoef_d3_write(be_reg, static_reg_cfg->hsf_coef_d3);

    isp_sharpen_hfgain_sft_write(be_reg, static_reg_cfg->hf_gain_sft);
    isp_sharpen_mfgain_sft_write(be_reg, static_reg_cfg->mf_gain_sft);
    isp_sharpen_lfgainwgt_write(be_reg, static_reg_cfg->lf_gain_wgt);

    isp_sharpen_lmt_hf_write(be_reg, static_reg_cfg->lmt_hf);
    isp_sharpen_lmt_mf_write(be_reg, static_reg_cfg->lmt_mf);

    isp_sharpen_udhsfcoef0_write(be_reg, static_reg_cfg->hsf_coef_ud[0]); /* array index 0 */
    isp_sharpen_udhsfcoef1_write(be_reg, static_reg_cfg->hsf_coef_ud[1]); /* array index 1 */
    isp_sharpen_udhsfcoef2_write(be_reg, static_reg_cfg->hsf_coef_ud[2]); /* array index 2 */
    isp_sharpen_udhsfsft_write(be_reg, static_reg_cfg->hsf_sft_ud);
    isp_sharpen_omaxchg_write(be_reg, static_reg_cfg->o_max_chg);
    isp_sharpen_umaxchg_write(be_reg, static_reg_cfg->u_max_chg);
    isp_sharpen_dhsfsft_write(be_reg, static_reg_cfg->hsf_sft_d);
    isp_sharpen_chr_write(be_reg, static_reg_cfg);

    isp_sharpen_dlpfcoef0_write(be_reg, static_reg_cfg->lpf_coef_d[0]); /* array index 0 */
    isp_sharpen_dlpfcoef1_write(be_reg, static_reg_cfg->lpf_coef_d[1]); /* array index 1 */
    isp_sharpen_dlpfcoef2_write(be_reg, static_reg_cfg->lpf_coef_d[2]); /* array index 2 */
    isp_sharpen_dlpfsft_write(be_reg, static_reg_cfg->lpf_sft_d);
    isp_sharpen_udlpfcoef0_write(be_reg, static_reg_cfg->lpf_coef_ud[0]); /* array index 0 */
    isp_sharpen_udlpfcoef1_write(be_reg, static_reg_cfg->lpf_coef_ud[1]); /* array index 1 */
    isp_sharpen_udlpfcoef2_write(be_reg, static_reg_cfg->lpf_coef_ud[2]); /* array index 2 */
    isp_sharpen_udlpfsft_write(be_reg, static_reg_cfg->lpf_sft_ud);

    isp_sharpen_ben8dir_sel_write(be_reg, static_reg_cfg->en_shp8_dir);
    isp_sharpen_benshtvar_sel_write(be_reg, static_reg_cfg->sht_var_sel);
    isp_sharpen_detailthd_sel_write(be_reg, static_reg_cfg->detail_thd_sel);
    isp_sharpen_dirrly0_write(be_reg, static_reg_cfg->dir_rly[0]); /* array index 0 */
    isp_sharpen_dirrly1_write(be_reg, static_reg_cfg->dir_rly[1]); /* array index 1 */
    isp_sharpen_dirvarscale_write(be_reg, static_reg_cfg->dir_var_scale);
    isp_sharpen_mhfthdseld_write(be_reg, static_reg_cfg->gain_thd_sel_d);
    isp_sharpen_max_var_clip_write(be_reg, static_reg_cfg->max_var_clip_min);
    isp_sharpen_shtvarthd0_write(be_reg, static_reg_cfg->sht_var_thd0);
    isp_sharpen_shtvarwgt1_write(be_reg, static_reg_cfg->sht_var_wgt1);
    isp_sharpen_shtvardiffwgt0_write(be_reg, static_reg_cfg->sht_var_diff_wgt0);
    isp_sharpen_shtvar5x5_sft_write(be_reg, static_reg_cfg->sht_var5x5_sft);
    isp_sharpen_shtvarsft_write(be_reg, static_reg_cfg->sht_var_sft);
    isp_sharpen_skinedgesft_write(be_reg, static_reg_cfg->skin_edge_sft);
    isp_sharpen_skinedgethd0_write(be_reg, static_reg_cfg->skin_edge_thd[0]); /* array index 0 */
    isp_sharpen_skinedgethd1_write(be_reg, static_reg_cfg->skin_edge_thd[1]); /* array index 1 */
    isp_sharpen_dtl_thdsft_write(be_reg, static_reg_cfg->detail_thd_sft);
}

static td_s32 isp_sharpen_usr_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_post_be_reg_type *be_reg,
    isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        if (mpi_dyna_reg_cfg->switch_mode != TD_TRUE) {
            /* online lut2stt regconfig */
            buf_id = mpi_dyna_reg_cfg->buf_id;
            be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
            isp_check_pointer_return(be_lut_stt_reg);

            isp_sharpen_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, mpi_dyna_reg_cfg->mf_gain_d,
                mpi_dyna_reg_cfg->mf_gain_ud, mpi_dyna_reg_cfg->hf_gain_d, mpi_dyna_reg_cfg->hf_gain_ud);

            ret = isp_sharpen_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
            if (ret != TD_SUCCESS) {
                isp_err_trace("ISP[%d] isp_sharpen_lut_wstt_addr_write failed\n", vi_pipe);
                return ret;
            }

            isp_sharpen_stt2lut_en_write(be_reg, TD_TRUE);
            isp_sharpen_stt2lut_regnew_write(be_reg, TD_TRUE);

            mpi_dyna_reg_cfg->buf_id = 1 - buf_id;
        }
    } else {
        isp_sharpen_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, mpi_dyna_reg_cfg->mf_gain_d,
            mpi_dyna_reg_cfg->mf_gain_ud, mpi_dyna_reg_cfg->hf_gain_d, mpi_dyna_reg_cfg->hf_gain_ud);

        isp_sharpen_stt2lut_en_write(be_reg, TD_TRUE);
        isp_sharpen_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_sharpen_online_lut2stt_info_check(td_bool offline_mode, isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_sharpen_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        isp_sharpen_stt2lut_regnew_write(be_reg, TD_TRUE);
        isp_sharpen_stt2lut_clr_write(be_reg, 1);
    }
}
static td_s32 isp_sharpen_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_bool usr_resh;
    td_bool idx_resh;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_sharpen_mpi_dyna_reg_cfg *mpi_dyna_reg_cfg = TD_NULL;
    isp_sharpen_static_reg_cfg *static_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_sharpen_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(post_viproc);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].sharpen_reg_cfg.static_reg_cfg;
        mpi_dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg;

        if (static_reg_cfg->static_resh) {
            isp_sharpen_lut_width_word_write(post_viproc, OT_ISP_SHARPEN_LUT_WIDTH_WORD_DEFAULT);

            isp_sharpen_static_reg_config(be_reg, static_reg_cfg);
            static_reg_cfg->static_resh = TD_FALSE;
        }

        idx_resh = (isp_sharpen_update_index_read(be_reg) != mpi_dyna_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (mpi_dyna_reg_cfg->resh && idx_resh) : (mpi_dyna_reg_cfg->resh);

        if (usr_resh) {
            isp_sharpen_update_index_write(be_reg, mpi_dyna_reg_cfg->update_index);

            ret = isp_sharpen_usr_reg_config(vi_pipe, i, be_reg, mpi_dyna_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }

            mpi_dyna_reg_cfg->resh = offline_mode;
        }
        isp_sharpen_online_lut2stt_info_check(offline_mode, be_reg);
        reg_cfg_info->cfg_key.bit1_sharpen_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_demosaic_lut_reg_config(const isp_usr_ctx *isp_ctx, isp_post_be_reg_type *be_reg,
    isp_demosaic_dyna_cfg *dyna_reg_cfg)
{
    td_u16 j;

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        isp_nddm_gf_lut_waddr_write(be_reg, 0);

        for (j = 0; j < OT_ISP_DEMOSAIC_LUT_LENGTH; j++) {
            isp_nddm_gf_lut_wdata_write(be_reg, dyna_reg_cfg->gf_blur_luts[j]);
        }
    } else {
        isp_nddm_gflut_write(&be_reg->be_lut.be_apb_lut, dyna_reg_cfg->gf_blur_luts);
    }
}

static td_s32 isp_demosaic_aidm_detial_lut_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, td_u8 i,
    isp_post_be_reg_type *be_reg, isp_demosaic_static_cfg *usr_reg_cfg)
{
    td_u8  buf_id;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);
    isp_dmnr_lut_width_word_write(post_viproc, OT_ISP_DEMOSAIC_AIDM_DETAIL_LUT_WIDTH_WORD_DEFAULT);
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        buf_id = usr_reg_cfg->buf_id;
        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_return(be_lut_stt_reg);

        isp_dmnr_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->aidm_detail_lut);

        ret = isp_dmnr_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_demosaic_aidm_detail_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }
        isp_dmnr_stt2lut_en_write(be_reg, TD_TRUE);

        usr_reg_cfg->buf_id = 1 - buf_id;
        *stt2_lut_regnew = TD_TRUE;
    } else {
        isp_dmnr_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->aidm_detail_lut);

        isp_dmnr_stt2lut_en_write(be_reg, TD_TRUE);
        isp_dmnr_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_demosaic_static_reg_write(isp_post_be_reg_type *be_reg, isp_demosaic_static_cfg *static_reg_cfg)
{
    isp_demosaic_desat_enable_write(be_reg, static_reg_cfg->de_sat_enable);
    isp_demosaic_ahd_en_write(be_reg, static_reg_cfg->ahd_enable);
    isp_demosaic_de_fake_en_write(be_reg, static_reg_cfg->de_fake_enable);
    isp_demosaic_lpf_f3_write(be_reg, static_reg_cfg->lpff3);
    isp_demosaic_desat_thresh1_write(be_reg, static_reg_cfg->de_sat_thresh1);
    isp_demosaic_desat_thresh2_write(be_reg, static_reg_cfg->de_sat_thresh2);
    isp_demosaic_desat_hig_write(be_reg, static_reg_cfg->de_sat_hig);
    isp_demosaic_desat_protect_sl_write(be_reg, static_reg_cfg->de_sat_prot_sl);
    isp_demosaic_bld_limit1_write(be_reg, static_reg_cfg->hv_blend_limit1);
    isp_demosaic_bld_limit2_write(be_reg, static_reg_cfg->hv_blend_limit2);
    isp_demosaic_ahd_par1_write(be_reg, static_reg_cfg->ahd_part1);
    isp_demosaic_ahd_par2_write(be_reg, static_reg_cfg->ahd_part2);
    isp_demosaic_cc_var_thresh_write(be_reg, static_reg_cfg->cc_var_thresh);
    isp_demosaic_g_clip_sft_bit_write(be_reg, static_reg_cfg->g_clip_bit_sft);
    isp_demosaic_hv_ratio_write(be_reg, static_reg_cfg->hv_color_ratio);
    isp_demosaic_hv_sel_write(be_reg, static_reg_cfg->hv_selection);
    isp_demosaic_cbcr_avg_thld_write(be_reg, static_reg_cfg->cb_cr_avg_thr);
    isp_nddm_dith_mask_write(be_reg, static_reg_cfg->dither_mask);
    isp_nddm_dith_ratio_write(be_reg, static_reg_cfg->dither_ratio);
    isp_nddm_gf_th_low_write(be_reg, static_reg_cfg->gf_th_low);
    isp_nddm_gf_th_high_write(be_reg, static_reg_cfg->gf_th_hig);
}

static td_void isp_demosaic_dyna_reg_write(isp_post_be_reg_type *be_reg, isp_demosaic_dyna_cfg *dyna_reg_cfg)
{
    isp_demosaic_fake_cr_var_thr_high_write(be_reg, dyna_reg_cfg->fake_cr_var_thr_high);
    isp_demosaic_fake_cr_var_thr_low_write(be_reg, dyna_reg_cfg->fake_cr_var_thr_low);
    isp_demosaic_defcolor_cr_write(be_reg, dyna_reg_cfg->de_f_color_cr);
    isp_demosaic_defcolor_cb_write(be_reg, dyna_reg_cfg->de_f_color_cb);
    isp_demosaic_cc_hf_max_ratio_write(be_reg, dyna_reg_cfg->cc_hf_max_ratio);
    isp_demosaic_cc_hf_min_ratio_write(be_reg, dyna_reg_cfg->cc_hf_min_ratio);
    isp_demosaic_lpf_f0_write(be_reg, dyna_reg_cfg->lpff0);
    isp_demosaic_lpf_f1_write(be_reg, dyna_reg_cfg->lpff1);
    isp_demosaic_lpf_f2_write(be_reg, dyna_reg_cfg->lpff2);
    isp_demosaic_desat_low_write(be_reg, dyna_reg_cfg->de_sat_low);
    isp_demosaic_desat_ratio_write(be_reg, dyna_reg_cfg->de_sat_ratio);
    isp_demosaic_desat_protect_th_write(be_reg, dyna_reg_cfg->de_sat_prot_th);
    isp_nddm_dith_max_write(be_reg, dyna_reg_cfg->dither_max);
    isp_nddm_fcr_gf_gain_write(be_reg, dyna_reg_cfg->fcr_gf_gain);
    isp_nddm_awb_gf_gn_low_write(be_reg, dyna_reg_cfg->awb_gf_gain_low);
    isp_nddm_awb_gf_gn_high_write(be_reg, dyna_reg_cfg->awb_gf_gain_hig);
    isp_nddm_awb_gf_gn_max_write(be_reg, dyna_reg_cfg->awb_gf_gain_max);
    isp_nddm_ehc_gray_write(be_reg, dyna_reg_cfg->ehc_gray);
    isp_demosaic_hf_intp_blur_th1_write(be_reg, dyna_reg_cfg->hf_intp_blur_th_low);
    isp_demosaic_hf_intp_blur_th2_write(be_reg, dyna_reg_cfg->hf_intp_blur_th_hig);
    isp_demosaic_hf_intp_blur_ratio_write(be_reg, dyna_reg_cfg->hf_intp_blur_ratio);
    isp_nddm_fcr_det_low_write(be_reg, dyna_reg_cfg->fcr_det_low);
    isp_cc_soft_clip0_step_write(be_reg, dyna_reg_cfg->color_noise_str_y);
    isp_cc_darkprev_write(be_reg, OT_ISP_CCM_DARK_RANGE_MAX - dyna_reg_cfg->color_noise_thd_y);
}

static td_void isp_demosaic_usr_reg_write(isp_post_be_reg_type *be_reg, isp_demosaic_usr_cfg *usr_reg_cfg)
{
    isp_dmnr_aidm_blendrate_write(be_reg, usr_reg_cfg->ai_detail_strength);
}

static td_void isp_demosaic_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_dmnr_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_dmnr_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_demosaic_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_bool gf_lut_update = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool  stt2_lut_regnew = TD_FALSE;
    td_bool usr_resh = TD_FALSE;
    td_u8 idx_resh;
    isp_demosaic_static_cfg *static_reg_cfg = TD_NULL;
    isp_demosaic_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_demosaic_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);

    if (reg_cfg_info->cfg_key.bit1_dem_cfg) {
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dem_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dem_reg_cfg.dyna_reg_cfg;

        if (static_reg_cfg->resh) { /* static */
            isp_demosaic_static_reg_write(be_reg, static_reg_cfg);
            isp_demosaic_aidm_detial_lut_reg_config(&stt2_lut_regnew, vi_pipe, i, be_reg, static_reg_cfg);
            reg_cfg_info->alg_reg_cfg[i].dem_reg_cfg.static_reg_cfg.resh = TD_FALSE;
        }
        if (dyna_reg_cfg->resh) { /* dynamic */
            isp_demosaic_dyna_reg_write(be_reg, dyna_reg_cfg);
            isp_demosaic_lut_reg_config(isp_ctx, be_reg, dyna_reg_cfg);
            gf_lut_update = dyna_reg_cfg->update_gf;
            dyna_reg_cfg->resh = offline_mode;
        }

        /* usr */
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dem_reg_cfg.usr_reg_cfg;
        idx_resh = (isp_demosaic_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? ((td_u8)usr_reg_cfg->resh & idx_resh) : (usr_reg_cfg->resh);
        if (usr_resh) {
            isp_demosaic_update_index_write(be_reg, usr_reg_cfg->update_index);
            isp_demosaic_usr_reg_write(be_reg, usr_reg_cfg);
            usr_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_dem_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
    isp_demosaic_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.nddm_gf_lut_update = gf_lut_update || offline_mode;
    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.dmnr_stt2lut_regnew = stt2_lut_regnew;

    return TD_SUCCESS;
}

static td_s32 isp_fpn_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
#ifdef CONFIG_OT_VI_PIPE_FPN
    td_bool offline_mode;
    td_u8 blk_num = reg_cfg_info->cfg_num;

    isp_fpn_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_fpn_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);

        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].fpn_reg_cfg.dyna_reg_cfg;

        isp_fpn_overflowthr_write(be_reg, dyna_reg_cfg->isp_fpn_overflow_thr);
        isp_fpn_strength0_write(be_reg, dyna_reg_cfg->isp_fpn_strength[0]);

        reg_cfg_info->cfg_key.bit1_fpn_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
#else
    ot_unused(vi_pipe);
    ot_unused(reg_cfg_info);
    ot_unused(i);
#endif
    return TD_SUCCESS;
}

static td_s32 isp_ldci_read_stt_addr_write(isp_viproc_reg_type *post_viproc, td_u64 phy_addr)
{
    if (phy_addr == 0) {
        return TD_FAILURE;
    }

    viproc_para_dci_addr_low_write(post_viproc, get_low_addr(phy_addr));
    viproc_para_dci_addr_high_write(post_viproc, get_high_addr(phy_addr));

    return TD_SUCCESS;
}

static td_s32 isp_ldci_static_lut_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_post_be_reg_type *be_reg,
    isp_ldci_dyna_cfg *dyna_reg_cfg)
{
    td_u16 k;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        for (k = 0; k < 2; k++) { /* config all 2 lut2stt buffer for static lut */
            be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, k);
            isp_check_pointer_return(be_lut_stt_reg);
            isp_ldci_drc_cgain_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, dyna_reg_cfg->color_gain_lut);
            isp_ldci_usm_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, dyna_reg_cfg->usm_pos_lut,
                dyna_reg_cfg->usm_neg_lut);
        }
    } else {
        isp_ldci_drc_cgain_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, dyna_reg_cfg->color_gain_lut);
        isp_ldci_usm_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, dyna_reg_cfg->usm_pos_lut,
            dyna_reg_cfg->usm_neg_lut);
    }

    return TD_SUCCESS;
}

static td_s32 isp_ldci_tpr_flt_attr_config(ot_vi_pipe vi_pipe, td_u8 i, isp_post_be_reg_type *be_reg,
    isp_viproc_reg_type *post_viproc, isp_ldci_dyna_cfg *dyna_reg_cfg)
{
    td_bool rdstat_en;
    td_u8 read_buf_idx;
    td_u8 blk_num;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_ldci_read_stt_buf *ldci_read_stt_buf = TD_NULL;
    isp_ldci_stat *read_stt_buf = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    blk_num = isp_ctx->block_attr.block_num;

    if (isp_ctx->ldci_tpr_flt_en == TD_TRUE) {
        ldci_buf_get_ctx(vi_pipe, ldci_read_stt_buf);
        read_buf_idx = ldci_read_stt_buf->buf_idx;

        if (i == 0) {
            read_stt_buf = (isp_ldci_stat *)isp_get_ldci_read_stt_vir_addr(vi_pipe, read_buf_idx);
            isp_check_pointer_return(read_stt_buf);
            (td_void)memcpy_s(read_stt_buf, sizeof(isp_ldci_stat), &dyna_reg_cfg->tpr_stat, sizeof(isp_ldci_stat));
        }

        if ((i + 1) == blk_num) {
            ldci_read_stt_buf->buf_idx = (read_buf_idx + 1) % div_0_to_1(ldci_read_stt_buf->buf_num);
        }

        /* Set ReadStt Addr */
        ret = isp_ldci_read_stt_addr_write(post_viproc, ldci_read_stt_buf->read_buf[read_buf_idx].phy_addr);
        rdstat_en = (ret == TD_SUCCESS) ? (dyna_reg_cfg->rdstat_en) : (TD_FALSE);
        isp_ldci_rdstat_en_write(be_reg, rdstat_en);
    } else {
        isp_ldci_rdstat_en_write(be_reg, dyna_reg_cfg->rdstat_en);
    }

    return TD_SUCCESS;
}

static td_s32 isp_ldci_dyna_lut_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_post_be_reg_type *be_reg,
    isp_viproc_reg_type *post_viproc, isp_ldci_dyna_cfg *dyna_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        /* online Lut2stt regconfig */
        buf_id = dyna_reg_cfg->buf_id;

        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_return(be_lut_stt_reg);

        isp_ldci_he_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt,
            dyna_reg_cfg->he_pos_lut, dyna_reg_cfg->he_neg_lut);
        ret = isp_ldci_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_ldci_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }
        dyna_reg_cfg->buf_id = 1 - buf_id;
    } else {
        isp_ldci_he_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt,
            dyna_reg_cfg->he_pos_lut, dyna_reg_cfg->he_neg_lut);
    }

    isp_ldci_stt2lut_en_write(be_reg, TD_TRUE);
    isp_ldci_stt2lut_regnew_write(be_reg, TD_TRUE);

    return TD_SUCCESS;
}

static td_s32 isp_ldci_static_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i,
    isp_post_be_reg_type *be_reg)
{
    td_s32 ret;
    isp_ldci_static_cfg *static_reg_cfg = TD_NULL;
    isp_ldci_dyna_cfg *dyna_reg_cfg = TD_NULL;

    static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.static_reg_cfg;
    dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.dyna_reg_cfg;

    isp_ldci_stat_evratio_write(be_reg, 0x1000);
    isp_ldci_luma_sel_write(be_reg, static_reg_cfg->calc_luma_sel);
    isp_ldci_lpfsft_write(be_reg, static_reg_cfg->lpf_sft);
    isp_ldci_chrposdamp_write(be_reg, static_reg_cfg->chr_pos_damp);
    isp_ldci_chrnegdamp_write(be_reg, static_reg_cfg->chr_neg_damp);

    ret = isp_ldci_static_lut_reg_config(vi_pipe, i, be_reg, dyna_reg_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ldci_dyna_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_post_be_reg_type *be_reg,
    isp_viproc_reg_type *post_viproc, isp_ldci_dyna_cfg *dyna_reg_cfg)
{
    td_s32 ret;
    ret = isp_ldci_tpr_flt_attr_config(vi_pipe, i, be_reg, post_viproc, dyna_reg_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_ldci_wrstat_en_write(be_reg, dyna_reg_cfg->wrstat_en);
    isp_ldci_calc_en_write(be_reg, dyna_reg_cfg->calc_enable);
    isp_ldci_calc_map_offsetx_write(be_reg, dyna_reg_cfg->calc_map_offset_x);
    isp_ldci_smlmapstride_write(be_reg, dyna_reg_cfg->calc_sml_map_stride);
    isp_ldci_smlmapheight_write(be_reg, dyna_reg_cfg->calc_sml_map_height);
    isp_ldci_total_zone_write(be_reg, dyna_reg_cfg->calc_total_zone);
    isp_ldci_scalex_write(be_reg, dyna_reg_cfg->calc_scale_x);
    isp_ldci_scaley_write(be_reg, dyna_reg_cfg->calc_scale_y);
    isp_ldci_stat_smlmapwidth_write(be_reg, dyna_reg_cfg->stat_sml_map_width);
    isp_ldci_stat_smlmapheight_write(be_reg, dyna_reg_cfg->stat_sml_map_height);
    isp_ldci_stat_total_zone_write(be_reg, dyna_reg_cfg->stat_total_zone);
    isp_ldci_blk_smlmapwidth0_write(be_reg, dyna_reg_cfg->blk_sml_map_width[0]); /* array index 0 */
    isp_ldci_blk_smlmapwidth1_write(be_reg, dyna_reg_cfg->blk_sml_map_width[1]); /* array index 1 */
    isp_ldci_blk_smlmapwidth2_write(be_reg, dyna_reg_cfg->blk_sml_map_width[2]); /* array index 2 */
    isp_ldci_hstart_write(be_reg, dyna_reg_cfg->stat_h_start);
    isp_ldci_hend_write(be_reg, dyna_reg_cfg->stat_h_end);
    isp_ldci_vstart_write(be_reg, dyna_reg_cfg->stat_v_start);
    isp_ldci_vend_write(be_reg, dyna_reg_cfg->stat_v_end);

    ret = isp_ldci_dyna_lut_reg_config(vi_pipe, i, be_reg, post_viproc, dyna_reg_cfg);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_ldci_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode, cur_ldci_calc_lut_reg_new;
    td_bool ldci_drc_lut_update = TD_FALSE;
    td_bool cur_ldci_drc_lut_reg_new = TD_FALSE;
    td_bool ldci_calc_lut_update = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_ldci_static_cfg *static_reg_cfg = TD_NULL;
    isp_ldci_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_ldci_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(post_viproc);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.dyna_reg_cfg;

        if (static_reg_cfg->static_resh) {
            isp_ldci_lut_width_word_write(post_viproc, OT_ISP_LDCI_LUT_WIDTH_WORD_DEFAULT);

            ret = isp_ldci_static_reg_config(vi_pipe, reg_cfg_info, i, be_reg);
            if (ret != TD_SUCCESS) {
                return ret;
            }

            cur_ldci_drc_lut_reg_new = TD_TRUE;
            static_reg_cfg->static_resh = TD_FALSE;
        }

        ldci_drc_lut_update = static_reg_cfg->pre_drc_lut_update;
        static_reg_cfg->pre_drc_lut_update = cur_ldci_drc_lut_reg_new;

        /* dynamic */
        isp_ldci_en_write(post_viproc, dyna_reg_cfg->enable);
        ret = isp_ldci_dyna_reg_config(vi_pipe, i, be_reg, post_viproc, dyna_reg_cfg);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        cur_ldci_calc_lut_reg_new = TD_TRUE;
        ldci_calc_lut_update = dyna_reg_cfg->pre_calc_lut_reg_new;
        dyna_reg_cfg->pre_calc_lut_reg_new = cur_ldci_calc_lut_reg_new;

        reg_cfg_info->cfg_key.bit1_ldci_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.ldci_calc_lut_update = ldci_calc_lut_update || offline_mode;
    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.ldci_drc_lut_update = ldci_drc_lut_update || offline_mode;

    return TD_SUCCESS;
}

static td_s32 isp_fcr_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_antifalsecolor_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_antifalsecolor_static_cfg *static_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_fcr_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].anti_false_color_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].anti_false_color_reg_cfg.dyna_reg_cfg;

        /* static */
        if (static_reg_cfg->resh) {
            isp_demosaic_fcr_limit1_write(be_reg, static_reg_cfg->fcr_limit1);
            isp_demosaic_fcr_limit2_write(be_reg, static_reg_cfg->fcr_limit2);
            static_reg_cfg->resh = TD_FALSE;
        }

        /* dynamic */
        if (dyna_reg_cfg->resh) {
            isp_demosaic_fcr_gain_write(be_reg, dyna_reg_cfg->fcr_gain);
            isp_demosaic_fcr_ratio_write(be_reg, dyna_reg_cfg->fcr_ratio);
            isp_demosaic_fcr_gray_ratio_write(be_reg, dyna_reg_cfg->fcr_gray_ratio);
            isp_demosaic_fcr_cmax_sel_write(be_reg, dyna_reg_cfg->fcr_cmax_sel);
            isp_demosaic_fcr_detg_sel_write(be_reg, dyna_reg_cfg->fcr_detg_sel);
            isp_demosaic_fcr_thresh1_write(be_reg, dyna_reg_cfg->fcr_hf_thresh_low);
            isp_demosaic_fcr_thresh2_write(be_reg, dyna_reg_cfg->fcr_hf_thresh_hig);
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_fcr_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_cac_static_reg_config(isp_be_reg_type *be_reg, isp_cac_static_cfg *static_reg_cfg)
{
    isp_demosaic_local_cac_en_write(be_reg, static_reg_cfg->local_cac_en);
    isp_gcac_normfactor_write(be_reg, static_reg_cfg->cac_norm_factor);
    isp_gcac_upperlmtsft_write(be_reg, static_reg_cfg->cac_upper_limit_shift);
    isp_gcac_lowerlmtsft_write(be_reg, static_reg_cfg->cac_lower_limit_shift);
    isp_demosaic_r_counter_thr_write(be_reg, static_reg_cfg->r_counter_thr);
    isp_demosaic_g_counter_thr_write(be_reg, static_reg_cfg->g_counter_thr);
    isp_demosaic_b_counter_thr_write(be_reg, static_reg_cfg->b_counter_thr);
    isp_gcac_out_diffthd_write(be_reg, static_reg_cfg->diff_thd);
}
static td_void isp_cac_usr_reg_config(isp_be_reg_type *be_reg, isp_cac_usr_cfg *usr_reg_cfg)
{
    isp_gcac_update_index_write(be_reg, usr_reg_cfg->update_index);
    isp_gcac_edgemode_write(be_reg, usr_reg_cfg->cac_det_mode);
    isp_demosaic_cbcr_ratio_high_limit_write(be_reg, usr_reg_cfg->cr_cb_ratio_high_limit);
    isp_demosaic_cbcr_ratio_low_limit_write(be_reg, usr_reg_cfg->cr_cb_ratio_low_limit);
    isp_demosaic_r_luma_thr_write(be_reg, usr_reg_cfg->r_luma_thr);
    isp_demosaic_g_luma_thr_write(be_reg, usr_reg_cfg->g_luma_thr);
    isp_demosaic_b_luma_thr_write(be_reg, usr_reg_cfg->b_luma_thr);
    isp_demosaic_purple_var_thr_write(be_reg, usr_reg_cfg->var_thr);
}

static td_void isp_cac_dyna_reg_config(isp_be_reg_type *be_reg, isp_cac_dyna_cfg *dyna_reg_cfg)
{
    isp_gcac_globalstr_write(be_reg, dyna_reg_cfg->cac_edge_str);
    isp_gcac_lamdathd1_write(be_reg, dyna_reg_cfg->lamda_thd0);
    isp_gcac_lamdathd2_write(be_reg, dyna_reg_cfg->lamda_thd1);
    isp_gcac_lamdamul_write(be_reg, dyna_reg_cfg->lamda_mul);
    isp_gcac_thrbthd1_write(be_reg, dyna_reg_cfg->edge_thd0);
    isp_gcac_thrbthd2_write(be_reg, dyna_reg_cfg->edge_thd1);
    isp_gcac_thrbmul_write(be_reg, dyna_reg_cfg->edge_thd_mul);
    isp_gcac_distance_thd1_write(be_reg, dyna_reg_cfg->distance_thd0);
    isp_gcac_distance_thd2_write(be_reg, dyna_reg_cfg->distance_thd1);
    isp_gcac_distance_mul_write(be_reg, dyna_reg_cfg->distance_mul);
    isp_gcac_tao_write(be_reg, dyna_reg_cfg->cac_tao);
    isp_gcac_purplealpha_write(be_reg, dyna_reg_cfg->purple_alpha);
    isp_gcac_edge_alpha_write(be_reg, dyna_reg_cfg->edge_alpha);
    isp_gcac_desatuthr_write(be_reg, dyna_reg_cfg->det_satu_thr);
    isp_gcac_desatuthr_hig_write(be_reg, dyna_reg_cfg->det_satu_thr_high);
    isp_demosaic_depurplectrcr_write(be_reg, dyna_reg_cfg->de_purple_ctr_cr);
    isp_demosaic_depurplectrcb_write(be_reg, dyna_reg_cfg->de_purple_ctr_cb);
}

static td_s32 isp_cac_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool usr_resh = TD_FALSE;
    td_u8 idx_resh;
    isp_cac_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_cac_static_cfg *static_reg_cfg = TD_NULL;
    isp_cac_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));
    if (reg_cfg_info->cfg_key.bit1_cac_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        /* static */
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].cac_reg_cfg.static_reg_cfg;
        if (static_reg_cfg->static_resh) {
            isp_cac_static_reg_config(be_reg, static_reg_cfg);
            static_reg_cfg->static_resh = TD_FALSE;
        }
        /* usr */
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].cac_reg_cfg.usr_reg_cfg;
        idx_resh = (isp_gcac_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? ((td_u8)usr_reg_cfg->usr_resh & idx_resh) : (usr_reg_cfg->usr_resh);
        if (usr_resh) {
            isp_cac_usr_reg_config(be_reg, usr_reg_cfg);
            usr_reg_cfg->usr_resh = offline_mode;
        }
        /* dyna */
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].cac_reg_cfg.dyna_reg_cfg;
        if (dyna_reg_cfg->dyna_resh) {
            isp_cac_dyna_reg_config(be_reg, dyna_reg_cfg);
            dyna_reg_cfg->dyna_resh = TD_FALSE;
        }
        reg_cfg_info->cfg_key.bit1_cac_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_bshp_static_reg_write(isp_post_be_reg_type *be_reg, isp_bshp_static_cfg *static_reg_cfg)
{
    isp_bnrshp_enshtctrl_byvar_write(be_reg, static_reg_cfg->sht_ctrl_by_var_en);
    isp_bnrshp_enshtctrl_shrink_write(be_reg, static_reg_cfg->sht_ctrl_shrink_en);
    isp_bnrshp_bitdepth_write(be_reg, static_reg_cfg->bit_depth);
    isp_bnrshp_dirrate_hv_write(be_reg, static_reg_cfg->dir_rate_hv);
    isp_bnrshp_dirrate_cd_write(be_reg, static_reg_cfg->dir_rate_cd);
    isp_bnrshp_lpfcoefud0_write(be_reg, static_reg_cfg->lpf_coef_ud[0]);            /* idx 0 */
    isp_bnrshp_lpfcoefud1_write(be_reg, static_reg_cfg->lpf_coef_ud[1]);            /* idx 1 */
    isp_bnrshp_lpfcoefud2_write(be_reg, static_reg_cfg->lpf_coef_ud[2]);            /* idx 2 */
    isp_bnrshp_hsfcoefud0_write(be_reg, static_reg_cfg->hsf_coef_ud[0]);            /* idx 0 */
    isp_bnrshp_hsfcoefud1_write(be_reg, static_reg_cfg->hsf_coef_ud[1]);            /* idx 1 */
    isp_bnrshp_hsfcoefud2_write(be_reg, static_reg_cfg->hsf_coef_ud[2]);            /* idx 2 */
    isp_bnrshp_shtctrlvar_wgt0_write(be_reg, static_reg_cfg->sht_ctrl_var_wgt[0]);  /* idx 0 */
    isp_bnrshp_shtctrlvar_wgt1_write(be_reg, static_reg_cfg->sht_ctrl_var_wgt[1]);  /* idx 1 */
}

static td_void isp_bshp_user_reg_write(isp_post_be_reg_type *be_reg, isp_bshp_user_cfg *usr_reg_cfg)
{
    isp_bnrshp_darkthd0_write(be_reg, usr_reg_cfg->dark_thd[0]);                    /* idx 0 */
    isp_bnrshp_darkthd1_write(be_reg, usr_reg_cfg->dark_thd[1]);                    /* idx 1 */
    isp_bnrshp_shtctrlvar_thd0_write(be_reg, usr_reg_cfg->sht_ctrl_var_thd[0]);     /* idx 0 */
    isp_bnrshp_shtctrlvar_thd1_write(be_reg, usr_reg_cfg->sht_ctrl_var_thd[1]);     /* idx 1 */
}


static td_void isp_bshp_dyna_reg_write(isp_post_be_reg_type *be_reg, isp_bshp_dyna_cfg *dyna_reg_cfg)
{
    isp_bnrshp_mf_gain_write(be_reg, dyna_reg_cfg->mf_gain);
    isp_bnrshp_hf_gain_write(be_reg, dyna_reg_cfg->hf_gain);
    isp_bnrshp_darktex_gain_write(be_reg, dyna_reg_cfg->dark_gain);
    isp_bnrshp_texvarmap_thd_write(be_reg, dyna_reg_cfg->tex_var_map_thd);
    isp_bnrshp_osht_amt_write(be_reg, dyna_reg_cfg->over_shoot_amt);
    isp_bnrshp_usht_amt_write(be_reg, dyna_reg_cfg->under_shoot_amt);
    isp_bnrshp_omaxchg_write(be_reg, dyna_reg_cfg->over_max_chg);
    isp_bnrshp_umaxchg_write(be_reg, dyna_reg_cfg->under_max_chg);
    isp_bnrshp_mfgain0_write(be_reg, dyna_reg_cfg->mf_gain_lut[0]);                  /* idx 0 */
    isp_bnrshp_mfgain1_write(be_reg, dyna_reg_cfg->mf_gain_lut[1]);                  /* idx 1 */
    isp_bnrshp_mfgain2_write(be_reg, dyna_reg_cfg->mf_gain_lut[2]);                  /* idx 2 */
    isp_bnrshp_mfgain3_write(be_reg, dyna_reg_cfg->mf_gain_lut[3]);                  /* idx 3 */
    isp_bnrshp_mfgain4_write(be_reg, dyna_reg_cfg->mf_gain_lut[4]);                  /* idx 4 */
    isp_bnrshp_mfgain5_write(be_reg, dyna_reg_cfg->mf_gain_lut[5]);                  /* idx 5 */
    isp_bnrshp_mfgain6_write(be_reg, dyna_reg_cfg->mf_gain_lut[6]);                  /* idx 6 */
    isp_bnrshp_mfgain7_write(be_reg, dyna_reg_cfg->mf_gain_lut[7]);                  /* idx 7 */
    isp_bnrshp_hfgain0_write(be_reg, dyna_reg_cfg->hf_gain_lut[0]);                  /* idx 0 */
    isp_bnrshp_hfgain1_write(be_reg, dyna_reg_cfg->hf_gain_lut[1]);                  /* idx 1 */
    isp_bnrshp_hfgain2_write(be_reg, dyna_reg_cfg->hf_gain_lut[2]);                  /* idx 2 */
    isp_bnrshp_hfgain3_write(be_reg, dyna_reg_cfg->hf_gain_lut[3]);                  /* idx 3 */
    isp_bnrshp_hfgain4_write(be_reg, dyna_reg_cfg->hf_gain_lut[4]);                  /* idx 4 */
    isp_bnrshp_hfgain5_write(be_reg, dyna_reg_cfg->hf_gain_lut[5]);                  /* idx 5 */
    isp_bnrshp_hfgain6_write(be_reg, dyna_reg_cfg->hf_gain_lut[6]);                  /* idx 6 */
    isp_bnrshp_hfgain7_write(be_reg, dyna_reg_cfg->hf_gain_lut[7]);                  /* idx 7 */
    isp_bnrshp_darkgain0_write(be_reg, dyna_reg_cfg->dark_gain_lut[0]);              /* idx 0 */
    isp_bnrshp_darkgain1_write(be_reg, dyna_reg_cfg->dark_gain_lut[1]);              /* idx 1 */
    isp_bnrshp_darkgain2_write(be_reg, dyna_reg_cfg->dark_gain_lut[2]);              /* idx 2 */
    isp_bnrshp_darkgain3_write(be_reg, dyna_reg_cfg->dark_gain_lut[3]);              /* idx 3 */
    isp_bnrshp_darkgain4_write(be_reg, dyna_reg_cfg->dark_gain_lut[4]);              /* idx 4 */
    isp_bnrshp_darkgain5_write(be_reg, dyna_reg_cfg->dark_gain_lut[5]);              /* idx 5 */
    isp_bnrshp_darkgain6_write(be_reg, dyna_reg_cfg->dark_gain_lut[6]);              /* idx 6 */
    isp_bnrshp_darkgain7_write(be_reg, dyna_reg_cfg->dark_gain_lut[7]);              /* idx 7 */
}

static td_s32 isp_bshp_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    td_bool lut_update = TD_FALSE;
    isp_bshp_static_cfg *static_reg_cfg = TD_NULL;
    isp_bshp_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_bshp_user_cfg *usr_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *vi_proc_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));
    if (reg_cfg_info->cfg_key.bit1_bshp_cfg) {
        vi_proc_reg = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(vi_proc_reg);
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].bshp_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].bshp_reg_cfg.dyna_reg_cfg;
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].bshp_reg_cfg.usr_reg_cfg;
        if (static_reg_cfg->static_resh) { /* static */
            isp_bshp_static_reg_write(be_reg, static_reg_cfg);
            static_reg_cfg->static_resh = TD_FALSE;
        }
        if (dyna_reg_cfg->dyna_resh) { /* dynamic */
            isp_bshp_dyna_reg_write(be_reg, dyna_reg_cfg);
            dyna_reg_cfg->dyna_resh = TD_FALSE;
        }
        if (usr_reg_cfg->user_resh) { /* usr */
            isp_bshp_user_reg_write(be_reg, usr_reg_cfg);
            usr_reg_cfg->user_resh = (td_bool)offline_mode;
        }
        reg_cfg_info->cfg_key.bit1_bshp_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.bshp_lut_update = (td_bool)(lut_update || offline_mode);

    return TD_SUCCESS;
}

static td_s32 isp_dpc_bpt_lut_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, td_u8 i,
    isp_pre_be_reg_type *be_reg, isp_dpc_usr_cfg *usr_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(pre_viproc);

    if (is_online_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        /* online lut2stt regconfig */
        buf_id = usr_reg_cfg->usr_sta_cor_reg_cfg.buf_id;
        if (is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
            isp_pre_be_lut_wstt_type *pre_lut_stt_reg = TD_NULL;
            pre_lut_stt_reg = (isp_pre_be_lut_wstt_type *)isp_get_pre_on_post_off_lut2stt_vir_addr(vi_pipe, i, buf_id);
            isp_check_pointer_return(pre_lut_stt_reg);
            isp_dpc_lut_wstt_write(pre_lut_stt_reg, usr_reg_cfg->usr_sta_cor_reg_cfg.dpcc_bp_table);
        } else {
            isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
            be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
            isp_check_pointer_return(be_lut_stt_reg);
            isp_dpc_lut_wstt_write(&be_lut_stt_reg->pre_be_lut2stt, usr_reg_cfg->usr_sta_cor_reg_cfg.dpcc_bp_table);
        }

        ret = isp_dpc_lut_wstt_addr_write(vi_pipe, i, buf_id, pre_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_dpc_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }

        isp_dpc_stt2lut_en_write(be_reg, TD_TRUE);

        usr_reg_cfg->usr_sta_cor_reg_cfg.buf_id = 1 - buf_id;

        *stt2_lut_regnew = TD_TRUE;
    } else {
        isp_dpc_lut_wstt_write(&be_reg->be_lut.be_lut2stt.pre_be_lut2stt,
                               usr_reg_cfg->usr_sta_cor_reg_cfg.dpcc_bp_table);
        isp_dpc_stt2lut_en_write(be_reg, TD_TRUE);
        isp_dpc_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_dpc_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_dpc_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_dpc_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_dpc_usr_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i,
    isp_pre_be_reg_type *be_reg, td_bool offline_mode)
{
    isp_dpc_usr_cfg *usr_reg_cfg = TD_NULL;
    td_bool usr_resh, idx_resh;
    td_s32 ret;
    td_bool stt2_lut_regnew = TD_FALSE;

    usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dp_reg_cfg.usr_reg_cfg;

    if (usr_reg_cfg->usr_dyna_cor_reg_cfg.resh) {
        isp_dpc_ex_soft_thr_max_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_max[0]);
        isp_dpc_ex_soft_thr_min_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_min[0]);
        isp_dpc_ex_hard_thr_en_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.hard_thr_en[0]);
        isp_dpc_ex_rake_ratio_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.rake_ratio[0]);

        isp_dpc1_ex_soft_thr_max_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_max[1]);
        isp_dpc1_ex_soft_thr_min_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.sup_twinkle_thr_min[1]);
        isp_dpc1_ex_hard_thr_en_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.hard_thr_en[1]);
        isp_dpc1_ex_rake_ratio_write(be_reg, usr_reg_cfg->usr_dyna_cor_reg_cfg.rake_ratio[1]);

        usr_reg_cfg->usr_dyna_cor_reg_cfg.resh = offline_mode;
    }

    idx_resh = (isp_dpc_update_index_read(be_reg) != usr_reg_cfg->usr_sta_cor_reg_cfg.update_index);
    usr_resh =
        (offline_mode) ? (usr_reg_cfg->usr_sta_cor_reg_cfg.resh && idx_resh) : (usr_reg_cfg->usr_sta_cor_reg_cfg.resh);

    if (usr_resh) {
        isp_dpc_update_index_write(be_reg, usr_reg_cfg->usr_sta_cor_reg_cfg.update_index);
        isp_dpc_bpt_number_write(be_reg, usr_reg_cfg->usr_sta_cor_reg_cfg.dpcc_bpt_number);

        ret = isp_dpc_bpt_lut_reg_config(&stt2_lut_regnew, vi_pipe, i, be_reg, usr_reg_cfg);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        usr_reg_cfg->usr_sta_cor_reg_cfg.resh = offline_mode;
    }

    isp_dpc_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.dpc_stt2lut_regnew = stt2_lut_regnew;

    return TD_SUCCESS;
}

static td_void isp_dpc_dyn_reg_config(isp_pre_be_reg_type *be_reg, isp_dpc_dyna_cfg *dyna_reg_cfg)
{
    isp_dpc_bpt_thresh_write(be_reg, dyna_reg_cfg->dpcc_bad_thresh);
    isp_dpc_bit_depth_sel_write(be_reg, dyna_reg_cfg->dpcc_bit_depth_sel);

    isp_dpc_blend_write(be_reg, dyna_reg_cfg->dpcc_alpha[0]);
    isp_dpc_mode_write(be_reg, dyna_reg_cfg->dpcc_mode[0]);
    isp_dpc_set_use_write(be_reg, dyna_reg_cfg->dpcc_set_use[0]);
    isp_dpc_methods_set_1_write(be_reg, dyna_reg_cfg->dpcc_methods_set1[0]);
    isp_dpc_line_thresh_1_write(be_reg, dyna_reg_cfg->dpcc_line_thr[0]);
    isp_dpc_line_mad_fac_1_write(be_reg, dyna_reg_cfg->dpcc_line_mad_fac[0]);
    isp_dpc_pg_fac_1_write(be_reg, dyna_reg_cfg->dpcc_pg_fac[0]);
    isp_dpc_rg_fac_1_write(be_reg, dyna_reg_cfg->dpcc_rg_fac[0]);
    isp_dpc_ro_limits_write(be_reg, dyna_reg_cfg->dpcc_ro_limits[0]);

    isp_dpc_line_kerdiff_fac_write(be_reg, dyna_reg_cfg->dpcc_line_kerdiff_fac[0]);
    isp_dpc_amp_coef_k_write(be_reg, dyna_reg_cfg->dpcc_amp_coef_k[0]);
    isp_dpc_amp_coef_min_write(be_reg, dyna_reg_cfg->dpcc_amp_coef_min[0]);

    isp_dpc_line_std_thr_1_write(be_reg, dyna_reg_cfg->dpcc_line_std_thr[0]);
    isp_dpc_line_diff_thr_1_write(be_reg, dyna_reg_cfg->dpcc_line_diff_thr[0]);
    isp_dpc_line_aver_fac_1_write(be_reg, dyna_reg_cfg->dpcc_line_aver_fac[0]);
    isp_dpc_rg_fac_1_mtp_write(be_reg, dyna_reg_cfg->dpcc_rg_fac_mtp[0]);

    isp_dpc1_blend_write(be_reg, dyna_reg_cfg->dpcc_alpha[1]);
    isp_dpc1_mode_write(be_reg, dyna_reg_cfg->dpcc_mode[1]);
    isp_dpc1_set_use_write(be_reg, dyna_reg_cfg->dpcc_set_use[1]);
    isp_dpc1_methods_set_1_write(be_reg, dyna_reg_cfg->dpcc_methods_set1[1]);
    isp_dpc1_line_thresh_1_write(be_reg, dyna_reg_cfg->dpcc_line_thr[1]);
    isp_dpc1_line_mad_fac_1_write(be_reg, dyna_reg_cfg->dpcc_line_mad_fac[1]);
    isp_dpc1_pg_fac_1_write(be_reg, dyna_reg_cfg->dpcc_pg_fac[1]);
    isp_dpc1_rg_fac_1_write(be_reg, dyna_reg_cfg->dpcc_rg_fac[1]);
    isp_dpc1_ro_limits_write(be_reg, dyna_reg_cfg->dpcc_ro_limits[1]);

    isp_dpc1_line_kerdiff_fac_write(be_reg, dyna_reg_cfg->dpcc_line_kerdiff_fac[1]);
    isp_dpc1_amp_coef_k_write(be_reg, dyna_reg_cfg->dpcc_amp_coef_k[1]);
    isp_dpc1_amp_coef_min_write(be_reg, dyna_reg_cfg->dpcc_amp_coef_min[1]);

    isp_dpc1_line_std_thr_1_write(be_reg, dyna_reg_cfg->dpcc_line_std_thr[1]);
    isp_dpc1_line_diff_thr_1_write(be_reg, dyna_reg_cfg->dpcc_line_diff_thr[1]);
    isp_dpc1_line_aver_fac_1_write(be_reg, dyna_reg_cfg->dpcc_line_aver_fac[1]);
    isp_dpc1_rg_fac_1_mtp_write(be_reg, dyna_reg_cfg->dpcc_rg_fac_mtp[1]);
}

static td_s32 isp_dpc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;

    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_dpc_static_cfg *static_reg_cfg = TD_NULL;
    isp_dpc_dyna_cfg *dyna_reg_cfg = TD_NULL;

    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_dp_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(pre_viproc);

        /* static */
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dp_reg_cfg.static_reg_cfg;

        if (static_reg_cfg->static_resh) {
            isp_dpc_bpt_ctrl_write(be_reg, static_reg_cfg->dpcc_bpt_ctrl);
            isp_dpc_lut_width_word_write(pre_viproc, OT_ISP_DPC_LUT_WIDTH_WORD_DEFAULT);
            static_reg_cfg->static_resh = TD_FALSE;
        }

        /* usr */
        ret = isp_dpc_usr_reg_config(vi_pipe, reg_cfg_info, i, be_reg, offline_mode);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        /* dynamic */
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dp_reg_cfg.dyna_reg_cfg;

        if (dyna_reg_cfg->resh) {
            isp_dpc_stat_en_write(pre_viproc, dyna_reg_cfg->dpc_stat_en);
            isp_dpc_dyn_reg_config(be_reg, dyna_reg_cfg);
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_dp_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_CR_SUPPORT
static td_void isp_ge_static_reg_config(isp_pre_be_reg_type *be_reg, isp_ge_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->static_resh) {
        isp_ge_bit_depth_sel_write(be_reg, OT_ISP_DPC_GE_DEFAULT_BIT_DEPTH_SEL);

        static_reg_cfg->static_resh = TD_FALSE;
    }
}

static td_void isp_ge_usr_reg_config(isp_pre_be_reg_type *be_reg, isp_ge_usr_cfg *usr_reg_cfg)
{
    isp_ge_ge0_ct_th2_write(be_reg, usr_reg_cfg->ge_ct_th2[0]);
    isp_ge_ge1_ct_th2_write(be_reg, usr_reg_cfg->ge_ct_th2[2]); /* ge_ct_th2[2] */

    isp_ge_ge0_ct_slope1_write(be_reg, usr_reg_cfg->ge_ct_slope1[0]);
    isp_ge_ge1_ct_slope1_write(be_reg, usr_reg_cfg->ge_ct_slope1[2]); /* ge_ct_slope1[2] */

    isp_ge_ge0_ct_slope2_write(be_reg, usr_reg_cfg->ge_ct_slope2[0]);
    isp_ge_ge1_ct_slope2_write(be_reg, usr_reg_cfg->ge_ct_slope2[2]); /* ge_ct_slope2[2] */
}

static td_void isp_ge_dyna_reg_config(isp_pre_be_reg_type *be_reg, isp_ge_dyna_cfg *dyna_reg_cfg)
{
    isp_ge_ge0_ct_th1_write(be_reg, dyna_reg_cfg->ge_ct_th1[0]);
    isp_ge_ge1_ct_th1_write(be_reg, dyna_reg_cfg->ge_ct_th1[2]); /* ge_ct_th1[2] */

    isp_ge_ge0_ct_th3_write(be_reg, dyna_reg_cfg->ge_ct_th3[0]);
    isp_ge_ge1_ct_th3_write(be_reg, dyna_reg_cfg->ge_ct_th3[2]); /* ge_ct_th3[2] */

    isp_ge_strength_write(be_reg, dyna_reg_cfg->ge_strength);
}
#endif
static td_s32 isp_ge_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
#ifdef CONFIG_OT_ISP_CR_SUPPORT
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_ge_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_ge_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_ge_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);

        isp_ge_static_reg_config(be_reg, &reg_cfg_info->alg_reg_cfg[i].ge_reg_cfg.static_reg_cfg);

        /* usr */
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ge_reg_cfg.usr_reg_cfg;
        if (usr_reg_cfg->resh) {
            isp_ge_usr_reg_config(be_reg, usr_reg_cfg);

            usr_reg_cfg->resh = offline_mode;
        }

        /* dynamic */
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ge_reg_cfg.dyna_reg_cfg;
        if (dyna_reg_cfg->resh) {
            isp_ge_dyna_reg_config(be_reg, dyna_reg_cfg);
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_ge_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
#endif

    return TD_SUCCESS;
}

static td_void isp_lsc_win_x_info_write(isp_be_reg_type *be_reg, isp_lsc_usr_cfg *usr_reg_cfg)
{
    isp_mlsc_x0_write(be_reg, usr_reg_cfg->delta_x[0], usr_reg_cfg->inv_x[0]); /* delta_x[0] */
    isp_mlsc_x1_write(be_reg, usr_reg_cfg->delta_x[1], usr_reg_cfg->inv_x[1]); /* delta_x[1] */
    isp_mlsc_x2_write(be_reg, usr_reg_cfg->delta_x[2], usr_reg_cfg->inv_x[2]); /* delta_x[2] */
    isp_mlsc_x3_write(be_reg, usr_reg_cfg->delta_x[3], usr_reg_cfg->inv_x[3]); /* delta_x[3] */
    isp_mlsc_x4_write(be_reg, usr_reg_cfg->delta_x[4], usr_reg_cfg->inv_x[4]); /* delta_x[4] */
    isp_mlsc_x5_write(be_reg, usr_reg_cfg->delta_x[5], usr_reg_cfg->inv_x[5]); /* delta_x[5] */
    isp_mlsc_x6_write(be_reg, usr_reg_cfg->delta_x[6], usr_reg_cfg->inv_x[6]); /* delta_x[6] */
    isp_mlsc_x7_write(be_reg, usr_reg_cfg->delta_x[7], usr_reg_cfg->inv_x[7]); /* delta_x[7] */
    isp_mlsc_x8_write(be_reg, usr_reg_cfg->delta_x[8], usr_reg_cfg->inv_x[8]); /* delta_x[8] */
    isp_mlsc_x9_write(be_reg, usr_reg_cfg->delta_x[9], usr_reg_cfg->inv_x[9]); /* delta_x[9] */
    isp_mlsc_x10_write(be_reg, usr_reg_cfg->delta_x[10], usr_reg_cfg->inv_x[10]); /* delta_x[10] */
    isp_mlsc_x11_write(be_reg, usr_reg_cfg->delta_x[11], usr_reg_cfg->inv_x[11]); /* delta_x[11] */
    isp_mlsc_x12_write(be_reg, usr_reg_cfg->delta_x[12], usr_reg_cfg->inv_x[12]); /* delta_x[12] */
    isp_mlsc_x13_write(be_reg, usr_reg_cfg->delta_x[13], usr_reg_cfg->inv_x[13]); /* delta_x[13] */
    isp_mlsc_x14_write(be_reg, usr_reg_cfg->delta_x[14], usr_reg_cfg->inv_x[14]); /* delta_x[14] */
    isp_mlsc_x15_write(be_reg, usr_reg_cfg->delta_x[15], usr_reg_cfg->inv_x[15]); /* delta_x[15] */
    isp_mlsc_x16_write(be_reg, usr_reg_cfg->delta_x[16], usr_reg_cfg->inv_x[16]); /* delta_x[16] */
    isp_mlsc_x17_write(be_reg, usr_reg_cfg->delta_x[17], usr_reg_cfg->inv_x[17]); /* delta_x[17] */
    isp_mlsc_x18_write(be_reg, usr_reg_cfg->delta_x[18], usr_reg_cfg->inv_x[18]); /* delta_x[18] */
    isp_mlsc_x19_write(be_reg, usr_reg_cfg->delta_x[19], usr_reg_cfg->inv_x[19]); /* delta_x[19] */
    isp_mlsc_x20_write(be_reg, usr_reg_cfg->delta_x[20], usr_reg_cfg->inv_x[20]); /* delta_x[20] */
    isp_mlsc_x21_write(be_reg, usr_reg_cfg->delta_x[21], usr_reg_cfg->inv_x[21]); /* delta_x[21] */
    isp_mlsc_x22_write(be_reg, usr_reg_cfg->delta_x[22], usr_reg_cfg->inv_x[22]); /* delta_x[22] */
    isp_mlsc_x23_write(be_reg, usr_reg_cfg->delta_x[23], usr_reg_cfg->inv_x[23]); /* delta_x[23] */
    isp_mlsc_x24_write(be_reg, usr_reg_cfg->delta_x[24], usr_reg_cfg->inv_x[24]); /* delta_x[24] */
    isp_mlsc_x25_write(be_reg, usr_reg_cfg->delta_x[25], usr_reg_cfg->inv_x[25]); /* delta_x[25] */
    isp_mlsc_x26_write(be_reg, usr_reg_cfg->delta_x[26], usr_reg_cfg->inv_x[26]); /* delta_x[26] */
    isp_mlsc_x27_write(be_reg, usr_reg_cfg->delta_x[27], usr_reg_cfg->inv_x[27]); /* delta_x[27] */
    isp_mlsc_x28_write(be_reg, usr_reg_cfg->delta_x[28], usr_reg_cfg->inv_x[28]); /* delta_x[28] */
    isp_mlsc_x29_write(be_reg, usr_reg_cfg->delta_x[29], usr_reg_cfg->inv_x[29]); /* delta_x[29] */
    isp_mlsc_x30_write(be_reg, usr_reg_cfg->delta_x[30], usr_reg_cfg->inv_x[30]); /* delta_x[30] */
    isp_mlsc_x31_write(be_reg, usr_reg_cfg->delta_x[31], usr_reg_cfg->inv_x[31]); /* delta_x[31] */
}

static td_void isp_lsc_win_y_info_write(isp_be_reg_type *be_reg, isp_lsc_usr_cfg *usr_reg_cfg)
{
    isp_mlsc_deltay0_write(be_reg, usr_reg_cfg->delta_y[0]); /* delta_y[0] */
    isp_mlsc_deltay1_write(be_reg, usr_reg_cfg->delta_y[1]); /* delta_y[1] */
    isp_mlsc_deltay2_write(be_reg, usr_reg_cfg->delta_y[2]); /* delta_y[2] */
    isp_mlsc_deltay3_write(be_reg, usr_reg_cfg->delta_y[3]); /* delta_y[3] */
    isp_mlsc_deltay4_write(be_reg, usr_reg_cfg->delta_y[4]); /* delta_y[4] */
    isp_mlsc_deltay5_write(be_reg, usr_reg_cfg->delta_y[5]); /* delta_y[5] */
    isp_mlsc_deltay6_write(be_reg, usr_reg_cfg->delta_y[6]); /* delta_y[6] */
    isp_mlsc_deltay7_write(be_reg, usr_reg_cfg->delta_y[7]); /* delta_y[7] */
    isp_mlsc_deltay8_write(be_reg, usr_reg_cfg->delta_y[8]); /* delta_y[8] */
    isp_mlsc_deltay9_write(be_reg, usr_reg_cfg->delta_y[9]); /* delta_y[9] */
    isp_mlsc_deltay10_write(be_reg, usr_reg_cfg->delta_y[10]); /* delta_y[10] */
    isp_mlsc_deltay11_write(be_reg, usr_reg_cfg->delta_y[11]); /* delta_y[11] */
    isp_mlsc_deltay12_write(be_reg, usr_reg_cfg->delta_y[12]); /* delta_y[12] */
    isp_mlsc_deltay13_write(be_reg, usr_reg_cfg->delta_y[13]); /* delta_y[13] */
    isp_mlsc_deltay14_write(be_reg, usr_reg_cfg->delta_y[14]); /* delta_y[14] */
    isp_mlsc_deltay15_write(be_reg, usr_reg_cfg->delta_y[15]); /* delta_y[15] */

    isp_mlsc_invy0_write(be_reg, usr_reg_cfg->inv_y[0]); /* inv_y[0] */
    isp_mlsc_invy1_write(be_reg, usr_reg_cfg->inv_y[1]); /* inv_y[1] */
    isp_mlsc_invy2_write(be_reg, usr_reg_cfg->inv_y[2]); /* inv_y[2] */
    isp_mlsc_invy3_write(be_reg, usr_reg_cfg->inv_y[3]); /* inv_y[3] */
    isp_mlsc_invy4_write(be_reg, usr_reg_cfg->inv_y[4]); /* inv_y[4] */
    isp_mlsc_invy5_write(be_reg, usr_reg_cfg->inv_y[5]); /* inv_y[5] */
    isp_mlsc_invy6_write(be_reg, usr_reg_cfg->inv_y[6]); /* inv_y[6] */
    isp_mlsc_invy7_write(be_reg, usr_reg_cfg->inv_y[7]); /* inv_y[7] */
    isp_mlsc_invy8_write(be_reg, usr_reg_cfg->inv_y[8]); /* inv_y[8] */
    isp_mlsc_invy9_write(be_reg, usr_reg_cfg->inv_y[9]); /* inv_y[9] */
    isp_mlsc_invy10_write(be_reg, usr_reg_cfg->inv_y[10]); /* inv_y[10] */
    isp_mlsc_invy11_write(be_reg, usr_reg_cfg->inv_y[11]); /* inv_y[11] */
    isp_mlsc_invy12_write(be_reg, usr_reg_cfg->inv_y[12]); /* inv_y[12] */
    isp_mlsc_invy13_write(be_reg, usr_reg_cfg->inv_y[13]); /* inv_y[13] */
    isp_mlsc_invy14_write(be_reg, usr_reg_cfg->inv_y[14]); /* inv_y[14] */
    isp_mlsc_invy15_write(be_reg, usr_reg_cfg->inv_y[15]); /* inv_y[15] */
}

static td_s32 isp_lsc_usr_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, const isp_reg_cfg *reg_cfg_info,
    td_u8 i, isp_lsc_usr_cfg *usr_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_be_reg_type *be_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);
    isp_check_pointer_return(be_reg);

    isp_mlsc_width_offset_write(be_reg, usr_reg_cfg->width_offset);

    isp_lsc_win_y_info_write(be_reg, usr_reg_cfg);
    isp_lsc_win_x_info_write(be_reg, usr_reg_cfg);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        if (reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.lut2_stt_en == TD_TRUE) { /* online lut2stt regconfig */
            buf_id = usr_reg_cfg->buf_id;

            be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
            isp_check_pointer_return(be_lut_stt_reg);

            isp_mlsc_lut_wstt_write(&be_lut_stt_reg->pre_be_lut2stt, usr_reg_cfg->r_gain, usr_reg_cfg->gr_gain,
                usr_reg_cfg->gb_gain, usr_reg_cfg->b_gain);
            ret = isp_lsc_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
            if (ret != TD_SUCCESS) {
                isp_err_trace("isp[%d]_lsc_lut_wstt_addr_write failed\n", vi_pipe);
                return ret;
            }

            isp_mlsc_stt2lut_en_write(be_reg, TD_TRUE);

            usr_reg_cfg->buf_id = 1 - buf_id;
            *stt2_lut_regnew = TD_TRUE;
        }
    } else {
        isp_mlsc_lut_wstt_write(&be_reg->be_lut.be_lut2stt.pre_be_lut2stt, usr_reg_cfg->r_gain, usr_reg_cfg->gr_gain,
            usr_reg_cfg->gb_gain, usr_reg_cfg->b_gain);
        isp_mlsc_stt2lut_en_write(be_reg, TD_TRUE);
        isp_mlsc_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_lsc_static_reg_config(isp_post_be_reg_type *be_reg, isp_viproc_reg_type *post_viproc,
    isp_lsc_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->static_resh) {
        isp_mlsc_numh_write(be_reg, static_reg_cfg->win_num_h);
        isp_mlsc_numv_write(be_reg, static_reg_cfg->win_num_v);
        isp_mlsc_lut_width_word_write(post_viproc, OT_ISP_LSC_LUT_WIDTH_WORD_DEFAULT);
        static_reg_cfg->static_resh = TD_FALSE;
    }
}

static td_void isp_lsc_usr_coef_reg_config(td_bool offline_mode, isp_post_be_reg_type *be_reg,
    isp_lsc_usr_cfg *usr_reg_cfg)
{
    if (usr_reg_cfg->coef_update) {
        isp_lsc_mesh_str_write(be_reg, usr_reg_cfg->mesh_str);
        isp_mlsc_mesh_str_write(be_reg, usr_reg_cfg->mesh_str);
        usr_reg_cfg->coef_update = offline_mode;
    }
}

static td_void isp_lsc_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_mlsc_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_mlsc_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_lsc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode, usr_resh, idx_resh;
    td_bool lut_update = TD_FALSE;
    td_bool stt2_lut_regnew = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_lsc_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_lsc_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(post_viproc);
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg;

        isp_lsc_static_reg_config(be_reg, post_viproc, &reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.static_reg_cfg);
        isp_lsc_usr_coef_reg_config(offline_mode, be_reg, usr_reg_cfg);

        idx_resh = (isp_lsc_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (usr_reg_cfg->lut_update && idx_resh) : (usr_reg_cfg->lut_update);

        if (usr_resh) {
            isp_lsc_update_index_write(be_reg, usr_reg_cfg->update_index);

            ret = isp_lsc_usr_reg_config(&stt2_lut_regnew, vi_pipe, reg_cfg_info, i, usr_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }

            lut_update = TD_TRUE;

            usr_reg_cfg->lut_update = offline_mode;
        }
        isp_lsc_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
        reg_cfg_info->cfg_key.bit1_lsc_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.lsc_lut_update = lut_update;
    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.lsc_stt2lut_regnew = stt2_lut_regnew;

    if (reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg.switch_lut2_stt_reg_new == TD_TRUE) {
        if (reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg.switch_reg_new_cnt < 3) { /* lsc reg config conunt 3 */
            reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.lsc_stt2lut_regnew = TD_TRUE;
            reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg.switch_reg_new_cnt++;
        } else {
            reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg.switch_lut2_stt_reg_new = TD_FALSE;
            reg_cfg_info->alg_reg_cfg[i].lsc_reg_cfg.usr_reg_cfg.switch_reg_new_cnt = 0;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_fe_lblc_delta_x_write(isp_fe_reg_type *fe_reg, td_u16 *delta_x)
{
    isp_lblc_deltax0_write(fe_reg, delta_x[0]); /* delta_x[0] */
    isp_lblc_deltax1_write(fe_reg, delta_x[1]); /* delta_x[1] */
    isp_lblc_deltax2_write(fe_reg, delta_x[2]); /* delta_x[2] */
    isp_lblc_deltax3_write(fe_reg, delta_x[3]); /* delta_x[3] */
    isp_lblc_deltax4_write(fe_reg, delta_x[4]); /* delta_x[4] */
    isp_lblc_deltax5_write(fe_reg, delta_x[5]); /* delta_x[5] */
    isp_lblc_deltax6_write(fe_reg, delta_x[6]); /* delta_x[6] */
    isp_lblc_deltax7_write(fe_reg, delta_x[7]); /* delta_x[7] */
    isp_lblc_deltax8_write(fe_reg, delta_x[8]); /* delta_x[8] */
    isp_lblc_deltax9_write(fe_reg, delta_x[9]); /* delta_x[9] */
    isp_lblc_deltax10_write(fe_reg, delta_x[10]); /* delta_x[10] */
    isp_lblc_deltax11_write(fe_reg, delta_x[11]); /* delta_x[11] */
    isp_lblc_deltax12_write(fe_reg, delta_x[12]); /* delta_x[12] */
    isp_lblc_deltax13_write(fe_reg, delta_x[13]); /* delta_x[13] */
    isp_lblc_deltax14_write(fe_reg, delta_x[14]); /* delta_x[14] */
    isp_lblc_deltax15_write(fe_reg, delta_x[15]); /* delta_x[15] */
    isp_lblc_deltax16_write(fe_reg, delta_x[16]); /* delta_x[16] */
    isp_lblc_deltax17_write(fe_reg, delta_x[17]); /* delta_x[17] */
    isp_lblc_deltax18_write(fe_reg, delta_x[18]); /* delta_x[18] */
    isp_lblc_deltax19_write(fe_reg, delta_x[19]); /* delta_x[19] */
    isp_lblc_deltax20_write(fe_reg, delta_x[20]); /* delta_x[20] */
    isp_lblc_deltax21_write(fe_reg, delta_x[21]); /* delta_x[21] */
    isp_lblc_deltax22_write(fe_reg, delta_x[22]); /* delta_x[22] */
    isp_lblc_deltax23_write(fe_reg, delta_x[23]); /* delta_x[23] */
    isp_lblc_deltax24_write(fe_reg, delta_x[24]); /* delta_x[24] */
    isp_lblc_deltax25_write(fe_reg, delta_x[25]); /* delta_x[25] */
    isp_lblc_deltax26_write(fe_reg, delta_x[26]); /* delta_x[26] */
    isp_lblc_deltax27_write(fe_reg, delta_x[27]); /* delta_x[27] */
    isp_lblc_deltax28_write(fe_reg, delta_x[28]); /* delta_x[28] */
    isp_lblc_deltax29_write(fe_reg, delta_x[29]); /* delta_x[29] */
    isp_lblc_deltax30_write(fe_reg, delta_x[30]); /* delta_x[30] */
    isp_lblc_deltax31_write(fe_reg, delta_x[31]); /* delta_x[31] */
}

static td_void isp_fe_lblc_inv_x_write(isp_fe_reg_type *fe_reg, td_u16 *inv_x)
{
    isp_lblc_invx0_write(fe_reg, inv_x[0]);  /* inv_x[0] */
    isp_lblc_invx1_write(fe_reg, inv_x[1]);  /* inv_x[1] */
    isp_lblc_invx2_write(fe_reg, inv_x[2]);  /* inv_x[2] */
    isp_lblc_invx3_write(fe_reg, inv_x[3]);  /* inv_x[3] */
    isp_lblc_invx4_write(fe_reg, inv_x[4]);  /* inv_x[4] */
    isp_lblc_invx5_write(fe_reg, inv_x[5]);  /* inv_x[5] */
    isp_lblc_invx6_write(fe_reg, inv_x[6]);  /* inv_x[6] */
    isp_lblc_invx7_write(fe_reg, inv_x[7]);  /* inv_x[7] */
    isp_lblc_invx8_write(fe_reg, inv_x[8]);  /* inv_x[8] */
    isp_lblc_invx9_write(fe_reg, inv_x[9]);  /* inv_x[9] */
    isp_lblc_invx10_write(fe_reg, inv_x[10]);  /* inv_x[10] */
    isp_lblc_invx11_write(fe_reg, inv_x[11]);  /* inv_x[11] */
    isp_lblc_invx12_write(fe_reg, inv_x[12]);  /* inv_x[12] */
    isp_lblc_invx13_write(fe_reg, inv_x[13]);  /* inv_x[13] */
    isp_lblc_invx14_write(fe_reg, inv_x[14]);  /* inv_x[14] */
    isp_lblc_invx15_write(fe_reg, inv_x[15]);  /* inv_x[15] */
    isp_lblc_invx16_write(fe_reg, inv_x[16]);  /* inv_x[16] */
    isp_lblc_invx17_write(fe_reg, inv_x[17]);  /* inv_x[17] */
    isp_lblc_invx18_write(fe_reg, inv_x[18]);  /* inv_x[18] */
    isp_lblc_invx19_write(fe_reg, inv_x[19]);  /* inv_x[19] */
    isp_lblc_invx20_write(fe_reg, inv_x[20]);  /* inv_x[20] */
    isp_lblc_invx21_write(fe_reg, inv_x[21]);  /* inv_x[21] */
    isp_lblc_invx22_write(fe_reg, inv_x[22]);  /* inv_x[22] */
    isp_lblc_invx23_write(fe_reg, inv_x[23]);  /* inv_x[23] */
    isp_lblc_invx24_write(fe_reg, inv_x[24]);  /* inv_x[24] */
    isp_lblc_invx25_write(fe_reg, inv_x[25]);  /* inv_x[25] */
    isp_lblc_invx26_write(fe_reg, inv_x[26]);  /* inv_x[26] */
    isp_lblc_invx27_write(fe_reg, inv_x[27]);  /* inv_x[27] */
    isp_lblc_invx28_write(fe_reg, inv_x[28]);  /* inv_x[28] */
    isp_lblc_invx29_write(fe_reg, inv_x[29]);  /* inv_x[29] */
    isp_lblc_invx30_write(fe_reg, inv_x[30]);  /* inv_x[30] */
    isp_lblc_invx31_write(fe_reg, inv_x[31]);  /* inv_x[31] */
}

static td_void isp_lblc_win_x_info_write(isp_fe_reg_type *fe_reg, isp_lblc_static_cfg *static_reg_cfg)
{
    isp_fe_lblc_delta_x_write(fe_reg, static_reg_cfg->delta_x);
    isp_fe_lblc_inv_x_write(fe_reg, static_reg_cfg->inv_x);
}

static td_void isp_lblc_win_y_info_write(isp_fe_reg_type *fe_reg, isp_lblc_static_cfg *static_reg_cfg)
{
    isp_lblc_deltay0_write(fe_reg, static_reg_cfg->delta_y[0]); /* delta_y[0] */
    isp_lblc_deltay1_write(fe_reg, static_reg_cfg->delta_y[1]); /* delta_y[1] */
    isp_lblc_deltay2_write(fe_reg, static_reg_cfg->delta_y[2]); /* delta_y[2] */
    isp_lblc_deltay3_write(fe_reg, static_reg_cfg->delta_y[3]); /* delta_y[3] */
    isp_lblc_deltay4_write(fe_reg, static_reg_cfg->delta_y[4]); /* delta_y[4] */
    isp_lblc_deltay5_write(fe_reg, static_reg_cfg->delta_y[5]); /* delta_y[5] */
    isp_lblc_deltay6_write(fe_reg, static_reg_cfg->delta_y[6]); /* delta_y[6] */
    isp_lblc_deltay7_write(fe_reg, static_reg_cfg->delta_y[7]); /* delta_y[7] */
    isp_lblc_deltay8_write(fe_reg, static_reg_cfg->delta_y[8]); /* delta_y[8] */
    isp_lblc_deltay9_write(fe_reg, static_reg_cfg->delta_y[9]); /* delta_y[9] */
    isp_lblc_deltay10_write(fe_reg, static_reg_cfg->delta_y[10]); /* delta_y[10] */
    isp_lblc_deltay11_write(fe_reg, static_reg_cfg->delta_y[11]); /* delta_y[11] */
    isp_lblc_deltay12_write(fe_reg, static_reg_cfg->delta_y[12]); /* delta_y[12] */
    isp_lblc_deltay13_write(fe_reg, static_reg_cfg->delta_y[13]); /* delta_y[13] */
    isp_lblc_deltay14_write(fe_reg, static_reg_cfg->delta_y[14]); /* delta_y[14] */
    isp_lblc_deltay15_write(fe_reg, static_reg_cfg->delta_y[15]); /* delta_y[15] */

    isp_lblc_invy0_write(fe_reg, static_reg_cfg->inv_y[0]); /* inv_y[0] */
    isp_lblc_invy1_write(fe_reg, static_reg_cfg->inv_y[1]); /* inv_y[1] */
    isp_lblc_invy2_write(fe_reg, static_reg_cfg->inv_y[2]); /* inv_y[2] */
    isp_lblc_invy3_write(fe_reg, static_reg_cfg->inv_y[3]); /* inv_y[3] */
    isp_lblc_invy4_write(fe_reg, static_reg_cfg->inv_y[4]); /* inv_y[4] */
    isp_lblc_invy5_write(fe_reg, static_reg_cfg->inv_y[5]); /* inv_y[5] */
    isp_lblc_invy6_write(fe_reg, static_reg_cfg->inv_y[6]); /* inv_y[6] */
    isp_lblc_invy7_write(fe_reg, static_reg_cfg->inv_y[7]); /* inv_y[7] */
    isp_lblc_invy8_write(fe_reg, static_reg_cfg->inv_y[8]); /* inv_y[8] */
    isp_lblc_invy9_write(fe_reg, static_reg_cfg->inv_y[9]); /* inv_y[9] */
    isp_lblc_invy10_write(fe_reg, static_reg_cfg->inv_y[10]); /* inv_y[10] */
    isp_lblc_invy11_write(fe_reg, static_reg_cfg->inv_y[11]); /* inv_y[11] */
    isp_lblc_invy12_write(fe_reg, static_reg_cfg->inv_y[12]); /* inv_y[12] */
    isp_lblc_invy13_write(fe_reg, static_reg_cfg->inv_y[13]); /* inv_y[13] */
    isp_lblc_invy14_write(fe_reg, static_reg_cfg->inv_y[14]); /* inv_y[14] */
    isp_lblc_invy15_write(fe_reg, static_reg_cfg->inv_y[15]); /* inv_y[15] */
}

static td_void isp_lblc_static_reg_config(isp_fe_reg_type *fe_reg, isp_lblc_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->static_resh) {
        isp_lblc_num_col_write(fe_reg, static_reg_cfg->lblc_num_col);
        isp_lblc_num_row_write(fe_reg, static_reg_cfg->lblc_num_row);
        isp_lblc_win_y_info_write(fe_reg, static_reg_cfg);
        isp_lblc_win_x_info_write(fe_reg, static_reg_cfg);

        isp_lblc_dither_en_write(fe_reg, static_reg_cfg->lblc_dither_en);
        isp_lblc_bitwidth_in_write(fe_reg, static_reg_cfg->bitwidth_in);
        isp_lblc_width_offset_write(fe_reg, static_reg_cfg->width_offset);
        isp_lblc_lut_shift_bit_write(fe_reg, static_reg_cfg->lut_shift_bit);
    }
}

static td_void isp_lblc_save_sync_reg(isp_lblc_usr_cfg *usr_reg_cfg)
{
    usr_reg_cfg->lut2stt_sync_cfg.offset_update = TD_TRUE;
    usr_reg_cfg->lut2stt_sync_cfg.offset_r = usr_reg_cfg->offset_r;
    usr_reg_cfg->lut2stt_sync_cfg.offset_gr = usr_reg_cfg->offset_gr;
    usr_reg_cfg->lut2stt_sync_cfg.offset_gb = usr_reg_cfg->offset_gb;
    usr_reg_cfg->lut2stt_sync_cfg.offset_b = usr_reg_cfg->offset_b;
}

static td_s32 isp_lblc_usr_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, ot_vi_pipe mast_pipe,
    const isp_reg_cfg *reg_cfg_info, isp_lblc_usr_cfg *usr_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_fe_lut_wstt_type *fe_lut_stt_reg = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_vicap_ch_reg_type *vicap_ch_reg = TD_NULL;

    fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe);
    vicap_ch_reg = (isp_vicap_ch_reg_type *)isp_get_vicap_ch_virt_addr(vi_pipe);
    isp_check_pointer_return(fe_reg);
    isp_check_pointer_return(vicap_ch_reg);
    isp_lblc_save_sync_reg(usr_reg_cfg);
    if (reg_cfg_info->alg_reg_cfg[0].lblc_reg_cfg.lut2_stt_en == TD_TRUE) { /* online lut2stt regconfig */
        buf_id = usr_reg_cfg->buf_id;

        fe_lut_stt_reg = (isp_fe_lut_wstt_type *)isp_get_fe_lut2stt_vir_addr(mast_pipe, buf_id);
        isp_check_pointer_return(fe_lut_stt_reg);

        isp_lblc_lut_wstt_write(fe_lut_stt_reg, usr_reg_cfg->mesh_blc_r, usr_reg_cfg->mesh_blc_gr,
            usr_reg_cfg->mesh_blc_gb, usr_reg_cfg->mesh_blc_b);
        ret = isp_lblc_lut_wstt_addr_write(mast_pipe, buf_id, vicap_ch_reg);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_lblc_lut_wstt_addr_write failed, mast_pipe:%d\n", vi_pipe, mast_pipe);
            return ret;
        }

        isp_lblc_stt2lut_en_write(fe_reg, TD_TRUE);

        usr_reg_cfg->buf_id = 1 - buf_id;
        *stt2_lut_regnew = TD_TRUE;
    }

    return TD_SUCCESS;
}

static td_void isp_lblc_usr_coef_reg_config(isp_fe_reg_type *fe_reg,
    isp_lblc_usr_cfg *usr_reg_cfg)
{
    if (usr_reg_cfg->coef_update) {
        isp_lblc_strength_write(fe_reg, usr_reg_cfg->strength);
    }
}

static td_void isp_lblc_lut2stt_info_check(td_bool *stt2_lut_regnew, isp_fe_reg_type *fe_reg)
{
    td_u16 lut2stt_info;

    lut2stt_info = isp_lblc_stt2lut_info_read(fe_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_lblc_stt2lut_clr_write(fe_reg, 1);
    }
}
static td_void isp_lblc_config_last_sync_reg(isp_fe_reg_type *fe_reg, isp_lblc_usr_cfg *usr_reg_cfg)
{
    if (usr_reg_cfg->lut2stt_sync_cfg.offset_update == TD_TRUE) {
        isp_lblc_offset_r_write(fe_reg, usr_reg_cfg->lut2stt_sync_cfg.offset_r);
        isp_lblc_offset_gr_write(fe_reg, usr_reg_cfg->lut2stt_sync_cfg.offset_gr);
        isp_lblc_offset_gb_write(fe_reg, usr_reg_cfg->lut2stt_sync_cfg.offset_gb);
        isp_lblc_offset_b_write(fe_reg, usr_reg_cfg->lut2stt_sync_cfg.offset_b);
        usr_reg_cfg->lut2stt_sync_cfg.offset_update = TD_FALSE;
    }
}

static td_s32 isp_lblc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u8 local_blc_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_LOCAL_BLC_ENABLE };
    td_u16 i;
    td_bool stt2_lut_regnew = TD_FALSE;
    td_s32 ret;
    ot_vi_pipe vi_pipe_bind, mast_pipe;
    isp_lblc_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return TD_SUCCESS;
    }
    mast_pipe = vi_pipe;
    if (reg_cfg_info->cfg_key.bit1_lblc_cfg != TD_TRUE) {
        return TD_SUCCESS;
    }
    usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[0].lblc_reg_cfg.usr_reg_cfg;
    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        if (local_blc_en[vi_pipe_bind] != 1) {
            continue;
        }
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        isp_fe_local_blc_en_write(fe_reg, reg_cfg_info->alg_reg_cfg[0].lblc_reg_cfg.lblc_en);
        isp_lblc_static_reg_config(fe_reg, &reg_cfg_info->alg_reg_cfg[0].lblc_reg_cfg.static_reg_cfg);
        isp_lblc_usr_coef_reg_config(fe_reg, usr_reg_cfg);
        isp_lblc_config_last_sync_reg(fe_reg, usr_reg_cfg);
        if (usr_reg_cfg->lut_update) {
            ret = isp_lblc_usr_reg_config(&stt2_lut_regnew, vi_pipe_bind, mast_pipe, reg_cfg_info, usr_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }
        }
        isp_lblc_lut2stt_info_check(&stt2_lut_regnew, fe_reg);
    }

    reg_cfg_info->cfg_key.bit1_lblc_cfg = TD_FALSE;
    usr_reg_cfg->lut_update = TD_FALSE;
    usr_reg_cfg->coef_update  = TD_FALSE;
    reg_cfg_info->alg_reg_cfg[0].lblc_reg_cfg.static_reg_cfg.static_resh = TD_FALSE;

    reg_cfg_info->alg_reg_cfg[0].fe_lut_update_cfg.lblc_lut_update = stt2_lut_regnew;
    return TD_SUCCESS;
}

static td_s32 isp_rc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_radial_crop_reg_cfg *rc_reg_cfg = TD_NULL;

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);

    rc_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].rc_reg_cfg;
    isp_radial_crop_en_write(post_viproc, rc_reg_cfg->rc_en);
    isp_ratio_center_hor_write(post_viproc, rc_reg_cfg->usr_reg_cfg.center_hor_coor);
    isp_ratio_center_ver_write(post_viproc, rc_reg_cfg->usr_reg_cfg.center_ver_coor);
    isp_ratio_square_of_radius_write(post_viproc, rc_reg_cfg->usr_reg_cfg.square_radius);

    return TD_SUCCESS;
}

static td_s32 isp_gamma_lut_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, td_u8 i,
    isp_post_be_reg_type *be_reg, isp_gamma_usr_cfg *usr_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);

    isp_gamma_lut_width_word_write(post_viproc, OT_ISP_GAMMA_LUT_WIDTH_WORD_DEFAULT);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        if (usr_reg_cfg->switch_mode != TD_TRUE) {
            buf_id = usr_reg_cfg->buf_id;

            be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
            isp_check_pointer_return(be_lut_stt_reg);

            isp_gamma_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->gamma_lut);
            ret = isp_gamma_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
            if (ret != TD_SUCCESS) {
                isp_err_trace("isp[%d]_gamma_lut_wstt_addr_write failed\n", vi_pipe);
                return ret;
            }
            isp_gamma_stt2lut_en_write(be_reg, TD_TRUE);
            usr_reg_cfg->buf_id = 1 - buf_id;
            *stt2_lut_regnew = TD_TRUE;
        }
    } else {
        isp_gamma_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->gamma_lut);
        isp_gamma_stt2lut_en_write(be_reg, TD_TRUE);
        isp_gamma_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_gamma_inseg_write(isp_post_be_reg_type *be_reg, td_u16 *gamma_in_seg)
{
    isp_gamma_inseg_0_write(be_reg, gamma_in_seg[0]); /* index[0] */
    isp_gamma_inseg_1_write(be_reg, gamma_in_seg[1]); /* index[1] */
    isp_gamma_inseg_2_write(be_reg, gamma_in_seg[2]); /* index[2] */
    isp_gamma_inseg_3_write(be_reg, gamma_in_seg[3]); /* index[3] */
    isp_gamma_inseg_4_write(be_reg, gamma_in_seg[4]); /* index[4] */
    isp_gamma_inseg_5_write(be_reg, gamma_in_seg[5]); /* index[5] */
    isp_gamma_inseg_6_write(be_reg, gamma_in_seg[6]); /* index[6] */
    isp_gamma_inseg_7_write(be_reg, gamma_in_seg[7]); /* index[7] */
}

static td_void isp_gamma_pos_write(isp_post_be_reg_type *be_reg, td_u16 *gamma_pos)
{
    isp_gamma_pos_0_write(be_reg, gamma_pos[0]); /* index[0] */
    isp_gamma_pos_1_write(be_reg, gamma_pos[1]); /* index[1] */
    isp_gamma_pos_2_write(be_reg, gamma_pos[2]); /* index[2] */
    isp_gamma_pos_3_write(be_reg, gamma_pos[3]); /* index[3] */
    isp_gamma_pos_4_write(be_reg, gamma_pos[4]); /* index[4] */
    isp_gamma_pos_5_write(be_reg, gamma_pos[5]); /* index[5] */
    isp_gamma_pos_6_write(be_reg, gamma_pos[6]); /* index[6] */
    isp_gamma_pos_7_write(be_reg, gamma_pos[7]); /* index[7] */
}

static td_void isp_gamma_step_write(isp_post_be_reg_type *be_reg, td_u8 *gamma_step)
{
    isp_gamma_step0_write(be_reg, gamma_step[0]); /* index[0] */
    isp_gamma_step1_write(be_reg, gamma_step[1]); /* index[1] */
    isp_gamma_step2_write(be_reg, gamma_step[2]); /* index[2] */
    isp_gamma_step3_write(be_reg, gamma_step[3]); /* index[3] */
    isp_gamma_step4_write(be_reg, gamma_step[4]); /* index[4] */
    isp_gamma_step5_write(be_reg, gamma_step[5]); /* index[5] */
    isp_gamma_step6_write(be_reg, gamma_step[6]); /* index[6] */
    isp_gamma_step7_write(be_reg, gamma_step[7]); /* index[7] */
}

static td_void isp_gamma_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_gamma_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_gamma_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_gamma_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode, usr_resh, idx_resh;
    td_bool stt2_lut_regnew = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_gamma_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].gamma_cfg.usr_reg_cfg;

    if (reg_cfg_info->cfg_key.bit1_gamma_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);

        idx_resh = (isp_gamma_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (usr_reg_cfg->gamma_lut_update_en && idx_resh) : (usr_reg_cfg->gamma_lut_update_en);

        if (usr_resh) {
            isp_gamma_update_index_write(be_reg, usr_reg_cfg->update_index);

            ret = isp_gamma_lut_reg_config(&stt2_lut_regnew, vi_pipe, i, be_reg, usr_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }

            isp_gamma_inseg_write(be_reg, usr_reg_cfg->gamma_in_seg);
            isp_gamma_pos_write(be_reg, usr_reg_cfg->gamma_pos);
            isp_gamma_step_write(be_reg, usr_reg_cfg->gamma_step);

            usr_reg_cfg->gamma_lut_update_en = offline_mode;
            usr_reg_cfg->switch_mode = TD_FALSE;
        }
        isp_gamma_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
        reg_cfg_info->cfg_key.bit1_gamma_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.gamma_stt2lut_regnew = stt2_lut_regnew;

    return TD_SUCCESS;
}

static td_void isp_csp_lut_apb_config(isp_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    td_s32 i;

    isp_cc_dgain_lut_waddr_write(be_reg, 0);
    isp_cc_gamma_lut_waddr_write(be_reg, 0);

    for (i = 0; i < DRC_CSP_LUT_NODE_NUM; i++) {
        isp_cc_dgain_lut_wdata_write(be_reg, usr_reg_cfg->csp_lutb_lut[i] & 0x3FFF);
        isp_cc_gamma_lut_wdata_write(be_reg, usr_reg_cfg->csp_lutc_lut[i] & 0xFFF);
    }
}

static td_s32 isp_csp_lut_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, td_u8 i,
    isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    td_u8 buf_id;
    td_s32 ret;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);

    isp_cc_lut_width_word_write(post_viproc, OT_ISP_CC_LUT_WIDTH_WORD_DEFAULT);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        buf_id = usr_reg_cfg->buf_id;

        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_return(be_lut_stt_reg);

        isp_cc_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->csp_lutb_lut,
                              usr_reg_cfg->csp_lutc_lut);
        ret = isp_cc_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_csp_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }
        isp_cc_stt2lut_en_write(be_reg, TD_TRUE);
        usr_reg_cfg->buf_id = 1 - buf_id;
        *stt2_lut_regnew = TD_TRUE;
    } else {
        isp_cc_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->csp_lutb_lut,
                              usr_reg_cfg->csp_lutc_lut);
        isp_cc_stt2lut_en_write(be_reg, TD_TRUE);
        isp_cc_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_csp_lutb_inseg_write(isp_post_be_reg_type *be_reg, td_u16 *csp_in_seg)
{
    isp_cc_dgain_inseg_0_write(be_reg, csp_in_seg[0]); /* index[0] */
    isp_cc_dgain_inseg_1_write(be_reg, csp_in_seg[1]); /* index[1] */
    isp_cc_dgain_inseg_2_write(be_reg, csp_in_seg[2]); /* index[2] */
    isp_cc_dgain_inseg_3_write(be_reg, csp_in_seg[3]); /* index[3] */
    isp_cc_dgain_inseg_4_write(be_reg, csp_in_seg[4]); /* index[4] */
    isp_cc_dgain_inseg_5_write(be_reg, csp_in_seg[5]); /* index[5] */
    isp_cc_dgain_inseg_6_write(be_reg, csp_in_seg[6]); /* index[6] */
    isp_cc_dgain_inseg_7_write(be_reg, csp_in_seg[7]); /* index[7] */
}

static td_void isp_csp_lutb_pos_write(isp_post_be_reg_type *be_reg, td_u16 *csp_pos)
{
    isp_cc_dgain_pos_0_write(be_reg, csp_pos[0]); /* index[0] */
    isp_cc_dgain_pos_1_write(be_reg, csp_pos[1]); /* index[1] */
    isp_cc_dgain_pos_2_write(be_reg, csp_pos[2]); /* index[2] */
    isp_cc_dgain_pos_3_write(be_reg, csp_pos[3]); /* index[3] */
    isp_cc_dgain_pos_4_write(be_reg, csp_pos[4]); /* index[4] */
    isp_cc_dgain_pos_5_write(be_reg, csp_pos[5]); /* index[5] */
    isp_cc_dgain_pos_6_write(be_reg, csp_pos[6]); /* index[6] */
    isp_cc_dgain_pos_7_write(be_reg, csp_pos[7]); /* index[7] */
}

static td_void isp_csp_lutb_step_write(isp_post_be_reg_type *be_reg, td_u8 *csp_step)
{
    isp_cc_dgain_step0_write(be_reg, csp_step[0]); /* index[0] */
    isp_cc_dgain_step1_write(be_reg, csp_step[1]); /* index[1] */
    isp_cc_dgain_step2_write(be_reg, csp_step[2]); /* index[2] */
    isp_cc_dgain_step3_write(be_reg, csp_step[3]); /* index[3] */
    isp_cc_dgain_step4_write(be_reg, csp_step[4]); /* index[4] */
    isp_cc_dgain_step5_write(be_reg, csp_step[5]); /* index[5] */
    isp_cc_dgain_step6_write(be_reg, csp_step[6]); /* index[6] */
    isp_cc_dgain_step7_write(be_reg, csp_step[7]); /* index[7] */
}

static td_void isp_csp_lutc_inseg_write(isp_post_be_reg_type *be_reg, td_u16 *csp_in_seg)
{
    isp_cc_gamma_inseg_0_write(be_reg, csp_in_seg[0]); /* index[0] */
    isp_cc_gamma_inseg_1_write(be_reg, csp_in_seg[1]); /* index[1] */
    isp_cc_gamma_inseg_2_write(be_reg, csp_in_seg[2]); /* index[2] */
    isp_cc_gamma_inseg_3_write(be_reg, csp_in_seg[3]); /* index[3] */
    isp_cc_gamma_inseg_4_write(be_reg, csp_in_seg[4]); /* index[4] */
    isp_cc_gamma_inseg_5_write(be_reg, csp_in_seg[5]); /* index[5] */
    isp_cc_gamma_inseg_6_write(be_reg, csp_in_seg[6]); /* index[6] */
    isp_cc_gamma_inseg_7_write(be_reg, csp_in_seg[7]); /* index[7] */
}

static td_void isp_csp_lutc_pos_write(isp_post_be_reg_type *be_reg, td_u16 *csp_pos)
{
    isp_cc_gamma_pos_0_write(be_reg, csp_pos[0]); /* index[0] */
    isp_cc_gamma_pos_1_write(be_reg, csp_pos[1]); /* index[1] */
    isp_cc_gamma_pos_2_write(be_reg, csp_pos[2]); /* index[2] */
    isp_cc_gamma_pos_3_write(be_reg, csp_pos[3]); /* index[3] */
    isp_cc_gamma_pos_4_write(be_reg, csp_pos[4]); /* index[4] */
    isp_cc_gamma_pos_5_write(be_reg, csp_pos[5]); /* index[5] */
    isp_cc_gamma_pos_6_write(be_reg, csp_pos[6]); /* index[6] */
    isp_cc_gamma_pos_7_write(be_reg, csp_pos[7]); /* index[7] */
}

static td_void isp_csp_lutc_step_write(isp_post_be_reg_type *be_reg, td_u8 *csp_step)
{
    isp_cc_gamma_step0_write(be_reg, csp_step[0]); /* index[0] */
    isp_cc_gamma_step1_write(be_reg, csp_step[1]); /* index[1] */
    isp_cc_gamma_step2_write(be_reg, csp_step[2]); /* index[2] */
    isp_cc_gamma_step3_write(be_reg, csp_step[3]); /* index[3] */
    isp_cc_gamma_step4_write(be_reg, csp_step[4]); /* index[4] */
    isp_cc_gamma_step5_write(be_reg, csp_step[5]); /* index[5] */
    isp_cc_gamma_step6_write(be_reg, csp_step[6]); /* index[6] */
    isp_cc_gamma_step7_write(be_reg, csp_step[7]); /* index[7] */
}

static td_void isp_csp_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_cc_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_cc_stt2lut_clr_write(be_reg, 1);
    }
}
static td_s32 isp_csp_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_bool usr_resh;
    td_bool idx_resh;
    td_bool stt2_lut_regnew = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_drc_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.usr_reg_cfg;
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);

    if (reg_cfg_info->cfg_key.bit1_csp_cfg) {
        idx_resh = (isp_csp_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (usr_reg_cfg->csp_lut_update_en && idx_resh) : (usr_reg_cfg->csp_lut_update_en);

        if (usr_resh) {
            isp_csp_update_index_write(be_reg, usr_reg_cfg->update_index);

            ret = isp_csp_lut_reg_config(&stt2_lut_regnew, vi_pipe, i, be_reg, usr_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }
            isp_cc_luma_wdr_coefr_write(be_reg, usr_reg_cfg->csp_coefr);
            isp_cc_luma_wdr_coefb_write(be_reg, usr_reg_cfg->csp_coefb);
            isp_csp_lutb_inseg_write(be_reg, usr_reg_cfg->csp_lutb_in_seg);
            isp_csp_lutb_pos_write(be_reg, usr_reg_cfg->csp_lutb_pos);
            isp_csp_lutb_step_write(be_reg, usr_reg_cfg->csp_lutb_step);
            isp_csp_lutc_inseg_write(be_reg, usr_reg_cfg->csp_lutc_in_seg);
            isp_csp_lutc_pos_write(be_reg, usr_reg_cfg->csp_lutc_pos);
            isp_csp_lutc_step_write(be_reg, usr_reg_cfg->csp_lutc_step);
            usr_reg_cfg->csp_lut_update_en = offline_mode;
            usr_reg_cfg->switch_mode = TD_FALSE;
        }

        reg_cfg_info->cfg_key.bit1_csp_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
    isp_csp_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.csp_stt2lut_regnew = stt2_lut_regnew;

    return TD_SUCCESS;
}

static td_s32 isp_csc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_csc_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_csc_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].csc_cfg.dyna_reg_cfg;

        /* dynamic */
        if (dyna_reg_cfg->resh) {
            isp_csc_coef00_write(be_reg, dyna_reg_cfg->csc_coef[0]);  /* coef0 */
            isp_csc_coef01_write(be_reg, dyna_reg_cfg->csc_coef[1]);  /* coef1 */
            isp_csc_coef02_write(be_reg, dyna_reg_cfg->csc_coef[2]);  /* coef2 */
            isp_csc_coef10_write(be_reg, dyna_reg_cfg->csc_coef[3]);  /* coef3 */
            isp_csc_coef11_write(be_reg, dyna_reg_cfg->csc_coef[4]);  /* coef4 */
            isp_csc_coef12_write(be_reg, dyna_reg_cfg->csc_coef[5]);  /* coef5 */
            isp_csc_coef20_write(be_reg, dyna_reg_cfg->csc_coef[6]);  /* coef6 */
            isp_csc_coef21_write(be_reg, dyna_reg_cfg->csc_coef[7]);  /* coef7 */
            isp_csc_coef22_write(be_reg, dyna_reg_cfg->csc_coef[8]);  /* coef8 */
            isp_csc_in_dc0_write(be_reg, dyna_reg_cfg->csc_in_dc[0]); /* in_dc0 */
            isp_csc_in_dc1_write(be_reg, dyna_reg_cfg->csc_in_dc[1]); /* in_dc1 */
            isp_csc_in_dc2_write(be_reg, dyna_reg_cfg->csc_in_dc[2]); /* in_dc2 */

            isp_csc_out_dc0_write(be_reg, dyna_reg_cfg->csc_out_dc[0]); /* out_dc0 */
            isp_csc_out_dc1_write(be_reg, dyna_reg_cfg->csc_out_dc[1]); /* out_dc1 */
            isp_csc_out_dc2_write(be_reg, dyna_reg_cfg->csc_out_dc[2]); /* out_dc2 */
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_csc_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

#ifdef CONFIG_OT_ISP_CA_SUPPORT
static td_s32 isp_ca_lut_reg_config(td_bool *stt2_lut_regnew, ot_vi_pipe vi_pipe, td_u8 i,
    isp_post_be_reg_type *be_reg, isp_ca_usr_cfg *usr_reg_cfg)
{
    td_u8  buf_id;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_viproc);
    isp_ca_lut_width_word_write(post_viproc, OT_ISP_CA_LUT_WIDTH_WORD_DEFAULT);
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        buf_id = usr_reg_cfg->buf_id;
        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_return(be_lut_stt_reg);

        isp_ca_y_ratio_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_ratio_lut);
        isp_ca_y_sat_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_sat_lut);
        isp_ca_y_ch1_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_ch1_lut);
        isp_ca_y_ch2_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_ch2_lut);
        isp_ca_y_slu_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_slu_lut);
        isp_ca_y_slv_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, usr_reg_cfg->y_slv_lut);

        ret = isp_ca_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_ca_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }
        isp_ca_stt2lut_en_write(be_reg, TD_TRUE);

        usr_reg_cfg->buf_id = 1 - buf_id;
        *stt2_lut_regnew = TD_TRUE;
    } else {
        isp_ca_y_ratio_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_ratio_lut);
        isp_ca_y_sat_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_sat_lut);
        isp_ca_y_ch1_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_ch1_lut);
        isp_ca_y_ch2_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_ch2_lut);
        isp_ca_y_slu_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_slu_lut);
        isp_ca_y_slv_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, usr_reg_cfg->y_slv_lut);
        isp_ca_stt2lut_en_write(be_reg, TD_TRUE);
        isp_ca_stt2lut_regnew_write(be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_void isp_ca_static_reg_config(isp_post_be_reg_type *be_reg, isp_ca_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->static_resh) {
        isp_ca_llhcproc_en_write(be_reg, static_reg_cfg->ca_llhc_proc_en);
        isp_ca_skinproc_en_write(be_reg, static_reg_cfg->ca_skin_proc_en);
        isp_ca_satvssat_en_write(be_reg, static_reg_cfg->ca_satvssat_en);
        isp_ca_satadj_en_write(be_reg, static_reg_cfg->ca_satu_adj_en);
        isp_ca_des_point_write(be_reg, static_reg_cfg->ca_des_point);
        isp_ca_des_mix_write(be_reg, static_reg_cfg->ca_des_mix);
        isp_ca_sat_c2_write(be_reg, static_reg_cfg->ca_sat_c2);
        isp_ca_sat_c1_write(be_reg, static_reg_cfg->ca_sat_c1);
        static_reg_cfg->static_resh = TD_FALSE;
    }
}
static td_void isp_ca_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_ca_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_ca_stt2lut_clr_write(be_reg, 1);
    }
}
#endif

static td_s32 isp_ca_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
#ifdef CONFIG_OT_ISP_CA_SUPPORT
    td_bool offline_mode, usr_resh, idx_resh;
    td_bool  stt2_lut_regnew = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_ca_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);
    if (reg_cfg_info->cfg_key.bit1_ca_cfg) {
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(post_viproc);

        /* usr */
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ca_reg_cfg.usr_reg_cfg;
        idx_resh = (isp_ca_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (usr_reg_cfg->resh && idx_resh) : (usr_reg_cfg->resh);

        if (usr_resh) {
            isp_ca_update_index_write(be_reg, usr_reg_cfg->update_index);
            ret = isp_ca_lut_reg_config(&stt2_lut_regnew, vi_pipe, i, be_reg, usr_reg_cfg);
            if (ret != TD_SUCCESS) {
                return ret;
            }
            usr_reg_cfg->resh = offline_mode;
        }

        /* static */
        isp_ca_static_reg_config(be_reg, &reg_cfg_info->alg_reg_cfg[i].ca_reg_cfg.static_reg_cfg);

        reg_cfg_info->cfg_key.bit1_ca_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    isp_ca_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);

    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.ca_stt2lut_regnew = stt2_lut_regnew;
#endif

    return TD_SUCCESS;
}

static td_s32 isp_mcds_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_mcds_static_reg_cfg *static_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_mcds_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(post_viproc);

        isp_hcds_en_write(post_viproc, reg_cfg_info->alg_reg_cfg[i].mcds_reg_cfg.hcds_en);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].mcds_reg_cfg.static_reg_cfg;

        if (static_reg_cfg->static_resh) {
            isp_hcds_coefh0_write(be_reg, static_reg_cfg->h_coef[0]); /* coefh0 */
            isp_hcds_coefh1_write(be_reg, static_reg_cfg->h_coef[1]); /* coefh1 */
            isp_hcds_coefh2_write(be_reg, static_reg_cfg->h_coef[2]); /* coefh2 */
            isp_hcds_coefh3_write(be_reg, static_reg_cfg->h_coef[3]); /* coefh3 */
            isp_hcds_coefh4_write(be_reg, static_reg_cfg->h_coef[4]); /* coefh4 */
            isp_hcds_coefh5_write(be_reg, static_reg_cfg->h_coef[5]); /* coefh5 */
            isp_hcds_coefh6_write(be_reg, static_reg_cfg->h_coef[6]); /* coefh6 */
            isp_hcds_coefh7_write(be_reg, static_reg_cfg->h_coef[7]); /* coefh7 */
            isp_sharpen_vcds_filterv_write(be_reg, TD_TRUE);
            static_reg_cfg->static_resh = TD_FALSE;
        }

        reg_cfg_info->cfg_key.bit1_mcds_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_wdr_lut_def_reg_config(ot_vi_pipe vi_pipe, isp_pre_be_reg_type *pre_be_reg,
    isp_fswdr_dyna_cfg *dyna_reg_cfg)
{
    td_u16 j;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        isp_wdr_snoise_lut_waddr_write(pre_be_reg, 0x0);
        isp_wdr_lnoise_lut_waddr_write(pre_be_reg, 0x0);
        isp_wdr_normalize_lut_waddr_write(pre_be_reg, 0x0);

        for (j = 0; j < WDR_NOISE_LUT_NUM; j++) {
            isp_wdr_snoise_lut_wdata_write(pre_be_reg, dyna_reg_cfg->noise_profile[0][j]);
        }

        for (j = 0; j < WDR_NOISE_LUT_NUM; j++) {
            isp_wdr_lnoise_lut_wdata_write(pre_be_reg, dyna_reg_cfg->noise_profile[1][j]);
        }

        for (j = 0; j < WDR_SIGMA_LUT_NUM; j++) {
            isp_wdr_normalize_lut_wdata_write(pre_be_reg, dyna_reg_cfg->sigma_nml_lut[j]);
        }
    } else {
        isp_wdr_snoise_lut_write(&pre_be_reg->be_lut.be_apb_lut, dyna_reg_cfg->noise_profile[0]);
        isp_wdr_lnoise_lut_write(&pre_be_reg->be_lut.be_apb_lut, dyna_reg_cfg->noise_profile[1]);
        isp_wdr_normalize_lut_write(&pre_be_reg->be_lut.be_apb_lut, dyna_reg_cfg->sigma_nml_lut);
    }

    isp_wdr_snoise_lut_update_write(pre_be_reg, 0x1);
    isp_wdr_lnoise_lut_update_write(pre_be_reg, 0x1);
    isp_wdr_normalize_lut_update_write(pre_be_reg, 0x1);
}

static td_void isp_wdr_static_reg_config_first_frame(isp_pre_be_reg_type *be_reg, isp_fswdr_static_cfg *static_reg_cfg,
    isp_fswdr_dyna_cfg *dyna_reg_cfg, isp_fswdr_usr_cfg *usr_reg_cfg)
{
    td_u32 expo_value[4]; /* 4 means the number of  max frame for wdr mode */

    expo_value[0] = static_reg_cfg->expo_value_lut[0];                      /* array index 0  assignment */
    expo_value[1] = static_reg_cfg->expo_value_lut[1];                      /* array index 1  assignment */
    expo_value[2] = static_reg_cfg->expo_value_lut[2];                      /* array index 2  assignment */
    expo_value[3] = static_reg_cfg->expo_value_lut[3];                      /* array index 3  assignment */
    isp_wdr_expovalue0_write(be_reg, expo_value[0]);                        /* expovalue 0 */
    isp_wdr_expovalue1_write(be_reg, expo_value[1]);                        /* expovalue 1 */
    isp_wdr_exporratio0_write(be_reg, static_reg_cfg->expo_r_ratio_lut[0]); /* expo_r_ratio 0 */
    isp_flick_exporatio_write(be_reg, static_reg_cfg->flick_exp_ratio);
    isp_wdr_mdt_en_write(be_reg, dyna_reg_cfg->wdr_mdt_en);
    isp_wdr_fusionmode_write(be_reg, usr_reg_cfg->fusion_mode);
    isp_wdr_maxratio_write(be_reg, static_reg_cfg->max_ratio);
    isp_wdr_short_thr_write(be_reg, dyna_reg_cfg->short_thr);
    isp_wdr_long_thr_write(be_reg, dyna_reg_cfg->long_thr);

    static_reg_cfg->first_frame = TD_FALSE;
}

static td_void isp_wdr_static_reg_config(isp_pre_be_reg_type *be_reg, isp_fswdr_static_cfg *static_reg_cfg)
{
    isp_wdr_grayscale_mode_write(be_reg, static_reg_cfg->gray_scale_mode);
    isp_wdr_bsaveblc_write(be_reg, static_reg_cfg->save_blc);
    isp_wdr_mask_similar_thr_write(be_reg, static_reg_cfg->mask_similar_thr);
    isp_wdr_mask_similar_cnt_write(be_reg, static_reg_cfg->mask_similar_cnt);
    isp_wdr_dftwgt_fl_write(be_reg, static_reg_cfg->dft_wgt_fl);
    isp_wdr_bldrlhfidx_write(be_reg, static_reg_cfg->bldr_lhf_idx);
    isp_wdr_forcelong_smooth_en_write(be_reg, static_reg_cfg->force_long_smooth_en);
    isp_wdr_seg_idx_base0_write(be_reg, static_reg_cfg->seg_idx_base[0]);
    isp_wdr_seg_idx_base1_write(be_reg, static_reg_cfg->seg_idx_base[1]);
    isp_wdr_seg_idx_base2_write(be_reg, static_reg_cfg->seg_idx_base[2]); /* 2 */
    isp_wdr_seg_idx_base3_write(be_reg, static_reg_cfg->seg_idx_base[3]); /* 3 */
    isp_wdr_seg_idx_base4_write(be_reg, static_reg_cfg->seg_idx_base[4]); /* 4 */
    isp_wdr_seg_idx_base5_write(be_reg, static_reg_cfg->seg_idx_base[5]); /* 5 */
    isp_wdr_seg_idx_base6_write(be_reg, static_reg_cfg->seg_idx_base[6]); /* 6 */
    isp_wdr_seg_idx_base7_write(be_reg, static_reg_cfg->seg_idx_base[7]); /* 7 */

    isp_wdr_seg_maxval0_write(be_reg, static_reg_cfg->seg_max_val[0]);
    isp_wdr_seg_maxval1_write(be_reg, static_reg_cfg->seg_max_val[1]);
    isp_wdr_seg_maxval2_write(be_reg, static_reg_cfg->seg_max_val[2]); /* 2 */
    isp_wdr_seg_maxval3_write(be_reg, static_reg_cfg->seg_max_val[3]); /* 3 */
    isp_wdr_seg_maxval4_write(be_reg, static_reg_cfg->seg_max_val[4]); /* 4 */
    isp_wdr_seg_maxval5_write(be_reg, static_reg_cfg->seg_max_val[5]); /* 5 */
    isp_wdr_seg_maxval6_write(be_reg, static_reg_cfg->seg_max_val[6]); /* 6 */
    isp_wdr_seg_maxval7_write(be_reg, static_reg_cfg->seg_max_val[7]); /* 7 */

    static_reg_cfg->resh = TD_FALSE;
}

static td_void isp_wdr_dyna_reg_config_first(isp_pre_be_reg_type *be_reg, isp_fswdr_dyna_cfg *dyna_reg_cfg)
{
    isp_wdr_sqrt_again_g_write(be_reg, dyna_reg_cfg->sqrt_again_g);
    isp_wdr_sqrt_dgain_g_write(be_reg, dyna_reg_cfg->sqrt_dgain_g);
    isp_wdr_mdt_nosfloor_write(be_reg, dyna_reg_cfg->mdt_nos_floor);

    isp_wdr_mdthr_low_gain_write(be_reg, dyna_reg_cfg->md_thr_low_gain);
    isp_wdr_mdthr_hig_gain_write(be_reg, dyna_reg_cfg->md_thr_hig_gain);

    isp_wdr_f0_still_thr_write(be_reg, dyna_reg_cfg->still_thr_lut[0]);

    isp_wdr_erosion_en_write(be_reg, dyna_reg_cfg->erosion_en);
    isp_wdr_fusion_f0_thr_write(be_reg, dyna_reg_cfg->fusion_thr_lut[0]);
    isp_wdr_fusion_f1_thr_write(be_reg, dyna_reg_cfg->fusion_thr_lut[1]);

    isp_wdr_rgain_write(be_reg, dyna_reg_cfg->awb_rgain);
    isp_wdr_ggain_write(be_reg, dyna_reg_cfg->awb_ggain);
    isp_wdr_bgain_write(be_reg, dyna_reg_cfg->awb_bgain);
    isp_wdr_rev_rgain_write(be_reg, dyna_reg_cfg->awb_rev_rgain);
    isp_wdr_rev_ggain_write(be_reg, dyna_reg_cfg->awb_rev_ggain);
    isp_wdr_rev_bgain_write(be_reg, dyna_reg_cfg->awb_rev_bgain);
}

static td_void isp_wdr_dyna_reg_config_second(isp_pre_be_reg_type *be_reg, isp_fswdr_dyna_cfg *dyna_reg_cfg,
    isp_post_be_reg_type *post_be_reg)
{
    isp_bcom_alpha_write(be_reg, dyna_reg_cfg->bcom_alpha);
    isp_bdec_alpha_write(post_be_reg, dyna_reg_cfg->bdec_alpha);
    isp_wdr_forcelong_en_write(be_reg, dyna_reg_cfg->force_long);
    isp_wdr_forcelong_low_thd_write(be_reg, dyna_reg_cfg->force_long_low_thr);
    isp_wdr_forcelong_high_thd_write(be_reg, dyna_reg_cfg->force_long_hig_thr);

    isp_wdr_shortchk_thd_write(be_reg, dyna_reg_cfg->short_check_thd);
    isp_wdr_fusion_data_mode_write(be_reg, dyna_reg_cfg->fusion_data_mode);
    isp_wdr_fusion_data_select_write(be_reg, dyna_reg_cfg->pixel_fusion_data_select);
    isp_wdr_fuionblend_en_write(be_reg, dyna_reg_cfg->fusion_blend_en);
    isp_wdr_pixelfuion_en_write(be_reg, dyna_reg_cfg->pixel_fusion_en);
    isp_wdr_fusion_lhf_blend_wgt_write(be_reg, dyna_reg_cfg->fusion_blend_wgt);

    isp_wdr_fusion_sepr_thd_en_write(be_reg, dyna_reg_cfg->fusion_separated_threshold_en);
    isp_wdr_fusion_aux_data_en_write(be_reg, dyna_reg_cfg->fusion_aux_data_en);
    isp_wdr_fusion_clamp_en_write(be_reg, dyna_reg_cfg->fusion_clamp_en);
    isp_wdr_fusion_lf_low_thd_write(be_reg, dyna_reg_cfg->fusion_lf_threshold_low);
    isp_wdr_fusion_lf_high_thd_write(be_reg, dyna_reg_cfg->fusion_lf_threshold_high);
    isp_wdr_fusion_bld_low_thd_write(be_reg, dyna_reg_cfg->fusion_blend_low);
    isp_wdr_fusion_bld_high_thd_write(be_reg, dyna_reg_cfg->fusion_blend_high);
    isp_wdr_fusion_f0_calc_thr_write(be_reg, dyna_reg_cfg->fusion_calc_thr_lut[0]);
    isp_wdr_fusion_f1_calc_thr_write(be_reg, dyna_reg_cfg->fusion_calc_thr_lut[1]);
}

static td_void isp_wdr_user_reg_config(isp_pre_be_reg_type *be_reg, isp_fswdr_usr_cfg *usr_reg_cfg,
    td_bool offline_mode)
{
    td_bool usr_resh;
    td_bool idx_resh;

    idx_resh = (isp_wdr_update_index_read(be_reg) != usr_reg_cfg->update_index);
    usr_resh = (offline_mode) ? (usr_reg_cfg->resh && idx_resh) : (usr_reg_cfg->resh);
    if (usr_resh) {
        isp_wdr_update_index_write(be_reg, usr_reg_cfg->update_index);
        isp_wdr_shortexpo_chk_write(be_reg, usr_reg_cfg->short_expo_chk);
        isp_wdr_mdtlbld_write(be_reg, usr_reg_cfg->mdt_l_bld);
        isp_wdr_mdt_full_thr_write(be_reg, usr_reg_cfg->mdt_full_thr);
        isp_wdr_mdt_still_thr_write(be_reg, usr_reg_cfg->mdt_still_thr);
        isp_wdr_pixel_avg_max_diff_write(be_reg, usr_reg_cfg->pixel_avg_max_diff);
        usr_reg_cfg->resh = offline_mode;
    }
}

static td_void isp_wdr_module_en_write(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(pre_viproc);
    isp_check_pointer_void_return(post_viproc);

    isp_wdr_en_write(pre_viproc, reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.wdr_en);
    isp_bcom_en_write(pre_viproc, reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.dyna_reg_cfg.bcom_en);
    isp_bdec_en_write(post_viproc, reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.dyna_reg_cfg.bdec_en);
}

static td_s32 isp_wdr_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_fswdr_static_cfg *static_reg_cfg = TD_NULL;
    isp_fswdr_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_fswdr_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));
    if (reg_cfg_info->cfg_key.bit1_fs_wdr_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(post_be_reg);
        isp_wdr_module_en_write(vi_pipe, reg_cfg_info, i);

        isp_bnr_wdr_enable_write(post_be_reg, reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.wdr_en);
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.static_reg_cfg;
        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.usr_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.dyna_reg_cfg;

        if (static_reg_cfg->first_frame == TD_TRUE) {
            isp_wdr_static_reg_config_first_frame(be_reg, static_reg_cfg, dyna_reg_cfg, usr_reg_cfg);
        }
        if (static_reg_cfg->resh) {
            isp_wdr_static_reg_config(be_reg, static_reg_cfg);
        }

        isp_wdr_user_reg_config(be_reg, usr_reg_cfg, offline_mode);

        if (dyna_reg_cfg->resh) {
            isp_wdr_dyna_reg_config_first(be_reg, dyna_reg_cfg);
            isp_wdr_dyna_reg_config_second(be_reg, dyna_reg_cfg, post_be_reg);
            isp_wdr_lut_def_reg_config(vi_pipe, be_reg, dyna_reg_cfg);
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_fs_wdr_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }
    return TD_SUCCESS;
}

static td_void isp_drc_cclut_reg_config(td_bool offline_mode, isp_post_be_reg_type *be_reg,
    isp_drc_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    if (offline_mode == TD_FALSE) {
        isp_adrc_cclut_waddr_write(be_reg, 0);

        for (j = 0; j < OT_ISP_DRC_CC_NODE_NUM; j++) {
            isp_adrc_cclut_wdata_write(be_reg, usr_reg_cfg->cc_lut[j]);
        }
    } else {
        isp_drc_cclut_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->cc_lut);
    }
    isp_adrc_cclut_update_write(be_reg, 1);
}

static td_void isp_drc_tmlut_reg_config(td_bool *tm_lut_update, const isp_usr_ctx *isp_ctx,
    isp_post_be_reg_type *be_reg, isp_drc_dyna_cfg *dyna_reg_cfg)
{
    td_u16 j;

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        if (dyna_reg_cfg->lut_update) {
            isp_adrc_tmlut0_waddr_write(be_reg, 0);
            for (j = 0; j < OT_ISP_DRC_TM_NODE_NUM; j++) {
                isp_adrc_tmlut0_wdata_write(be_reg, ((dyna_reg_cfg->tone_mapping_value[j]) << 14) | /* 14 bit */
                    (dyna_reg_cfg->tone_mapping_diff[j]));
            }
            *tm_lut_update = TD_TRUE;
        }
    } else {
        isp_drc_tmlut0_value_write(&be_reg->be_lut.be_apb_lut, dyna_reg_cfg->tone_mapping_value);
        isp_drc_tmlut0_diff_write(&be_reg->be_lut.be_apb_lut, dyna_reg_cfg->tone_mapping_diff);
    }
}


static td_void isp_drc_mixlut_reg_config(td_bool offline_mode, isp_post_be_reg_type *be_reg,
    isp_drc_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    if (offline_mode == TD_FALSE) {
        isp_adrc_localmix_waddr_write(be_reg, 0);
        for (j = 0; j < OT_ISP_DRC_LMIX_NODE_NUM; j++) {
            isp_adrc_localmix_wdata_write(be_reg, (((td_u32)usr_reg_cfg->mixing_bright1[j] << 24) | /* from 24 bit */
                ((td_u32)usr_reg_cfg->mixing_dark1[j] << 16) |                                      /* from 16 bit */
                ((td_u32)usr_reg_cfg->mixing_bright0[j] << 8) |                                     /* from 8 bit */
                (td_u32)usr_reg_cfg->mixing_dark0[j]));
        }
        isp_adrc_mixlut_update_write(be_reg, 1);
    } else {
        isp_drc_local_mix_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->mixing_dark0, usr_reg_cfg->mixing_bright0,
                                usr_reg_cfg->mixing_dark1, usr_reg_cfg->mixing_bright1);
        isp_adrc_mixlut_update_write(be_reg, 1);
    }
}

static td_void isp_drc_declut_reg_config(td_bool offline_mode, isp_post_be_reg_type *be_reg,
    isp_drc_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    isp_adrc_detail_rd_en_write(be_reg, usr_reg_cfg->detail_rd_en);
    if (offline_mode == TD_FALSE) {
        isp_adrc_detaildec_waddr_write(be_reg, 0);
        for (j = 0; j < DRC_DEC_LUT_NODE_NUM; j++) {
            isp_adrc_detaildec_wdata_write(be_reg, usr_reg_cfg->detail_dec_lut[j]);
        }
        isp_adrc_declut_update_write(be_reg, 1);
    } else {
        isp_drc_detail_dec_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->detail_dec_lut);
        isp_adrc_declut_update_write(be_reg, 1);
    }
}

static td_void isp_drc_logcvtlut_reg_config(td_bool offline_mode, isp_post_be_reg_type *be_reg,
    isp_drc_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    if (offline_mode == TD_FALSE) {
        if (usr_reg_cfg->update_log_cvt == TD_TRUE) {
            isp_adrc_logcvt_waddr_write(be_reg, 0);
            isp_adrc_logcvt_luma_waddr_write(be_reg, 0);
            for (j = 0; j < DRC_DEC_LUT_NODE_NUM; j++) {
                isp_adrc_logcvt_wdata_write(be_reg, usr_reg_cfg->log_cvt_lut[j]);
                isp_adrc_logcvt_luma_wdata_write(be_reg, usr_reg_cfg->log_cvt_lut[j]);
            }
            usr_reg_cfg->update_log_cvt = TD_FALSE;
        }
    } else {
        isp_drc_logcvt_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->log_cvt_lut);
        isp_drc_logcvt_luma_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->log_cvt_lut);
    }
}

static td_void isp_drc_static_comm_reg_config(isp_post_be_reg_type *be_reg, isp_drc_static_cfg *static_reg_cfg)
{
    isp_adrc_r_wgt_write(be_reg, static_reg_cfg->r_wgt);
    isp_adrc_g_wgt_write(be_reg, static_reg_cfg->g_wgt);
    isp_adrc_b_wgt_write(be_reg, static_reg_cfg->b_wgt);

    isp_adrc_wgt_r_tm_write(be_reg, static_reg_cfg->r_wgt_tm);
    isp_adrc_wgt_g_tm_write(be_reg, static_reg_cfg->g_wgt_tm);
    isp_adrc_wgt_b_tm_write(be_reg, static_reg_cfg->b_wgt_tm);

    isp_adrc_mono_chroma_en_write(be_reg, static_reg_cfg->monochrome_mode);

    isp_adrc_tmlutidx_sel_write(be_reg, static_reg_cfg->tm_idx_sel);
    isp_adrc_wgt_box_tri_sel_write(be_reg, static_reg_cfg->wgt_box_tri_sel);

    isp_adrc_bin_write(be_reg, static_reg_cfg->bin_num_z);
    isp_adrc_gain_clip_mode_write(be_reg, static_reg_cfg->gain_clip_mode);

    isp_adrc_wgtm_sft_write(be_reg, static_reg_cfg->wgt_m_sft);
    isp_adrc_wgtm_en_write(be_reg, static_reg_cfg->wgt_m_en);
    isp_adrc_wgtm_pd_slo_write(be_reg, static_reg_cfg->wgt_m_pd_slo);
    isp_adrc_wgtm_pd_sft_write(be_reg, static_reg_cfg->wgt_m_pd_sft);
    isp_adrc_wgtm_pd_thr_write(be_reg, static_reg_cfg->wgt_m_pd_thr);

    isp_adrc_pdbg_slo_write(be_reg, static_reg_cfg->pd_bg_slo);
    isp_adrc_pdbg_thr_write(be_reg, static_reg_cfg->pd_bg_thr);
    isp_adrc_pdrg_slo_write(be_reg, static_reg_cfg->pd_rg_slo);
    isp_adrc_pdrg_wid_write(be_reg, static_reg_cfg->pd_rg_wid);
    isp_adrc_pdrg_crt_write(be_reg, static_reg_cfg->pd_rg_crt);
    isp_adrc_pdwhighslo_write(be_reg, static_reg_cfg->pdw_high_slo);
    isp_adrc_pdwlowthr_write(be_reg, static_reg_cfg->pdw_low_thr);
    isp_adrc_pdwlowslo_write(be_reg, static_reg_cfg->pdw_low_slo);

    isp_adrc_maskc_slo_write(be_reg, static_reg_cfg->maskc_slo);
    isp_adrc_maskc_thr_write(be_reg, static_reg_cfg->maskc_thr);
    isp_adrc_maskc_max_write(be_reg, static_reg_cfg->maskc_max);
    isp_adrc_maskc_min_write(be_reg, static_reg_cfg->maskc_min);

    isp_adrc_lumapwthr0_write(be_reg, static_reg_cfg->luma_pwthr0);
    isp_adrc_lumapwthr1_write(be_reg, static_reg_cfg->luma_pwthr1);
    isp_adrc_lumapwthr2_write(be_reg, static_reg_cfg->luma_pwthr2);
    isp_adrc_lumapwthr3_write(be_reg, static_reg_cfg->luma_pwthr3);
    isp_adrc_lumapwslo0_write(be_reg, static_reg_cfg->luma_pwslo0);
    isp_adrc_lumapwslo1_write(be_reg, static_reg_cfg->luma_pwslo1);
    isp_adrc_lumapwslo2_write(be_reg, static_reg_cfg->luma_pwslo2);
    isp_adrc_lumapwslo3_write(be_reg, static_reg_cfg->luma_pwslo3);
    isp_adrc_lumapwsft0_write(be_reg, static_reg_cfg->luma_pwsft0);
    isp_adrc_lumapwsft1_write(be_reg, static_reg_cfg->luma_pwsft1);
    isp_adrc_lumapwsft2_write(be_reg, static_reg_cfg->luma_pwsft2);
    isp_adrc_lumapwsft3_write(be_reg, static_reg_cfg->luma_pwsft3);
}

static td_void isp_drc_seg_reg_config(isp_post_be_reg_type *be_reg, isp_drc_static_cfg *static_reg_cfg)
{
    isp_adrc_seg_idxbase0_write(be_reg, static_reg_cfg->seg_idx_base[0]); /* idxbase[0] */
    isp_adrc_seg_idxbase1_write(be_reg, static_reg_cfg->seg_idx_base[1]); /* idxbase[1] */
    isp_adrc_seg_idxbase2_write(be_reg, static_reg_cfg->seg_idx_base[2]); /* idxbase[2] */
    isp_adrc_seg_idxbase3_write(be_reg, static_reg_cfg->seg_idx_base[3]); /* idxbase[3] */
    isp_adrc_seg_idxbase4_write(be_reg, static_reg_cfg->seg_idx_base[4]); /* idxbase[4] */
    isp_adrc_seg_idxbase5_write(be_reg, static_reg_cfg->seg_idx_base[5]); /* idxbase[5] */
    isp_adrc_seg_idxbase6_write(be_reg, static_reg_cfg->seg_idx_base[6]); /* idxbase[6] */
    isp_adrc_seg_idxbase7_write(be_reg, static_reg_cfg->seg_idx_base[7]); /* idxbase[7] */

    isp_adrc_seg_maxval0_write(be_reg, static_reg_cfg->seg_max_val[0]); /* maxval[0] */
    isp_adrc_seg_maxval1_write(be_reg, static_reg_cfg->seg_max_val[1]); /* maxval[1] */
    isp_adrc_seg_maxval2_write(be_reg, static_reg_cfg->seg_max_val[2]); /* maxval[2] */
    isp_adrc_seg_maxval3_write(be_reg, static_reg_cfg->seg_max_val[3]); /* maxval[3] */
    isp_adrc_seg_maxval4_write(be_reg, static_reg_cfg->seg_max_val[4]); /* maxval[4] */
    isp_adrc_seg_maxval5_write(be_reg, static_reg_cfg->seg_max_val[5]); /* maxval[5] */
    isp_adrc_seg_maxval6_write(be_reg, static_reg_cfg->seg_max_val[6]); /* maxval[6] */
    isp_adrc_seg_maxval7_write(be_reg, static_reg_cfg->seg_max_val[7]); /* maxval[7] */
}

static td_void isp_drc_prev_luma_reg_config(isp_post_be_reg_type *be_reg, isp_drc_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->first_frame) {
        isp_adrc_shp_log_luma_write(be_reg, static_reg_cfg->shp_log);
        isp_adrc_shp_exp_luma_write(be_reg, static_reg_cfg->shp_exp);
        isp_adrc_div_denom_log_luma_write(be_reg, static_reg_cfg->div_denom_log);
        isp_adrc_denom_exp_luma_write(be_reg, static_reg_cfg->denom_exp);
        isp_adrc_prev_luma_0_write(be_reg, static_reg_cfg->prev_luma[0]); /* prev_luma[0] */
        isp_adrc_prev_luma_1_write(be_reg, static_reg_cfg->prev_luma[1]); /* prev_luma[1] */
        isp_adrc_prev_luma_2_write(be_reg, static_reg_cfg->prev_luma[2]); /* prev_luma[2] */
        isp_adrc_prev_luma_3_write(be_reg, static_reg_cfg->prev_luma[3]); /* prev_luma[3] */
        isp_adrc_prev_luma_4_write(be_reg, static_reg_cfg->prev_luma[4]); /* prev_luma[4] */
        isp_adrc_prev_luma_5_write(be_reg, static_reg_cfg->prev_luma[5]); /* prev_luma[5] */
        isp_adrc_prev_luma_6_write(be_reg, static_reg_cfg->prev_luma[6]); /* prev_luma[6] */
        isp_adrc_prev_luma_7_write(be_reg, static_reg_cfg->prev_luma[7]); /* prev_luma[7] */

        static_reg_cfg->first_frame = TD_FALSE;
    }
}

static td_void isp_drc_usr_comm_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    isp_adrc_sft1_y_write(be_reg, usr_reg_cfg->ysft1);
    isp_adrc_val1_y_write(be_reg, usr_reg_cfg->yval1);
    isp_adrc_sft2_y_write(be_reg, usr_reg_cfg->ysft2);
    isp_adrc_val2_y_write(be_reg, usr_reg_cfg->yval2);

    isp_adrc_sft1_c_write(be_reg, usr_reg_cfg->csft1);
    isp_adrc_val1_c_write(be_reg, usr_reg_cfg->cval1);
    isp_adrc_sft2_c_write(be_reg, usr_reg_cfg->csft2);
    isp_adrc_val2_c_write(be_reg, usr_reg_cfg->cval2);

    isp_adrc_sft_write(be_reg, usr_reg_cfg->sft);
    isp_adrc_val_write(be_reg, usr_reg_cfg->val);

    isp_adrc_gain_clip_knee_write(be_reg, usr_reg_cfg->gain_clip_knee);
    isp_adrc_gain_clip_step_write(be_reg, usr_reg_cfg->gain_clip_step);

    isp_adrc_shp_log_raw_write(be_reg, usr_reg_cfg->shp_log_raw);
    isp_adrc_shp_exp_raw_write(be_reg, usr_reg_cfg->shp_exp_raw);
    isp_adrc_div_denom_log_raw_write(be_reg, usr_reg_cfg->div_denom_log_raw);
    isp_adrc_denom_exp_raw_write(be_reg, usr_reg_cfg->denom_exp_raw);

    isp_adrc_cc_lin_pow_write(be_reg, usr_reg_cfg->cc_lin_pow);
    isp_adrc_cc_lut_inout_write(be_reg, usr_reg_cfg->cc_lut_inout);
    isp_adrc_tm_lut_ctrl_write(be_reg, usr_reg_cfg->tm_ctrl);
}

static td_void isp_drc_flt_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    isp_adrc_var_spa_fine_write(be_reg, usr_reg_cfg->var_spa_fine);
    isp_adrc_var_rng_fine_write(be_reg, usr_reg_cfg->var_rng_fine);

    isp_adrc_grad_rev_shift_write(be_reg, usr_reg_cfg->grad_rev_shift);
    isp_adrc_grad_rev_slope_write(be_reg, usr_reg_cfg->grad_rev_slope);
    isp_adrc_grad_rev_max_write(be_reg, usr_reg_cfg->grad_rev_max);
    isp_adrc_grad_rev_thres_write(be_reg, usr_reg_cfg->grad_rev_thr);

    isp_adrc_flt_spa_fine_write(be_reg, usr_reg_cfg->flt_spa_fine);
    isp_adrc_flt_rng_fine_write(be_reg, usr_reg_cfg->flt_rng_fine);

    isp_adrc_dsubfact0_write(be_reg, usr_reg_cfg->detail_adjust_coef0);

    isp_adrc_brightmax0_write(be_reg, usr_reg_cfg->detail_sup_bright_max0);
    isp_adrc_brightmin0_write(be_reg, usr_reg_cfg->detail_sup_bright_min0);
    isp_adrc_brightthr0_write(be_reg, usr_reg_cfg->detail_sup_bright_thr0);
    isp_adrc_brightslo0_write(be_reg, usr_reg_cfg->detail_sup_bright_slo0);
    isp_adrc_darkmax0_write(be_reg, usr_reg_cfg->detail_sup_dark_max0);
    isp_adrc_darkmin0_write(be_reg, usr_reg_cfg->detail_sup_dark_min0);
    isp_adrc_darkthr0_write(be_reg, usr_reg_cfg->detail_sup_dark_thr0);
    isp_adrc_darkslo0_write(be_reg, usr_reg_cfg->detail_sup_dark_slo0);

    isp_adrc_supfact1_write(be_reg, usr_reg_cfg->detail_sup1);

    isp_adrc_bld_sftl_write(be_reg, usr_reg_cfg->bld_l_shift);
    isp_adrc_brightslol_write(be_reg, usr_reg_cfg->bld_l_bright_slo);
    isp_adrc_darkslol_write(be_reg, usr_reg_cfg->bld_l_dark_slo);

    isp_adrc_bld_sftd_write(be_reg, usr_reg_cfg->bld_d_shift);
    isp_adrc_brightslod_write(be_reg, usr_reg_cfg->bld_d_bright_slo);
    isp_adrc_darkslod_write(be_reg, usr_reg_cfg->bld_d_dark_slo);
}

static td_void isp_drc_advanced_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    isp_adrc_dsubfact1_write(be_reg, usr_reg_cfg->detail_adjust_coef1);
    isp_adrc_boostfact1_write(be_reg, usr_reg_cfg->detail_boost1);

    isp_adrc_brightmaxl_write(be_reg, usr_reg_cfg->bld_l_bright_max);
    isp_adrc_brightminl_write(be_reg, usr_reg_cfg->bld_l_bright_min);
    isp_adrc_brightthrl_write(be_reg, usr_reg_cfg->bld_l_bright_thr);
    isp_adrc_darkmaxl_write(be_reg, usr_reg_cfg->bld_l_dark_max);
    isp_adrc_darkminl_write(be_reg, usr_reg_cfg->bld_l_dark_min);
    isp_adrc_darkthrl_write(be_reg, usr_reg_cfg->bld_l_dark_thr);

    isp_adrc_brightmaxd_write(be_reg, usr_reg_cfg->bld_d_bright_max);
    isp_adrc_brightmind_write(be_reg, usr_reg_cfg->bld_d_bright_min);
    isp_adrc_brightthrd_write(be_reg, usr_reg_cfg->bld_d_bright_thr);
    isp_adrc_darkmaxd_write(be_reg, usr_reg_cfg->bld_d_dark_max);
    isp_adrc_darkmind_write(be_reg, usr_reg_cfg->bld_d_dark_min);
    isp_adrc_darkthrd_write(be_reg, usr_reg_cfg->bld_d_dark_thr);

    isp_adrc_statflttype_write(be_reg, usr_reg_cfg->stat_flt_type);
    isp_adrc_statformat_write(be_reg, usr_reg_cfg->stat_format);

    isp_adrc_inadapt_mode_write(be_reg, usr_reg_cfg->in_adapt_mode);

    isp_adrc_ltm_gain_coef_write(be_reg, usr_reg_cfg->ltm_gain_coef);
    isp_adrc_gtm_gain_coef_write(be_reg, usr_reg_cfg->gtm_gain_coef);
}

static td_void isp_drc_wgtm_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    isp_adrc_wgtm_0_write(be_reg, usr_reg_cfg->wgt_m_lut[0]);   /* wgt_m[0] */
    isp_adrc_wgtm_1_write(be_reg, usr_reg_cfg->wgt_m_lut[1]);   /* wgt_m[1] */
    isp_adrc_wgtm_2_write(be_reg, usr_reg_cfg->wgt_m_lut[2]);   /* wgt_m[2] */
    isp_adrc_wgtm_3_write(be_reg, usr_reg_cfg->wgt_m_lut[3]);   /* wgt_m[3] */
    isp_adrc_wgtm_4_write(be_reg, usr_reg_cfg->wgt_m_lut[4]);   /* wgt_m[4] */
    isp_adrc_wgtm_5_write(be_reg, usr_reg_cfg->wgt_m_lut[5]);   /* wgt_m[5] */
    isp_adrc_wgtm_6_write(be_reg, usr_reg_cfg->wgt_m_lut[6]);   /* wgt_m[6] */
    isp_adrc_wgtm_7_write(be_reg, usr_reg_cfg->wgt_m_lut[7]);   /* wgt_m[7] */
    isp_adrc_wgtm_8_write(be_reg, usr_reg_cfg->wgt_m_lut[8]);   /* wgt_m[8] */
    isp_adrc_wgtm_9_write(be_reg, usr_reg_cfg->wgt_m_lut[9]);   /* wgt_m[9] */
    isp_adrc_wgtm_10_write(be_reg, usr_reg_cfg->wgt_m_lut[10]); /* wgt_m[10] */
    isp_adrc_wgtm_11_write(be_reg, usr_reg_cfg->wgt_m_lut[11]); /* wgt_m[11] */
    isp_adrc_wgtm_12_write(be_reg, usr_reg_cfg->wgt_m_lut[12]); /* wgt_m[12] */
    isp_adrc_wgtm_13_write(be_reg, usr_reg_cfg->wgt_m_lut[13]); /* wgt_m[13] */
    isp_adrc_wgtm_14_write(be_reg, usr_reg_cfg->wgt_m_lut[14]); /* wgt_m[14] */
}

static td_void isp_drc_statcoef_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg)
{
    isp_adrc_tmgainskip_write(be_reg, usr_reg_cfg->tm_gain_skip);

    isp_adrc_statfltlinlog_write(be_reg, usr_reg_cfg->stat_flt_linlog);
    isp_adrc_statfltrawluma_write(be_reg, usr_reg_cfg->stat_flt_rawluma);

    isp_adrc_statcoefoffset00_write(be_reg, usr_reg_cfg->stat_coef_offst00);
    isp_adrc_statcoefoffset01_write(be_reg, usr_reg_cfg->stat_coef_offst01);
    isp_adrc_statcoefoffset02_write(be_reg, usr_reg_cfg->stat_coef_offst02);
    isp_adrc_statcoefoffset03_write(be_reg, usr_reg_cfg->stat_coef_offst03);
    isp_adrc_statcoefoffset10_write(be_reg, usr_reg_cfg->stat_coef_offst10);
    isp_adrc_statcoefoffset11_write(be_reg, usr_reg_cfg->stat_coef_offst11);
    isp_adrc_statcoefoffset12_write(be_reg, usr_reg_cfg->stat_coef_offst12);
    isp_adrc_statcoefoffset13_write(be_reg, usr_reg_cfg->stat_coef_offst13);
    isp_adrc_statcoefoffset20_write(be_reg, usr_reg_cfg->stat_coef_offst20);
    isp_adrc_statcoefoffset21_write(be_reg, usr_reg_cfg->stat_coef_offst21);
    isp_adrc_statcoefoffset22_write(be_reg, usr_reg_cfg->stat_coef_offst22);
    isp_adrc_statcoefoffset23_write(be_reg, usr_reg_cfg->stat_coef_offst23);

    isp_adrc_statcoefscale00_write(be_reg, usr_reg_cfg->stat_coef_scale00);
    isp_adrc_statcoefscale01_write(be_reg, usr_reg_cfg->stat_coef_scale01);
    isp_adrc_statcoefscale02_write(be_reg, usr_reg_cfg->stat_coef_scale02);
    isp_adrc_statcoefscale03_write(be_reg, usr_reg_cfg->stat_coef_scale03);
    isp_adrc_statcoefscale10_write(be_reg, usr_reg_cfg->stat_coef_scale10);
    isp_adrc_statcoefscale11_write(be_reg, usr_reg_cfg->stat_coef_scale11);
    isp_adrc_statcoefscale12_write(be_reg, usr_reg_cfg->stat_coef_scale12);
    isp_adrc_statcoefscale13_write(be_reg, usr_reg_cfg->stat_coef_scale13);
    isp_adrc_statcoefscale20_write(be_reg, usr_reg_cfg->stat_coef_scale20);
    isp_adrc_statcoefscale21_write(be_reg, usr_reg_cfg->stat_coef_scale21);
    isp_adrc_statcoefscale22_write(be_reg, usr_reg_cfg->stat_coef_scale22);
    isp_adrc_statcoefscale23_write(be_reg, usr_reg_cfg->stat_coef_scale23);

    isp_adrc_statcoefsft00_write(be_reg, usr_reg_cfg->stat_coef_shift00);
    isp_adrc_statcoefsft01_write(be_reg, usr_reg_cfg->stat_coef_shift01);
    isp_adrc_statcoefsft02_write(be_reg, usr_reg_cfg->stat_coef_shift02);
    isp_adrc_statcoefsft03_write(be_reg, usr_reg_cfg->stat_coef_shift03);
    isp_adrc_statcoefsft10_write(be_reg, usr_reg_cfg->stat_coef_shift10);
    isp_adrc_statcoefsft11_write(be_reg, usr_reg_cfg->stat_coef_shift11);
    isp_adrc_statcoefsft12_write(be_reg, usr_reg_cfg->stat_coef_shift12);
    isp_adrc_statcoefsft13_write(be_reg, usr_reg_cfg->stat_coef_shift13);
    isp_adrc_statcoefsft20_write(be_reg, usr_reg_cfg->stat_coef_shift20);
    isp_adrc_statcoefsft21_write(be_reg, usr_reg_cfg->stat_coef_shift21);
    isp_adrc_statcoefsft22_write(be_reg, usr_reg_cfg->stat_coef_shift22);
    isp_adrc_statcoefsft23_write(be_reg, usr_reg_cfg->stat_coef_shift23);

    isp_adrc_statfltminout_write(be_reg, usr_reg_cfg->stat_flt_min_out);
    isp_adrc_lumaavegmanual_write(be_reg, usr_reg_cfg->luma_avg_manual);
    isp_adrc_dmsmooth_en_write(be_reg, usr_reg_cfg->dm_smooth_en);
    isp_drclog_shp_log_raw_write(be_reg, usr_reg_cfg->drclog_shp);
    isp_drclog_div_denom_log_raw_write(be_reg, usr_reg_cfg->drclog_div_denom);
}

static td_void isp_drc_stat_reg_config(isp_post_be_reg_type *be_reg, isp_drc_dyna_cfg *dyna_reg_cfg)
{
    isp_adrc_vbiflt_en_write(be_reg, dyna_reg_cfg->vbiflt_en);
    isp_adrc_rdstat_en_write(be_reg, dyna_reg_cfg->rdstat_en);
    isp_adrc_wrstat_en_write(be_reg, dyna_reg_cfg->wrstat_en);

    isp_adrc_big_x_init_write(be_reg, dyna_reg_cfg->big_x_init);
    isp_adrc_idx_x_init_write(be_reg, dyna_reg_cfg->idx_x_init);
    isp_adrc_cnt_x_init_write(be_reg, dyna_reg_cfg->cnt_x_init);
    isp_adrc_acc_x_init_write(be_reg, dyna_reg_cfg->acc_x_init);
    isp_adrc_blk_wgt_init_write(be_reg, dyna_reg_cfg->wgt_x_init);
    isp_adrc_total_width_write(be_reg, dyna_reg_cfg->total_width - 1);
    isp_adrc_stat_width_write(be_reg, dyna_reg_cfg->stat_width - 1);

    isp_adrc_hnum_write(be_reg, dyna_reg_cfg->block_h_num);
    isp_adrc_vnum_write(be_reg, dyna_reg_cfg->block_v_num);

    isp_adrc_zone_hsize_write(be_reg, dyna_reg_cfg->block_h_size - 1);
    isp_adrc_zone_vsize_write(be_reg, dyna_reg_cfg->block_v_size - 1);
    isp_adrc_chk_x_write(be_reg, dyna_reg_cfg->block_chk_x);
    isp_adrc_chk_y_write(be_reg, dyna_reg_cfg->block_chk_y);

    isp_adrc_div_x0_write(be_reg, dyna_reg_cfg->div_x0);
    isp_adrc_div_x1_write(be_reg, dyna_reg_cfg->div_x1);
    isp_adrc_div_y0_write(be_reg, dyna_reg_cfg->div_y0);
    isp_adrc_div_y1_write(be_reg, dyna_reg_cfg->div_y1);

    isp_adrc_bin_scale_write(be_reg, dyna_reg_cfg->bin_scale);
}

static td_void isp_drc_static_reg_config(isp_post_be_reg_type *be_reg, isp_drc_static_cfg *static_reg_cfg)
{
    if (static_reg_cfg->resh) {
        isp_drc_static_comm_reg_config(be_reg, static_reg_cfg);
        isp_drc_seg_reg_config(be_reg, static_reg_cfg);
        isp_drc_prev_luma_reg_config(be_reg, static_reg_cfg);

        static_reg_cfg->resh = TD_FALSE;
    }
}

static td_void isp_drc_usr_reg_config(isp_post_be_reg_type *be_reg, isp_drc_usr_cfg *usr_reg_cfg, td_bool offline_mode)
{
    td_bool usr_resh, idx_resh;
    /* user */
    idx_resh = (isp_drc_update_index_read(be_reg) != usr_reg_cfg->update_index);
    usr_resh = (offline_mode) ? (usr_reg_cfg->resh && idx_resh) : (usr_reg_cfg->resh);

    isp_drc_advanced_reg_config(be_reg, usr_reg_cfg);
    if (usr_resh) {
        isp_drc_update_index_write(be_reg, usr_reg_cfg->update_index);

        isp_drc_usr_comm_reg_config(be_reg, usr_reg_cfg);
        isp_drc_flt_reg_config(be_reg, usr_reg_cfg);
        isp_drc_wgtm_reg_config(be_reg, usr_reg_cfg);
        isp_drc_statcoef_reg_config(be_reg, usr_reg_cfg);

        isp_drc_cclut_reg_config(offline_mode, be_reg, usr_reg_cfg);
        isp_drc_mixlut_reg_config(offline_mode, be_reg, usr_reg_cfg);
        isp_drc_declut_reg_config(offline_mode, be_reg, usr_reg_cfg);
        isp_drc_logcvtlut_reg_config(offline_mode, be_reg, usr_reg_cfg);

        usr_reg_cfg->resh = offline_mode;
    }
}

static td_s32 isp_drc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode, drc_en, drclog_en;
    td_bool tm_lut_update = TD_FALSE;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_drc_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_drc_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);

        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(post_viproc);

        drc_en = reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.enable;
        drclog_en = reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.usr_reg_cfg.drclog_en;
        isp_adrc_en_write(post_viproc, drc_en);
        isp_drclog_en_write(post_viproc, drc_en && drclog_en);

        isp_drc_static_reg_config(be_reg, &reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.static_reg_cfg);
        isp_drc_usr_reg_config(be_reg, &reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.usr_reg_cfg, offline_mode);

        /* dynamic */
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.dyna_reg_cfg;

        if (dyna_reg_cfg->resh) {
            isp_adrc_color_corr_en_write(be_reg, dyna_reg_cfg->cc_en);
            isp_adrc_pdwhighthr_write(be_reg, dyna_reg_cfg->pdw_high_thr);
            isp_drc_tmlut_reg_config(&tm_lut_update, isp_ctx, be_reg, dyna_reg_cfg);

            if (dyna_reg_cfg->img_size_changed) {
                isp_drc_stat_reg_config(be_reg, dyna_reg_cfg);
                dyna_reg_cfg->img_size_changed = offline_mode;
            }

            isp_adrc_strength_write(be_reg, dyna_reg_cfg->strength);

            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_drc_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg.drc_tm_lut_update = tm_lut_update || offline_mode;

    return TD_SUCCESS;
}

static td_void isp_dehaze_static_reg_config(isp_post_be_reg_type *be_reg, isp_dehaze_static_cfg *static_reg_cfg)
{
    /* static registers */
    if (static_reg_cfg->resh) {
        isp_dehaze_blthld_write(be_reg, static_reg_cfg->dehaze_blthld);
        isp_dehaze_block_sum_write(be_reg, static_reg_cfg->block_sum);
        isp_dehaze_dc_numh_write(be_reg, static_reg_cfg->dchnum);
        isp_dehaze_dc_numv_write(be_reg, static_reg_cfg->dcvnum);

        static_reg_cfg->resh = TD_FALSE;
    }
}

static td_s32 isp_dehaze_dyna_lut_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i,
    isp_post_be_reg_type *be_reg, isp_viproc_reg_type *post_viproc)
{
    td_u8 buf_id;
    td_u16 blk_num;
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_dehaze_static_cfg *static_reg_cfg = TD_NULL;
    isp_dehaze_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.static_reg_cfg;
    dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.dyna_reg_cfg;

    blk_num = ((static_reg_cfg->dchnum + 1) * (static_reg_cfg->dcvnum + 1) + 1) / 2; /* blk calculation 2 */
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        /* online lut2stt regconfig */
        buf_id = dyna_reg_cfg->buf_id;

        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_return(be_lut_stt_reg);

        isp_dehaze_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, blk_num, dyna_reg_cfg->prestat, dyna_reg_cfg->lut);
        ret = isp_dehaze_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
        if (ret != TD_SUCCESS) {
            isp_err_trace("isp[%d]_dehaze_lut_wstt_addr_write failed\n", vi_pipe);
            return ret;
        }

        dyna_reg_cfg->buf_id = 1 - buf_id;
    } else {
        isp_dehaze_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, blk_num, dyna_reg_cfg->prestat,
            dyna_reg_cfg->lut);
    }

    isp_dehaze_stt2lut_en_write(be_reg, TD_TRUE);
    isp_dehaze_stt2lut_regnew_write(be_reg, TD_TRUE);

    return TD_SUCCESS;
}

static td_s32 isp_dehaze_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_u8 sys_blk_num = reg_cfg_info->cfg_num;
    td_s32 ret;
    isp_dehaze_static_cfg *static_reg_cfg = TD_NULL;
    isp_dehaze_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_dehaze_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(post_viproc);
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.dyna_reg_cfg;

        isp_dehaze_static_reg_config(be_reg, static_reg_cfg);
        isp_dehaze_lut_width_word_write(post_viproc, OT_ISP_DEHAZE_LUT_WIDTH_WORD_DEFAULT);

        ret = isp_dehaze_dyna_lut_reg_config(vi_pipe, reg_cfg_info, i, be_reg, post_viproc);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        isp_dehaze_block_sizeh_write(be_reg, dyna_reg_cfg->blockhsize);
        isp_dehaze_block_sizev_write(be_reg, dyna_reg_cfg->blockvsize);
        isp_dehaze_phase_x_write(be_reg, dyna_reg_cfg->phasex);
        isp_dehaze_phase_y_write(be_reg, dyna_reg_cfg->phasey);

        isp_dehaze_smlmapoffset_write(be_reg, dyna_reg_cfg->sml_map_offset);
        isp_dehaze_statstartx_write(be_reg, dyna_reg_cfg->stat_start_x);
        isp_dehaze_statendx_write(be_reg, dyna_reg_cfg->stat_end_x);

        isp_dehaze_stat_numv_write(be_reg, dyna_reg_cfg->statnum_v);
        isp_dehaze_stat_numh_write(be_reg, dyna_reg_cfg->statnum_h);

        isp_dehaze_thld_tr_write(be_reg, dyna_reg_cfg->dehaze_thld_r);
        isp_dehaze_thld_tg_write(be_reg, dyna_reg_cfg->dehaze_thld_g);
        isp_dehaze_thld_tb_write(be_reg, dyna_reg_cfg->dehaze_thld_b);

        reg_cfg_info->cfg_key.bit1_dehaze_cfg = offline_mode ? 1 : ((sys_blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_bnr_lut_wstt_write(isp_pre_be_lut_wstt_type *pre_be_lut2stt, isp_bayernr_dyna_cfg *dyna_reg_cfg)
{
    isp_bnr_fbratiotable_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->u8fb_ratio_table);
    isp_bnr_noised_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->u16noise_sd_lut);
    isp_bnr_noiseds_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->u16noise_sd_lut_s);
    isp_bnr_noisedinv_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->u32noise_inv_lut);
    isp_bnr_noisedinv_mag_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->u8noise_inv_magidx);

    isp_bnr_noise_inv_l_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->noise_inv_1_lut);
    isp_bnr_coring_low_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->coring_low_lut);
    isp_bnr_mix_gain_lut_wstt_write(pre_be_lut2stt, dyna_reg_cfg->mix_gain_lut);

    isp_bnr_ynet_wstt_write(pre_be_lut2stt, dyna_reg_cfg->ynet_fg_noise_str_lut, dyna_reg_cfg->ynet_bg_noise_str_lut);
    isp_bnr_ynet_mot_wstt_write(pre_be_lut2stt, dyna_reg_cfg->ynet_mot_lut);
}

static td_void isp_bnr_lut_reg_config(ot_vi_pipe vi_pipe, isp_post_be_reg_type *be_reg,
    isp_bayernr_dyna_cfg *dyna_reg_cfg, td_u8 i)
{
    td_u8 buf_id = 0;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(viproc_reg);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        buf_id = dyna_reg_cfg->buf_id;

        be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
        isp_check_pointer_void_return(be_lut_stt_reg);

        isp_bnr_lut_wstt_write(&be_lut_stt_reg->pre_be_lut2stt, dyna_reg_cfg);
        isp_bnr_lut_wstt_addr_write(vi_pipe, i, buf_id, viproc_reg);

        dyna_reg_cfg->buf_id = 1 - buf_id;
    } else {
        isp_bnr_lut_wstt_write(&be_reg->be_lut.be_lut2stt.pre_be_lut2stt, dyna_reg_cfg);
    }
    isp_bnr_stt2lut_en_write(be_reg, TD_TRUE);
    isp_bnr_stt2lut_regnew_write(be_reg, TD_TRUE);
    isp_bnr_lut_width_word_write(viproc_reg, OT_ISP_BNR_LUT_WIDTH_WORD_DEFAULT);
}

static td_void isp_bnr_dyna_reg_config(isp_post_be_reg_type *be_reg, isp_bayernr_dyna_cfg *dyna_reg_cfg)
{
    isp_bnr_ensptnr_write(be_reg, dyna_reg_cfg->snr_enable);
    isp_bnr_en_irmd_write(be_reg, dyna_reg_cfg->ir_md);
    isp_bnr_bltev500_n_limit_gain_sad_d_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_n_limit_gain_sad_d);
    isp_bnr_bltev500_pnr_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_pnr);
    isp_bnr_bltev500_en_imp_nr_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_en_imp_nr);
    isp_bnr_bltev500_imp_nr_str_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_imp_nr_str);
    isp_bnr_bltev500_wt_ccoef_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_wt_ccoef);
    isp_bnr_bltev500_wt_cmax_write(be_reg, dyna_reg_cfg->isp_bnr_bltev500_wt_cmax);
    isp_bnr_jnlm_lmt_gain0_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain[0]); /* index 0 */
    isp_bnr_jnlm_lmt_gain1_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain[1]); /* index 1 */
    isp_bnr_jnlm_lmt_gain2_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain[2]); /* index 2 */
    isp_bnr_jnlm_lmt_gain3_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain[3]); /* index 3 */
    isp_bnr_jnlm_lmt_gain4_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain_s[0]); /* index 0 */
    isp_bnr_jnlm_lmt_gain5_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain_s[1]); /* index 1 */
    isp_bnr_jnlm_lmt_gain6_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain_s[2]); /* index 2 */
    isp_bnr_jnlm_lmt_gain7_write(be_reg, dyna_reg_cfg->jnlm_limit_mult_gain_s[3]); /* index 3 */
    isp_bnr_mdet_size_write(be_reg, dyna_reg_cfg->isp_bnr_mdet_size);
    isp_bnr_mdet_size_alpha_write(be_reg, dyna_reg_cfg->isp_bnr_mdet_size_alpha);
    isp_bnr_mdet_cor_level_write(be_reg, dyna_reg_cfg->isp_bnr_mdet_cor_level);
    isp_bnr_rnt_th_write(be_reg, dyna_reg_cfg->isp_bnr_rnt_th);
    isp_bnr_rnt_wid_write(be_reg, dyna_reg_cfg->isp_bnr_rnt_wid);
    isp_bnr_nr_strength_st_int_write(be_reg, dyna_reg_cfg->isp_bnr_nr_strength_st_int);
    isp_bnr_nr_strength_mv_int_write(be_reg, dyna_reg_cfg->isp_bnr_nr_strength_mv_int);
    isp_bnr_nr_strength_slope_write(be_reg, dyna_reg_cfg->isp_bnr_nr_strength_slope);
    isp_bnr_target_noise_ratio_write(be_reg, dyna_reg_cfg->isp_bnr_target_noise_ratio);
    isp_bnr_mix_gain_0_r_write(be_reg, dyna_reg_cfg->isp_bnr_mix_gain_0_r);
    isp_bnr_mix_gain_0_b_write(be_reg, dyna_reg_cfg->isp_bnr_mix_gain_0_b);
    isp_bnr_wb_gain_r_write(be_reg, dyna_reg_cfg->isp_bnr_wb_gain_r);
    isp_bnr_wb_gain_b_write(be_reg, dyna_reg_cfg->isp_bnr_wb_gain_b);
    isp_bnr_wb_gain_inv_r_write(be_reg, dyna_reg_cfg->isp_bnr_wb_gain_inv_r);
    isp_bnr_wb_gain_inv_b_write(be_reg, dyna_reg_cfg->isp_bnr_wb_gain_inv_b);
    isp_bnr_bnrlscen_write(be_reg, dyna_reg_cfg->bnr_lsc_en);
    isp_bnr_bnrlscratio_write(be_reg, dyna_reg_cfg->bnr_lsc_ratio);
    isp_bnr_wdr_en_fusion_write(be_reg, dyna_reg_cfg->wdr_en_fusion);
    isp_bnr_wdr_map_flt_mod_write(be_reg, dyna_reg_cfg->wdr_map_flt_mod);
    isp_bnr_wdr_map_gain_write(be_reg, dyna_reg_cfg->wdr_map_gain);
    isp_bnr_en_2nd_tmp_nr_write(be_reg, 1);

    isp_bnr_irmd_mixratio_rgb_write(be_reg, dyna_reg_cfg->isp_bnr_mixratio_rgb);
    isp_bnr_alphamax_st_write(be_reg, dyna_reg_cfg->alpha_max_st);
    isp_bnr_maxv_mode_write(be_reg, dyna_reg_cfg->maxv_mode);
    isp_bnr_coring_hig_write(be_reg, dyna_reg_cfg->coring_hig);
    isp_bnr_coring_enable_write(be_reg, dyna_reg_cfg->coring_enable);
    isp_bnr_jnlm_gain_write(be_reg, dyna_reg_cfg->fine_str);
    isp_bnr_alphamax_st_write(be_reg, dyna_reg_cfg->alpha_max_st);
    isp_bnr_maxv_mode_write(be_reg, dyna_reg_cfg->maxv_mode);
    isp_bnr_geq_edge_thresh_write(be_reg, dyna_reg_cfg->geq_edge_thresh);
    isp_bnr_geq_denoise_str_write(be_reg, dyna_reg_cfg->geq_denoise_str);
    isp_bnr_geq_slope_write(be_reg, dyna_reg_cfg->geq_slope);
    isp_bnr_geq_th_write(be_reg, dyna_reg_cfg->geq_th);
    isp_bnr_geq_str_st_write(be_reg, dyna_reg_cfg->geq_str_st);
    isp_bnr_geq_str_mv_write(be_reg, dyna_reg_cfg->geq_str_mv);
    isp_bnr_bnrlscen_write(be_reg, dyna_reg_cfg->bnr_lsc_en);
    isp_bnr_bnrlscratio_write(be_reg, dyna_reg_cfg->bnr_lsc_ratio);
    isp_bnr_mixgain_sprs_en_write(be_reg, dyna_reg_cfg->en_mix_gain_sprs);
    isp_bnr_inputraw_ratio_write(be_reg, dyna_reg_cfg->input_raw_ratio);

    isp_bnr_mdet_mixratio_write(be_reg, dyna_reg_cfg->isp_bnr_mdet_mix_ratio);

    isp_bnr_tnr1_ratio_write(be_reg, dyna_reg_cfg->tnr1_ratio);
    isp_bnr_mixing_mot_thresh_write(be_reg, dyna_reg_cfg->mixing_mot_thresh);
    isp_bnr_mixing_mot_gain_write(be_reg, dyna_reg_cfg->mixing_mot_gain);
    isp_bnr_mixing_mot_ratio_write(be_reg, dyna_reg_cfg->mixing_mot_ratio);

    isp_bnr_blt_edge_mode_write(be_reg, dyna_reg_cfg->edge_mode);

    isp_bnr_ynet_alpha_write(be_reg, dyna_reg_cfg->ynet_alpha);
    isp_bnr_ynet_mot_bitw_write(be_reg, dyna_reg_cfg->ynet_mot_bitw);

    isp_bnr_blt_mix_r1_n0_write(be_reg, dyna_reg_cfg->blt_mix_r1_n0);
    isp_bnr_blt_mix_r2_n0_write(be_reg, dyna_reg_cfg->blt_mix_r2_n0);
    isp_bnr_blt_mix_m1_n0_write(be_reg, dyna_reg_cfg->blt_mix_m1_n0);
    isp_bnr_blt_mix_m2_n0_write(be_reg, dyna_reg_cfg->blt_mix_m2_n0);
    isp_bnr_blt_mix_r1_n1_write(be_reg, dyna_reg_cfg->blt_mix_r1_n1);
    isp_bnr_blt_mix_r2_n1_write(be_reg, dyna_reg_cfg->blt_mix_r2_n1);
    isp_bnr_blt_mix_m1_n1_write(be_reg, dyna_reg_cfg->blt_mix_m1_n1);
    isp_bnr_blt_mix_m2_n1_write(be_reg, dyna_reg_cfg->blt_mix_m2_n1);
    isp_bnr_blt_clipl_write(be_reg, dyna_reg_cfg->isp_bnr_blt_clipl);
    isp_bnr_blt_cliph_write(be_reg, dyna_reg_cfg->isp_bnr_blt_cliph);
    isp_bnr_blt_weight_c1_write(be_reg, dyna_reg_cfg->isp_bnr_blt_weight_c1);
    isp_bnr_blt_weight_c_write(be_reg, dyna_reg_cfg->isp_bnr_blt_weight_c);

    isp_bnr_std_mix_thr_0_write(be_reg, dyna_reg_cfg->std_mix_thr_0);
    isp_bnr_std_mix_thr_1_write(be_reg, dyna_reg_cfg->std_mix_thr_1);
    isp_bnr_std_mix_thr_2_write(be_reg, dyna_reg_cfg->std_mix_thr_2);
    isp_bnr_std_mix_thr_3_write(be_reg, dyna_reg_cfg->std_mix_thr_3);
    isp_bnr_std_mix_thr_4_write(be_reg, dyna_reg_cfg->std_mix_thr_4);
    isp_bnr_std_mix_thr_5_write(be_reg, dyna_reg_cfg->std_mix_thr_5);
    isp_bnr_std_mix_thr_6_write(be_reg, dyna_reg_cfg->std_mix_thr_6);
    isp_bnr_std_mix_thr_7_write(be_reg, dyna_reg_cfg->std_mix_thr_7);
    isp_bnr_dgain_value_write(be_reg, dyna_reg_cfg->dgain_value);
    isp_bnr_dgain_slp_write(be_reg, dyna_reg_cfg->dgain_slp);
}

static td_void isp_bnr_online_lut2stt_info_check(td_bool offline_mode, isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_bnr_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        isp_bnr_stt2lut_regnew_write(be_reg, TD_TRUE);
        isp_bnr_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_bayer_nr_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_bayernr_static_cfg *static_reg_cfg = TD_NULL;
    isp_bayernr_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_bayernr_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(viproc_reg);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].bnr_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].bnr_reg_cfg.dyna_reg_cfg;
        if (static_reg_cfg->resh) { /* static */
            isp_bnr_en_write(viproc_reg, reg_cfg_info->alg_reg_cfg[i].bnr_reg_cfg.bnr_enable);
            isp_bnr_entmpnr_write(be_reg, dyna_reg_cfg->tnr_enable);
            isp_bnr_isinitial_write(be_reg, TD_TRUE);
            isp_bnr_jnlm_lum_sel_write(be_reg, static_reg_cfg->isp_bnr_jnlm_lum_sel);
            isp_bnr_bltev500_win_size_write(be_reg, static_reg_cfg->isp_bnr_bltev500_win_size);
            isp_bnr_en_mixing_write(be_reg, static_reg_cfg->mix_enable);
            static_reg_cfg->resh = TD_FALSE;
        }

        if (dyna_reg_cfg->resh) {
            isp_geq_en_write(viproc_reg, dyna_reg_cfg->geq_enable);
            isp_bnr_dyna_reg_config(be_reg, dyna_reg_cfg);
            isp_bnr_lut_reg_config(vi_pipe, be_reg, dyna_reg_cfg, i);
            dyna_reg_cfg->resh = offline_mode;
        }

        isp_bnr_online_lut2stt_info_check(offline_mode, be_reg);

        reg_cfg_info->cfg_key.bit1_bayernr_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_s32 isp_dg_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_dg_static_cfg *static_reg_cfg = TD_NULL;
    isp_dg_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_viproc_reg_type *viproc_reg = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_dg_cfg) {
        be_reg = (isp_post_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        viproc_reg = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(viproc_reg);
        isp_dg_en_write(viproc_reg, reg_cfg_info->alg_reg_cfg[i].dg_reg_cfg.dg_en);

        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dg_reg_cfg.static_reg_cfg;
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].dg_reg_cfg.dyna_reg_cfg;

        if (static_reg_cfg->resh) {
            isp_dg_rgain_write(be_reg, dyna_reg_cfg->gain_r);
            isp_dg_grgain_write(be_reg, dyna_reg_cfg->gain_gr);
            isp_dg_gbgain_write(be_reg, dyna_reg_cfg->gain_gb);
            isp_dg_bgain_write(be_reg, dyna_reg_cfg->gain_b);

            static_reg_cfg->resh = TD_FALSE;
        }

        if (dyna_reg_cfg->resh) {
            isp_dg_clip_value_write(be_reg, dyna_reg_cfg->clip_value);
            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_dg_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_s32 isp_4dg_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u8 blk_num = reg_cfg_info->cfg_num;
    td_bool offline_mode;
    isp_4dg_static_cfg *static_reg_cfg = TD_NULL;
    isp_4dg_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_wdr_dg_cfg) {
        pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(pre_viproc);
        isp_check_pointer_return(be_reg);

        if (is_linear_mode(isp_ctx->sns_wdr_mode)) {
            isp_4dg_en_write(pre_viproc, reg_cfg_info->alg_reg_cfg[i].dg_reg_cfg.dg_en);
        } else {
            isp_4dg_en_write(pre_viproc, TD_TRUE);
        }

        /* static */
        static_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].four_dg_reg_cfg.static_reg_cfg;

        if (static_reg_cfg->resh) {
            isp_4dg0_rgain_write(be_reg, static_reg_cfg->gain_r0);
            isp_4dg0_grgain_write(be_reg, static_reg_cfg->gain_gr0);
            isp_4dg0_gbgain_write(be_reg, static_reg_cfg->gain_gb0);
            isp_4dg0_bgain_write(be_reg, static_reg_cfg->gain_b0);
            isp_4dg1_rgain_write(be_reg, static_reg_cfg->gain_r1);
            isp_4dg1_grgain_write(be_reg, static_reg_cfg->gain_gr1);
            isp_4dg1_gbgain_write(be_reg, static_reg_cfg->gain_gb1);
            isp_4dg1_bgain_write(be_reg, static_reg_cfg->gain_b1);
            isp_4dg2_rgain_write(be_reg, static_reg_cfg->gain_r2);
            isp_4dg2_grgain_write(be_reg, static_reg_cfg->gain_gr2);
            isp_4dg2_gbgain_write(be_reg, static_reg_cfg->gain_gb2);
            isp_4dg2_bgain_write(be_reg, static_reg_cfg->gain_b2);
            static_reg_cfg->resh = TD_FALSE;
        }

        /* dynamic */
        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].four_dg_reg_cfg.dyna_reg_cfg;

        if (dyna_reg_cfg->resh) {
            isp_4dg0_clip_value_write(be_reg, dyna_reg_cfg->clip_value0);
            isp_4dg1_clip_value_write(be_reg, dyna_reg_cfg->clip_value1);
            isp_4dg2_clip_value_write(be_reg, dyna_reg_cfg->clip_value2);

            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_wdr_dg_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_s32 isp_flick_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_flicker_dyna_cfg *dyna_reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_flicker_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);
        isp_check_pointer_return(pre_viproc);

        isp_flicker_en_write(pre_viproc, reg_cfg_info->alg_reg_cfg[i].flicker_reg_cfg.enable);

        dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].flicker_reg_cfg.dyna_reg_cfg;

        if (dyna_reg_cfg->resh) {
            isp_flick_overth_write(be_reg, dyna_reg_cfg->over_thr);
            isp_flick_gravg_pre_write(be_reg, dyna_reg_cfg->gr_avg_pre);
            isp_flick_gbavg_pre_write(be_reg, dyna_reg_cfg->gb_avg_pre);

            dyna_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_flicker_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_pre_be_blc_static_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_be_blc_static_cfg *static_cfg)
{
    isp_pre_be_reg_type *be_reg = TD_NULL;

    be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(be_reg);

    /* 4Dg */
    isp_4dg_en_in_write(be_reg, static_cfg->wdr_dg_blc[0].blc_in);
    isp_4dg_en_out_write(be_reg, static_cfg->wdr_dg_blc[0].blc_out);

    /* ge */
    isp_ge0_blc_offset_en_write(be_reg, static_cfg->wdr_dg_blc[0].blc_in);
    isp_ge1_blc_offset_en_write(be_reg, static_cfg->wdr_dg_blc[1].blc_in);

    /* WDR */
    isp_wdr_bsaveblc_write(be_reg, static_cfg->wdr_blc[0].blc_out);
}

static td_void isp_post_be_blc_static_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_be_blc_static_cfg *static_cfg)
{
    isp_post_be_reg_type *be_reg = TD_NULL;

    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(be_reg);

    /* lsc */
    isp_lsc_blc_in_en_write(be_reg, static_cfg->lsc_blc.blc_in);
    isp_lsc_blc_out_en_write(be_reg, static_cfg->lsc_blc.blc_out);

    /* Dg */
    isp_dg_en_in_write(be_reg, static_cfg->dg_blc.blc_in);
    isp_dg_en_out_write(be_reg, static_cfg->dg_blc.blc_out);
    isp_dg_en_comp_write(be_reg, static_cfg->dg_blc.blc_compensation_en);

    /* AE */
    isp_ae_blc_en_write(be_reg, static_cfg->ae_blc.blc_in);
    isp_ae_offset_r_write(be_reg, 0);
    isp_ae_offset_gr_write(be_reg, 0);
    isp_ae_offset_gb_write(be_reg, 0);
    isp_ae_offset_b_write(be_reg, 0);

    /* MG */
    isp_la_blc_en_write(be_reg, static_cfg->mg_blc.blc_in);
    isp_la_offset_r_write(be_reg, 0);
    isp_la_offset_gr_write(be_reg, 0);
    isp_la_offset_gb_write(be_reg, 0);
    isp_la_offset_b_write(be_reg, 0);

    /* WB */
    isp_wb_en_in_write(be_reg, static_cfg->wb_blc.blc_in);
    isp_wb_en_out_write(be_reg, static_cfg->wb_blc.blc_out);
    /* AF */
    isp_af_offset_en_write(be_reg, static_cfg->af_blc.blc_in);
    isp_af_offset_gr_write(be_reg, 0);
    isp_af_offset_gb_write(be_reg, 0);
}

static td_void isp_4dg_blc_dyna_reg_config(isp_pre_be_reg_type *be_reg, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_4dg0_ofsr_write(be_reg, dyna_cfg->wdr_dg_blc[0].blc[0]);  /* index 0 ,0 */
    isp_4dg0_ofsgr_write(be_reg, dyna_cfg->wdr_dg_blc[0].blc[1]); /* index 0 ,1 */
    isp_4dg0_ofsgb_write(be_reg, dyna_cfg->wdr_dg_blc[0].blc[2]); /* index 0 ,2 */
    isp_4dg0_ofsb_write(be_reg, dyna_cfg->wdr_dg_blc[0].blc[3]);  /* index 0 ,3 */

    isp_4dg1_ofsr_write(be_reg, dyna_cfg->wdr_dg_blc[1].blc[0]);  /* index 1 ,0 */
    isp_4dg1_ofsgr_write(be_reg, dyna_cfg->wdr_dg_blc[1].blc[1]); /* index 1 ,1 */
    isp_4dg1_ofsgb_write(be_reg, dyna_cfg->wdr_dg_blc[1].blc[2]); /* index 1 ,2 */
    isp_4dg1_ofsb_write(be_reg, dyna_cfg->wdr_dg_blc[1].blc[3]);  /* index 1 ,3 */

    isp_4dg2_ofsr_write(be_reg, dyna_cfg->wdr_dg_blc[2].blc[0]);  /* index 2 ,0 */
    isp_4dg2_ofsgr_write(be_reg, dyna_cfg->wdr_dg_blc[2].blc[1]); /* index 2 ,1 */
    isp_4dg2_ofsgb_write(be_reg, dyna_cfg->wdr_dg_blc[2].blc[2]); /* index 2 ,2 */
    isp_4dg2_ofsb_write(be_reg, dyna_cfg->wdr_dg_blc[2].blc[3]);  /* index 2 ,3 */

    isp_4dg3_ofsr_write(be_reg, dyna_cfg->wdr_dg_blc[3].blc[0]);  /* index 3 ,0 */
    isp_4dg3_ofsgr_write(be_reg, dyna_cfg->wdr_dg_blc[3].blc[1]); /* index 3 ,1 */
    isp_4dg3_ofsgb_write(be_reg, dyna_cfg->wdr_dg_blc[3].blc[2]); /* index 3 ,2 */
    isp_4dg3_ofsb_write(be_reg, dyna_cfg->wdr_dg_blc[3].blc[3]);  /* index 3 ,3 */
}

static td_void isp_ge_blc_dyna_reg_config(isp_pre_be_reg_type *be_reg, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_ge0_blc_offset_gr_write(be_reg, dyna_cfg->ge_blc[0].blc[1]);     /* index 0 ,1 */
    isp_ge0_blc_offset_gb_write(be_reg, dyna_cfg->ge_blc[0].blc[2]);     /* index 0 ,2 */
    isp_ge1_blc_offset_gr_write(be_reg, dyna_cfg->ge_blc[1].blc[1]);     /* index 1 ,1 */
    isp_ge1_blc_offset_gb_write(be_reg, dyna_cfg->ge_blc[1].blc[2]);     /* index 1 ,2 */
}

static td_void isp_wdr_blc_dyna_reg_config(isp_pre_be_reg_type *be_reg, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_wdr_f0_inblc_r_write(be_reg, dyna_cfg->wdr_blc[0].blc[0]);  /* index 0 ,0 */
    isp_wdr_f0_inblc_gr_write(be_reg, dyna_cfg->wdr_blc[0].blc[1]); /* index 0 ,1 */
    isp_wdr_f0_inblc_gb_write(be_reg, dyna_cfg->wdr_blc[0].blc[2]); /* index 0 ,2 */
    isp_wdr_f0_inblc_b_write(be_reg, dyna_cfg->wdr_blc[0].blc[3]);  /* index 0 ,3 */

    isp_wdr_f1_inblc_r_write(be_reg, dyna_cfg->wdr_blc[1].blc[0]);  /* index 1 ,0 */
    isp_wdr_f1_inblc_gr_write(be_reg, dyna_cfg->wdr_blc[1].blc[1]); /* index 1 ,1 */
    isp_wdr_f1_inblc_gb_write(be_reg, dyna_cfg->wdr_blc[1].blc[2]); /* index 1 ,2 */
    isp_wdr_f1_inblc_b_write(be_reg, dyna_cfg->wdr_blc[1].blc[3]);  /* index 1 ,3 */
}

static td_void isp_flicker_blc_dyna_reg_config(isp_pre_be_reg_type *be_reg, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_flick_f0_inblc_r_write(be_reg, dyna_cfg->flicker_blc[0].blc[0]);  /* index 0 ,0 */
    isp_flick_f0_inblc_gr_write(be_reg, dyna_cfg->flicker_blc[0].blc[1]); /* index 0 ,1 */
    isp_flick_f0_inblc_gb_write(be_reg, dyna_cfg->flicker_blc[0].blc[2]); /* index 0 ,2 */
    isp_flick_f0_inblc_b_write(be_reg, dyna_cfg->flicker_blc[0].blc[3]);  /* index 0 ,3 */

    isp_flick_f1_inblc_r_write(be_reg, dyna_cfg->flicker_blc[1].blc[0]);  /* index 1 ,0 */
    isp_flick_f1_inblc_gr_write(be_reg, dyna_cfg->flicker_blc[1].blc[1]); /* index 1 ,1 */
    isp_flick_f1_inblc_gb_write(be_reg, dyna_cfg->flicker_blc[1].blc[2]); /* index 1 ,2 */
    isp_flick_f1_inblc_b_write(be_reg, dyna_cfg->flicker_blc[1].blc[3]);  /* index 1 ,3 */
}

static td_void isp_pre_be_blc_dyna_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_pre_be_reg_type *be_reg = TD_NULL;

    be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(be_reg);

    isp_be_glb_blc_mode_write(be_reg, dyna_cfg->global_blc.blc_mode);
    isp_be_glb_tg_mode_write(be_reg, dyna_cfg->global_blc.target_mode);
    isp_be_global_blc0_write(be_reg, dyna_cfg->global_blc.global_blc0[0]);
    isp_be_global_blc1_write(be_reg, dyna_cfg->global_blc.global_blc1[0]);
    isp_be_target_black_level(be_reg, dyna_cfg->global_blc.target_blc[0]);

    isp_4dg_blc_dyna_reg_config(be_reg, dyna_cfg);     /* 4Dg */
    isp_ge_blc_dyna_reg_config(be_reg, dyna_cfg);      /* ge */
    isp_wdr_blc_dyna_reg_config(be_reg, dyna_cfg);     /* WDR */
    isp_flicker_blc_dyna_reg_config(be_reg, dyna_cfg); /* flicker */

    /* expander */
    isp_expander_offset_r_write(be_reg, dyna_cfg->expander_blc.blc[0]);  /* array index 0 */
    isp_expander_offset_gr_write(be_reg, dyna_cfg->expander_blc.blc[1]); /* array index 1 */
    isp_expander_offset_gb_write(be_reg, dyna_cfg->expander_blc.blc[2]); /* array index 2 */
    isp_expander_offset_b_write(be_reg, dyna_cfg->expander_blc.blc[3]);  /* array index 3 */
}

static td_void isp_post_be_blc_dyna_reg_config(ot_vi_pipe vi_pipe, td_u8 i, isp_be_blc_dyna_cfg *dyna_cfg)
{
    isp_post_be_reg_type *be_reg = TD_NULL;

    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(be_reg);

    /* lsc */
    isp_lsc_blc_r_write(be_reg, dyna_cfg->lsc_blc.blc[0]);  /* array index 0 */
    isp_lsc_blc_gr_write(be_reg, dyna_cfg->lsc_blc.blc[1]); /* array index 1 */
    isp_lsc_blc_gb_write(be_reg, dyna_cfg->lsc_blc.blc[2]); /* array index 2 */
    isp_lsc_blc_b_write(be_reg, dyna_cfg->lsc_blc.blc[3]);  /* array index 3 */

    /* Dg */
    isp_dg_ofsr_write(be_reg, dyna_cfg->dg_blc.blc[0]);  /* array index 0 */
    isp_dg_ofsgr_write(be_reg, dyna_cfg->dg_blc.blc[1]); /* array index 1 */
    isp_dg_ofsgb_write(be_reg, dyna_cfg->dg_blc.blc[2]); /* array index 2 */
    isp_dg_ofsb_write(be_reg, dyna_cfg->dg_blc.blc[3]);  /* array index 3 */
    isp_dg_real_offset_write(be_reg, dyna_cfg->dg_real_blc.blc);

    /* WB */
    isp_wb_ofsr_write(be_reg, dyna_cfg->wb_blc.blc[0]);  /* array index 0 */
    isp_wb_ofsgr_write(be_reg, dyna_cfg->wb_blc.blc[1]); /* array index 1 */
    isp_wb_ofsgb_write(be_reg, dyna_cfg->wb_blc.blc[2]); /* array index 2 */
    isp_wb_ofsb_write(be_reg, dyna_cfg->wb_blc.blc[3]);  /* array index 3 */
}

static td_void isp_be_blc_bit_shift_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool wdr_en = reg_cfg_info->alg_reg_cfg[i].wdr_reg_cfg.wdr_en;
    isp_pre_be_reg_type *be_reg = TD_NULL;

    be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(be_reg);

    if (wdr_en == TD_TRUE) {
        isp_dg_ofs_shift_write(be_reg, ISP_BLACK_LEVEL_RIGHT_SHIFT_BIT_WDR);
        isp_lsc_blc_right_shift_write(be_reg, ISP_BLACK_LEVEL_RIGHT_SHIFT_BIT_WDR);
    } else {
        isp_dg_ofs_shift_write(be_reg, 0);
        isp_lsc_blc_right_shift_write(be_reg, 0);
    }
}

static td_s32 isp_be_blc_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_be_blc_static_cfg *static_cfg = TD_NULL;
    isp_be_blc_dyna_cfg *dyna_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_be_blc_cfg) {
        static_cfg = &reg_cfg_info->alg_reg_cfg[i].be_blc_cfg.static_blc;
        dyna_cfg = &reg_cfg_info->alg_reg_cfg[i].be_blc_cfg.dyna_blc;

        if (static_cfg->resh_static) {
            isp_pre_be_blc_static_reg_config(vi_pipe, i, static_cfg);
            isp_post_be_blc_static_reg_config(vi_pipe, i, static_cfg);
            static_cfg->resh_static = offline_mode;
        }

        isp_be_blc_bit_shift_reg_config(vi_pipe, reg_cfg_info, i);

        if (reg_cfg_info->alg_reg_cfg[i].be_blc_cfg.resh_dyna_init == TD_TRUE) {
            isp_pre_be_blc_dyna_reg_config(vi_pipe, i, dyna_cfg);
            isp_post_be_blc_dyna_reg_config(vi_pipe, i, dyna_cfg);
            reg_cfg_info->alg_reg_cfg[i].be_blc_cfg.resh_dyna_init = TD_FALSE;
        }

        reg_cfg_info->cfg_key.bit1_be_blc_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_void isp_clut_online_lut2stt_info_check(td_bool offline_mode, td_bool *stt2_lut_regnew,
    isp_post_be_reg_type *be_reg)
{
    td_u16 lut2stt_info;
    if (offline_mode == TD_TRUE) {
        return;
    }

    lut2stt_info = isp_clut_stt2lut_info_read(be_reg);
    if (lut2stt_info != 0) {
        *stt2_lut_regnew = TD_TRUE;
        isp_clut_stt2lut_clr_write(be_reg, 1);
    }
}

static td_s32 isp_clut_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode, usr_resh, idx_resh;
    td_bool stt2_lut_regnew = TD_FALSE;
    td_u8 buf_id;
    td_u8 blk_num = reg_cfg_info->cfg_num;
    isp_clut_usr_coef_cfg *clut_usr_coef_cfg = TD_NULL;
    isp_be_lut_wstt_type *be_lut_stt_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode =
        (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode));
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);
    if ((reg_cfg_info->cfg_key.bit1_clut_cfg)) {
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        isp_check_pointer_return(post_viproc);

        isp_clut_sel_write(be_reg, OT_ISP_CLUT_SEL_WRITE);
        clut_usr_coef_cfg = &reg_cfg_info->alg_reg_cfg[i].clut_cfg.usr_reg_cfg.clut_usr_coef_cfg;

        idx_resh = (isp_clut_update_index_read(be_reg) != clut_usr_coef_cfg->update_index);
        usr_resh = (offline_mode) ? (clut_usr_coef_cfg->resh && idx_resh) : (clut_usr_coef_cfg->resh);

        if (usr_resh) {
            isp_clut_update_index_write(be_reg, clut_usr_coef_cfg->update_index);

            if (is_online_mode(isp_ctx->block_attr.running_mode)) {
                /* online lut2stt regconfig */
                buf_id = clut_usr_coef_cfg->buf_id;

                be_lut_stt_reg = (isp_be_lut_wstt_type *)isp_get_be_lut2stt_vir_addr(vi_pipe, i, buf_id);
                isp_check_pointer_return(be_lut_stt_reg);

                isp_clut_lut_wstt_write(&be_lut_stt_reg->post_be_lut2stt, clut_usr_coef_cfg->lu_stt);

                isp_clut_lut_wstt_addr_write(vi_pipe, i, buf_id, post_viproc);
                isp_clut_stt2lut_en_write(be_reg, TD_TRUE);

                clut_usr_coef_cfg->buf_id = 1 - buf_id;
                stt2_lut_regnew = TD_TRUE;
            } else {
                isp_clut_lut_wstt_write(&be_reg->be_lut.be_lut2stt.post_be_lut2stt, clut_usr_coef_cfg->lu_stt);
                isp_clut_lut_width_word_write(post_viproc, OT_ISP_CLUT_LUT_WIDTH_WORD_DEFAULT);
                isp_clut_stt2lut_en_write(be_reg, TD_TRUE);
                isp_clut_stt2lut_regnew_write(be_reg, TD_TRUE);
            }

            clut_usr_coef_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_clut_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    isp_clut_online_lut2stt_info_check(offline_mode, &stt2_lut_regnew, be_reg);
    reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg.clut_stt2lut_regnew = stt2_lut_regnew;

    return TD_SUCCESS;
}

static td_void isp_expander_lut_reg_config(const isp_usr_ctx *isp_ctx, isp_pre_be_reg_type *be_reg,
    isp_expander_usr_cfg *usr_reg_cfg)
{
    td_u16 j;

    if (is_online_mode(isp_ctx->block_attr.running_mode) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        isp_expander_lut_waddr_write(be_reg, 0);

        for (j = 0; j < OT_ISP_EXPANDER_NODE_NUM; j++) {
            isp_expander_lut_wdata_write(be_reg, usr_reg_cfg->lut[j]);
        }
    } else {
        isp_expander_lut_write(&be_reg->be_lut.be_apb_lut, usr_reg_cfg->lut);
    }
}

static td_s32 isp_expander_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool offline_mode;
    td_bool usr_resh, idx_resh;
    td_u8 blk_num = reg_cfg_info->cfg_num;

    isp_expander_usr_cfg *usr_reg_cfg = TD_NULL;
    isp_pre_be_reg_type *be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    offline_mode = (is_offline_mode(isp_ctx->block_attr.running_mode) ||
                    is_striping_mode(isp_ctx->block_attr.running_mode));

    if (reg_cfg_info->cfg_key.bit1_expander_cfg) {
        be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        isp_check_pointer_return(be_reg);

        usr_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].expander_cfg.usr_cfg;

        idx_resh = (isp_expander_update_index_read(be_reg) != usr_reg_cfg->update_index);
        usr_resh = (offline_mode) ? (usr_reg_cfg->resh && idx_resh) : (usr_reg_cfg->resh);

        if (usr_resh) {
            isp_expander_update_index_write(be_reg, usr_reg_cfg->update_index);
            isp_expander_bitw_out_write(be_reg, usr_reg_cfg->bit_depth_out);
            isp_expander_bitw_in_write(be_reg, usr_reg_cfg->bit_depth_in);

            isp_expander_lut_reg_config(isp_ctx, be_reg, usr_reg_cfg);

            usr_reg_cfg->resh = offline_mode;
        }

        reg_cfg_info->cfg_key.bit1_expander_cfg = offline_mode ? 1 : ((blk_num <= (i + 1)) ? 0 : 1);
    }

    return TD_SUCCESS;
}

static td_s32 isp_fe_lblc_stt2lut_regnew_config(ot_vi_pipe vi_pipe, isp_fe_reg_type *fe_reg, td_bool lblc_lut_update)
{
    td_u8 local_blc_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_LOCAL_BLC_ENABLE };

    if (local_blc_en[vi_pipe] != 1) {
        return TD_SUCCESS;
    }

    if (lblc_lut_update == TD_TRUE) {
        isp_lblc_stt2lut_regnew_write(fe_reg, TD_TRUE);
    }

    return TD_FALSE;
}

static td_s32 isp_fe_update_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u32 i;
    td_bool reg_new;
    ot_vi_pipe vi_pipe_bind;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        isp_fe_update_mode_write(fe_reg, TD_FALSE);
        isp_fe_update_write(fe_reg, TD_TRUE);

        if (reg_cfg_info->alg_reg_cfg[0].fe_lut_update_cfg.ae1_lut_update) {
            isp_fe_ae_lut_update_write(fe_reg, reg_cfg_info->alg_reg_cfg[0].fe_lut_update_cfg.ae1_lut_update);
        }
        reg_new = reg_cfg_info->alg_reg_cfg[0].fe_lut_update_cfg.lblc_lut_update;
        isp_fe_lblc_stt2lut_regnew_config(vi_pipe_bind, fe_reg, reg_new);
    }

    return TD_SUCCESS;
}

static td_void isp_fe_size_reg_config(const isp_usr_ctx *isp_ctx, isp_fe_reg_type *fe_reg)
{
    td_bool isp_crop_en;
    td_s32 x, y;
    td_u32 width, height, pipe_w, pipe_h;
    td_u32 merge_frame;

    x = isp_ctx->sys_rect.x;
    y = isp_ctx->sys_rect.y;
    width = isp_ctx->sys_rect.width;
    height = isp_ctx->sys_rect.height;
    pipe_w = isp_ctx->pipe_size.width;
    pipe_h = isp_ctx->pipe_size.height;

    /* ISP crop low-power process */
    if ((x == 0) && (y == 0) && (width == pipe_w) && (height == pipe_h)) {
        isp_crop_en = TD_FALSE;
    } else {
        isp_crop_en = TD_TRUE;
    }

    isp_fe_crop_pos_write(fe_reg, x, y);
    isp_fe_crop_size_out_write(fe_reg, width - 1, height - 1);

    if (isp_ctx->stagger_attr.stagger_en == TD_TRUE) {
        if (isp_ctx->stagger_attr.crop_info.enable == TD_TRUE) {
            isp_fe_crop_en_write(fe_reg, TD_TRUE);
            isp_fe_crop_pos_write(fe_reg, isp_ctx->stagger_attr.crop_info.rect.x,
                                  isp_ctx->stagger_attr.crop_info.rect.y);
            isp_fe_crop_size_out_write(fe_reg, isp_ctx->stagger_attr.crop_info.rect.width - 1,
                                       isp_ctx->stagger_attr.crop_info.rect.height - 1);
        } else {
            isp_fe_crop_en_write(fe_reg, TD_FALSE);
        }
        merge_frame = div_0_to_1(isp_ctx->stagger_attr.merge_frame_num);
        isp_mul_u32_limit(width, pipe_w, merge_frame);
        height = (pipe_h + (merge_frame - 1)) / div_0_to_1(merge_frame);
        isp_fe_size_write(fe_reg, width - 1, height - 1);
        isp_fe_blk_size_write(fe_reg, width - 1, height - 1);
    } else {
        isp_fe_crop_en_write(fe_reg, isp_crop_en);
        isp_fe_size_write(fe_reg, pipe_w - 1, pipe_h - 1);
        isp_fe_blk_size_write(fe_reg, pipe_w - 1, pipe_h - 1);
    }

    isp_fe_delay_write(fe_reg, height >> 1); /* set fe delay interrup trigger threshold */
}

static td_s32 isp_fe_system_reg_config(ot_vi_pipe vi_pipe)
{
    td_u8 rggb_cfg;
    td_u32 i;
    ot_vi_pipe vi_pipe_bind;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    rggb_cfg = ot_ext_system_rggb_cfg_read(vi_pipe);

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        /* ISP FE/BE Set Offline Mode */
        /* isp regs uptate mode:   0: update; 1:frame */
        isp_fe_rggb_cfg_write(fe_reg, rggb_cfg);
        isp_fe_fix_timing_write(fe_reg, OT_ISP_FE_FIX_TIMING_STAT);
        isp_fe_blk_f_hblank_write(fe_reg, 0);
        isp_fe_hsync_mode_write(fe_reg, 0);
        isp_fe_vsync_mode_write(fe_reg, 0);

        isp_fe_size_reg_config(isp_ctx, fe_reg);
    }

    return TD_SUCCESS;
}

static td_s32 isp_fe_roi_reg_config(ot_vi_pipe vi_pipe)
{
    td_u8 local_blc_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_LOCAL_BLC_ENABLE };
    td_u8 aibnr_fpn_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_AIBNR_FPN_ENABLE };
    td_u8 fe_nr_dgain_en[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FE_NR_DGAIN_ENABLE };
    td_u32 i;
    ot_isp_fe_roi_cfg *fe_roi_cfg = TD_NULL;
    ot_vi_pipe vi_pipe_bind;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    fe_roi_cfg = &isp_ctx->fe_roi_cfg;
    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);
        isp_fe_blc1_roi_en_write(fe_reg, fe_roi_cfg->enable);
        isp_fe_blc1_roi_pos_write(fe_reg, fe_roi_cfg->roi_rect.x, fe_roi_cfg->roi_rect.y);
        isp_fe_blc1_roi_size_write(fe_reg, fe_roi_cfg->roi_rect.width - 1, fe_roi_cfg->roi_rect.height - 1);
        if (local_blc_en[vi_pipe_bind] == 1) {
            isp_lblc_roi_en_write(fe_reg, fe_roi_cfg->enable);
            isp_lblc_roi_pos_write(fe_reg, fe_roi_cfg->roi_rect.x, fe_roi_cfg->roi_rect.y);
            isp_lblc_roi_size_write(fe_reg, fe_roi_cfg->roi_rect.width - 1, fe_roi_cfg->roi_rect.height - 1);
        }
        if (aibnr_fpn_en[vi_pipe_bind] == 1) {
            isp_aibnr_fpn_roi_en_write(fe_reg, fe_roi_cfg->enable);
            isp_aibnr_fpn_roi_pos_write(fe_reg, fe_roi_cfg->roi_rect.x, fe_roi_cfg->roi_rect.y);
            isp_aibnr_fpn_roi_size_write(fe_reg, fe_roi_cfg->roi_rect.width - 1, fe_roi_cfg->roi_rect.height - 1);
        }
        if (fe_nr_dgain_en[vi_pipe_bind] == 1) {
            isp_fe_dg1_roi_en_write(fe_reg, fe_roi_cfg->enable);
            isp_fe_dg1_roi_pos_write(fe_reg, fe_roi_cfg->roi_rect.x, fe_roi_cfg->roi_rect.y);
            isp_fe_dg1_roi_size_write(fe_reg, fe_roi_cfg->roi_rect.width - 1, fe_roi_cfg->roi_rect.height - 1);
        }
        isp_fe_dg2_roi_en_write(fe_reg, fe_roi_cfg->enable);
        isp_fe_dg2_roi_pos_write(fe_reg, fe_roi_cfg->roi_rect.x, fe_roi_cfg->roi_rect.y);
        isp_fe_dg2_roi_size_write(fe_reg, fe_roi_cfg->roi_rect.width - 1, fe_roi_cfg->roi_rect.height - 1);
    }

    return TD_SUCCESS;
}

static td_s32 isp_reg_default(ot_vi_pipe vi_pipe, const isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    ot_unused(reg_cfg_info);

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(pre_be_reg);
    isp_check_pointer_return(post_viproc);

    /* pre be */
    isp_clip_y_min_write(pre_be_reg, ISP_CLIP_Y_MIN_DEFAULT);
    isp_clip_y_max_write(pre_be_reg, ISP_CLIP_Y_MAX_DEFAULT);
    isp_clip_c_min_write(pre_be_reg, ISP_CLIP_C_MIN_DEFAULT);
    isp_clip_c_max_write(pre_be_reg, ISP_CLIP_C_MAX_DEFAULT);
    isp_blk_f_hblank_write(pre_be_reg, OT_ISP_BLK_F_HBLANK_DEFAULT);
    isp_blk_f_vblank_write(pre_be_reg, OT_ISP_BLK_F_VBLANK_DEFAULT);
    isp_blk_b_hblank_write(pre_be_reg, OT_ISP_BLK_B_HBLANK_DEFAULT);
    isp_blk_b_vblank_write(pre_be_reg, OT_ISP_BLK_B_VBLANK_DEFAULT);

    /* post be */
    isp_clip_y_min_write(post_be_reg, ISP_CLIP_Y_MIN_DEFAULT);
    isp_clip_y_max_write(post_be_reg, ISP_CLIP_Y_MAX_DEFAULT);
    isp_clip_c_min_write(post_be_reg, ISP_CLIP_C_MIN_DEFAULT);
    isp_clip_c_max_write(post_be_reg, ISP_CLIP_C_MAX_DEFAULT);
    isp_blk_f_hblank_write(post_be_reg, OT_ISP_BLK_F_HBLANK_DEFAULT);
    isp_blk_f_vblank_write(post_be_reg, OT_ISP_BLK_F_VBLANK_DEFAULT);
    isp_blk_b_hblank_write(post_be_reg, OT_ISP_BLK_B_HBLANK_DEFAULT);
    isp_blk_b_vblank_write(post_be_reg, OT_ISP_BLK_B_VBLANK_DEFAULT);

    isp_blc_en_write(post_viproc, TD_FALSE);
    isp_split_en_write(post_viproc, TD_FALSE);

    return TD_SUCCESS;
}

static td_s32 isp_system_reg_config(ot_vi_pipe vi_pipe, td_u8 i)
{
    td_u32 rggb_cfg;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    rggb_cfg = ot_ext_system_rggb_cfg_read(vi_pipe);

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(pre_be_reg);
    isp_check_pointer_return(post_viproc);
    isp_check_pointer_return(pre_viproc);

    isp_be_rggb_cfg_write(post_viproc, rggb_cfg);
    isp_be_rggb_cfg_write(pre_viproc, rggb_cfg);

    if ((is_offline_mode(isp_ctx->block_attr.running_mode)) || (is_striping_mode(isp_ctx->block_attr.running_mode))) {
        isp_be_stt_en_write(post_be_reg, TD_TRUE);
        isp_be_stt_en_write(pre_be_reg, TD_TRUE);
    } else if (is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        isp_be_stt_en_write(post_be_reg, TD_TRUE);
        isp_be_stt_en_write(pre_be_reg, TD_FALSE); // todo pre_online_post_offline
    } else {
        isp_be_stt_en_write(post_be_reg, TD_FALSE);
        isp_be_stt_en_write(pre_be_reg, TD_FALSE);
    }

    isp_sumy_en_write(pre_viproc, TD_TRUE);

    isp_sumy_en_write(post_viproc, TD_TRUE);

    return TD_SUCCESS;
}

static td_s32 isp_dither_reg_config(ot_vi_pipe vi_pipe, const isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_bool binary_en;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_post_be_reg_type *be_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);

    binary_en = ot_ext_system_sys_binary_en_read(vi_pipe);
    if (binary_en == TD_TRUE) {
        ot_unused(reg_cfg_info);
        ot_unused(isp_ctx);
        isp_drc_dither_en_write(be_reg, TD_FALSE);
        isp_dmnr_dither_en_write(be_reg, TD_FALSE);
        isp_sharpen_dither_en_write(be_reg, TD_FALSE);
    } else {
        /* after drc module */
        if (reg_cfg_info->alg_reg_cfg[i].drc_reg_cfg.enable == TD_TRUE) {
            isp_drc_dither_en_write(be_reg, TD_FALSE);
        } else {
            isp_drc_dither_en_write(be_reg, !(isp_ctx->hdr_attr.dynamic_range == OT_DYNAMIC_RANGE_XDR));
        }
        isp_drc_dither_out_bits_write(be_reg, ISP_DRC_DITHER_OUT_BITS_DEFAULT);
        isp_drc_dither_round_write(be_reg, ISP_DRC_DITHER_ROUND_DEFAULT);
        isp_drc_dither_spatial_mode_write(be_reg, ISP_DRC_DITHER_SPATIAL_MODE_DEFAULT);

        /* after gamma module */
        isp_dmnr_dither_en_write(be_reg, TD_TRUE);
        isp_dmnr_dither_out_bits_write(be_reg, ISP_DMNR_DITHER_OUT_BITS_DEFAULT);
        isp_dmnr_dither_round_write(be_reg, ISP_DMNR_DITHER_ROUND_DEFAULT);
        isp_dmnr_dither_spatial_mode_write(be_reg, ISP_DMNR_DITHER_SPATIAL_MODE_DEFAULT);

        /* after sharpen module, should delete */
        isp_sharpen_dither_en_write(be_reg, TD_FALSE);
        isp_sharpen_dither_out_bits_write(be_reg, ISP_SHARPEN_DITHER_OUT_BITS_DEFAULT);
        isp_sharpen_dither_round_write(be_reg, ISP_SHARPEN_DITHER_ROUND_DEFAULT);
        isp_sharpen_dither_spatial_mode_write(be_reg, ISP_SHARPEN_DITHER_SPATIAL_MODE_DEFAULT);

        isp_sharpen_ldci_dither_round_write(be_reg, ISP_SHARPEN_LDCI_DITHER_ROUND_DEFAULT);
    }

    return TD_SUCCESS;
}

static td_s32 isp_fe_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u8 af_enbale[OT_ISP_MAX_FE_PIPE_NUM] = { ISP_PIPE_FEAF_MODULE_ENABLE };
    td_u16 num_h, num_v;
    td_u32 k;
    td_bool stt_enable = TD_TRUE;
    ot_vi_pipe vi_pipe_bind;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    for (k = 0; k < isp_ctx->wdr_attr.pipe_num; k++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[k];
        isp_check_no_fe_pipe_return(vi_pipe_bind);
        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_return(fe_reg);

        if (isp_ctx->special_opt.fe_stt_update) {
            isp_fe_ae_stt_en_write(fe_reg, stt_enable);
            isp_fe_ae_stt_bst_write(fe_reg, 0xF);
            if (af_enbale[vi_pipe_bind]) {
                isp_fe_af_stt_en_write(fe_reg, stt_enable);
                isp_fe_af_stt_bst_write(fe_reg, 0xF);
            }
        }

        /* ae */
        num_h = reg_cfg_info->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg.fe_weight_table_width;
        num_v = reg_cfg_info->alg_reg_cfg[0].ae_reg_cfg.dyna_reg_cfg.fe_weight_table_height;
        isp_fe_ae_stt_size_write(fe_reg, (num_h * num_v + 3) / 4); /* plus 3 divide 4 is byte align */

        /* AF */
        num_h = reg_cfg_info->alg_reg_cfg[0].fe_af_reg_cfg.window_hnum;
        num_v = reg_cfg_info->alg_reg_cfg[0].fe_af_reg_cfg.window_vnum;
        isp_fe_af_stt_size_write(fe_reg, (num_h * num_v * 4 + 3) / 4); /* plus 3 divide 4 is byte align */
    }

    isp_ctx->special_opt.fe_stt_update = TD_FALSE;

    return TD_SUCCESS;
}

static td_void isp_ae_online_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u16 num_h, num_v;
    isp_ae_dyna_cfg *dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].ae_reg_cfg.dyna_reg_cfg;
    isp_be_reg_type *be_reg = TD_NULL;

    if (dyna_reg_cfg->is_online == TD_FALSE) {
        return;
    }

    if (dyna_reg_cfg->is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    }

    isp_check_pointer_void_return(be_reg);

    isp_ae_stt_en_write(be_reg, TD_TRUE);
    isp_ae_stt_bst_write(be_reg, 0xF);
    num_h = dyna_reg_cfg->be_weight_table_width;
    num_v = dyna_reg_cfg->be_weight_table_height;
    isp_ae_stt_size_write(be_reg, (num_h * num_v + 3) / 4); /* plus 3 divide 4 is byte align */
}

static td_void isp_la_online_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u16 num_h, num_v;
    isp_mg_dyna_cfg *dyna_reg_cfg = &reg_cfg_info->alg_reg_cfg[i].mg_reg_cfg.dyna_reg_cfg;
    isp_be_reg_type *be_reg = TD_NULL;

    if (dyna_reg_cfg->is_online == TD_FALSE) {
        return;
    }

    if (dyna_reg_cfg->is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    }

    isp_check_pointer_void_return(be_reg);

    isp_la_stt_en_write(be_reg, TD_TRUE);
    isp_la_stt_bst_write(be_reg, 0xF);
    num_h = dyna_reg_cfg->zone_width;
    num_v = dyna_reg_cfg->zone_height;
    isp_la_stt_size_write(be_reg, (num_h * num_v + 3) / 4); /* plus 3 divide 4 is byte align */
}

static td_void isp_awb_online_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u16 num_h, num_v;
    isp_be_reg_type *be_reg = TD_NULL;

    if (reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg.is_online == TD_FALSE) {
        return;
    }

    if (reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg.awb_reg_dyn_cfg.is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    }

    isp_check_pointer_void_return(be_reg);

    isp_awb_stt_en_write(be_reg, TD_TRUE);
    isp_awb_stt_bst_write(be_reg, 0xF);
    num_h = reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg.awb_reg_usr_cfg.be_zone_col;
    num_v = reg_cfg_info->alg_reg_cfg[i].awb_reg_cfg.awb_reg_usr_cfg.be_zone_row;
    isp_awb_stt_size_write(be_reg, (num_h * num_v * 2 + 3) / 4); /* 2 plus 3 divide 4 is byte align */
}

static td_void isp_af_online_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u16 num_h, num_v;
    isp_be_reg_type *be_reg = TD_NULL;

    if (reg_cfg_info->alg_reg_cfg[i].be_af_reg_cfg.be_is_online == TD_FALSE) {
        return;
    }

    if (reg_cfg_info->alg_reg_cfg[i].be_af_reg_cfg.is_in_pre_be == TD_TRUE) {
        be_reg = (isp_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    } else {
        be_reg = (isp_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    }

    isp_check_pointer_void_return(be_reg);

    isp_af_stt_en_write(be_reg, TD_TRUE);
    isp_af_stt_bst_write(be_reg, 0xF);
    num_h = reg_cfg_info->alg_reg_cfg[i].be_af_reg_cfg.window_hnum;
    num_v = reg_cfg_info->alg_reg_cfg[i].be_af_reg_cfg.window_vnum;
    isp_af_stt_size_write(be_reg, (num_h * num_v * 4 + 3) / 4); /* plus 3 divide 4 is byte align */
}

static td_void isp_online_stt_enable_reg_config(isp_post_be_reg_type *post_be_reg)
{
    td_bool stt_enable = TD_TRUE;

    isp_dehaze_stt_en_write(post_be_reg, stt_enable);
    isp_ldci_lpfstt_en_write(post_be_reg, stt_enable);

    isp_dehaze_stt_bst_write(post_be_reg, 0xF);
    isp_ldci_lpfstt_bst_write(post_be_reg, 0xF);
}

static td_s32 isp_online_stt_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_u16 num_h, num_v;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if ((is_offline_mode(isp_ctx->block_attr.running_mode)) || (is_striping_mode(isp_ctx->block_attr.running_mode))) {
        return TD_SUCCESS;
    }

    isp_ae_online_stt_reg_config(vi_pipe, reg_cfg_info, i);
    isp_la_online_stt_reg_config(vi_pipe, reg_cfg_info, i);
    isp_awb_online_stt_reg_config(vi_pipe, reg_cfg_info, i);
    isp_af_online_stt_reg_config(vi_pipe, reg_cfg_info, i);

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);

    if (isp_ctx->special_opt.be_on_stt_update[i]) {
        isp_online_stt_enable_reg_config(post_be_reg);

        isp_ctx->special_opt.be_on_stt_update[i] = TD_FALSE;
    }

    /* dehaze */
    num_h = reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.static_reg_cfg.dchnum;
    num_v = reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.static_reg_cfg.dcvnum;
    isp_dehaze_stt_size_write(post_be_reg, (((num_h + 1) * (num_v + 1)) + 3) / 4); /* plus 3 divide 4 is byte align */

    /* Ldci */
    num_h = reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.dyna_reg_cfg.stat_sml_map_width;
    num_v = reg_cfg_info->alg_reg_cfg[i].ldci_reg_cfg.dyna_reg_cfg.stat_sml_map_height;
    isp_ldci_lpfstt_size_write(post_be_reg, (num_h * num_v + 3) / 4); /* plus 3 divide 4 is byte align */

    return TD_SUCCESS;
}

static td_s32 isp_be_roi_reg_config(ot_vi_pipe vi_pipe, td_u8 i)
{
    td_u16 width, height;
    isp_usr_ctx *isp_ctx     = TD_NULL;
    isp_be_reg_type *be_reg  = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(be_reg);

    width = isp_ctx->block_attr.block_rect[i].width;
    height = isp_ctx->block_attr.block_rect[i].height;

    isp_blc_roi_en_write(be_reg, TD_FALSE);
    isp_blc_roi_pos_write(be_reg, 0, 0);
    isp_blc_roi_size_write(be_reg, width - 1, height - 1);

    isp_dg_roi_en_write(be_reg, TD_FALSE);
    isp_dg_roi_pos_write(be_reg, 0, 0);
    isp_dg_roi_size_write(be_reg, width - 1, height - 1);

    isp_fpn_roi_en_write(be_reg, TD_FALSE);
    isp_fpn_roi_pos_write(be_reg, 0, 0);
    isp_fpn_roi_size_write(be_reg, width - 1, height - 1);
    return TD_SUCCESS;
}

static td_s32 isp_be_alg_lut2stt_reg_new_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_be_stt2lut_regnew_reg_cfg *be_stt2lut_regnew_cfg = &reg_cfg_info->alg_reg_cfg[i].stt2lut_regnew_cfg;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(pre_be_reg);

    if (be_stt2lut_regnew_cfg->gamma_stt2lut_regnew) {
        isp_gamma_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->csp_stt2lut_regnew) {
        isp_cc_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->lsc_stt2lut_regnew) {
        isp_mlsc_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->ca_stt2lut_regnew) {
        isp_ca_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->dmnr_stt2lut_regnew) {
        isp_dmnr_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->clut_stt2lut_regnew) {
        isp_clut_stt2lut_regnew_write(post_be_reg, TD_TRUE);
    }

    if ((isp_ctx->frame_cnt < STT_LUT_CONFIG_TIMES) || be_stt2lut_regnew_cfg->dpc_stt2lut_regnew) {
        isp_dpc_stt2lut_regnew_write(pre_be_reg, TD_TRUE);
    }

    return TD_SUCCESS;
}

static td_s32 isp_be_alg_lut_update_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_be_lut_update_reg_cfg *be_lut_update_cfg = &reg_cfg_info->alg_reg_cfg[i].be_lut_update_cfg;
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);

    if (be_lut_update_cfg->ae_lut_update) {
        isp_ae_lut_update_write(post_be_reg, be_lut_update_cfg->ae_lut_update);
    }

    if (be_lut_update_cfg->drc_tm_lut_update) {
        isp_adrc_tmlut_update_write(post_be_reg, be_lut_update_cfg->drc_tm_lut_update);
    }

    if (be_lut_update_cfg->lcac_lut_update) {
        isp_demosaic_depurplut_update_write(post_be_reg, be_lut_update_cfg->lcac_lut_update);
    }

    if (be_lut_update_cfg->nddm_gf_lut_update) {
        isp_nddm_gf_lut_update_write(post_be_reg, be_lut_update_cfg->nddm_gf_lut_update);
    }

    return TD_SUCCESS;
}

static td_void isp_be_cur_enable_reg_config(ot_vi_pipe vi_pipe, isp_alg_reg_cfg *alg_reg_cfg, td_u8 i)
{
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    if ((post_viproc == TD_NULL) || (pre_viproc == TD_NULL) || (post_be_reg == TD_NULL) || (pre_be_reg == TD_NULL)) {
        return;
    }

    /* pre be */
    isp_dpc_en_write(pre_viproc, alg_reg_cfg->dp_reg_cfg.dpc_en[0]);       /* array index 0 */
    isp_dpc_dpc_en1_write(pre_be_reg, alg_reg_cfg->dp_reg_cfg.dpc_en[1]);
    isp_ge_en_write(pre_viproc, alg_reg_cfg->ge_reg_cfg.ge_en[0]);         /* array index 0 */
    isp_ge_ge1_en_write(pre_be_reg, alg_reg_cfg->ge_reg_cfg.ge_en[1]);     /* array index 2 */
    isp_expander_en_write(pre_viproc, alg_reg_cfg->expander_cfg.enable);
    isp_wb_en_write(pre_viproc, alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_wb_work_en);
    isp_lsc_en_write(pre_viproc, alg_reg_cfg->lsc_reg_cfg.lsc_en);

    /* post be */
    /* viproc part */
    isp_cc_en_write(post_viproc, alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_cc_en);
    isp_sharpen_en_write(post_viproc, alg_reg_cfg->sharpen_reg_cfg.enable);
    isp_sharpen_printfsel_write(post_be_reg, alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.print_sel);
    isp_dmnr_vhdm_en_write(post_viproc, alg_reg_cfg->dem_reg_cfg.vhdm_enable);
    isp_dmnr_nddm_en_write(post_viproc, alg_reg_cfg->dem_reg_cfg.nddm_enable);
    isp_gamma_en_write(post_viproc, alg_reg_cfg->gamma_cfg.gamma_en);
    isp_csc_en_write(post_viproc, alg_reg_cfg->csc_cfg.enable);
    isp_ca_en_write(post_viproc, alg_reg_cfg->ca_reg_cfg.ca_en);
    isp_dehaze_en_write(post_viproc, alg_reg_cfg->dehaze_reg_cfg.dehaze_en);
    isp_clut_en_write(post_viproc, alg_reg_cfg->clut_cfg.enable);

    /* be part */
    isp_cc_colortone_en_write(post_be_reg, alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_cc_colortone_en);
    isp_cc_lutb_en_write(post_be_reg, alg_reg_cfg->drc_reg_cfg.usr_reg_cfg.csp_en);
    isp_ca_cp_en_write(post_be_reg, alg_reg_cfg->ca_reg_cfg.usr_reg_cfg.ca_cp_en);
    isp_demosaic_gcac_en_write(post_be_reg, alg_reg_cfg->cac_reg_cfg.cac_en);
    isp_demosaic_bshp_en_write(post_be_reg, alg_reg_cfg->bshp_reg_cfg.bshp_enable);
    isp_demosaic_fcr_en_write(post_be_reg, alg_reg_cfg->anti_false_color_reg_cfg.fcr_enable);
}

static td_s32 isp_be_cur_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_alg_reg_cfg *alg_reg_cfg = &reg_cfg_info->alg_reg_cfg[i];
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);

    isp_check_pointer_return(post_be_reg);

    /* module enable */
    isp_be_cur_enable_reg_config(vi_pipe, alg_reg_cfg, i);

    /* ldci */
    isp_ldci_blc_ctrl_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.calc_blc_ctrl);
    isp_ldci_lpfcoef0_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef[0]); /* array index 0 */
    isp_ldci_lpfcoef1_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef[1]); /* array index 1 */
    isp_ldci_lpfcoef2_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef[2]); /* array index 2 */
    isp_ldci_lpfcoef3_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef[3]); /* array index 3 */
    isp_ldci_lpfcoef4_write(post_be_reg, alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef[4]); /* array index 4 */

    /* lsc */
    if (alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.lut_update) {
        isp_lsc_mesh_scale_write(post_be_reg, alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.mesh_scale);
        isp_mlsc_mesh_scale_write(post_be_reg, alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.mesh_scale);
    }

    /* ca */
    isp_ca_isoratio_write(post_be_reg, alg_reg_cfg->ca_reg_cfg.dyna_reg_cfg.ca_iso_ratio);

    /* clut */
    if (alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.resh) {
        isp_clut_gain0_write(post_be_reg, alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_r);
        isp_clut_gain1_write(post_be_reg, alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_g);
        isp_clut_gain2_write(post_be_reg, alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_b);
    }
    /* dehaze */
    isp_dehaze_air_r_write(post_be_reg, alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_r);
    isp_dehaze_air_g_write(post_be_reg, alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_g);
    isp_dehaze_air_b_write(post_be_reg, alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_b);
    isp_dehaze_gstrth_write(post_be_reg, alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.strength);

    /* sharpen */
    isp_sharpen_dyna_reg_config(post_be_reg, &alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg,
        &alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.default_dyna_reg_cfg);

    return TD_SUCCESS;
}

static td_void isp_pre_be_last_reg_config_enable(isp_viproc_reg_type *pre_viproc, isp_pre_be_reg_type *pre_be_reg,
    isp_lut2stt_sync_reg_cfg *lut2stt_sync_cfg)
{
    isp_dpc_en_write(pre_viproc, lut2stt_sync_cfg->dpc_en[0]);       /* array index 0 */
    isp_dpc_dpc_en1_write(pre_be_reg, lut2stt_sync_cfg->dpc_en[1]);
    isp_ge_en_write(pre_viproc, lut2stt_sync_cfg->ge_en[0]);         /* array index 0 */
    isp_ge_ge1_en_write(pre_be_reg, lut2stt_sync_cfg->ge_en[1]);     /* array index 2 */
    isp_expander_en_write(pre_viproc, lut2stt_sync_cfg->expander_en);
    isp_wb_en_write(pre_viproc, lut2stt_sync_cfg->wb_en);
}

static td_void isp_be_last_reg_config_enable(ot_vi_pipe vi_pipe, isp_lut2stt_sync_reg_cfg *lut2stt_sync_cfg, td_u8 i)
{
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(post_be_reg);
    isp_check_pointer_void_return(pre_be_reg);
    isp_check_pointer_void_return(post_viproc);
    isp_check_pointer_void_return(pre_viproc);

    /* pre be */
    isp_pre_be_last_reg_config_enable(pre_viproc, pre_be_reg, lut2stt_sync_cfg);

    /* post be */
    /* viproc part */
    isp_cc_en_write(post_viproc, lut2stt_sync_cfg->ccm_en);
    isp_sharpen_en_write(post_viproc, lut2stt_sync_cfg->sharpen_en);
    isp_sharpen_printfsel_write(post_be_reg, lut2stt_sync_cfg->print_sel);
    isp_dmnr_vhdm_en_write(post_viproc, lut2stt_sync_cfg->vhdm_en);
    isp_dmnr_nddm_en_write(post_viproc, lut2stt_sync_cfg->nddm_en);
    isp_lsc_en_write(post_viproc, lut2stt_sync_cfg->lsc_en);
    isp_gamma_en_write(post_viproc, lut2stt_sync_cfg->gamma_en);
    isp_csc_en_write(post_viproc, lut2stt_sync_cfg->csc_en);
    isp_ca_en_write(post_viproc, lut2stt_sync_cfg->ca_en);
    isp_dehaze_en_write(post_viproc, lut2stt_sync_cfg->dehaze_en);
    isp_clut_en_write(post_viproc, lut2stt_sync_cfg->clut_en);
    /* be part */
    isp_cc_colortone_en_write(post_be_reg, lut2stt_sync_cfg->ccm_color_tone_en);
    isp_cc_lutb_en_write(post_be_reg, lut2stt_sync_cfg->csp_en);
    isp_ca_cp_en_write(post_be_reg, lut2stt_sync_cfg->ca_cp_en);
    isp_demosaic_gcac_en_write(post_be_reg, lut2stt_sync_cfg->cac_en);
    isp_demosaic_bshp_en_write(post_be_reg, lut2stt_sync_cfg->bshp_en);
    isp_demosaic_fcr_en_write(post_be_reg, lut2stt_sync_cfg->fcr_en);
}

static td_s32 isp_be_last_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_lut2stt_sync_reg_cfg *lut2stt_sync_cfg = &reg_cfg_info->alg_reg_cfg[i].lut2stt_sync_cfg[0];
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    isp_check_pointer_return(post_be_reg);

    /* module enable */
    isp_be_last_reg_config_enable(vi_pipe, lut2stt_sync_cfg, i);

    /* ldci */
    isp_ldci_blc_ctrl_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.calc_blc_ctrl);
    isp_ldci_lpfcoef0_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef[0]); /* array index 0 */
    isp_ldci_lpfcoef1_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef[1]); /* array index 1 */
    isp_ldci_lpfcoef2_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef[2]); /* array index 2 */
    isp_ldci_lpfcoef3_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef[3]); /* array index 3 */
    isp_ldci_lpfcoef4_write(post_be_reg, lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef[4]); /* array index 4 */

    /* lsc */
    if (lut2stt_sync_cfg->lsc_sync_cfg.resh) {
        isp_lsc_mesh_scale_write(post_be_reg, lut2stt_sync_cfg->lsc_sync_cfg.mesh_scale);
        isp_mlsc_mesh_scale_write(post_be_reg, lut2stt_sync_cfg->lsc_sync_cfg.mesh_scale);
    }
    /* ca */
    isp_ca_isoratio_write(post_be_reg, lut2stt_sync_cfg->ca_sync_cfg.ca_iso_ratio);
    /* clut */
    if (lut2stt_sync_cfg->clut_sync_cfg.resh) {
        isp_clut_gain0_write(post_be_reg, lut2stt_sync_cfg->clut_sync_cfg.gain_r);
        isp_clut_gain1_write(post_be_reg, lut2stt_sync_cfg->clut_sync_cfg.gain_g);
        isp_clut_gain2_write(post_be_reg, lut2stt_sync_cfg->clut_sync_cfg.gain_b);
    }
    /* dehaze */
    if (reg_cfg_info->alg_reg_cfg[i].dehaze_reg_cfg.lut2_stt_en == TD_TRUE) {
        isp_dehaze_air_r_write(post_be_reg, lut2stt_sync_cfg->dehaze_sync_cfg.air_r);
        isp_dehaze_air_g_write(post_be_reg, lut2stt_sync_cfg->dehaze_sync_cfg.air_g);
        isp_dehaze_air_b_write(post_be_reg, lut2stt_sync_cfg->dehaze_sync_cfg.air_b);
        isp_dehaze_gstrth_write(post_be_reg, lut2stt_sync_cfg->dehaze_sync_cfg.strength);
    }
    /* sharpen */
    isp_sharpen_dyna_reg_config(post_be_reg, &lut2stt_sync_cfg->sharpen_sync_cfg.mpi_dyna_reg_cfg,
        &lut2stt_sync_cfg->sharpen_sync_cfg.default_dyna_reg_cfg);

    return TD_SUCCESS;
}

static td_s32 isp_be_alg_sync_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    td_s32 ret;
    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        ret = isp_be_cur_reg_config(vi_pipe, reg_cfg_info, i);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Pipe:%d isp_be_cur_reg_config failed!\n", vi_pipe);
        }
    } else {
        ret = isp_be_last_reg_config(vi_pipe, reg_cfg_info, i);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Pipe:%d isp_be_last_reg_config failed!\n", vi_pipe);
        }
    }

    return TD_SUCCESS;
}

static td_void isp_save_be_sync_enable_reg(isp_lut2stt_sync_reg_cfg *lut2stt_sync_cfg, isp_alg_reg_cfg *alg_reg_cfg)
{
    td_u8 j;
    lut2stt_sync_cfg->ae_en = alg_reg_cfg->ae_reg_cfg.static_reg_cfg.be_enable;
    lut2stt_sync_cfg->la_en = alg_reg_cfg->mg_reg_cfg.static_reg_cfg.enable;
    lut2stt_sync_cfg->awb_en = alg_reg_cfg->awb_reg_cfg.awb_reg_sta_cfg.be_awb_work_en;
    lut2stt_sync_cfg->wb_en = alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_wb_work_en;
    lut2stt_sync_cfg->awblsc_en = alg_reg_cfg->awblsc_reg_cfg.awblsc_reg_sta_cfg.awb_work_en;
    lut2stt_sync_cfg->ccm_en = alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_cc_en;
    lut2stt_sync_cfg->ccm_color_tone_en = alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_cc_colortone_en;
    lut2stt_sync_cfg->af_en = alg_reg_cfg->be_af_reg_cfg.af_enable;
    lut2stt_sync_cfg->sharpen_en = alg_reg_cfg->sharpen_reg_cfg.enable;
    lut2stt_sync_cfg->print_sel = alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.print_sel;
    lut2stt_sync_cfg->vhdm_en = alg_reg_cfg->dem_reg_cfg.vhdm_enable;
    lut2stt_sync_cfg->nddm_en = alg_reg_cfg->dem_reg_cfg.nddm_enable;
    lut2stt_sync_cfg->ldci_en = alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.enable;
    lut2stt_sync_cfg->cac_en = alg_reg_cfg->cac_reg_cfg.cac_en;
    lut2stt_sync_cfg->bshp_en = alg_reg_cfg->bshp_reg_cfg.bshp_enable;
    lut2stt_sync_cfg->fcr_en = alg_reg_cfg->anti_false_color_reg_cfg.fcr_enable;
    lut2stt_sync_cfg->lsc_en = alg_reg_cfg->lsc_reg_cfg.lsc_en;
    lut2stt_sync_cfg->gamma_en = alg_reg_cfg->gamma_cfg.gamma_en;
    lut2stt_sync_cfg->csc_en = alg_reg_cfg->csc_cfg.enable;
    lut2stt_sync_cfg->ca_en = alg_reg_cfg->ca_reg_cfg.ca_en;
    lut2stt_sync_cfg->ca_cp_en = alg_reg_cfg->ca_reg_cfg.usr_reg_cfg.ca_cp_en;
    lut2stt_sync_cfg->ca_sync_cfg.ca_iso_ratio      = alg_reg_cfg->ca_reg_cfg.dyna_reg_cfg.ca_iso_ratio;
    lut2stt_sync_cfg->wdr_en = alg_reg_cfg->wdr_reg_cfg.wdr_en;
    lut2stt_sync_cfg->drc_en = alg_reg_cfg->drc_reg_cfg.enable;
    lut2stt_sync_cfg->dehaze_en = alg_reg_cfg->dehaze_reg_cfg.dehaze_en;
    lut2stt_sync_cfg->bnr_lsc_en = alg_reg_cfg->bnr_reg_cfg.dyna_reg_cfg.bnr_lsc_en;
    lut2stt_sync_cfg->dg_en = alg_reg_cfg->dg_reg_cfg.dg_en;
    lut2stt_sync_cfg->four_dg_en = alg_reg_cfg->four_dg_reg_cfg.enable;
    lut2stt_sync_cfg->pregamma_en = alg_reg_cfg->pregamma_reg_cfg.enable;
    lut2stt_sync_cfg->clut_en = alg_reg_cfg->clut_cfg.enable;
    lut2stt_sync_cfg->expander_en = alg_reg_cfg->expander_cfg.enable;
    lut2stt_sync_cfg->csp_en = alg_reg_cfg->drc_reg_cfg.usr_reg_cfg.csp_en;

    for (j = 0; j < ISP_DPC_MAX_CHN_NUM; j++) { /*  dpc channel number is 2 */
        lut2stt_sync_cfg->dpc_en[j] = alg_reg_cfg->dp_reg_cfg.dpc_en[j];
    }

    for (j = 0; j < 4; j++) { /*  ge channel number is 4 */
        lut2stt_sync_cfg->ge_en[j] = alg_reg_cfg->ge_reg_cfg.ge_en[j];
    }
}

static td_s32 isp_save_be_sync_reg(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_lut2stt_sync_reg_cfg *lut2stt_sync_cfg = &reg_cfg_info->alg_reg_cfg[i].lut2stt_sync_cfg[0];
    isp_alg_reg_cfg *alg_reg_cfg = &reg_cfg_info->alg_reg_cfg[i];

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return TD_SUCCESS;
    }

    isp_save_be_sync_enable_reg(lut2stt_sync_cfg, alg_reg_cfg);

    lut2stt_sync_cfg->lsc_sync_cfg.resh = alg_reg_cfg->be_lut_update_cfg.lsc_lut_update;
    lut2stt_sync_cfg->lsc_sync_cfg.mesh_scale = alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.mesh_scale;
    lut2stt_sync_cfg->clut_sync_cfg.resh = alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.resh;
    lut2stt_sync_cfg->clut_sync_cfg.gain_r = alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_r;
    lut2stt_sync_cfg->clut_sync_cfg.gain_g = alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_g;
    lut2stt_sync_cfg->clut_sync_cfg.gain_b = alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.gain_b;

    lut2stt_sync_cfg->dehaze_sync_cfg.air_r = alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_r;
    lut2stt_sync_cfg->dehaze_sync_cfg.air_g = alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_g;
    lut2stt_sync_cfg->dehaze_sync_cfg.air_b = alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.air_b;
    lut2stt_sync_cfg->dehaze_sync_cfg.strength = alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.strength;

    lut2stt_sync_cfg->ldci_sync_cfg.calc_blc_ctrl = alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.calc_blc_ctrl;
    (td_void)memcpy_s(lut2stt_sync_cfg->ldci_sync_cfg.lpf_coef, sizeof(td_u32) * LDCI_LPF_LUT_SIZE,
        alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg.lpf_coef, sizeof(td_u32) * LDCI_LPF_LUT_SIZE);

    (td_void)memcpy_s(&lut2stt_sync_cfg->sharpen_sync_cfg.mpi_dyna_reg_cfg, sizeof(isp_sharpen_mpi_dyna_reg_cfg),
        &alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg, sizeof(isp_sharpen_mpi_dyna_reg_cfg));
    (td_void)memcpy_s(
        &lut2stt_sync_cfg->sharpen_sync_cfg.default_dyna_reg_cfg, sizeof(isp_sharpen_default_dyna_reg_cfg),
        &alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.default_dyna_reg_cfg, sizeof(isp_sharpen_default_dyna_reg_cfg));

    return TD_SUCCESS;
}

static td_void isp_be_reah_cfg_raw_domain1(isp_alg_reg_cfg *alg_reg_cfg)
{
    alg_reg_cfg->dp_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->dp_reg_cfg.usr_reg_cfg.usr_dyna_cor_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dp_reg_cfg.usr_reg_cfg.usr_sta_cor_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dp_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->ge_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->ge_reg_cfg.usr_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->ge_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->four_dg_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->four_dg_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->wdr_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->wdr_reg_cfg.usr_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->wdr_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
}

static td_void isp_be_reah_cfg_raw_domain2(isp_alg_reg_cfg *alg_reg_cfg)
{
    alg_reg_cfg->expander_cfg.usr_cfg.resh = TD_TRUE;
    alg_reg_cfg->bnr_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->bnr_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->bnr_reg_cfg.usr_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->lsc_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.coef_update = TD_TRUE;
    alg_reg_cfg->lsc_reg_cfg.usr_reg_cfg.lut_update = TD_TRUE;

    alg_reg_cfg->dg_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dg_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->be_blc_cfg.static_blc.resh_static = TD_TRUE;
    alg_reg_cfg->be_blc_cfg.resh_dyna_init = TD_TRUE;

    alg_reg_cfg->awb_reg_cfg.awb_reg_sta_cfg.be_awb_sta_cfg = TD_TRUE;
    alg_reg_cfg->awb_reg_cfg.awb_reg_usr_cfg.resh = TD_TRUE;

    alg_reg_cfg->drc_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->drc_reg_cfg.usr_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->drc_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->dem_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dem_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->cac_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->cac_reg_cfg.usr_reg_cfg.usr_resh = TD_TRUE;
    alg_reg_cfg->cac_reg_cfg.dyna_reg_cfg.dyna_resh = TD_TRUE;

    alg_reg_cfg->anti_false_color_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->anti_false_color_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
}

static td_void isp_be_reah_cfg_rgb_domain(isp_alg_reg_cfg *alg_reg_cfg)
{
    alg_reg_cfg->gamma_cfg.usr_reg_cfg.gamma_lut_update_en = TD_TRUE;
    alg_reg_cfg->csc_cfg.dyna_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dehaze_reg_cfg.static_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->dehaze_reg_cfg.dyna_reg_cfg.lut_update = 1;
    alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_ctrl_cfg.resh = TD_TRUE;
    alg_reg_cfg->clut_cfg.usr_reg_cfg.clut_usr_coef_cfg.resh = TD_TRUE;
}

static td_void isp_be_reah_cfg_yuv_domain(isp_alg_reg_cfg *alg_reg_cfg)
{
    alg_reg_cfg->ldci_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->ca_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->ca_reg_cfg.dyna_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->ca_reg_cfg.usr_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->ca_reg_cfg.usr_reg_cfg.ca_lut_update_en = TD_TRUE;

    alg_reg_cfg->sharpen_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.default_dyna_reg_cfg.resh = TD_TRUE;
    alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg.resh = TD_TRUE;

    alg_reg_cfg->mcds_reg_cfg.static_reg_cfg.static_resh = TD_TRUE;
    alg_reg_cfg->mcds_reg_cfg.dyna_reg_cfg.dyna_resh = TD_TRUE;
}

static td_void isp_be_resh_cfg(isp_alg_reg_cfg *alg_reg_cfg)
{
    isp_be_reah_cfg_raw_domain1(alg_reg_cfg);

    isp_be_reah_cfg_raw_domain2(alg_reg_cfg);

    isp_be_reah_cfg_rgb_domain(alg_reg_cfg);

    isp_be_reah_cfg_yuv_domain(alg_reg_cfg);
}

td_s32 isp_reset_fe_stt_en(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    ot_vi_pipe vi_pipe_bind;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_fe_reg_type *fe_reg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe) {
        for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
            vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];
            isp_check_no_fe_pipe_return(vi_pipe_bind);

            fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
            isp_check_pointer_return(fe_reg);

            isp_fe_ae_stt_en_write(fe_reg, TD_FALSE);
            isp_fe_af_stt_en_write(fe_reg, TD_FALSE);
            isp_fe_update_write(fe_reg, TD_TRUE);
        }
    }

    return TD_SUCCESS;
}

static td_void isp_fe_alg_en_exit(isp_usr_ctx *isp_ctx)
{
    td_u8 i;
    ot_vi_pipe vi_pipe_bind;
    isp_fe_reg_type *fe_reg = TD_NULL;

    if (isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) {
        return;
    }

    for (i = 0; i < isp_ctx->wdr_attr.pipe_num; i++) {
        vi_pipe_bind = isp_ctx->wdr_attr.pipe_id[i];

        if ((vi_pipe_bind < 0) || (vi_pipe_bind >= OT_ISP_MAX_FE_PIPE_NUM)) {
            continue;
        }

        fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe_bind);
        isp_check_pointer_void_return(fe_reg);

        isp_fe_ae_en_write(fe_reg, TD_FALSE);
        isp_fe_wb_en_write(fe_reg, TD_FALSE);
        isp_fe_dg2_en_write(fe_reg, TD_FALSE);
        isp_fe_af_en_write(fe_reg, TD_FALSE);
        isp_fe_ae_stt_en_write(fe_reg, TD_FALSE);
        isp_fe_af_stt_en_write(fe_reg, TD_FALSE);
        isp_fe_local_blc_en_write(fe_reg, TD_FALSE);

        isp_lblc_stt2lut_en_write(fe_reg, TD_FALSE);
        isp_fe_update_write(fe_reg, TD_TRUE);
    }
}

static td_void isp_disable_tnr_reg_write(isp_post_be_reg_type *post_be_reg, isp_viproc_reg_type *post_viproc)
{
    isp_sharpen_en_write(post_viproc, TD_FALSE);

    isp_bnr_en_write(post_viproc, TD_FALSE);
    isp_bnr_ensptnr_write(post_be_reg, TD_FALSE);
}

static td_void isp_disable_bnr_tnr(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_bnr_temporal_filt tpr_filt;
    tpr_filt.nr_en = TD_FALSE;
    tpr_filt.tnr_en = TD_FALSE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_DISABLE_BNR_WMOT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("set bnr mot off failure!\n");
    }

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BNR_TEMPORAL_FILT_CFG_SET, &tpr_filt);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] set bnr temporal filer failed %#x!\n", vi_pipe, ret);
    }
}

static td_void isp_be_alg_en_exit(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx)
{
    td_u8 i;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_viproc_reg_type *pre_viproc = TD_NULL;

    if (is_offline_mode(isp_ctx->block_attr.running_mode) || is_striping_mode(isp_ctx->block_attr.running_mode)) {
        return;
    }

    for (i = 0; i < isp_ctx->block_attr.block_num; i++) {
        post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
        pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
        post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
        pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
        if ((post_be_reg == TD_NULL) || (pre_be_reg == TD_NULL) || (post_viproc == TD_NULL) ||
            (pre_viproc == TD_NULL)) {
            return;
        }

        /* stat to ddr:disable module_en */
        isp_ae_en_write(post_viproc, TD_FALSE);
        isp_la_en_write(post_viproc, TD_FALSE);
        isp_awb_en_write(post_viproc, TD_FALSE);
        isp_af_en_write(post_viproc, TD_FALSE);
        isp_dehaze_en_write(post_viproc, TD_FALSE);
        isp_ldci_en_write(post_viproc, TD_FALSE);
        isp_flicker_en_write(pre_viproc, TD_FALSE);
        isp_ldci_wrstat_en_write(post_be_reg, TD_FALSE);
        isp_ldci_rdstat_en_write(post_be_reg, TD_FALSE);

        /* lut2stt: disable stt2lut_en  */
        isp_dpc_stt2lut_en_write(pre_be_reg, TD_FALSE);
        isp_sharpen_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_ldci_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_dehaze_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_gamma_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_mlsc_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_bnr_stt2lut_en_write(post_be_reg, TD_FALSE);
        isp_clut_stt2lut_en_write(post_be_reg, TD_FALSE);

        isp_disable_bnr_tnr(vi_pipe);

        isp_disable_tnr_reg_write(post_be_reg, post_viproc);
    }
}

#define ms_to_us(milli_sec) (1000 * (milli_sec))

static td_void isp_alg_en_exit_wait(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx)
{
    td_u32 milli_sec;
    td_u32 fe_cnt_base, fe_cnt;
    td_u64 time_begin;
    isp_fe_reg_type *fe_reg = TD_NULL;

    if ((isp_ctx->wdr_attr.is_mast_pipe != TD_TRUE) || is_virt_pipe(vi_pipe) || is_no_fe_phy_pipe(vi_pipe)) {
        return;
    }

    fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe);
    isp_check_pointer_void_return(fe_reg);

    milli_sec = (td_u32)(2000 / div_0_to_1_float(isp_ctx->sns_image_mode.fps)); /* 2000:2 * 1000, wait 2 frame */

    fe_cnt_base = fe_reg->isp_fe_startup.bits.isp_fe_fcnt;
    fe_cnt      = fe_cnt_base;
    time_begin  = get_sys_time_by_usec();
    while ((fe_cnt - fe_cnt_base) < 2) { /* 2 for a full frame process */
        fe_cnt = fe_reg->isp_fe_startup.bits.isp_fe_fcnt;
        if ((get_sys_time_by_usec() - time_begin) >= ms_to_us(milli_sec)) {
            return;
        }
#ifdef __LITEOS__
        usleep(1); /* msleep 10 */
#else
        usleep(ms_to_us(10)); /* msleep 10 */
#endif
    }
}

td_s32 isp_alg_en_exit(ot_vi_pipe vi_pipe)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_check_pointer_return(isp_ctx);

    if (isp_ctx->para_rec.init == TD_FALSE) {
        return TD_SUCCESS;
    }

    /* FE */
    isp_fe_alg_en_exit(isp_ctx);

    /* BE */
    isp_be_alg_en_exit(vi_pipe, isp_ctx);

    /* wait */
    isp_alg_en_exit_wait(vi_pipe, isp_ctx);

    return TD_SUCCESS;
}

static td_s32 isp_fe_regs_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_s64 ret = TD_SUCCESS;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_check_no_fe_pipe_return(vi_pipe);
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->wdr_attr.is_mast_pipe) {
        /* FE alg cfgs setting to register */
        ret += isp_fe_ae_reg_config(vi_pipe, reg_cfg_info);  /* Ae */
        ret += isp_fe_awb_reg_config(vi_pipe, reg_cfg_info); /* awb */
        ret += isp_fe_af_reg_config(vi_pipe, reg_cfg_info);  /* Af */
        ret += isp_fe_dg_reg_config(vi_pipe, reg_cfg_info);  /* DG */
        ret += isp_fe_blc_reg_config(vi_pipe, reg_cfg_info); /* fe blc */
        ret += isp_fe_system_reg_config(vi_pipe);
        ret += isp_fe_stt_reg_config(vi_pipe, reg_cfg_info);
        ret += isp_fe_dynamicblc_reg_config(vi_pipe, reg_cfg_info);
        ret += isp_lblc_reg_config(vi_pipe, reg_cfg_info);
        ret += isp_aibnr_fpn_reg_config(vi_pipe, reg_cfg_info);
        ret += isp_fe_roi_reg_config(vi_pipe);

        ret += isp_fe_update_reg_config(vi_pipe, reg_cfg_info);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] isp_fe_regs_config failed!\n", vi_pipe);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_be_drc_param_init(isp_be_drc_sync_param *drc_sync_param, isp_drc_reg_cfg *drc_reg_cfg)
{
    td_u8 j;
    drc_sync_param->shp_log = drc_reg_cfg->static_reg_cfg.shp_log;
    drc_sync_param->div_denom_log = drc_reg_cfg->static_reg_cfg.div_denom_log;
    drc_sync_param->denom_exp = drc_reg_cfg->static_reg_cfg.denom_exp;

    for (j = 0; j < OT_ISP_DRC_EXP_COMP_SAMPLE_NUM; j++) {
        drc_sync_param->prev_luma[j] = drc_reg_cfg->static_reg_cfg.prev_luma[j];
    }
}

static td_void isp_be_wdr_param_init(isp_be_wdr_sync_param *be_sync_param, isp_wdr_reg_cfg *wdr_reg_cfg)
{
    td_u8 j;

    be_sync_param->wdr_exp_ratio = wdr_reg_cfg->static_reg_cfg.expo_r_ratio_lut[0];
    be_sync_param->flick_exp_ratio = wdr_reg_cfg->static_reg_cfg.flick_exp_ratio;

    for (j = 0; j < OT_ISP_WDR_MAX_FRAME_NUM; j++) {
        be_sync_param->wdr_exp_val[j] = wdr_reg_cfg->static_reg_cfg.expo_value_lut[j];
    }

    be_sync_param->wdr_mdt_en = wdr_reg_cfg->dyna_reg_cfg.wdr_mdt_en;
    be_sync_param->fusion_mode = wdr_reg_cfg->usr_reg_cfg.fusion_mode;

    be_sync_param->short_thr = wdr_reg_cfg->dyna_reg_cfg.short_thr;
    be_sync_param->long_thr = wdr_reg_cfg->dyna_reg_cfg.long_thr;

    be_sync_param->wdr_max_ratio = wdr_reg_cfg->static_reg_cfg.max_ratio;
    be_sync_param->fusion_max_ratio = wdr_reg_cfg->static_reg_cfg.max_ratio;
}

static td_s32 isp_be_sync_param_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u8 j;
    td_s32 ret;
    isp_be_sync_para be_sync_param = { 0 };
    isp_alg_reg_cfg *alg_reg_cfg = &reg_cfg_info->alg_reg_cfg[0];

    /* DG */
    be_sync_param.isp_dgain[0] = alg_reg_cfg->dg_reg_cfg.dyna_reg_cfg.gain_r;  /* array 0 assignment */
    be_sync_param.isp_dgain[1] = alg_reg_cfg->dg_reg_cfg.dyna_reg_cfg.gain_gr; /* array 1 assignment */
    be_sync_param.isp_dgain[2] = alg_reg_cfg->dg_reg_cfg.dyna_reg_cfg.gain_gb; /* array 2 assignment */
    be_sync_param.isp_dgain[3] = alg_reg_cfg->dg_reg_cfg.dyna_reg_cfg.gain_b;  /* array 3 assignment */
    for (j = 0; j < OT_ISP_WDR_MAX_FRAME_NUM; j++) {
        be_sync_param.wdr_gain[j] = 0x100;
    }

    /* LDCI */
    be_sync_param.ldci.ldci_comp = 0x1000;

    /* DRC */
    isp_be_drc_param_init(&be_sync_param.drc, &alg_reg_cfg->drc_reg_cfg);

    /* WDR */
    isp_be_wdr_param_init(&be_sync_param.wdr, &alg_reg_cfg->wdr_reg_cfg);

    /* AWB */
    for (j = 0; j < OT_ISP_BAYER_CHN_NUM; j++) {
        be_sync_param.wb_gain[j] = alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_white_balance_gain[j];
    }

    /* blc */
    (td_void)memcpy_s(&be_sync_param.be_blc, sizeof(isp_be_blc_dyna_cfg), &alg_reg_cfg->be_blc_cfg.dyna_blc,
        sizeof(isp_be_blc_dyna_cfg));

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_SYNC_PARAM_INIT, &be_sync_param);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] Init BE Sync Param Failed with ec %#x!\n", vi_pipe, ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 isp_set_be_dump_raw_pre_be(ot_vi_pipe vi_pipe, const isp_usr_ctx *isp_ctx, td_u8 i)
{
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;

    if ((isp_ctx->be_frame_attr.frame_pos == OT_ISP_DUMP_FRAME_POS_NORMAL)
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
        && isp_dump_dbg_need_return_pre_be(isp_ctx->be_pos_attr)
#endif
        ) {
        return TD_SUCCESS;
    }

    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);

    isp_check_pointer_return(pre_be_reg);
    isp_check_pointer_return(pre_viproc);

#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
    isp_dump_dbg_process_pre_be(vi_pipe, pre_viproc);
#endif
    isp_expander_en_write(pre_viproc, TD_FALSE);
    isp_bcom_en_write(pre_viproc, TD_FALSE);
    isp_bnr_en_write(pre_viproc, TD_FALSE);
    isp_geq_en_write(pre_viproc, TD_FALSE);
    isp_bdec_en_write(pre_viproc, TD_FALSE);
    isp_lsc_en_write(pre_viproc, TD_FALSE);
    isp_dg_en_write(pre_viproc, TD_FALSE);
    isp_ae_en_write(pre_viproc, TD_FALSE);
    isp_awb_en_write(pre_viproc, TD_FALSE);
    isp_af_en_write(pre_viproc, TD_FALSE);
    isp_wb_en_write(pre_viproc, TD_FALSE);

    return TD_SUCCESS;
}

static td_s32 isp_set_be_dump_raw_post_be(ot_vi_pipe vi_pipe, td_u8 i)
{
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);

    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(post_viproc);
    if (isp_ctx->be_frame_attr.frame_pos == OT_ISP_DUMP_FRAME_POS_AFTER_WDR) {
        isp_la_en_write(post_viproc, TD_FALSE);
        isp_adrc_en_write(post_viproc, TD_FALSE);
        isp_drclog_en_write(post_viproc, TD_FALSE);
        isp_drc_dither_en_write(post_be_reg, TD_FALSE);
    }
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
    isp_dump_dbg_process_post_be(vi_pipe, post_be_reg, post_viproc);
#endif
    isp_dmnr_vhdm_en_write(post_viproc, TD_FALSE);
    isp_dmnr_nddm_en_write(post_viproc, TD_FALSE);
    isp_demosaic_local_cac_en_write(post_be_reg, TD_FALSE);
    isp_demosaic_bshp_en_write(post_be_reg, TD_FALSE);
    isp_demosaic_fcr_en_write(post_be_reg, TD_FALSE);

    return TD_SUCCESS;
}

static td_s32 isp_set_be_dump_raw_post_be_after_demosaic(ot_vi_pipe vi_pipe, td_u8 i)
{
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);

    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(post_viproc);

    isp_cc_en_write(post_viproc, TD_FALSE);
    isp_cc_colortone_en_write(post_be_reg, TD_FALSE);
    isp_cc_lutb_en_write(post_be_reg, TD_FALSE);
    isp_clut_en_write(post_viproc, TD_FALSE);
    isp_gamma_en_write(post_viproc, TD_FALSE);
    isp_dehaze_en_write(post_viproc, TD_FALSE);
    isp_csc_en_write(post_viproc, TD_FALSE);
    isp_ldci_en_write(post_viproc, TD_FALSE);
    isp_ca_en_write(post_viproc, TD_FALSE);
    isp_ca_cp_en_write(post_be_reg, TD_FALSE);
    isp_hcds_en_write(post_viproc, TD_FALSE);
    isp_sharpen_en_write(post_viproc, TD_FALSE);
    isp_radial_crop_en_write(post_viproc, TD_FALSE);

    /* dither */
    isp_dmnr_dither_en_write(post_be_reg, TD_FALSE);
    isp_sharpen_dither_en_write(post_be_reg, TD_FALSE);

    return TD_SUCCESS;
}

static td_s32 isp_set_be_dump_raw(ot_vi_pipe vi_pipe, td_u8 i)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->be_frame_attr.frame_pos == OT_ISP_DUMP_FRAME_POS_NORMAL
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
        && isp_dump_dbg_is_pos_normal(&isp_ctx->be_pos_attr)
#endif
    ) {
        return TD_SUCCESS;
    }

    ret = isp_set_be_dump_raw_pre_be(vi_pipe, isp_ctx, i);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_set_be_dump_raw_post_be(vi_pipe, i);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = isp_set_be_dump_raw_post_be_after_demosaic(vi_pipe, i);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
static td_s32 isp_set_be_dump_dbg_send(ot_vi_pipe vi_pipe, td_u8 i)
{
    td_s32 ret;

    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_pre_be_reg_type *pre_be_reg = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;
    isp_post_be_reg_type *post_be_reg = TD_NULL;

    pre_be_reg = (isp_pre_be_reg_type *)isp_get_pre_be_vir_addr(vi_pipe, i);
    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    post_be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);

    isp_check_pointer_return(pre_be_reg);
    isp_check_pointer_return(pre_viproc);
    isp_check_pointer_return(post_be_reg);
    isp_check_pointer_return(post_viproc);

    ret = isp_dump_dbg_be_send_config(vi_pipe, pre_be_reg, pre_viproc, post_be_reg, post_viproc);

    return ret;
}
#endif

static td_s32 isp_be_alg_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info, td_u8 i)
{
    td_s64 ret = TD_SUCCESS;

    ret += isp_ae_reg_config(vi_pipe, reg_cfg_info, i);       /* ae, pre or post */
    ret += isp_awb_reg_config(vi_pipe, reg_cfg_info, i);      /* awb, pre or post  */
    ret += isp_af_reg_config(vi_pipe, reg_cfg_info, i);       /* AF, pre or post  */
    ret += isp_sharpen_reg_config(vi_pipe, reg_cfg_info, i);  /* sharpen, post */
    ret += isp_demosaic_reg_config(vi_pipe, reg_cfg_info, i); /* demosaic, post */
    ret += isp_bshp_reg_config(vi_pipe, reg_cfg_info, i);     /* bayer sharpen, post */
    ret += isp_fpn_reg_config(vi_pipe, reg_cfg_info, i);      /* FPN, pre */
    ret += isp_ldci_reg_config(vi_pipe, reg_cfg_info, i);     /* ldci, post */
    ret += isp_cac_reg_config(vi_pipe, reg_cfg_info, i);      /* cac, post */
    ret += isp_fcr_reg_config(vi_pipe, reg_cfg_info, i);      /* FCR, post */
    ret += isp_dpc_reg_config(vi_pipe, reg_cfg_info, i);      /* dpc, pre */
    ret += isp_ge_reg_config(vi_pipe, reg_cfg_info, i);       /* ge, pre */
    ret += isp_lsc_reg_config(vi_pipe, reg_cfg_info, i);      /* BE LSC, pre */
    ret += isp_gamma_reg_config(vi_pipe, reg_cfg_info, i);    /* gamma, post */
    ret += isp_csc_reg_config(vi_pipe, reg_cfg_info, i);      /* csc, post */
    ret += isp_ca_reg_config(vi_pipe, reg_cfg_info, i);       /* ca, post */
    ret += isp_mcds_reg_config(vi_pipe, reg_cfg_info, i);     /* mcds, post */
    ret += isp_wdr_reg_config(vi_pipe, reg_cfg_info, i);      /* wdr, pre */
    ret += isp_drc_reg_config(vi_pipe, reg_cfg_info, i);      /* drc, pre or post */
    ret += isp_dehaze_reg_config(vi_pipe, reg_cfg_info, i);   /* dehaze, post */
    ret += isp_bayer_nr_reg_config(vi_pipe, reg_cfg_info, i); /* BayerNR, pre */
    ret += isp_dg_reg_config(vi_pipe, reg_cfg_info, i);       /* DG, pre */
    ret += isp_4dg_reg_config(vi_pipe, reg_cfg_info, i);      /* 4DG, pre */
    ret += isp_flick_reg_config(vi_pipe, reg_cfg_info, i);    /* Flicker, pre */
    ret += isp_be_blc_reg_config(vi_pipe, reg_cfg_info, i);   /* be blc, most pre */
    ret += isp_clut_reg_config(vi_pipe, reg_cfg_info, i);     /* CLUT, post */
    ret += isp_expander_reg_config(vi_pipe, reg_cfg_info, i); /* expander, pre */
    ret += isp_rc_reg_config(vi_pipe, reg_cfg_info, i);       /* radial crop, post */
    ret += isp_csp_reg_config(vi_pipe, reg_cfg_info, i);      /* csp, post */
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_void isp_be_reg_up_config(ot_vi_pipe vi_pipe, td_u8 i)
{
    isp_viproc_reg_type *pre_viproc = TD_NULL;
    isp_viproc_reg_type *post_viproc = TD_NULL;

    pre_viproc = (isp_viproc_reg_type *)isp_get_pre_vi_proc_vir_addr(vi_pipe, i);
    post_viproc = (isp_viproc_reg_type *)isp_get_post_vi_proc_vir_addr(vi_pipe, i);
    isp_check_pointer_void_return(pre_viproc);
    isp_check_pointer_void_return(post_viproc);

    isp_be_reg_up_write(pre_viproc, TD_TRUE);
    isp_be_reg_up_write(post_viproc, TD_TRUE);
}

static td_s64 isp_be_regs_config_update(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u32 i;
    td_s64 ret = TD_SUCCESS;

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_system_reg_config(vi_pipe, i);     /* sys */
        ret += isp_dither_reg_config(vi_pipe, reg_cfg_info, i);     /* dither */
        ret += isp_online_stt_reg_config(vi_pipe, reg_cfg_info, i); /* online stt */
        ret += isp_be_roi_reg_config(vi_pipe, i);

        /* Be alg cfgs setting to register */
        ret += isp_be_alg_reg_config(vi_pipe, reg_cfg_info, i);
    }

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_be_alg_sync_reg_config(vi_pipe, reg_cfg_info, i);
        ret += isp_be_alg_lut2stt_reg_new_reg_config(vi_pipe, reg_cfg_info, i);
        ret += isp_be_alg_lut_update_reg_config(vi_pipe, reg_cfg_info, i);
    }
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    ret += isp_reg_copy_to_extend_reg(vi_pipe);
#endif
    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_set_be_dump_raw(vi_pipe, i);
#ifdef CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT
        ret += isp_set_be_dump_dbg_send(vi_pipe, i);
#endif
    }

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_save_be_sync_reg(vi_pipe, reg_cfg_info, i);
    }

    return ret;
}

static td_s32 isp_be_regs_config(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_s64 ret;
    td_s32 ret1;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->alg_run_select != OT_ISP_ALG_RUN_FE_ONLY) {
        ret = isp_be_regs_config_update(vi_pipe, reg_cfg_info);
        if (ret != TD_SUCCESS) {
            isp_err_trace("ISP[%d] isp_be_regs_config_update failed!\n", vi_pipe);
            return TD_FAILURE;
        }
    }

    if ((is_offline_mode(isp_ctx->block_attr.running_mode)) ||
        (is_striping_mode(isp_ctx->block_attr.running_mode)) ||
        is_pre_online_post_offline(isp_ctx->block_attr.running_mode)) {
        ret1 = isp_cfg_be_buf_ctl(vi_pipe);
        if (ret1 != TD_SUCCESS) {
            isp_err_trace("ISP[%d] Be config bufs ctl failed %x!\n", vi_pipe, ret1);
            return ret1;
        }
    }

    return TD_SUCCESS;
}

static td_s32 isp_be_regs_config_init(ot_vi_pipe vi_pipe, isp_reg_cfg *reg_cfg_info)
{
    td_u32 i;
    td_s64 ret = TD_SUCCESS;

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_save_be_sync_reg(vi_pipe, reg_cfg_info, i);
    }

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_reg_default(vi_pipe, reg_cfg_info, i);
        ret += isp_system_reg_config(vi_pipe, i); /* sys */
        ret += isp_dither_reg_config(vi_pipe, reg_cfg_info, i); /* dither */
        ret += isp_online_stt_reg_config(vi_pipe, reg_cfg_info, i);
        /* Be alg cfgs setting to register */
        ret += isp_be_alg_reg_config(vi_pipe, reg_cfg_info, i);
    }

    for (i = 0; i < reg_cfg_info->cfg_num; i++) {
        ret += isp_be_alg_sync_reg_config(vi_pipe, reg_cfg_info, i);
        ret += isp_be_alg_lut2stt_reg_new_reg_config(vi_pipe, reg_cfg_info, i);
        isp_be_reg_up_config(vi_pipe, i);
        ret += isp_be_alg_lut_update_reg_config(vi_pipe, reg_cfg_info, i);
    }
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
    ret += isp_reg_copy_to_extend_reg(vi_pipe);
#endif
    ret += isp_be_sync_param_init(vi_pipe, reg_cfg_info);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d] isp_be_regs_config_init failed!\n", vi_pipe);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 isp_reg_cfg_init(ot_vi_pipe vi_pipe)
{
    isp_reg_cfg_attr *reg_cfg_ctx = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg_ctx);

    if (reg_cfg_ctx == TD_NULL) {
        reg_cfg_ctx = (isp_reg_cfg_attr *)isp_malloc(sizeof(isp_reg_cfg_attr));
        if (reg_cfg_ctx == TD_NULL) {
            isp_err_trace("Isp[%d] RegCfgCtx malloc memory failed!\n", vi_pipe);
            return OT_ERR_ISP_NOMEM;
        }
    }

    (td_void)memset_s(reg_cfg_ctx, sizeof(isp_reg_cfg_attr), 0, sizeof(isp_reg_cfg_attr));

    isp_regcfg_set_ctx(vi_pipe, reg_cfg_ctx);

    return TD_SUCCESS;
}

td_void isp_reg_cfg_exit(ot_vi_pipe vi_pipe)
{
    isp_reg_cfg_attr *reg_cfg_ctx = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg_ctx);
    isp_free(reg_cfg_ctx);
    isp_regcfg_reset_ctx(vi_pipe);
}

td_s32 isp_get_reg_cfg_ctx(ot_vi_pipe vi_pipe, td_void **reg_cfg_info)
{
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);

    if (!reg_cfg->init) {
        reg_cfg->reg_cfg.cfg_key.key = 0;

        reg_cfg->init = TD_TRUE;
    }

    reg_cfg->reg_cfg.cfg_num = isp_ctx->block_attr.block_num;

    *reg_cfg_info = &reg_cfg->reg_cfg;

    return TD_SUCCESS;
}

td_s32 isp_reg_cfg_info_init(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);

    ret = isp_fe_regs_config(vi_pipe, &reg_cfg->reg_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_fe_regs_config failed!\n", vi_pipe);
    }

    if (isp_ctx->alg_run_select != OT_ISP_ALG_RUN_FE_ONLY) {
        ret = isp_be_regs_config_init(vi_pipe, &reg_cfg->reg_cfg);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Pipe:%d isp_be_regs_config_init failed!\n", vi_pipe);
        }
    }
    return TD_SUCCESS;
}

td_s32 isp_reg_cfg_info_set(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);

    ret = isp_fe_regs_config(vi_pipe, &reg_cfg->reg_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_fe_regs_config failed!\n", vi_pipe);
    }

    ret = isp_be_regs_config(vi_pipe, &reg_cfg->reg_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("Pipe:%d isp_be_regs_config failed!\n", vi_pipe);
    }

    if (reg_cfg->reg_cfg.kernel_reg_cfg.cfg_key.key) {
        ret = ioctl(isp_get_fd(vi_pipe), ISP_REG_CFG_SET, &reg_cfg->reg_cfg.kernel_reg_cfg);
        if (ret != TD_SUCCESS) {
            isp_err_trace("Config ISP register Failed with ec %#x!\n", ret);
            return ret;
        }
    }

    return TD_SUCCESS;
}

static td_void isp_sns_regs_info_check(ot_vi_pipe vi_pipe, const ot_isp_sns_regs_info *sns_regs_info)
{
    ot_unused(vi_pipe);

    if ((sns_regs_info->sns_type >= OT_ISP_SNS_TYPE_BUTT)) {
        isp_err_trace("senor's regs info invalid, sns_type %d\n", sns_regs_info->sns_type);
        return;
    }

    if (sns_regs_info->reg_num > OT_ISP_MAX_SNS_REGS) {
        isp_err_trace("senor's regs info invalid, reg_num %u\n", sns_regs_info->reg_num);
        return;
    }

    return;
}

static td_void isp_get_start_and_end_vipipe(ot_vi_pipe vi_pipe, isp_usr_ctx *isp_ctx, td_s32 *pipe_st, td_s32 *pipe_ed)
{
    td_s8 stitch_main_pipe;

    if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
        stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];

        if (is_stitch_main_pipe(vi_pipe, stitch_main_pipe)) {
            *pipe_st = 0;
            *pipe_ed = isp_ctx->stitch_attr.stitch_pipe_num - 1;
        } else {
            *pipe_st = vi_pipe;
            *pipe_ed = vi_pipe - 1;
        }
    } else {
        *pipe_st = vi_pipe;
        *pipe_ed = vi_pipe;
    }
}

static td_void isp_normal_sync_cfg_get(ot_vi_pipe vi_pipe, ot_isp_sns_regs_info *sns_regs_info,
    isp_alg_reg_cfg *alg_reg_cfg, isp_sync_cfg_buf_node *sync_cfg_node)
{
    (td_void)memcpy_s(&sync_cfg_node->sns_regs_info, sizeof(ot_isp_sns_regs_info), sns_regs_info,
        sizeof(ot_isp_sns_regs_info));
    isp_sns_regs_info_check(vi_pipe, &sync_cfg_node->sns_regs_info);
    (td_void)memcpy_s(&sync_cfg_node->ae_reg_cfg, sizeof(isp_ae_reg_cfg_2), &alg_reg_cfg->ae_reg_cfg2,
        sizeof(isp_ae_reg_cfg_2));
    (td_void)memcpy_s(&sync_cfg_node->awb_reg_cfg.be_white_balance_gain[0], sizeof(td_u32) * OT_ISP_BAYER_CHN_NUM,
        &alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_white_balance_gain[0], sizeof(td_u32) * OT_ISP_BAYER_CHN_NUM);
    (td_void)memcpy_s(&sync_cfg_node->awb_reg_cfg.color_matrix[0], sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE,
        &alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_color_matrix[0], sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE);
    (td_void)memcpy_s(&sync_cfg_node->drc_reg_cfg, sizeof(isp_drc_sync_cfg), &alg_reg_cfg->drc_reg_cfg.sync_reg_cfg,
        sizeof(isp_drc_sync_cfg));
    (td_void)memcpy_s(&sync_cfg_node->wdr_reg_cfg, sizeof(isp_fswdr_sync_cfg), &alg_reg_cfg->wdr_reg_cfg.sync_reg_cfg,
        sizeof(isp_fswdr_sync_cfg));
    (td_void)memcpy_s(&sync_cfg_node->be_blc_reg_cfg, sizeof(isp_be_blc_dyna_cfg), &alg_reg_cfg->be_blc_cfg.dyna_blc,
        sizeof(isp_be_blc_dyna_cfg));
    (td_void)memcpy_s(&sync_cfg_node->fe_blc_reg_cfg, sizeof(isp_fe_blc_dyna_cfg), &alg_reg_cfg->fe_blc_cfg.dyna_blc,
        sizeof(isp_fe_blc_dyna_cfg));
    (td_void)memcpy_s(&sync_cfg_node->dynamic_blc_cfg, sizeof(isp_dynamic_blc_sync_cfg),
        &alg_reg_cfg->dynamic_blc_reg_cfg.sync_cfg, sizeof(isp_dynamic_blc_sync_cfg));
    (td_void)memcpy_s(&sync_cfg_node->fpn_cfg, sizeof(isp_fpn_sync_cfg), &alg_reg_cfg->fpn_reg_cfg.sync_cfg,
        sizeof(isp_fpn_sync_cfg));
    sync_cfg_node->awb_reg_cfg.be_awb_switch = alg_reg_cfg->awb_reg_cfg.awb_reg_usr_cfg.be_awb_switch;
}
static td_void isp_main_pipe_stitch_sync_cfg_get(const isp_usr_ctx *isp_ctx, isp_sync_cfg_buf_node *sync_cfg_node)
{
    td_u8 i, stitch_idx;
    ot_vi_pipe stitch_pipe;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    isp_awb_reg_dyn_cfg *awb_reg = TD_NULL;

    for (stitch_idx = 0; stitch_idx < isp_ctx->stitch_attr.stitch_pipe_num; stitch_idx++) {
        stitch_pipe = isp_ctx->stitch_attr.stitch_bind_id[stitch_idx];
        isp_regcfg_get_ctx(stitch_pipe, reg_cfg);
        if (reg_cfg == TD_NULL) {
            continue;
        }
        awb_reg = &reg_cfg->reg_cfg.alg_reg_cfg[0].awb_reg_cfg.awb_reg_dyn_cfg;
        for (i = 0; i < OT_ISP_BAYER_CHN_NUM; i++) {
            sync_cfg_node->awb_reg_cfg_stitch[stitch_idx].be_white_balance_gain[i] = awb_reg->be_white_balance_gain[i];
        }

        for (i = 0; i < OT_ISP_CCM_MATRIX_SIZE; i++) {
            sync_cfg_node->awb_reg_cfg_stitch[stitch_idx].color_matrix[i] = awb_reg->be_color_matrix[i];
        }

        (td_void)memcpy_s(&sync_cfg_node->fe_blc_reg_cfg_stitch[stitch_idx], sizeof(isp_fe_blc_dyna_cfg),
            &reg_cfg->reg_cfg.alg_reg_cfg[0].fe_blc_cfg.dyna_blc, sizeof(isp_fe_blc_dyna_cfg));
        (td_void)memcpy_s(&sync_cfg_node->be_blc_reg_cfg_stitch[stitch_idx], sizeof(isp_be_blc_dyna_cfg),
            &reg_cfg->reg_cfg.alg_reg_cfg[0].be_blc_cfg.dyna_blc, sizeof(isp_be_blc_dyna_cfg));
    }
}

static td_void isp_stitch_sync_cfg_get(ot_vi_pipe vi_pipe, ot_isp_sns_regs_info *sns_regs_info,
    isp_alg_reg_cfg *alg_reg_cfg, isp_sync_cfg_buf_node *sync_cfg_node)
{
    td_s8 stitch_main_pipe;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg_s = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (isp_ctx->stitch_attr.stitch_enable != TD_TRUE) {
        return;
    }

    stitch_main_pipe = isp_ctx->stitch_attr.stitch_bind_id[0];
    if (is_stitch_main_pipe(vi_pipe, stitch_main_pipe)) {
        isp_main_pipe_stitch_sync_cfg_get(isp_ctx, sync_cfg_node);
        return;
    }

    isp_regcfg_get_ctx(stitch_main_pipe, reg_cfg_s);
    if (reg_cfg_s == TD_NULL) {
        return;
    }

    (td_void)memcpy_s(&sync_cfg_node->sns_regs_info, sizeof(ot_isp_sns_regs_info),
        &reg_cfg_s->sync_cfg_node.sns_regs_info, sizeof(ot_isp_sns_regs_info));
    (td_void)memcpy_s(&sync_cfg_node->sns_regs_info.com_bus, sizeof(ot_isp_sns_commbus), &sns_regs_info->com_bus,
        sizeof(ot_isp_sns_commbus));
    (td_void)memcpy_s(&sync_cfg_node->sns_regs_info.slv_sync.slave_bind_dev, sizeof(td_u32),
        &sns_regs_info->slv_sync.slave_bind_dev, sizeof(td_u32));
    (td_void)memcpy_s(&sync_cfg_node->ae_reg_cfg, sizeof(isp_ae_reg_cfg_2),
        &reg_cfg_s->reg_cfg.alg_reg_cfg[0].ae_reg_cfg2, sizeof(isp_ae_reg_cfg_2));
    (td_void)memcpy_s(&sync_cfg_node->awb_reg_cfg.color_matrix[0], sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE,
        &alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_color_matrix[0], sizeof(td_u16) * OT_ISP_CCM_MATRIX_SIZE);
    (td_void)memcpy_s(&sync_cfg_node->awb_reg_cfg.be_white_balance_gain[0], sizeof(td_u32) * OT_ISP_BAYER_CHN_NUM,
        &alg_reg_cfg->awb_reg_cfg.awb_reg_dyn_cfg.be_white_balance_gain[0], sizeof(td_u32) * OT_ISP_BAYER_CHN_NUM);
    sync_cfg_node->awb_reg_cfg.be_awb_switch = alg_reg_cfg->awb_reg_cfg.awb_reg_usr_cfg.be_awb_switch;
}

td_s32 isp_sync_cfg_set(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_s32 pipe_st = 0;
    td_s32 pipe_ed = 0;
    ot_vi_pipe pipe_tmp = vi_pipe;

    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    ot_isp_sns_regs_info *sns_regs_info = TD_NULL;

    isp_get_ctx(pipe_tmp, isp_ctx);

    isp_get_start_and_end_vipipe(pipe_tmp, isp_ctx, &pipe_st, &pipe_ed);

    while (pipe_st <= pipe_ed) {
        if (isp_ctx->stitch_attr.stitch_enable == TD_TRUE) {
            pipe_tmp = isp_ctx->stitch_attr.stitch_bind_id[pipe_st];
        } else {
            pipe_tmp = pipe_st;
        }

        isp_get_ctx(pipe_tmp, isp_ctx);
        isp_regcfg_get_ctx(pipe_tmp, reg_cfg);
        isp_check_pointer_return(reg_cfg);
        isp_check_open_return(pipe_tmp);

        if (isp_sensor_update_sns_reg(pipe_tmp) != TD_SUCCESS) {
            /* If Users need to config AE sync info themselves, they can set pfn_cmos_get_sns_reg_info
             * to TD_NULL in cmos.c
             */
            /* Then there will be NO AE sync configs in kernel of firmware */
            return TD_SUCCESS;
        }

        isp_sensor_get_sns_reg(pipe_tmp, &sns_regs_info);

        isp_normal_sync_cfg_get(pipe_tmp, sns_regs_info, &reg_cfg->reg_cfg.alg_reg_cfg[0], &reg_cfg->sync_cfg_node);
        isp_stitch_sync_cfg_get(pipe_tmp, sns_regs_info, &reg_cfg->reg_cfg.alg_reg_cfg[0], &reg_cfg->sync_cfg_node);

        reg_cfg->sync_cfg_node.valid = TD_TRUE;
        ret = ioctl(isp_get_fd(pipe_tmp), ISP_SYNC_CFG_SET, &reg_cfg->sync_cfg_node);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        sns_regs_info->config = TD_TRUE;

        pipe_st++;
    }

    return TD_SUCCESS;
}
#ifdef CONFIG_OT_SNAP_SUPPORT
td_bool isp_pro_trigger_get(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_bool enable;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_PRO_TRIGGER_GET, &enable);
    if (ret != TD_SUCCESS) {
        return TD_FALSE;
    }

    return enable;
}
#endif
td_s32 isp_reg_cfg_ctrl(ot_vi_pipe vi_pipe)
{
    td_u8 i;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;
    isp_get_ctx(vi_pipe, isp_ctx);
    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);
    reg_cfg->reg_cfg.cfg_key.key = 0xFFFFFFFFFFFFFFFF;
    for (i = isp_ctx->block_attr.pre_block_num; i < isp_ctx->block_attr.block_num; i++) {
        (td_void)memcpy_s(&reg_cfg->reg_cfg.alg_reg_cfg[i], sizeof(isp_alg_reg_cfg), &reg_cfg->reg_cfg.alg_reg_cfg[0],
            sizeof(isp_alg_reg_cfg));
    }

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        for (i = 0; i < MIN2(ISP_MAX_BE_NUM, isp_ctx->block_attr.block_num); i++) {
            isp_ctx->special_opt.be_on_stt_update[i] = TD_TRUE;
        }

        for (i = isp_ctx->block_attr.pre_block_num; i < isp_ctx->block_attr.block_num; i++) {
            isp_be_resh_cfg(&reg_cfg->reg_cfg.alg_reg_cfg[i]);
        }
    }

    reg_cfg->reg_cfg.cfg_num = isp_ctx->block_attr.block_num;

    return TD_SUCCESS;
}

static td_void isp_gamma_lut_apb_reg_config(isp_post_be_reg_type *be_reg, isp_gamma_usr_cfg *usr_reg_cfg)
{
    td_u16 j;
    isp_gamma_lut_waddr_write(be_reg, 0);

    for (j = 0; j < GAMMA_REG_NODE_NUM; j++) {
        isp_gamma_lut_wdata_write(be_reg, usr_reg_cfg->gamma_lut[j]);
    }
}

static td_void isp_sharpen_lut_apb_reg_config(isp_post_be_reg_type *be_reg, isp_sharpen_mpi_dyna_reg_cfg *dyna)
{
    td_u16 j;
    isp_sharpen_mhfgaind_waddr_write(be_reg, 0);
    isp_sharpen_mhfgainud_waddr_write(be_reg, 0);

    for (j = 0; j < SHRP_GAIN_LUT_SIZE; j++) {
        isp_sharpen_mhfgaind_wdata_write(be_reg, (dyna->hf_gain_d[j] << 12) + dyna->mf_gain_d[j]);         /* 12 */
        isp_sharpen_mhfgainud_wdata_write(be_reg, (dyna->hf_gain_ud[j] << 12) + dyna->mf_gain_ud[j]);      /* 12 */
    }
}

static td_void isp_ldci_lut_apb_reg_config(isp_post_be_reg_type *be_reg, isp_ldci_dyna_cfg *dyna_reg_cfg)
{
    td_u16 j;
    isp_ldci_cgain_waddr_write(be_reg, 0);
    isp_ldci_de_usm_waddr_write(be_reg, 0);
    isp_ldci_he_waddr_write(be_reg, 0);

    for (j = 0; j < LDCI_COLOR_GAIN_LUT_SIZE; j++) {
        isp_ldci_cgain_wdata_write(be_reg, dyna_reg_cfg->color_gain_lut[j]);
    }

    for (j = 0; j < LDCI_DE_USM_LUT_SIZE; j++) {
        isp_ldci_de_usm_wdata_write(be_reg, dyna_reg_cfg->usm_pos_lut[j], dyna_reg_cfg->usm_neg_lut[j]);
    }

    for (j = 0; j < LDCI_HE_LUT_SIZE; j++) {
        isp_ldci_he_wdata_write(be_reg, dyna_reg_cfg->he_pos_lut[j], dyna_reg_cfg->he_neg_lut[j]);
    }
}

static td_s32 isp_lut_apb_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg_attr *reg_cfg)
{
    td_u8 i;
    isp_post_be_reg_type *be_reg = TD_NULL;
    isp_alg_reg_cfg *alg_reg_cfg = TD_NULL;

    for (i = 0; i < reg_cfg->reg_cfg.cfg_num; i++) {
        alg_reg_cfg = &reg_cfg->reg_cfg.alg_reg_cfg[i];
        be_reg = (isp_post_be_reg_type *)isp_get_post_be_vir_addr(vi_pipe, i);

        isp_check_pointer_return(be_reg);

        /* gamma */
        isp_gamma_lut_apb_reg_config(be_reg, &alg_reg_cfg->gamma_cfg.usr_reg_cfg);

        /* sharpen */
        isp_sharpen_lut_apb_reg_config(be_reg, &alg_reg_cfg->sharpen_reg_cfg.dyna_reg_cfg.mpi_dyna_reg_cfg);

        /* LDCI */
        isp_ldci_lut_apb_reg_config(be_reg, &alg_reg_cfg->ldci_reg_cfg.dyna_reg_cfg);

        /* csp */
        if (reg_cfg->reg_cfg.cfg_key.bit1_csp_cfg && alg_reg_cfg->drc_reg_cfg.usr_reg_cfg.csp_lut_update_en &&
            alg_reg_cfg->drc_reg_cfg.usr_reg_cfg.csp_en) {
            isp_csp_lut_apb_config(be_reg, &alg_reg_cfg->drc_reg_cfg.usr_reg_cfg);
        }
    }

    return TD_SUCCESS;
}

static td_void isp_lblc_lut_apb_config(isp_fe_reg_type *fe_reg, isp_lblc_usr_cfg *usr_reg_cfg)
{
    td_u32 i;
    fe_reg->isp_lblc_rgain_waddr.u32 = 0;
    fe_reg->isp_lblc_grgain_waddr.u32 = 0;
    fe_reg->isp_lblc_gbgain_waddr.u32 = 0;
    fe_reg->isp_lblc_bgain_waddr.u32 = 0;

    for (i = 0; i < OT_ISP_LBLC_GRID_POINTS; i++) {
        fe_reg->isp_lblc_rgain_wdata.u32 = usr_reg_cfg->mesh_blc_r[i] & 0x3FFF;  /* 4 * i: r_gain */
        fe_reg->isp_lblc_grgain_wdata.u32 = usr_reg_cfg->mesh_blc_gr[i] & 0x3FFF; /* 4 * i + 1: gr_gain */
        fe_reg->isp_lblc_gbgain_wdata.u32 = usr_reg_cfg->mesh_blc_gb[i] & 0x3FFF;  /* 4 * i + 2: b_gain */
        fe_reg->isp_lblc_bgain_wdata.u32 = usr_reg_cfg->mesh_blc_b[i] & 0x3FFF; /* 4 * i + 3: gb_gain */
    }

    return;
}


static td_s32 isp_fe_lut_apb_reg_config(ot_vi_pipe vi_pipe, isp_reg_cfg_attr *reg_cfg)
{
    isp_fe_reg_type *fe_reg = TD_NULL;
    isp_alg_reg_cfg *alg_reg_cfg = TD_NULL;
    isp_check_no_fe_pipe_return(vi_pipe);

    fe_reg = (isp_fe_reg_type *)isp_get_fe_vir_addr(vi_pipe);
    isp_check_pointer_return(fe_reg);
    // no striping, fe do not split
    alg_reg_cfg = &reg_cfg->reg_cfg.alg_reg_cfg[0];
    // apb config, the first frame need to config lut by apb, because of there's no blanking zone after a frame
    if (alg_reg_cfg->lblc_reg_cfg.lblc_en == TD_TRUE) {
        isp_lblc_lut_apb_config(fe_reg, &alg_reg_cfg->lblc_reg_cfg.usr_reg_cfg);
    }
    return TD_SUCCESS;
}

td_s32 isp_switch_reg_set(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        isp_regcfg_get_ctx(vi_pipe, reg_cfg);
        isp_check_pointer_return(reg_cfg);
        isp_lut_apb_reg_config(vi_pipe, reg_cfg);
        ret = isp_reg_cfg_info_init(vi_pipe);
        if (ret != TD_SUCCESS) {
            isp_err_trace("vi_pipe %d isp_reg_cfg_info_init failed \n", vi_pipe);
        }
        if (reg_cfg->reg_cfg.kernel_reg_cfg.cfg_key.key) {
            ret = ioctl(isp_get_fd(vi_pipe), ISP_REG_CFG_SET, &reg_cfg->reg_cfg.kernel_reg_cfg);
            if (ret != TD_SUCCESS) {
                isp_err_trace("Config ISP register Failed with ec %#x!\n", ret);
                return ret;
            }
        }

        isp_ctx->block_attr.pre_block_num = isp_ctx->block_attr.block_num;

        return TD_SUCCESS;
    }

    /* record the register config information to fhy and kernel,and be valid in next frame. */
    ret = isp_reg_cfg_info_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    isp_ctx->para_rec.stitch_sync = TD_TRUE;
    ret = ioctl(isp_get_fd(vi_pipe), ISP_SYNC_INIT_SET, &isp_ctx->para_rec.stitch_sync);
    if (ret != TD_SUCCESS) {
        isp_ctx->para_rec.stitch_sync = TD_FALSE;
        isp_err_trace("ISP[%d] set isp stitch sync failed!\n", vi_pipe);
    }

    ret = isp_all_cfgs_be_buf_init(vi_pipe);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe:%d init all be bufs failed \n", vi_pipe);
        return ret;
    }

    isp_ctx->block_attr.pre_block_num = isp_ctx->block_attr.block_num;

    return TD_SUCCESS;
}

td_s32 isp_lut2stt_apb_reg(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    isp_usr_ctx *isp_ctx = TD_NULL;
    isp_reg_cfg_attr *reg_cfg = TD_NULL;

    isp_regcfg_get_ctx(vi_pipe, reg_cfg);
    isp_check_pointer_return(reg_cfg);
    isp_get_ctx(vi_pipe, isp_ctx);

    /* config lut need before sensor init */
    if (is_online_mode(isp_ctx->block_attr.running_mode)) {
        ret = isp_lut_apb_reg_config(vi_pipe, reg_cfg);
        if (ret != TD_SUCCESS) {
            isp_err_trace("pipe:%d init all be bufs failed \n", vi_pipe);
            return ret;
        }
    }
    ret = isp_fe_lut_apb_reg_config(vi_pipe, reg_cfg);
    if (ret != TD_SUCCESS) {
        isp_err_trace("pipe:%d init fe lut bufs failed \n", vi_pipe);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_switch_state_set(ot_vi_pipe vi_pipe)
{
    td_s32 ret;

    ret = ioctl(isp_get_fd(vi_pipe), ISP_BE_SWITCH_FINISH_STATE_SET);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}
