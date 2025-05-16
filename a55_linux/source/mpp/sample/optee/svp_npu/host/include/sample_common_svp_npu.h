/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef SAMPLE_COMMON_SVP_NPU_H
#define SAMPLE_COMMON_SVP_NPU_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "ss_mpi_sys.h"
#include "ot_common.h"
#include "ot_common_svp.h"
#include "sample_comm.h"
#include "svp_acl.h"
#include "svp_acl_mdl.h"

#define SAMPLE_SVP_NPU_MAX_THREAD_NUM    16
#define SAMPLE_SVP_NPU_MAX_TASK_NUM      16
#define SAMPLE_SVP_NPU_MAX_STREAM_NUM    16
#define SAMPLE_SVP_NPU_MAX_MODEL_NUM     2
#define SAMPLE_SVP_NPU_EXTRA_INPUT_NUM   2
#define SAMPLE_SVP_NPU_BYTE_BIT_NUM      8
#define SAMPLE_SVP_NPU_SHOW_TOP_NUM      5
#define SAMPLE_SVP_NPU_MAX_NAME_LEN      32
#define SAMPLE_SVP_NPU_MAX_MEM_SIZE      0xFFFFFFFF
#define SAMPLE_SVP_NPU_THRESHOLD_NUM     4

#define SAMPLE_SVP_NPU_RFCN_THRESHOLD_NUM      2
#define SAMPLE_SVP_NPU_AICPU_WAIT_TIME         1000
#define SAMPLE_SVP_NPU_RECT_COLOR              0x0000FF00
#define SAMPLE_SVP_NPU_MILLIC_SEC              20000
#define SAMPLE_SVP_NPU_IMG_THREE_CHN           3
#define SAMPLE_SVP_NPU_DOUBLE                  2

#define SAMPLE_SVP_NPU_PRIOR_TIMEOUT    1200

typedef enum {
    SAMPLE_SVP_NPU_PRIORITY_0 = 0,
    SAMPLE_SVP_NPU_PRIORITY_1 = 1,
    SAMPLE_SVP_NPU_PRIORITY_2 = 2,
    SAMPLE_SVP_NPU_PRIORITY_3 = 3,
    SAMPLE_SVP_NPU_PRIORITY_4 = 4,
    SAMPLE_SVP_NPU_PRIORITY_5 = 5,
    SAMPLE_SVP_NPU_PRIORITY_6 = 6,
    SAMPLE_SVP_NPU_PRIORITY_7 = 7,
    SAMPLE_SVP_NPU_PRIORITY_BUTT,
} sample_svp_npu_model_priority;

typedef struct {
    td_u32 priority;
    td_u32 prior_preemp_en;
    td_u32 prior_up_step_timeout;
    td_u32 prior_up_top_timeout;
} sample_svp_npu_model_prior_cfg;

typedef struct {
    td_u32 model_id;
    td_bool is_load_flag;
    td_ulong model_mem_size;
    td_void *model_mem_ptr;
    svp_acl_mdl_desc *model_desc;
    svp_acl_mdl_config_handle *handle;
    sample_svp_npu_model_prior_cfg prior_cfg;
    size_t input_num;
    size_t output_num;
    size_t dynamic_batch_idx;
} sample_svp_npu_model_info;

typedef struct {
    td_u32 max_batch_num;
    td_u32 dynamic_batch_num;
    td_u32 total_t;
    td_bool is_cached;
    td_u32 model_idx;
} sample_svp_npu_task_cfg;

typedef struct {
    sample_svp_npu_task_cfg cfg;
    svp_acl_mdl_dataset *input_dataset;
    svp_acl_mdl_dataset *output_dataset;
    td_void *task_buf_ptr;
    size_t task_buf_size;
    size_t task_buf_stride;
    td_void *work_buf_ptr;
    size_t work_buf_size;
    size_t work_buf_stride;
} sample_svp_npu_task_info;

typedef struct {
    td_float score;
    td_u32 class_id;
} sample_svp_npu_top_n_result;

typedef struct {
    td_char *num_name;
    td_char *roi_name;
    td_bool has_background;
    td_u32 roi_offset;
} sample_svp_npu_detection_info;

typedef struct {
    td_float nms_threshold;
    td_float score_threshold;
    td_float min_height;
    td_float min_width;
    td_char *name;
} sample_svp_npu_threshold;

