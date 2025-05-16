/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <sys/time.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "sample_comm.h"
#include "ss_mpi_isp.h"
#include "ot_common_isp.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_awb.h"
#include "ss_mpi_sys.h"

#ifdef __LITEOS__
#include <asm/io.h>
#endif

#include <fcntl.h>
#include <sys/mman.h>
#include "ir_auto_prev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
static volatile sig_atomic_t g_sig_flag = 0;

typedef struct {
    td_bool thread_flag;
    pthread_t thread;
}ot_ir_auto_thread;
static ot_ir_auto_thread g_ast_ir_thread[OT_ISP_MAX_PIPE_NUM] = {{0}};

static ot_isp_ir_auto_attr g_ast_ir_attr[OT_ISP_MAX_PIPE_NUM] = {{0}};
static ot_vi_pipe g_vi_pipe = 0;

#define SAMPLE_IR_CALIBRATION_MODE  (0)
#define SAMPLE_IR_AUTO_MODE         (1)
#define GAIN_MAX_COEF               (280)
#define GAIN_MIN_COEF               (190)
#define SHIFT_8BIT                  (8)
#define SAMPLE_IR_AUTO_RETURN            (2)
static td_s32 sample_ir_auto_stop_vi_vo(sample_vi_cfg *vi_config, sample_vo_cfg *vo_config)
{
    sample_comm_vo_stop_vo(vo_config);

    sample_comm_vi_stop_vi(vi_config);

    return TD_SUCCESS;
}

td_s32 sample_ir_auto_start_vi_vo(sample_vi_cfg *vi_config, sample_vo_cfg *vo_config)
{
    td_s32  ret;

    ret = sample_comm_vi_start_vi(vi_config);
    if (TD_SUCCESS != ret) {
        sample_print("start vi failed!\n");
        return ret;
    }

    ret = sample_comm_vo_start_vo(vo_config);
    if (TD_SUCCESS != ret) {
        sample_print("SAMPLE_VIO start VO failed with %#x!\n", ret);
        goto EXIT;
    }

    return ret;

EXIT:
    sample_comm_vi_stop_vi(vi_config);

    return ret;
}

static td_void sample_ir_auto_get_default_vb_config(ot_size *size, ot_vb_cfg *vb_cfg)
{
    ot_vb_calc_cfg calc_cfg;
    ot_pic_buf_attr buf_attr;

    (td_void)memset_s(vb_cfg, sizeof(ot_vb_cfg), 0, sizeof(ot_vb_cfg));
    vb_cfg->max_pool_cnt = 128; /* 128 pool limit */

    buf_attr.width         = size->width;
    buf_attr.height        = size->height;
    buf_attr.align         = OT_DEFAULT_ALIGN;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    buf_attr.video_format = OT_VIDEO_FORMAT_LINEAR;
    ot_common_get_pic_buf_cfg(&buf_attr, &calc_cfg);

    vb_cfg->common_pool[0].blk_size = calc_cfg.vb_size;
    vb_cfg->common_pool[0].blk_cnt  = 30; /* 30 pool cnt */
}

