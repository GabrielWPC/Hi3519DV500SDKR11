/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_common_svp_npu_model.h"
#include <limits.h>
#include "svp_acl_rt.h"
#include "svp_acl_ext.h"

#define SAMPLE_SVP_NPU_EVEN 2

static sample_svp_npu_model_info g_svp_npu_model[SAMPLE_SVP_NPU_MAX_MODEL_NUM] = {0};

static td_s32 sample_svp_npu_get_model_base_info(td_u32 model_index)
{
    svp_acl_error ret;

    g_svp_npu_model[model_index].input_num = svp_acl_mdl_get_num_inputs(g_svp_npu_model[model_index].model_desc);
    sample_svp_check_exps_return(g_svp_npu_model[model_index].input_num < SAMPLE_SVP_NPU_EXTRA_INPUT_NUM + 1,
        TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "get input num failed!\n");

    g_svp_npu_model[model_index].output_num = svp_acl_mdl_get_num_outputs(g_svp_npu_model[model_index].model_desc);
    sample_svp_check_exps_return(g_svp_npu_model[model_index].output_num < 1,
        TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "get output num failed!\n");

    ret = svp_acl_mdl_get_input_index_by_name(g_svp_npu_model[model_index].model_desc,
        SVP_ACL_DYNAMIC_TENSOR_NAME, &g_svp_npu_model[model_index].dynamic_batch_idx);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get dynamic batch idx failed, model id is %u, error code is %d!\n", model_index, ret);

    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_get_line_num_and_line_byte_num(const sample_svp_npu_task_info *task, td_u32 idx,
    td_bool is_input, td_u32 *total_line_num, td_u32 *line_byte_num)
{
    td_s32 ret;
    td_u32 i;
    svp_acl_mdl_io_dims dims;
    svp_acl_data_type data_type;
    size_t data_size;
    svp_acl_aipp_info aipp_info = { 0 };

    if (is_input == TD_TRUE) {
        ret = svp_acl_mdl_get_input_dims(g_svp_npu_model[task->cfg.model_idx].model_desc, idx, &dims);
    } else {
        ret = svp_acl_mdl_get_output_dims(g_svp_npu_model[task->cfg.model_idx].model_desc, idx, &dims);
    }
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get %u-th input/output dims failed!\n", idx);

    if (is_input == TD_TRUE) {
        data_type = svp_acl_mdl_get_input_data_type(g_svp_npu_model[task->cfg.model_idx].model_desc, idx);
    } else {
        data_type = svp_acl_mdl_get_output_data_type(g_svp_npu_model[task->cfg.model_idx].model_desc, idx);
    }
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get %u-th input/output data type failed!\n", idx);

    data_size = svp_acl_data_type_size(data_type);
    sample_svp_check_exps_return(data_size == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data size failed!\n");

    *line_byte_num = dims.dims[dims.dim_count - 1] *
        ((data_size + SAMPLE_SVP_NPU_BYTE_BIT_NUM - 1) / SAMPLE_SVP_NPU_BYTE_BIT_NUM);

    *total_line_num = 1;
    for (i = 0; i < dims.dim_count - 1; i++) {
        *total_line_num *= dims.dims[i];
    }

    if (is_input == TD_TRUE) {
        ret = svp_acl_mdl_get_first_aipp_info(g_svp_npu_model[task->cfg.model_idx].model_id, idx, &aipp_info);
        sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th input aipp_info failed!\n", idx);

        if (aipp_info.aipp_format == SVP_ACL_YUV420SP_U8 || aipp_info.aipp_format == SVP_ACL_YVU420SP_U8) {
            *total_line_num /= SAMPLE_SVP_NPU_EVEN;
        }
    }

    /* lstm xt line num */
    if ((task->cfg.total_t != 0) && (idx == 0)) {
        sample_svp_check_exps_return(task->cfg.total_t > dims.dims[0], TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "total t(%u) can't be greater than max total t(%ld)!\n", task->cfg.total_t, dims.dims[0]);
        *total_line_num /= dims.dims[0];
        *total_line_num *= task->cfg.total_t;
    }
    return TD_SUCCESS;
}

static td_s32 sample_svp_check_task_cfg(const sample_svp_npu_task_info *task)
{
    sample_svp_check_exps_return(task == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "task is NULL!\n");

    sample_svp_check_exps_return(task->cfg.max_batch_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "max_batch_num(%u) is 0!\n", task->cfg.max_batch_num);

    sample_svp_check_exps_return(task->cfg.dynamic_batch_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "dynamic_batch_num(%u) is 0!\n", task->cfg.dynamic_batch_num);

    sample_svp_check_exps_return(task->cfg.total_t != 0 && task->cfg.dynamic_batch_num != 1, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "dynamic_batch_num(%u) should be 1 when total_t(%u) is not 0!\n",
        task->cfg.dynamic_batch_num, task->cfg.total_t);

    sample_svp_check_exps_return((task->cfg.is_cached != TD_TRUE && task->cfg.is_cached != TD_FALSE), TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "is_cached(%u) should be [%u, %u]!\n", task->cfg.is_cached, TD_FALSE, TD_TRUE);

    sample_svp_check_exps_return(task->cfg.model_idx >= SAMPLE_SVP_NPU_MAX_MODEL_NUM, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "model_idx(%u) should be less than %u!\n",
        task->cfg.model_idx, SAMPLE_SVP_NPU_MAX_MODEL_NUM);

    sample_svp_check_exps_return(g_svp_npu_model[task->cfg.model_idx].model_desc == TD_NULL, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th model_desc is NULL!\n", task->cfg.model_idx);
    return TD_SUCCESS;
}

static td_s32 sample_common_svp_npu_check_input_src_file_num(sample_svp_npu_model_info *model_info,
    td_u32 file_num)
{
    size_t input_num, attached_index;
    svp_acl_mdl_aipp_type type;
    td_u32 aipp_num = 0;
    td_u32 i;
    td_s32 ret;

    input_num = model_info->input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM;
    for (i = 0; i < input_num; i++) {
        ret = svp_acl_mdl_get_input_aipp_type(model_info->model_id, i, &type, &attached_index);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

        if (type == SVP_ACL_DATA_WITH_DYNAMIC_AIPP) {
            aipp_num++;
        }
    }

    input_num -= aipp_num;
    sample_svp_check_exps_return(input_num != file_num, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "input file num(%u) should be equal to %lu!\n", file_num, input_num);

    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_get_input_data(const td_char *src[], td_u32 file_num,
    const sample_svp_npu_task_info *task)
{
    td_s32 ret;
    td_u32 i, line, total_line_num, line_byte_num;
    td_char path[PATH_MAX + 1] = { 0 };
    size_t stride, size;
    FILE *fp = TD_NULL;
    td_void *data = TD_NULL;
    svp_acl_data_buffer *data_buffer = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    sample_svp_check_exps_return(src == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "src is NULL!\n");

    if (sample_common_svp_npu_check_input_src_file_num(&g_svp_npu_model[task->cfg.model_idx], file_num) != TD_SUCCESS) {
        sample_svp_trace_err("check file num failed!\n");
        return TD_FAILURE;
    }

    for (i = 0; i < file_num; i++) {
        sample_svp_check_exps_return(realpath(src[i], path) == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Invalid file!\n");

        fp = fopen(path, "rb");
        sample_svp_check_exps_return(fp == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");

        ret = sample_svp_npu_get_line_num_and_line_byte_num(task, i, TD_TRUE, &total_line_num, &line_byte_num);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get line num failed!\n");

        data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, i);
        sample_svp_check_exps_goto(data_buffer == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data buffer NULL!\n");

        data = svp_acl_get_data_buffer_addr(data_buffer);
        sample_svp_check_exps_goto(data == TD_NULL, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data addr NULL!\n");

        stride = svp_acl_get_data_buffer_stride(data_buffer);
        sample_svp_check_exps_goto(stride == 0, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data stride failed!\n");

        size = svp_acl_get_data_buffer_size(data_buffer);
        sample_svp_check_exps_goto(size < (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride, end,
            SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th data buffer size(%lu) is less than needed(%llu)!\n",
            i, size, (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride);

        for (line = 0; line < task->cfg.dynamic_batch_num * total_line_num; line++) {
            ret = fread(data + line * stride, line_byte_num, 1, fp);
            sample_svp_check_exps_goto(ret != 1, end, SAMPLE_SVP_ERR_LEVEL_ERROR, "Read file failed!\n");
        }
        if (task->cfg.is_cached == TD_TRUE) {
            (td_void)svp_acl_rt_mem_flush(data, task->cfg.dynamic_batch_num * total_line_num * stride);
        }
        (td_void)fclose(fp);
    }
    return TD_SUCCESS;

end:
    if (fp != TD_NULL) {
        (td_void)fclose(fp);
    }
    return TD_FAILURE;
}

static td_s32 sample_svp_npu_read_model(const td_char *model_path, td_u32 model_index, td_bool is_cached)
{
    FILE *fp = TD_NULL;
    td_s32 ret;

    /* Get model file size */
    fp = fopen(model_path, "rb");
    sample_svp_check_exps_return(fp == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "open model file failed, model file is %s!\n", model_path);

    ret = fseek(fp, 0L, SEEK_END);
    sample_svp_check_exps_goto(ret == -1, end_0, SAMPLE_SVP_ERR_LEVEL_ERROR, "fseek failed!\n");

    g_svp_npu_model[model_index].model_mem_size = ftell(fp);
    sample_svp_check_exps_goto(g_svp_npu_model[model_index].model_mem_size <= 0, end_0,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "ftell failed!\n");

    ret = fseek(fp, 0L, SEEK_SET);
    sample_svp_check_exps_goto(ret == -1, end_0, SAMPLE_SVP_ERR_LEVEL_ERROR, "fseek failed!\n");

    /* malloc model file mem */
    if (is_cached == TD_TRUE) {
        ret = svp_acl_rt_malloc_cached(&g_svp_npu_model[model_index].model_mem_ptr,
            g_svp_npu_model[model_index].model_mem_size, SVP_ACL_MEM_MALLOC_NORMAL_ONLY);
    } else {
        ret = svp_acl_rt_malloc(&g_svp_npu_model[model_index].model_mem_ptr,
            g_svp_npu_model[model_index].model_mem_size, SVP_ACL_MEM_MALLOC_NORMAL_ONLY);
    }
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "malloc mem failed, erroe code %d!\n", ret);

    ret = fread(g_svp_npu_model[model_index].model_mem_ptr, g_svp_npu_model[model_index].model_mem_size, 1, fp);
    sample_svp_check_exps_goto(ret != 1, end_1, SAMPLE_SVP_ERR_LEVEL_ERROR, "read model file failed!\n");

    if (is_cached == TD_TRUE) {
        ret = svp_acl_rt_mem_flush(g_svp_npu_model[model_index].model_mem_ptr,
            g_svp_npu_model[model_index].model_mem_size);
        sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_1, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "flush mem failed!, error code is %d\n", ret);
    }
    (td_void)fclose(fp);
    return TD_SUCCESS;
end_1:
    (td_void)svp_acl_rt_free(g_svp_npu_model[model_index].model_mem_ptr);
end_0:
    (td_void)fclose(fp);
    return TD_FAILURE;
}

static td_s32 sample_svp_npu_create_desc(td_u32 model_index)
{
    svp_acl_error ret;

    g_svp_npu_model[model_index].model_desc = svp_acl_mdl_create_desc();
    sample_svp_check_exps_return(g_svp_npu_model[model_index].model_desc == TD_NULL, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "create model description failed!\n");

    ret = svp_acl_mdl_get_desc(g_svp_npu_model[model_index].model_desc, g_svp_npu_model[model_index].model_id);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "get model description failed, error code is %d!\n", ret);

    sample_svp_trace_info("create model description success!\n");

    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_load_model(const td_char *model_path, td_u32 model_index, td_bool is_cached)
{
    td_s32 ret;

    sample_svp_check_exps_return(model_path == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "path is null!\n");
    sample_svp_check_exps_return(model_index >= SAMPLE_SVP_NPU_MAX_MODEL_NUM, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "model_index(%u) should in [0,%d) !\n", model_index, SAMPLE_SVP_NPU_MAX_MODEL_NUM);

    sample_svp_check_exps_return(g_svp_npu_model[model_index].is_load_flag == TD_TRUE, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th node has already loaded a model!\n", model_index);

    ret = sample_svp_npu_read_model(model_path, model_index, is_cached);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "query model failed, model file is %s!\n", model_path);

    ret = svp_acl_mdl_load_from_mem(g_svp_npu_model[model_index].model_mem_ptr,
        g_svp_npu_model[model_index].model_mem_size, &g_svp_npu_model[model_index].model_id);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_0,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "load model from mem failed, error code is %d!\n", ret);

    ret = sample_svp_npu_create_desc(model_index);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_0,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "create desc failed, model file is %s!\n", model_path);

    ret = sample_svp_npu_get_model_base_info(model_index);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_1,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "get model base info failed, model file is %s!\n", model_path);

    sample_svp_trace_info("load mem_size:%lu, id:%u!\n", g_svp_npu_model[model_index].model_mem_size,
        g_svp_npu_model[model_index].model_id);

    g_svp_npu_model[model_index].is_load_flag = TD_TRUE;
    sample_svp_trace_info("load model %s success!\n", model_path);

    return TD_SUCCESS;
end_1:
    (td_void)svp_acl_mdl_destroy_desc(g_svp_npu_model[model_index].model_desc);
    g_svp_npu_model[model_index].model_desc = TD_NULL;
end_0:
    (td_void)svp_acl_rt_free(g_svp_npu_model[model_index].model_mem_ptr);
    g_svp_npu_model[model_index].model_mem_ptr = TD_NULL;
    g_svp_npu_model[model_index].model_mem_size = 0;
    return TD_FAILURE;
}

td_void sample_common_svp_npu_unload_model(td_u32 model_index)
{
    svp_acl_error ret;

    sample_svp_check_exps_return(g_svp_npu_model[model_index].is_load_flag != TD_TRUE, (td_void)0,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th node has not loaded a model!\n", model_index);

    ret = svp_acl_mdl_unload(g_svp_npu_model[model_index].model_id);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("unload model failed, model_id is %u, error code is %d!\n",
            g_svp_npu_model[model_index].model_id, ret);
    }

    if (g_svp_npu_model[model_index].model_desc != TD_NULL) {
        (td_void)svp_acl_mdl_destroy_desc(g_svp_npu_model[model_index].model_desc);
        g_svp_npu_model[model_index].model_desc = TD_NULL;
    }

    if (g_svp_npu_model[model_index].model_mem_ptr != TD_NULL) {
        (td_void)svp_acl_rt_free(g_svp_npu_model[model_index].model_mem_ptr);
        g_svp_npu_model[model_index].model_mem_ptr = TD_NULL;
        g_svp_npu_model[model_index].model_mem_size = 0;
    }

    if (g_svp_npu_model[model_index].handle != TD_NULL) {
        (td_void)svp_acl_mdl_destroy_config_handle(g_svp_npu_model[model_index].handle);
        g_svp_npu_model[model_index].handle = TD_NULL;
    }

    g_svp_npu_model[model_index].is_load_flag = TD_FALSE;
    sample_svp_trace_info("unload model SUCCESS, model id is %u!\n", g_svp_npu_model[model_index].model_id);
}

static td_s32 sample_common_svp_npu_set_model_config(td_u32 model_index)
{
    size_t value = SVP_ACL_MDL_LOAD_FROM_MEM;
    td_s32 ret;

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_LOAD_TYPE_SIZET,
        &(value), sizeof(size_t));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "set_load_type failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_MEM_ADDR_PTR,
        (td_void *)(&(g_svp_npu_model[model_index].model_mem_ptr)), sizeof(td_void *));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "set_mem_ptr failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_MEM_SIZET,
        &(g_svp_npu_model[model_index].model_mem_size), sizeof(size_t));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "set_mem_size failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_PRIORITY_INT32,
        &(g_svp_npu_model[model_index].prior_cfg.priority), sizeof(td_s32));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "set_mem_size failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_PRIORITY_PREEMP_EN_BOOL,
        &g_svp_npu_model[model_index].prior_cfg.prior_preemp_en, sizeof(td_bool));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "prior_preemp_en failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_PRIORITY_UP_STEP_TIMEOUT_UINT32,
        &g_svp_npu_model[model_index].prior_cfg.prior_up_step_timeout, sizeof(td_u32));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR,
        "prior_up_step_timeout failed!\n");

    ret = svp_acl_mdl_set_config_opt(g_svp_npu_model[model_index].handle, SVP_ACL_MDL_PRIORITY_UP_TOP_TIMEOUT_UINT32,
        &g_svp_npu_model[model_index].prior_cfg.prior_up_top_timeout, sizeof(td_u32));
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR,
        "prior_up_top_timeout failed!\n");

    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_load_model_with_config(const td_char *model_path, td_u32 model_index,
    td_bool is_cached)
{
    td_s32 ret;

    sample_svp_check_exps_return(model_path == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "path is null!\n");
    sample_svp_check_exps_return(model_index >= SAMPLE_SVP_NPU_MAX_MODEL_NUM, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "model_index(%u) should in [0,%d) !\n", model_index, SAMPLE_SVP_NPU_MAX_MODEL_NUM);

    sample_svp_check_exps_return(g_svp_npu_model[model_index].is_load_flag == TD_TRUE, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th node has already loaded a model!\n", model_index);

    ret = sample_svp_npu_read_model(model_path, model_index, is_cached);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "query model failed, model file is %s!\n", model_path);

    g_svp_npu_model[model_index].handle = svp_acl_mdl_create_config_handle();
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_0, SAMPLE_SVP_ERR_LEVEL_ERROR, "create handle failed!\n");

    ret = sample_common_svp_npu_set_model_config(model_index);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_1, SAMPLE_SVP_ERR_LEVEL_ERROR, "set handle failed!\n");

    ret = svp_acl_mdl_load_with_config(g_svp_npu_model[model_index].handle, &g_svp_npu_model[model_index].model_id);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_1,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "load model from mem failed, error code is %d!\n", ret);

    ret = sample_svp_npu_create_desc(model_index);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_1,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "create desc failed, model file is %s!\n", model_path);

    ret = sample_svp_npu_get_model_base_info(model_index);
    sample_svp_check_exps_goto(ret != SVP_ACL_SUCCESS, end_2,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "get model base info failed, model file is %s!\n", model_path);

    sample_svp_trace_info("load mem_size:%lu, id:%u!\n", g_svp_npu_model[model_index].model_mem_size,
        g_svp_npu_model[model_index].model_id);

    g_svp_npu_model[model_index].is_load_flag = TD_TRUE;
    sample_svp_trace_info("load model %s success!\n", model_path);

    return TD_SUCCESS;

