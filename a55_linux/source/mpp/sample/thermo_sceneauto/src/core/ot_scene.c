/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include "securec.h"
#include <math.h>

#include "ot_scene_inner.h"
#include "ot_scenecomm.h"
#include "ot_scene_setparam.h"
#include "sample_comm.h"
#include "ss_mpi_ae.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_sys.h"

#include "ss_mpi_thermo.h"
#include "scene_setparam_inner.h"
#include "ss_mpi_aidestrip.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* notes scene mode */
static ot_scene_mode g_scene_mode;

/* notes scene state */
static scene_state g_scene_state;

static ot_scene_fps g_scene_auto_fps = {0};

static td_s32 g_init_iso = -1;
static td_s64 g_init_exp = -1;

static td_u8 g_ooc_data[308][400] = { [0 ... (308-1)][0 ... (400-1)] = 32 };
static td_u32 g_tmp_raw[308][400] = {{0}};

static int g_fd[OT_ISP_MAX_PIPE_NUM] = {[0 ...(OT_ISP_MAX_PIPE_NUM - 1)] = -1};
sample_vi_user_frame_info g_user_frame_info; // OOC frame
#define AE_HIST_BIN_NUM 1024
#define DRC_TM_NODE_NUM 200
#define MAX_DATA_VALUE 65536
#define MIN_THERMO_ISP_DGAIN 3000
#define AIDESTRIP_STRENGTH 8
#define DRC_COEF 10
#define USER_BLC 6000
#define SHARE_MEM_ID 1234
#define SHARE_MEM_SIZE 1024
#define SHARE_MEM_PARAM 0600

static td_u8 g_t3_sns_cfg[] = {
    0x40, 0x10, 0x03, 0xFF, 0xC0, 0x01, 0x8C, 0xAB,
    0x0E, 0x10, 0x82, 0x0F, 0x11, 0x60, 0x84, 0xB6,
    0xF4, 0xBC, 0x20, 0x00, 0x00, 0x07, 0xFF, 0xBF,
    0xFF, 0xFF, 0xFF, 0xF4, 0x78, 0x00, 0x10, 0xAA,
    0x1C, 0x00, 0x8A, 0xA0, 0xA3, 0xD0, 0x80, 0x00,
    0x4F, 0x83, 0x34, 0xB0, 0x32, 0x2C, 0x1E, 0xF2,
};

static sample_vi_fpn_correction_cfg g_correction_cfg = {
    .op_mode       = OT_OP_MODE_MANUAL,
    .fpn_type      = OT_ISP_FPN_TYPE_FRAME,
    .strength      = 256,
    .pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP,
    .compress_mode = OT_COMPRESS_MODE_NONE,
};

static sample_vi_fpn_calibration_cfg g_calibration_cfg = {
    .threshold     = 4095,
    .frame_num     = 4,
    .fpn_type      = OT_ISP_FPN_TYPE_FRAME,
    .pixel_format  = OT_PIXEL_FORMAT_RGB_BAYER_16BPP,
    .compress_mode = OT_COMPRESS_MODE_NONE,
};


/* scene lock */
pthread_mutex_t g_scene_lock = PTHREAD_MUTEX_INITIALIZER;

/* use to check if the module init */
#define scene_check_init_return() do {                                                             \
        pthread_mutex_lock(&g_scene_lock);                                                         \
        if (g_scene_state.scene_init == TD_FALSE) {                                                \
            scene_loge("func:%s,line:%d, please init sceneauto first!\n", __FUNCTION__, __LINE__); \
            pthread_mutex_unlock(&g_scene_lock);                                                   \
            return OT_SCENE_ENOTINIT;                                                              \
        }                                                                                          \
        pthread_mutex_unlock(&g_scene_lock);                                                       \
    } while (0)

/* use to check if scene was pause */
#define scene_check_pause_return() do {       \
        if (g_scene_state.pause == TD_TRUE) { \
            return TD_SUCCESS;                \
        }                                     \
    } while (0)

#define scene_thermo_check_fp_return(fp, file_name)                                   \
    do {                                                       \
        if (fp == NULL) {    \
            printf("Cannot open %s.\n", file_name); \
            return;                   \
        }                                                      \
    } while (0)

/* -------------------------internal function interface------------------------- */
td_void scene_thermo_gpio_export(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[THERMO_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, THERMO_FILE_NAME_LEN, "/sys/class/gpio/export");
    if (ret < 0) {
        sample_print("export sprintf_s failed with %d.\n", ret);
        return;
    }
    fp = fopen(file_name, "w");
    scene_thermo_check_fp_return(fp, file_name);
    ret = fprintf(fp, "%u", gpio_num);
    if (ret < 0) {
        sample_print("print gpio_num failed!!!\n");
    }

    ret = fclose(fp);
    if (ret != 0) {
        sample_print("export close fp err ret:%d\n", ret);
    }
    return;
}

td_void scene_thermo_gpio_dir(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[THERMO_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, THERMO_FILE_NAME_LEN, "/sys/class/gpio/gpio%u/direction", gpio_num);
    if (ret < 0) {
        sample_print("dir sprintf_s failed with %d.\n", ret);
        return;
    }
    fp = fopen(file_name, "rb+");
    scene_thermo_check_fp_return(fp, file_name);
    ret = fprintf(fp, "out");
    if (ret < 0) {
        sample_print("print out failed!!!\n");
    }
    ret = fclose(fp);
    if (ret != 0) {
        sample_print("dir close fp err, ret:%d\n", ret);
    }

    return;
}

td_void scene_thermo_gpio_write(td_u32 gpio_chip_num, td_u32 gpio_offset, td_u32 gpio_out_val)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char buf[LEN];
    char file_name[THERMO_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, THERMO_FILE_NAME_LEN, "/sys/class/gpio/gpio%u/value", gpio_num);
    if (ret < 0) {
        sample_print("write sprintf_s failed with %d.\n", ret);
        return;
    }

    fp = fopen(file_name, "rb+");
    scene_thermo_check_fp_return(fp, file_name);

    if (gpio_out_val) {
        strcpy_s(buf, LEN, "1");
    } else {
        strcpy_s(buf, LEN, "0");
    }
    ret = fwrite(buf, sizeof(char), sizeof(buf) - 1, fp);
    if (ret != (sizeof(buf) - 1)) {
        sample_print("%s: gpio%u_%u = %s, fwrite err ret:%d\n", __func__, gpio_chip_num, gpio_offset, buf, ret);
    }
    ret = fclose(fp);
    if (ret != 0) {
        sample_print("write close fp err, ret:%d\n", ret);
    }

    return;
}

