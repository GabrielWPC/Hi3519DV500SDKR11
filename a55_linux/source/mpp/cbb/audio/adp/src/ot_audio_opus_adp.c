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
#include "ot_audio_opus_adp.h"

#define DUMP_PATH_NAME_MAX_BYTES 256
#define DUMP_MAX_TIMES 10000
#define AENC_OPUS_MAX_FRAME_SIZE_NUM 6

#define opus_err_trace(fmt, ...)  printf("[func]:%s [line]:%d [error]:"fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef struct {
    ot_audio_sample_rate sample_rate;
    td_s32 frame_size[AENC_OPUS_MAX_FRAME_SIZE_NUM];
} aenc_opus_frame_size;

static aenc_opus_frame_size g_aenc_opus_frame_size[] = {
    {OT_AUDIO_SAMPLE_RATE_8000, {20, 40, 80, 160, 320, 480}},
    {OT_AUDIO_SAMPLE_RATE_12000, {30, 60, 120, 240, 480, 720}},
    {OT_AUDIO_SAMPLE_RATE_16000, {40, 80, 160, 320, 640, 960}},
    {OT_AUDIO_SAMPLE_RATE_24000, {60, 120, 240, 480, 960, 1440}},
    {OT_AUDIO_SAMPLE_RATE_48000, {120, 240, 480, 960, 1920, 2880}}
};

typedef struct {
    ot_opus_encoder *state;
    ot_aenc_attr_opus attr;
} aenc_opus_encoder;

typedef struct {
    ot_opus_decoder *state;
    ot_adec_attr_opus attr;
} adec_opus_decoder;

static td_s32 g_opus_encoder_handle = -1;
static td_s32 g_opus_decoder_handle = -1;

#ifdef DUMP_OPUSENC
static FILE *g_in_enc_fd = TD_NULL;
static FILE *g_out_enc_fd = TD_NULL;
static td_s32 g_cnt_aenc = 100000;
#endif

#ifdef DUMP_OPUSDEC
static FILE *g_in_dec_fd = TD_NULL;
static FILE *g_out_dec_fd = TD_NULL;
static td_s32 g_cnt_adec = 100000;
#endif