static td_s32 sample_ir_auto_sys_init(ot_vi_vpss_mode_type mode_type, ot_vi_aiisp_mode aiisp_mode)
{
    td_s32 ret;
    ot_size size;
    ot_vb_cfg vb_cfg;
    td_u32 supplement_config;

    sample_comm_vi_get_size_by_sns_type(SENSOR0_TYPE, &size);
    sample_ir_auto_get_default_vb_config(&size, &vb_cfg);

    supplement_config = OT_VB_SUPPLEMENT_BNR_MOT_MASK;
    ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, supplement_config);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    ret = sample_comm_vi_set_vi_vpss_mode(mode_type, aiisp_mode);
    if (ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 sample_ir_auto_start_prev(ir_auto_prev *ir_auto_prev)
{
    td_s32 ret;
    sample_sns_type sns_type;
    ot_size in_size;
    ot_vi_vpss_mode_type mast_pipe_mode = OT_VI_ONLINE_VPSS_OFFLINE;
    ot_vi_aiisp_mode aiisp_mode = OT_VI_AIISP_MODE_DEFAULT;

    ir_auto_prev->vo_dev = SAMPLE_VO_DEV_DHD0;
    ir_auto_prev->vo_chn = 0;
    ir_auto_prev->vi_pipe = 0;
    ir_auto_prev->vi_chn = 0;

    /* ***********************************************
    step1:  get all sensors information
    ************************************************ */

    /* ***********************************************
    step2:  get  input size
    ************************************************ */
    ret = sample_ir_auto_sys_init(mast_pipe_mode, aiisp_mode);
    if (ret != TD_SUCCESS) {
        goto sys_init_failed;
    }

    sns_type = SENSOR0_TYPE;

    sample_comm_vi_get_size_by_sns_type(sns_type, &in_size);
    sample_comm_vi_get_default_vi_cfg(sns_type, &ir_auto_prev->vi_config);

    /* ***********************************************
    step4:  init VI and VO
    ************************************************ */
    sample_comm_vo_get_def_config(&ir_auto_prev->vo_config);
    ret = sample_ir_auto_start_vi_vo(&ir_auto_prev->vi_config, &ir_auto_prev->vo_config);
    if (TD_SUCCESS != ret) {
        sample_print("sample_ir_auto_start_vi_vo failed witfh %d\n", ret);
        goto EXIT;
    }

    /* ***********************************************
    step5:  bind VI and VO
    ************************************************ */
    ret = sample_comm_vi_bind_vo(ir_auto_prev->vi_pipe, ir_auto_prev->vi_chn, ir_auto_prev->vo_dev,
        ir_auto_prev->vo_chn);
    if (ret != TD_SUCCESS) {
        sample_print("sample_comm_vi_bind_vo failed with %#x!\n", ret);
        goto EXIT1;
    }

    return ret;

EXIT1:
    sample_ir_auto_stop_vi_vo(&ir_auto_prev->vi_config, &ir_auto_prev->vo_config);
EXIT:
    sample_comm_sys_exit();
sys_init_failed:
    return ret;
}

td_void sample_ir_auto_stop_prev(ir_auto_prev *ir_auto_prev)
{
    td_s32 ret;

    ret = sample_comm_vi_un_bind_vo(ir_auto_prev->vi_pipe, ir_auto_prev->vi_chn, ir_auto_prev->vo_dev,
        ir_auto_prev->vo_chn);
    if (ret != TD_SUCCESS) {
        return;
    }

    ret = sample_ir_auto_stop_vi_vo(&ir_auto_prev->vi_config, &ir_auto_prev->vo_config);
    if (ret != TD_SUCCESS) {
        return;
    }

    sample_comm_sys_exit();

    return;
}

#define sample_ir_cut_check_fp_return(fp, file_name)                                   \
    do {                                                       \
        if ((fp) == NULL) {    \
            printf("Cannot open %s.\n", file_name); \
            return;                   \
        }                                                      \
    } while (0)

#define LEN 10
#define IR_CUT_FILE_NAME_LEN 50
td_void sample_ir_cut_gpio_export(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[IR_CUT_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, IR_CUT_FILE_NAME_LEN, "/sys/class/gpio/export");
    if (ret < 0) {
        printf("export sprintf_s failed with %d.\n", ret);
        return;
    }
    fp = fopen(file_name, "w");
    sample_ir_cut_check_fp_return(fp, file_name);
    ret = fprintf(fp, "%u", gpio_num);
    if (ret < 0) {
        printf("print gpio_num failed!!!\n");
    }

    ret = fclose(fp);
    if (ret != 0) {
        printf("export close fp err ret:%d\n", ret);
    }
    return;
}

td_void sample_ir_cut_gpio_dir(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[IR_CUT_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, IR_CUT_FILE_NAME_LEN, "/sys/class/gpio/gpio%u/direction", gpio_num);
    if (ret < 0) {
        printf("dir sprintf_s failed with %d.\n", ret);
        return;
    }
    fp = fopen(file_name, "rb+");
    sample_ir_cut_check_fp_return(fp, file_name);
    ret = fprintf(fp, "out");
    if (ret < 0) {
        printf("print out failed!!!\n");
    }
    ret = fclose(fp);
    if (ret != 0) {
        printf("dir close fp err, ret:%d\n", ret);
    }

    return;
}

td_void sample_ir_cut_gpio_write(td_u32 gpio_chip_num, td_u32 gpio_offset, td_u32 gpio_out_val)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char buf[LEN];
    char file_name[IR_CUT_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, IR_CUT_FILE_NAME_LEN, "/sys/class/gpio/gpio%u/value", gpio_num);
    if (ret < 0) {
        printf("write sprintf_s failed with %d.\n", ret);
        return;
    }

    fp = fopen(file_name, "rb+");
    sample_ir_cut_check_fp_return(fp, file_name);

    if (gpio_out_val) {
        ret = strcpy_s(buf, LEN, "1");
        if (ret != EOK) {
            printf("strcpy_s buf 1 failed,pls check it!!!\n");
        }
    } else {
        ret = strcpy_s(buf, LEN, "0");
        if (ret != EOK) {
            printf("strcpy_s buf 0 failed,pls check it!!!\n");
        }
    }
    ret = fwrite(buf, sizeof(char), sizeof(buf) - 1, fp);
    if (ret != (sizeof(buf) - 1)) {
        printf("%s: gpio%u_%u = %s, fwrite err ret:%d\n", __func__, gpio_chip_num, gpio_offset, buf, ret);
    }
    printf("%s: gpio%u_%u = %s\n", __func__, gpio_chip_num, gpio_offset, buf);
    ret = fclose(fp);
    if (ret != 0) {
        printf("write close fp err, ret:%d\n", ret);
    }

    return;
}