td_void scene_thermo_gpio_unexport(td_u32 gpio_chip_num, td_u32 gpio_offset)
{
    td_s32 ret;
    FILE *fp = TD_NULL;
    char file_name[THERMO_FILE_NAME_LEN];
    td_u32 gpio_num = gpio_chip_num * 8 + gpio_offset; // 8 pins

    ret = sprintf_s(file_name, THERMO_FILE_NAME_LEN, "/sys/class/gpio/unexport");
    if (ret < 0) {
        sample_print("unexport sprintf_s failed with %d.\n", ret);
        return;
    }

    fp = fopen(file_name, "w");
    scene_thermo_check_fp_return(fp, file_name);
    ret = fprintf(fp, "%u", gpio_num);
    if (ret < 0) {
        sample_print("print unexport gpio_num failed!!!\n");
    }
    ret = fclose(fp);
    if (ret != 0) {
        sample_print("unexport close fp err, ret:%d\n", ret);
    }

    return;
}

td_s32 thermo_gst412c_i2c_init(ot_vi_pipe vi_pipe)
{
    char dev_file[I2C_DEV_FILE_NUM] = {0};

    if (g_fd[vi_pipe] >= 0) {
        return TD_SUCCESS;
    }
#ifdef OT_GPIO_I2C
    g_fd[vi_pipe] = open("/dev/gpioi2c_ex", O_RDONLY, S_IRUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open gpioi2c_ex error!\n");
        return TD_FAILURE;
    }
#else
    td_u8 dev_num = GST412C_DEV_NUM;
    (td_void)snprintf_s(dev_file, sizeof(dev_file), sizeof(dev_file) - 1, "/dev/i2c-%u", dev_num);

    g_fd[vi_pipe] = open(dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_fd[vi_pipe] < 0) {
        isp_err_trace("Open /dev/ot_i2c_drv-%u error!\n", dev_num);
        return TD_FAILURE;
    }

    td_s32 ret = ioctl(g_fd[vi_pipe], OT_I2C_SLAVE_FORCE, (GST412C_I2C_ADDR >> 1));
    if (ret < 0) {
        isp_err_trace("I2C_SLAVE_FORCE error!\n");
        close(g_fd[vi_pipe]);
        g_fd[vi_pipe] = -1;
        return ret;
    }
#endif

    return TD_SUCCESS;
}

td_u32 i2c_ioc_init(struct i2c_rdwr_ioctl_data *rdwr, unsigned char *buf, size_t buf_size, struct i2c_rdwr_args args)
{
    if (memset_s(buf, buf_size, 0, I2C_READ_BUF_LEN) != EOK) {
        sample_print("memset_s fail!\n");
        return -1;
    }
    rdwr->msgs[0].addr = args.dev_addr;
    rdwr->msgs[1].addr = args.dev_addr;
    rdwr->msgs[0].flags = 0;
    rdwr->msgs[1].flags = 0;
    rdwr->msgs[1].flags |= I2C_M_RD;
    rdwr->msgs[0].len = args.reg_width;
    rdwr->msgs[1].len = args.data_width;
    rdwr->msgs[0].buf = buf;
    rdwr->msgs[1].buf = buf;

    return 0;
}

td_u32 thermo_gst412c_read_register(ot_vi_pipe vi_pipe, td_u32 addr)
{
    if (g_fd[vi_pipe] < 0) {
        sample_print("gst412c_read_register fd not opened!\n");
        return TD_FAILURE;
    }
    struct i2c_rdwr_args args;
    static struct i2c_rdwr_ioctl_data rdwr;
    static struct i2c_msg msg[I2C_MSG_CNT];
    unsigned char buf[I2C_READ_BUF_LEN];
    td_u32 cur_addr;
    td_u32 data;

    rdwr.msgs = &msg[0];
    rdwr.nmsgs = (__u32)I2C_MSG_CNT;
    args.i2c_num = GST412C_DEV_NUM;
    args.dev_addr = (GST412C_I2C_ADDR >> 1);
    args.reg_width = GST412C_ADDR_BYTE;
    args.data_width = GST412C_DATA_BYTE;
    args.reg_addr = addr;
    args.w_r_union.reg_addr_end = addr;
    args.reg_step = 1;
    if (i2c_ioc_init(&rdwr, buf, sizeof(buf), args) != 0) {
        return -1;
    }

    for (cur_addr = args.reg_addr; cur_addr <= args.w_r_union.reg_addr_end; cur_addr += args.reg_step) {
        if (args.reg_width == 2) {  /* 2 byte */
            buf[0] = (cur_addr >> 8) & 0xff;  /* shift 8 */
            buf[1] = cur_addr & 0xff;
        } else {
            buf[0] = cur_addr & 0xff;
        }

        if (ioctl(g_fd[vi_pipe], I2C_RDWR, &rdwr) != I2C_READ_STATUS_OK) {
            sample_print("CMD_I2C_READ error!\n");
            return TD_FAILURE;
        }

        if (args.data_width == 2) {  /* 2 byte */
            data = buf[1] | (buf[0] << 8);  /* shift 8 */
        } else {
            data = buf[0];
        }
    }

    return data;
}

td_s32 thermo_gst412c_write_register(ot_vi_pipe vi_pipe, td_u32 addr, td_u32 data)
{
    if (g_fd[vi_pipe] < 0) {
        return TD_SUCCESS;
    }
    td_s32 ret;
#ifdef OT_GPIO_I2C
    i2c_data.dev_addr = GST412C_I2C_ADDR;
    i2c_data.reg_addr = addr;
    i2c_data.addr_byte_num = GST412C_ADDR_BYTE;
    i2c_data.data_byte_num = GST412C_DATA_BYTE;
    i2c_data.data = data;

    ret = ioctl(g_fd[vi_pipe], GPIO_I2C_WRITE, &i2c_data);
    if (ret) {
        isp_err_trace("GPIO-I2C write failed!\n");
        return ret;
    }
#else
    td_u8 buf[I2C_BUF_NUM];
    td_u32 idx_buf = 0;
    td_s32 addr_byte = GST412C_ADDR_BYTE;
    td_s32 data_byte = GST412C_DATA_BYTE;
    if (addr_byte == 2) {  /* 2 byte */
        buf[idx_buf] = (addr >> 8) & 0xff;  /* shift 8 */
        idx_buf++;
        buf[idx_buf] = addr & 0xff;
        idx_buf++;
    } else {
        buf[idx_buf] = addr & 0xff;
        idx_buf++;
    }

    if (data_byte == 2) {  /* 2 byte */
    } else {
        buf[idx_buf] = data & 0xff;
        idx_buf++;
    }

    ret = write(g_fd[vi_pipe], buf, GST412C_ADDR_BYTE + GST412C_DATA_BYTE);
    if (ret < 0) {
        isp_err_trace("I2C_WRITE error!\n");
        return TD_FAILURE;
    }

#endif
    return TD_SUCCESS;
}