end_2:
    (td_void)svp_acl_mdl_destroy_desc(g_svp_npu_model[model_index].model_desc);
    g_svp_npu_model[model_index].model_desc = TD_NULL;

end_1:
    (td_void)svp_acl_mdl_destroy_config_handle(g_svp_npu_model[model_index].handle);

end_0:
    (td_void)svp_acl_rt_free(g_svp_npu_model[model_index].model_mem_ptr);
    g_svp_npu_model[model_index].model_mem_ptr = TD_NULL;
    g_svp_npu_model[model_index].model_mem_size = 0;
    return TD_FAILURE;
}

td_void sample_svp_npu_set_model_prior_info(td_u32 model_index, sample_svp_npu_model_prior_cfg *prior_cfg)
{
    if (model_index >= SAMPLE_SVP_NPU_MAX_MODEL_NUM) {
        sample_svp_trace_err("model index[%u] is out of range [0 %u)!\n", model_index, SAMPLE_SVP_NPU_MAX_MODEL_NUM);
        return;
    }

    g_svp_npu_model[model_index].prior_cfg.priority = prior_cfg->priority;
    g_svp_npu_model[model_index].prior_cfg.prior_preemp_en = prior_cfg->prior_preemp_en;
    g_svp_npu_model[model_index].prior_cfg.prior_up_step_timeout = prior_cfg->prior_up_step_timeout;
    g_svp_npu_model[model_index].prior_cfg.prior_up_top_timeout = prior_cfg->prior_up_top_timeout;
}

