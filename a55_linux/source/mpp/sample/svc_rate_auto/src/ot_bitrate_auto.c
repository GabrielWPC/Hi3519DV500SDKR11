/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>
#include "sample_comm.h"
#include "ot_bitrate_auto.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define QPMAP_MAX_POSI_VALUE 31
#define QPMAP_MIN_NEGA_VALUE 32
#define QPMAP_MAX_VALUE 64
#define MIN_I_QP     26
#define MIN_P_QP     28
#define QPMAP_OFFSET_64 6
#define QPMAP_OFFSET_128 7
#define MAX_STILL_PERCENT 100
#define MIN_STILL_PERCENT 5
#define QUALITY_MODE_PERCENT 110
#define RATE_MODE_PERCENT 70

typedef struct {
    td_bool start_flag;
    pthread_t thread;
    pthread_attr_t attr;
} rate_auto_thread_attr;

typedef enum {
    RATE_AUTO_STATE_STOPED = 0x0,
    RATE_AUTO_STATE_STARTED = 0x1,
    RATE_AUTO_STATE_BUTT
} rate_auto_state;

static rate_auto_thread_attr g_rate_auto_thread;
static rate_auto_state g_rate_auto_running_state = RATE_AUTO_STATE_STOPED;
pthread_mutex_t g_rate_auto_lock = PTHREAD_MUTEX_INITIALIZER;
static rate_auto_param g_rate_auto_init_param;

static td_s32 clip_3(td_s32 min, td_s32 max, td_s32 val)
{
    val = (val > max) ? max : val;
    val = (val < min) ? min : val;
    return val;
}

static td_void qpmap_to_deltaqp(td_u8 qpmap_value, td_u8 *abs_flag, td_s8 *qp)
{
    *abs_flag = (qpmap_value >> QPMAP_OFFSET_64) & 1; // 6: 6 bits Convert QpMap value

    if ((*abs_flag) == 1) {
        *qp = qpmap_value & 63;  // 63: 6 binary number to Decimal number
        *qp = clip_3(0, 51, (td_s32)(*qp)); // 51: MaxQp Convert QpMap value
    } else {
        *qp = (((qpmap_value >> (QPMAP_OFFSET_64 - 1)) & 1) << QPMAP_OFFSET_64)
            + (((qpmap_value >> (QPMAP_OFFSET_64 - 1)) & 1)<< QPMAP_OFFSET_128)
            + (qpmap_value & 63); // 63: 6 binary number to Decimal number
    }
}

static td_void deltaqp_to_qpmap(td_u8 *qpmap_value, td_u8 abs_flag, td_s8 qp)
{
    td_s32 tmp_qp;
    if (abs_flag == 1) {
        *qpmap_value = qp + QPMAP_MAX_VALUE;
    } else {
        if (qp < -32) {                     // -32:Restrict QpMap value
            tmp_qp = -32;                   // -32:Restrict QpMap value
        } else {
            tmp_qp = qp;
        }

        if (tmp_qp > QPMAP_MAX_POSI_VALUE) {
            tmp_qp = QPMAP_MAX_POSI_VALUE;
        }

        if (tmp_qp >= 0) {
            *qpmap_value = tmp_qp;
        } else {
            *qpmap_value = QPMAP_MAX_VALUE + tmp_qp;
        }
    }
}

static td_s32 add_qp(td_u8 *qpmap_value, td_s32 add_value)
{
    td_u8 abs_flag;
    td_s8 tmp_qp;
    td_u8 org_qpmap_value = *qpmap_value;

    qpmap_to_deltaqp(org_qpmap_value, &abs_flag, &tmp_qp);
    tmp_qp += add_value;

    deltaqp_to_qpmap(qpmap_value, abs_flag, tmp_qp);
    return TD_SUCCESS;
}