static td_s32 aenc_check_opus_attr(const ot_aenc_attr_opus *opus_attr)
{
    if (opus_attr->bit_rate < OT_OPUS_BPS_16K || opus_attr->bit_rate > OT_OPUS_BPS_320K) {
        opus_err_trace("invalid bit_rate(%d), range is [%d, %d]", opus_attr->bit_rate,
            OT_OPUS_BPS_16K, OT_OPUS_BPS_320K);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (opus_attr->app != OT_OPUS_APPLICATION_VOIP && opus_attr->app != OT_OPUS_APPLICATION_AUDIO &&
        opus_attr->app != OT_OPUS_APPLICATION_RESTRICTED_LOWDELAY) {
        opus_err_trace("invalid opus app(%d), range is [%d, %d].", opus_attr->app,
            OT_OPUS_APPLICATION_VOIP, OT_OPUS_APPLICATION_RESTRICTED_LOWDELAY);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (opus_attr->bit_width != OT_AUDIO_BIT_WIDTH_16) {
        opus_err_trace("invalid bit_width(%d), only support 16bit.", opus_attr->bit_width);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (opus_attr->sample_rate != OT_AUDIO_SAMPLE_RATE_8000 && opus_attr->sample_rate != OT_AUDIO_SAMPLE_RATE_12000 &&
        opus_attr->sample_rate != OT_AUDIO_SAMPLE_RATE_16000 && opus_attr->sample_rate != OT_AUDIO_SAMPLE_RATE_24000 &&
        opus_attr->sample_rate != OT_AUDIO_SAMPLE_RATE_48000) {
        opus_err_trace("invalid samplerate(%d), range is [%d, %d]", opus_attr->sample_rate,
            OT_AUDIO_SAMPLE_RATE_8000, OT_AUDIO_SAMPLE_RATE_48000);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (opus_attr->snd_mode >= OT_AUDIO_SOUND_MODE_BUTT) {
        opus_err_trace("invalid sound_mode(%d), it should be %d or %d.", opus_attr->snd_mode,
            OT_AUDIO_SOUND_MODE_MONO, OT_AUDIO_SOUND_MODE_STEREO);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

#ifdef DUMP_OPUSENC
static td_void init_encoder_dump_file(td_void)
{
    td_s32 ret;
    td_char name_in[DUMP_PATH_NAME_MAX_BYTES] = {'\0'};
    td_char name_out[DUMP_PATH_NAME_MAX_BYTES] = {'\0'};

    ret = snprintf_s(name_in, DUMP_PATH_NAME_MAX_BYTES, DUMP_PATH_NAME_MAX_BYTES - 1, "opusenc_sin_t%d_t%d.raw", 1, 1);
    if (ret < 0) {
        opus_err_trace("name_in snprintf_s failed with 0x%x.", ret);
        return;
    }

    ret = snprintf_s(name_out, DUMP_PATH_NAME_MAX_BYTES, DUMP_PATH_NAME_MAX_BYTES - 1,
                     "opusenc_sout_t%d_t%d.opus", 1, 1);
    if (ret < 0) {
        opus_err_trace("name_out snprintf_s failed with 0x%x.", ret);
        return;
    }

    g_in_enc_fd = fopen(name_in, "w+");
    if (g_in_enc_fd == TD_NULL) {
        opus_err_trace("failed to open %s.", name_in);
    }

    g_out_enc_fd = fopen(name_out, "w+");
    if (g_out_enc_fd == TD_NULL) {
        opus_err_trace("failed to open %s.", name_out);
    }

    g_cnt_aenc = DUMP_MAX_TIMES;
}
#endif

td_s32 open_opus_encoder(td_void *encoder_attr, td_void **encoder)
{
    td_s32 ret;
    ot_opusenc_config config;
    aenc_opus_encoder *encoder_tmp = TD_NULL;
    ot_aenc_attr_opus *attr = TD_NULL;

    if (encoder_attr == TD_NULL || encoder == TD_NULL) {
        return OT_ERR_AENC_NULL_PTR;
    }

    /* check attribute of encoder */
    attr = (ot_aenc_attr_opus *)encoder_attr;
    ret = aenc_check_opus_attr(attr);
    if (ret != TD_SUCCESS) {
        opus_err_trace("check opus attr failed with 0x%x.", ret);
        return ret;
    }

    /* allocate memory for encoder */
    encoder_tmp = (aenc_opus_encoder *)malloc(sizeof(aenc_opus_encoder));
    if (encoder_tmp == TD_NULL) {
        opus_err_trace("malloc encoder_tmp failed.");
        return OT_ERR_AENC_NO_MEM;
    }
    ret = memset_s(encoder_tmp, sizeof(aenc_opus_encoder), 0, sizeof(aenc_opus_encoder));
    if (ret != EOK) {
        free(encoder_tmp);
        opus_err_trace("memset_s failed with 0x%x\n", ret);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    config.channels = 1 << attr->snd_mode;

    config.bit_rate = attr->bit_rate;
    config.sample_rate = attr->sample_rate;
    config.app = (ot_opusenc_application)attr->app;

    /* create encoder */
    encoder_tmp->state = ot_opusenc_create(&config);
    if (encoder_tmp->state == TD_NULL) {
        free(encoder_tmp);
        opus_err_trace("failed to create opus encoder.");
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    ret = memcpy_s(&encoder_tmp->attr, sizeof(ot_aenc_attr_opus), attr, sizeof(ot_aenc_attr_opus));
    if (ret != EOK) {
        ot_opusenc_destroy(encoder_tmp->state);
        free(encoder_tmp);
        opus_err_trace("memcpy_s failed with 0x%x\n", ret);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    *encoder = (td_void *)encoder_tmp;

#ifdef DUMP_OPUSENC
    init_encoder_dump_file();
#endif

    return TD_SUCCESS;
}

static td_s32 opus_aenc_check_buf_info(const ot_aenc_buf_info *buf_info)
{
    if (buf_info == TD_NULL) {
        return OT_ERR_AENC_NULL_PTR;
    }

    if (buf_info->out_buf == TD_NULL || buf_info->out_valid_len == TD_NULL) {
        return OT_ERR_AENC_NULL_PTR;
    }

    if (buf_info->out_max_len == 0 || buf_info->out_max_len > OT_MAX_AUDIO_STREAM_LEN) {
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 opus_aenc_check_frame_size(td_u32 sample_rate, td_s32 samples)
{
    size_t i, j;
    td_bool match = TD_FALSE;

    for (i = 0; i < sizeof(g_aenc_opus_frame_size) / sizeof(g_aenc_opus_frame_size[0]); i++) {
        if (g_aenc_opus_frame_size[i].sample_rate != sample_rate) {
            continue;
        }
        for (j = 0; j < AENC_OPUS_MAX_FRAME_SIZE_NUM; j++) {
            if (g_aenc_opus_frame_size[i].frame_size[j] == samples) {
                match = TD_TRUE;
                break;
            }
        }
        break;
    }

    if (!match) {
        opus_err_trace("frame_size must be 2.5, 5, 10, 20, 40 or 60 ms, samples(%u) sample_rate(%u).\n",
            samples, sample_rate);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 opus_aenc_check_frame(aenc_opus_encoder *encoder, const ot_audio_frame *frame)
{
    td_s32 ret;
    td_u32 samples;
    td_u32 sample_rate;

    if (encoder->attr.snd_mode != frame->snd_mode) {
        opus_err_trace("invalid sound_mode(%d), it should match attr sound_mode(%d).",
            encoder->attr.snd_mode, frame->snd_mode);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    /* whether the bit width of frame and encoder is match */
    if (frame->bit_width != encoder->attr.bit_width) {
        opus_err_trace("invalid bit_width(%d), it should match frame bit_width(%d).",
            encoder->attr.bit_width, frame->bit_width);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    /* calculate point number */
    samples = frame->len / (frame->bit_width + 1);
    sample_rate = encoder->attr.sample_rate;

    /* if frame sample larger than protocol sample, reject to receive, or buffer will be overflow */
    ret = opus_aenc_check_frame_size(sample_rate, samples);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return TD_SUCCESS;
}

static td_void interleaved(td_s16 *dest, td_s16 *src_left, td_s16 *src_right, td_s32 sample, ot_audio_snd_mode snd_mode)
{
    td_s32 i;

    if (snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
        for (i = sample - 1; i >= 0; i--) {
            dest[2 * i] = *(src_left + i);      /* 2: stereo */
            dest[2 * i + 1] = *(src_right + i); /* 2: stereo */
        }
    } else { /* if inbuf is momo, copy left to right */
        for (i = sample - 1; i >= 0; i--) {
            dest[i] = *(src_left + i);
        }
    }
}

td_s32 encode_opus_frame(td_void *encoder, const ot_audio_frame *frame, ot_aenc_buf_info *buf_info)
{
    td_s32 ret, samples;
    aenc_opus_encoder *encoder_tmp = TD_NULL;
    static td_s16 data_tmp[OT_OPUSENC_MAX_IN_SAMPLES * OT_OPUSENC_MAX_CHANNEL_NUM];

    if (encoder == TD_NULL || frame == TD_NULL) {
        return OT_ERR_AENC_NULL_PTR;
    }

    ret = opus_aenc_check_buf_info(buf_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    encoder_tmp = (aenc_opus_encoder *)encoder;

    ret = opus_aenc_check_frame(encoder_tmp, frame);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    samples = frame->len / (frame->bit_width + 1);
    /* OPUS encoder need interleaved data,here change LLLRRR to LRLRLR. */
    interleaved(data_tmp, (td_s16 *)frame->virt_addr[0], (td_s16 *)frame->virt_addr[1], samples,
        encoder_tmp->attr.snd_mode);

#ifdef DUMP_OPUSENC
    if (g_cnt_aenc > 0) {
        td_s32 size;
        if (encoder_tmp->attr.snd_mode == OT_AUDIO_SOUND_MODE_STEREO) {
            size = samples * sizeof(td_s16) * 2; /* 2: stereo */
        } else {
            size = samples * sizeof(td_s16);
        }
        fwrite((td_u8 *)data_tmp, 1, size, g_in_enc_fd);
    }
#endif

    ret = ot_opusenc_process_frame(encoder_tmp->state, data_tmp, samples, buf_info->out_buf, buf_info->out_valid_len);
    if (ret != TD_SUCCESS) {
        opus_err_trace("encode opus failed with 0x%x.", ret);
        return OT_ERR_AENC_ENCODER_ERR;
    }

#ifdef DUMP_OPUSENC
    if (g_cnt_aenc > 0) {
        fwrite((td_u8 *)buf_info->out_buf, 1, *(buf_info->out_valid_len), g_out_enc_fd);
        g_cnt_aenc--;
    }
#endif

    return TD_SUCCESS;
}

td_s32 close_opus_encoder(td_void *encoder)
{
    aenc_opus_encoder *encoder_tmp = TD_NULL;

    if (encoder == TD_NULL) {
        return OT_ERR_AENC_NULL_PTR;
    }
    encoder_tmp = (aenc_opus_encoder *)encoder;

    ot_opusenc_destroy(encoder_tmp->state);

    free(encoder_tmp);

#ifdef DUMP_OPUSENC
    if (g_in_enc_fd) {
        fclose(g_in_enc_fd);
        g_in_enc_fd = TD_NULL;
    }

    if (g_out_enc_fd) {
        fclose(g_out_enc_fd);
        g_out_enc_fd = TD_NULL;
    }
#endif

    return TD_SUCCESS;
}

#ifdef DUMP_OPUSDEC
static td_void init_decoder_dump_file(td_void)
{
    td_s32 ret;
    td_char name_in[DUMP_PATH_NAME_MAX_BYTES] = {'\0'};
    td_char name_out[DUMP_PATH_NAME_MAX_BYTES] = {'\0'};
    td_char name_out_l[DUMP_PATH_NAME_MAX_BYTES] = {'\0'};

    ret = snprintf_s(name_in, DUMP_PATH_NAME_MAX_BYTES, DUMP_PATH_NAME_MAX_BYTES - 1, "opusdec_sin_t%d.opus", 1);
    if (ret < 0) {
        opus_err_trace("name_in snprintf_s failed with 0x%x.", ret);
        return;
    }

    ret = snprintf_s(name_out, DUMP_PATH_NAME_MAX_BYTES, DUMP_PATH_NAME_MAX_BYTES - 1, "opusdec_sout_t%d.raw", 1);
    if (ret < 0) {
        opus_err_trace("name_out snprintf_s failed with 0x%x.", ret);
        return;
    }

    ret = snprintf_s(name_out_l, DUMP_PATH_NAME_MAX_BYTES, DUMP_PATH_NAME_MAX_BYTES - 1, "opusdec_sout_t%d_l.raw", 1);
    if (ret < 0) {
        opus_err_trace("name_out_1 snprintf_s failed with 0x%x.", ret);
        return;
    }

    g_in_dec_fd = fopen(name_in, "w+");
    if (g_in_dec_fd == TD_NULL) {
        opus_err_trace("failed to open %s.", name_in);
    }

    g_out_dec_fd = fopen(name_out, "w+");
    if (g_out_dec_fd == TD_NULL) {
        opus_err_trace("failed to open %s.", name_out);
    }

    g_cnt_adec = DUMP_MAX_TIMES;
}
#endif

static td_void adec_opus_init_config(ot_adec_attr_opus *attr, ot_opusdec_config *config)
{
    config->sample_rate = attr->sample_rate;
    if (attr->snd_mode == OT_AUDIO_SOUND_MODE_MONO) {
        config->channels = 1;
    } else {
        config->channels = 2; /* 2: stereo */
    }
}

td_s32 open_opus_decoder(td_void *decoder_attr, td_void **decoder)
{
    td_s32 ret;
    ot_opusdec_config config;
    adec_opus_decoder *decoder_tmp = TD_NULL;
    ot_adec_attr_opus *attr = (ot_adec_attr_opus *)decoder_attr;

    if (decoder_attr == TD_NULL || decoder == TD_NULL) {
        return OT_ERR_ADEC_NULL_PTR;
    }

    if (attr->sample_rate != OT_AUDIO_SAMPLE_RATE_8000 && attr->sample_rate != OT_AUDIO_SAMPLE_RATE_12000 &&
        attr->sample_rate != OT_AUDIO_SAMPLE_RATE_16000 && attr->sample_rate != OT_AUDIO_SAMPLE_RATE_24000 &&
        attr->sample_rate != OT_AUDIO_SAMPLE_RATE_48000) {
        opus_err_trace("invalid samplerate(%d), range is [%d, %d].", attr->sample_rate,
            OT_AUDIO_SAMPLE_RATE_8000, OT_AUDIO_SAMPLE_RATE_48000);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    if (attr->snd_mode >= OT_AUDIO_SOUND_MODE_BUTT) {
        opus_err_trace("invalid sound_mode(%d), it must be less than %d.", attr->snd_mode, OT_AUDIO_SOUND_MODE_BUTT);
        return OT_ERR_AENC_ILLEGAL_PARAM;
    }

    adec_opus_init_config(attr, &config);

    /* allocate memory for decoder_tmp */
    decoder_tmp = (adec_opus_decoder *)malloc(sizeof(adec_opus_decoder));
    if (decoder_tmp == TD_NULL) {
        opus_err_trace("malloc decoder_tmp failed.");
        return OT_ERR_ADEC_NO_MEM;
    }
    ret = memset_s(decoder_tmp, sizeof(adec_opus_decoder), 0, sizeof(adec_opus_decoder));
    if (ret != EOK) {
        free(decoder_tmp);
        opus_err_trace("memset_s failed with 0x%x.", ret);
        return OT_ERR_ADEC_ILLEGAL_PARAM;
    }

    /* create decoder */
    decoder_tmp->state = ot_opusdec_create(&config);
    if (decoder_tmp->state == TD_NULL) {
        free(decoder_tmp);
        opus_err_trace("failed to create opus decoder.");
        return OT_ERR_ADEC_DECODER_ERR;
    }

    ret = memcpy_s(&decoder_tmp->attr, sizeof(ot_adec_attr_opus), attr, sizeof(ot_adec_attr_opus));
    if (ret != EOK) {
        ot_opusdec_destroy(decoder_tmp->state);
        free(decoder_tmp);
        opus_err_trace("memcpy_s failed with 0x%x.", ret);
        return OT_ERR_ADEC_ILLEGAL_PARAM;
    }

    *decoder = (td_void *)decoder_tmp;

#ifdef DUMP_OPUSDEC
    init_decoder_dump_file();
#endif

    return TD_SUCCESS;
}

static td_s32 opus_adec_check_buf_info(const ot_adec_buf_info *buf_info)
{
    if (buf_info == TD_NULL) {
        return OT_ERR_ADEC_NULL_PTR;
    }

    if (buf_info->in_buf == TD_NULL || buf_info->in_left_byte == TD_NULL || buf_info->out_buf == TD_NULL ||
        buf_info->out_valid_len == TD_NULL) {
        return OT_ERR_ADEC_NULL_PTR;
    }

    if (buf_info->out_max_len == 0 || buf_info->out_max_len > OT_MAX_AUDIO_STREAM_LEN) {
        return OT_ERR_ADEC_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 decode_opus_frame(td_void *decoder, ot_adec_buf_info *buf_info, td_u32 *out_chns)
{
    td_s32 ret;
    td_u32 out_bytes_per_chn;
    adec_opus_decoder *decoder_tmp = TD_NULL;

    if (decoder == TD_NULL || out_chns == TD_NULL) {
        return OT_ERR_ADEC_NULL_PTR;
    }

    ret = opus_adec_check_buf_info(buf_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    decoder_tmp = (adec_opus_decoder *)decoder;

#ifdef DUMP_OPUSDEC
    td_u32 pre_in_bytes_left = *(buf_info->in_left_byte);
    td_u8 *pre_in_buf = *(buf_info->in_buf);
#endif
    /* Notes: in_buf will updated */
    ret = ot_opusdec_process_frame(decoder_tmp->state, buf_info->in_buf, buf_info->in_left_byte,
        (td_s16 *)(td_void *)buf_info->out_buf, &out_bytes_per_chn);
    if (ret != TD_SUCCESS) {
        opus_err_trace("opus decode failed with 0x%x.", ret);
        return OT_ERR_ADEC_DECODER_ERR;
    }

    if (decoder_tmp->attr.snd_mode == OT_AUDIO_SOUND_MODE_MONO) {
        *out_chns = 1;
    } else {
        *out_chns = 2; /* 2: stereo */
    }

    *(buf_info->out_valid_len) = out_bytes_per_chn;

#ifdef DUMP_OPUSDEC
    if (g_cnt_adec > 0) {
        fwrite(pre_in_buf, 1, (pre_in_bytes_left - *(buf_info->in_left_byte)), g_in_dec_fd);
        fwrite((td_u8 *)buf_info->out_buf, 1, out_bytes_per_chn * (*out_chns), g_out_dec_fd);
        g_cnt_adec--;
    }
#endif

    return TD_SUCCESS;
}

td_s32 get_opus_frame_info(td_void *decoder, td_void *info)
{
    return TD_SUCCESS;
}

td_s32 close_opus_decoder(td_void *decoder)
{
    adec_opus_decoder *decoder_tmp = TD_NULL;

    if (decoder == TD_NULL) {
        return OT_ERR_ADEC_NULL_PTR;
    }

    decoder_tmp = (adec_opus_decoder *)decoder;

    ot_opusdec_destroy(decoder_tmp->state);

    free(decoder_tmp);

#ifdef DUMP_OPUSDEC
    if (g_in_dec_fd) {
        fclose(g_in_dec_fd);
        g_in_dec_fd = TD_NULL;
    }

    if (g_out_dec_fd) {
        fclose(g_out_dec_fd);
        g_out_dec_fd = TD_NULL;
    }
#endif

    return TD_SUCCESS;
}

td_s32 ss_mpi_aenc_opus_init(td_void)
{
    td_s32 handle, ret;
    ot_aenc_encoder opus;

    opus.type = OT_PT_OPUS;
    ret = snprintf_s(opus.name, sizeof(opus.name), sizeof(opus.name) - 1, "opus");
    if (ret <= EOK) {
        opus_err_trace("snprintf_s failed with 0x%x.", ret);
        return TD_FAILURE;
    }
    opus.max_frame_len = OT_MAX_OPUS_MAINBUF_SIZE;
    opus.func_open_encoder = open_opus_encoder;
    opus.func_enc_frame = encode_opus_frame;
    opus.func_close_encoder = close_opus_encoder;

    ret = ss_mpi_aenc_register_encoder(&handle, &opus);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    g_opus_encoder_handle = handle;

    return TD_SUCCESS;
}

td_s32 ss_mpi_adec_opus_init(td_void)
{
    td_s32 handle, ret;
    ot_adec_decoder opus;

    opus.type = OT_PT_OPUS;
    ret = snprintf_s(opus.name, sizeof(opus.name), sizeof(opus.name) - 1, "opus");
    if (ret <= EOK) {
        opus_err_trace("snprintf_s failed with 0x%x.", ret);
        return TD_FAILURE;
    }
    opus.func_open_decoder = open_opus_decoder;
    opus.func_dec_frame = decode_opus_frame;
    opus.func_get_frame_info = get_opus_frame_info;
    opus.func_close_decoder = close_opus_decoder;

    ret = ss_mpi_adec_register_decoder(&handle, &opus);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    g_opus_decoder_handle = handle;

    return TD_SUCCESS;
}

td_s32 ss_mpi_aenc_opus_deinit(td_void)
{
    return ss_mpi_aenc_unregister_encoder(g_opus_encoder_handle);
}

td_s32 ss_mpi_adec_opus_deinit(td_void)
{
    return ss_mpi_adec_unregister_decoder(g_opus_decoder_handle);
}
