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

/*Copy from libswscale/output.c*/
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
        int m = i * 4;
        int j = i + 2;

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

/*Copy from libswscale/output.c*/
static av_always_inline void
yuv2rgb_write(uint8_t *_dest, int i, int Y1, int Y2,
              unsigned A1, unsigned A2,
              const void *_r, const void *_g, const void *_b, int y,
              enum AVPixelFormat target, int hasAlpha)
{
    if (target == AV_PIX_FMT_ARGB || target == AV_PIX_FMT_RGBA ||
        target == AV_PIX_FMT_ABGR || target == AV_PIX_FMT_BGRA) {
        uint32_t *dest = (uint32_t *) _dest;
        const uint32_t *r = (const uint32_t *) _r;
        const uint32_t *g = (const uint32_t *) _g;
        const uint32_t *b = (const uint32_t *) _b;

#if CONFIG_SMALL
        dest[i * 2 + 0] = r[Y1] + g[Y1] + b[Y1];
        dest[i * 2 + 1] = r[Y2] + g[Y2] + b[Y2];
#else
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
        int sh = (target == AV_PIX_FMT_RGB32_1 ||
                  target == AV_PIX_FMT_BGR32_1) ? 0 : 24;
        av_assert2((((r[Y1] + g[Y1] + b[Y1]) >> sh) & 0xFF) == 0xFF);
#endif
        dest[i * 2 + 0] = r[Y1] + g[Y1] + b[Y1];
        dest[i * 2 + 1] = r[Y2] + g[Y2] + b[Y2];
#endif

    } else if (target == AV_PIX_FMT_RGB565 || target == AV_PIX_FMT_BGR565 ||
               target == AV_PIX_FMT_RGB555 || target == AV_PIX_FMT_BGR555 ||
               target == AV_PIX_FMT_RGB444 || target == AV_PIX_FMT_BGR444) {
        uint16_t *dest = (uint16_t *) _dest;
        const uint16_t *r = (const uint16_t *) _r;
        const uint16_t *g = (const uint16_t *) _g;
        const uint16_t *b = (const uint16_t *) _b;
        int dr1, dg1, db1, dr2, dg2, db2;

        if (target == AV_PIX_FMT_RGB565 || target == AV_PIX_FMT_BGR565) {
            dr1 = ff_dither_2x2_8[ y & 1     ][0];
            dg1 = ff_dither_2x2_4[ y & 1     ][0];
            db1 = ff_dither_2x2_8[(y & 1) ^ 1][0];
            dr2 = ff_dither_2x2_8[ y & 1     ][1];
            dg2 = ff_dither_2x2_4[ y & 1     ][1];
            db2 = ff_dither_2x2_8[(y & 1) ^ 1][1];
        }

        dest[i * 2 + 0] = r[Y1 + dr1] + g[Y1 + dg1] + b[Y1 + db1];
        dest[i * 2 + 1] = r[Y2 + dr2] + g[Y2 + dg2] + b[Y2 + db2];
    }
}

