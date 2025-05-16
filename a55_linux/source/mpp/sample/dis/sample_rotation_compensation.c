/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include "sample_rotation_compensation.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ot_motionsensor_mng_cmd.h"
#include "ot_common_motionfusion.h"
#include "ss_mpi_motionfusion.h"
#include "sample_comm.h"
#include "sample_dis.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    pthread_t pthread_id;
    ot_msensor_buf_attr msensor_buf_attr;
} sample_dis_pthread_info;
sample_dis_pthread_info g_pthread_info = { 0 };
td_s32 g_msensor_userfd = -1;
td_s32 g_msensor_mng_dev_fd = -1;

const ot_mfusion_quaternion_data g_quaternion_data = {    // 90°
    (td_s32)(0.7071 * (1 << 20)),  // w
    0,                             // x
    0,                             // y
    (td_s32)(0.7071 * (1 << 20)), // z
    0                             // pts
};

static td_s32 sample_dis_pthread_get_msensordata(sample_dis_pthread_info *pthread_info,
    td_u64 begin_pts, td_u64 end_pts, ot_mfusion_quaternion_buf *quaternion_buf)
{
    td_u32 i, j;
    td_s32 ret;
    td_u32 num;
    td_u64 *pts_viraddr = TD_NULL;
    ot_msensor_data_info msensor_data_info = {0};
    msensor_data_info.id = g_msensor_userfd;
    msensor_data_info.data_type = MSENSOR_DATA_GYRO;
    msensor_data_info.begin_pts = begin_pts;
    msensor_data_info.end_pts = end_pts;

    ret = ioctl(g_msensor_mng_dev_fd, MSENSOR_CMD_GET_DATA, &msensor_data_info);
    if (ret != TD_SUCCESS) {
        sample_print("MSENSOR_CMD_GET_DATA failed\n");
        return TD_FAILURE;
    }

    quaternion_buf->num = 0;
    for (i = 0; i < 2; i++) { /* cyclic buffer 2 */
        num = msensor_data_info.data[i].num;
        if (num == 0) {
            continue;
        }
        pts_viraddr = (td_u64 *)(td_uintptr_t)(pthread_info->msensor_buf_attr.virt_addr +
            ((td_u64)(td_uintptr_t)msensor_data_info.data[i].pts_phys_addr - pthread_info->msensor_buf_attr.phys_addr));

        for (j = 0; j < num; j++) {
            quaternion_buf->data[quaternion_buf->num + j].pts = pts_viraddr[j];
            quaternion_buf->data[quaternion_buf->num + j].w = g_quaternion_data.w;
            quaternion_buf->data[quaternion_buf->num + j].x = g_quaternion_data.x;
            quaternion_buf->data[quaternion_buf->num + j].y = g_quaternion_data.y;
            quaternion_buf->data[quaternion_buf->num + j].z = g_quaternion_data.z;
        }

        quaternion_buf->num += num;
    }

    return ret;
}

td_bool g_send_quaternion_en = TD_TRUE;
td_void *sample_dis_pthread_send_quaternion(void *arg)
{
    td_s32 ret;
    td_u64 begin_pts = 0;
    td_u64 end_pts = 0;
    td_u32 fusion_id = 0;

    ot_mfusion_quaternion_buf quaternion_buf;
    sample_dis_pthread_info *pthread_info = (sample_dis_pthread_info *)arg;

    while (g_send_quaternion_en == TD_TRUE) {
        ss_mpi_sys_get_cur_pts(&end_pts);

        usleep(2000); /* sleep 2000 us */
        if (begin_pts != 0) {
            sample_dis_pthread_get_msensordata(pthread_info, begin_pts, end_pts, &quaternion_buf);
            ret = ss_mpi_mfusion_send_quaternion(fusion_id, &quaternion_buf);
            if (ret != TD_SUCCESS) {
                sample_print("ss_mpi_motionfusion_send_quaternion failed!\n");
                return TD_NULL;
            }
            begin_pts = end_pts + 1;
        } else {
            begin_pts = end_pts - 10000; /* get for 10000 us */
        }
    }

    return TD_NULL;
}