static td_void scene_get_mainpipe_array(td_u32 *pipe_cnt)
{
    td_u32 main_pipe_cnt = 0;
    td_u32 j;

    /* get mainpipe array */
    for (td_s32 i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        if (g_scene_mode.pipe_attr[i].enable != TD_TRUE) {
            continue;
        }

        if (main_pipe_cnt == 0) {
            g_scene_state.main_pipe[main_pipe_cnt].main_pipe_hdl = g_scene_mode.pipe_attr[i].main_pipe_hdl;
            main_pipe_cnt++;
            continue;
        }

        for (j = 0; j < main_pipe_cnt; j++) {
            if (g_scene_state.main_pipe[j].main_pipe_hdl == g_scene_mode.pipe_attr[i].main_pipe_hdl) {
                break;
            }
        }

        if (main_pipe_cnt == j) {
            g_scene_state.main_pipe[main_pipe_cnt].main_pipe_hdl = g_scene_mode.pipe_attr[i].main_pipe_hdl;
            main_pipe_cnt++;
        }
    }
    *pipe_cnt = main_pipe_cnt;
}

static td_s32 scene_set_main_pipe_state(const ot_scene_mode *scene_mode)
{
    td_u32 i, j;
    td_u32 main_pipe_cnt = 0;
    /* if not use, set to 0 */
    (td_void)memset_s(g_scene_state.main_pipe, sizeof(g_scene_state.main_pipe), 0, sizeof(g_scene_state.main_pipe));

    for (i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        g_scene_state.main_pipe[i].long_exp = TD_FALSE;
        g_scene_state.main_pipe[i].metry_fixed = TD_FALSE;
    }

    /* get mainpipe array */
    scene_get_mainpipe_array(&main_pipe_cnt);

    /* set subpipe in certain mainpipe */
    for (i = 0; i < main_pipe_cnt; i++) {
        td_u32 sub_pipe_cnt = 0;

        for (j = 0; j < OT_SCENE_PIPE_MAX_NUM; j++) {
            if (g_scene_mode.pipe_attr[j].enable != TD_TRUE) {
                continue;
            }

            ot_scenecomm_expr_true_return(sub_pipe_cnt >= OT_SCENE_PIPE_MAX_NUM, TD_FAILURE);
            if (g_scene_state.main_pipe[i].main_pipe_hdl == g_scene_mode.pipe_attr[j].main_pipe_hdl) {
                g_scene_state.main_pipe[i].sub_pipe_hdl[sub_pipe_cnt] = g_scene_mode.pipe_attr[j].vcap_pipe_hdl;
                sub_pipe_cnt++;
            }
        }

        g_scene_state.main_pipe[i].sub_pipe_num = sub_pipe_cnt;
    }

    g_scene_state.main_pipe_num = main_pipe_cnt;

    for (i = 0; i < g_scene_state.main_pipe_num; i++) {
        scene_logd("The mainpipe is %u.", g_scene_state.main_pipe[i].main_pipe_hdl);
        for (j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            scene_logd("The subpipe in mainpipe %d is %u.", g_scene_state.main_pipe[i].main_pipe_hdl,
                g_scene_state.main_pipe[i].sub_pipe_hdl[j]);
        }
        scene_logd("\n");
    }
    return TD_SUCCESS;
}

