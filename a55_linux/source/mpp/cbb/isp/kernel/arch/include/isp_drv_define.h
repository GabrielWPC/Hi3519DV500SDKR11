/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef ISP_DRV_DEFINE_H
#define ISP_DRV_DEFINE_H

#include "isp.h"
#include "mkp_isp.h"
#include "isp_stt_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* 0x00F8 bit16: int0_isp0_en
   0x00F8 bit0: int0_pt0_en
   0x00F8 bit8: int0_ch0_en */
/* 0x00E8 bit16: int1_isp0_en
   0x00E8 bit0: int1_pt0_en
   0x00E8 bit8: int1_ch0_en */
#define VICAP_HD_INT0_MASK              0x00F8
#define VICAP_HD_INTTERRUPT0            0x00F0
#define VICAP_HD_INT1_MASK              0x00E8
#define VICAP_HD_INTTERRUPT1            0x00E0
#define vicap_int_mask_pt(vi_dev)        (1 << (vi_dev))
#define vicap_int_mask_isp(vi_pipe)      (0x10000 << (vi_pipe))

#define VI_WCH_MASK             0x00F8
#define VI_WCH_INT              0xF0
#define VI_WCH_INT_FSTART       (1<<0)
#define VI_WCH_INT_DELAY0       (1<<15)
#define VI_WCH_INT_LOW_DELAY    (1<<21)

#define vi_pt_base(vi_dev)      (0x1000 + (vi_dev) * 0x100)
#define VI_PT_INT_MASK          0xF8
#define VI_PT_INT               0xF0
#define VI_PT_INT_FSTART        (1<<0)
#define VI_PT_INT_ERR           ((1<<1) | (2<<1))
#define VI_PT_FSTART_DLY        0x60

#define VI_ISP_MODE             0x58
#define VI_ISP0_P2_EN_BIT       8

#define ISP_INT_FE_MASK         0x00F8
#define ISP_INT_FE              0x00F0
#define ISP_INT_FE_FSTART       (1 << 0)
#define ISP_INT_FE_DELAY        (1 << 3)
#define ISP_INT_FE_DYNABLC_END  (1 << 5)

#define VIPROC_INT_BE_MASK      0x00F0
#define VIPROC_INT_BE           0x0310
#define VIPROC_INT_BE_FSTART    (1<<16)

#define VI_PT0_ID               0xE0

#define CH_REG_NEWER            0x1004
#define ISP_FE_REG_NEWER        0x01EC
#define ISP_BE_REG_NEWER        0x01EC

/* 0x01A4: [31]:enable; [30]:mode; [29]:reset; [5:4]:vc_num; [3:2]:vc_num_max; [1:0]:vc_num_init_value; */
#define VC_NUM_ADDR             0x01A4

#define ISP_RESET               0x0078
#define CHN_SWITCH              0x10028

#define ISP_MAGIC_OFFSET        1
#define PROC_PRT_SLICE_SIZE     256

#define BUILT_IN_WDR_RATIO_VS_S 0x400
#define BUILT_IN_WDR_RATIO_S_M  0x400
#define BUILT_IN_WDR_RATIO_M_L  0x40

#define io_pt_address(x)                    ((td_uintptr_t)isp_drv_get_reg_vicap_base_va() + (x))
#define io_wch_address(vi_pipe, x)          ((td_uintptr_t)isp_drv_get_vicap_ch_base_va(vi_pipe) + (x))
#define io_fe_address(vi_pipe, x)           ((td_uintptr_t)isp_drv_get_ispfe_base_va(vi_pipe) + (x))

#define io_rw_pt_address(x)                 (*((volatile unsigned int *)io_pt_address(x)))
#define io_rw_fe_address(vi_pipe, x)        (*((volatile unsigned long *)io_fe_address(vi_pipe, x)))
#define io_rw_ch_address(vi_pipe, x)        (*((volatile unsigned long *)io_wch_address(vi_pipe, x)))

#define isp_drv_fereg_ctx(vi_pipe, ctx)      ctx = (isp_fe_reg_type *)(isp_drv_get_ispfe_base_va(vi_pipe))
#define isp_drv_festtreg_ctx(vi_pipe, ctx)   ctx = (isp_vicap_ch_reg_type *)(isp_drv_get_vicap_ch_base_va(vi_pipe))

td_s32  isp_drv_be_remap(void);
td_void isp_drv_be_unmap(void);

td_s32  isp_drv_vicap_remap(void);
td_void isp_drv_vicap_unmap(void);

td_s32  isp_drv_fe_remap(void);
td_void isp_drv_fe_unmap(void);

td_s32 isp_check_mod_param(ot_vi_pipe vi_pipe);

td_u8 isp_drv_get_block_num(ot_vi_pipe vi_pipe);

td_s32  isp_drv_fe_stitch_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info);

td_s32 isp_drv_be_apb_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key);
td_s32 isp_drv_be_stt_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat, isp_stat_key stat_key);

