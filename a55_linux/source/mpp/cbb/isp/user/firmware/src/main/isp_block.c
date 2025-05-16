/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_block.h"
#include <sys/ioctl.h>
#include "mkp_isp.h"
#include "ot_isp_debug.h"
#include "isp_main.h"
#include "isp_ext_config.h"

static td_s32 isp_check_running_mode_block_num(ot_vi_pipe vi_pipe, isp_running_mode running_mode, td_u8 block_num)
{
    switch (running_mode) {
        case ISP_MODE_RUNNING_OFFLINE:
        case ISP_MODE_RUNNING_ONLINE:
        case ISP_MODE_RUNNING_PRE_ONLINE_POST_OFFLINE:
            if (block_num != ISP_NORMAL_BLOCK_NUM) {
                isp_err_trace("vi_pipe :%d,when isp_running_mode = %d,block_num should be equal to %d!\n",
                              vi_pipe, running_mode, ISP_NORMAL_BLOCK_NUM);
                return TD_FAILURE;
            }
            break;
        case ISP_MODE_RUNNING_STRIPING:
            if (block_num < MIN_SPLIT_NUM) {
                isp_err_trace("vi_pipe :%d,when isp_running_mode = %d,block_num should not be less than %d!\n",
                              vi_pipe, running_mode, MIN_SPLIT_NUM);
                return TD_FAILURE;
            }
            break;
        default:
            break;
    }
    return TD_SUCCESS;
}

static td_s32 isp_get_block_hw_attr(ot_vi_pipe vi_pipe, isp_block_attr *block)
{
    td_s32 ret, isp_fd;
    isp_block_attr blk_attr = { 0 };

    isp_fd = isp_get_fd(vi_pipe);

    ret = ioctl(isp_fd, ISP_WORK_MODE_INIT, &blk_attr);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ret:%d!\n", ret);
        return ret;
    }

    isp_check_running_mode_return(blk_attr.running_mode);
    isp_check_block_num_return(blk_attr.block_num);
    ret = isp_check_running_mode_block_num(vi_pipe, blk_attr.running_mode, blk_attr.block_num);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    block->running_mode = blk_attr.running_mode;
    block->block_num    = blk_attr.block_num;
    block->over_lap     = blk_attr.over_lap;

    block->frame_rect.width  = blk_attr.frame_rect.width;
    block->frame_rect.height = blk_attr.frame_rect.height;

    (td_void)memcpy_s(block->block_rect, sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM,
                      blk_attr.block_rect, sizeof(ot_rect) * OT_ISP_STRIPING_MAX_NUM);

    ot_ext_system_be_total_width_write(vi_pipe,  block->frame_rect.width);
    ot_ext_system_be_total_height_write(vi_pipe, block->frame_rect.height);

    return TD_SUCCESS;
}

td_s32 isp_block_init(ot_vi_pipe vi_pipe, isp_block_attr *block)
{
    td_s32 ret;
    td_s32 isp_fd = isp_get_fd(vi_pipe);

    ret = isp_get_block_hw_attr(vi_pipe, block);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get isp block HW attr failed!\n");
        return ret;
    }

    block->pre_block_num = block->block_num;

    ret = ioctl(isp_fd, ISP_PRE_BLK_NUM_UPDATE, &block->pre_block_num);
    if (ret != TD_SUCCESS) {
        isp_err_trace("ISP[%d]:update pre block num failed\n", vi_pipe);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_block_update(ot_vi_pipe vi_pipe, isp_block_attr *block)
{
    td_s32 ret;

    ret = isp_get_block_hw_attr(vi_pipe, block);
    if (ret != TD_SUCCESS) {
        isp_err_trace("get isp block HW attr failed!\n");
        return ret;
    }

    return TD_SUCCESS;
}

td_void isp_block_exit(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_s32 isp_fd = isp_get_fd(vi_pipe);

    ret = ioctl(isp_fd, ISP_WORK_MODE_EXIT);
    if (ret != TD_SUCCESS) {
        isp_err_trace("isp block exit failed!\n");
        return;
    }
}

td_u32 isp_get_block_rect(isp_rect *block_rect, isp_block_attr *block, td_u8 block_id)
{
    isp_check_block_id_return(block_id);

    if (block->block_num == 1) {
        block_rect->x      = 0;
        block_rect->y      = 0;
        block_rect->width  = block->frame_rect.width;
        block_rect->height = block->frame_rect.height;

        return TD_SUCCESS;
    }

    block_rect->x      = block->block_rect[block_id].x;
    block_rect->y      = block->block_rect[block_id].y;
    block_rect->width  = block->block_rect[block_id].width;
    block_rect->height = block->block_rect[block_id].height;

    return TD_SUCCESS;
}