static td_s32 sample_svp_npu_malloc_mem(td_void **buffer, td_u32 buffer_size, td_bool is_cached, td_ulong class_id)
{
    svp_acl_error ret;

    if (class_id != 0) {
        ret = svp_acl_rt_malloc_sec(buffer, buffer_size, class_id);
    } else {
        if (is_cached == TD_TRUE) {
            ret = svp_acl_rt_malloc_cached(buffer, buffer_size, SVP_ACL_MEM_MALLOC_NORMAL_ONLY);
        } else {
            ret = svp_acl_rt_malloc(buffer, buffer_size, SVP_ACL_MEM_MALLOC_NORMAL_ONLY);
        }
    }
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "can't malloc buffer, size is %u, error code is %d!\n", buffer_size, ret);
    if (class_id == 0) {
        (td_void)memset_s(*buffer, buffer_size, 0, buffer_size);
        if (is_cached == TD_TRUE) {
            (td_void)svp_acl_rt_mem_flush(*buffer, buffer_size);
        }
    }
    return ret;
}

static td_void sample_svp_npu_destroy_data_buffer(svp_acl_data_buffer *input_data, td_ulong class_id)
{
    td_void *data = svp_acl_get_data_buffer_addr(input_data);
    if (class_id == 0) {
        (td_void)svp_acl_rt_free(data);
    } else {
        (td_void)svp_acl_rt_free_sec(data);
    }
    (td_void)svp_acl_destroy_data_buffer(input_data);
}