td_s32 isp_drv_be_offline_stitch_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info);

td_s32 isp_drv_be_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info);
td_void isp_drv_read_af_offline_stats_end_int(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, td_u8 blk_num,
                                              isp_stat_info *stat_info, isp_stat_key stat_key);
td_void isp_drv_be_offline_addr_info_update(ot_vi_pipe vi_pipe, td_u32 viproc_id, td_u64 pts,
    isp_be_offline_stt_buf *be_offline_stt_addr);
td_void isp_drv_switch_be_offline_stt_buf_index(ot_vi_pipe vi_pipe, td_u32 viproc_id,
    isp_drv_ctx *drv_ctx, td_u32 stat_valid);
td_s32 isp_drv_stitch_sync_be_stt_info(ot_vi_pipe vi_pipe, td_u32 viproc_id, isp_drv_ctx *drv_ctx);
td_u8 isp_drv_get_fe_sync_index(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx, td_bool is_first);
td_u8 isp_drv_get_pre_be_sync_index(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx);

td_s32 isp_drv_reg_config_chn_sel(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_wdr(ot_vi_pipe vi_pipe, isp_fswdr_sync_cfg *wdr_reg_cfg, td_u32 *ratio);
td_s32 isp_drv_reg_config_ldci(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_drc(ot_vi_pipe vi_pipe, isp_drc_sync_cfg *drc_reg_cfg);

td_s32 isp_drv_reg_config_fe_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_fe_dgain(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_be_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_sync_cfg_buf_node *cfg_node,
                                 isp_sync_cfg_buf_node *pre_be_cfg_node);

td_s32 isp_drv_reg_config_4dgain(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_sync_4dgain_cfg *sync_4dgain_cfg);

td_void isp_drv_set_input_sel(ot_vi_pipe vi_pipe, td_u32 *input_sel, td_u8 length);
td_void isp_drv_reg_config_sync_awb_ccm(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx,
                                        td_u8 cfg_node_idx, td_u8 cfg_node_vc);

td_s32 isp_drv_fe_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info);

td_s32 isp_drv_fe_stt_addr_init(ot_vi_pipe vi_pipe);
td_s32 isp_drv_fe_stitch_stt_addr_init(ot_vi_pipe vi_pipe);

td_s32 isp_drv_stt_buf_init(ot_vi_pipe vi_pipe);
td_s32 isp_drv_stt_buf_exit(ot_vi_pipe vi_pipe);

td_s32 isp_drv_get_p2_en_info(ot_vi_pipe vi_pipe, td_bool *p2_en);
td_s32 isp_drv_fe_stitch_non_statistics_read(ot_vi_pipe vi_pipe, isp_stat_info *stat_info);
td_s32 isp_drv_af_stats_kernel_get(ot_vi_pipe vi_pipe, ot_isp_drv_af_statistics *focus_stat);
td_s32 isp_drv_ae_stats_kernel_get(ot_vi_pipe vi_pipe, ot_isp_ae_stats *ae_stat);

td_s32 isp_drv_stitch_sync_ctrl_init(ot_vi_pipe vi_pipe);
td_s32 isp_drv_get_stitch_be_sync_para(ot_vi_pipe vi_pipe, isp_be_sync_para *be_sync_para);
td_s32 isp_drv_get_stitch_be_sync_para_specify(ot_vi_pipe vi_pipe, isp_be_sync_para *be_sync_para,
    const ot_video_frame_info *frame_info);
td_s32 isp_drv_reset_fe_cfg(ot_vi_pipe vi_pipe);

td_s32 isp_drv_ldci_offline_statistics_read(ot_vi_pipe vi_pipe, isp_stat *stat);
td_s32 isp_drv_ldci_online_attr_update(ot_vi_pipe vi_pipe, isp_stat *stat);
td_void isp_drv_set_ldci_write_stt_addr_offline(isp_be_all_reg_type *be_reg_cfg, td_phys_addr_t write_addr);
td_void isp_drv_set_ldci_read_stt_addr(isp_viproc_reg_type *viproc_reg, td_phys_addr_t read_addr);

td_s32 isp_drv_update_ldci_tpr_stt_addr(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx, isp_be_wo_reg_cfg *be_cfg);
td_s32 isp_drv_update_ldci_normal_offline_attr(ot_vi_pipe vi_pipe);
td_s32 isp_drv_set_ldci_stt_addr(ot_vi_pipe vi_pipe, td_phys_addr_t read_stt_addr, td_phys_addr_t write_stt_addr);

td_u8 isp_drv_get_be_sync_index(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx);
td_u8 isp_get_drc_comp_sync_index(ot_vi_pipe vi_pipe, const isp_drv_ctx *drv_ctx);
td_void isp_drv_reg_config_bnr_offline(isp_be_wo_reg_cfg *be_cfg, isp_drv_ctx *drv_ctx);
td_void isp_drv_reg_config_vi_fpn_offline(isp_be_wo_reg_cfg *be_cfg, const isp_drv_ctx *drv_ctx);
td_s32 isp_drv_update_be_offline_addr_info(ot_vi_pipe vi_pipe, isp_be_wo_cfg_buf *be_cfg_buf);

isp_drv_ctx *isp_drv_get_ctx(ot_vi_pipe vi_pipe);
osal_spinlock *isp_drv_get_lock(ot_vi_pipe vi_pipe);
osal_spinlock *isp_drv_get_sync_lock(ot_vi_pipe vi_pipe);

ot_isp_ob_stats_update_pos isp_drv_get_ob_stats_update_pos(ot_vi_pipe vi_pipe);
ot_isp_alg_run_select isp_drv_get_alg_run_select(ot_vi_pipe vi_pipe);
ot_isp_run_wakeup_select isp_drv_get_run_wakeup_sel(ot_vi_pipe vi_pipe);

td_s32 isp_drv_update_ldci_tpr_offline_stat(ot_vi_pipe vi_pipe, isp_be_wo_reg_cfg *be_cfg);

td_void isp_drv_be_cfg_buf_addr_init(ot_vi_pipe vi_pipe, isp_be_buf_node *node, td_s32 i,
    isp_mmz_buf_ex *be_buf_temp, td_u64 extend_size);
td_void *isp_drv_get_reg_vicap_base_va(td_void);
td_void *isp_drv_get_vicap_ch_base_va(ot_vi_pipe vi_pipe);
td_void *isp_drv_get_ispfe_base_va(ot_vi_pipe vi_pipe);

td_s32 isp_drv_set_be_sync_para_offline(ot_vi_pipe vi_pipe, td_void *be_node, isp_be_sync_para *be_sync_para,
    isp_aibnr_cfg *ainr_cfg);
td_s32 isp_drv_set_be_format(ot_vi_pipe vi_pipe, td_void *be_node, isp_be_format be_format);
td_bool isp_drv_is_stitch_sync_lost_frame(ot_vi_pipe vi_pipe, td_u32 viproc_id);
td_s32 isp_drv_set_isp_be_sync_param_online(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);

td_s32 isp_drv_reg_config_fe_dynamic_blc_single_pipe(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_be_dynamic_blc_single_pipe(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);
td_s32 isp_drv_reg_config_dynamic_blc(ot_vi_pipe vi_pipe, isp_drv_ctx *drv_ctx);

#ifdef CONFIG_OT_ISP_PQ_FOR_AI_SUPPORT
td_s32 isp_set_pq_ai_attr(pq_ai_attr *ai_attr);
td_s32 isp_get_pq_ai_attr(ot_vi_pipe vi_pipe, pq_ai_attr *ai_attr);
td_s32 isp_set_pq_ai_post_nr_attr(ot_vi_pipe vi_pipe, ot_pq_ai_noiseness_post_attr *noise_attr);
td_s32 isp_get_pq_ai_post_nr_attr(ot_vi_pipe vi_pipe, ot_pq_ai_noiseness_post_attr *noise_attr);
#endif

td_s32 isp_get_ctrl_param(ot_vi_pipe vi_pipe, ot_isp_ctrl_param *isp_ctrl_param);
td_s32 isp_get_mod_param(ot_isp_mod_param *mod_param);

td_void isp_drv_set_aibnr_bnr_cfg(isp_be_wo_reg_cfg *be_reg, isp_drv_ctx *drv_ctx, td_bool bnr_bypass);
td_void isp_drv_set_aidrc_drc_cfg(isp_be_wo_reg_cfg *be_reg, isp_drv_ctx *drv_ctx, isp_drc_sync_cfg* drc_reg_cfg);

#ifdef CONFIG_OT_SNAP_SUPPORT
td_s32 isp_get_preview_dcf_info(ot_vi_pipe vi_pipe, ot_isp_dcf_info *isp_dcf);
#endif

td_s32 isp_drv_set_aibnr_fpn_cor_cfg(ot_vi_pipe vi_pipe, isp_fpn_cor_reg_cfg *fpn_cor_cfg);
td_void isp_drv_get_ai_be_alg_pos(ot_vi_pipe vi_pipe, isp_vi_ai_work_param *work_param,
    isp_ai_alg_cfg *alg_cfg);
#ifdef CONFIG_OT_ISP_DETAIL_STATS_SUPPORT
td_void isp_drv_read_be_detail_stats(isp_drv_ctx *drv_ctx, isp_be_offline_detail_stat *stt_addr,
    ot_isp_detail_stats *detail_stats);
td_s32 isp_drv_detail_stats_buf_init(ot_vi_pipe vi_pipe);
td_s32 isp_drv_detail_stats_buf_exit(ot_vi_pipe vi_pipe);
td_void isp_drv_reset_detail_stats_cfg(ot_vi_pipe vi_pipe);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
