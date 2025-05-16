/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "sample_svp_npu_process.h"
#include <pthread.h>
#include <sys/prctl.h>
#include "svp_acl_rt.h"
#include "svp_acl.h"
#include "svp_acl_ext.h"
#include "sample_common_svp_npu.h"
#include "sample_common_svp_npu_model.h"

#define SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM 1
#define SAMPLE_SVP_NPU_LSTM_INPUT_FILE_NUM     4

#define SAMPLE_SVP_NPU_PREEMPTION_SLEEP_SECOND    30

static td_bool g_svp_npu_terminate_signal = TD_FALSE;
static td_s32 g_svp_npu_dev_id = 0;
static sample_svp_npu_task_info g_svp_npu_task[SAMPLE_SVP_NPU_MAX_TASK_NUM] = {0};
static svp_acl_rt_stream g_svp_npu_stream[SAMPLE_SVP_NPU_MAX_MODEL_NUM] = {0};
static td_bool g_svp_npu_is_thread_start = TD_FALSE;
static td_bool g_sample_svp_init_flag = TD_FALSE;

static td_void sample_svp_npu_acl_terminate(td_void)
{
    if (g_svp_npu_terminate_signal == TD_TRUE) {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
}

/* System init */
static td_s32 sample_comm_svp_sys_init(td_void)
{
    td_s32 ret;

    ret = ss_mpi_sys_exit();
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x):ss_mpi_sys_exit failed!\n", ret);

    ret = ss_mpi_sys_init();
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x):ss_mpi_sys_init failed!\n", ret);

    return ret;
}

/* System exit */
static td_s32 sample_comm_svp_sys_exit(td_void)
{
    td_s32 ret;

    ret = ss_mpi_sys_exit();
    sample_svp_check_exps_return(ret != TD_SUCCESS, ret, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x):ss_mpi_sys_exit failed!\n", ret);

    return TD_SUCCESS;
}

/* System init */
td_s32 sample_common_svp_check_sys_init(td_void)
{
    if (g_sample_svp_init_flag == TD_FALSE) {
        if (sample_comm_svp_sys_init() != TD_SUCCESS) {
            sample_svp_trace_err("Svp mpi init failed!\n");
            return TD_FALSE;
        }
        g_sample_svp_init_flag = TD_TRUE;
    }

    sample_svp_trace_info("Svp mpi init ok!\n");
    return TD_TRUE;
}

/* System exit */
td_void sample_common_svp_check_sys_exit(td_void)
{
    td_s32 ret;

    if (g_sample_svp_init_flag == TD_TRUE) {
        ret = sample_comm_svp_sys_exit();
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("svp mpi exit failed!\n");
        }
    }
    g_sample_svp_init_flag = TD_FALSE;
    sample_svp_trace_info("Svp mpi exit ok!\n");
}

/* function : svp npu signal handle */
td_void sample_svp_npu_acl_handle_sig(td_void)
{
    g_svp_npu_terminate_signal = TD_TRUE;
}

static td_void sample_svp_npu_acl_deinit(td_void)
{
    svp_acl_error ret;

    ret = svp_acl_rt_reset_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("reset device fail\n");
    }
    sample_svp_trace_info("end to reset device is %d\n", g_svp_npu_dev_id);

    ret = svp_acl_finalize();
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("finalize acl fail\n");
    }
    sample_svp_trace_info("end to finalize acl\n");
    (td_void)sample_common_svp_check_sys_exit();
}

static td_s32 sample_svp_npu_acl_init(const td_char *acl_config_path)
{
    /* svp acl init */
    svp_acl_rt_run_mode run_mode;
    svp_acl_error ret;
    td_bool is_mpi_init;

    is_mpi_init = sample_common_svp_check_sys_init();
    sample_svp_check_exps_return(is_mpi_init != TD_TRUE, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "mpi init failed!\n");

    ret = svp_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != SVP_ACL_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "acl init failed!\n");

    sample_svp_trace_info("svp acl init success!\n");

    /* open device */
    ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("svp acl open device %d failed!\n", g_svp_npu_dev_id);
        return TD_FAILURE;
    }
    sample_svp_trace_info("open device %d success!\n", g_svp_npu_dev_id);

    /* get run mode */
    ret = svp_acl_rt_get_run_mode(&run_mode);
    if ((ret != SVP_ACL_SUCCESS) || (run_mode != SVP_ACL_DEVICE)) {
        (td_void)svp_acl_rt_reset_device(g_svp_npu_dev_id);
        (td_void)svp_acl_finalize();
        sample_svp_trace_err("acl get run mode failed!\n");
        return TD_FAILURE;
    }
    sample_svp_trace_info("get run mode success!\n");

    return TD_SUCCESS;
}

