/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <linux/usb/ch9.h>
#include "securec.h"

#include "ot_camera.h"
#include "uvc.h"
#include "ot_stream.h"

#undef OT_DEBUG

#ifdef OT_DEBUG
#define debug(fmt, args...) printf(fmt, ##args)
#else
#define debug(fmt, args...)
#endif

#define ERR_INFO printf
#define RIGHT_SHIFT_8BIT 8
#define UDC_PATH_LENGTH 389
#define GET_MAX_PACKET_SIZE_PATH 398

static struct ot_uvc_dev *g_ot_cd = 0;

static const td_char *image_fmt_to_string(td_u32 format)
{
    switch (format) {
        case VIDEO_IMG_FORMAT_H264:
            return "H264";
        case VIDEO_IMG_FORMAT_H265:
            return "H265";
        case VIDEO_IMG_FORMAT_MJPEG:
            return "MJPEG";
        case VIDEO_IMG_FORMAT_YUYV:
            return "YUYV";
        case VIDEO_IMG_FORMAT_YUV420:
            return "YUV420";
        case VIDEO_IMG_FORMAT_NV21:
            return "NV21";
        case VIDEO_IMG_FORMAT_NV12:
            return "NV12";
        default:
            return "unknown format";
    }
}

#ifdef OT_DEBUG
static const td_char *get_code(td_s32 code)
{
    if (code == 0x01) { // 0x01 == SET_CUR
        return "SET_CUR";
    } else if (code == 0x81) { // 0x81 == GET_CUR
        return "GET_CUR";
    } else if (code == 0x82) { // 0x82 == GET_MIN
        return "GET_MIN";
    } else if (code == 0x83) { // 0x83 == GET_MAX
        return "GET_MAX";
    } else if (code == 0x84) { // 0x84 == GET_RES
        return "GET_RES";
    } else if (code == 0x85) { // 0x85 == GET_LEN
        return "GET_LEN";
    } else if (code == 0x86) { // 0x86 == GET_INFO
        return "GET_INFO";
    } else if (code == 0x87) { // 0x87 == GET_DEF
        return "GET_DEF";
    } else {
        return "UNKNOWN";
    }
}

static const td_char *get_intf_cs_string(td_s32 code)
{
    if (code == 0x01) {
        return "PROB_CONTROL";
    } else if (code == 0x02) {
        return "COMMIT_CONTROL";
    } else {
        return "UNKNOWN";
    }
}
#endif

static struct ot_uvc_dev *ot_uvc_init(const td_char *dev_name)
{
    struct ot_uvc_dev *uvc_dev = NULL;
    struct video_ability cap;
    td_s32 ret;
    td_s32 fd;

    td_char *regular_path = realpath(dev_name, NULL);
    if (regular_path == NULL) {
        ERR_INFO("unable to get real path: %s\n", dev_name);
        return NULL;
    }
    fd = open(regular_path, O_RDWR | O_NONBLOCK);
    free(regular_path);
    if (fd == -1) {
        ERR_INFO("v4l2 open failed(%s): %d\n", dev_name, errno);
        return NULL;
    }

    ret = ioctl(fd, VIDEO_IOCTL_QUERY_CAP, &cap);
    if (ret < 0) {
        ERR_INFO("unable to query device: %d\n", errno);
        close(fd);
        return NULL;
    }

    debug("open succeeded(%s:caps=0x%04x)\n", dev_name, cap.dw_caps);

    if (!(cap.dw_caps & 0x02)) {
        close(fd);
        return NULL;
    }

    debug("device is %s on bus %s\n", cap.a_card, cap.a_bus_info);

    uvc_dev = (struct ot_uvc_dev *)malloc(sizeof *uvc_dev);
    if (uvc_dev == NULL) {
        close(fd);
        return NULL;
    }

    (td_void)memset_s(uvc_dev, sizeof(*uvc_dev), 0, sizeof(*uvc_dev));
    uvc_dev->i_fd = fd;

    return uvc_dev;
}

static td_s32 ot_uvc_video_process_user_ptr(struct ot_uvc_dev *uvc_dev)
{
    struct video_cache v_cache;
    td_s32 ret;

    (td_void)memset_s(&v_cache, sizeof(v_cache), 0, sizeof(v_cache));
    v_cache.dw_type = VIDEO_CACHE_TYPE_OPT;
    v_cache.dw_mem = VIDEO_MM_USER;

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DQUEUE_BUF, &v_cache);
    if (ret < 0) {
        return ret;
    }

    ret = ot_stream_get_venc_send_uvc();
    if (ret < 0) {
        ERR_INFO("ot_stream_get_venc_send_uvc: %d\n", errno);
        return ret;
    }

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_QUEUE_BUF, &v_cache);
    if (ret < 0) {
        ERR_INFO("Unable to requeue buffer(1): %d\n", errno);
        return ret;
    }
    usleep(4000); /* 4000 sleep 4ms give up CPU */

    return 0;
}

