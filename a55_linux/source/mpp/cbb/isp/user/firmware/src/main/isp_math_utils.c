/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "isp_math_utils.h"
#include "ot_math.h"
#include "isp_main.h"

td_u8 sqrt16(td_u32 arg)
{
    const td_u8 mask = 128;
    td_u8 res  = 0;
    td_u8 i;
    const td_u8 loops = 8;

    for (i = 0; i < loops; i++) {
        if ((res + (mask >> i)) * (res + (mask >> i)) <= arg) {
            res = res + (mask >> i);
        }
    }

    return res;
}

td_u8 log16(td_u32 arg)
{
    td_u8 k;
    td_u8 res = 0;
    const td_u8 loops = 16;

    for (k = 0; k < loops; k++) {
        if (arg > (td_u32)(1 << k)) {
            res = (k << 4) + ((arg << 4) / (1 << (k))); /* left shift 4bits */
        }
    }
    return res;
}

td_u16 sqrt32(td_u32 arg)
{
    td_u32 mask = (td_u32)1 << 15;  /* left shift 15bits */
    td_u16 res  = 0;
    td_u32 i;
    const td_u8 loops = 16;

    for (i = 0; i < loops; i++) {
        if ((res + (mask >> i)) * (res + (mask >> i)) <= arg) {
            res = res + (mask >> i);
        }
    }

    /* rounding */
    if ((td_u32)(res * res + res) < arg) {
        ++res;
    }

    return res;
}

td_void get_linear_inter_info(ot_vi_pipe vi_pipe, linear_inter_info *inter_info)
{
    isp_usr_ctx *isp_ctx = TD_NULL;

    isp_get_ctx(vi_pipe, isp_ctx);

    inter_info->cur_iso  = isp_ctx->linkage.iso;
    inter_info->idx_high = get_iso_index(inter_info->cur_iso);
    inter_info->idx_low  = MAX2((td_s8)inter_info->idx_high - 1, 0);
    inter_info->iso_high = get_iso(inter_info->idx_high);
    inter_info->iso_low  = get_iso(inter_info->idx_low);
}

td_s32 linear_inter(td_s32 v, td_s32 x0, td_s32 y0, td_s32 x1, td_s32 y1)
{
    td_s32 res;

    if (v <= x0) {
        return y0;
    }
    if (v >= x1) {
        return y1;
    }

    res = (td_s64)(y1 - y0) * (td_s64)(v - x0) / (x1 - x0) + y0;
    return res;
}

td_s32 calc_mul_coef(td_s32 x0, td_s32 y0, td_s32 x1, td_s32 y1, td_u8 sft)
{
    if (x0 == x1) {
        return 0;
    } else {
        return (signed_left_shift((y1 - y0), sft) / (x1 - x0));
    }
}

static td_u8 leading_one_position(const td_u32 in)
{
    td_u8 pos = 0;
    td_u32 val = in;
    if (val >= (1 << 16)) {  /* compare with 1 << 16 */
        val >>= 16;          /* right shift 16bits */
        pos += 16;           /* added to 16 */
    }
    if (val >= (1 << 8)) { /* compare with 1 << 8 */
        val >>= 8;         /* right shift 8bits */
        pos += 8;          /* added to 8 */
    }
    if (val >= (1 << 4)) {  /* compare with 1 << 4 */
        val >>= 4;          /* right shift 4bits */
        pos += 4;           /* added to 4 */
    }
    if (val >= (1 << 2)) {  /* compare with 1 << 2 */
        val >>= 2;          /* right shift 2bits */
        pos += 2;           /* added to 2 */
    }
    if (val >= (1 << 1)) {  /* compare with 1 << 1 */
        pos += 1;           /* added to 1 */
    }
    return pos;
}

/* y := log2(x) */
/* input:  integer: val */
/* output: fixed point x.y */
/* y: out_precision */
td_u32 log2_int_to_fixed(const td_u32 val, const td_u8 out_precision, const td_u8 shift_out)
{
    td_u8  bit_shift;
    int i, pos;
    td_u32 a;
    td_u32 b;
    td_u32 in = val;
    td_u32 result = 0;
    const unsigned char precision = out_precision;

    if (val == 0) {
        return 0;
    }
    /* integral part */
    pos = leading_one_position(val);
    /* fractional part */
    if (pos <= 15) { /* const value 15 */
        bit_shift = 15 - pos; /* const value 15 */
        a = in << bit_shift;
    } else {
        bit_shift = pos - 15; /* const value 15 */
        a = in >> bit_shift;
    }

    for (i = 0; i < precision; ++i) {
        b = a * a;
        if (b & (1U << 31)) {  /* left shift 31bits */
            result = (result << 1) + 1;
            a = b >> 16;       /* right shift 16bits */
        } else {
            result = (result << 1);
            a = b >> 15;       /* right shift 15bits */
        }
    }
    return (td_u32)((((td_u32)signed_left_shift(pos, precision) + result) << shift_out) |
                    ((a & 0x7fff) >> (15 - shift_out)));  /* right shift (15 - shift_out)bits */
}