static td_s32 sample_svp_npu_acl_create_stream(td_u32 stream_num)
{
    td_u32 i, j;
    td_s32 ret;

    sample_svp_check_exps_return(stream_num > SAMPLE_SVP_NPU_MAX_STREAM_NUM, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "check stream num failed!\n");

    for (i = 0; i < stream_num; i++) {
        ret = svp_acl_rt_create_stream(&g_svp_npu_stream[i]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, create_stream_err, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "create stream failed!\n");
    }

    return SVP_ACL_SUCCESS;

create_stream_err:
    for (j = 0; j < i; j++) {
        (td_void)svp_acl_rt_destroy_stream(g_svp_npu_stream[j]);
    }

    return ret;
}

static td_void sample_svp_npu_acl_destroy_stream(td_u32 stream_num)
{
    td_u32 i;

    for (i = 0; i < stream_num; i++) {
        (td_void)svp_acl_rt_destroy_stream(g_svp_npu_stream[i]);
    }
}

static td_s32 sample_svp_npu_acl_dataset_init(td_u32 task_idx)
{
    td_s32 ret = sample_common_svp_npu_create_input(&g_svp_npu_task[task_idx]);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_FAILURE, SAMPLE_SVP_ERR_LEVEL_ERROR, "create input failed!\n");

    ret = sample_common_svp_npu_create_output(&g_svp_npu_task[task_idx]);
    if (ret != TD_SUCCESS) {
        sample_common_svp_npu_destroy_input(&g_svp_npu_task[task_idx]);
        sample_svp_trace_err("execute create output fail.\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void sample_svp_npu_acl_dataset_deinit(td_u32 task_idx)
{
    (td_void)sample_common_svp_npu_destroy_input(&g_svp_npu_task[task_idx]);
    (td_void)sample_common_svp_npu_destroy_output(&g_svp_npu_task[task_idx]);
}

static td_void *sample_svp_npu_acl_thread_multi_model_process(td_void *args)
{
    td_s32 ret;
    td_u32 task_idx = *(td_u32 *)args;
    td_u32 proc_cnt = 0;

    ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "open device %d failed!\n", g_svp_npu_dev_id);

    while (g_svp_npu_is_thread_start == TD_TRUE && g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[task_idx]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "proc sync failed!\n");
        sample_svp_trace_info("model_index:%u complete cnt:%u\n", g_svp_npu_task[task_idx].cfg.model_idx, ++proc_cnt);
    }

process_end0:
    ret = svp_acl_rt_reset_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("task[%u] reset device failed!\n", task_idx);
    }
    return TD_NULL;
}

static td_void *sample_svp_npu_acl_thread_execute(td_void *args)
{
    td_s32 ret;
    td_u32 task_idx = *(td_u32 *)args;

    ret = svp_acl_rt_set_device(g_svp_npu_dev_id);
    sample_svp_check_exps_return(ret != TD_SUCCESS, TD_NULL, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "open device %d failed!\n", g_svp_npu_dev_id);

    ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[task_idx]);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("execute inference failed of task[%u]!\n", task_idx);
    }

    ret = svp_acl_rt_reset_device(g_svp_npu_dev_id);
    if (ret != SVP_ACL_SUCCESS) {
        sample_svp_trace_err("task[%u] reset device failed!\n", task_idx);
    }
    return TD_NULL;
}

static td_void sample_svp_npu_acl_model_execute_multithread()
{
    pthread_t execute_threads[SAMPLE_SVP_NPU_MAX_THREAD_NUM] = {0};
    td_u32 idx[SAMPLE_SVP_NPU_MAX_THREAD_NUM] = {0};
    td_u32 task_idx;

    for (task_idx = 0; task_idx < SAMPLE_SVP_NPU_MAX_THREAD_NUM; task_idx++) {
        idx[task_idx] = task_idx;
        pthread_create(&execute_threads[task_idx], NULL, sample_svp_npu_acl_thread_execute, &idx[task_idx]);
    }

    td_void *waitret[SAMPLE_SVP_NPU_MAX_THREAD_NUM];
    for (task_idx = 0; task_idx < SAMPLE_SVP_NPU_MAX_THREAD_NUM; task_idx++) {
        pthread_join(execute_threads[task_idx], &waitret[task_idx]);
    }

    for (task_idx = 0; task_idx < SAMPLE_SVP_NPU_MAX_THREAD_NUM; task_idx++) {
        sample_svp_trace_info("output %u-th task data\n", task_idx);
        sample_common_svp_npu_output_classification_result(&g_svp_npu_task[task_idx]);
    }
}