static td_s32 scene_calculate_exp(ot_vi_pipe vi_pipe, td_u32 *out_iso, td_u64 *out_exposure)
{
    td_s32 ret;
    td_u64 exposure;
    td_u32 iso_info;

    ot_isp_exp_info isp_exp_info;
    ot_isp_pub_attr pub_attr;

    scene_check_pause_return();

    ret = ss_mpi_isp_query_exposure_info(vi_pipe, &isp_exp_info);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    iso_info = isp_exp_info.iso;

    ret = ss_mpi_isp_get_pub_attr(vi_pipe, &pub_attr);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    if (pub_attr.wdr_mode == OT_WDR_MODE_4To1_LINE) {
        exposure = ((td_u64)iso_info * isp_exp_info.long_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_3To1_LINE) {
        exposure = ((td_u64)iso_info * isp_exp_info.median_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_2To1_LINE) {
        exposure = ((td_u64)iso_info * isp_exp_info.short_exp_time) / 100; /* 100 as AE ISO ratio */
    } else if (pub_attr.wdr_mode == OT_WDR_MODE_2To1_FRAME) {
        exposure = ((td_u64)iso_info * isp_exp_info.short_exp_time) / 100; /* 100 as AE ISO ratio */
    } else {
        exposure = ((td_u64)iso_info * isp_exp_info.exp_time) / 100; /* 100 as AE ISO ratio */
    }
    *out_iso = iso_info;
    *out_exposure = exposure;

    return TD_SUCCESS;
}

static td_s32 scene_set_pipe_dynamic_param(td_void)
{
    td_s32 ret;
    ot_3dnr_pos_type pos = OT_3DNR_POS_VI;

    ret = ss_mpi_sys_get_3dnr_pos(&pos);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
        for (td_u32 j = 0; j < g_scene_state.main_pipe[i].sub_pipe_num; j++) {
            td_s32 index = g_scene_state.main_pipe[i].sub_pipe_hdl[j];
            td_u8 pipe_index = g_scene_mode.pipe_attr[index].pipe_param_index;
            if (g_scene_mode.pipe_attr[index].pipe_type == OT_SCENE_PIPE_TYPE_SNAP) {
                continue;
            }
            td_u32 iso = g_scene_state.main_pipe[i].iso;
            if (pos == OT_3DNR_POS_VI) {
                ret = ot_scene_set_dynamic_3dnr(g_scene_mode.pipe_attr[index].vcap_pipe_hdl, iso, pipe_index, pos);
            } else if (pos == OT_3DNR_POS_VPSS) {
                ret = ot_scene_set_dynamic_3dnr(g_scene_mode.pipe_attr[index].vpss_hdl, iso, pipe_index, pos);
            }
            ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
        }
    }

    return TD_SUCCESS;
}

static td_s32 scene_set_vi_pipe_param(td_void)
{
    /* set mainIsp param */
    td_s32 ret;
    for (td_u32 i = 0; i < g_scene_state.main_pipe_num; i++) {
        ot_vi_pipe vi_pipe = g_scene_state.main_pipe[i].main_pipe_hdl;

        if ((g_scene_state.thread_normal.thread_flag == 0) && (g_init_iso != -1) && (g_init_exp != -1)) {
            scene_logd("init iso: %d exp: %lld\n", g_init_iso, g_init_exp);
            g_scene_state.main_pipe[i].exposure = g_init_exp;
            g_scene_state.main_pipe[i].iso = g_init_iso;
        } else {
            ret = scene_calculate_exp(vi_pipe, &(g_scene_state.main_pipe[i].iso),
                &(g_scene_state.main_pipe[i].exposure));
        }
        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    return TD_SUCCESS;
}

td_void set_ooc_data(sample_vi_user_frame_info *user_frame_info, td_u8 ooc_data[][400])
{
    td_u32 i, j;
    ot_vi_dev vi_dev = 1;
    ot_vi_thermo_sns_attr sns_attr;

    ot_video_frame *v_buf1 = &(user_frame_info->frame_info.video_frame);
    td_u8 *data;
    data = (td_u8*)(td_uintptr_t)v_buf1->virt_addr[0];

    for (i = 0; i < THERMO_HEIGHT_308; i++) {
        for (j = 0; j < THERMO_WIDTH_400; j++) {
            data[j] = ooc_data[i][j] * 4;   /* multi 4 */
        }
        data = data + v_buf1->stride[0];
    }

    sns_attr.work_mode = OT_VI_THERMO_WORK_MODE_T3;
    sns_attr.cfg_num = (td_u32)(sizeof(g_t3_sns_cfg) / sizeof(td_u8));
    (td_void)memcpy_s(&sns_attr.sns_cfg, sizeof(td_u8) * OT_VI_MAX_SNS_CFG_NUM, g_t3_sns_cfg, sizeof(g_t3_sns_cfg));

    memcpy_s(&sns_attr.ooc_frame_info, sizeof(ot_video_frame_info),
             &user_frame_info->frame_info, sizeof(ot_video_frame_info));
    sns_attr.frame_rate = 30;                  /* framerate 30 */
    sns_attr.sd_mux[0] = OT_VI_SD_MUX_T2_SDA0; /* pin_mux0:4 */
    sns_attr.sd_mux[1] = OT_VI_SD_MUX_T2_SDA1; /* pin_mux1:5 */
    sns_attr.sd_mux[2] = OT_VI_SD_MUX_T2_SDA2; /* pin_mux2:6 */
    sns_attr.sd_mux[3] = OT_VI_SD_MUX_T2_FS;   /* pin_mux3:7 */
    ss_mpi_vi_set_thermo_sns_attr(vi_dev, &sns_attr);
}

static void delay_ms(int ms)
{
    usleep(ms * 1000); /* 1ms: 1000us */
    return;
}

td_void scene_thermo_iris_control(td_u8 open)
{
    td_u32 gpio_chip_num2 = 2; // gpio2
    scene_thermo_gpio_export(gpio_chip_num2, 0x1);  // gpio2_1
    scene_thermo_gpio_export(gpio_chip_num2, 0x3);  // gpio2_3
    scene_thermo_gpio_dir(gpio_chip_num2, 0x1);  // gpio2_1 out
    scene_thermo_gpio_dir(gpio_chip_num2, 0x3);  // gpio2_3 out
    if (open == 1) {
        scene_thermo_gpio_write(gpio_chip_num2, 0x1, 0); // set GPIO2_1 0
        scene_thermo_gpio_write(gpio_chip_num2, 0x3, 1); // set GPIO2_3 1
        delay_ms(50); // delay 50ms
        scene_thermo_gpio_write(gpio_chip_num2, 0x1, 0); // set GPIO2_1 0
        scene_thermo_gpio_write(gpio_chip_num2, 0x3, 0); // set GPIO2_3 0
    } else if (open == 0) {
        scene_thermo_gpio_write(gpio_chip_num2, 0x1, 1); // set GPIO2_1 1
        scene_thermo_gpio_write(gpio_chip_num2, 0x3, 0); // set GPIO2_3 0
        delay_ms(50); // delay 50ms
        scene_thermo_gpio_write(gpio_chip_num2, 0x1, 0); // set GPIO2_1 0
        scene_thermo_gpio_write(gpio_chip_num2, 0x3, 0); // set GPIO2_3 0
    } else {}
    scene_thermo_gpio_unexport(gpio_chip_num2, 0x1);
    scene_thermo_gpio_unexport(gpio_chip_num2, 0x3);
}

static td_s32 scene_thermo_convert_14bit_pixel(td_u8 *data, td_u32 data_num, td_u32 bit_width, td_u16 *out_data)
{
    td_s32 i, tmp_data_num, out_cnt;
    td_u64 u64_val;
    td_u8 *tmp_data = data;

    out_cnt = 0;
    switch (bit_width) {
        case 14: {   /* 14 bit */
            /* 4 pixels consist of 7 bytes  */
            tmp_data_num = data_num / 4;

            for (i = 0; i < tmp_data_num; i++) {
                tmp_data = data + 7 * i;    /* 7 bytes */
                u64_val = tmp_data[0] + ((td_u32)tmp_data[1] << 8) +   /* 8 shift */
                    ((td_u32)tmp_data[0x2] << 16) + ((td_u32)tmp_data[0x3] << 24) +    /* 16 shift 24 shift */
                    ((td_u64)tmp_data[0x4] << 32) + ((td_u64)tmp_data[0x5] << 40) +    /* 32 shift 40 shift */
                    ((td_u64)tmp_data[0x6] << 48);    /* 48 shift */

                out_data[out_cnt++] = (u64_val >> 0)  & 0x3fff;
                out_data[out_cnt++] = (u64_val >> 14) & 0x3fff;   /* 14 shift */
                out_data[out_cnt++] = (u64_val >> 28) & 0x3fff;   /* 28 shift */
                out_data[out_cnt++] = (u64_val >> 42) & 0x3fff;   /* 42 shift */
            }
            break;
        }
        default:
            sample_print("unsuport bit_width: %d\n", bit_width);
            return TD_FAILURE;
    }

    return out_cnt;
}

td_s32 get_raw_data(ot_video_frame *v_buf, td_u32 nbit, td_u32 tmp_raw[][400], td_u32 height, td_u32 width)
{
    td_u32 i, j;
    td_u32 h_bias, w_bias;
    td_u64 phys_addr;
    td_u64 size;
    td_u8 *virt_addr;
    td_u8 *u8_data;

    size = (v_buf->stride[0]) * (v_buf->height);
    phys_addr = v_buf->phys_addr[0];
    virt_addr = (td_u8 *)ss_mpi_sys_mmap(phys_addr, size);
    if (virt_addr == TD_NULL) {
        sample_print("ss_mpi_sys_mmap failed!\n");
        return TD_FAILURE;
    }

    u8_data = virt_addr;
    td_u16 *u16_data = (td_u16 *)malloc(v_buf->width * 0x2);
    if (u16_data == TD_NULL) {
        sample_print("malloc memory failed\n");
        ss_mpi_sys_munmap(virt_addr, size);
        virt_addr = TD_NULL;
        return TD_FAILURE;
    }

    h_bias = (height - v_buf->height) / 0x2;
    w_bias = (width - v_buf->width) / 0x2;
    for (i = 0; i < v_buf->height; i++) {
        scene_thermo_convert_14bit_pixel(u8_data, v_buf->width, 14, u16_data);   /* 14 bit */
        for (j = 0; j < v_buf->width; j++) {
            tmp_raw[h_bias + i][w_bias + j] = u16_data[j];
        }
        u8_data += v_buf->stride[0];
    }
    free(u16_data);
    return TD_SUCCESS;
}

td_void update_ooc(td_u8 ooc_data[][400], td_u32 tmp_raw[][400], td_u32 ooc_step)
{
    td_u32 i, j;
    td_u32 data;
    for (i = 0; i < THERMO_HEIGHT; i++) {
        for (j = 0; j < THERMO_WIDTH; j++) {
            if ((tmp_raw[10 + i][8 + j] <= 8600) &&  /* bias 10 8 value 8600 */
                (tmp_raw[10 + i][8 + j] >= 7800)) { /* bias 10 8 value 7800 */
            } else if (tmp_raw[10 + i][8 + j] > 8600) {  /* bias 10 bias 8 value 8600 */
                data = ooc_data[10 + i][8 + j] + ooc_step;  /* bias 10 8 */
                ooc_data[10 + i][8 + j] = (data > 62 ? 62 : data);    /* bias 10 8 value 62 62 */
            } else if (tmp_raw[10 + i][8 + j] < 7800) {  /* bias 10 bias 8 value 7800 */
                ooc_data[10 + i][8 + j] = ooc_data[10 + i][8 + j] - ooc_step;   /* bias 10 8 10 8 */
            } else {}
        }
    }
}

static td_void scene_thermo_ooc_correction(ot_vi_pipe vi_pipe)
{
    td_s32 ret;
    td_u32 ooc_step = 16;

    ot_vi_pipe phys_pipe = 0;
    ot_video_frame_info frame_info;
    sample_vi_user_frame_info *user_frame_info = &g_user_frame_info;

    ot_vi_frame_dump_attr dump_attr = {
        .depth = 2,
        .enable = TD_TRUE
    };
    ss_mpi_vi_set_pipe_frame_dump_attr(phys_pipe, &dump_attr);

    set_ooc_data(user_frame_info, g_ooc_data);
    scene_thermo_iris_control(0);
    sleep(1);
    while (ooc_step > 0) {
        ret = ss_mpi_vi_get_pipe_frame(phys_pipe, &frame_info, 1000);   /* 1000ms */
        if (ret != TD_SUCCESS) {
            continue;
        }
        get_raw_data(&frame_info.video_frame, 14, g_tmp_raw, THERMO_HEIGHT_308, THERMO_WIDTH_400);  /* 14 bit */
        update_ooc(g_ooc_data, g_tmp_raw, ooc_step);
        
        set_ooc_data(user_frame_info, g_ooc_data);

        ooc_step = ooc_step >> 1;
        ss_mpi_vi_release_pipe_frame(phys_pipe, &frame_info);
        sleep(1);
    }
}

td_u32 sample_thermo_temperature_change(ot_vi_pipe vi_pipe)
{
    td_u32 dvtemp_d1, dvtemp_d2, dvtemp_d3, dvtemp_d4, dvtemp_d5, dvtemp_d6;
    td_u32 d16_vtemp_d1, d16_vtemp_d2, dvtemp;
    td_u32 wr_data, delt_volt;
    td_s32 t17_vtemp_v, dvtemp0, diff;
    static td_s32 temperature = 0;

    thermo_gst412c_write_register(vi_pipe, 0x7c, 0xa8);
    thermo_gst412c_write_register(vi_pipe, 0x47, 0x01);
    dvtemp_d1 = thermo_gst412c_read_register(vi_pipe, 0x3d);
    dvtemp_d2 = thermo_gst412c_read_register(vi_pipe, 0x3e);
    d16_vtemp_d2 = ((dvtemp_d2 & 0xff) << 8) | (dvtemp_d1 & 0xff);  /* shift 8 */

    dvtemp_d3 = thermo_gst412c_read_register(vi_pipe, 0x3f);
    dvtemp_d4 = thermo_gst412c_read_register(vi_pipe, 0x40);
    d16_vtemp_d1 = ((dvtemp_d4 & 0xff) << 8) | (dvtemp_d3 & 0xff);  /* shift 8 */

    dvtemp_d5 = thermo_gst412c_read_register(vi_pipe, 0x41);
    dvtemp_d6 = thermo_gst412c_read_register(vi_pipe, 0x42);
    dvtemp = ((dvtemp_d6 & 0xff) << 8) | (dvtemp_d5 & 0xff);        /* shift 8 */

    thermo_gst412c_write_register(vi_pipe, 0x47, 0x00);
    thermo_gst412c_write_register(vi_pipe, 0x7c, 0xa9);
    wr_data = (thermo_gst412c_read_register(vi_pipe, 0x06) & 0x30) >> 4;  /* shift 4 */

    thermo_gst412c_write_register(vi_pipe, 0x7c, 0xa8);

    delt_volt =  250 + 100 * wr_data;   /* value 250 100 */
    t17_vtemp_v = (dvtemp - d16_vtemp_d1) * 1000 / div_0_to_1(d16_vtemp_d2 - d16_vtemp_d1)   /* value 1000 */
        + delt_volt;
    dvtemp0 = (t17_vtemp_v - 1546) * 10000 / 1200 + 3000;   /* value 1546 10000 1200 3000 */
    if (temperature == 0) {
        temperature = dvtemp0;
    } else if (temperature > dvtemp0) {
        diff = temperature - dvtemp0;
        if (diff > THERMO_TEMP_DIFF) {
            temperature = dvtemp0;
            return 1;
        }
    } else if (temperature < dvtemp0) {
        diff = dvtemp0 - temperature;
        if (diff > THERMO_TEMP_DIFF) {
            temperature = dvtemp0;
            return 1;
        }
    } else {}

    return 0;
}

static td_u32 scene_thermo_histogram_equalization(ot_isp_thermo_curve_attr *curve_attr)
{
    td_s32 ret, i;
    ot_vi_pipe phys_pipe = 0;
    ot_isp_thermo_info thermo_info;
    ot_isp_drc_attr drc_attr;

    ret = ss_mpi_isp_set_thermo_curve_attr(phys_pipe, curve_attr);
    usleep(40000); /* 40000 40 ms */
    ret = ss_mpi_thermo_query_exposure_info(phys_pipe, &thermo_info);
    ot_scenecomm_check_return(ret, ret);
    ret = ss_mpi_isp_get_drc_attr(phys_pipe, &drc_attr);
    ot_scenecomm_check_return(ret, ret);
    for (i = 0; i < OT_ISP_DRC_TM_NODE_NUM; i++) {
        drc_attr.tone_mapping_value[i] = thermo_info.tone_mapping_value[i];
    }

    ret = ss_mpi_isp_set_drc_attr(phys_pipe, &drc_attr);
    ot_scenecomm_check_return(ret, ret);
    
    return ret;
}

static td_u32 scene_thermo_init_param(ot_isp_thermo_curve_attr *curve_attr)
{
    td_s32 ret, i;
    ot_vi_pipe phys_pipe = 0;
    ot_aidestrip_attr aidattr;
    ot_3dnr_attr nrattr;
    ot_isp_thermo_attr thermo_attr;
    ot_isp_drc_attr drc_attr;

    ret = ss_mpi_aidestrip_get_attr(phys_pipe, &aidattr);
    ot_scenecomm_check_return(ret, ret);
    aidattr.enable = TD_TRUE;
    aidattr.param.strength = AIDESTRIP_STRENGTH;
    ret = ss_mpi_aidestrip_set_attr(phys_pipe, &aidattr);
    ot_scenecomm_check_return(ret, ret);

    ret = ss_mpi_vi_get_pipe_3dnr_attr(phys_pipe, &nrattr);
    ot_scenecomm_check_return(ret, ret);
    nrattr.enable = TD_TRUE;
    ret = ss_mpi_vi_set_pipe_3dnr_attr(phys_pipe, &nrattr);
    ot_scenecomm_check_return(ret, ret);
    ot_scene_3dnr _3dnr = get_pipe_params()[phys_pipe].static_threednr.threednr_value[0];
    ot_3dnr_pos_type pos3dnr = OT_3DNR_POS_VI;
    ret = scene_set_3dnr(phys_pipe, &_3dnr, 0, pos3dnr);
    ot_scenecomm_check_return(ret, ret);

    curve_attr->clip = get_pipe_params()[phys_pipe].static_thermo.clip;
    curve_attr->fusion_ratio = get_pipe_params()[phys_pipe].static_thermo.fusion_ratio;
    for (i = 0; i < OT_ISP_GAMMA_NODE_NUM; i++) {
        curve_attr->fusion_curve_value[i] = get_pipe_params()[phys_pipe].static_thermo.fusion_curve_value[i];
    }

    ret = ss_mpi_isp_get_thermo_attr(phys_pipe, &thermo_attr);
    thermo_attr.op_type = OT_OP_MODE_AUTO;
    ret = ss_mpi_isp_set_thermo_attr(phys_pipe, &thermo_attr);
    ot_scenecomm_check_return(ret, TD_NULL);

    ret = ss_mpi_isp_get_drc_attr(phys_pipe, &drc_attr);
    ot_scenecomm_check_return(ret, ret);
    drc_attr.detail_adjust_coef = DRC_COEF;
    ret = ss_mpi_isp_set_drc_attr(phys_pipe, &drc_attr);
    ot_scenecomm_check_return(ret, ret);

    return ret;
}

static td_void scene_thermo_fpn_calibrate(ot_vi_pipe vi_pipe)
{
    td_u32 ret;
    ss_mpi_vi_set_pipe_frame_source(vi_pipe, OT_VI_PIPE_FRAME_SOURCE_FE);
    ot_isp_black_level_attr black_level, saved_lack_level;
    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, &black_level);
    ret = ss_mpi_isp_get_black_level_attr(vi_pipe, &saved_lack_level);
    for (int i = 0; i < OT_ISP_WDR_MAX_FRAME_NUM; i++) {
        for (int j = 0; j < OT_ISP_WDR_MAX_FRAME_NUM; j++) {
            black_level.user_black_level[i][j] = 0;
            black_level.manual_attr.black_level[i][j] = USER_BLC;
        }
    }
    black_level.user_black_level_en = TD_TRUE;
    black_level.black_level_mode = OT_ISP_BLACK_LEVEL_MODE_MANUAL;
    ret = ss_mpi_isp_set_black_level_attr(vi_pipe, &black_level);
    sleep(1);
    sample_comm_vi_fpn_calibrate_for_thermo(vi_pipe, &g_calibration_cfg);
    sample_print("fpn calibrate done.\n");
    ret = ss_mpi_isp_set_black_level_attr(vi_pipe, &saved_lack_level);
    if (ret != TD_SUCCESS) {
        printf("ss_mpi_isp_set_black_level_attr failed!");
    }

    scene_thermo_iris_control(1);
    ss_mpi_vi_set_pipe_frame_source(vi_pipe, OT_VI_PIPE_FRAME_SOURCE_USER);
}

td_void *scene_thermo_ooc_fpn_correction_thread(td_void *arg)
{
    td_u8  bupdata = 1;
    ot_vi_pipe vi_pipe = 0;
    static time_t time_s = 0;
    time_t time_e = 0;
    td_u32 time_d = 0;
    td_u32 ret;
    ot_isp_thermo_curve_attr curve_attr;
    ret = scene_thermo_init_param(&curve_attr);
    ot_scenecomm_check_return(ret, TD_NULL);
    scene_thermo_iris_control(1);
    thermo_gst412c_i2c_init(vi_pipe);
    sample_comm_vi_disable_fpn_correction_for_thermo(vi_pipe, &g_correction_cfg);
    td_s32 *ptr = NULL;
    td_u32 shm_id = shmget((key_t)SHARE_MEM_ID, SHARE_MEM_SIZE, IPC_CREAT | SHARE_MEM_PARAM);
    ptr = (td_s32*)shmat(shm_id, 0, 0);
    while (g_scene_state.thread_normal.thread_flag == TD_TRUE) {
        if (bupdata == 1 || time_d > THERMO_TIME_DIFF) {
            ret = ss_mpi_vi_disable_chn(vi_pipe, 0);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_vi_disable_chn failed!");
            }
            scene_thermo_iris_control(0);
            *ptr = -1;
            sleep(1);
            if (bupdata == 1) {
                sample_print(" begin ooc correction.\n");
                scene_thermo_ooc_correction(vi_pipe);
                bupdata = 0;
            }

            scene_thermo_fpn_calibrate(vi_pipe);

            *ptr = 1;
            time_s = time(NULL);
            sleep(1);
            ret = ss_mpi_vi_enable_chn(vi_pipe, 0);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_vi_enable_chn failed!");
            }
        }
        ret = scene_thermo_histogram_equalization(&curve_attr);
        ot_scenecomm_check_return(ret, TD_NULL);
        
        time_e = time(NULL);
        time_d = (td_u32)difftime(time_e, time_s);
        bupdata = sample_thermo_temperature_change(vi_pipe);
    }
    *ptr = 0;
    shmdt(ptr);
    shmctl(SHARE_MEM_ID, IPC_RMID, TD_NULL);
    return TD_NULL;
}

