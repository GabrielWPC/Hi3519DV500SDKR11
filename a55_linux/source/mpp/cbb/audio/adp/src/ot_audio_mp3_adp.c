/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "securec.h"
#include "ot_common_aenc.h"
#include "ot_common_adec.h"
#include "ss_mpi_audio.h"
#include "ot_audio_dl_adp.h"
#include "ot_audio_mp3_adp.h"

#define mp3_check_false_print(expr)                                                   \
    do {                                                                              \
        if (!(expr)) {                                                                \
            printf("[func]:%s [line]:%d [info]:%s\n", __FUNCTION__, __LINE__, #expr); \
            return TD_FAILURE;                                                        \
        }                                                                             \
    } while (0)

#define mp3_err_trace(fmt, ...)  printf("[func]:%s [line]:%d [error]:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define MP3_ENC_LIB_NAME "libmp3_enc.so"
#define MP3_DEC_LIB_NAME "libmp3_dec.so"

typedef struct {
    td_void *state;
    ot_aenc_attr_mp3 attr;
} mp3_aenc_encoder;

typedef struct {
    ot_adec_attr_mp3 attr;
    ot_mp3dec_handle state;
} mp3_adec_decoder;

typedef ot_mp3enc_handle (*mp3enc_create_callback)(const ot_mp3enc_attr *attr);
typedef td_s32 (*mp3enc_process_frame_callback)(ot_mp3enc_handle mp3_encoder, td_s16 *pcm_buf, td_u8 *out_buf,
    td_s32 *num_out_bytes);
typedef td_void (*mp3enc_destroy_callback)(ot_mp3enc_handle mp3_encoder);

typedef ot_mp3dec_handle (*mp3dec_init_decoder_callback)(td_void);
typedef td_s32 (*mp3dec_decode_callback)(ot_mp3dec_handle mp3_decoder, td_u8 **in_buff, td_s32 *bytes_left,
    td_s16 *out_pcm, td_s32 reserved);
typedef td_s32 (*mp3dec_get_last_frame_info_callback)(ot_mp3dec_handle mp3_decoder, ot_mp3dec_frame_info *frame_info);
typedef td_s32 (*mp3dec_find_sync_header_callback)(ot_mp3dec_handle mp3_decoder, td_u8 **in_buff, td_s32 *bytes_left);
typedef td_void (*mp3dec_free_decoder_callback)(ot_mp3dec_handle mp3_decoder);

typedef struct {
    td_void *lib_handle;

    mp3enc_create_callback mp3enc_create_call;
    mp3enc_process_frame_callback mp3enc_process_frame_call;
    mp3enc_destroy_callback mp3enc_destroy_call;
} mp3enc_fun;

typedef struct {
    td_void *lib_handle;

    mp3dec_init_decoder_callback mp3dec_init_decoder_call;
    mp3dec_decode_callback mp3dec_decode_call;
    mp3dec_get_last_frame_info_callback mp3dec_get_last_frame_info_call;
    mp3dec_find_sync_header_callback mp3dec_find_sync_header_call;
    mp3dec_free_decoder_callback mp3dec_free_decoder_call;
} mp3dec_fun;

static mp3enc_fun g_mp3enc_func = {0};
static mp3dec_fun g_mp3dec_func = {0};

#define MAX_MP3_MAINBUF_SIZE (2048 * 2) /* max length of MP3 stream by bytes */
#define MP3_SAMPLES_PER_FRAME 1152
#define NO_ENOUGH_MAIN_DATA_ERROR 11
#define MP3_ENCODER_MAX_QUALITY 9
#define MP3_UNINTERLEAVED_NUMS 2 /* 32bits/16bits */
#define BITS_PER_BYTE 8 /* 1byte = 8bits */
#define AENC_DUMP_CNT_PRIM 10000

static td_s32 g_mp3_encoder_handle = -1;
static td_s32 g_mp3_decoder_handle = -1;

#ifdef DUMP_MP3ENC
static FILE *g_enc_in_fp = TD_NULL;
static FILE *g_enc_out_fp = TD_NULL;
static td_s32 g_cnt_aenc = 100000;
#endif

static td_s32 aenc_check_mp3_base_attr(ot_audio_sample_rate sample_rate, ot_audio_bit_width bit_width,
    ot_audio_snd_mode sound_mode)
{
    if (bit_width != OT_AUDIO_BIT_WIDTH_16) {
        mp3_err_trace("invalid bit_width(%d), only support 16bit.", bit_width);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (sample_rate != OT_AUDIO_SAMPLE_RATE_8000 && sample_rate != OT_AUDIO_SAMPLE_RATE_12000 &&
        sample_rate != OT_AUDIO_SAMPLE_RATE_11025 && sample_rate != OT_AUDIO_SAMPLE_RATE_16000 &&
        sample_rate != OT_AUDIO_SAMPLE_RATE_22050 && sample_rate != OT_AUDIO_SAMPLE_RATE_24000 &&
        sample_rate != OT_AUDIO_SAMPLE_RATE_32000 && sample_rate != OT_AUDIO_SAMPLE_RATE_44100 &&
        sample_rate != OT_AUDIO_SAMPLE_RATE_48000) {
        mp3_err_trace("invalid samplerate(%d), range is [%d, %d].\n",
            sample_rate, OT_AUDIO_SAMPLE_RATE_8000, OT_AUDIO_SAMPLE_RATE_48000);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (sound_mode >= OT_AUDIO_SOUND_MODE_BUTT) {
        mp3_err_trace("invalid sound_mode(%d), it should be %d or %d.",
            sound_mode, OT_AUDIO_SOUND_MODE_MONO, OT_AUDIO_SOUND_MODE_STEREO);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 aenc_check_mp3_attr(const ot_aenc_attr_mp3 *mp3_encoder)
{
    td_s32 ret = OT_ERR_AENC_ILLEGAL_PARAM;
    if (mp3_encoder->bit_rate < OT_MP3_BPS_32K || mp3_encoder->bit_rate > OT_MP3_BPS_320K) {
        mp3_err_trace("invalid bit_rate(%d), range is [%d, %d].", mp3_encoder->bit_rate,
            OT_MP3_BPS_32K, OT_MP3_BPS_320K);
        return ret;
    }

    if (mp3_encoder->quality > MP3_ENCODER_MAX_QUALITY) {
        mp3_err_trace("invalid quality(%u), range is [0, %d]", mp3_encoder->quality,
            MP3_ENCODER_MAX_QUALITY);
        return ret;
    }

    ret = aenc_check_mp3_base_attr(mp3_encoder->sample_rate, mp3_encoder->bit_width, mp3_encoder->sound_mode);
    return ret;
}

#ifdef DUMP_MP3ENC
static td_void mp3_enc_dump(const ot_mp3enc_attr *config)
{
    td_s32 ret;
    td_char name_in[OT_MP3_ADP_MAX_NAME_LEN];
    td_char name_out[OT_MP3_ADP_MAX_NAME_LEN];

    ret = snprintf_s(name_in, OT_MP3_ADP_MAX_NAME_LEN, (OT_MP3_ADP_MAX_NAME_LEN - 1), "MP3ENC_sin_T%d_t%d.raw",
        config->bit_rate, config->sample_rate);
    if (ret <= EOK) {
        mp3_err_trace("name_in snprintf_s failed with 0x%x.\n", ret);
    }
    ret = snprintf_s(name_out, OT_MP3_ADP_MAX_NAME_LEN, (OT_MP3_ADP_MAX_NAME_LEN - 1), "MP3ENC_sout_T%d_t%d.mp3",
        config->bit_rate, config->sample_rate);
    if (ret <= EOK) {
        mp3_err_trace("name_out snprintf_s failed with 0x%x.\n", ret);
    }
    g_enc_in_fp = fopen(name_in, "w+");
    g_enc_out_fp = fopen(name_out, "w+");
    g_cnt_aenc = AENC_DUMP_CNT_PRIM;
}
#endif

static td_s32 open_mp3_encdoer(td_void *encoder_attr, td_void **encoder)
{
    mp3_aenc_encoder *mp3_encoder = TD_NULL;
    ot_aenc_attr_mp3 *mp3_attr = TD_NULL;
    td_s32 ret;
    ot_mp3enc_attr config;

    mp3_check_false_print(encoder_attr != TD_NULL);
    mp3_check_false_print(encoder != TD_NULL);
    mp3_check_false_print(g_mp3enc_func.mp3enc_create_call != TD_NULL);

    /* check attribute of encoder */
    mp3_attr = (ot_aenc_attr_mp3 *)encoder_attr;
    ret = aenc_check_mp3_attr(mp3_attr);
    if (ret) {
        mp3_err_trace("check mp3 attr failed with 0x%x.", ret);
        return ret;
    }

    /* allocate memory for encoder */
    mp3_encoder = (mp3_aenc_encoder *)malloc(sizeof(mp3_aenc_encoder));
    if (mp3_encoder == TD_NULL) {
        mp3_err_trace("malloc mp3_encoder failed.");
        return OT_ERR_ADEC_NO_MEM;
    }
    ret = memset_s(mp3_encoder, sizeof(mp3_aenc_encoder), 0, sizeof(mp3_aenc_encoder));
    if (ret != EOK) {
        mp3_err_trace("memset_s failed with 0x%x.", ret);
    }

    if (mp3_attr->sound_mode == OT_AUDIO_SOUND_MODE_MONO) {
        config.channels = 1;
    } else {
        config.channels = OT_MP3ENC_MAX_CHN_NUM;
    }

    config.bit_rate = mp3_attr->bit_rate;
    config.sample_rate = mp3_attr->sample_rate;
    config.quality = mp3_attr->quality;

    /* create encoder */
    mp3_encoder->state = g_mp3enc_func.mp3enc_create_call(&config);
    if (mp3_encoder->state == TD_NULL) {
        free(mp3_encoder);
        mp3_err_trace("failed to create mp3 encoder.");
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    ret = memcpy_s(&mp3_encoder->attr, sizeof(ot_aenc_attr_mp3), mp3_attr, sizeof(ot_aenc_attr_mp3));
    if (ret != EOK) {
        mp3_err_trace("memset_s failed with 0x%x.\n", ret);
    }

    *encoder = (td_void *)mp3_encoder;

#ifdef DUMP_MP3ENC
    mp3_enc_dump(&config);
#endif

    return TD_SUCCESS;
}

static td_s32 open_mp3_decoder(td_void *decoder_attr, td_void **decoder)
{
    mp3_adec_decoder *mp3_decoder = TD_NULL;
    ot_adec_attr_mp3 *mp3_attr = TD_NULL;
    td_s32 ret;

    mp3_check_false_print(decoder_attr != TD_NULL);
    mp3_check_false_print(decoder != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_init_decoder_call != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_free_decoder_call != TD_NULL);

    mp3_attr = (ot_adec_attr_mp3 *)decoder_attr;

    /* allocate memory for  decoder */
    mp3_decoder = (mp3_adec_decoder *)malloc(sizeof(mp3_adec_decoder));
    if (mp3_decoder == TD_NULL) {
        mp3_err_trace("malloc mp3 decoder failed!\n");
        return OT_ERR_ADEC_NO_MEM;
    }

    ret = memset_s(mp3_decoder, sizeof(mp3_adec_decoder), 0, sizeof(mp3_adec_decoder));
    if (ret != EOK) {
        mp3_err_trace("memset_s failed wtih 0x%x.\n", ret);
        free(mp3_decoder);
        return OT_ERR_ADEC_NOT_PERM;
    }

    /* create decoder */
    mp3_decoder->state = g_mp3dec_func.mp3dec_init_decoder_call();
    if (mp3_decoder->state == TD_NULL) {
        mp3_err_trace("mp3 decoder is not ready.");
        free(mp3_decoder);
        mp3_decoder = TD_NULL;
        return OT_ERR_ADEC_DECODER_ERR;
    }
    ret = memcpy_s(&mp3_decoder->attr, sizeof(ot_adec_attr_mp3), mp3_attr, sizeof(ot_adec_attr_mp3));
    if (ret != EOK) {
        mp3_err_trace("memcpy_s failed with 0x%x.\n", ret);
        g_mp3dec_func.mp3dec_free_decoder_call(mp3_decoder->state);
        free(mp3_decoder);
        mp3_decoder = TD_NULL;
        return OT_ERR_ADEC_ILLEGAL_PARAM;
    }

    *decoder = (td_void *)mp3_decoder;

    return TD_SUCCESS;
}

static td_s32 uninterleaved_data(const mp3_aenc_encoder *encoder, const ot_audio_frame *frame, td_s16 *frame_array,
    td_u32 array_len)
{
    td_s32 ret;
    td_u32 water_line = OT_MP3ENC_MAX_IN_SAMPELS;

    ret = memcpy_s(frame_array, array_len * sizeof(td_s16), frame->virt_addr[0], water_line * sizeof(td_s16));
    if (ret != EOK) {
        mp3_err_trace("memcpy_s failed with 0x%x.\n", ret);
        return ret;
    }

    if (encoder->attr.sound_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        ret = memcpy_s(frame_array + water_line, (array_len - water_line) * sizeof(td_s16), frame->virt_addr[1],
            water_line * sizeof(td_s16));
        if (ret != EOK) {
            mp3_err_trace("memcpy_s failed with 0x%x.\n", ret);
            return ret;
        }
    }

    return ret;
}

static td_s32 encode_mp3_frame(td_void *encoder, const ot_audio_frame *frame, ot_aenc_buf_info *buff_info)
{
    td_s32 ret;
    mp3_aenc_encoder *mp3_encoder = TD_NULL;
    td_u32 point_nums;
    td_u32 water_line = OT_MP3ENC_MAX_IN_SAMPELS;
    td_s16 frame_array[OT_MP3ENC_MAX_IN_SAMPELS * MP3_UNINTERLEAVED_NUMS * OT_MP3ENC_MAX_CHN_NUM];
    td_u32 array_len = OT_MP3ENC_MAX_IN_SAMPELS * MP3_UNINTERLEAVED_NUMS * OT_MP3ENC_MAX_CHN_NUM;

    mp3_check_false_print(encoder != TD_NULL);
    mp3_check_false_print(frame != TD_NULL);
    mp3_check_false_print(buff_info != TD_NULL);
    mp3_check_false_print(g_mp3enc_func.mp3enc_process_frame_call != TD_NULL);

    mp3_encoder = (mp3_aenc_encoder *)encoder;

    if (mp3_encoder->attr.sound_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        /* whether the sound mode of frame and channel is match  */
        if (frame->snd_mode != OT_AUDIO_SOUND_MODE_STEREO) {
            mp3_err_trace("sound_mode(%d) of the received frame does not match, it should be %d.",
                frame->snd_mode, OT_AUDIO_SOUND_MODE_STEREO);
            return OT_ERR_AENC_ILLEGAL_PARAM;
        }
    }

    /* calculate point number */
    point_nums = frame->len / (frame->bit_width + 1);

    /* if frame sample larger than protocol sample, reject to receive, or buffer will be overflow */
    if (point_nums != water_line) {
        mp3_err_trace("invalid point_nums(%u), it should be %u.\n", point_nums, water_line);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    /* mp3 encoder need uninterleaved data */
    ret = uninterleaved_data(mp3_encoder, frame, frame_array, array_len);
    if (ret != TD_SUCCESS) {
        return OT_ERR_AENC_ENCODER_ERR;
    }

#ifdef DUMP_MP3ENC
    if (g_cnt_aenc > 0) {
        fwrite((td_u8 *)frame_array, 1,
            (mp3_encoder->attr.sound_mode == OT_AUDIO_SOUND_MODE_STEREO) ?
            frame->len * MP3_UNINTERLEAVED_NUMS : frame->len,
            g_enc_in_fp);
    }
#endif

    ret = g_mp3enc_func.mp3enc_process_frame_call(mp3_encoder->state, frame_array, buff_info->out_buf,
        (td_s32 *)(td_void *)buff_info->out_valid_len);
    if (ret != TD_SUCCESS) {
        mp3_err_trace("encode mp3 frame failed with 0x%x.", ret);
        return OT_ERR_AENC_ENCODER_ERR;
    }

#ifdef DUMP_MP3ENC
    if (g_cnt_aenc > 0) {
        fwrite((td_u8 *)buff_info->out_buf, 1, *((td_s32 *)buff_info->out_valid_len), g_enc_out_fp);
        g_cnt_aenc--;
    }
#endif
    return ret;
}

static td_s32 decode_mp3_frame(td_void *decoder, ot_adec_buf_info *buff_info, td_u32 *chn_num)
{
    td_s32 ret;
    mp3_adec_decoder *mp3_decoder = TD_NULL;
    td_s32 frame_len;
    ot_mp3dec_frame_info frame_info;

    mp3_check_false_print(decoder != TD_NULL);
    mp3_check_false_print(buff_info != TD_NULL);
    mp3_check_false_print(chn_num != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_find_sync_header_call != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_get_last_frame_info_call != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_decode_call != TD_NULL);

    *chn_num = 1; /* voice encoder only one channle */

    mp3_decoder = (mp3_adec_decoder *)decoder;

    frame_len = g_mp3dec_func.mp3dec_find_sync_header_call(mp3_decoder->state,
        buff_info->in_buf, buff_info->in_left_byte);
    if (frame_len < 0) {
        return OT_ERR_ADEC_BUF_LACK;
    }
    ret = g_mp3dec_func.mp3dec_decode_call(mp3_decoder->state, buff_info->in_buf,
        buff_info->in_left_byte, (td_s16 *)(td_void *)buff_info->out_buf, 0);
    if (ret) {
        if (ret == NO_ENOUGH_MAIN_DATA_ERROR) {
            return OT_ERR_ADEC_BUF_LACK;
        }
        return ret;
    }

    g_mp3dec_func.mp3dec_get_last_frame_info_call(mp3_decoder->state, &frame_info);
    *chn_num = frame_info.chn_num;
    *buff_info->out_valid_len = (frame_info.output_samples / frame_info.chn_num) *
                                (frame_info.bits_per_sample / BITS_PER_BYTE);

    return ret;
}

static td_s32 get_mp3_frm_info(td_void *decoder, td_void *frm_info)
{
    mp3_adec_decoder *mp3_decoder = TD_NULL;
    ot_mp3dec_frame_info mp3_frame_info;
    ot_mp3_frame_info *mp3_frm = TD_NULL;

    mp3_check_false_print(decoder != TD_NULL);
    mp3_check_false_print(frm_info != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_get_last_frame_info_call != TD_NULL);

    mp3_decoder = (mp3_adec_decoder *)decoder;
    mp3_frm = (ot_mp3_frame_info *)frm_info;

    g_mp3dec_func.mp3dec_get_last_frame_info_call(mp3_decoder->state, &mp3_frame_info);

    mp3_frm->bit_rate = mp3_frame_info.bit_rate;
    mp3_frm->bits_per_sample = mp3_frame_info.bits_per_sample;
    mp3_frm->chn_num = mp3_frame_info.chn_num;
    mp3_frm->layer = mp3_frame_info.layer;
    mp3_frm->output_samples = mp3_frame_info.output_samples;
    mp3_frm->sample_rate = mp3_frame_info.sample_rate;
    mp3_frm->version = mp3_frame_info.version;

    return TD_SUCCESS;
}

static td_s32 close_mp3_encoder(td_void *encoder)
{
    mp3_aenc_encoder *mp3_encoder = TD_NULL;

    mp3_check_false_print(encoder != TD_NULL);
    mp3_check_false_print(g_mp3enc_func.mp3enc_destroy_call != TD_NULL);
    mp3_encoder = (mp3_aenc_encoder *)encoder;

    g_mp3enc_func.mp3enc_destroy_call(mp3_encoder->state);

    free(mp3_encoder);

#ifdef DUMP_MP3ENC
    if (g_enc_in_fp) {
        fclose(g_enc_in_fp);
        g_enc_in_fp = TD_NULL;
    }

    if (g_enc_out_fp) {
        fclose(g_enc_out_fp);
        g_enc_out_fp = TD_NULL;
    }
#endif

    return TD_SUCCESS;
}

static td_s32 close_mp3_decoder(td_void *decoder)
{
    mp3_adec_decoder *mp3_decoder = TD_NULL;

    mp3_check_false_print(decoder != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_free_decoder_call != TD_NULL);

    mp3_decoder = (mp3_adec_decoder *)decoder;

    g_mp3dec_func.mp3dec_free_decoder_call(mp3_decoder->state);

    free(mp3_decoder);
    return TD_SUCCESS;
}

static td_s32 reset_mp3_decoder(td_void *decoder)
{
    mp3_adec_decoder *mp3_decoder = TD_NULL;

    mp3_check_false_print(decoder != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_init_decoder_call != TD_NULL);
    mp3_check_false_print(g_mp3dec_func.mp3dec_free_decoder_call != TD_NULL);

    mp3_decoder = (mp3_adec_decoder *)decoder;

    g_mp3dec_func.mp3dec_free_decoder_call(mp3_decoder->state);

    /* create decoder */
    mp3_decoder->state = g_mp3dec_func.mp3dec_init_decoder_call();
    if (!mp3_decoder->state) {
        mp3_err_trace("failed to reset mp3 decoder.");
        return OT_ERR_ADEC_DECODER_ERR;
    }

    return TD_SUCCESS;
}

#ifdef OT_MP3_USE_DYNAMIC_LOAD
static td_s32 mp3_enc_lib_dlsym(mp3enc_fun *mp3_enc_fun)
{
    td_s32 ret;

    ret = ot_audio_dlsym((td_void **)&(mp3_enc_fun->mp3enc_create_call),
        mp3_enc_fun->lib_handle, "ot_mp3enc_create");
    if (ret != TD_SUCCESS) {
        return OT_ERR_AENC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_enc_fun->mp3enc_process_frame_call),
        mp3_enc_fun->lib_handle, "ot_mp3enc_process_frame");
    if (ret != TD_SUCCESS) {
        return OT_ERR_AENC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_enc_fun->mp3enc_destroy_call),
        mp3_enc_fun->lib_handle, "ot_mp3enc_destroy");
    if (ret != TD_SUCCESS) {
        return OT_ERR_AENC_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}

static td_s32 mp3_aenc_init_lib(void)
{
    td_s32 ret;
    mp3enc_fun mp3_enc_fun;

    ret = ot_audio_dlopen(&(mp3_enc_fun.lib_handle), MP3_ENC_LIB_NAME);
    if (ret != TD_SUCCESS) {
        printf("[func]:%s [line]:%d [info]:%s\n", __FUNCTION__, __LINE__, "load aenc lib fail!\n");
        return OT_ERR_AENC_NOT_SUPPORT;
    }

    ret = mp3_enc_lib_dlsym(&mp3_enc_fun);
    if (ret != TD_SUCCESS) {
        printf("[func]:%s [line]:%d [info]:%s\n", __FUNCTION__, __LINE__, "find symbol error!\n");
        ot_audio_dlclose(mp3_enc_fun.lib_handle);
        mp3_enc_fun.lib_handle = TD_NULL;
        return ret;
    }

    (td_void)memcpy_s(&g_mp3enc_func, sizeof(g_mp3enc_func), &mp3_enc_fun, sizeof(mp3_enc_fun));

    return TD_SUCCESS;
}
#else
td_s32 mp3_aenc_init_lib(void)
{
    g_mp3enc_func.mp3enc_create_call = ot_mp3enc_create;
    g_mp3enc_func.mp3enc_process_frame_call = ot_mp3enc_process_frame;
    g_mp3enc_func.mp3enc_destroy_call = ot_mp3enc_destroy;
    return TD_SUCCESS;
}
#endif

#ifdef OT_MP3_USE_DYNAMIC_LOAD
static td_s32 mp3_dec_lib_dlsym(mp3dec_fun *mp3_dec_fun)
{
    td_s32 ret;

    ret = ot_audio_dlsym((td_void **)&(mp3_dec_fun->mp3dec_init_decoder_call),
        mp3_dec_fun->lib_handle, "ot_mp3dec_init_decoder");
    if (ret != TD_SUCCESS) {
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_dec_fun->mp3dec_decode_call),
        mp3_dec_fun->lib_handle, "ot_mp3dec_frame");
    if (ret != TD_SUCCESS) {
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_dec_fun->mp3dec_get_last_frame_info_call),
        mp3_dec_fun->lib_handle, "ot_mp3dec_get_last_frame_info");
    if (ret != TD_SUCCESS) {
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_dec_fun-> mp3dec_find_sync_header_call),
        mp3_dec_fun->lib_handle, "ot_mp3dec_find_sync_header");
    if (ret != TD_SUCCESS) {
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    ret = ot_audio_dlsym((td_void **)&(mp3_dec_fun->mp3dec_free_decoder_call),
        mp3_dec_fun->lib_handle, "ot_mp3dec_free_decoder");
    if (ret != TD_SUCCESS) {
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}

static td_s32 mp3_adec_init_lib(void)
{
    td_s32 ret;
    mp3dec_fun mp3_dec_fun;

    ret = ot_audio_dlopen(&(mp3_dec_fun.lib_handle), MP3_DEC_LIB_NAME);
    if (ret != TD_SUCCESS) {
        printf("[func]:%s [line]:%d [info]:%s\n", __FUNCTION__, __LINE__, "load adec lib fail!\n");
        return OT_ERR_ADEC_NOT_SUPPORT;
    }

    ret = mp3_dec_lib_dlsym(&mp3_dec_fun);
    if (ret != TD_SUCCESS) {
        printf("[func]:%s [line]:%d [info]:%s\n", __FUNCTION__, __LINE__, "find symbol error!\n");
        ot_audio_dlclose(mp3_dec_fun.lib_handle);
        mp3_dec_fun.lib_handle = TD_NULL;
        return ret;
    }

    (td_void)memcpy_s(&g_mp3dec_func, sizeof(g_mp3dec_func), &mp3_dec_fun, sizeof(mp3_dec_fun));

    return TD_SUCCESS;
}
#else
static td_s32 mp3_adec_init_lib(void)
{
    g_mp3dec_func.mp3dec_init_decoder_call = ot_mp3dec_init_decoder;
    g_mp3dec_func.mp3dec_decode_call = ot_mp3dec_frame;
    g_mp3dec_func.mp3dec_get_last_frame_info_call = ot_mp3dec_get_last_frame_info;
    g_mp3dec_func.mp3dec_find_sync_header_call = ot_mp3dec_find_sync_header;
    g_mp3dec_func.mp3dec_free_decoder_call = ot_mp3dec_free_decoder;
    return TD_SUCCESS;
}
#endif

td_s32 ss_mpi_aenc_mp3_init(td_void)
{
    td_s32 ret;
    ot_aenc_encoder mp3_encoder;

    ret = mp3_aenc_init_lib();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mp3_encoder.type = OT_PT_MP3;
    ret = snprintf_s(mp3_encoder.name, sizeof(mp3_encoder.name), sizeof(mp3_encoder.name) - 1, "mp3");
    if (ret <= EOK) {
        mp3_err_trace("snprintf_s failed with 0x%x.\n", ret);
        return ret;
    }
    mp3_encoder.max_frame_len = OT_MP3ENC_MAX_OUTBUF_SIZE;
    mp3_encoder.func_open_encoder = open_mp3_encdoer;
    mp3_encoder.func_enc_frame = encode_mp3_frame;
    mp3_encoder.func_close_encoder = close_mp3_encoder;

    return ss_mpi_aenc_register_encoder(&g_mp3_encoder_handle, &mp3_encoder);
}

td_s32 ss_mpi_adec_mp3_init(td_void)
{
    td_s32 ret;
    ot_adec_decoder mp3_decoder;

    ret = mp3_adec_init_lib();
    if (ret != TD_SUCCESS) {
        return ret;
    }

    mp3_decoder.type = OT_PT_MP3;
    ret = snprintf_s(mp3_decoder.name, sizeof(mp3_decoder.name), sizeof(mp3_decoder.name) - 1, "mp3");
    if (ret <= EOK) {
        mp3_err_trace("snprintf_s failed with 0x%x.\n", ret);
        return OT_ERR_ADEC_NOT_SUPPORT;
    }
    mp3_decoder.func_open_decoder = open_mp3_decoder;
    mp3_decoder.func_dec_frame = decode_mp3_frame;
    mp3_decoder.func_get_frame_info = get_mp3_frm_info;
    mp3_decoder.func_close_decoder = close_mp3_decoder;
    mp3_decoder.func_reset_decoder = reset_mp3_decoder;
    return ss_mpi_adec_register_decoder(&g_mp3_decoder_handle, (const ot_adec_decoder *)&mp3_decoder);
}

td_s32 ss_mpi_aenc_mp3_deinit(td_void)
{
    return ss_mpi_aenc_unregister_encoder(g_mp3_encoder_handle);
}

td_s32 ss_mpi_adec_mp3_deinit(td_void)
{
    return ss_mpi_adec_unregister_decoder(g_mp3_decoder_handle);
}