static av_always_inline void
yuv2rgb_X_msa_template(SwsContext *c, const int16_t *lumFilter,
                       const int16_t **lumSrc, int lumFilterSize,
                       const int16_t *chrFilter, const int16_t **chrUSrc,
                       const int16_t **chrVSrc, int chrFilterSize,
                       const int16_t **alpSrc, uint8_t *dest, int dstW,
                       int y, enum AVPixelFormat target, int hasAlpha)
{
    int i, j;
    int count = 0;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);

    for (i = 0; i < len; i += 8) {
        int t = 1 << 18;
        v8i16 l_src, u_src, v_src;
        v4i32 lumsrc_r, lumsrc_l, usrc, vsrc, temp;
        v4i32 y_r, y_l, u, v;

        y_r = __msa_fill_w(t);
        y_l = y_r;
        u   = y_r;
        v   = y_r;
        for (j = 0; j < lumFilterSize; j++) {
            temp     = __msa_fill_w(lumFilter[j]);
            l_src    = LD_V(v8i16, (lumSrc[j] + i));
            UNPCK_SH_SW(l_src, lumsrc_r, lumsrc_l);       /*can use lsx optimization*/
            y_r      = __msa_maddv_w(lumsrc_r, temp, y_r);
            y_l      = __msa_maddv_w(lumsrc_l, temp, y_l);
        }
        for (j = 0; j < chrFilterSize; j++) {
            u_src = LD_V(v8i16, (chrUSrc[j] + count));
            v_src = LD_V(v8i16, (chrVSrc[j] + count));
            UNPCK_R_SH_SW(u_src, usrc);
            UNPCK_R_SH_SW(v_src, vsrc);
            temp  = __msa_fill_w(chrFilter[j]);
            u     = __msa_maddv_w(usrc, temp, u);
            v     = __msa_maddv_w(vsrc, temp, v);
        }
        y_r = __msa_srai_w(y_r, 19);
        y_l = __msa_srai_w(y_l, 19);
        u   = __msa_srai_w(u, 19);
        v   = __msa_srai_w(v, 19);
        u   = __msa_addv_w(u, headroom);
        v   = __msa_addv_w(v, headroom);
        for (j = 0; j < 2; j++) {
            int Y1, Y2, U, V;
            int m = j * 2;
            int n = j + 2;

            Y1 = y_r[m];
            Y2 = y_r[m + 1];
            U  = u[j];
            V  = v[j];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
            Y1 = y_l[m];
            Y2 = y_l[m + 1];
            U  = u[n];
            V  = v[n];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
        count += 4;
    }
    for (count; count < len_count; count++) {
        int Y1 = 1 << 18;
        int Y2 = 1 << 18;
        int U  = 1 << 18;
        int V  = 1 << 18;

        for (j = 0; j < lumFilterSize; j++) {
            Y1 += lumSrc[j][count * 2]     * lumFilter[j];
            Y2 += lumSrc[j][count * 2 + 1] * lumFilter[j];
        }
        for (j = 0; j < chrFilterSize; j++) {
            U += chrUSrc[j][count] * chrFilter[j];
            V += chrVSrc[j][count] * chrFilter[j];
        }
        Y1 >>= 19;
        Y2 >>= 19;
        U  >>= 19;
        V  >>= 19;
        r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM];
        g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
             c->table_gV[V + YUVRGB_TABLE_HEADROOM]);
        b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

        yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                      r, g, b, y, target, 0);
    }
}

static av_always_inline void
yuv2rgb_2_msa_template(SwsContext *c, const int16_t *buf[2],
                       const int16_t *ubuf[2], const int16_t *vbuf[2],
                       const int16_t *abuf[2], uint8_t *dest, int dstW,
                       int yalpha, int uvalpha, int y,
                       enum AVPixelFormat target, int hasAlpha)
{
    const int16_t *buf0  = buf[0],  *buf1  = buf[1],
                  *ubuf0 = ubuf[0], *ubuf1 = ubuf[1],
                  *vbuf0 = vbuf[0], *vbuf1 = vbuf[1],
                  *abuf0 = NULL,
                  *abuf1 = NULL;
    int yalpha1  = 4096 - yalpha;
    int uvalpha1 = 4096 - uvalpha;
    int i, count = 0;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    v4i32 v_yalpha1  = (v4i32)__msa_fill_w(yalpha1);
    v4i32 v_uvalpha1 = (v4i32)__msa_fill_w(uvalpha1);
    v4i32 v_yalpha   = (v4i32)__msa_fill_w(yalpha);
    v4i32 v_uvalpha  = (v4i32)__msa_fill_w(uvalpha);
    v4i32 headroom   = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);

    for (i = 0; i < len; i += 8) {
        v8i16 src_y, src_u, src_v;
        v4i32 y0_r, y0_l, u0, v0;
        v4i32 y1_r, y1_l, u1, v1;
        v4i32 y_r, y_l, u, v;

        src_y = LD_V(v8i16, buf0 + i);
        src_u = LD_V(v8i16, ubuf0 + count);
        src_v = LD_V(v8i16, vbuf0 + count);
        UNPCK_SH_SW(src_y, y0_r, y0_l);
        UNPCK_R_SH_SW(src_u, u0);
        UNPCK_R_SH_SW(src_v, v0);
        src_y = LD_V(v8i16, buf1  + i);
        src_u = LD_V(v8i16, ubuf1 + count);
        src_v = LD_V(v8i16, vbuf1 + count);
        UNPCK_SH_SW(src_y, y1_r, y1_l);
        UNPCK_R_SH_SW(src_u, u1);
        UNPCK_R_SH_SW(src_v, v1);
        y0_r = __msa_mulv_w(y0_r, v_yalpha1);
        y0_l = __msa_mulv_w(y0_l, v_yalpha1);
        u0   = __msa_mulv_w(u0, v_uvalpha1);
        v0   = __msa_mulv_w(v0, v_uvalpha1);
        y_r  = __msa_maddv_w(y1_r, v_yalpha, y0_r);
        y_l  = __msa_maddv_w(y1_l, v_yalpha, y0_l);
        u    = __msa_maddv_w(u1, v_uvalpha, u0);
        v    = __msa_maddv_w(v1, v_uvalpha, v0);
        y_r  = __msa_srai_w(y_r, 19);
        y_l  = __msa_srai_w(y_l, 19);
        u    = __msa_srai_w(u, 19);
        v    = __msa_srai_w(v, 19);
        u    = __msa_addv_w(u, headroom);
        v    = __msa_addv_w(v, headroom);
        for (int j = 0; j < 2; j++) {
            int Y1, Y2, U, V;
            int m = j * 2;
            int n = j + 2;

            Y1 = y_r[m];
            Y2 = y_r[m + 1];
            U  = u[j];
            V  = v[j];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
            Y1 = y_l[m];
            Y2 = y_l[m + 1];
            U  = u[n];
            V  = v[n];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
        count += 4;
    }

    for (count; count < len_count; count++) {
        int Y1 = (buf0[count * 2]     * yalpha1  +
                  buf1[count * 2]     * yalpha)  >> 19;
        int Y2 = (buf0[count * 2 + 1] * yalpha1  +
                  buf1[count * 2 + 1] * yalpha) >> 19;
        int U  = (ubuf0[count] * uvalpha1 + ubuf1[count] * uvalpha) >> 19;
        int V  = (vbuf0[count] * uvalpha1 + vbuf1[count] * uvalpha) >> 19;

        r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
        g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
             c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
        b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

        yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                      r, g, b, y, target, 0);
    }
}

