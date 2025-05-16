/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heif_format.h"
#include "securec.h"

#define HEIF_URL_LEN 512

static int32_t heif_muxer_create_process(const char *out_url, heif_handle *handle, heif_format_profile  profile)
{
    heif_config config;
    config.file_desc.file_type = HEIF_FILE_TYPE_URL;
    if (strcpy_s(config.file_desc.input.url, HEIF_MAX_URL_PATH_LEN, out_url) != EOK) {
        printf("strcpy_s failed \n");
        return -1;
    }
    config.config_type = HEIF_CONFIG_MUXER;
    config.muxer_config.is_grid = false;
    config.muxer_config.row_image_num = 1;
    config.muxer_config.column_image_num = 1;
    config.muxer_config.format_profile = profile;
    return heif_create(handle, &config);
}

static int32_t read_frame_and_fill_item(FILE *fp, heif_image_item *item, uint8_t **data_buffer)
{
    (void)fseek(fp, 0, SEEK_END);
    int32_t length = (int32_t)ftell(fp);
    if (length <= 0) {
        printf("input file length <= 0\n");
        return -1;
    }
    (void)fseek(fp, 0, SEEK_SET);

    *data_buffer = (uint8_t *)malloc(length);
    if (*data_buffer == NULL) {
        printf("malloc failed\n");
        return -1;
    }

    if (fread(*data_buffer, 1, length, fp) != (size_t)length) {
        printf("fread failed\n");
        return -1;
    }

    item->item_id = 0;
    item->timestamp = -1;
    item->data = *data_buffer;
    item->length = (uint32_t)length;
    item->key_frame = true;
    return 0;
}

static heif_format_profile get_muxer_profile(void)
{
    heif_format_profile profile = HEIF_PROFILE_HEIC;
    char cmd[0x10];
    char *ret = NULL;
    printf("please input format: heic or avci\n");
    while ((ret = fgets(cmd, 0x10, stdin)) != NULL) {  /* 10: use array length */
        cmd[0xf] = '\0';  /* 10: end of string */
        if (strncmp(cmd, "heic", 0x4) == 0) {
            break;
        } else if (strncmp(cmd, "avci", 0x4) == 0) {
            profile = HEIF_PROFILE_AVCI;
            break;
        } else {
            printf("please input format: heic or avci\n");
        }
    }
    return profile;
}

static int32_t muxer_process(const char *in_url, const char *out_url)
{
    printf("muxer_process in_url:%s out_url:%s\n", in_url, out_url);
    char path_in[PATH_MAX];
    if (realpath(in_url, path_in) == NULL) {
        printf("realpath failed\n");
        return -1;
    }
    FILE *fp = fopen(path_in, "r+");
    if (fp == NULL) {
        printf("fopen audio es failed \n");
        return -1;
    }

    heif_format_profile profile = get_muxer_profile();
    heif_handle handle = NULL;
    int32_t ret = heif_muxer_create_process(out_url, &handle, profile);
    if (ret != 0) {
        printf("heif_muxer_create_process failed\n");
        if (fclose(fp) != 0) {
            printf("fclose failed\n");
        }
        return -1;
    }

    uint8_t *data_buffer = NULL;
    heif_image_item item;
    ret = read_frame_and_fill_item(fp, &item, &data_buffer);
    if (ret != 0) {
        printf("read_frame_and_fill_item failed, ret:%d\n", ret);
        goto FAIL;
    }

    ret = heif_write_master_image(handle, 0, &item, 1);
    if (ret != 0) {
        printf("heif_write_master_image failed, ret:%d\n", ret);
        goto FAIL;
    }
FAIL:
    if (data_buffer != NULL) {
        free(data_buffer);
    }
    heif_destroy(handle);
    if (fclose(fp) != 0) {
        printf("fclose failed\n");
    }
    printf("heif muxer test end, ret:%d\n", ret);
    return ret;
}

static int32_t save_demuxer_data(const char *out_url, const heif_extend_info *exten_info, const heif_image_item *item,
    uint32_t item_cnt)
{
    (void)exten_info;
    if (item_cnt == 0 || item->length == 0 || item->data == NULL) {
        printf("[%s:%d] item invalid\n", __FUNCTION__, __LINE__);
        return -1;
    }

    FILE *fout = fopen(out_url, "w+");
    if (fout == NULL) {
        printf("fopen audio es failed \n");
        return -1;
    }

    if (fwrite(item->data, 1, item->length, fout) != item->length) {
        printf("[%s:%d] fwrite data fail\n", __FUNCTION__, __LINE__);
        (void)fclose(fout);
        return -1;
    }
    (void)fclose(fout);
    return 0;
}

static void save_thumbnail_image(uint8_t *data, uint32_t len)
{
    FILE *fout = fopen("./temp_thumbnail.jpg", "wb");
    if (fout == NULL) {
        printf("fopen ./temp_thumbnail.jpg failed \n");
        return;
    }

    if (fwrite(data, 1, len, fout) != len) {
        printf("[%s:%d] fwrite data fail\n", __FUNCTION__, __LINE__);
        (void)fclose(fout);
        return;
    }
    (void)fflush(fout);
    (void)fclose(fout);
}