static td_void sample_svp_npu_acl_deinit_task(td_u32 task_num)
{
    td_u32 task_idx;

    for (task_idx = 0; task_idx < task_num; task_idx++) {
        (td_void)sample_common_svp_npu_destroy_work_buf(&g_svp_npu_task[task_idx]);
        (td_void)sample_common_svp_npu_destroy_task_buf(&g_svp_npu_task[task_idx]);
        (td_void)sample_svp_npu_acl_dataset_deinit(task_idx);
        (td_void)memset_s(&g_svp_npu_task[task_idx], sizeof(sample_svp_npu_task_cfg), 0,
            sizeof(sample_svp_npu_task_cfg));
    }
}

static td_s32 sample_svp_npu_acl_init_task(td_u32 task_num)
{
    td_u32 task_idx;
    td_s32 ret;

    for (task_idx = 0; task_idx < task_num; task_idx++) {
        ret = sample_svp_npu_acl_dataset_init(task_idx);
        if (ret != TD_SUCCESS) {
            goto task_init_end_0;
        }
        ret = sample_common_svp_npu_create_task_buf(&g_svp_npu_task[task_idx]);
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("create task buf failed.\n");
            goto task_init_end_0;
        }
        ret = sample_common_svp_npu_create_work_buf(&g_svp_npu_task[task_idx]);
        if (ret != TD_SUCCESS) {
            sample_svp_trace_err("create work buf failed.\n");
            goto task_init_end_0;
        }
    }
    return TD_SUCCESS;

task_init_end_0:
    (td_void)sample_svp_npu_acl_deinit_task(task_num);
    return ret;
}

static td_s32 sample_svp_npu_acl_load_multi_model(const td_char *model_path,
    td_u32 model_num, td_bool is_with_config, const td_char *src_file[], td_u32 file_num)
{
    td_u32 i, j;
    td_s32 ret;

    for (i = 0; i < model_num; i++) {
        if (is_with_config == TD_TRUE) {
            ret = sample_common_svp_npu_load_model_with_config(model_path, i, TD_FALSE);
        } else {
            ret = sample_common_svp_npu_load_model(model_path, i, TD_FALSE);
        }
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "load model failed!\n");

        /* set task cfg */
        g_svp_npu_task[i].cfg.max_batch_num = 1;
        g_svp_npu_task[i].cfg.dynamic_batch_num = 1;
        g_svp_npu_task[i].cfg.total_t = 0;
        g_svp_npu_task[i].cfg.is_cached = TD_FALSE;
        g_svp_npu_task[i].cfg.model_idx = i;
    }

    ret = sample_svp_npu_acl_init_task(model_num);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");

    for (i = 0; i < model_num; i++) {
        ret = sample_common_svp_npu_get_input_data(src_file, file_num, &g_svp_npu_task[i]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");
    }

    return TD_SUCCESS;

process_end1:
    (td_void)sample_svp_npu_acl_deinit_task(model_num);

process_end0:
    for (j = 0; j < i; j++) {
        (td_void)sample_common_svp_npu_unload_model(j);
    }

    return ret;
}

static td_void sample_svp_npu_acl_unload_multi_model(td_u32 model_num)
{
    td_u32 i;

    (td_void)sample_svp_npu_acl_deinit_task(model_num);

    for (i = 0; i < model_num; i++) {
        (td_void)sample_common_svp_npu_unload_model(i);
    }
}