static svp_acl_data_buffer *sample_common_svp_npu_create_input_data_buffer(sample_svp_npu_task_info *task, td_u32 idx,
    td_ulong *class_id)
{
    td_s32 ret;
    size_t buffer_size, stride;
    td_void *input_buffer = TD_NULL;
    svp_acl_data_buffer *input_data = TD_NULL;

    stride = svp_acl_mdl_get_input_default_stride(g_svp_npu_model[task->cfg.model_idx].model_desc, idx);
    sample_svp_check_exps_return(stride == 0, input_data, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get %u-th input stride failed!\n", idx);

    buffer_size = svp_acl_mdl_get_input_size_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, idx);
    if (idx < g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM) {
        buffer_size *= task->cfg.max_batch_num;
    }
    sample_svp_check_exps_return((buffer_size == 0 || buffer_size > SAMPLE_SVP_NPU_MAX_MEM_SIZE), input_data,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "buffer_size(%lu) can't be 0 and should be less than %u!\n",
        buffer_size, SAMPLE_SVP_NPU_MAX_MEM_SIZE);

    ret = svp_acl_mdl_get_input_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, idx, class_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, input_data, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, %u-th svp_acl_mdl_get_input_class_id_by_index failed!\n", idx);

    if (sample_svp_npu_malloc_mem(&input_buffer, (td_u32)buffer_size, task->cfg.is_cached, *class_id) != TD_SUCCESS) {
        sample_svp_trace_err("%u-th input malloc mem failed!\n", idx);
        return input_data;
    }

    input_data = svp_acl_create_data_buffer(input_buffer, buffer_size, stride);
    if (input_data == TD_NULL) {
        sample_svp_trace_err("can't create %u-th input data buffer!\n", idx);
        if (*class_id == 0) {
            (td_void)svp_acl_rt_free(input_buffer);
        } else {
            (td_void)svp_acl_rt_free_sec(input_buffer);
        }
        return input_data;
    }
    if (idx == g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM) {
        task->task_buf_ptr = input_buffer;
        task->task_buf_size = buffer_size;
        task->task_buf_stride = stride;
    } else if (idx == g_svp_npu_model[task->cfg.model_idx].input_num - 1) {
        task->work_buf_ptr = input_buffer;
        task->work_buf_size = buffer_size;
        task->work_buf_stride = stride;
    }
    return input_data;
}

static svp_acl_data_buffer *sample_common_svp_npu_create_output_data_buffer(const sample_svp_npu_task_info *task,
    td_u32 idx, td_ulong *class_id)
{
    td_s32 ret;
    size_t buffer_size, stride;
    td_void *output_buffer = TD_NULL;
    svp_acl_data_buffer *output_data = TD_NULL;

    stride = svp_acl_mdl_get_output_default_stride(g_svp_npu_model[task->cfg.model_idx].model_desc, idx);
    sample_svp_check_exps_return(stride == 0, output_data, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get %u-th output stride failed!\n", idx);

    buffer_size = svp_acl_mdl_get_output_size_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, idx) *
        (td_u64)task->cfg.max_batch_num;
    sample_svp_check_exps_return((buffer_size == 0 || buffer_size > SAMPLE_SVP_NPU_MAX_MEM_SIZE), output_data,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "buffer_size(%lu) can't be 0 and should be less than %u!\n",
        buffer_size, SAMPLE_SVP_NPU_MAX_MEM_SIZE);

    ret = svp_acl_mdl_get_output_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, idx, class_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, output_data, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, %u-th svp_acl_mdl_get_output_class_id_by_index failed!\n", idx);

    if (sample_svp_npu_malloc_mem(&output_buffer, buffer_size, task->cfg.is_cached, *class_id) != TD_SUCCESS) {
        sample_svp_trace_err("%u-th output malloc mem failed!\n", idx);
        return output_data;
    }

    output_data = svp_acl_create_data_buffer(output_buffer, buffer_size, stride);
    if (output_data == TD_NULL) {
        sample_svp_trace_err("can't create %u-th output data buffer!\n", idx);
        if (*class_id == 0) {
            (td_void)svp_acl_rt_free(output_buffer);
        } else {
            (td_void)svp_acl_rt_free_sec(output_buffer);
        }
        return output_data;
    }
    return output_data;
}

