/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_scene_setparam.h"
#include "scene_setparam_inner.h"
#include <unistd.h>
#include <string.h>
#include "securec.h"
#include "ss_mpi_awb.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_venc.h"
#include "ot_common_rc.h"
#include "sample_comm.h"
#include "ss_mpi_mcf.h"
#include "ss_mpi_aidrc.h"
#include "ss_mpi_aibnr.h"

#include "ot_scenecomm.h"

#ifdef __cplusplus
extern "C" {
#endif

td_char g_dir_name[DIR_NAME_BUFF];

td_void set_dir_name(const td_char *dir_name)
{
    snprintf_truncated_s(g_dir_name, DIR_NAME_BUFF, "%s", dir_name);
}

td_s32 ot_scene_set_dynamic_3dnr(ot_vi_pipe vi_pipe, td_u32 iso, td_u8 index, ot_3dnr_pos_type pos3dnr)
{
    if (get_pipe_params()[index].module_state.dynamic_3dnr != TD_TRUE) {
        return TD_SUCCESS;
    }

    td_u32 iso_level = 0;
    ot_scene_3dnr nrx_attr;
    const ot_scene_static_3dnr *nrx_param = &(get_pipe_params()[index].static_threednr);
    td_u32 count = nrx_param->threed_nr_count;
    td_u32 *p_thresh = (td_u32 *)nrx_param->threed_nr_iso;
    iso_level = get_level_ltoh(iso, iso_level, count, p_thresh);
    if (iso_level == 0) {
        nrx_attr = nrx_param->threednr_value[0];
    } else {
        ot_scenecomm_expr_true_return(iso_level >= OT_SCENE_3DNR_MAX_COUNT, TD_FAILURE);
        td_u32 mid = iso;
        td_u32 left = p_thresh[iso_level - 1];
        td_u32 right = p_thresh[iso_level];

        nrx_attr = nrx_param->threednr_value[iso_level];

        _3dnr_nrx_param param = { &nrx_attr, nrx_param, iso_level, mid, left, right };

        scene_set_3dnr_nrx_adv_iey(param);

        scene_set_3dnr_nrx_adv_sfy(param);

        scene_set_3dnr_nrx_nry(param);

        scene_set_3dnr_nrx_iey(param);

        scene_set_3dnr_nrx_sfy(param);

        scene_set_3dnr_nrx_tfy(param, pos3dnr);

        scene_set_3dnr_nrx_mdy(param);

        scene_set_3dnr_nrx_nrc0(param);

        scene_set_3dnr_nrx_nrc1(param);
    }

    td_s32 ret = scene_set_3dnr(vi_pipe, &nrx_attr, index, pos3dnr);
    check_scene_ret(ret);
    return TD_SUCCESS;
}

td_s32 ot_scene_set_pause(td_bool pause)
{
    *(get_isp_pause()) = pause;
    return TD_SUCCESS;
}

td_s32 ot_scene_set_pipe_param(const ot_scene_pipe_param *pscene_pipe_param, td_u32 num)
{
    if (pscene_pipe_param == TD_NULL) {
        printf("null pointer");
        return TD_FAILURE;
    }

    errno_t ret = memcpy_s(get_pipe_params(), sizeof(ot_scene_pipe_param) * OT_SCENE_PIPETYPE_NUM, pscene_pipe_param,
        sizeof(ot_scene_pipe_param) * num);
    if (ret != EOK) {
        printf("copy scene pipe params fail. num = %u\n", num);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

#ifdef __cplusplus
}
#endif
