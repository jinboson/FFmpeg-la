/*
 * Copyright (C) 2020 Loongson Technology Co. Ltd.
 * Contributed by Gu Xiwei(guxiwei-hf@loongson.cn)
 * All rights reserved.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "swscale_mips.h"
#include "libavutil/mips/generic_macros_msa.h"

void ff_hscale_8_to_15_msa(SwsContext *c, int16_t *dst, int dstW,
                           const uint8_t *src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize)
{
    int i;
    if (filterSize == 8) {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };

            src0 = LD_V(v8i16, src + filterPos[i]);
            filter0 = LD_V(v8i16, filter + (i << 3));
            src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
            out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
            out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
            val += (__msa_copy_s_w((v4i32)out0, 0) +
                    __msa_copy_s_w((v4i32)out0, 2));
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else if (filterSize == 4) {
        int len = dstW & (~1);

        for (i = 0; i < len; i += 2) {
            v8i16 src1, src2, src3;
            v8i16 filter0;
            v4i32 out0;
            int val1 = 0;
            int val2 = 0;
            v16i8 zero = {0};

            src1 = LD_V(v8i16, src + filterPos[i]);
            src2 = LD_V(v8i16, src + filterPos[i + 1]);
            filter0 = LD_V(v8i16, filter + (i << 2));
            src1 = (v8i16)__msa_ilvr_b(zero, (v16i8)src1);
            src2 = (v8i16)__msa_ilvr_b(zero, (v16i8)src2);
            src3 = (v8i16)__msa_ilvr_d((v2i64)src2, (v2i64)src1);
            out0 = (v4i32)__msa_dotp_s_w(src3, filter0);
            val1 = __msa_copy_s_w(out0, 0) + __msa_copy_s_w(out0, 1);
            val2 = __msa_copy_s_w(out0, 2) + __msa_copy_s_w(out0, 3);
            dst[i] = FFMIN(val1 >> 7, (1 << 15) - 1);
            dst[i + 1] = FFMIN(val2 >> 7, (1 << 15) - 1);
        }
        if (i < dstW) {
           int val = 0;
           uint8_t *srcPos = src + filterPos[i];
           int16_t *filterStart = filter + filterSize * i;

           for (int j = 0; j < 4; j++) {
               val += ((int)srcPos[j]) * filterStart[j];
           }
           dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else if (filterSize > 8) {
        int len  = filterSize >> 3;
        int part = len << 3;

        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;
            int j, val = 0;

            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + (j << 3));
                filter0 = LD_V(v8i16, filterStart + (j << 3));
                src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
                out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
                out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
                val += (__msa_copy_s_w((v4i32)out0, 0) +
                        __msa_copy_s_w((v4i32)out0, 2));
            }
            for (j = part; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;

            for (int j = 0; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    }
}

void ff_yuv2planeX_8_msa(const int16_t *filter, int filterSize,
                         const int16_t **src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset)
{
    int i;
    int len  = dstW >> 3;
    int part = len << 3;
    for (i = 0; i < len; i++) {
        int j;
        v8i16 src0, flags;
        v4i32 src_l, src_r, filter0;
        v4i32 val_r = { dither[(i * 8 + 0 + offset) & 7] << 12,
                        dither[(i * 8 + 1 + offset) & 7] << 12,
                        dither[(i * 8 + 2 + offset) & 7] << 12,
                        dither[(i * 8 + 3 + offset) & 7] << 12 };
        v4i32 val_l = { dither[(i * 8 + 4 + offset) & 7] << 12,
                        dither[(i * 8 + 5 + offset) & 7] << 12,
                        dither[(i * 8 + 6 + offset) & 7] << 12,
                        dither[(i * 8 + 7 + offset) & 7] << 12 };
        v8i16 zero = { 0 };

        for (j = 0; j < filterSize; j++) {
            src0 = LD_V(v8i16, &src[j][i * 8]);
            filter0 = __msa_fill_w(filter[j]);
            flags = __msa_clt_s_h(src0, zero);
            ILVRL_H2_SW(flags, src0, src_r, src_l);
            val_r += src_r * filter0;
            val_l += src_l * filter0;
        }
        val_r >>= 19;
        val_l >>= 19;
        CLIP_SW2_0_255(val_r, val_l);
        src0 = __msa_pckev_h((v8i16)val_l, (v8i16)val_r);
        src0 = (v8i16)__msa_pckev_b((v16i8)src0, (v16i8)src0);
        SD(__msa_copy_s_d((v2i64)src0, 0), dest + i * 8);
    }
    for (i = part; i < dstW; i++) {
        int val = dither[(i + offset) & 7] << 12;
        int j;
        for (j = 0; j< filterSize; j++)
            val += src[j][i] * filter[j];

        dest[i] = av_clip_uint8(val >> 19);
    }
}

static av_always_inline void yuv2bgrx32_write_full(SwsContext *c,
                                                   uint8_t *dest, int Y, int U, int V)
{
    int R, G, B;

    Y -= c->yuv2rgb_y_offset;
    Y *= c->yuv2rgb_y_coeff;
    Y += 1 << 21;
    R = (unsigned)Y + V*c->yuv2rgb_v2r_coeff;
    G = (unsigned)Y + V*c->yuv2rgb_v2g_coeff + U*c->yuv2rgb_u2g_coeff;
    B = (unsigned)Y +                          U*c->yuv2rgb_u2b_coeff;
    if ((R | G | B) & 0xC0000000) {
        R = av_clip_uintp2(R, 30);
        G = av_clip_uintp2(G, 30);
        B = av_clip_uintp2(B, 30);
    }
    dest[0] = B >> 22;
    dest[1] = G >> 22;
    dest[2] = R >> 22;
    dest[3] = 255;
}

static av_always_inline void yuv2bgrx32_write_full_msa(uint8_t *dest, v4i32 Y, v4i32 U, v4i32 V,
                                                       int v2r, v4i32 uv_coe, int u2b)
{
    int R, G, B;
    v4i32 vr = V * v2r;
    v4i32 ub = U * u2b;

    v4i32 uv_r = __msa_ilvr_w(V,U);
    v4i32 uv_l = __msa_ilvl_w(V,U);
    uv_r = (v4i32)__msa_dotp_s_d(uv_r, uv_coe);
    uv_l = (v4i32)__msa_dotp_s_d(uv_l, uv_coe);

    for (int i = 0; i < 2; i++) {
        uint32_t y =(unsigned)Y[i];
        int n = i * 2;
        int j = i + 2;
        int m = i * 4;

        R = y + vr[i];
        G = y + uv_r[n];
        B = y + ub[i];
        if ((R | G | B) & 0xC0000000) {
            R = av_clip_uintp2(R, 30);
            G = av_clip_uintp2(G, 30);
            B = av_clip_uintp2(B, 30);
        }
        dest[m + 0] = B >> 22;
        dest[m + 1] = G >> 22;
        dest[m + 2] = R >> 22;
        dest[m + 3] = 255;

        y = (unsigned)Y[j];
        R = y + vr[j];
        G = y + uv_l[n];
        B = y + ub[j];
        if ((R | G | B) & 0xC0000000) {
            R = av_clip_uintp2(R, 30);
            G = av_clip_uintp2(G, 30);
            B = av_clip_uintp2(B, 30);
        }
        m += 8;
        dest[m + 0] = B >> 22;
        dest[m + 1] = G >> 22;
        dest[m + 2] = R >> 22;
        dest[m + 3] = 255;
    }
}

void yuv2bgrx32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y)
{
    int i;
    int step = 4;
    int err[4] = {0};
    int len = dstW & (~0x07);
    int t = 1 << 9;
    int n = 1 << 21;
    int u = t - (128 << 19);
    int v2r, u2b, y_off, y_coe;
    v4i32 v2g = __msa_fill_w(c->yuv2rgb_v2g_coeff);
    v4i32 u2g = __msa_fill_w(c->yuv2rgb_u2g_coeff);
    v4i32 uv_coeff = __msa_ilvr_w(v2g, u2g);

    v2r   = c->yuv2rgb_v2r_coeff;
    u2b   = c->yuv2rgb_u2b_coeff;
    y_off = c->yuv2rgb_y_offset;
    y_coe = c->yuv2rgb_y_coeff;

    for (i = 0; i < len; i += 8) {
        int j;
        v8i16 l_src, u_src, v_src;
        v4i32 lum_r, lum_l, usrc_r, usrc_l, vsrc_r, vsrc_l;
        v4i32 y_r, y_l, u_r, u_l, v_r, v_l, temp;

        y_r   = __msa_fill_w(t);
        y_l   = y_r;
        u_r   = __msa_fill_w(u);
        u_l   = u_r;
        v_r   = u_r;
        v_l   = u_r;

        for (j = 0; j < lumFilterSize; j++) {
            temp  = __msa_fill_w(lumFilter[j]);
            l_src = LD_V(v8i16, (lumSrc[j] + i));
            UNPCK_SH_SW(l_src, lum_r, lum_l);
            y_l   = __msa_maddv_w(lum_l, temp, y_l);
            y_r   = __msa_maddv_w(lum_r, temp, y_r);
        }
        for (j = 0; j < chrFilterSize; j++) {
            temp   = __msa_fill_w(chrFilter[j]);
            u_src  = LD_V(v8i16, (chrUSrc[j] + i));
            v_src  = LD_V(v8i16, (chrVSrc[j] + i));
            UNPCK_SH_SW(u_src, usrc_r, usrc_l);
            UNPCK_SH_SW(v_src, vsrc_r, vsrc_l);
            u_l   = __msa_maddv_w(usrc_l, temp, u_l);
            u_r   = __msa_maddv_w(usrc_r, temp, u_r);
            v_l   = __msa_maddv_w(vsrc_l, temp, v_l);
            v_r   = __msa_maddv_w(vsrc_r, temp, v_r);
        }
        y_r = __msa_srai_w(y_r, 10);
        y_l = __msa_srai_w(y_l, 10);
        u_r = __msa_srai_w(u_r, 10);
        u_l = __msa_srai_w(u_l, 10);
        v_r = __msa_srai_w(v_r, 10);
        v_l = __msa_srai_w(v_l, 10);
        y_r -= y_off;
        y_l -= y_off;
        y_r *= y_coe;
        y_l *= y_coe;
        y_r += n;
        y_l += n;

        yuv2bgrx32_write_full_msa(dest, y_r, u_r, v_r, v2r, uv_coeff, u2b);
        dest += 16;
        yuv2bgrx32_write_full_msa(dest, y_l, u_l, v_l, v2r, uv_coeff, u2b);
        dest += 16;
    }
    for (i; i < dstW; i++) {
        int j;
        int Y = 1<<9;
        int U = (Y)-(128 << 19);
        int V = U;

        for (j = 0; j < lumFilterSize; j++) {
            Y += lumSrc[j][i] * lumFilter[j];
        }
        for (j = 0; j < chrFilterSize; j++) {
            U += chrUSrc[j][i] * chrFilter[j];
            V += chrVSrc[j][i] * chrFilter[j];
        }
        Y >>= 10;
        U >>= 10;
        V >>= 10;

        yuv2bgrx32_write_full(c, dest, Y, U, V);
        dest += step;
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}