/* function : show the sample of svp npu resnet50 */
td_void sample_svp_npu_acl_resnet50(td_void)
{
    td_s32 ret;
    const td_u32 model_idx = 0;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/3_224_224_batch_1.bgr"};
    const td_char *om_model_path = "./data/model/resnet50.om";

    g_svp_npu_terminate_signal = TD_FALSE;

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        /* init acl */
        ret = sample_svp_npu_acl_init(acl_config_path);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

        /* load model */
        ret = sample_common_svp_npu_load_model(om_model_path, model_idx, TD_FALSE);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "load model failed!\n");

        /* set task cfg */
        g_svp_npu_task[0].cfg.max_batch_num = 1;
        g_svp_npu_task[0].cfg.dynamic_batch_num = 1;
        g_svp_npu_task[0].cfg.total_t = 0;
        g_svp_npu_task[0].cfg.is_cached = TD_FALSE;
        g_svp_npu_task[0].cfg.model_idx = model_idx;

        ret = sample_svp_npu_acl_init_task(1);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "init task failed!\n");
    }

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_get_input_data(src_file, SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM,
            &g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");
    }
    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");
        (td_void)sample_common_svp_npu_output_classification_result(&g_svp_npu_task[0]);
    }

process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(1);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

td_void sample_svp_npu_acl_resnet50_dynamic_batch(td_void)
{
    td_s32 ret;
    const td_u32 model_idx = 0;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/3_224_224_batch_8.bgr"};
    const td_char *om_model_path = "./data/model/resnet50.om";

    g_svp_npu_terminate_signal = TD_FALSE;

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        /* init acl */
        ret = sample_svp_npu_acl_init(acl_config_path);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

        /* load model */
        ret = sample_common_svp_npu_load_model(om_model_path, model_idx, TD_TRUE);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "load model failed!\n");

        /* set task cfg */
        g_svp_npu_task[0].cfg.max_batch_num = 8; /* 8 is max batch num, it can't be less than dynamic_batch_num */
        g_svp_npu_task[0].cfg.dynamic_batch_num = 8; /* 8 is batch num of task to be processed */
        g_svp_npu_task[0].cfg.total_t = 0;
        g_svp_npu_task[0].cfg.is_cached = TD_TRUE;
        g_svp_npu_task[0].cfg.model_idx = model_idx;
        ret = sample_svp_npu_acl_init_task(1);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");
    }

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_get_input_data(src_file, SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM,
            &g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get input data failed!\n");
        ret = sample_common_svp_npu_set_dynamic_batch(&g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "set dynamic batch failed!\n");
    }
    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");
        (td_void)sample_common_svp_npu_output_classification_result(&g_svp_npu_task[0]);
    }

process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(1);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

/* function : show the sample of svp npu resnet50 multi thread */
td_void sample_svp_npu_acl_resnet50_multi_thread(td_void)
{
    td_u32 i;
    td_s32 ret;
    const td_u32 model_idx = 0;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/3_224_224_batch_1.bgr"};
    const td_char *om_model_path = "./data/model/resnet50.om";

    g_svp_npu_terminate_signal = TD_FALSE;

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        /* init acl */
        ret = sample_svp_npu_acl_init(acl_config_path);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

        /* load model */
        ret = sample_common_svp_npu_load_model(om_model_path, model_idx, TD_FALSE);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "load model failed!\n");

        /* set cfg */
        for (i = 0; i < SAMPLE_SVP_NPU_MAX_TASK_NUM; i++) {
            g_svp_npu_task[i].cfg.max_batch_num = 1;
            g_svp_npu_task[i].cfg.dynamic_batch_num = 1;
            g_svp_npu_task[i].cfg.total_t = 0;
            g_svp_npu_task[i].cfg.is_cached = TD_FALSE;
            g_svp_npu_task[i].cfg.model_idx = model_idx;
        }

        ret = sample_svp_npu_acl_init_task(SAMPLE_SVP_NPU_MAX_TASK_NUM);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");
    }

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        for (i = 0; i < SAMPLE_SVP_NPU_MAX_TASK_NUM; i++) {
            ret = sample_common_svp_npu_get_input_data(src_file, SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM,
                &g_svp_npu_task[i]);
            sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2,
                SAMPLE_SVP_ERR_LEVEL_ERROR, "get %u-th input failed!\n", i);
        }
    }

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        (td_void)sample_svp_npu_acl_model_execute_multithread();
    }

process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(SAMPLE_SVP_NPU_MAX_TASK_NUM);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