static td_void ot_uvc_video_stream_user_ptr(struct ot_uvc_dev *uvc_dev)
{
    struct video_cache v_cache;
    td_s32 cache_type = VIDEO_CACHE_TYPE_OPT;
    td_s32 ret;

    debug("%s:Starting video stream.\n", __func__);

    (td_void)memset_s(&v_cache, sizeof(v_cache), 0, sizeof(v_cache));

    v_cache.dw_index = 0;
    v_cache.dw_type = VIDEO_CACHE_TYPE_OPT;
    v_cache.dw_mem = VIDEO_MM_USER;

    usleep(100 * 1000); // need sleep 100 * 1000 us for usb when stream on.
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_QUEUE_BUF, &v_cache);
    if (ret < 0) {
        ERR_INFO("Unable to queue buffer: %d\n", errno);
        return;
    }

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_STREAM_ON, &cache_type);
    if (ret < 0) {
        ERR_INFO("Unable to stream on: %d\n", errno);
        return;
    }

    uvc_dev->i_streaming = 1;
    return;
}

static td_s32 ot_uvc_video_set_fmt(struct ot_uvc_dev *uvc_dev)
{
    struct video_fmt v_fmt;
    td_s32 ret;

    (td_void)memset_s(&v_fmt, sizeof(v_fmt), 0, sizeof(v_fmt));
    v_fmt.dw_type = VIDEO_CACHE_TYPE_OPT;
    v_fmt.fmt.pix.dw_width = uvc_dev->dw_width;
    v_fmt.fmt.pix.dw_height = uvc_dev->dw_height;
    v_fmt.fmt.pix.dw_fmt = uvc_dev->dw_fcc;
    v_fmt.fmt.pix.dw_fld = VIDEO_FLD_NOTHING;

    if ((uvc_dev->dw_fcc == VIDEO_IMG_FORMAT_MJPEG) ||
        (uvc_dev->dw_fcc == VIDEO_IMG_FORMAT_H264) ||
        (uvc_dev->dw_fcc == VIDEO_IMG_FORMAT_H265)) {
        v_fmt.fmt.pix.dw_size = uvc_dev->dw_img_size;
    }

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_SET_FORMAT, &v_fmt);
    if (ret < 0) {
        ERR_INFO("Unable to set format: %d\n", errno);
    }

    return ret;
}

static td_void ot_uvc_stream_off(struct ot_uvc_dev *uvc_dev)
{
    td_s32 cache_type = VIDEO_CACHE_TYPE_OPT;
    td_s32 ret;

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_STREAM_OFF, &cache_type);
    if (ret < 0) {
        ERR_INFO("Unable to stream off: %d\n", errno);
    }

    ot_stream_shutdown();
    uvc_dev->i_streaming = 0;
    debug("Stopping video stream.\n");
}

static td_void ot_uvc_video_disable(struct ot_uvc_dev *uvc_dev)
{
    ot_uvc_stream_off(uvc_dev);
}

static td_void ot_uvc_video_enable(struct ot_uvc_dev *uvc_dev)
{
    encoder_property ep;

    ot_uvc_video_disable(uvc_dev);

    ep.format = uvc_dev->dw_fcc;
    ep.width = uvc_dev->dw_width;
    ep.height = uvc_dev->dw_height;
    ep.fps = uvc_dev->i_fps;
    ep.compsite = 0;

    ot_stream_set_enc_property(&ep);
    ot_stream_shutdown();
    ot_stream_startup();

    ot_uvc_video_stream_user_ptr(uvc_dev);
}

static td_s32 is_contain_udc_sub_dir(const td_char *path, td_s32 path_size)
{
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    if (path_size < 0) {
        return -1;
    }

    p_dir = opendir(path);
    if (p_dir == NULL) {
        ERR_INFO("No such path: %s\n", path);
        return -1;
    }

    while ((p_dirent = readdir(p_dir)) != NULL) {
        if (strcmp(p_dirent->d_name, "UDC") == 0) {
            closedir(p_dir);
            return 0;
        }
    }

    closedir(p_dir);

    return -1;
}

