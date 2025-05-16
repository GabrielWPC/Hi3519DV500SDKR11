/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_SVP_NPU_PROCESS_H
#define SAMPLE_SVP_NPU_PROCESS_H

#include "ot_type.h"

/* function : show the sample of acl resnet50 */
td_void sample_svp_npu_acl_resnet50(td_void);

/* function : show the sample of acl resnet50_multithread */
td_void sample_svp_npu_acl_resnet50_multi_thread(td_void);

/* function : show the sample of resnet50 dyanamic batch with mem cached */
td_void sample_svp_npu_acl_resnet50_dynamic_batch(td_void);

/* function : show the sample of lstm */
td_void sample_svp_npu_acl_lstm(td_void);

/* function : show the sample of rfcn */
td_void sample_svp_npu_acl_rfcn(td_void);

/* function : show the sample of event */
td_void sample_svp_npu_acl_event(td_void);

/* function : show the sample of preemption */
td_void sample_svp_npu_acl_preemption(td_void);

/* function : show the sample of aipp */
td_void sample_svp_npu_acl_aipp(td_void);

/* function : show the sample of sign handle */
td_void sample_svp_npu_acl_handle_sig(td_void);

#endif
