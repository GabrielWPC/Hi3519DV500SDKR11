/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_MPI_ISP_INNER_H
#define OT_MPI_ISP_INNER_H

#include "ot_type.h"
#include "ot_common_aiisp.h"
#include "ot_common_isp_inner.h"
#ifdef CONFIG_OT_SNAP_SUPPORT
#include "ot_common_snap.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    td_s32 (*isp_register_alg_init)(ot_vi_pipe vi_pipe, td_void *reg_cfg);
    td_s32 (*isp_register_alg_run)(ot_vi_pipe vi_pipe, const td_void *stat_info, td_void *reg_cfg, td_s32 rsv);
    td_s32 (*isp_register_alg_ctrl)(ot_vi_pipe vi_pipe, td_u32 cmd, td_void *value);
    td_s32 (*isp_register_alg_exit)(ot_vi_pipe vi_pipe);
}isp_register_alg_func;

#ifdef CONFIG_OT_SNAP_SUPPORT
td_s32 mpi_isp_set_snap_attr(ot_vi_pipe vi_pipe, const ot_snap_attr *snap_attr, const isp_snap_pipe *snap_pipe);
#endif

td_s32 mpi_isp_set_ai_mode_attr(ot_vi_pipe vi_pipe, const ot_isp_ai_mode ai_mode, const td_bool blend_en);
td_s32 mpi_isp_get_ai_mode_attr(ot_vi_pipe vi_pipe, ot_isp_ai_mode *ai_mode, td_bool *blend_en);

td_s32 mpi_isp_register_alg(ot_vi_pipe vi_pipe, td_u32 alg_id, const isp_register_alg_func *funcs);

td_s32 mpi_isp_get_master_pipe(ot_vi_pipe vi_pipe, ot_vi_pipe *master_pipe);
td_u32 mpi_isp_get_pipe_index(ot_vi_pipe vi_pipe);

td_s32 mpi_isp_enable_aiisp(ot_vi_pipe vi_pipe, ot_aiisp_type aiisp_type);
td_s32 mpi_isp_disable_aiisp(ot_vi_pipe vi_pipe, ot_aiisp_type aiisp_type);

td_s32 mpi_isp_set_aidrc_mode(ot_vi_pipe vi_pipe, td_bool is_advanced);
td_s32 mpi_isp_set_aidrc_dataflow_status(ot_vi_pipe vi_pipe, td_bool dataflow_en);
td_s32 mpi_isp_set_drc_blend(ot_vi_pipe vi_pipe, const isp_drc_blend *blend);
td_s32 mpi_isp_get_drc_blend(ot_vi_pipe vi_pipe, isp_drc_blend *blend);
td_s32 mpi_isp_set_ynet_cfg(ot_vi_pipe vi_pipe, const ot_isp_nr_ynet_attr *ynet_cfg);
td_s32 mpi_isp_get_ynet_cfg(ot_vi_pipe vi_pipe, ot_isp_nr_ynet_attr *ynet_cfg);
td_s32 mpi_isp_set_binary_enable(ot_vi_pipe vi_pipe, td_bool binary_en);
td_s32 mpi_isp_set_check_sum_enable(ot_vi_pipe vi_pipe, td_bool sum_en);
td_s32 mpi_isp_get_check_sum_stats(ot_vi_pipe vi_pipe, isp_check_sum_stat *sum_stat);
td_s32 mpi_isp_set_mesh_shading_gain_lut_attr(ot_vi_pipe vi_pipe,
    const ot_isp_shading_lut_attr *shading_lut_attr, td_bool inner_update);
td_s32 mpi_isp_set_mesh_shading_attr(ot_vi_pipe vi_pipe,
    const ot_isp_shading_attr *shading_attr, td_bool inner_update);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* OT_MPI_ISP_INNER_H */