static td_void adjust_avbr_bitrate(ot_venc_chn_attr *chn_attr, ot_venc_chn chn)
{
    ot_venc_rc_param rc_param;
    static td_u32 org_min_still_percent = 0;
    td_s32 tmp_min_still_percent = 0;
    td_u32 index;
    td_u32 min_i_qp, min_qp;
    static td_s32 start_flag = 0;
    if ((chn_attr->rc_attr.rc_mode != OT_VENC_RC_MODE_H264_AVBR) &&
        (chn_attr->rc_attr.rc_mode != OT_VENC_RC_MODE_H265_AVBR)) {
        return;
    }
    if (g_rate_auto_init_param.avbr_rate_control != 0) {
        ss_mpi_venc_get_rc_param(chn, &rc_param);
        if (chn_attr->rc_attr.rc_mode == OT_VENC_RC_MODE_H264_AVBR) {
            min_i_qp = rc_param.h264_avbr_param.min_i_qp;
            min_qp  = rc_param.h264_avbr_param.min_qp;
        } else {
            min_i_qp = rc_param.h265_avbr_param.min_i_qp;
            min_qp  = rc_param.h265_avbr_param.min_qp;
        }
        if (start_flag == 0) {
            if (chn_attr->rc_attr.rc_mode == OT_VENC_RC_MODE_H264_AVBR) {
                org_min_still_percent = rc_param.h264_avbr_param.min_still_percent;
            } else {
                org_min_still_percent = rc_param.h265_avbr_param.min_still_percent;
            }
            start_flag = 1;
        }
        if (g_rate_auto_init_param.avbr_rate_control == 2) {  // 2:Quality mode
            index = 18;      // 18: Default Index1 value
            tmp_min_still_percent = (org_min_still_percent * (index + QUALITY_MODE_PERCENT)) >> QPMAP_OFFSET_128;
        } else if (g_rate_auto_init_param.avbr_rate_control == 1) {                // 1:Rate mode
            index = 40;       // 40: Restrict Index value
            tmp_min_still_percent = (org_min_still_percent * (index + RATE_MODE_PERCENT)) >> QPMAP_OFFSET_128;
            min_i_qp = (min_i_qp > MIN_I_QP) ? min_i_qp : MIN_I_QP;
            min_qp = (min_qp > MIN_P_QP) ? min_qp : MIN_P_QP;
        }
        tmp_min_still_percent = clip_3(MIN_STILL_PERCENT, MAX_STILL_PERCENT, tmp_min_still_percent);
        if (chn_attr->rc_attr.rc_mode == OT_VENC_RC_MODE_H264_AVBR) {
            rc_param.h264_avbr_param.min_still_percent = tmp_min_still_percent;
            rc_param.h264_avbr_param.min_i_qp = min_i_qp;
            rc_param.h264_avbr_param.min_qp = min_qp;
        } else {
            rc_param.h265_avbr_param.min_still_percent = tmp_min_still_percent;
            rc_param.h265_avbr_param.min_i_qp = min_i_qp;
            rc_param.h265_avbr_param.min_qp = min_qp;
        }
        ss_mpi_venc_set_rc_param(chn, &rc_param);
    }
}

static td_void init_svc_param(ot_venc_svc_param *svc_param)
{
    td_s32 i;
    svc_param->fg_protect_adaptive_en = g_rate_auto_init_param.fg_protect_adjust;
    for (i = 0; i < SVC_RECT_TYPE_NUM; i++) {
        svc_param->fg_region[i].qpmap_value_i = g_rate_auto_init_param.svc_fg_qpmap_val_i[i];
        svc_param->fg_region[i].qpmap_value_p = g_rate_auto_init_param.svc_fg_qpmap_val_p[i];
        svc_param->fg_region[i].skipmap_value = 0;
    }
    svc_param->activity_region.qpmap_value_i = g_rate_auto_init_param.svc_roi_qpmap_val_i;
    svc_param->activity_region.qpmap_value_p = g_rate_auto_init_param.svc_roi_qpmap_val_p;
    svc_param->activity_region.skipmap_value = 0;
    svc_param->bg_region.qpmap_value_i = g_rate_auto_init_param.svc_bg_qpmap_val_i;
    svc_param->bg_region.qpmap_value_p = g_rate_auto_init_param.svc_bg_qpmap_val_p;
    svc_param->bg_region.skipmap_value = 0;
}
static td_void adjust_qpmap_based_on_max_qp(td_s32 max_qpmap_value, ot_venc_svc_param *svc_param)
{
    td_s32 i;

    for (i = 0; i < SVC_RECT_TYPE_NUM; i++) {
        if (svc_param->fg_region[i].qpmap_value_i > max_qpmap_value &&
            svc_param->fg_region[i].qpmap_value_i <= QPMAP_MAX_POSI_VALUE) {
            svc_param->fg_region[i].qpmap_value_i = max_qpmap_value;
        }
        if (svc_param->fg_region[i].qpmap_value_p > max_qpmap_value &&
            svc_param->fg_region[i].qpmap_value_p <= QPMAP_MAX_POSI_VALUE) {
            svc_param->fg_region[i].qpmap_value_p = max_qpmap_value;
        }
    }

    return;
}