static av_always_inline void
yuv2rgb_1_msa_template(SwsContext *c, const int16_t *buf0,
                       const int16_t *ubuf[2], const int16_t *vbuf[2],
                       const int16_t *abuf0, uint8_t *dest, int dstW,
                       int uvalpha, int y, enum AVPixelFormat target,
                       int hasAlpha)
{
    const int16_t *ubuf0 = ubuf[0], *vbuf0 = vbuf[0];
    int i, j;
    const void *r, *g, *b;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;

    if (uvalpha < 2048) {
        int count = 0;
        v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);
        v4i32 bias_64   = (v4i32)__msa_fill_w(64);

        for (i = 0; i < len; i += 8) {
            v8i16 src_y, src_u, src_v;
            v4i32 y_r, y_l, u, v;

            src_y = LD_V(v8i16, buf0 + i);
            src_u = LD_V(v8i16, ubuf0 + count);
            src_v = LD_V(v8i16, vbuf0 + count);
            UNPCK_SH_SW(src_y, y_r, y_l);
            UNPCK_R_SH_SW(src_u, u);
            UNPCK_R_SH_SW(src_v, v);
            y_r = __msa_addv_w(y_r, bias_64);
            y_l = __msa_addv_w(y_l, bias_64);
            u   = __msa_addv_w(u, bias_64);
            v   = __msa_addv_w(v, bias_64);
            y_r = __msa_srai_w(y_r, 7);
            y_l = __msa_srai_w(y_l, 7);
            u   = __msa_srai_w(u, 7);
            v   = __msa_srai_w(v, 7);
            u   = __msa_addv_w(u, headroom);
            v   = __msa_addv_w(v, headroom);
            for (j = 0; j < 2; j++) {
                int Y1, Y2, U, V;
                int m = j * 2;
                int n = j + 2;

                Y1 = y_r[m];
                Y2 = y_r[m + 1];
                U  = u[j];
                V  = v[j];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
                Y1 = y_l[m];
                Y2 = y_l[m + 1];
                U  = u[n];
                V  = v[n];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
            }
            count += 4;
        }
        for (count; count < len_count; count++) {
            int Y1 = (buf0[count * 2    ] + 64) >> 7;
            int Y2 = (buf0[count * 2 + 1] + 64) >> 7;
            int U  = (ubuf0[count]        + 64) >> 7;
            int V  = (vbuf0[count]        + 64) >> 7;

            r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
            g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
                 c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
            b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

            yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
    } else {
        const int16_t *ubuf1 = ubuf[1], *vbuf1 = vbuf[1];
        int count = 0;
        v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);
        v4i32 bias_64   = (v4i32)__msa_fill_w(64);
        v4i32 bias_128  = (v4i32)__msa_fill_w(128);

        for (i = 0; i < len; i += 8) {
            v8i16 src_y, src_u, src_v;
            v4i32 y_r, y_l, u0, v0, u1, v1, u, v;

            src_y = LD_V(v8i16, buf0 + i);
            src_u = LD_V(v8i16, ubuf0 + count);
            src_v = LD_V(v8i16, vbuf0 + count);
            UNPCK_SH_SW(src_y, y_r, y_l);
            UNPCK_R_SH_SW(src_u, u0);
            UNPCK_R_SH_SW(src_v, v0);
            src_u = LD_V(v8i16, ubuf1 + count);
            src_v = LD_V(v8i16, vbuf1 + count);
            UNPCK_R_SH_SW(src_u, u1);
            UNPCK_R_SH_SW(src_v, v1);

            u   = __msa_addv_w(u0, u1);
            v   = __msa_addv_w(v0, v1);
            y_r = __msa_addv_w(y_r, bias_64);
            y_l = __msa_addv_w(y_l, bias_64);
            u   = __msa_addv_w(u, bias_128);
            v   = __msa_addv_w(v, bias_128);
            y_r = __msa_srai_w(y_r, 7);
            y_l = __msa_srai_w(y_l, 7);
            u   = __msa_srai_w(u, 8);
            v   = __msa_srai_w(v, 8);
            u   = __msa_addv_w(u, headroom);
            v   = __msa_addv_w(v, headroom);
            for (j = 0; j < 2; j++) {
                int Y1, Y2, U, V;
                int m = j * 2;
                int n = j + 2;

                Y1 = y_r[m];
                Y2 = y_r[m + 1];
                U  = u[j];
                V  = v[j];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
                Y1 = y_l[m];
                Y2 = y_l[m + 1];
                U  = u[n];
                V  = v[n];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
            }
            count += 4;
        }
        for (count; count < len_count; count++) {
            int Y1 = (buf0[count * 2    ]         +  64) >> 7;
            int Y2 = (buf0[count * 2 + 1]         +  64) >> 7;
            int U  = (ubuf0[count] + ubuf1[count] + 128) >> 8;
            int V  = (vbuf0[count] + vbuf1[count] + 128) >> 8;

            r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
            g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
                 c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
            b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

            yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
    }
}