td_void *scene_normal_auto_thread(td_void *arg)
{
    td_s32 ret;
    td_u32 i;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"OT_SCENE_NormalThread", 0, 0, 0);

    while (g_scene_state.thread_normal.thread_flag == TD_TRUE) {
        ret = scene_set_pipe_dynamic_param();
        ot_scenecomm_check(ret, OT_SCENE_EINTER);
        
        for (i = 0; i < g_scene_state.main_pipe_num; i++) {
            g_scene_state.main_pipe[i].last_normal_exposure = g_scene_state.main_pipe[i].exposure;
            g_scene_state.main_pipe[i].last_normal_iso = g_scene_state.main_pipe[i].iso;
        }

        usleep(200000); /* 200000 200 ms */
    }

    return TD_NULL;
}

static td_s32 scene_thermo_get_ooc_frame_blk(sample_vi_get_frame_vb_cfg *get_frame_vb_cfg,
    sample_vi_user_frame_info user_frame_info[], td_s32 frame_cnt)
{
    td_u32 y_stride, blk_size;
    td_u64 phys_addr;
    td_u8 *virt_addr;
    ot_vb_pool pool_id;
    ot_vb_pool_cfg vb_pool_cfg;

    /* calc OOC frame */
    y_stride  = sample_comm_vi_get_raw_stride(get_frame_vb_cfg->pixel_format,
        get_frame_vb_cfg->size.width, 1, OT_DEFAULT_ALIGN);
    blk_size = OT_ALIGN_UP((get_frame_vb_cfg->size.height * y_stride), OT_DEFAULT_ALIGN);

    memset_s(&vb_pool_cfg, sizeof(ot_vb_pool_cfg), 0, sizeof(ot_vb_pool_cfg));
    vb_pool_cfg.blk_size   = blk_size;
    vb_pool_cfg.blk_cnt    = frame_cnt;
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    pool_id = ss_mpi_vb_create_pool(&vb_pool_cfg);
    if (pool_id == OT_VB_INVALID_POOL_ID) {
        return TD_FAILURE;
    }

    for (int i = 0; i < frame_cnt; i++) {
        ot_vb_blk vb_blk = ss_mpi_vb_get_blk(pool_id, blk_size, TD_NULL);
        if (vb_blk == OT_VB_INVALID_HANDLE) {
            return TD_FAILURE;
        }

        phys_addr = ss_mpi_vb_handle_to_phys_addr(vb_blk);
        virt_addr = (td_u8 *)ss_mpi_sys_mmap(phys_addr, blk_size);
        if (virt_addr == TD_NULL) {
            return TD_FAILURE;
        }

        ot_video_frame_info *frame_info = &user_frame_info[i].frame_info;

        frame_info->pool_id = pool_id;
        frame_info->mod_id  = OT_ID_VI;

        frame_info->video_frame.phys_addr[0] = phys_addr;
        frame_info->video_frame.phys_addr[1] = frame_info->video_frame.phys_addr[0];
        frame_info->video_frame.virt_addr[0] = virt_addr;
        frame_info->video_frame.virt_addr[1] = frame_info->video_frame.virt_addr[0];
        frame_info->video_frame.stride[0]    = y_stride;
        frame_info->video_frame.stride[1]    = y_stride; /* same as y_stride */

        frame_info->video_frame.width         = get_frame_vb_cfg->size.width;
        frame_info->video_frame.height        = get_frame_vb_cfg->size.height;
        frame_info->video_frame.pixel_format  = get_frame_vb_cfg->pixel_format;
        frame_info->video_frame.video_format  = get_frame_vb_cfg->video_format;
        frame_info->video_frame.compress_mode = get_frame_vb_cfg->compress_mode;
        frame_info->video_frame.dynamic_range = get_frame_vb_cfg->dynamic_range;
        frame_info->video_frame.field         = OT_VIDEO_FIELD_FRAME;
        frame_info->video_frame.color_gamut   = OT_COLOR_GAMUT_BT601;

        user_frame_info[i].vb_blk   = vb_blk;
        user_frame_info[i].blk_size = blk_size;
    }

    return TD_SUCCESS;
}


