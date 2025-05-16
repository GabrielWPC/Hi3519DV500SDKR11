/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "ss_mpi_vo.h"
#include "ss_mpi_vo_dev.h"

typedef enum {
    OT_VO_CONFIG_TYPE_LAYER = 0, /* config layer csc */
    OT_VO_CONFIG_TYPE_VGA = 1,   /* config vga param */
    OT_VO_CONFIG_TYPE_HDMI = 2,  /* config hdmi param */
    OT_VO_CONFIG_TYPE_RGB = 3,   /* config rgb param */
    OT_VO_CONFIG_TYPE_MIPI = 4,  /* config mipi param */
    OT_VO_CONFIG_TYPE_HDMI1 = 5, /* config hdmi1 param */
    OT_VO_CONFIG_TYPE_BUTT,
} ot_vo_config_type;

typedef enum {
    OT_VO_CONFIG_PARAM_CMD = 0,               /* config param cmd */
    OT_VO_CONFIG_PARAM_TYPE = 1,              /* config param type */
    OT_VO_CONFIG_PARAM_ID = 2,                /* config param id */
    OT_VO_CONFIG_PARAM_CSC_MATRIX = 3,        /* config param csc matrix */
    OT_VO_CONFIG_PARAM_LUMA = 4,              /* config param csc luma */
    OT_VO_CONFIG_PARAM_CONTRAST = 5,          /* config param csc contrast */
    OT_VO_CONFIG_PARAM_HUE = 6,               /* config param csc hue */
    OT_VO_CONFIG_PARAM_SATURATION = 7,        /* config param csc saturation */
    OT_VO_CONFIG_PARAM_EX_CSC_EN = 8,         /* config param csc ex_csc_en */
    OT_VO_CONFIG_PARAM_GAIN = 9,              /* config param vga gain */
    OT_VO_CONFIG_PARAM_SHARPEN_STRENGTH = 10, /* config param vga sharpen strength */
    OT_VO_CONFIG_PARAM_BUTT,
} ot_vo_config_param;

#define VO_CONFIG_DEF_ARGC_NUM (OT_VO_CONFIG_PARAM_EX_CSC_EN + 1)        /* default 9 argc */
#define VO_CONFIG_VGA_ARGC_NUM (OT_VO_CONFIG_PARAM_SHARPEN_STRENGTH + 1) /* vga param have 11 argc */

#define VO_DUMP_ARG_NUMBER_BASE 10  /* 10: decimal scale */

#define vo_config_check_return(express, name)                                                    \
    do {                                                                                         \
        td_s32 _ret;                                                                             \
        _ret = express;                                                                          \
        if (_ret != TD_SUCCESS) {                                                                \
            printf("%s failed at %s: LINE: %d with %#x!\n", name, __FUNCTION__, __LINE__, _ret); \
            return _ret;                                                                         \
        }                                                                                        \
    } while (0)

static td_void usage(td_void)
{
    printf("\nnotice: this tool only can be used for TESTING !!!\n");
    printf("notice: video layer csc_matrix[0,15], gfx layer csc_matrix[24,27] ");
    printf("vga or rgb csc_matrix[16,23], hdmi or mipi csc_matrix[0,23].\n");
    printf("\tluma/contrast/hue/saturation[0,100], ex_csc_en[0,1], gain[0, 64), sharpen_strength[0, 255].\n");
    printf("notice: layer config usage: ./vo_config [0] [layer] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en].\n");
    printf("\tsample: ./vo_config 0 0  9 50 50 50 50 0\n");
    printf("\tvga config usage: ./vo_config [1] [dev] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en] [gain] [sharpen_strength].\n");
    printf("\tsample: ./vo_config 1 0 17 50 50 50 50 0 10 128\n");
    printf("\thdmi config usage: ./vo_config [2] [dev] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en].\n");
    printf("\tsample: ./vo_config 2 0  9 50 50 50 50 0\n");
    printf("\trgb config usage: ./vo_config [3] [dev] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en].\n");
    printf("\tsample: ./vo_config 3 0 17 50 50 50 50 0\n");
    printf("\tmipi config usage: ./vo_config [4] [dev] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en].\n");
    printf("\tsample: ./vo_config 4 0  9 50 50 50 50 0\n");
    printf("\thdmi1 config usage: ./vo_config [2] [dev] [csc_matrix] [luma] [contrast] [hue] ");
    printf("[saturation] [ex_csc_en].\n");
    printf("\tsample: ./vo_config 5 0  9 50 50 50 50 0\n");
}

static td_s32 vo_get_input_argc_num(ot_vo_config_type config_type)
{
    td_s32 argc = VO_CONFIG_DEF_ARGC_NUM;

    if (config_type == OT_VO_CONFIG_TYPE_VGA) {
        argc = VO_CONFIG_VGA_ARGC_NUM;
    }

    return argc;
}

