/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_SAMPLE_GYRO_DIS_H
#define OT_SAMPLE_GYRO_DIS_H

#include "ot_type.h"
#include "ot_common_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

td_s32 sample_dis_ipc_gyro(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_dv_gyro(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_gyro_demo(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_gyro_ldc_switch(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_gyro_ldcv2_switch(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_ipc_gyro_two_sensor(ot_size *size);

td_void sample_dis_stop_gyro(td_void);

td_s32 sample_dis_gyro_rotation_compensation(ot_vo_intf_type vo_intf_type);

td_s32 sample_dis_send(ot_vo_intf_type vo_intf_type, ot_size* input_size);

td_s32 sample_dis_query_center_point(ot_vo_intf_type vo_intf_type);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of __cplusplus */

#endif /* OT_SAMPLE_GYRO_DIS_H */