static td_s32 read_file(const td_char *path, td_u32 path_size, const td_char *dest, td_u32 size)
{
    FILE *input = NULL;
    td_char *realpath_res = NULL;
    td_s32 ret;

    realpath_res = realpath(path, NULL);
    if (realpath_res == NULL) {
        return -1;
    }

    input = fopen(realpath_res, "rw");
    free(realpath_res);
    realpath_res = NULL;
    if (input == NULL) {
        ERR_INFO("No such path: %s\n", path);
        return -1;
    }

    ret = fscanf_s(input, "%s", dest, size);
    if (ret != 1) {
        ERR_INFO("fscanf_s from %s fail\n", path);
        ret = TD_FAILURE;
    } else {
        ret = TD_SUCCESS;
    }

    if (fclose(input) != 0) {
        ERR_INFO("close file failed!\n");
    }

    return ret;
}

static td_s32 get_udc_node_name(td_char *node_name, td_u32 size)
{
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    const td_char *path_tmp = "/sys/kernel/config/usb_gadget/";
    td_char tmp[UDC_PATH_LENGTH]; /* 389: 256 bytes used for d_name, 128 bytes used for path_tmp, 5 bytes for UDC */

    p_dir = opendir(path_tmp);
    if (p_dir == NULL) {
        ERR_INFO("No such path: %s\n", path_tmp);
        return -1;
    }

    while ((p_dirent = readdir(p_dir)) != NULL) {
        if (strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0) {
            continue;
        }

        if (p_dirent->d_type == DT_DIR) {
            /* 389 = 128+256+5; 128 bytes used for path_tmp */
            strncpy_s(tmp, UDC_PATH_LENGTH, path_tmp, 128);
            strncat_s(tmp, UDC_PATH_LENGTH, p_dirent->d_name, 256); /* 389 = 128+256+5; 256 bytes used for d_name */

            if (is_contain_udc_sub_dir(tmp, UDC_PATH_LENGTH) == 0) {
                closedir(p_dir);
                strncat_s(tmp, UDC_PATH_LENGTH, "/UDC", 5); /* 389 = 128+256+5; 5 bytes for UDC */
                return read_file(tmp, UDC_PATH_LENGTH, node_name, size);
            }
        }
    }

    closedir(p_dir);

    return -1;
}

static td_u32 get_max_payload_transfer_size(const struct ot_uvc_frame_info *frm_info)
{
    const td_u32 high_speed_size = 3072;
    const td_u32 full_speed_size = 1023;
    const td_char *path_tmp = "/sys/class/udc/";

    td_char tmp[128];
    td_char target_path[GET_MAX_PACKET_SIZE_PATH]; // 398=256+128+14
    td_u32 result = high_speed_size;

    if (get_udc_node_name(tmp, 128) != 0) { /* 128 bytes used for tmp */
        return result;
    }

    /* 398=256+128+14   256 bytes used for path_tmp */
    strncpy_s(target_path, GET_MAX_PACKET_SIZE_PATH, path_tmp, 256);
    /* 398=256+128+14   128 bytes used for tmp */
    strncat_s(target_path, GET_MAX_PACKET_SIZE_PATH, tmp, 128);
    /* 398=256+128+14   14 bytes used for current_speed */
    strncat_s(target_path, GET_MAX_PACKET_SIZE_PATH, "/current_speed", 14);

    if (read_file(target_path, GET_MAX_PACKET_SIZE_PATH, tmp, 128) != 0) { /* 128 bytes used for tmp */
        return result;
    }

    if (strcmp(tmp, "super-speed") == 0) {
        result = frm_info->ss_xfersize;
    } else if (strcmp(tmp, "high-speed") == 0) {
        result = frm_info->hs_xfersize;
    } else if (strcmp(tmp, "full-speed") == 0) {
        result = full_speed_size;
    } else {
        ERR_INFO("USB cable is not connected yet.\n");
    }

    return result;
}

static td_void ot_uvc_get_buf_size(struct ot_uvc_dev *uvc_dev, const struct ot_uvc_frame_info *frm_info, td_u32 fcc,
    td_u32 *dw_max_video_frame_size)
{
    switch (fcc) {
        case VIDEO_IMG_FORMAT_YUYV:
            *dw_max_video_frame_size = frm_info->width * frm_info->height * 2; /* 2 bytes a pixel */
            break;
        case VIDEO_IMG_FORMAT_YUV420:
        case VIDEO_IMG_FORMAT_NV12:
        case VIDEO_IMG_FORMAT_NV21:
            *dw_max_video_frame_size = frm_info->width * frm_info->height * 3 / 2; /* 3 / 2,  1.5 byte a pixel */
            break;
        case VIDEO_IMG_FORMAT_MJPEG:
        case VIDEO_IMG_FORMAT_H264:
        case VIDEO_IMG_FORMAT_H265:
            *dw_max_video_frame_size = uvc_dev->dw_img_size;
            break;
        default:
            printf("format error!\n");
            break;
    }
}