td_s32 sample_motionfuson_start_rotation_compensation(ot_msensor_param *msensor_param)
{
    const td_u32 fusion_id = 0;
    td_s32 ret;
    ot_mfusion_rotation_cfg cfg;
    ot_mfusion_rotation_compensation compensation;

    sample_dis_pause("Init rotation compensation");

    cfg.buf_size = 102400; /* buffer size is 102400 */
    ret = ss_mpi_mfusion_init_rotation_compensation(fusion_id, &cfg);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mfusion_init_rotation_compensation failed!\n");
        return TD_FAILURE;
    }

    g_msensor_mng_dev_fd = open("/dev/motionsensor_mng", O_RDWR);
    if (g_msensor_mng_dev_fd < 0) {
        sample_print("open motionsensor_mng failed\n");
        ret = TD_FAILURE;
        goto deinit;
    }

    ret = ioctl(g_msensor_mng_dev_fd, MSENSOR_CMD_ADD_USER, &g_msensor_userfd);
    if (ret != TD_SUCCESS) {
        sample_print("MSENSOR_CMD_ADD_USER failed!\n");
        ret = TD_FAILURE;
        goto close_fd;
    }

    memcpy_s(&g_pthread_info.msensor_buf_attr, sizeof(ot_msensor_buf_attr), msensor_param, sizeof(ot_msensor_buf_attr));

    if (pthread_create(&g_pthread_info.pthread_id, NULL, sample_dis_pthread_send_quaternion, &g_pthread_info) != 0) {
        ret = TD_FAILURE;
        goto delete_user;
    }
    g_send_quaternion_en = TD_TRUE;

    sample_dis_pause("Enable rotation compensation!");

    ret = ss_mpi_mfusion_get_rotation_compensation(fusion_id, &compensation);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mfusion_get_rotation_compensation failed!\n");
        goto pthread_exit;
    }

    compensation.enable = TD_TRUE;
    compensation.crop_ratio = 80; /* crop ratio as 80 */
    ret = ss_mpi_mfusion_set_rotation_compensation(fusion_id, &compensation);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_mfusion_set_rotation_compensation failed!\n");
        goto pthread_exit;
    }

    return TD_SUCCESS;

pthread_exit:
    g_send_quaternion_en = TD_FALSE;
    pthread_join(g_pthread_info.pthread_id, NULL);
delete_user:
    ioctl(g_msensor_mng_dev_fd, MSENSOR_CMD_DELETE_USER, &g_msensor_userfd);
    g_msensor_userfd = -1;
close_fd:
    close(g_msensor_mng_dev_fd);
    g_msensor_mng_dev_fd = -1;
deinit:
    ss_mpi_mfusion_deinit_rotation_compensation(fusion_id);

    return ret;
}

td_s32 sample_motionfuson_stop_rotation_compensation(td_void)
{
    td_u32 fusion_id = 0;
    td_s32 ret;
    ot_mfusion_rotation_compensation compensation;

    compensation.enable = TD_FALSE;
    compensation.crop_ratio = 100; /* crop ratio as 100 is full */
    ret = ss_mpi_mfusion_set_rotation_compensation(fusion_id, &compensation);
    if (ret != TD_SUCCESS) {
        sample_print("ss_mpi_motionfusion_set_rotation_compensation failed!\n");
        ss_mpi_mfusion_deinit_rotation_compensation(fusion_id);
        return TD_FAILURE;
    }

    if (g_send_quaternion_en == TD_TRUE) {
        g_send_quaternion_en = TD_FALSE;
        pthread_join(g_pthread_info.pthread_id, NULL);
    }

    if (g_msensor_userfd > 0) {
        ioctl(g_msensor_mng_dev_fd, MSENSOR_CMD_DELETE_USER, &g_msensor_userfd);
        g_msensor_userfd = -1;
    }

    if (g_msensor_mng_dev_fd > 0) {
        close(g_msensor_mng_dev_fd);
        g_msensor_mng_dev_fd = -1;
    }

    ss_mpi_sys_munmap((td_void *)(td_uintptr_t)g_pthread_info.msensor_buf_attr.virt_addr,
        g_pthread_info.msensor_buf_attr.buf_len);

    ret = ss_mpi_mfusion_deinit_rotation_compensation(fusion_id);

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of __cplusplus */