static td_void vo_get_input_csc_param(char *argv[], td_s32 *id, ot_vo_csc *csc)
{
    *id = (td_s32)strtol(argv[OT_VO_CONFIG_PARAM_ID], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);

    csc->csc_matrix = (ot_vo_csc_matrix)strtoul(argv[OT_VO_CONFIG_PARAM_CSC_MATRIX], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    csc->luma = (td_u32)strtoul(argv[OT_VO_CONFIG_PARAM_LUMA], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    csc->contrast = (td_u32)strtoul(argv[OT_VO_CONFIG_PARAM_CONTRAST], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    csc->hue = (td_u32)strtoul(argv[OT_VO_CONFIG_PARAM_HUE], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    csc->saturation = (td_u32)strtoul(argv[OT_VO_CONFIG_PARAM_SATURATION], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    csc->ex_csc_en = (td_bool)strtoul(argv[OT_VO_CONFIG_PARAM_EX_CSC_EN], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
}

static td_void vo_print_csc_config(const ot_vo_csc *csc)
{
    printf("csc_matrix %u, luma %u, contrast %u, hue %u, saturation %u, ex_csc_en %u", csc->csc_matrix, csc->luma,
        csc->contrast, csc->hue, csc->saturation, csc->ex_csc_en);
}

static td_s32 vo_do_layer_config(char *argv[])
{
    ot_vo_layer layer = 0;
    ot_vo_csc csc = {0};

    vo_get_input_csc_param(argv, &layer, &csc);
    vo_config_check_return(ss_mpi_vo_set_layer_csc(layer, &csc), "ss_mpi_vo_set_layer_csc");
    vo_config_check_return(ss_mpi_vo_get_layer_csc(layer, &csc), "ss_mpi_vo_get_layer_csc");
    printf("layer config layer_id %d ", layer);
    vo_print_csc_config(&csc);
    printf("\n");
    return TD_SUCCESS;
}

static td_s32 vo_do_vga_config(char *argv[])
{
    ot_vo_dev dev = 0;
    ot_vo_vga_param vga_param = {0};

    vo_get_input_csc_param(argv, &dev, &(vga_param.csc));
    vga_param.gain = (td_u32)strtoul(argv[OT_VO_CONFIG_PARAM_GAIN], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    vga_param.sharpen_strength = (td_s32)strtol(argv[OT_VO_CONFIG_PARAM_SHARPEN_STRENGTH], TD_NULL,
        VO_DUMP_ARG_NUMBER_BASE);
    vo_config_check_return(ss_mpi_vo_set_vga_param(dev, &vga_param), "ss_mpi_vo_set_vga_param");
    vo_config_check_return(ss_mpi_vo_get_vga_param(dev, &vga_param), "ss_mpi_vo_get_vga_param");
    printf("vga config dev_id %d ", dev);
    vo_print_csc_config(&(vga_param.csc));
    printf(" gain %u, sharpen_strength %d\n", vga_param.gain, vga_param.sharpen_strength);
    return TD_SUCCESS;
}

static td_s32 vo_do_hdmi_config(char *argv[])
{
    ot_vo_dev dev = 0;
    ot_vo_hdmi_param hdmi_param = {0};

    vo_get_input_csc_param(argv, &dev, &(hdmi_param.csc));
    vo_config_check_return(ss_mpi_vo_set_hdmi_param(dev, &hdmi_param), "ss_mpi_vo_set_hdmi_param");
    vo_config_check_return(ss_mpi_vo_get_hdmi_param(dev, &hdmi_param), "ss_mpi_vo_get_hdmi_param");
    printf("hdmi config dev_id %d ", dev);
    vo_print_csc_config(&(hdmi_param.csc));
    printf("\n");
    return TD_SUCCESS;
}

static td_s32 vo_do_rgb_config(char *argv[])
{
    ot_vo_dev dev = 0;
    ot_vo_rgb_param rgb_param = {0};
    ot_vo_rgb_param old_rgb_param = {0};

    vo_get_input_csc_param(argv, &dev, &(rgb_param.csc));
    vo_config_check_return(ss_mpi_vo_get_rgb_param(dev, &old_rgb_param), "ss_mpi_vo_get_rgb_param");
    rgb_param.rgb_inverted_en = old_rgb_param.rgb_inverted_en;
    rgb_param.bit_inverted_en = old_rgb_param.bit_inverted_en;
    rgb_param.rgb_dummy_out_en = old_rgb_param.rgb_dummy_out_en;
    rgb_param.rgb_dummy_order = old_rgb_param.rgb_dummy_order;
    rgb_param.gamma_info = old_rgb_param.gamma_info;

    vo_config_check_return(ss_mpi_vo_set_rgb_param(dev, &rgb_param), "ss_mpi_vo_set_rgb_param");
    vo_config_check_return(ss_mpi_vo_get_rgb_param(dev, &rgb_param), "ss_mpi_vo_get_rgb_param");
    printf("rgb config dev_id %d ", dev);
    vo_print_csc_config(&(rgb_param.csc));
    printf("\n");
    return TD_SUCCESS;
}

static td_s32 vo_do_mipi_config(char *argv[])
{
    ot_vo_dev dev = 0;
    ot_vo_mipi_param mipi_param = {0};
    ot_vo_mipi_param old_mipi_param = {0};

    vo_get_input_csc_param(argv, &dev, &(mipi_param.csc));
    vo_config_check_return(ss_mpi_vo_get_mipi_param(dev, &old_mipi_param), "ss_mpi_vo_get_mipi_param");
    mipi_param.gamma_info = old_mipi_param.gamma_info;

    vo_config_check_return(ss_mpi_vo_set_mipi_param(dev, &mipi_param), "ss_mpi_vo_set_mipi_param");
    vo_config_check_return(ss_mpi_vo_get_mipi_param(dev, &mipi_param), "ss_mpi_vo_get_mipi_param");
    printf("mipi config dev_id %d ", dev);
    vo_print_csc_config(&(mipi_param.csc));
    printf("\n");
    return TD_SUCCESS;
}

static td_s32 vo_do_hdmi1_config(char *argv[])
{
    ot_vo_dev dev = 0;
    ot_vo_hdmi_param hdmi1_param = {0};

    vo_get_input_csc_param(argv, &dev, &(hdmi1_param.csc));
    vo_config_check_return(ss_mpi_vo_set_hdmi1_param(dev, &hdmi1_param), "ss_mpi_vo_set_hdmi1_param");
    vo_config_check_return(ss_mpi_vo_get_hdmi1_param(dev, &hdmi1_param), "ss_mpi_vo_get_hdmi1_param");
    printf("hdmi1 config dev_id %d ", dev);
    vo_print_csc_config(&(hdmi1_param.csc));
    printf("\n");
    return TD_SUCCESS;
}

static td_s32 vo_do_config(char *argv[])
{
    ot_vo_config_type config_type = (ot_vo_config_type)strtoul(argv[OT_VO_CONFIG_PARAM_TYPE], TD_NULL,
        VO_DUMP_ARG_NUMBER_BASE);
                                                               
    switch (config_type) {
        case OT_VO_CONFIG_TYPE_LAYER:
            return vo_do_layer_config(argv);

        case OT_VO_CONFIG_TYPE_VGA:
            return vo_do_vga_config(argv);

        case OT_VO_CONFIG_TYPE_HDMI:
            return vo_do_hdmi_config(argv);

        case OT_VO_CONFIG_TYPE_RGB:
            return vo_do_rgb_config(argv);

        case OT_VO_CONFIG_TYPE_MIPI:
            return vo_do_mipi_config(argv);

        case OT_VO_CONFIG_TYPE_HDMI1:
            return vo_do_hdmi1_config(argv);

        default:
            printf("config type %d is invalid!\n", config_type);
            return OT_ERR_VO_ILLEGAL_PARAM;
    }
}

static td_s32 vo_check_config_param(int argc, char *argv[])
{
    ot_vo_config_type config_type;
    int argc_num;

    if (argc <= 1) {
        printf("Failure: too little parameters!\n");
        usage();
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((strncmp(argv[1], "-h", 2)) == 0) { /* 2 chars */
        usage();
        return TD_SUCCESS;
    }

    config_type = (ot_vo_config_type)strtoul(argv[OT_VO_CONFIG_PARAM_TYPE], TD_NULL, VO_DUMP_ARG_NUMBER_BASE);
    if (config_type >= OT_VO_CONFIG_TYPE_BUTT) {
        printf("Failure: config type %d is invalid!\n", config_type);
        usage();
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    argc_num = vo_get_input_argc_num(config_type);
    if (argc != argc_num) {
        printf("Failure: config type %d should have %d param, current input %d param!\n", config_type, argc_num, argc);
        usage();
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

#ifdef __LITEOS__
td_s32 vo_config(int argc, char *argv[])
#else
td_s32 main(int argc, char *argv[])
#endif
{
    td_s32 ret;

    printf("\nnotice: this tool only can be used for TESTING !!!\n");
#ifndef __LITEOS__
    printf("\tTo see more usage, please enter: ./vo_config -h\n\n");
#else
    printf("\tTo see more usage, please enter: vo_config -h\n\n");
#endif

    ret = vo_check_config_param(argc, argv);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_do_config(argv);
}