static td_void adjust_qpmap_based_on_max_bgqp(td_u32 max_bg_qp, td_u32 start_qp, ot_venc_svc_param *svc_param)
{
    td_s32 max_qpmap_value = max_bg_qp - start_qp;
    if (max_qpmap_value < 0) {
        max_qpmap_value = 0;
    }

    adjust_qpmap_based_on_max_qp(max_qpmap_value, svc_param);

    if (svc_param->activity_region.qpmap_value_i > max_qpmap_value &&
        svc_param->activity_region.qpmap_value_i <= QPMAP_MAX_POSI_VALUE) {
        svc_param->activity_region.qpmap_value_i = max_qpmap_value;
    }
    if (svc_param->activity_region.qpmap_value_p > max_qpmap_value &&
        svc_param->activity_region.qpmap_value_p <= QPMAP_MAX_POSI_VALUE) {
        svc_param->activity_region.qpmap_value_p = max_qpmap_value;
    }
    if (svc_param->bg_region.qpmap_value_i > max_qpmap_value &&
        svc_param->bg_region.qpmap_value_i <= QPMAP_MAX_POSI_VALUE) {
        svc_param->bg_region.qpmap_value_i = max_qpmap_value;
    }
    if (svc_param->bg_region.qpmap_value_p > max_qpmap_value &&
        svc_param->bg_region.qpmap_value_p <= QPMAP_MAX_POSI_VALUE) {
        svc_param->bg_region.qpmap_value_p = max_qpmap_value;
    }
}

static td_void adjust_qpmap_based_on_min_fgqp(td_u32 min_fg_qp, td_u32 start_qp, ot_venc_svc_param *svc_param)
{
    td_s32 min_qpmap_value;
    td_s8 tmp_curr_qp;
    td_u8 tmp_curr_abs;
    td_s32 i;

    min_qpmap_value = min_fg_qp - start_qp;
    if (min_qpmap_value > 0) {
        min_qpmap_value = 0;
    }
    for (i = 0; i < SVC_RECT_TYPE_NUM; i++) {
        qpmap_to_deltaqp(svc_param->fg_region[i].qpmap_value_i, &tmp_curr_abs, &tmp_curr_qp);
        if (tmp_curr_qp < min_qpmap_value) {
            tmp_curr_qp = min_qpmap_value;
            deltaqp_to_qpmap(&(svc_param->fg_region[i].qpmap_value_i), tmp_curr_abs, tmp_curr_qp);
        }
        qpmap_to_deltaqp(svc_param->fg_region[i].qpmap_value_p, &tmp_curr_abs, &tmp_curr_qp);
        if (tmp_curr_qp < min_qpmap_value) {
            tmp_curr_qp = min_qpmap_value;
            deltaqp_to_qpmap(&(svc_param->fg_region[i].qpmap_value_p), tmp_curr_abs, tmp_curr_qp);
        }
    }
}