td_void sample_ir_cut_gpio_unexport(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[IR_CUT_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, IR_CUT_FILE_NAME_LEN, "/sys/class/gpio/unexport");
    if (ret < 0) {
        printf("unexport sprintf_s failed with %d.\n", ret);
        return;
    }

    fp = fopen(file_name, "w");
    sample_ir_cut_check_fp_return(fp, file_name);
    ret = fprintf(fp, "%u", gpio_num);
    if (ret < 0) {
        printf("print unexport gpio_num failed!!!\n");
    }
    ret = fclose(fp);
    if (ret != 0) {
        printf("unexport close fp err, ret:%d\n", ret);
    }

    return;
}

// normal mode
void sample_ir_cut_enable(void)
{
    td_u32 gpio_chip_num5 = 5; // gpio5
    td_u32 gpio_chip_num2 = 2; // gpio2
    td_u32 gpio_offset1 = 1;  // gpio5_1
    td_u32 gpio_offset0 = 0;  // gpio2_0
    // export
    sample_ir_cut_gpio_export(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_export(gpio_chip_num2, gpio_offset0);
    // dir
    sample_ir_cut_gpio_dir(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_dir(gpio_chip_num2, gpio_offset0);
    // data
    sample_ir_cut_gpio_write(gpio_chip_num5, gpio_offset1, 0); // set GPIO5_1 0
    sample_ir_cut_gpio_write(gpio_chip_num2, gpio_offset0, 1); // set GPIO2_0 1
    /* 1000000 1000ms */
    usleep(1000000);
    // back to original
    sample_ir_cut_gpio_write(gpio_chip_num5, gpio_offset1, 0);
    sample_ir_cut_gpio_write(gpio_chip_num2, gpio_offset0, 0);
    sample_ir_cut_gpio_unexport(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_unexport(gpio_chip_num2, gpio_offset0);
    return;
}

void sample_ir_cut_disable(void)
{
    td_u32 gpio_chip_num5 = 5; // gpio5
    td_u32 gpio_chip_num2 = 2; // gpio2
    td_u32 gpio_offset1 = 1;  // gpio5_1
    td_u32 gpio_offset0 = 0;  // gpio2_0
    // export
    sample_ir_cut_gpio_export(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_export(gpio_chip_num2, gpio_offset0);
    // dir
    sample_ir_cut_gpio_dir(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_dir(gpio_chip_num2, gpio_offset0);
    // data
    sample_ir_cut_gpio_write(gpio_chip_num5, gpio_offset1, 1); // set GPIO5_1 1
    sample_ir_cut_gpio_write(gpio_chip_num2, gpio_offset0, 0); // set GPIO2_0 0
    /* 1000000 1000ms */
    usleep(1000000);
    // back to original
    sample_ir_cut_gpio_write(gpio_chip_num5, gpio_offset1, 0);
    sample_ir_cut_gpio_write(gpio_chip_num2, gpio_offset0, 0);
    sample_ir_cut_gpio_unexport(gpio_chip_num5, gpio_offset1);
    sample_ir_cut_gpio_unexport(gpio_chip_num2, gpio_offset0);
    return;
}

void sample_ir_auto_usage(const char* s_prg_nm)
{
    printf("usage : %s <mode> <normal2_ir_exp_thr> <ir2_normal_exp_thr>"
        " <rg_max> <rg_min> <bg_max> <bg_min> <ir_status>\n", s_prg_nm);

    printf("mode:\n");
    printf("\t 0) SAMPLE_IR_CALIBRATION_MODE.\n");
    printf("\t 1) SAMPLE_IR_AUTO_MODE.\n");

    printf("normal2_ir_exp_thr:\n");
    printf("\t ISO threshold of switching from normal to IR mode.\n");

    printf("ir2_normal_exp_thr:\n");
    printf("\t ISO threshold of switching from IR to normal mode.\n");

    printf("rg_max/rg_min/bg_max/bg_min:\n");
    printf("\t maximum(minimum) value of R/G(B/G) in IR scene.\n");

    printf("ir_status:\n");
    printf("\t current IR status. 0: normal mode; 1: IR mode.\n");

    printf("e.g : %s 0 (SAMPLE_IR_CALIBRATION_MODE)\n", s_prg_nm);
    printf("e.g : %s 1 (SAMPLE_IR_AUTO_MODE, default parameters)\n", s_prg_nm);
    printf("e.g : %s 1 16000 400 280 190 280 190 1 (SAMPLE_IR_AUTO_MODE, user_define parameters)\n", s_prg_nm);

    return;
}

td_s32 isp_ir_switch_to_normal(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ot_isp_saturation_attr isp_saturation_attr;
    ot_isp_wb_attr isp_wb_attr;
    ot_isp_color_matrix_attr isp_ccm_attr;

    /* switch ir-cut to normal */
    sample_ir_cut_enable();
    /* switch isp config to normal */
    ret = ss_mpi_isp_get_saturation_attr(vi_pipe, &isp_saturation_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_saturation_attr failed\n");
        return ret;
    }
    isp_saturation_attr.op_type = OT_OP_MODE_AUTO;
    ret = ss_mpi_isp_set_saturation_attr(vi_pipe, &isp_saturation_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_saturation_attr failed\n");
        return ret;
    }

    ret = ss_mpi_isp_get_wb_attr(vi_pipe, &isp_wb_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_wb_attr failed\n");
        return ret;
    }
    isp_wb_attr.op_type = OT_OP_MODE_AUTO;
    ret = ss_mpi_isp_set_wb_attr(vi_pipe, &isp_wb_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_wb_attr failed\n");
        return ret;
    }

    ret = ss_mpi_isp_get_ccm_attr(vi_pipe, &isp_ccm_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_ccm_attr failed\n");
        return ret;
    }
    isp_ccm_attr.op_type = OT_OP_MODE_AUTO;
    ret = ss_mpi_isp_set_ccm_attr(vi_pipe, &isp_ccm_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_ccm_attr failed\n");
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 isp_ir_change_ccm_attr(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ot_isp_color_matrix_attr isp_ccm_attr;
    ret = ss_mpi_isp_get_ccm_attr(vi_pipe, &isp_ccm_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_ccm_attr failed\n");
        return ret;
    }
    isp_ccm_attr.op_type = OT_OP_MODE_MANUAL;
    isp_ccm_attr.manual_attr.ccm[0] = 0x100;
    isp_ccm_attr.manual_attr.ccm[1] = 0x0;
    isp_ccm_attr.manual_attr.ccm[2] = 0x0; // idx 2
    isp_ccm_attr.manual_attr.ccm[3] = 0x0; // idx 3
    isp_ccm_attr.manual_attr.ccm[4] = 0x100; // idx 4
    isp_ccm_attr.manual_attr.ccm[5] = 0x0; // idx 5
    isp_ccm_attr.manual_attr.ccm[6] = 0x0; // idx 6
    isp_ccm_attr.manual_attr.ccm[7] = 0x0; // idx 7
    isp_ccm_attr.manual_attr.ccm[8] = 0x100; // idx 8
    ret = ss_mpi_isp_set_ccm_attr(vi_pipe, &isp_ccm_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_ccm_attr failed\n");
        return ret;
    }
    return TD_SUCCESS;
}

td_s32 isp_ir_switch_to_ir(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    ot_isp_saturation_attr isp_saturation_attr;
    ot_isp_wb_attr isp_wb_attr;

    /* switch isp config to ir */
    ret = ss_mpi_isp_get_saturation_attr(vi_pipe, &isp_saturation_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_saturation_attr failed\n");
        return ret;
    }
    isp_saturation_attr.op_type = OT_OP_MODE_MANUAL;
    isp_saturation_attr.manual_attr.saturation = 0;
    ret = ss_mpi_isp_set_saturation_attr(vi_pipe, &isp_saturation_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_saturation_attr failed\n");
        return ret;
    }

    ret = ss_mpi_isp_get_wb_attr(vi_pipe, &isp_wb_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_wb_attr failed\n");
        return ret;
    }
    isp_wb_attr.op_type = OT_OP_MODE_MANUAL;
    isp_wb_attr.manual_attr.b_gain  = 0x100;
    isp_wb_attr.manual_attr.gb_gain = 0x100;
    isp_wb_attr.manual_attr.gr_gain = 0x100;
    isp_wb_attr.manual_attr.r_gain  = 0x100;
    ret = ss_mpi_isp_set_wb_attr(vi_pipe, &isp_wb_attr);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_set_wb_attr failed\n");
        return ret;
    }

    ret = isp_ir_change_ccm_attr(vi_pipe);
    if (TD_SUCCESS != ret) {
        return ret;
    }

    usleep(1000000); // sleep 1000000 us

    /* switch ir-cut to ir */
    sample_ir_cut_disable();

    return TD_SUCCESS;
}

td_s32 sample_isp_ir_auto_run(ot_vi_pipe vi_pipe)
{
    td_s32 ret = TD_SUCCESS;
    ot_isp_ir_auto_attr ir_attr;

    ir_attr.enable            = g_ast_ir_attr[vi_pipe].enable;
    ir_attr.normal_to_ir_iso_threshold = g_ast_ir_attr[vi_pipe].normal_to_ir_iso_threshold;
    ir_attr.ir_to_normal_iso_threshold = g_ast_ir_attr[vi_pipe].ir_to_normal_iso_threshold;
    ir_attr.rg_max           = g_ast_ir_attr[vi_pipe].rg_max;
    ir_attr.rg_min           = g_ast_ir_attr[vi_pipe].rg_min;
    ir_attr.bg_max           = g_ast_ir_attr[vi_pipe].bg_max;
    ir_attr.bg_min           = g_ast_ir_attr[vi_pipe].bg_min;
    ir_attr.ir_status         = g_ast_ir_attr[vi_pipe].ir_status;
    ir_attr.ir_switch         = g_ast_ir_attr[vi_pipe].ir_switch;
    while (TD_TRUE == g_ast_ir_thread[vi_pipe].thread_flag) {
        /* run_interval: 40 ms */
        usleep(40000 * 4); // sleep 40000 * 4 us
        ret = ss_mpi_isp_ir_auto(vi_pipe, &ir_attr);
        if (TD_SUCCESS != ret) {
            printf("ss_mpi_isp_ir_auto_run_once failed\n");
            return ret;
        }

        if (OT_ISP_IR_SWITCH_TO_IR == ir_attr.ir_switch) { /* normal to IR */
            printf("\n[normal -> IR]\n");

            ret = isp_ir_switch_to_ir(vi_pipe);
            if (TD_SUCCESS != ret) {
                printf("isp_ir_switch_to_ir failed\n");
                return ret;
            }

            ir_attr.ir_status = OT_ISP_IR_STATUS_IR;
        } else if (OT_ISP_IR_SWITCH_TO_NORMAL == ir_attr.ir_switch) { /* IR to normal */
            printf("\n[IR -> normal]\n");

            ret = isp_ir_switch_to_normal(vi_pipe);
            if (TD_SUCCESS != ret) {
                printf("isp_ir_switch_to_normal failed\n");
                return ret;
            }

            ir_attr.ir_status = OT_ISP_IR_STATUS_NORMAL;
        }
    }

    return TD_SUCCESS;
}

static void* sample_isp_ir_auto_thread(void* param)
{
    ot_vi_pipe vi_pipe;

    vi_pipe = *(ot_vi_pipe *)param;

    prctl(PR_SET_NAME, "isp_ir_auto", 0, 0, 0);

    sample_isp_ir_auto_run(vi_pipe);

    return NULL;
}

td_s32 sample_isp_ir_auto_exit(ot_vi_pipe vi_pipe)
{
    if (g_ast_ir_thread[vi_pipe].thread) {
        g_ast_ir_thread[vi_pipe].thread_flag = TD_FALSE;
        pthread_join(g_ast_ir_thread[vi_pipe].thread, NULL);
        g_ast_ir_thread[vi_pipe].thread = 0;
    }

    return TD_SUCCESS;
}
void sample_ir_auto_handle_sig(td_s32 signo)
{
    int ret;

    if (SIGINT == signo || SIGTERM == signo) {
        ret = sample_isp_ir_auto_exit(g_vi_pipe);
        if (ret != 0) {
            printf("sample_isp_ir_auto_exit failed\n");
            return;
        }
    }
    return;
}

td_s32 sample_ir_calibration(td_void)
{
    ot_isp_wb_stats stat;
    td_u32 rg, bg;
    td_s32 ret;

    /* 1. infrared scene without visible light */

    /* 2. switch to IR */
    ret = isp_ir_switch_to_ir(g_vi_pipe);
    if (TD_SUCCESS != ret) {
        printf("isp_ir_switch_to_ir failed\n");
        return ret;
    }

    /* 3. waiting for AE to stabilize */
    usleep(1000000); // sleep 1000000 us

    /* 4. calculate rg_max/rg_min/bg_max/bg_min */
    ret = ss_mpi_isp_get_wb_stats(g_vi_pipe, &stat);
    if (TD_SUCCESS != ret) {
        printf("ss_mpi_isp_get_statistics failed\n");
        return ret;
    }

    rg = ((td_u32)stat.global_r << SHIFT_8BIT) / div_0_to_1(stat.global_g);
    bg = ((td_u32)stat.global_b << SHIFT_8BIT) / div_0_to_1(stat.global_g);
    printf("rg_max:%u, rg_min:%u, bg_max:%u, bg_min:%u\n",
           (rg * GAIN_MAX_COEF) >> SHIFT_8BIT, (rg * GAIN_MIN_COEF) >> SHIFT_8BIT,
           (bg * GAIN_MAX_COEF) >> SHIFT_8BIT, (bg * GAIN_MIN_COEF) >> SHIFT_8BIT);

    return TD_SUCCESS;
}


int sample_ir_auto_thread(int argc, char* argv[])
{
    td_s32 ret = TD_SUCCESS;
    td_u32 mode = SAMPLE_IR_AUTO_MODE;
    td_char *para_stop;

    g_vi_pipe = 0;
    mode = (td_u32)strtol(argv[1], &para_stop, 10); /* idx 2 10 dec */
    /* SAMPLE_IR_CALIBRATION_MODE */
    if (mode == SAMPLE_IR_CALIBRATION_MODE) {
        sample_ir_calibration();
    }
    /* SAMPLE_IR_AUTO_MODE */
    if (argc > 8) { // argc num 8
        g_ast_ir_attr[g_vi_pipe].normal_to_ir_iso_threshold =
            (td_u32)strtol(argv[2], &para_stop, 10); /* idx 2, 10 dec */
        g_ast_ir_attr[g_vi_pipe].ir_to_normal_iso_threshold =
            (td_u32)strtol(argv[3], &para_stop, 10); /* idx 3, 10 dec */
        g_ast_ir_attr[g_vi_pipe].rg_max = (td_u32)strtol(argv[4], &para_stop, 10); /* idx 4, 10 dec */
        g_ast_ir_attr[g_vi_pipe].rg_min = (td_u32)strtol(argv[5], &para_stop, 10); /* idx 5, 10 dec */
        g_ast_ir_attr[g_vi_pipe].bg_max = (td_u32)strtol(argv[6], &para_stop, 10); /* idx 6, 10 dec */
        g_ast_ir_attr[g_vi_pipe].bg_min = (td_u32)strtol(argv[7], &para_stop, 10); /* idx 7, 10 dec */
        g_ast_ir_attr[g_vi_pipe].ir_status = (td_u32)strtol(argv[8], &para_stop, 10); /* idx 8, 10 dec */
        if ((g_ast_ir_attr[g_vi_pipe].ir_status != OT_ISP_IR_STATUS_NORMAL)
            && (g_ast_ir_attr[g_vi_pipe].ir_status != OT_ISP_IR_STATUS_IR)) {
            printf("the mode is invalid!\n");
            sample_ir_auto_usage(argv[0]);
            return TD_SUCCESS;
        }
        printf("[user_config] normal_to_ir_iso_threshold:%u, ir_to_normal_iso_threshold:%u,"
        "RG:[%u,%u], BG:[%u,%u], ir_status:%d\n",
            g_ast_ir_attr[g_vi_pipe].normal_to_ir_iso_threshold, g_ast_ir_attr[g_vi_pipe].ir_to_normal_iso_threshold,
            g_ast_ir_attr[g_vi_pipe].rg_min, g_ast_ir_attr[g_vi_pipe].rg_max,
            g_ast_ir_attr[g_vi_pipe].bg_min, g_ast_ir_attr[g_vi_pipe].bg_max,
            g_ast_ir_attr[g_vi_pipe].ir_status);
    }
    g_ast_ir_thread[g_vi_pipe].thread_flag = TD_TRUE;

    ret = pthread_create(&g_ast_ir_thread[g_vi_pipe].thread, TD_NULL, sample_isp_ir_auto_thread, (td_void *)&g_vi_pipe);
    if (ret != 0) {
        printf("%s: create isp ir_auto thread failed!, error: %d, %s\r\n", __FUNCTION__, ret, strerror(ret));
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}
td_s32 sample_ir_parse_args(int argc, char *argv[])
{
    td_u32 mode = SAMPLE_IR_AUTO_MODE;

    if (argc < 2) { // argc num is 2
        sample_ir_auto_usage(argv[0]);
        return SAMPLE_IR_AUTO_RETURN;
    }
    if (!strncmp(argv[1], "-h", 2)) { // size 2
        sample_ir_auto_usage(argv[0]);
        return SAMPLE_IR_AUTO_RETURN;
    }

    if (strlen(argv[1]) != 1) {
        sample_ir_auto_usage(argv[0]);
        return TD_FAILURE;
    }

    mode = atoi(argv[1]); // idx 1
    if ((mode != SAMPLE_IR_AUTO_MODE) && (mode != SAMPLE_IR_CALIBRATION_MODE)) {
        printf("the mode is invalid!\n");
        sample_ir_auto_usage(argv[0]);
        return SAMPLE_IR_AUTO_RETURN;
    }

    if ((mode == SAMPLE_IR_CALIBRATION_MODE) && (argc != 2)) {  // num of argc 2
        printf("SAMPLE_IR_CALIBRATION_MODE, input parameter error\n");
        sample_ir_auto_usage(argv[0]);
        return SAMPLE_IR_AUTO_RETURN;
    }

    if ((mode == SAMPLE_IR_AUTO_MODE) && (argc != 9) && (argc != 2)) { // num of argc 9, 2
        printf("SAMPLE_IR_AUTO_MODE, input parameter error\n");
        sample_ir_auto_usage(argv[0]);
        return SAMPLE_IR_AUTO_RETURN;
    }

    return TD_SUCCESS;
}

static td_void sample_ir_cut_handle_sig(td_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        g_sig_flag = 1;
    }
}

static td_void sample_register_sig_handler(td_void (*sig_handle)(td_s32))
{
    struct sigaction sa;

    (td_void)memset_s(&sa, sizeof(struct sigaction), 0, sizeof(struct sigaction));
    sa.sa_handler = sig_handle;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, TD_NULL);
    sigaction(SIGTERM, &sa, TD_NULL);
}

static td_void ir_auto_getchar(td_void)
{
    if (g_sig_flag == 1) {
        return;
    }

    (td_void)getchar();

    if (g_sig_flag == 1) {
        return;
    }
}

/******************************************************************************
* function    : main()
* description : video preview sample
******************************************************************************/
#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char* argv[])
#endif
{
    td_s32 ret = TD_FAILURE;
    g_vi_pipe = 0;
    /* need to modified when sensor/lens/IR-cut/infrared light changed */
    g_ast_ir_attr[g_vi_pipe].enable = TD_TRUE;
    g_ast_ir_attr[g_vi_pipe].normal_to_ir_iso_threshold= 16000; // normal_to_ir_iso_threshold 16000
    g_ast_ir_attr[g_vi_pipe].ir_to_normal_iso_threshold = 400; // ir_to_normal_iso_threshold 400
    g_ast_ir_attr[g_vi_pipe].rg_max = 280; // max 280
    g_ast_ir_attr[g_vi_pipe].rg_min = 190; // 190
    g_ast_ir_attr[g_vi_pipe].bg_max = 280; // 280
    g_ast_ir_attr[g_vi_pipe].bg_min = 190; // 190
    g_ast_ir_attr[g_vi_pipe].ir_status = OT_ISP_IR_STATUS_IR;
    ret = sample_ir_parse_args(argc, argv);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#ifndef __LITEOS__
    sample_register_sig_handler(sample_ir_cut_handle_sig);
#endif
    ir_auto_prev ir_auto_prev;
    td_s32 s32ret;
    s32ret = sample_ir_auto_start_prev(&ir_auto_prev);
    if (TD_SUCCESS == s32ret) {
        sample_print("ISP is now running normally\n");
    } else {
        sample_print("ISP is not running normally!please check it\n");
        return -1;
    }
    printf("input anything to continue....\n");
    ir_auto_getchar();

    sleep(2); // sleep 2s
    ret = sample_ir_auto_thread(argc, argv);
    if (ret != TD_SUCCESS) {
        goto EXIT;
    }
    ir_auto_getchar();
    sample_isp_ir_auto_exit(g_vi_pipe);
EXIT:
    printf("input anything to continue....\n");
    ir_auto_getchar();
    sample_ir_auto_stop_prev(&ir_auto_prev);
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