td_s32 sample_common_svp_npu_create_input(sample_svp_npu_task_info *task)
{
    svp_acl_error ret;
    td_u32 i;
    td_ulong class_id;
    svp_acl_data_buffer *input_data = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    task->input_dataset = svp_acl_mdl_create_dataset();
    sample_svp_check_exps_return(task->input_dataset == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "create input dataset failed!\n");

    for (i = 0; i < g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM; i++) {
        input_data = sample_common_svp_npu_create_input_data_buffer(task, i, &class_id);
        if (input_data == TD_NULL) {
            sample_svp_trace_err("create %u-th input data buffer failed!\n", i);
            (td_void)sample_common_svp_npu_destroy_input(task);
            return TD_FAILURE;
        }

        ret = svp_acl_mdl_add_dataset_buffer(task->input_dataset, input_data);
        if (ret != SVP_ACL_SUCCESS) {
            sample_svp_trace_err("add %u-th input data buffer failed!\n", i);
            (td_void)sample_svp_npu_destroy_data_buffer(input_data, class_id);
            (td_void)sample_common_svp_npu_destroy_input(task);
            return TD_FAILURE;
        }
    }
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_create_output(sample_svp_npu_task_info *task)
{
    svp_acl_error ret;
    td_u32 i;
    td_ulong class_id;
    svp_acl_data_buffer *output_data = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    task->output_dataset = svp_acl_mdl_create_dataset();
    sample_svp_check_exps_return(task->input_dataset == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "create output dataset failed!\n");

    for (i = 0; i < g_svp_npu_model[task->cfg.model_idx].output_num; i++) {
        output_data = sample_common_svp_npu_create_output_data_buffer(task, i, &class_id);
        if (output_data == TD_NULL) {
            sample_svp_trace_err("create %u-th output data buffer failed!\n", i);
            (td_void)sample_common_svp_npu_destroy_input(task);
            return TD_FAILURE;
        }
        ret = svp_acl_mdl_add_dataset_buffer(task->output_dataset, output_data);
        if (ret != SVP_ACL_SUCCESS) {
            sample_svp_trace_err("add %u-th output data buffer failed!\n", i);
            (td_void)sample_svp_npu_destroy_data_buffer(output_data, class_id);
            (td_void)sample_common_svp_npu_destroy_output(task);
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

td_void sample_common_svp_npu_destroy_input(sample_svp_npu_task_info *task)
{
    td_u32 i;
    size_t input_num;
    td_s32 ret;
    td_ulong class_id;
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    if (task->input_dataset == TD_NULL) {
        return;
    }

    input_num = svp_acl_mdl_get_dataset_num_buffers(task->input_dataset);
    for (i = 0; i < input_num; i++) {
        data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, i);
        if (i < g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM) {
            ret = svp_acl_mdl_get_input_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, i,
                &class_id);
            sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
                "Error, %u-th svp_acl_mdl_get_input_class_id_by_index failed!\n", i);
            data = svp_acl_get_data_buffer_addr(data_buffer);
            if (class_id == 0) {
                (td_void)svp_acl_rt_free(data);
            } else {
                (td_void)svp_acl_rt_free_sec(data);
            }
        }
        (td_void)svp_acl_destroy_data_buffer(data_buffer);
    }
    (td_void)svp_acl_mdl_destroy_dataset(task->input_dataset);
    task->input_dataset = TD_NULL;
}

td_void sample_common_svp_npu_destroy_output(sample_svp_npu_task_info *task)
{
    td_u32 i;
    size_t output_num;
    td_s32 ret;
    td_ulong class_id;
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    if (task->output_dataset == TD_NULL) {
        return;
    }

    output_num = svp_acl_mdl_get_dataset_num_buffers(task->output_dataset);

    for (i = 0; i < output_num; i++) {
        data_buffer = svp_acl_mdl_get_dataset_buffer(task->output_dataset, i);
        ret = svp_acl_mdl_get_output_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, i, &class_id);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error, %u-th svp_acl_mdl_get_output_class_id_by_index failed!\n", i);
        data = svp_acl_get_data_buffer_addr(data_buffer);
        if (class_id == 0) {
            (td_void)svp_acl_rt_free(data);
        } else {
            (td_void)svp_acl_rt_free_sec(data);
        }
        (td_void)svp_acl_destroy_data_buffer(data_buffer);
    }

    (td_void)svp_acl_mdl_destroy_dataset(task->output_dataset);
    task->output_dataset = TD_NULL;
}

td_void sample_common_svp_npu_destroy_task_buf(sample_svp_npu_task_info *task)
{
    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    if (task->task_buf_ptr == TD_NULL) {
        return;
    }
    (td_void)svp_acl_rt_free(task->task_buf_ptr);
    task->task_buf_ptr = TD_NULL;
    task->task_buf_stride = 0;
    task->task_buf_size = 0;
}

td_void sample_common_svp_npu_destroy_work_buf(sample_svp_npu_task_info *task)
{
    td_s32 ret;
    td_ulong class_id;
    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    if (task->work_buf_ptr == TD_NULL) {
        return;
    }
    ret = svp_acl_mdl_get_input_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc,
        g_svp_npu_model[task->cfg.model_idx].input_num - 1, &class_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, svp_acl_mdl_get_input_class_id_by_index for taskbuf failed!\n");
    if (class_id == 0) {
        (td_void)svp_acl_rt_free(task->work_buf_ptr);
    } else {
        (td_void)svp_acl_rt_free_sec(task->work_buf_ptr);
    }
    task->work_buf_ptr = TD_NULL;
    task->work_buf_stride = 0;
    task->work_buf_size = 0;
}