static td_void adjust_fg_quality_based_on_max_bgqp(td_u32 max_bg_qp, td_u32 start_qp, ot_venc_svc_param *svc_param)
{
    td_s32 max_qpmap_value;            // Default value
    td_s32 i;

    max_qpmap_value = start_qp - max_bg_qp;
    if (max_qpmap_value > 0) {
        for (i = 0; i < SVC_RECT_TYPE_NUM; i++) {
            if (svc_param->fg_region[i].qpmap_value_i >= QPMAP_MIN_NEGA_VALUE &&
                svc_param->fg_region[i].qpmap_value_i < QPMAP_MAX_VALUE) {
                add_qp(&(svc_param->fg_region[i].qpmap_value_i), max_qpmap_value);
                svc_param->fg_region[i].qpmap_value_i =
                    (svc_param->fg_region[i].qpmap_value_i <= QPMAP_MAX_POSI_VALUE) ?
                    0 : svc_param->fg_region[i].qpmap_value_i;
            }
            if (svc_param->fg_region[i].qpmap_value_p >= QPMAP_MIN_NEGA_VALUE &&
                svc_param->fg_region[i].qpmap_value_p < QPMAP_MAX_VALUE) {
                add_qp(&(svc_param->fg_region[i].qpmap_value_p), max_qpmap_value);
                svc_param->fg_region[i].qpmap_value_p =
                    (svc_param->fg_region[i].qpmap_value_p <= QPMAP_MAX_POSI_VALUE) ?
                    0 : svc_param->fg_region[i].qpmap_value_p;
            }
        }

        if (svc_param->activity_region.qpmap_value_i >= QPMAP_MIN_NEGA_VALUE &&
            svc_param->activity_region.qpmap_value_i < QPMAP_MAX_VALUE) {
            add_qp(&(svc_param->activity_region.qpmap_value_i), max_qpmap_value);
            svc_param->activity_region.qpmap_value_i =
                (svc_param->activity_region.qpmap_value_i <= QPMAP_MAX_POSI_VALUE) ?
                0 : svc_param->activity_region.qpmap_value_i;
        }
        if (svc_param->activity_region.qpmap_value_p >= QPMAP_MIN_NEGA_VALUE &&
            svc_param->activity_region.qpmap_value_p < QPMAP_MAX_VALUE) {
            add_qp(&(svc_param->activity_region.qpmap_value_p), max_qpmap_value);
            svc_param->activity_region.qpmap_value_p =
                (svc_param->activity_region.qpmap_value_p <= QPMAP_MAX_POSI_VALUE) ?
                0 : svc_param->activity_region.qpmap_value_p;
        }
    }
}

static td_void adjust_bg_quality_set_value(td_s32 min_qpmap_value, td_u8 *qpmap_value)
{
    td_u8 value = *qpmap_value;
    if (min_qpmap_value < 0 && value <= QPMAP_MAX_POSI_VALUE) {
        value += min_qpmap_value;
        value = ((value >= QPMAP_MIN_NEGA_VALUE) ? 0 : value);
    }

    *qpmap_value = value;

    return;
}

static td_void adjust_bg_quality_based_on_min_fgqp(td_u32 min_fg_qp, td_u32 start_qp, ot_venc_svc_param *svc_param)
{
    td_s32 min_qpmap_value;

    min_qpmap_value = start_qp - min_fg_qp;

    adjust_bg_quality_set_value(min_qpmap_value, &svc_param->bg_region.qpmap_value_p);
    adjust_bg_quality_set_value(min_qpmap_value, &svc_param->bg_region.qpmap_value_i);
    adjust_bg_quality_set_value(min_qpmap_value, &svc_param->activity_region.qpmap_value_p);
    adjust_bg_quality_set_value(min_qpmap_value, &svc_param->activity_region.qpmap_value_i);

    return;
}

static td_void *rate_auto_proc(td_void *p)
{
    ot_venc_chn_status status;
    ot_venc_svc_param svc_param;
    ot_venc_chn_attr chn_attr;
    rate_auto_thread_attr *para = TD_NULL;
    const ot_venc_chn chn = 0;

    td_u32 start_qp;
    td_s32 max_qpmap_value;

    td_u32 max_bg_qp = g_rate_auto_init_param.max_bg_qp;
    td_u32 min_fg_qp = g_rate_auto_init_param.min_fg_qp;
    td_u32 max_fg_qp = g_rate_auto_init_param.max_fg_qp;

    para = (rate_auto_thread_attr *)p;
    ss_mpi_venc_get_chn_attr(chn, &chn_attr);
    ss_mpi_venc_enable_svc(chn, TD_TRUE);

    while (para->start_flag == TD_TRUE) {
        ss_mpi_venc_query_status(chn, &status);
        start_qp = status.stream_info.start_qp;
        ss_mpi_venc_get_svc_param(chn, &svc_param);
        init_svc_param(&svc_param);

        // Adjust QpMap values so that QpMapValue + start_qp <= max_bg_qp
        adjust_qpmap_based_on_max_bgqp(max_bg_qp, start_qp, &svc_param);
        max_qpmap_value = max_fg_qp - start_qp;
        max_qpmap_value = (max_qpmap_value < 0) ? 0 : max_qpmap_value;
        adjust_qpmap_based_on_max_qp(max_qpmap_value, &svc_param);
        // Adjust FgQpMapvalues so that start_qp - FgQpMapValue >= min_fg_qp
        adjust_qpmap_based_on_min_fgqp(min_fg_qp, start_qp, &svc_param);
        // Start to increase foreground QP when background QP exceeds max_bg_qp
        adjust_fg_quality_based_on_max_bgqp(max_bg_qp, start_qp, &svc_param);
        // Start to decrease background QP when foreground QP exceeds min_fg_qp
        adjust_bg_quality_based_on_min_fgqp(min_fg_qp, start_qp, &svc_param);

        ss_mpi_venc_set_svc_param(chn, &svc_param);
        // AVBR rate adjustment algorithm
        adjust_avbr_bitrate(&chn_attr, chn);

        usleep(1000); // 1000 : 1000 ms
    }

    return NULL;
}