#define YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                        \
void name ## ext ## _X_msa(SwsContext *c, const int16_t *lumFilter,            \
                           const int16_t **lumSrc, int lumFilterSize,          \
                           const int16_t *chrFilter, const int16_t **chrUSrc,  \
                           const int16_t **chrVSrc, int chrFilterSize,         \
                           const int16_t **alpSrc, uint8_t *dest, int dstW,    \
                           int y)                                              \
{                                                                              \
    name ## base ## _X_msa_template(c, lumFilter, lumSrc, lumFilterSize,       \
                                    chrFilter, chrUSrc, chrVSrc, chrFilterSize,\
                                    alpSrc, dest, dstW, y, fmt, hasAlpha);     \
}

#define YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                       \
YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                                \
void name ## ext ## _2_msa(SwsContext *c, const int16_t *buf[2],               \
                           const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                           const int16_t *abuf[2], uint8_t *dest, int dstW,    \
                           int yalpha, int uvalpha, int y)                     \
{                                                                              \
    name ## base ## _2_msa_template(c, buf, ubuf, vbuf, abuf, dest,            \
                                    dstW, yalpha, uvalpha, y, fmt, hasAlpha);  \
}

#define YUV2RGBWRAPPER(name, base, ext, fmt, hasAlpha)                         \
YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                               \
void name ## ext ## _1_msa(SwsContext *c, const int16_t *buf0,                 \
                           const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                           const int16_t *abuf0, uint8_t *dest, int dstW,      \
                           int uvalpha, int y)                                 \
{                                                                              \
    name ## base ## _1_msa_template(c, buf0, ubuf, vbuf, abuf0, dest,          \
                                    dstW, uvalpha, y, fmt, hasAlpha);          \
}


#if CONFIG_SAMALL
#else
#if CONFIG_SWSCALE_ALPHA
#endif
YUV2RGBWRAPPER(yuv2rgb,, x32_1,  AV_PIX_FMT_RGB32_1, 0)
YUV2RGBWRAPPER(yuv2rgb,, x32,    AV_PIX_FMT_RGB32, 0)
#endif
YUV2RGBWRAPPER(yuv2rgb,,  16,    AV_PIX_FMT_RGB565,    0)