td_s32 sample_common_svp_npu_create_task_buf(sample_svp_npu_task_info *task)
{
    size_t num;
    svp_acl_data_buffer *task_buf = TD_NULL;
    svp_acl_error ret;
    td_ulong class_id;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    sample_svp_check_exps_return(task->input_dataset == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "input_dataset is NULL!\n");

    num = svp_acl_mdl_get_dataset_num_buffers(task->input_dataset);
    sample_svp_check_exps_return(num != g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM,
        TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "num of data buffer(%lu) should be %lu when create task buf!\n",
        num, g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM);

    task_buf = sample_common_svp_npu_create_input_data_buffer(task,
        g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM, &class_id);
    sample_svp_check_exps_return(task_buf == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "create task buf failed!\n");

    ret = svp_acl_mdl_add_dataset_buffer(task->input_dataset, task_buf);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("add task buf failed!\n");
        (td_void)sample_svp_npu_destroy_data_buffer(task_buf, 0);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_create_work_buf(sample_svp_npu_task_info *task)
{
    size_t num;
    svp_acl_data_buffer *work_buf = TD_NULL;
    td_ulong class_id;
    svp_acl_error ret;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    sample_svp_check_exps_return(task->input_dataset == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "input_dataset is NULL!\n");

    num = svp_acl_mdl_get_dataset_num_buffers(task->input_dataset);
    sample_svp_check_exps_return(num != g_svp_npu_model[task->cfg.model_idx].input_num - 1,
        TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "num of data buffer(%lu) should be %lu when create work buf!\n",
        num, g_svp_npu_model[task->cfg.model_idx].input_num - 1);

    work_buf = sample_common_svp_npu_create_input_data_buffer(task, g_svp_npu_model[task->cfg.model_idx].input_num - 1,
        &class_id);
    sample_svp_check_exps_return(work_buf == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "create work buf failed!\n");

    ret = svp_acl_mdl_add_dataset_buffer(task->input_dataset, work_buf);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("add work buf failed!\n");
        (td_void)sample_svp_npu_destroy_data_buffer(work_buf, class_id);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_get_work_buf_info(const sample_svp_npu_task_info *task,
    td_u32 *work_buf_size, td_u32 *work_buf_stride)
{
    if (work_buf_size == TD_NULL || work_buf_stride == TD_NULL) {
        sample_svp_trace_err("nullptr!\n");
        return TD_FAILURE;
    }

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    *work_buf_stride = (td_u32)svp_acl_mdl_get_input_default_stride(g_svp_npu_model[task->cfg.model_idx].model_desc,
        g_svp_npu_model[task->cfg.model_idx].input_num - 1);
    if (*work_buf_stride == 0) {
        sample_svp_trace_err("get work buf stride failed!\n");
        return TD_FAILURE;
    }

    *work_buf_size = (td_u32)svp_acl_mdl_get_input_size_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc,
        g_svp_npu_model[task->cfg.model_idx].input_num - 1);
    if (*work_buf_size == 0) {
        sample_svp_trace_err("get work buf size failed!\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 sample_common_svp_npu_update_input_info_after_aippset(
    sample_svp_npu_model_info *model, const sample_svp_npu_task_info *task, td_u32 index)
{
    svp_acl_data_buffer *data_buffer = TD_NULL;
    void *data = TD_NULL;
    td_s32 ret = SVP_ACL_ERROR_INVALID_PARAM;
    size_t stride, size;

    data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, index);
    sample_svp_check_exps_return(data_buffer == TD_NULL, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    data = svp_acl_get_data_buffer_addr(data_buffer);
    sample_svp_check_exps_return(data == TD_NULL, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    size = svp_acl_mdl_get_input_size_by_index(model->model_desc, index);
    stride = svp_acl_mdl_get_input_default_stride(model->model_desc, index);

    return svp_acl_update_data_buffer(data_buffer, data, size, stride);
}


static td_s32 sample_common_svp_npu_set_input_aipp_dynamic_param(svp_acl_mdl_aipp *aipp_set,
    const sample_aipp_dynamic_param *dynamic_param)
{
    td_s32 ret;

    ret = svp_acl_mdl_set_aipp_input_format(aipp_set, dynamic_param->input_format);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    ret = svp_acl_mdl_set_aipp_src_image_size(aipp_set, dynamic_param->image_width, dynamic_param->image_height);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    if (dynamic_param->csc_switch != 0) {
        ret = svp_acl_mdl_set_aipp_csc_params(aipp_set, 1,
            dynamic_param->csc_matrix_r0c0, dynamic_param->csc_matrix_r0c1, dynamic_param->csc_matrix_r0c2,
            dynamic_param->csc_matrix_r1c0, dynamic_param->csc_matrix_r1c1, dynamic_param->csc_matrix_r1c2,
            dynamic_param->csc_matrix_r2c0, dynamic_param->csc_matrix_r2c1, dynamic_param->csc_matrix_r2c2,
            dynamic_param->csc_output_bias_r0, dynamic_param->csc_output_bias_r1, dynamic_param->csc_output_bias_r2,
            dynamic_param->csc_input_bias_r0, dynamic_param->csc_input_bias_r1, dynamic_param->csc_input_bias_r2);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    if (dynamic_param->rbuv_swap_switch != 0) {
        ret = svp_acl_mdl_set_aipp_rbuv_swap_switch(aipp_set, 1);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    if (dynamic_param->ax_swap_switch != 0) {
        ret = svp_acl_mdl_set_aipp_ax_swap_switch(aipp_set, 1);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    return TD_SUCCESS;
}

static td_s32 sample_common_svp_npu_set_input_aipp_param(svp_acl_mdl_aipp *aipp_set,
    const sample_svp_npu_model_aipp *model_aipp)
{
    td_s32 ret;

    ret = sample_common_svp_npu_set_input_aipp_dynamic_param(aipp_set, &model_aipp->dynamic_param);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    if (model_aipp->dynamic_batch_param.crop_switch != 0) {
        ret = svp_acl_mdl_set_aipp_crop_params(aipp_set, 1,
            model_aipp->dynamic_batch_param.crop_pos_x, model_aipp->dynamic_batch_param.crop_pos_y,
            model_aipp->dynamic_batch_param.crop_width, model_aipp->dynamic_batch_param.crop_height, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    if (model_aipp->dynamic_batch_param.padding_switch != 0) {
        ret = svp_acl_mdl_set_aipp_padding_params(aipp_set, model_aipp->dynamic_batch_param.padding_switch,
            model_aipp->dynamic_batch_param.padding_top, model_aipp->dynamic_batch_param.padding_bottom,
            model_aipp->dynamic_batch_param.padding_left, model_aipp->dynamic_batch_param.padding_right, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    if (model_aipp->dynamic_batch_param.scf_switch != 0) {
        ret = svp_acl_mdl_set_aipp_scf_params(aipp_set, model_aipp->dynamic_batch_param.scf_switch,
            model_aipp->dynamic_batch_param.scf_input_width, model_aipp->dynamic_batch_param.scf_input_height,
            model_aipp->dynamic_batch_param.scf_output_width, model_aipp->dynamic_batch_param.scf_output_height, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    if (model_aipp->dynamic_batch_param.preproc_switch != 0) {
        ret = svp_acl_mdl_set_aipp_pixel_var_reci(aipp_set,
            model_aipp->dynamic_batch_param.reci_chn0, model_aipp->dynamic_batch_param.reci_chn1,
            model_aipp->dynamic_batch_param.reci_chn2, model_aipp->dynamic_batch_param.reci_chn3, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

        ret = svp_acl_mdl_set_aipp_dtc_pixel_mean(aipp_set,
            model_aipp->dynamic_batch_param.mean_chn0, model_aipp->dynamic_batch_param.mean_chn1,
            model_aipp->dynamic_batch_param.mean_chn2, model_aipp->dynamic_batch_param.mean_chn3, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

        ret = svp_acl_mdl_set_aipp_dtc_pixel_min(aipp_set,
            model_aipp->dynamic_batch_param.min_chn0, model_aipp->dynamic_batch_param.min_chn1,
            model_aipp->dynamic_batch_param.min_chn2, model_aipp->dynamic_batch_param.min_chn3, 0);
        sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    }

    return TD_SUCCESS;
}


td_s32 sample_common_svp_npu_set_input_aipp_info(td_u32 model_index, td_u32 index,
    const sample_svp_npu_model_aipp *model_aipp, const sample_svp_npu_task_info *task)
{
    svp_acl_mdl_aipp *aipp_set = TD_NULL;
    svp_acl_mdl_aipp_type aipp_type;
    size_t attached_index;
    td_s32 ret;

    if (model_index >= SAMPLE_SVP_NPU_MAX_MODEL_NUM || model_aipp == TD_NULL) {
        sample_svp_trace_err("model_index[%u] is out of range[0, %d) or model_aipp nullptr!\n",
            model_index, SAMPLE_SVP_NPU_MAX_MODEL_NUM);
        return TD_FAILURE;
    }

    sample_svp_check_exps_return(g_svp_npu_model[model_index].is_load_flag != TD_TRUE, TD_FAILURE,
        SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th node has not loaded a model!\n", model_index);

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    ret = svp_acl_mdl_get_input_aipp_type(g_svp_npu_model[model_index].model_id, index, &aipp_type, &attached_index);
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);
    if (aipp_type != SVP_ACL_DATA_WITH_DYNAMIC_AIPP) {
        sample_svp_trace_err("input without dynamic aipp!\n");
        return TD_FAILURE;
    }

    aipp_set = svp_acl_mdl_create_aipp(1);
    sample_svp_check_exps_return(aipp_set == TD_NULL, TD_FAILURE, TEST_PICO_ERR_LEVEL_ERROR, "create aipp failure\n");

    ret = sample_common_svp_npu_set_input_aipp_param(aipp_set, model_aipp);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    ret = svp_acl_mdl_set_input_aipp(g_svp_npu_model[model_index].model_id, task->input_dataset,
        attached_index, aipp_set);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail, TEST_PICO_ERR_LEVEL_ERROR, "ret:%d\n", ret);

    return sample_common_svp_npu_update_input_info_after_aippset(&g_svp_npu_model[model_index], task, index);

fail:
    (td_void)svp_acl_mdl_destroy_aipp(aipp_set);
    return ret;
}

/* print the top n confidence values with indexes */
static td_void sample_svp_npu_sort_output_result(const td_float *src, td_u32 src_len,
    sample_svp_npu_top_n_result *dst, td_u32 dst_len)
{
    td_u32 i, j, index;
    td_bool charge;

    for (i = 0; i < src_len; i++) {
        charge = TD_FALSE;

        for (j = 0; j < dst_len; j++) {
            if (src[i] > dst[j].score) {
                index = j;
                charge = TD_TRUE;
                break;
            }
        }

        if (charge == TD_TRUE) {
            for (j = dst_len - 1; j > index; j--) {
                dst[j].score = dst[j - 1].score;
                dst[j].class_id = dst[j - 1].class_id;
            }
            dst[index].score = src[i];
            dst[index].class_id = i;
        }
    }
}

td_void sample_common_svp_npu_output_classification_result(const sample_svp_npu_task_info *task)
{
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;
    td_u32 i, j, n;
    svp_acl_error ret;
    td_ulong class_id;
    size_t stride;
    svp_acl_mdl_io_dims dims;
    sample_svp_npu_top_n_result top[SAMPLE_SVP_NPU_SHOW_TOP_NUM] = { 0 };

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    sample_svp_check_exps_return(task->output_dataset == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "task->output_dataset is NULL!\n");

    for (i = 0; i < svp_acl_mdl_get_dataset_num_buffers(task->output_dataset); i++) {
        data_buffer = svp_acl_mdl_get_dataset_buffer(task->output_dataset, i);
        sample_svp_check_exps_return(data_buffer == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data buffer is NULL!\n", i);

        data = svp_acl_get_data_buffer_addr(data_buffer);
        sample_svp_check_exps_return(data == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data addr is NULL!\n", i);

        ret = svp_acl_mdl_get_output_class_id_by_index(g_svp_npu_model[task->cfg.model_idx].model_desc, i, &class_id);
        sample_svp_check_exps_return((ret != TD_SUCCESS || class_id != 0), (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error, %u-th svp_acl_mdl_get_output_class_id_by_index failed or output is security!\n", i);
        stride = svp_acl_get_data_buffer_stride(data_buffer);
        sample_svp_check_exps_return(data == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data stride is 0!\n", i);

        ret = svp_acl_mdl_get_output_dims(g_svp_npu_model[task->cfg.model_idx].model_desc, i, &dims);
        sample_svp_check_exps_return(data == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th output dims failed, error code is %d!\n", i, ret);

        for (n = 0; n < task->cfg.dynamic_batch_num; n++) {
            (td_void)sample_svp_npu_sort_output_result(data, (td_u32)dims.dims[dims.dim_count - 1],
                top, SAMPLE_SVP_NPU_SHOW_TOP_NUM);
            sample_svp_trace_info("%u-th batch result:\n", n);
            for (j = 0; j < SAMPLE_SVP_NPU_SHOW_TOP_NUM; j++) {
                sample_svp_trace_info("top %u: value[%lf], class_id[%u]!\n", j, top[j].score, top[j].class_id);
            }
            data += stride;
            (td_void)memset_s(top, sizeof(top), 0, sizeof(top));
        }
    }

    sample_svp_trace_info("output data success!\n");
}

td_s32 sample_common_svp_npu_set_dynamic_batch(const sample_svp_npu_task_info *task)
{
    svp_acl_error ret;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    ret = svp_acl_mdl_set_dynamic_batch_size(g_svp_npu_model[task->cfg.model_idx].model_id,
        task->input_dataset, g_svp_npu_model[task->cfg.model_idx].dynamic_batch_idx, task->cfg.dynamic_batch_num);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "set dynamic batch size failed, model id is %u, error code is %d!\n",
        g_svp_npu_model[task->cfg.model_idx].model_id, ret);

    if (task->cfg.total_t != 0) {
        ret = svp_acl_mdl_set_total_t(g_svp_npu_model[task->cfg.model_idx].model_id,
        task->input_dataset, task->cfg.total_t);
        sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "set total_t failed, model id is %u, error code is %d!\n",
            g_svp_npu_model[task->cfg.model_idx].model_id, ret);
    }

    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_model_execute(const sample_svp_npu_task_info *task)
{
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_void *data = TD_NULL;
    size_t size;
    td_u32 i;
    svp_acl_error ret;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    ret = svp_acl_mdl_execute(g_svp_npu_model[task->cfg.model_idx].model_id, task->input_dataset, task->output_dataset);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "svp_acl_mdl_execute failed, model_id is %u, error code is %d!\n",
        g_svp_npu_model[task->cfg.model_idx].model_id, ret);

    if (task->cfg.is_cached == TD_TRUE) {
        for (i = 0; i < g_svp_npu_model[task->cfg.model_idx].output_num; i++) {
            data_buffer = svp_acl_mdl_get_dataset_buffer(task->output_dataset, i);
            sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
                "get %u-th output data_buffer is NULL!\n", i);

            data = svp_acl_get_data_buffer_addr(data_buffer);
            sample_svp_check_exps_return(data == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
                "get %u-th output data is NULL!\n", i);

            size = svp_acl_get_data_buffer_size(data_buffer) / task->cfg.max_batch_num * task->cfg.dynamic_batch_num;
            sample_svp_check_exps_return(size == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
                "get %u-th output data size is 0!\n", i);

            ret = svp_acl_rt_mem_flush(data, size);
            sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
                "flush %u-th output data failed, error code is %d!\n", i, ret);
        }
    }
    return ret;
}

td_s32 sample_common_svp_npu_model_execute_async(const sample_svp_npu_task_info *task,
    svp_acl_rt_stream stream)
{
    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }

    return svp_acl_mdl_execute_async(g_svp_npu_model[task->cfg.model_idx].model_id, task->input_dataset,
        task->output_dataset, stream);
}

static td_void sample_svp_npu_dump_dataset(const sample_svp_npu_task_info *task, const td_char *file_name,
    td_bool is_input)
{
    td_u32 i, line, total_line_num, line_byte_num, data_num;
    svp_acl_error ret;
    size_t size, stride;
    svp_acl_data_buffer *data_buffer = TD_NULL;
    svp_acl_mdl_dataset *dataset = TD_NULL;
    td_void *data = TD_NULL;
    FILE *fp = TD_NULL;
    td_char name[SAMPLE_SVP_NPU_MAX_NAME_LEN] = {'\0'};

    data_num = is_input == TD_TRUE ? g_svp_npu_model[task->cfg.model_idx].input_num - SAMPLE_SVP_NPU_EXTRA_INPUT_NUM :
        g_svp_npu_model[task->cfg.model_idx].output_num;
    dataset = is_input == TD_TRUE ? task->input_dataset : task->output_dataset;

    for (i = 0; i < data_num; i++) {
        ret = sample_svp_npu_get_line_num_and_line_byte_num(task, i, is_input, &total_line_num, &line_byte_num);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get line num and line byte num failed!\n");

        data_buffer = svp_acl_mdl_get_dataset_buffer(dataset, i);
        sample_svp_check_exps_return(data_buffer == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data_buffer failed!\n", i);

        data = svp_acl_get_data_buffer_addr(data_buffer);
        sample_svp_check_exps_return(data == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data addr failed!\n", i);

        stride = svp_acl_get_data_buffer_stride(data_buffer);
        sample_svp_check_exps_return(stride == 0, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data stride failed!\n", i);

        size = svp_acl_get_data_buffer_size(data_buffer);
        sample_svp_check_exps_return(size == 0, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get %u-th data size failed!\n", i);

        sample_svp_check_exps_return(size < (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride, (td_void)0,
            SAMPLE_SVP_ERR_LEVEL_ERROR, "%u-th data buffer size(%lu) is less than needed(%llu)!\n",
            i, size, (td_u64)task->cfg.dynamic_batch_num * total_line_num * stride);

        ret = snprintf_s(name, SAMPLE_SVP_NPU_MAX_NAME_LEN - 1, SAMPLE_SVP_NPU_MAX_NAME_LEN - 1, file_name, i);
        sample_svp_check_exps_return(ret < 0, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "create file name failed!\n");

        fp = fopen(name, "wb");
        sample_svp_check_exps_return(fp == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "open file failed!\n");

        for (line = 0; line < task->cfg.dynamic_batch_num * total_line_num; line++) {
            ret = fwrite(data, line_byte_num, 1, fp);
            if (ret != 1) {
                fclose(fp);
                sample_svp_check_exps_return(ret != 1, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "write file failed!\n");
            }
            data += stride;
        }
        (td_void)fclose(fp);
    }
}

td_void sample_common_svp_npu_dump_task_data(const sample_svp_npu_task_info *task)
{
    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return;
    }

    sample_svp_check_exps_return(task->input_dataset == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "task->input_dataset is NULL!\n");
    sample_svp_check_exps_return(task->output_dataset == TD_NULL, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "task->input_dataset is NULL!\n");

    (td_void)sample_svp_npu_dump_dataset(task, "input_%u.bin", TD_TRUE);
    (td_void)sample_svp_npu_dump_dataset(task, "output_%u.bin", TD_FALSE);
}

td_s32 sample_common_svp_npu_update_input_data_buffer_info(td_u8 *virt_addr, td_u32 size, td_u32 stride, td_u32 idx,
    const sample_svp_npu_task_info *task)
{
    svp_acl_data_buffer *data_buffer = TD_NULL;
    svp_acl_error ret;

    data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, idx);
    sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get %u-th data_buffer failed!\n", idx);
    ret = svp_acl_update_data_buffer(data_buffer, (td_void *)virt_addr, size, stride);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "update data buffer failed!\n");
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_get_input_data_buffer_info(const sample_svp_npu_task_info *task, td_u32 idx,
    td_u8 **virt_addr, td_u32 *size, td_u32 *stride)
{
    svp_acl_data_buffer *data_buffer = TD_NULL;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    sample_svp_check_exps_return(virt_addr == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "virt_addr is NULL!\n");
    sample_svp_check_exps_return(size == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "size is NULL!\n");
    sample_svp_check_exps_return(stride == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "size is NULL!\n");

    data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, idx);
    sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get 0-th data_buffer failed!\n");
    *size = (td_u32)svp_acl_get_data_buffer_size(data_buffer);
    *stride = (td_u32)svp_acl_get_data_buffer_stride(data_buffer);
    *virt_addr = (td_u8 *)svp_acl_get_data_buffer_addr(data_buffer);
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_set_threshold(sample_svp_npu_threshold threshold[], td_u32 threshold_num,
    const sample_svp_npu_task_info *task)
{
    td_u32 i, n;
    svp_acl_error ret;
    svp_acl_data_buffer *data_buffer = TD_NULL;
    td_float *data = TD_NULL;
    size_t idx, size;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    sample_svp_check_exps_return(threshold == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "threshold is NULL!\n");
    sample_svp_check_exps_return(threshold_num == 0, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "threshold_num is 0!\n");

    for (i = 0; i < threshold_num; i++) {
        ret = svp_acl_mdl_get_input_index_by_name(g_svp_npu_model[task->cfg.model_idx].model_desc,
            threshold[i].name, &idx);
        sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get roi idx failed!\n");

        data_buffer = svp_acl_mdl_get_dataset_buffer(task->input_dataset, idx);
        sample_svp_check_exps_return(data_buffer == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get roi data_buffer is NULL!\n");

        size = svp_acl_get_data_buffer_size(data_buffer);
        sample_svp_check_exps_return(size < SAMPLE_SVP_NPU_THRESHOLD_NUM * sizeof(td_float), TD_FAILURE,
            SAMPLE_SVP_ERR_LEVEL_ERROR, "get size(%lu) is less than %lu!\n",
            size, SAMPLE_SVP_NPU_THRESHOLD_NUM * sizeof(td_float));

        data = (td_float *)svp_acl_get_data_buffer_addr(data_buffer);
        sample_svp_check_exps_return(data == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get roi data is NULL!\n");
        n = 0;
        data[n++] = threshold[i].nms_threshold;
        data[n++] = threshold[i].score_threshold;
        data[n++] = threshold[i].min_height;
        data[n++] = threshold[i].min_width;
    }
    return TD_SUCCESS;
}

td_s32 sample_common_svp_npu_check_has_aicpu_task(const sample_svp_npu_task_info *task, td_bool *has_aicpu_task)
{
    td_u32 aicpu_task_num;
    svp_acl_error ret;

    if (sample_svp_check_task_cfg(task) != TD_SUCCESS) {
        sample_svp_trace_err("check task cfg failed!\n");
        return TD_FAILURE;
    }
    sample_svp_check_exps_return(has_aicpu_task == TD_NULL, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "has_aicpu_task is NULL!\n");

    ret = svp_acl_ext_get_mdl_aicpu_task_num(g_svp_npu_model[task->cfg.model_idx].model_id, &aicpu_task_num);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "get aicpu task num failed, error code is %d!\n", ret);

    *has_aicpu_task = (aicpu_task_num == 0) ? TD_FALSE : TD_TRUE;
    return TD_SUCCESS;
}

sample_svp_npu_model_info* sample_common_svp_npu_get_model_info(td_u32 model_idx)
{
    sample_svp_check_exps_return(model_idx >= SAMPLE_SVP_NPU_MAX_MODEL_NUM, NULL, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "model_idx(%u) must be [0, %u)!\n", model_idx, SAMPLE_SVP_NPU_MAX_MODEL_NUM);
    return &g_svp_npu_model[model_idx];
}