typedef struct {
    td_u8 crop_switch;
    td_u8 padding_switch;
    td_u8 scf_switch;
    td_u8 preproc_switch;

    td_s32 crop_pos_x;
    td_s32 crop_pos_y;
    td_s32 crop_width;
    td_s32 crop_height;

    td_s32 padding_top;
    td_s32 padding_bottom;
    td_s32 padding_left;
    td_s32 padding_right;

    td_s32 scf_input_width;
    td_s32 scf_input_height;
    td_s32 scf_output_width;
    td_s32 scf_output_height;

    td_float reci_chn0;
    td_float reci_chn1;
    td_float reci_chn2;
    td_float reci_chn3;

    td_float mean_chn0;
    td_float mean_chn1;
    td_float mean_chn2;
    td_float mean_chn3;

    td_float min_chn0;
    td_float min_chn1;
    td_float min_chn2;
    td_float min_chn3;
} sample_aipp_dynamic_batch_param;

typedef struct {
    svp_acl_aipp_input_format input_format;
    td_s8 csc_switch;
    td_s8 rbuv_swap_switch;
    td_s8 ax_swap_switch;
    td_s32 image_width;
    td_s32 image_height;
    td_s32 csc_matrix_r0c0;
    td_s32 csc_matrix_r0c1;
    td_s32 csc_matrix_r0c2;
    td_s32 csc_matrix_r1c0;
    td_s32 csc_matrix_r1c1;
    td_s32 csc_matrix_r1c2;
    td_s32 csc_matrix_r2c0;
    td_s32 csc_matrix_r2c1;
    td_s32 csc_matrix_r2c2;
    td_u8 csc_output_bias_r0;
    td_u8 csc_output_bias_r1;
    td_u8 csc_output_bias_r2;
    td_u8 csc_input_bias_r0;
    td_u8 csc_input_bias_r1;
    td_u8 csc_input_bias_r2;
} sample_aipp_dynamic_param;

typedef struct {
    sample_aipp_dynamic_param dynamic_param;
    sample_aipp_dynamic_batch_param dynamic_batch_param;
} sample_svp_npu_model_aipp;

#define sample_svp_printf(level_str, msg, ...) \
do { \
    fprintf(stderr, "[level]:%s,[func]:%s [line]:%d [info]:"msg, level_str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)

#define sample_svp_printf_red(level_str, msg, ...) \
do { \
    fprintf(stderr, "\033[0;31m [level]:%s,[func]:%s [line]:%d [info]:"msg"\033[0;39m\n", level_str, __FUNCTION__, \
        __LINE__, ##__VA_ARGS__); \
} while (0)
/* system is unusable   */
#define sample_svp_trace_fatal(msg, ...)   sample_svp_printf_red("Fatal", msg, ##__VA_ARGS__)
/* action must be taken immediately */
#define sample_svp_trace_alert(msg, ...)   sample_svp_printf_red("Alert", msg, ##__VA_ARGS__)
/* critical conditions */
#define sample_svp_trace_critical(msg, ...)    sample_svp_printf_red("Critical", msg, ##__VA_ARGS__)
/* error conditions */
#define sample_svp_trace_err(msg, ...)     sample_svp_printf_red("Error", msg, ##__VA_ARGS__)
/* warning conditions */
#define sample_svp_trace_warning(msg, ...)    sample_svp_printf("Warning", msg, ##__VA_ARGS__)
/* normal but significant condition  */
#define sample_svp_trace_notic(msg, ...)  sample_svp_printf("Notice", msg, ##__VA_ARGS__)
/* informational */
#define sample_svp_trace_info(msg, ...)    sample_svp_printf("Info", msg, ##__VA_ARGS__)
/* debug-level messages  */
#define sample_svp_trace_debug(msg, ...)  sample_svp_printf("Debug", msg, ##__VA_ARGS__)

/* exps is true, goto */
#define sample_svp_check_exps_goto(exps, label, level, msg, ...)                  \
do {                                                                              \
    if ((exps)) {                                                                 \
        sample_svp_trace_err(msg, ## __VA_ARGS__);                                \
        goto label;                                                               \
    }                                                                             \
} while (0)

/* exps is true, return ret */
#define sample_svp_check_exps_return(exps, ret, level, msg, ...)                 \
do {                                                                             \
    if ((exps)) {                                                                \
        sample_svp_trace_err(msg, ##__VA_ARGS__);                                \
        return (ret);                                                            \
    }                                                                            \
} while (0)

/* acl init */
td_s32 sample_common_svp_npu_acl_init(const td_char *acl_config_path, td_s32 dev_id);
/* acl deinit */
td_void sample_common_svp_npu_acl_deinit(td_s32 dev_id);

#endif