static td_void ot_uvc_handle_streaming_control(struct ot_uvc_dev *uvc_dev, struct ot_uvc_streaming_control *u_str_ctrl,
    td_s32 i_frame, td_s32 i_format)
{
    const struct ot_uvc_format_info *fmt_info = NULL;
    const struct ot_uvc_frame_info *frm_info = NULL;
    td_u32 n_frm = 0;
    td_u32 max_video_frame_size = 0;

    if (i_format < 0) {
        i_format = (td_s32)(array_size(ot_fmt) + i_format);
    }

    if ((i_format < 0) || (i_format >= (td_s32)array_size(ot_fmt))) {
        return;
    }

    debug("i_format = %d\n", i_format);
    fmt_info = &ot_fmt[i_format];

    while (fmt_info->frames[n_frm].width != 0) {
        ++n_frm;
    }

    if (i_frame < 0) {
        i_frame = n_frm + i_frame;
    }

    if ((i_frame < 0) || (i_frame >= (td_s32)n_frm)) {
        return;
    }

    frm_info = &fmt_info->frames[i_frame];

    (td_void)memset_s(u_str_ctrl, sizeof(*u_str_ctrl), 0, sizeof(*u_str_ctrl));

    u_str_ctrl->bm_hint = 1;
    u_str_ctrl->b_format_index = i_format + 1;               /* Yuv: 1, Mjpeg: 2. */
    u_str_ctrl->b_frame_index = i_frame + 1;                 /* 360p: 1 720p: 2. */
    u_str_ctrl->dw_frame_interval = frm_info->intervals[0]; /* Corresponding to the number of frame rate. */

    ot_uvc_get_buf_size(uvc_dev, frm_info, fmt_info->fcc, &max_video_frame_size);
    (td_void)memcpy_s(&u_str_ctrl->dw_max_video_frame_size, sizeof(td_u32), &max_video_frame_size, sizeof(td_u32));

    if (uvc_dev->dw_bulk != 0) {
        u_str_ctrl->dw_max_payload_transfer_size = uvc_dev->dw_bulk_size; /* This should be filled by the driver. */
    } else {
        u_str_ctrl->dw_max_payload_transfer_size = get_max_payload_transfer_size(frm_info);
    }
    u_str_ctrl->bm_framing_info = 3; /* 3 format mode */
    u_str_ctrl->b_prefered_version = 1;
    u_str_ctrl->b_max_version = 1;
}

static td_void ot_uvc_handle_standard_request(const struct ot_uvc_dev *uvc_dev,
    const struct usb_ctrlrequest *u_ctrl_req, const struct uvc_request_data *u_req_data)
{
    debug("camera standard request\n");
}


static void ot_uvc_eve_undef_control(struct ot_uvc_dev *uvc_dev, td_u8 cs, struct uvc_request_data *u_req_data)
{
    switch (cs) {
        case OT_UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            u_req_data->length = uvc_dev->request_error_code.length;
            u_req_data->data[0] = uvc_dev->request_error_code.data[0];
            break;
        default:
            uvc_dev->request_error_code.length = 1;
            uvc_dev->request_error_code.data[0] = 0x06;
            break;
    }
}

static td_void ot_uvc_handle_control_request(struct ot_uvc_dev *uvc_dev, td_u8 req, td_u8 unit_id, td_u8 cs,
    struct uvc_request_data *u_req_data)
{
    switch (unit_id) {
        case OT_UVC_VC_DESCRIPTOR_UNDEFINED:
            ot_uvc_eve_undef_control(uvc_dev, cs, u_req_data);
            break;
        case OT_UVC_VC_HEADER:
            ot_stream_event_it_control(uvc_dev, req, unit_id, cs, u_req_data);
            break;
        case OT_UVC_VC_INPUT_TERMINAL:
            ot_stream_event_pu_control(uvc_dev, req, unit_id, cs, u_req_data);
            break;
        case UNIT_XU_H264:
            ot_stream_event_eu_h264_control(uvc_dev, req, unit_id, cs, u_req_data);
            break;
        default:
            uvc_dev->request_error_code.length = 1;
            uvc_dev->request_error_code.data[0] = 0x06;
    }
}

static td_void ot_uvc_handle_streaming_request(struct ot_uvc_dev *uvc_dev, td_u8 req, td_u8 cs,
    struct uvc_request_data *u_req_data)
{
    struct ot_uvc_streaming_control u_str_ctrl;
    struct ot_uvc_streaming_control *str_ctl;