td_void sample_svp_npu_acl_lstm(td_void)
{
    td_s32 ret;
    const td_u32 model_idx = 0;
    const td_char *src_file[SAMPLE_SVP_NPU_LSTM_INPUT_FILE_NUM] = {"./data/vector/xt.seq", "./data/vector/cont.vec",
        "./data/vector/h0.vec", "./data/vector/c0.vec"};
    const td_char *om_model_path = "./data/model/lstm.om";

    g_svp_npu_terminate_signal = TD_FALSE;

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        /* init acl */
        ret = sample_svp_npu_acl_init(TD_NULL);
        sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

        /* load model */
        ret = sample_common_svp_npu_load_model(om_model_path, model_idx, TD_TRUE);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "load model failed!\n");

        /* set task cfg */
        g_svp_npu_task[0].cfg.max_batch_num = 1;
        g_svp_npu_task[0].cfg.dynamic_batch_num = 1;
        g_svp_npu_task[0].cfg.total_t = 557; /* 557 is total t */
        g_svp_npu_task[0].cfg.is_cached = TD_TRUE;
        g_svp_npu_task[0].cfg.model_idx = model_idx;
        ret = sample_svp_npu_acl_init_task(1);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "init task failed!\n");
    }

    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_get_input_data(src_file, SAMPLE_SVP_NPU_LSTM_INPUT_FILE_NUM, &g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "get input data failed!\n");
        ret = sample_common_svp_npu_set_dynamic_batch(&g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "set dynamic batch failed!\n");
    }
    if (g_svp_npu_terminate_signal == TD_FALSE) {
        ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");
    }

process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(1);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

td_void sample_svp_npu_acl_event(td_void)
{
    td_s32 ret;
    const td_u32 model_num = 2;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/3_224_224_batch_1.bgr"};
    const td_char *om_model_path = "./data/model/resnet50.om";
    const td_u32 proc_cnt = 5;
    td_u32 i;

    svp_acl_rt_event event;

    /* init acl */
    ret = sample_svp_npu_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

    ret = sample_svp_npu_acl_create_stream(model_num);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "create stream failed!\n");

    ret = svp_acl_rt_create_event(&event);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR, "create events failed!\n");

    /* load model */
    ret = sample_svp_npu_acl_load_multi_model(om_model_path, model_num, TD_FALSE, src_file,
        SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "load model failed!\n");

    for (i = 0; i < proc_cnt; i++) {
        ret = sample_common_svp_npu_model_execute_async(&g_svp_npu_task[0], g_svp_npu_stream[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");

        ret = svp_acl_rt_record_event(event, g_svp_npu_stream[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "record event failed!\n");

        ret = svp_acl_rt_stream_wait_event(g_svp_npu_stream[1], event);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "wait event failed!\n");

        ret = sample_common_svp_npu_model_execute_async(&g_svp_npu_task[1], g_svp_npu_stream[1]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");

        ret = svp_acl_rt_synchronize_event(event);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "sync event failed!\n");

        ret = svp_acl_rt_reset_event(event, g_svp_npu_stream[0]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR,
            "reset event failed!\n");
    }

    ret = svp_acl_rt_synchronize_stream(g_svp_npu_stream[1]);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end3, SAMPLE_SVP_ERR_LEVEL_ERROR, "sync stream failed!\n");

process_end3:
    (td_void)sample_svp_npu_acl_unload_multi_model(model_num);

process_end2:
    (td_void)svp_acl_rt_destroy_event(event);

process_end1:
    (td_void)sample_svp_npu_acl_destroy_stream(model_num);

process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

static td_s32 sample_svp_npu_acl_set_input_aipp_info(td_u32 model_index, td_u32 index)
{
    const sample_svp_npu_model_aipp model_aipp = {
        {
            SVP_ACL_YUV420SP_U8,
            1, 1, 0,
            720, 576,       /* src image size */
            1192, 2166, 0,  /* csc coef: yuv420 -> BGR */
            1192, -218, -547,
            1192, 0, 1836,
            0, 0, 0,
            16, 128, 128
        }, {
            1, AIPP_PADDING_SWITCH_REPLICATION, AIPP_RESIZE_SWITCH_BILINEAR, 1,
            0, 0, 716, 572, /* crop param */
            2, 2, 2, 2, /* padding param */
            720, 576, 1920, 1080, /* resize param */
            1, 1, 1, 1, 0, 0, 0, 0, -32768, -32768, -32768, -32768 /* preprocess param */
        }
    };
    return sample_common_svp_npu_set_input_aipp_info(model_index, index, &model_aipp, &g_svp_npu_task[model_index]);
}

static td_s32 sample_svp_npu_acl_set_aipp_info(td_u32 model_index)
{
    td_s32 ret;

    ret = sample_svp_npu_acl_set_input_aipp_info(model_index, 0);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, fail, SAMPLE_SVP_ERR_LEVEL_ERROR, "set input aipp failed!\n");

    return TD_SUCCESS;

fail:
    return ret;
}