td_u32 math_log2(const td_u32 val, const td_u8 out_precision, const td_u8 shift_out)
{
    int i;
    int pos;
    td_u32 a;
    td_u32 b;
    td_u32 in = val;
    td_u32 result = 0;
    td_u8  bit_shift;
    const unsigned char precision = out_precision;

    if (val == 0) {
        return 0;
    }

    /* integral part */
    pos = leading_one_position(val);
    /* fractional part */
    if (pos <= 15) { /* const value 15 */
        bit_shift = 15 - pos; /* const value 15 */
        a = in << bit_shift;
    } else {
        bit_shift = pos - 15; /* const value 15 */
        a = in >> bit_shift;
    }

    for (i = 0; i < precision; ++i) {
        b = a * a;
        if (b & (1U << 31)) {   /* left shift 31bits */
            result = (result << 1) + 1;
            a = b >> 16;   /* right shift 16bits */
        } else {
            result = (result << 1);
            a = b >> 15;   /* right shift 15bits */
        }
    }

    return (td_u32)(((td_u32)signed_left_shift(pos, precision) + result) << shift_out);
}
#define POW2_LUT_NUM 33
static td_u32 g_pow2_lut[POW2_LUT_NUM] = {
    1073741824, 1097253708, 1121280436, 1145833280, 1170923762, 1196563654, 1222764986, 1249540052,
    1276901417, 1304861917, 1333434672, 1362633090, 1392470869, 1422962010, 1454120821, 1485961921,
    1518500250, 1551751076, 1585730000, 1620452965, 1655936265, 1692196547, 1729250827, 1767116489,
    1805811301, 1845353420, 1885761398, 1927054196, 1969251188, 2012372174, 2056437387, 2101467502,
    2147483647
};

td_u32 math_exp2(td_u32 val, const unsigned char shift_in, const unsigned char shift_out)
{
    unsigned int fract_part = (val & ((1 << shift_in) - 1));
    unsigned int int_part = val >> shift_in;
    if (shift_in <= 5) {  /* const value 5 */
        unsigned int lut_index = fract_part << (5 - shift_in); /* left shift (5-shift_in)bits */
        return g_pow2_lut[lut_index] >> (30 - shift_out - int_part); /* right shift (30 - shift_out - int_part)bits */
    } else {
        unsigned int lut_index = fract_part >> (shift_in - 5); /* right shift (shift_in-5)bits */
        unsigned int lut_fract = fract_part & ((1 << (shift_in - 5)) - 1); /* left shift (shift_in-5)bits */
        unsigned int a = g_pow2_lut[lut_index];
        unsigned int b = g_pow2_lut[lut_index + 1];
        unsigned int res = ((unsigned long long)(b - a) * lut_fract) >> (shift_in - 5); /* right shift (shift_in-5) */
        res = (res + a) >> (30 - shift_out - int_part); /* right shift (30 - shift_out - int_part)bits */
        return res;
    }
}

/* linear equation solving */
/* y1 : a * x1 + b */
/* y2 : a * x2 + b */
/* y1 - (a * x1) := y2 - (a * x2) */
/* y1 - y2 := (a * x1) - (a * x2) */
/* a : (y1 - y2) / (x1 - x2) */
/* b : y1 - (a * x1) */
/* result is coefficient "a" in fixed format   x.a_fraction_size */
/* y1, y2, x1, x2 - real value format */
td_s32 solving_lin_equation_a(td_s32 y1, td_s32 y2, td_s32 x1, td_s32 x2, td_u16 a_fraction_size)
{
    return ((y1 - y2) * (1 << a_fraction_size)) / div_0_to_1(x1 - x2);
}

/*    result is coefficient "b" in fixed format   x.a_fraction_size
 *    y1, x1 - real value format
 *    "a" in fixed format   x.a_fraction_size
 */
td_s32 solving_lin_equation_b(td_s32 y1, td_s32 a, td_s32 x1, td_u16 a_fraction_size)
{
    return (y1 * (1 << a_fraction_size)) - (a * x1);
}

/*    y := a / b
 *    a: fixed xxx.fraction_size
 *    b: fixed xxx.fraction_size
 */
td_u32 div_fixed(td_u32 a, td_u32 b, const td_u16 fraction_size)
{
    return (a << fraction_size) / div_0_to_1(b);
}
/*  nth root finding y = x^0.45
 *  not a precise equation - for speed issue
 *  result is coefficient "y" in fixed format   xxx.fraction_size
 */
td_s32 solving_nth_root_045(td_s32 x, const td_u16 fraction_size)
{
    td_s32 value = (td_s32)signed_left_shift(x, fraction_size);
    td_s32 y = (1 << fraction_size) + (value / 4) - (value / 8); /* const value 4 and 8 */
    return y;
}