    if ((cs != OT_UVC_VS_PROBE_CONTROL) && (cs != OT_UVC_VS_COMMIT_CONTROL)) {
        return;
    }

    errno_t ret = memcpy_s(&u_str_ctrl, sizeof(u_str_ctrl), u_req_data->data, sizeof(u_str_ctrl));
    if (ret != EOK) {
        ERR_INFO("copy out u_str_ctrl fail, ret %d\n", ret);
    }
    u_req_data->length = (td_s32)sizeof(u_str_ctrl);
    const struct ot_uvc_format_info *format = &ot_fmt[uvc_dev->probe.b_format_index - 1];
    const struct ot_uvc_frame_info *frame = &format->frames[uvc_dev->probe.b_frame_index - 1];

    switch (req) {
        case OT_UVC_SET_CUR:
            uvc_dev->i_control = cs;
            u_req_data->length = 0x22;
            u_str_ctrl.dw_max_payload_transfer_size = get_max_payload_transfer_size(frame);
            break;
        case OT_UVC_GET_CUR:
            str_ctl = (cs == OT_UVC_VS_PROBE_CONTROL) ? &uvc_dev->probe : &uvc_dev->commit;
            (td_void)memcpy_s(&u_str_ctrl, sizeof(u_str_ctrl), str_ctl, sizeof(u_str_ctrl));
            u_str_ctrl.dw_max_payload_transfer_size = get_max_payload_transfer_size(frame);
            break;
        case OT_UVC_GET_MIN:
        case OT_UVC_GET_MAX:
        case OT_UVC_GET_DEF:
            ot_uvc_handle_streaming_control(uvc_dev, &u_str_ctrl, uvc_dev->probe.b_frame_index - 1, \
                uvc_dev->probe.b_format_index - 1);
            break;
        case OT_UVC_GET_RES:
            (td_void)memset_s(u_req_data->data, sizeof(u_req_data->data), 0, sizeof(u_req_data->data));
            return;
        case OT_UVC_GET_LEN:
            u_req_data->data[0] = 0x00;
            u_req_data->data[1] = 0x22;
            u_req_data->length = 0x2;
            return;
        case OT_UVC_GET_INFO:
            u_req_data->data[0] = 0x03;
            u_req_data->length = 1;
            return;
        default:
            return;
    }
    ret = memcpy_s(u_req_data->data, sizeof(u_req_data->data), &u_str_ctrl, sizeof(u_str_ctrl));
    if (ret != EOK) {
        ERR_INFO("copy back u_str_ctrl to req_data fail %d\n", ret);
    }
}

static td_void set_probe_status(struct ot_uvc_dev *uvc_dev, td_s32 cs, td_s32 req)
{
    if (cs == 0x01) {
        switch (req) {
            case 0x01:
                uvc_dev->probe_status.b_set = 1;
                break;
            case 0x81:
                uvc_dev->probe_status.b_get = 1;
                break;
            case 0x82:
                uvc_dev->probe_status.b_min = 1;
                break;
            case 0x83:
                uvc_dev->probe_status.b_max = 1;
                break;
            case 0x84:
                break;
            case 0x85:
                break;
            case 0x86:
                break;
            default:
                break;
        }
    }
}

static td_void ot_uvc_handle_class_request(struct ot_uvc_dev *uvc_dev, struct usb_ctrlrequest *u_ctrl_req,
    struct uvc_request_data *u_req_data)
{
    const td_bool probe_status = TD_TRUE;

    if (probe_status) {
        td_u8 recip_type = u_ctrl_req->bRequestType & USB_RECIP_MASK;
        switch (recip_type) {
            case USB_RECIP_INTERFACE:
                debug("request type :td_s32ERFACE\n");
                debug("td_s32erface : %d\n", (u_ctrl_req->wIndex & 0xff));
                debug("unit id : %d\n", ((u_ctrl_req->wIndex & 0xff00) >> RIGHT_SHIFT_8BIT));
                debug("cs code : 0x%02x(%s)\n", (u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT),
                    (td_char *)get_intf_cs_string((u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT)));
                debug("req code: 0x%02x(%s)\n", u_ctrl_req->bRequest, (td_char *)get_code(u_ctrl_req->bRequest));

                set_probe_status(uvc_dev, (u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT), u_ctrl_req->bRequest);
                break;
            case USB_RECIP_DEVICE:
                debug("request type :DEVICE\n");
                break;
            case USB_RECIP_ENDPOINT:
                debug("request type :ENDPOINT\n");
                break;
            case USB_RECIP_OTHER:
                debug("request type :OTHER\n");
                break;
            default:
                break;
        }
    }