static void check_get_thumbnail(heif_handle handle, const heif_file_info *file_info)
{
    if (file_info->track_count <= 1) {
        printf("no thumbnail \n");
        return;
    }
    bool has_thumbnail = false;
    for (uint32_t i = 0; i < file_info->track_count; i++) {
        if (file_info->track[i].type == HEIF_TRACK_TYPE_THUMBNAL) {
            has_thumbnail = true;
            break;
        }
    }
    if (!has_thumbnail) {
        printf("no thumbnail \n");
        return;
    }
    heif_extend_info info;
    heif_thumbnail_info thmb_info;
    info.type = HEIF_EXTEND_TYPE_THUMBNAIL;
    info.data = (void *)&thmb_info;
    info.len = (uint32_t)sizeof(heif_thumbnail_info);
    int32_t ret = heif_get_extend_info(handle, 0, 0, &info);
    if (ret != 0) {
        printf("heif_get_extend_info failed \n");
        return;
    }
    printf("thmbInfo width:%u, height:%u \n", thmb_info.width, thmb_info.height);
    save_thumbnail_image(thmb_info.data, thmb_info.len);
}

static int32_t demuxer_process(const char *in_url, const char *out_url)
{
    printf("demuxer in_url:%s out_url:%s\n", in_url, out_url);
    heif_config config = { 0 };
    config.file_desc.file_type = HEIF_FILE_TYPE_URL;
    if (strcpy_s(config.file_desc.input.url, HEIF_MAX_URL_PATH_LEN, in_url) != EOK) {
        printf("strcpy_s failed \n");
        return -1;
    }

    config.config_type = HEIF_CONFIG_DEMUXER;
    heif_handle handle = NULL;
    if (heif_create(&handle, &config) != 0) {
        return -1;
    }

    heif_file_info file_info = { 0 };
    int32_t ret = heif_get_file_info(handle, &file_info);
    if (ret != 0) {
        printf("[%s:%d]heif_get_file_info failed ret:%d\n", __func__, __LINE__, ret);
        heif_destroy(handle);
        return ret;
    }
    heif_extend_info extend_info = { HEIF_EXTEND_TYPE_HEVC_EXT, NULL, 0 };
    if (file_info.profile == HEIF_PROFILE_AVCI) {
        extend_info.type = HEIF_EXTEND_TYPE_AVC_EXT;
    }
    ret = heif_get_extend_info(handle, 0, 0, &extend_info);
    if (ret != 0) {
        printf("[%s:%d] heif_get_extend_info fail\n", __FUNCTION__, __LINE__);
    }

    heif_image_item item = { 0 };
    uint32_t item_count = 1;
    ret = heif_get_master_image(handle, file_info.track[0].track_id, &item, &item_count);
    if (ret != 0) {
        printf("[%s:%d] heif_get_master_image fail\n", __FUNCTION__, __LINE__);
    } else {
        ret = save_demuxer_data(out_url, &extend_info, &item, item_count);
    }
    check_get_thumbnail(handle, &file_info);
    heif_destroy(handle);
    return ret;
}

static void print_help(void)
{
    printf("usage: ./sample_heif (muxer or demuxer or combine) inputfile outputfile \n");
    printf("eg: muxer h265 frame to heif file\n");
    printf("./sample_heif muxer ./data/test_01.265 ./out.heic\n");
    printf("input heic for input format\n");
    printf("\neg: muxer h264 frame to heif file\n");
    printf("./sample_heif muxer ./data/test_01.264 ./out.avci\n");
    printf("input avci for input format\n\n");
    printf("\neg: demuxer for a heic file:\n");
    printf("./sample_heif demuxer ./data/test_01.heic ./out.265\n");
    printf("./sample_heif demuxer ./data/test_01.avci ./out.264\n");
    printf("\neg: muxer and demuxer:\n");
    printf("./sample_heif combine ./data/test_01.265 ./out.265 ./temp.heic\n");
    printf("input heic for input format\n");
    printf("./sample_heif combine ./data/test_01.264 ./out.264 ./temp.avci\n");
    printf("input avci for input format\n");
}

int main(int argc, char* argv[])
{
    char url_in[HEIF_URL_LEN] = { 0 };
    char url_out[HEIF_URL_LEN] = { 0 };
    char url_tmp[HEIF_URL_LEN] = { 0 };
    if (argc < 0x4) {
        print_help();
        return 0;
    }

    if (strlen(argv[0x2]) >= HEIF_URL_LEN || strcpy_s(url_in, HEIF_URL_LEN, argv[0x2]) != EOK) {
        printf("strcpy_s failed \n");
        return -1;
    }
    if (strlen(argv[0x3]) >= HEIF_URL_LEN || strcpy_s(url_out, HEIF_URL_LEN, argv[0x3]) != EOK) {
        printf("strcpy_s failed \n");
        return -1;
    }

    if (strcmp(argv[1], "muxer") == 0) {
        if (muxer_process(url_in, url_out) != 0) {
            printf("muxer heif file faild\n");
        }
    } else if (strcmp(argv[1], "demuxer") == 0) {
        if (demuxer_process(url_in, url_out) != 0) {
            printf("demuxer heif file faild\n");
        }
    } else if (strcmp(argv[1], "combine") == 0) {
        if (argc < 0x5) {
            print_help();
            return 0;
        }
        if (strlen(argv[0x4]) >= HEIF_URL_LEN || strcpy_s(url_tmp, HEIF_URL_LEN, argv[0x4]) != EOK) {
            printf("strcpy_s failed \n");
            return -1;
        }
        if (muxer_process(url_in, url_tmp) != 0) {
            printf("muxer heif file faild\n");
            return -1;
        }
        if (demuxer_process(url_tmp, url_out) != 0) {
            printf("demuxer heif file faild\n");
        }
    }
    return 0;
}