#define SCENE_THREAD_STACK_SIZE 0x20000
static td_s32 scene_start_auto_thread(td_void)
{
    td_s32 ret = TD_SUCCESS;
    sample_vi_get_frame_vb_cfg get_frame_vb_cfg;
    get_frame_vb_cfg.size.width = THERMO_WIDTH_400;
    get_frame_vb_cfg.size.height = THERMO_HEIGHT_308;
    get_frame_vb_cfg.pixel_format = OT_PIXEL_FORMAT_RGB_BAYER_8BPP;
    get_frame_vb_cfg.video_format = OT_VIDEO_FORMAT_LINEAR;
    get_frame_vb_cfg.compress_mode = OT_COMPRESS_MODE_NONE;
    get_frame_vb_cfg.dynamic_range = OT_DYNAMIC_RANGE_SDR8;
    scene_thermo_get_ooc_frame_blk(&get_frame_vb_cfg, &g_user_frame_info, 1);

    pthread_attr_init(&(g_scene_state.thread_normal_attr));
    pthread_attr_setdetachstate(&(g_scene_state.thread_normal_attr), PTHREAD_CREATE_DETACHED);
#ifdef __LITEOS__
    pthread_attr_setstacksize(&(g_scene_state.thread_normal_attr), SCENE_THREAD_STACK_SIZE);
#endif

    if (g_scene_state.thread_normal.thread_flag == TD_FALSE) {
        g_scene_state.thread_normal.thread_flag = TD_TRUE;
        ret = pthread_create(&g_scene_state.thread_normal.thread, &(g_scene_state.thread_normal_attr),
            scene_thermo_ooc_fpn_correction_thread, NULL);

        ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    }

    return ret;
}