static td_s32 rate_auto_start_thread(td_void)
{
    td_s32 ret;

    g_rate_auto_thread.start_flag = TD_TRUE;
    pthread_attr_init(&g_rate_auto_thread.attr);
    pthread_attr_setdetachstate(&g_rate_auto_thread.attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&g_rate_auto_thread.attr, 0x10000);
    ret = pthread_create(&g_rate_auto_thread.thread, &g_rate_auto_thread.attr, rate_auto_proc,
        (td_void *)&g_rate_auto_thread);
    if (ret != TD_SUCCESS) {
        rate_auto_prt("rate_auto_start_thread failed,ret: 0x%x!\n", ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 rate_auto_stop_thread(td_void)
{
    if (g_rate_auto_thread.start_flag == TD_TRUE) {
        g_rate_auto_thread.start_flag = TD_FALSE;
        pthread_join(g_rate_auto_thread.thread, 0);
        pthread_attr_destroy(&(g_rate_auto_thread.attr));
    }

    return TD_SUCCESS;
}

td_s32 ot_rate_auto_init(const rate_auto_param *init_param)
{
    td_s32 ret;

    if (init_param == NULL) {
        rate_auto_prt("init_param can not be null!\n");
        return TD_FAILURE;
    }

    pthread_mutex_lock(&g_rate_auto_lock);
    if (g_rate_auto_running_state == RATE_AUTO_STATE_STARTED) {
        rate_auto_prt("rate auto already inited!\n");
        pthread_mutex_unlock(&g_rate_auto_lock);
        return TD_FAILURE;
    }

    (td_void)memcpy_s(&g_rate_auto_init_param, sizeof(g_rate_auto_init_param), init_param, sizeof(rate_auto_param));

    g_rate_auto_thread.start_flag = TD_FALSE;

    ret = rate_auto_start_thread();
    if (ret != TD_SUCCESS) {
        rate_auto_prt("rate_auto_start_thread failed,ret: 0x%x!\n", ret);
        pthread_mutex_unlock(&g_rate_auto_lock);
        return TD_FAILURE;
    }

    g_rate_auto_running_state = RATE_AUTO_STATE_STARTED;
    pthread_mutex_unlock(&g_rate_auto_lock);

    return TD_SUCCESS;
}

td_s32 ot_rate_auto_deinit(td_void)
{
    td_s32 ret;

    pthread_mutex_lock(&g_rate_auto_lock);
    if (g_rate_auto_running_state == RATE_AUTO_STATE_STOPED) {
        rate_auto_prt("rate auto already deinited!\n");
        pthread_mutex_unlock(&g_rate_auto_lock);
        return TD_FAILURE;
    }

    ret = rate_auto_stop_thread();
    if (ret != TD_SUCCESS) {
        rate_auto_prt("rate_auto_stop_thread failed,ret: 0x%x!\n", ret);
        pthread_mutex_unlock(&g_rate_auto_lock);
        return TD_FAILURE;
    }

    (td_void)memset_s(&g_rate_auto_init_param, sizeof(rate_auto_param), 0, sizeof(rate_auto_param));

    g_rate_auto_running_state = RATE_AUTO_STATE_STOPED;
    pthread_mutex_unlock(&g_rate_auto_lock);

    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
