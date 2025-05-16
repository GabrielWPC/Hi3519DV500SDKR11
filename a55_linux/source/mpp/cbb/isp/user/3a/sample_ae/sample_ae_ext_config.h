/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_AE_EXT_CONFIG_H
#define OT_AE_EXT_CONFIG_H

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
 * 1. Ae Mode
 * 2. Analog Gain
 *
 * ------------------------------------------------------------------------------
 */
/* -----------------------------------------------------------------------s-------  */
/* Register: AE_MODE */
/* ------------------------------------------------------------------------------  */
#define OT_EXT_SYSTEM_AE_MODE_DEFAULT             0x0
#define OT_EXT_SYSTEM_AE_MODE_DATASIZE            1

/* args: data (1-bit) */
static __inline td_void ot_ext_system_ae_mode_write(td_u8 id, td_u8 data)
{
    iowr_8direct((ae_lib_vreg_base(id) + 0x0), data & 0x01);
}
static __inline td_u8 ot_ext_system_ae_mode_read(td_u8 id)
{
    return (iord_8direct(ae_lib_vreg_base(id) + 0x0) & 0x01);
}

/* ------------------------------------------------------------------------------  */
/* Register: AGAIN */
/* ------------------------------------------------------------------------------  */
#define OT_EXT_SYSTEM_QUERY_EXPOSURE_ISO_DEFAULT  0x00
#define OT_EXT_SYSTEM_QUERY_EXPOSURE_ISO_DATASIZE 32

/* args: data (32-bit) */
static __inline td_void ot_ext_system_query_exposure_again_write(td_u8 id, td_u32 data)
{
    iowr_32direct((ae_lib_vreg_base(id) + 0x4), data);
}
static __inline td_u32 ot_ext_system_query_exposure_again_read(td_u8 id)
{
    return  iord_32direct(ae_lib_vreg_base(id) + 0x4);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