    if ((u_ctrl_req->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE) {
        return;
    }

    uvc_dev->i_control = (u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT);
    uvc_dev->i_unit_id = ((u_ctrl_req->wIndex & 0xff00) >> RIGHT_SHIFT_8BIT);
    uvc_dev->i_intf_id = (u_ctrl_req->wIndex & 0xff);

    switch (u_ctrl_req->wIndex & 0xff) {
        case OT_UVC_INTF_CONTROL:
            ot_uvc_handle_control_request(uvc_dev, u_ctrl_req->bRequest, u_ctrl_req->wIndex >> RIGHT_SHIFT_8BIT,
                u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT, u_req_data);
            break;
        case OT_UVC_INTF_STREAMING:
            ot_uvc_handle_streaming_request(uvc_dev, u_ctrl_req->bRequest, u_ctrl_req->wValue >> RIGHT_SHIFT_8BIT,
                u_req_data);
            break;
        default:
            break;
    }
}

static td_void do_ot_uvc_setup_event(struct ot_uvc_dev *uvc_dev, struct usb_ctrlrequest *u_ctrl_req,
    struct uvc_request_data *u_req_data)
{
    uvc_dev->i_control = 0;
    uvc_dev->i_unit_id = 0;
    uvc_dev->i_intf_id = 0;