static td_s32 scene_stop_auto_thread(td_void)
{
    if (g_scene_state.thread_normal.thread_flag == TD_TRUE) {
        g_scene_state.thread_normal.thread_flag = TD_FALSE;
        pthread_attr_destroy(&(g_scene_state.thread_normal_attr));
    }

    return TD_SUCCESS;
}

/* -------------------------external function interface------------------------- */
td_s32 ot_scene_init(const ot_scene_param *scene_param)
{
    ot_scenecomm_check_pointer_return(scene_param, OT_SCENE_ENONPTR);

    td_s32 ret = TD_SUCCESS;

    if (g_scene_state.scene_init == TD_TRUE) {
        scene_loge("SCENE module has already been inited.\n");
        return OT_SCENE_EINITIALIZED;
    }

    ret = ot_scene_set_pipe_param(scene_param->pipe_param, OT_SCENE_PIPETYPE_NUM);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    (td_void)memset_s(&g_scene_mode, sizeof(ot_scene_mode), 0, sizeof(ot_scene_mode));

    (td_void)memset_s(&g_scene_state, sizeof(scene_state), 0, sizeof(scene_state));

    g_scene_state.scene_init = TD_TRUE;

    scene_logd("SCENE module has been inited successfully.\n");

    return ret;
}

static td_s32 scene_check_pipe_attr(const ot_scene_mode *scene_mode)
{
    td_u32 i;
    for (i = 0; i < OT_SCENE_PIPE_MAX_NUM; i++) {
        if (scene_mode->pipe_attr[i].enable != TD_TRUE) {
            continue;
        }

        if (scene_mode->pipe_attr[i].vcap_pipe_hdl != i) {
            scene_loge("The value of pipe in scene pipe array must be equal to the index of array!\n");
            ot_mutex_unlock(g_scene_lock);
            return OT_SCENE_EINVAL;
        }

        if ((scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_SNAP) &&
            (scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_VIDEO) &&
            (scene_mode->pipe_attr[i].pipe_type != OT_SCENE_PIPE_TYPE_MCF_3DNR)) {
            scene_loge("Pipe Type is not video or snap!\n");
            ot_mutex_unlock(g_scene_lock);
            return OT_SCENE_EOUTOFRANGE;
        }

        if (scene_mode->pipe_attr[i].pipe_param_index >= OT_SCENE_PIPETYPE_NUM) {
            scene_loge("Pipe param index is out of range!\n");
            ot_mutex_unlock(g_scene_lock);
            return OT_SCENE_EOUTOFRANGE;
        }
    }
    return TD_SUCCESS;
}