/* transition matching. */
/* lut_int  - LUT for search,                    values in real format */
/* lut_out  - LUT for transition matching,        values in real format */
/* lut_size - size of LUT,                        value in real format */
/* value    - value for search,                value in fixed format size. xxx.value_fraction_size */
/* ret      - output value,                    value in real format */
/* a0             ax(value)          a1 */
/* |--------------|-----------------|    - lut_int */
/* b0             bx(ret)            b1 */
/* |--------------|-----------------|    - lut_out */
/* ax : (a1 - a0) * x + a0 */
/* bx : (b1 - b0) * x + b0 */
/* x : (ax - a0) / (a1 - a0) */
/* bx : ((b1 - b0) * (ax - a0) / (a1 - a0)) + b0 */
td_u32 transition(td_u32 *lut_in, td_u32 *lut_out, td_u32 lut_size, td_u32 value, td_u32 value_fraction_size)
{
    td_u32 ret;
    td_u32 i;
    td_u32 fast_search = value >> value_fraction_size;
    td_s32 a0, a1, b0, b1;
    td_s32 b10, ax0, a10, result;

    if (fast_search < lut_in[0]) {
        return lut_out[0];
    }

    if (fast_search >= lut_in[lut_size - 1]) {
        return lut_out[lut_size - 1];
    }

    /* transition matching. */
    /* searching */
    a0 = lut_in[0];
    a1 = lut_in[0];
    b0 = lut_out[0];
    b1 = lut_out[0];
    for (i = 1; i < lut_size; i++) {
        if (lut_in[i] > fast_search) {
            a0 = lut_in[i - 1];
            a1 = lut_in[i];
            b0 = lut_out[i - 1];
            b1 = lut_out[i];
            break;
        }
    }
    /* interpolation */
    b10 = b1 - b0;
    ax0 = value - (td_s32)signed_left_shift(a0, value_fraction_size);
    a10 = a1 - a0;
    if (a10 == 0) {
        result = 0;
    } else {
        result = b10 * ax0 / a10;
    }
    ret = (td_u32)(result + (td_s32)signed_left_shift(b0, value_fraction_size));
    ret = ret >> value_fraction_size;

    return ret;
}

td_s64 signed_right_shift(td_s64 value, td_u8 bit_shift)
{
    td_u64 value_pos;
    td_u64 tmp;
    value_pos = (td_u64)value;
    if (value < 0) {
        value_pos = value_pos >> bit_shift;
        tmp = (((0x1ULL << (bit_shift)) - 1ULL)) << (64 - bit_shift); /* left shift (64 - bit_shift)bits */
        value_pos = value_pos | tmp;
        return (td_s64)value_pos;
    } else {
        return (td_s64)(value_pos >> bit_shift);
    }
}

td_s64 signed_left_shift(td_s64 value, td_u8 bit_shift)
{
    td_u64 abs_value = ABS(value);

    if (value < 0) {
        return (td_s64)((0x1ULL << 63) | ((~(abs_value << bit_shift)) + 1)); /* left shift 63bits */
    } else {
        return (td_s64)(abs_value << bit_shift);
    }
}

void *isp_malloc(unsigned long size)
{
    if (size == 0) {
        return TD_NULL;
    }

    return malloc(size);
}

void memset_u16(td_u16 *vir, td_u16 temp, td_u32 size)
{
    td_u32 i;
    for (i = 0; i < size; i++) {
        *(vir + i) = temp;
    }
}

void memset_u32(td_u32 *vir, td_u32 temp, td_u32 size)
{
    td_u32 i;
    for (i = 0; i < size; i++) {
        *(vir + i) = temp;
    }
}

td_u16 complement_to_direct_u16(td_s16 value)
{
    td_u16 result;

    if (value >= 0) {
        result = value;
    } else {
        result = -value;
        result |= (1 << 15);  /* left shift 15bits */
    }

    return result;
}

td_s16 direct_to_complement_u16(td_u16 value)
{
    td_s16 result;

    result = value & (~(1 << 15));  /* left shift 15bits */

    if (value & (1 << 15)) {   /* left shift 15bits */
        result = -result;
    }

    return result;
}

const td_u32 g_iso_lut[OT_ISP_AUTO_ISO_NUM] = {
    100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800, 409600, 819200, 1638400, 3276800
};

td_u8 get_iso_index(td_u32 iso)
{
    td_u8 index;

    for (index = 0; index < OT_ISP_AUTO_ISO_NUM - 1; index++) {
        if (iso <= g_iso_lut[index]) {
            break;
        }
    }

    return index;
}

td_u32 get_iso(td_u8 index)
{
    return g_iso_lut[index];
}

const td_u32 *get_iso_lut(td_void)
{
    return &g_iso_lut[0];
}