    switch (u_ctrl_req->bRequestType & USB_TYPE_MASK) {
        case USB_TYPE_STANDARD:
            ot_uvc_handle_standard_request(uvc_dev, u_ctrl_req, u_req_data);
            break;
        case USB_TYPE_CLASS:
            ot_uvc_handle_class_request(uvc_dev, u_ctrl_req, u_req_data);
            break;
        default:
            break;
    }
}

static td_void handle_control_interface_data(struct ot_uvc_dev *uvc_dev, struct uvc_request_data *u_req_data)
{
    switch (uvc_dev->i_unit_id) {
        case OT_UVC_VC_HEADER:
            ot_stream_event_it_data(uvc_dev, uvc_dev->i_unit_id, uvc_dev->i_control, u_req_data);
            break;
        case OT_UVC_VC_INPUT_TERMINAL:
            ot_stream_event_pu_data(uvc_dev, uvc_dev->i_unit_id, uvc_dev->i_control, u_req_data);
            break;
        case UNIT_XU_H264:
            ot_stream_event_eu_h264_data(uvc_dev, uvc_dev->i_unit_id, uvc_dev->i_control, u_req_data);
            break;
        default:
            break;
    }
}

static td_u32 clamp(td_u32 val, td_u32 min, td_u32 max)
{
    td_u32 res = val;
    if (res < min) {
        return min;
    }
    if (res > max) {
        return max;
    }
    return val;
}

static void compute_video_frame_size(td_u32 img_fmt, const struct ot_uvc_dev *uvc_dev,
    struct ot_uvc_streaming_control *u_str_target, const struct ot_uvc_frame_info *frm_info)
{
    switch (img_fmt) {
        case VIDEO_IMG_FORMAT_YUYV:
            u_str_target->dw_max_video_frame_size = frm_info->width * frm_info->height * 2; /* 2 bytes a pixel */
            break;
        case VIDEO_IMG_FORMAT_NV12:
        case VIDEO_IMG_FORMAT_NV21:
        case VIDEO_IMG_FORMAT_YUV420:
            u_str_target->dw_max_video_frame_size =
                frm_info->width * frm_info->height * 3 / 2; /* 3 / 2, 1.5 bytes a pixel */
            break;
        case VIDEO_IMG_FORMAT_MJPEG:
        case VIDEO_IMG_FORMAT_H264:
        case VIDEO_IMG_FORMAT_H265:
            if (uvc_dev->dw_img_size == 0) {
                debug("WARNING: MJPEG requested and no image loaded.\n");
            }

            u_str_target->dw_max_video_frame_size = uvc_dev->dw_img_size;
            break;
        default:
            debug("WARNING: wrong image format\n");
            break;
    }
}

static struct ot_uvc_streaming_control *get_streaming_control(td_s32 control_type, struct ot_uvc_dev *uvc_dev,
    const struct uvc_request_data *u_req_data)
{
    switch (control_type) {
        case OT_UVC_VS_PROBE_CONTROL:
            debug("setting probe control, length = %d\n", u_req_data->length);
            return &uvc_dev->probe;
        case OT_UVC_VS_COMMIT_CONTROL:
            debug("setting commit control, length = %d\n", u_req_data->length);
            return &uvc_dev->commit;
        default:
            debug("setting unknown control, length = %d\n", u_req_data->length);
            return NULL;
    }
}

static td_void do_ot_uvc_data_event(struct ot_uvc_dev *uvc_dev, struct uvc_request_data *u_req_data)
{
    struct ot_uvc_streaming_control *u_str_target = NULL;
    struct ot_uvc_streaming_control u_str_ctrl;
    const struct ot_uvc_format_info *fmt_info = NULL;
    const struct ot_uvc_frame_info *frm_info = NULL;
    const td_u32 *intval = NULL;
    td_u32 i_fmt, i_frm;
    td_u32 n_frm;

    if ((uvc_dev->i_unit_id != 0) && (uvc_dev->i_intf_id == OT_UVC_INTF_CONTROL)) {
        return handle_control_interface_data(uvc_dev, u_req_data);
    }

    if ((u_str_target = get_streaming_control(uvc_dev->i_control, uvc_dev, u_req_data)) == NULL) {
        return;
    }

    td_s32 ret = memcpy_s(&u_str_ctrl, sizeof(u_str_ctrl), u_req_data->data, sizeof(u_str_ctrl));
    if (ret != EOK) {
        ERR_INFO("memcpy_s u_str_ctrl fail, ret %d\n", ret);
        return;
    }

    i_fmt = clamp((td_u32)u_str_ctrl.b_format_index, 1U, (td_u32)array_size(ot_fmt));

    fmt_info = &ot_fmt[i_fmt - 1];
    n_frm = 0;

    while (fmt_info->frames[n_frm].width != 0) {
        ++n_frm;
    }

    i_frm = clamp((td_u32)u_str_ctrl.b_frame_index, 1U, n_frm);
    frm_info = &fmt_info->frames[i_frm - 1];
    intval = frm_info->intervals;

    while ((intval[0] < u_str_ctrl.dw_frame_interval) && intval[1]) {
        ++intval;
    }

    u_str_target->b_format_index = (td_u8)i_fmt;
    u_str_target->b_frame_index = (td_u8)i_frm;

    compute_video_frame_size(fmt_info->fcc, uvc_dev, u_str_target, frm_info);

    u_str_target->dw_frame_interval = *intval;

    if (uvc_dev->i_control == OT_UVC_VS_COMMIT_CONTROL) {
        uvc_dev->dw_fcc = fmt_info->fcc;
        uvc_dev->dw_width = frm_info->width;
        uvc_dev->dw_height = frm_info->height;
        uvc_dev->i_fps = FRAME_INTERVAL_CALC_100NS / u_str_target->dw_frame_interval;

        printf("set format=%s  %ux%u\n", image_fmt_to_string(uvc_dev->dw_fcc), uvc_dev->dw_width, uvc_dev->dw_height);

        ot_uvc_video_set_fmt(uvc_dev);

        if (uvc_dev->dw_bulk != 0) {
            ot_uvc_video_disable(uvc_dev);
            ot_uvc_video_enable(uvc_dev);
        }
        (td_void)memset_s(&uvc_dev->probe_status, sizeof(uvc_dev->probe_status), 0, sizeof(uvc_dev->probe_status));
    }
}

static td_void do_ot_uvc_event(struct ot_uvc_dev *uvc_dev)
{
    struct video_event v_event;
    struct ot_uvc_event *ot_eve = (struct ot_uvc_event *)(td_void *)&v_event.u.a_data;
    struct uvc_request_data u_req_data;
    td_s32 ret;

    debug("#############do_ot_uvc_event()#############\n");

    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DQUEUE_EVENT, &v_event);
    if (ret < 0) {
        ERR_INFO("VIDEO_IOCTL_DQUEUE_EVENT failed: %d\n", errno);
        return;
    }

    (td_void)memset_s(&u_req_data, sizeof(u_req_data), 0, sizeof(u_req_data));
    u_req_data.length = 32; /* 32 byte data length */

    switch (v_event.dw_type) {
        case OT_UVC_EVE_CON:
            debug("handle connect event\n");
            ot_uvc_handle_streaming_control(uvc_dev, &uvc_dev->probe, 0, 0);
            ot_uvc_handle_streaming_control(uvc_dev, &uvc_dev->commit, 0, 0);
            return;
        case OT_UVC_EVE_DISCON:
            return;
        case OT_UVC_EVE_SETTING:
            do_ot_uvc_setup_event(uvc_dev, &ot_eve->req, &u_req_data);
            break;
        case OT_UVC_EVE_DATA:
            do_ot_uvc_data_event(uvc_dev, &ot_eve->data);
            return;
        case OT_UVC_EVE_STRON:
            if (uvc_dev->dw_bulk == 0) {
                ot_uvc_video_enable(uvc_dev);
            }
            return;
        case OT_UVC_EVE_STROFF:
            if (uvc_dev->dw_bulk == 0) {
                ot_uvc_video_disable(uvc_dev);
            }
            return;
        default:
            break;
    }

