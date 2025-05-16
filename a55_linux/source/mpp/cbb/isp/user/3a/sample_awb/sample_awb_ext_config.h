/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_AWB_EXT_CONFIG_H
#define OT_AWB_EXT_CONFIG_H

#include "isp_vreg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*
 * ------------------------------------------------------------------------------
 *          these two entity illustrate how to use a ext register
 *
 * 1. Awb Mode
 * 2. Detected color temperature
 *
 * ------------------------------------------------------------------------------
 */
/* ------------------------------------------------------------------------------  */
/* Register: ot_ext_system_WB_type */
/* ------------------------------------------------------------------------------  */
/* white balance type, 0 auto, 1 manual */
/* ------------------------------------------------------------------------------  */
#define OT_EXT_SYSTEM_WB_TYPE_DEFAULT         0x0
#define OT_EXT_SYSTEM_WB_TYPE_DATASIZE        1

/* args: data (2-bit) */
static __inline td_void ot_ext_system_wb_type_write(td_u8 id, td_u8 data)
{
    iowr_8direct((awb_lib_vreg_base(id)), data);
}
static __inline td_u8 ot_ext_system_wb_type_read(td_u8 id)
{
    return (iord_8direct(awb_lib_vreg_base(id)) & 0x1);
}

/* ------------------------------------------------------------------------------  */
/* Register: ot_ext_system_wb_detect_temp */
/* ------------------------------------------------------------------------------  */
/* the detected color temperature */
/* ------------------------------------------------------------------------------  */
#define OT_EXT_SYSTEM_WB_DETECT_TEMP_DEFAULT  5000
#define OT_EXT_SYSTEM_WB_DETECT_TEMP_DATASIZE 16

/* args: data (16-bit) */
static __inline td_void ot_ext_system_wb_detect_temp_write(td_u8 id, td_u16 data)
{
    iowr_16direct((awb_lib_vreg_base(id) + 0x2), data);
}

static __inline td_u16 ot_ext_system_wb_detect_temp_read(td_u8 id)
{
    return iord_16direct(awb_lib_vreg_base(id) + 0x2);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