td_s32 ot_scene_set_scene_mode(const ot_scene_mode *scene_mode)
{
    scene_check_init_return();
    ot_scenecomm_check_pointer_return(scene_mode, OT_SCENE_ENONPTR);

    td_s32 ret;

    ot_mutex_lock(g_scene_lock);

    if (scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_LINEAR && scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_WDR &&
        scene_mode->pipe_mode != OT_SCENE_PIPE_MODE_HDR) {
        scene_loge("The pipe mode must be LINEAR, WDR or HDR.\n");
        ot_mutex_unlock(g_scene_lock);
        return OT_SCENE_EINVAL;
    }

    ret = scene_check_pipe_attr(scene_mode);
    ot_scenecomm_check_return(ret, TD_FAILURE);

    (td_void)memcpy_s(&g_scene_mode, sizeof(ot_scene_mode), scene_mode, sizeof(ot_scene_mode));

    ret = scene_set_main_pipe_state(scene_mode);
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_SetMainIspState failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return ret;
    }

    ret = scene_set_vi_pipe_param();
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_SetIspParam failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return ret;
    }

    ret = scene_start_auto_thread();
    if (ret != TD_SUCCESS) {
        scene_loge("SCENE_StartThread failed.\n");
        ot_mutex_unlock(g_scene_lock);
        return ret;
    }

    ot_mutex_unlock(g_scene_lock);

    return ret;
}

td_s32 ot_scene_deinit(td_void)
{
    scene_check_init_return();

    td_s32 ret;

    ret = scene_stop_auto_thread();
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);
    ot_mutex_lock(g_scene_lock);
    g_scene_state.scene_init = TD_FALSE;
    ot_mutex_unlock(g_scene_lock);

    g_init_iso = -1;
    g_init_exp = -1;
    scene_logd("SCENE Module has been deinited successfully!\n");

    return TD_SUCCESS;
}

td_s32 ot_scene_get_scene_mode(ot_scene_mode *scene_mode)
{
    scene_check_init_return();

    ot_scenecomm_check_pointer_return(scene_mode, OT_SCENE_ENONPTR);

    (td_void)memcpy_s(scene_mode, sizeof(ot_scene_mode), &g_scene_mode, sizeof(ot_scene_mode));

    return TD_SUCCESS;
}

td_s32 ot_scene_set_scene_init_exp(td_s32 iso, td_s64 exp)
{
    scene_check_init_return();
    if (g_scene_state.thread_venc.thread_flag != 0) {
        scene_loge("scene auto thread in running, not support setinitExp \n");
        return TD_FAILURE;
    }

    g_init_iso = iso;
    g_init_exp = exp;

    return TD_SUCCESS;
}

td_s32 ot_scene_get_scene_fps(ot_scene_fps *scene_fps)
{
    (td_void)memcpy_s(scene_fps, sizeof(ot_scene_fps), &g_scene_auto_fps, sizeof(ot_scene_fps));
    return TD_SUCCESS;
}

td_s32 ot_scene_pause(td_bool enable)
{
    scene_check_init_return();

    td_s32 ret;

    g_scene_state.pause = enable;

    ret = ot_scene_set_pause(enable);
    ot_scenecomm_check_return(ret, OT_SCENE_EINTER);

    return TD_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