    ret = ioctl(uvc_dev->i_fd, OT_UVC_IOC_SEND_RESPONSE, &u_req_data);
    if (ret < 0) {
        ERR_INFO("OT_UVC_IOC_S_EVENT failed: %d\n", errno);
        return;
    }
}

static td_void ot_uvc_event_register(struct ot_uvc_dev *uvc_dev)
{
    struct video_event_descriptor v_event_desc;
    td_s32 ret;

    (td_void)memset_s(&v_event_desc, sizeof(v_event_desc), 0, sizeof(v_event_desc));

    v_event_desc.dw_type = OT_UVC_EVE_CON;
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DESC_EVENT, &v_event_desc);
    if (ret < 0) {
        ERR_INFO("Connect event failed: %d\n", errno);
    }

    v_event_desc.dw_type = OT_UVC_EVE_SETTING;
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DESC_EVENT, &v_event_desc);
    if (ret < 0) {
        ERR_INFO("Setup event failed: %d\n", errno);
    }

    v_event_desc.dw_type = OT_UVC_EVE_DATA;
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DESC_EVENT, &v_event_desc);
    if (ret < 0) {
        ERR_INFO("Data event failed: %d\n", errno);
    }

    v_event_desc.dw_type = OT_UVC_EVE_STRON;
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DESC_EVENT, &v_event_desc);
    if (ret < 0) {
        ERR_INFO("StreamOn event failed: %d\n", errno);
    }

    v_event_desc.dw_type = OT_UVC_EVE_STROFF;
    ret = ioctl(uvc_dev->i_fd, VIDEO_IOCTL_DESC_EVENT, &v_event_desc);
    if (ret < 0) {
        ERR_INFO("StreamOff event failed: %d\n", errno);
    }
}

td_s32 open_uvc_device(const td_char *dev_path)
{
    struct ot_uvc_dev *uvc_dev;

    uvc_dev = ot_uvc_init(dev_path);
    if (uvc_dev == 0) {
        return -1;
    }

    uvc_dev->dw_img_size = MAX_PAYLOAD_IMAGE_SIZE; /* 2160*3840*2 */

    debug("set imagesize = %d, set bulkmode =%d, set bulksize = %d\n", uvc_dev->dw_img_size, uvc_dev->dw_bulk,
        uvc_dev->dw_bulk_size);

    ot_uvc_event_register(uvc_dev);
    g_ot_cd = uvc_dev;

    return 0;
}

td_s32 close_uvc_device(td_void)
{
    if (g_ot_cd != 0) {
        ot_uvc_video_disable(g_ot_cd);
        close(g_ot_cd->i_fd);
        free(g_ot_cd);
    }

    g_ot_cd = 0;
    return 0;
}

td_s32 run_uvc_data(td_void)
{
    fd_set w_fds;
    td_s32 ret;
    struct timeval t_val;

    if (g_ot_cd == NULL) {
        return -1;
    }

    t_val.tv_sec = 1;
    t_val.tv_usec = 0;

    FD_ZERO(&w_fds);

    if (g_ot_cd->i_streaming == 1) {
        FD_SET(g_ot_cd->i_fd, &w_fds);
    }

    ret = select(g_ot_cd->i_fd + 1, NULL, &w_fds, NULL, &t_val);
    if (ret > 0) {
        if (FD_ISSET(g_ot_cd->i_fd, &w_fds)) {
            ot_uvc_video_process_user_ptr(g_ot_cd);
        }
    }

    return ret;
}

td_s32 run_uvc_device(td_void)
{
    fd_set e_fds;
    td_s32 ret;
    struct timeval t_val;

    if (g_ot_cd == NULL) {
        return -1;
    }

    t_val.tv_sec = 1;
    t_val.tv_usec = 0;

    FD_ZERO(&e_fds);
    FD_SET(g_ot_cd->i_fd, &e_fds);

    ret = select(g_ot_cd->i_fd + 1, NULL, NULL, &e_fds, &t_val);
    if (ret > 0) {
        if (FD_ISSET(g_ot_cd->i_fd, &e_fds)) {
            do_ot_uvc_event(g_ot_cd);
        }
    }

    return ret;
}