td_void sample_svp_npu_acl_aipp(td_void)
{
    td_s32 ret;
    const td_u32 model_idx = 0;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/st_lk_720x576_420sp.yuv"};
    const td_char *om_model_path = "./data/model/aipp.om";

    /* init acl */
    ret = sample_svp_npu_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

    /* load model */
    ret = sample_common_svp_npu_load_model(om_model_path, model_idx, TD_FALSE);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "load model failed!\n");

    /* set task cfg */
    g_svp_npu_task[0].cfg.max_batch_num = 1;
    g_svp_npu_task[0].cfg.dynamic_batch_num = 1;
    g_svp_npu_task[0].cfg.total_t = 0;
    g_svp_npu_task[0].cfg.is_cached = TD_FALSE;
    g_svp_npu_task[0].cfg.model_idx = model_idx;

    ret = sample_svp_npu_acl_init_task(1);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "init task failed!\n");

    ret = sample_svp_npu_acl_set_aipp_info(model_idx);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, SAMPLE_SVP_ERR_LEVEL_ERROR,
        "set aipp info failed!\n");

    ret = sample_common_svp_npu_get_input_data(src_file, SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM,
        &g_svp_npu_task[0]);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "get data failed!\n");

    ret = sample_common_svp_npu_model_execute(&g_svp_npu_task[0]);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end2, SAMPLE_SVP_ERR_LEVEL_ERROR, "execute failed!\n");

process_end2:
    (td_void)sample_svp_npu_acl_deinit_task(1);
process_end1:
    (td_void)sample_common_svp_npu_unload_model(model_idx);
process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}

td_void sample_svp_npu_acl_preemption(td_void)
{
    td_s32 ret;
    const td_u32 model_num = 2;
    const td_char *acl_config_path = "";
    const td_char *src_file[SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM] = {"./data/image/3_224_224_batch_1.bgr"};
    const td_char *om_model_path = "./data/model/resnet50_with_preemption.om";
    td_u32 i, j;
    td_u32 idx[SAMPLE_SVP_NPU_MAX_MODEL_NUM] = {0};
    pthread_t svp_npu_multi_thread[SAMPLE_SVP_NPU_MAX_MODEL_NUM] = {0};

    sample_svp_npu_model_prior_cfg prior_cfg[] = {
        { SAMPLE_SVP_NPU_PRIORITY_2, 1, 0, 0},
        { SAMPLE_SVP_NPU_PRIORITY_4, 1, SAMPLE_SVP_NPU_PRIOR_TIMEOUT, 0 }
    };

    /* init acl */
    ret = sample_svp_npu_acl_init(acl_config_path);
    sample_svp_check_exps_return(ret != TD_SUCCESS, (td_void)0, SAMPLE_SVP_ERR_LEVEL_ERROR, "init failed!\n");

    for (i = 0; i < model_num; i++) {
        sample_svp_npu_set_model_prior_info(i, &prior_cfg[i]);
    }

    /* load model */
    ret = sample_svp_npu_acl_load_multi_model(om_model_path, model_num, TD_TRUE, src_file,
        SAMPLE_SVP_NPU_RESNET50_INPUT_FILE_NUM);
    sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end0, SAMPLE_SVP_ERR_LEVEL_ERROR, "load model failed!\n");

    g_svp_npu_is_thread_start = TD_TRUE;

    for (i = 0; i < model_num; i++) {
        idx[i] = i;
        ret = pthread_create(&svp_npu_multi_thread[i], NULL, sample_svp_npu_acl_thread_multi_model_process, &idx[i]);
        sample_svp_check_exps_goto(ret != TD_SUCCESS, process_end1, TEST_PICO_ERR_LEVEL_ERROR,
            "pthread_create failed!\n");
    }

    sleep(SAMPLE_SVP_NPU_PREEMPTION_SLEEP_SECOND);

process_end1:
    g_svp_npu_is_thread_start = TD_FALSE;
    for (j = 0; j < i; j++) {
        pthread_join(svp_npu_multi_thread[j], NULL);
    }

    (td_void)sample_svp_npu_acl_unload_multi_model(model_num);

process_end0:
    (td_void)sample_svp_npu_acl_deinit();
    (td_void)sample_svp_npu_acl_terminate();
}
