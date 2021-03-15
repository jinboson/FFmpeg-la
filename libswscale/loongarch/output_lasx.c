/*
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 * Contributed by Hao Chen(chenhao@loongson.cn)
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

#include "swscale_loongarch.h"
#include "libavutil/attributes.h"
#include "libavutil/loongarch/generic_macros_lasx.h"
#include "config.h"

void ff_yuv2planeX_8_lasx(const int16_t *filter, int filterSize,
                          const int16_t **src, uint8_t *dest, int dstW,
                          const uint8_t *dither, int offset)
{
    int i;
    int len = dstW - 15;
    __m256i mask = {0x1C1814100C080400, 0x0101010101010101, 0x1C1814100C080400, 0x0101010101010101};
    __m256i val1, val2, val3;
    uint8_t dither0 = dither[offset & 7];
    uint8_t dither1 = dither[(offset + 1) & 7];
    uint8_t dither2 = dither[(offset + 2) & 7];
    uint8_t dither3 = dither[(offset + 3) & 7];
    uint8_t dither4 = dither[(offset + 4) & 7];
    uint8_t dither5 = dither[(offset + 5) & 7];
    uint8_t dither6 = dither[(offset + 6) & 7];
    uint8_t dither7 = dither[(offset + 7) & 7];
    int val_1[8] = {dither0, dither1, dither2, dither3, dither0, dither1, dither2, dither3};
    int val_2[8] = {dither4, dither5, dither6, dither7, dither4, dither5, dither6, dither7};
    int val_3[8] = {dither0, dither1, dither2, dither3, dither4, dither5, dither6, dither7};

    val1 = LASX_LD(val_1);
    val2 = LASX_LD(val_2);
    val3 = LASX_LD(val_3);

    for (i = 0; i < len; i += 16) {
        int j;
        __m256i src0, filter0, val_lh;
        __m256i val_l, val_h;

        val_l = __lasx_xvslli_w(val1, 12);
        val_h = __lasx_xvslli_w(val2, 12);

        for (j = 0; j < filterSize; j++) {
            src0  = LASX_LD(src[j]+ i);
            filter0 = __lasx_xvldrepl_h((filter + j), 0);
            LASX_MADDWL_W_H_128SV(val_l, src0, filter0, val_l);
            LASX_MADDWH_W_H_128SV(val_h, src0, filter0, val_h);
        }
        val_l = __lasx_xvsrai_w(val_l, 19);
        val_h = __lasx_xvsrai_w(val_h, 19);
        LASX_CLIP_W_0_255(val_l, val_l);
        LASX_CLIP_W_0_255(val_h, val_h);
        LASX_SHUF_B_128SV(val_h, val_l, mask, val_lh);
        __lasx_xvstelm_d(val_lh, (dest + i), 0, 0);
        __lasx_xvstelm_d(val_lh, (dest + i), 8, 2);
    }
    if (dstW - i >= 8){
        int j;
        __m256i src0, filter0, val_h;
        __m256i val_l;

        val_l = __lasx_xvslli_w(val3, 12);

        for (j = 0; j < filterSize; j++) {
            src0  = LASX_LD(src[j] + i);
            src0  = __lasx_xvpermi_d(src0, 0xD8);
            filter0 = __lasx_xvldrepl_h((filter + j), 0);
            LASX_MADDWL_W_H_128SV(val_l, src0, filter0, val_l);
        }
        val_l = __lasx_xvsrai_w(val_l, 19);
        LASX_CLIP_W_0_255(val_l, val_l);
        val_h = __lasx_xvpermi_d(val_l, 0x4E);
        LASX_SHUF_B_128SV(val_h, val_l, mask, val_l);
        __lasx_xvstelm_d(val_l, (dest + i), 0, 0);
        i += 8;
    }
    for (; i < dstW; i++) {
        int val = dither[(i + offset) & 7] << 12;
        int j;
        for (j = 0; j< filterSize; j++)
            val += src[j][i] * filter[j];

        dest[i] = av_clip_uint8(val >> 19);
    }
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
    } else if (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) {
        uint8_t *dest = (uint8_t *) _dest;
        const uint8_t *r = (const uint8_t *) _r;
        const uint8_t *g = (const uint8_t *) _g;
        const uint8_t *b = (const uint8_t *) _b;

#define r_b ((target == AV_PIX_FMT_RGB24) ? r : b)
#define b_r ((target == AV_PIX_FMT_RGB24) ? b : r)

        dest[i * 6 + 0] = r_b[Y1];
        dest[i * 6 + 1] =   g[Y1];
        dest[i * 6 + 2] = b_r[Y1];
        dest[i * 6 + 3] = r_b[Y2];
        dest[i * 6 + 4] =   g[Y2];
        dest[i * 6 + 5] = b_r[Y2];
#undef r_b
#undef b_r
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
    } else if (target == AV_PIX_FMT_RGB555 || target == AV_PIX_FMT_BGR555) {
            dr1 = ff_dither_2x2_8[ y & 1     ][0];
            dg1 = ff_dither_2x2_8[ y & 1     ][1];
            db1 = ff_dither_2x2_8[(y & 1) ^ 1][0];
            dr2 = ff_dither_2x2_8[ y & 1     ][1];
            dg2 = ff_dither_2x2_8[ y & 1     ][0];
            db2 = ff_dither_2x2_8[(y & 1) ^ 1][1];
        } else {
            dr1 = ff_dither_4x4_16[ y & 3     ][0];
            dg1 = ff_dither_4x4_16[ y & 3     ][1];
            db1 = ff_dither_4x4_16[(y & 3) ^ 3][0];
            dr2 = ff_dither_4x4_16[ y & 3     ][1];
            dg2 = ff_dither_4x4_16[ y & 3     ][0];
            db2 = ff_dither_4x4_16[(y & 3) ^ 3][1];
        }

        dest[i * 2 + 0] = r[Y1 + dr1] + g[Y1 + dg1] + b[Y1 + db1];
        dest[i * 2 + 1] = r[Y2 + dr2] + g[Y2 + dg2] + b[Y2 + db2];
    } else /* 8/4 bits */ {
        uint8_t *dest = (uint8_t *) _dest;
        const uint8_t *r = (const uint8_t *) _r;
        const uint8_t *g = (const uint8_t *) _g;
        const uint8_t *b = (const uint8_t *) _b;
        int dr1, dg1, db1, dr2, dg2, db2;

        if (target == AV_PIX_FMT_RGB8 || target == AV_PIX_FMT_BGR8) {
            const uint8_t * const d64 = ff_dither_8x8_73[y & 7];
            const uint8_t * const d32 = ff_dither_8x8_32[y & 7];
            dr1 = dg1 = d32[(i * 2 + 0) & 7];
            db1 =       d64[(i * 2 + 0) & 7];
            dr2 = dg2 = d32[(i * 2 + 1) & 7];
            db2 =       d64[(i * 2 + 1) & 7];
        } else {
            const uint8_t * const d64  = ff_dither_8x8_73 [y & 7];
            const uint8_t * const d128 = ff_dither_8x8_220[y & 7];
            dr1 = db1 = d128[(i * 2 + 0) & 7];
            dg1 =        d64[(i * 2 + 0) & 7];
            dr2 = db2 = d128[(i * 2 + 1) & 7];
            dg2 =        d64[(i * 2 + 1) & 7];
        }

        if (target == AV_PIX_FMT_RGB4 || target == AV_PIX_FMT_BGR4) {
            dest[i] = r[Y1 + dr1] + g[Y1 + dg1] + b[Y1 + db1] +
                    ((r[Y2 + dr2] + g[Y2 + dg2] + b[Y2 + db2]) << 4);
        } else {
            dest[i * 2 + 0] = r[Y1 + dr1] + g[Y1 + dg1] + b[Y1 + db1];
            dest[i * 2 + 1] = r[Y2 + dr2] + g[Y2 + dg2] + b[Y2 + db2];
        }
    }
}

#define WRITE_YUV2RGB_16(y_l, y_h, u, v, count, r, g, b, y,               \
                         target, Y1, Y2, U, V)                            \
{                                                                         \
    Y1 = __lasx_xvpickve2gr_w(y_l, 0);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 1);                                    \
    U  = __lasx_xvpickve2gr_w(u, 0);                                      \
    V  = __lasx_xvpickve2gr_w(v, 0);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 2);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 3);                                    \
    U  = __lasx_xvpickve2gr_w(u, 1);                                      \
    V  = __lasx_xvpickve2gr_w(v, 1);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 0);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 1);                                    \
    U  = __lasx_xvpickve2gr_w(u, 2);                                      \
    V  = __lasx_xvpickve2gr_w(v, 2);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 2);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 3);                                    \
    U  = __lasx_xvpickve2gr_w(u, 3);                                      \
    V  = __lasx_xvpickve2gr_w(v, 3);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 4);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 5);                                    \
    U  = __lasx_xvpickve2gr_w(u, 4);                                      \
    V  = __lasx_xvpickve2gr_w(v, 4);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 6);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 7);                                    \
    U  = __lasx_xvpickve2gr_w(u, 5);                                      \
    V  = __lasx_xvpickve2gr_w(v, 5);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 4);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 5);                                    \
    U  = __lasx_xvpickve2gr_w(u, 6);                                      \
    V  = __lasx_xvpickve2gr_w(v, 6);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 6);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 7);                                    \
    U  = __lasx_xvpickve2gr_w(u, 7);                                      \
    V  = __lasx_xvpickve2gr_w(v, 7);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
}

#define WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target,            \
                         Y1, Y2, U, V)                                    \
{                                                                         \
    Y1 = __lasx_xvpickve2gr_w(y_l, 0);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 1);                                    \
    U  = __lasx_xvpickve2gr_w(u, 0);                                      \
    V  = __lasx_xvpickve2gr_w(v, 0);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 2);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 3);                                    \
    U  = __lasx_xvpickve2gr_w(u, 1);                                      \
    V  = __lasx_xvpickve2gr_w(v, 1);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 4);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 5);                                    \
    U  = __lasx_xvpickve2gr_w(u, 2);                                      \
    V  = __lasx_xvpickve2gr_w(v, 2);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_l, 6);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_l, 7);                                    \
    U  = __lasx_xvpickve2gr_w(u, 3);                                      \
    V  = __lasx_xvpickve2gr_w(v, 3);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
}

#define WRITE_YUV2RGBH_8(y_h, u, v, count, r, g, b, y, target,            \
                         Y1, Y2, U, V)                                    \
{                                                                         \
    Y1 = __lasx_xvpickve2gr_w(y_h, 0);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 1);                                    \
    U  = __lasx_xvpickve2gr_w(u, 4);                                      \
    V  = __lasx_xvpickve2gr_w(v, 4);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 2);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 3);                                    \
    U  = __lasx_xvpickve2gr_w(u, 5);                                      \
    V  = __lasx_xvpickve2gr_w(v, 5);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 4);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 5);                                    \
    U  = __lasx_xvpickve2gr_w(u, 6);                                      \
    V  = __lasx_xvpickve2gr_w(v, 6);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
                                                                          \
    Y1 = __lasx_xvpickve2gr_w(y_h, 6);                                    \
    Y2 = __lasx_xvpickve2gr_w(y_h, 7);                                    \
    U  = __lasx_xvpickve2gr_w(u, 7);                                      \
    V  = __lasx_xvpickve2gr_w(v, 7);                                      \
    r  =  c->table_rV[V];                                                 \
    g  = (c->table_gU[U] + c->table_gV[V]);                               \
    b  =  c->table_bU[U];                                                 \
    yuv2rgb_write(dest, count, Y1, Y2, 0, 0,                              \
                  r, g, b, y, target, 0);                                 \
    count++;                                                              \
}

static av_always_inline void
yuv2rgb_X_template_lasx(SwsContext *c, const int16_t *lumFilter,
                        const int16_t **lumSrc, int lumFilterSize,
                        const int16_t *chrFilter, const int16_t **chrUSrc,
                        const int16_t **chrVSrc, int chrFilterSize,
                        const int16_t **alpSrc, uint8_t *dest, int dstW,
                        int y, enum AVPixelFormat target, int hasAlpha)
{
    int i, j;
    int count = 0;
    int t     = 1 << 18;
    int len   = dstW - 15;
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    int head = YUVRGB_TABLE_HEADROOM;
    __m256i headroom  = __lasx_xvldrepl_w(&head, 0);

    for (i = 0; i < len; i += 16) {
        int Y1, Y2, U, V;
        __m256i l_src, u_src, v_src;
        __m256i y_l, y_h, u, v, temp;

        y_l = __lasx_xvldrepl_w(&t, 0);
        y_h = y_l;
        u   = y_l;
        v   = y_l;
        for (j = 0; j < lumFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((lumFilter + j), 0);
            l_src = LASX_LD((lumSrc[j] + i));
            LASX_MADDWL_W_H_128SV(y_l, l_src, temp, y_l);
            LASX_MADDWH_W_H_128SV(y_h, l_src, temp, y_h);
        }
        for (j = 0; j < chrFilterSize; j++) {
            u_src = LASX_LD((chrUSrc[j] + count));
            v_src = LASX_LD((chrVSrc[j] + count));
            temp  = __lasx_xvldrepl_h((chrFilter + j), 0);
            u_src = __lasx_xvpermi_d(u_src, 0xD8);
            v_src = __lasx_xvpermi_d(v_src, 0xD8);
            LASX_MADDWL_W_H_2_128SV(u, temp, u_src, v, temp, v_src, u, v);
        }
        y_l = __lasx_xvsrai_w(y_l, 19);
        y_h = __lasx_xvsrai_w(y_h, 19);
        u   = __lasx_xvsrai_w(u, 19);
        v   = __lasx_xvsrai_w(v, 19);
        u   = __lasx_xvadd_w(u, headroom);
        v   = __lasx_xvadd_w(v, headroom);
        WRITE_YUV2RGB_16(y_l, y_h, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
    }
    if (dstW - i >= 8) {
        int Y1, Y2, U, V;
        __m256i l_src, u_src, v_src;
        __m256i y_l, u, v, temp;

        y_l = __lasx_xvldrepl_w(&t, 0);
        u   = y_l;
        v   = y_l;
        for (j = 0; j < lumFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((lumFilter + j), 0);
            l_src = LASX_LD((lumSrc[j] + i));
            l_src = __lasx_xvpermi_d(l_src, 0xD8);
            LASX_MADDWL_W_H_128SV(y_l, l_src, temp, y_l);

        }
        for (j = 0; j < chrFilterSize; j++) {
            u_src = __lasx_xvldrepl_d((chrUSrc[j] + count), 0);
            v_src = __lasx_xvldrepl_d((chrVSrc[j] + count), 0);
            temp  = __lasx_xvldrepl_h((chrFilter + j), 0);
            LASX_MADDWL_W_H_2_128SV(u, temp, u_src, v, temp, v_src, u, v);
        }
        y_l = __lasx_xvsrai_w(y_l, 19);
        u   = __lasx_xvsrai_w(u, 19);
        v   = __lasx_xvsrai_w(v, 19);
        u   = __lasx_xvadd_w(u, headroom);
        v   = __lasx_xvadd_w(v, headroom);
        WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
        i += 8;
    }
    for (; count < len_count; count++) {
        int Y1 = 1 << 18;
        int Y2 = Y1;
        int U  = Y1;
        int V  = Y1;

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
yuv2rgb_2_template_lasx(SwsContext *c, const int16_t *buf[2],
                        const int16_t *ubuf[2], const int16_t *vbuf[2],
                        const int16_t *abuf[2], uint8_t *dest, int dstW,
                        int yalpha, int uvalpha, int y,
                        enum AVPixelFormat target, int hasAlpha)
{
    const int16_t *buf0  = buf[0],  *buf1  = buf[1],
                  *ubuf0 = ubuf[0], *ubuf1 = ubuf[1],
                  *vbuf0 = vbuf[0], *vbuf1 = vbuf[1];
    int yalpha1   = 4096 - yalpha;
    int uvalpha1  = 4096 - uvalpha;
    int i, count  = 0;
    int len       = dstW - 15;
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    int head  = YUVRGB_TABLE_HEADROOM;
    __m256i v_yalpha1  = __lasx_xvldrepl_w(&yalpha1, 0);
    __m256i v_uvalpha1 = __lasx_xvldrepl_w(&uvalpha1, 0);
    __m256i v_yalpha   = __lasx_xvldrepl_w(&yalpha, 0);
    __m256i v_uvalpha  = __lasx_xvldrepl_w(&uvalpha, 0);
    __m256i headroom   = __lasx_xvldrepl_w(&head, 0);

    for (i = 0; i < len; i += 16) {
        int Y1, Y2, U, V;
        __m256i y0_h, y0_l, y0, u0, v0;
        __m256i y1_h, y1_l, y1, u1, v1;
        __m256i y_l, y_h, u, v;

        y0   = LASX_LD(buf0 + i);
        u0   = LASX_LD(ubuf0 + count);
        v0   = LASX_LD(vbuf0 + count);
        y1   = LASX_LD(buf1 + i);
        u1   = LASX_LD(ubuf1 + count);
        v1   = LASX_LD(vbuf1 + count);
        LASX_UNPCK_L_W_H_2(y0, y1, y0_l, y1_l);
        y0   = __lasx_xvpermi_d(y0, 0x4E);
        y1   = __lasx_xvpermi_d(y1, 0x4E);
        LASX_UNPCK_L_W_H_2(y0, y1, y0_h, y1_h);
        LASX_UNPCK_L_W_H_4(u0, u1, v0, v1, u0, u1, v0, v1);
        y0_l = __lasx_xvmul_w(y0_l, v_yalpha1);
        y0_h = __lasx_xvmul_w(y0_h, v_yalpha1);
        u0   = __lasx_xvmul_w(u0, v_uvalpha1);
        v0   = __lasx_xvmul_w(v0, v_uvalpha1);
        y_l  = __lasx_xvmadd_w(y0_l, v_yalpha, y1_l);
        y_h  = __lasx_xvmadd_w(y0_h, v_yalpha, y1_h);
        u    = __lasx_xvmadd_w(u0, v_uvalpha, u1);
        v    = __lasx_xvmadd_w(v0, v_uvalpha, v1);
        y_l  = __lasx_xvsrai_w(y_l, 19);
        y_h  = __lasx_xvsrai_w(y_h, 19);
        u    = __lasx_xvsrai_w(u, 19);
        v    = __lasx_xvsrai_w(v, 19);
        u    = __lasx_xvadd_w(u, headroom);
        v    = __lasx_xvadd_w(v, headroom);
        WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
        WRITE_YUV2RGBH_8(y_h, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
    }
    if (dstW - i >= 8) {
        int Y1, Y2, U, V;
        __m256i y0_l, y0, u0, v0;
        __m256i y1_l, y1, u1, v1;
        __m256i y_l, u, v;

        y0   = LASX_LD(buf0 + i);
        u0   = __lasx_xvldrepl_d((ubuf0 + count), 0);
        v0   = __lasx_xvldrepl_d((vbuf0 + count), 0);
        y1   = LASX_LD(buf1 + i);
        u1   = __lasx_xvldrepl_d((ubuf1 + count), 0);
        v1   = __lasx_xvldrepl_d((vbuf1 + count), 0);
        LASX_UNPCK_L_W_H_2(y0, y1, y0_l, y1_l);
        LASX_UNPCK_L_W_H_4(u0, u1, v0, v1, u0, u1, v0, v1);
        y0_l = __lasx_xvmul_w(y0_l, v_yalpha1);
        u0   = __lasx_xvmul_w(u0, v_uvalpha1);
        v0   = __lasx_xvmul_w(v0, v_uvalpha1);
        y_l  = __lasx_xvmadd_w(y0_l, v_yalpha, y1_l);
        u    = __lasx_xvmadd_w(u0, v_uvalpha, u1);
        v    = __lasx_xvmadd_w(v0, v_uvalpha, v1);
        y_l  = __lasx_xvsrai_w(y_l, 19);
        u    = __lasx_xvsrai_w(u, 19);
        v    = __lasx_xvsrai_w(v, 19);
        u    = __lasx_xvadd_w(u, headroom);
        v    = __lasx_xvadd_w(v, headroom);
        WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
        i += 8;
    }
    for (; count < len_count; count++) {
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
yuv2rgb_1_template_lasx(SwsContext *c, const int16_t *buf0,
                        const int16_t *ubuf[2], const int16_t *vbuf[2],
                        const int16_t *abuf0, uint8_t *dest, int dstW,
                        int uvalpha, int y, enum AVPixelFormat target,
                        int hasAlpha)
{
    const int16_t *ubuf0 = ubuf[0], *vbuf0 = vbuf[0];
    int i;
    int len       = (dstW - 15);
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;

    if (uvalpha < 2048) {
        int count    = 0;
        int16_t bias_int = 64;
        int head = YUVRGB_TABLE_HEADROOM;
        __m256i headroom  = __lasx_xvldrepl_w(&head, 0);
        __m256i bias_64   = __lasx_xvldrepl_h(&bias_int, 0);

        for (i = 0; i < len; i += 16) {
            int Y1, Y2, U, V;
            __m256i src_y, src_u, src_v;
            __m256i y_h, y_l, u, v;

            src_y = LASX_LD(buf0 + i);
            src_u = LASX_LD(ubuf0 + count);
            src_v = LASX_LD(vbuf0 + count);
            LASX_ADDWL_W_H_128SV(src_y, bias_64, y_l);
            LASX_ADDWH_W_H_128SV(src_y, bias_64, y_h);
            src_u = __lasx_xvpermi_d(src_u, 0xD8);
            src_v = __lasx_xvpermi_d(src_v, 0xD8);
            LASX_ADDWL_W_H_2_128SV(src_u, bias_64, src_v, bias_64, u, v);
            y_l   = __lasx_xvsrai_w(y_l, 7);
            y_h   = __lasx_xvsrai_w(y_h, 7);
            u     = __lasx_xvsrai_w(u, 7);
            v     = __lasx_xvsrai_w(v, 7);
            u     = __lasx_xvadd_w(u, headroom);
            v     = __lasx_xvadd_w(v, headroom);
            WRITE_YUV2RGB_16(y_l, y_h, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
        }
        if (dstW - i >= 8){
            int Y1, Y2, U, V;
            __m256i src_y, src_u, src_v;
            __m256i y_l, u, v;

            src_y = LASX_LD(buf0 + i);
            src_u = __lasx_xvldrepl_d((ubuf0 + count), 0);
            src_v = __lasx_xvldrepl_d((vbuf0 + count), 0);
            src_y = __lasx_xvpermi_d(src_y, 0xD8);
            LASX_ADDWL_W_H_2_128SV(src_y, bias_64, src_u, bias_64, y_l, u);
            LASX_ADDWL_W_H_128SV(src_v, bias_64, v);
            y_l   = __lasx_xvsrai_w(y_l, 7);
            u     = __lasx_xvsrai_w(u, 7);
            v     = __lasx_xvsrai_w(v, 7);
            u     = __lasx_xvadd_w(u, headroom);
            v     = __lasx_xvadd_w(v, headroom);
            WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
            i += 8;
        }
        for (; count < len_count; count++) {
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
        int16_t bias_int_64 = 64;
        int bias_int_128    = 128;
        int HEADROOM = YUVRGB_TABLE_HEADROOM;
        __m256i headroom    = __lasx_xvldrepl_w(&HEADROOM, 0);
        __m256i bias_64     = __lasx_xvldrepl_h(&bias_int_64, 0);
        __m256i bias_128    = __lasx_xvldrepl_w(&bias_int_128, 0);

        for (i = 0; i < len; i += 16) {
            int Y1, Y2, U, V;
            __m256i src_y, src_u0, src_v0, src_u1, src_v1;
            __m256i y_h, y_l, u, v;

            src_y  = LASX_LD(buf0 + i);
            src_u0 = LASX_LD(ubuf0 + count);
            src_v0 = LASX_LD(vbuf0 + count);
            src_u1 = LASX_LD(ubuf1 + count);
            src_v1 = LASX_LD(vbuf1 + count);
            src_u0 = __lasx_xvpermi_d(src_u0, 0xD8);
            src_v0 = __lasx_xvpermi_d(src_v0, 0xD8);
            src_u1 = __lasx_xvpermi_d(src_u1, 0xD8);
            src_v1 = __lasx_xvpermi_d(src_v1, 0xD8);
            LASX_ADDWL_W_H_128SV(src_y, bias_64, y_l);
            LASX_ADDWH_W_H_128SV(src_y, bias_64, y_h);
            LASX_ADDWL_W_H_2_128SV(src_u0, src_u1, src_v0, src_v1, u, v);
            u      = __lasx_xvadd_w(u, bias_128);
            v      = __lasx_xvadd_w(v, bias_128);
            y_l    = __lasx_xvsrai_w(y_l, 7);
            y_h    = __lasx_xvsrai_w(y_h, 7);
            u      = __lasx_xvsrai_w(u, 8);
            v      = __lasx_xvsrai_w(v, 8);
            u      = __lasx_xvadd_w(u, headroom);
            v      = __lasx_xvadd_w(v, headroom);
            WRITE_YUV2RGB_16(y_l, y_h, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
        }
        if (dstW - i >= 8){
            int Y1, Y2, U, V;
            __m256i src_y, src_u0, src_v0, src_u1, src_v1;
            __m256i y_l, u, v;

            src_y  = LASX_LD(buf0 + i);
            src_u0 = __lasx_xvldrepl_d((ubuf0 + count), 0);
            src_v0 = __lasx_xvldrepl_d((vbuf0 + count), 0);
            src_u1 = __lasx_xvldrepl_d((ubuf1 + count), 0);
            src_v1 = __lasx_xvldrepl_d((vbuf1 + count), 0);

            src_y  = __lasx_xvpermi_d(src_y, 0xD8);
            LASX_ADDWL_W_H_128SV(src_y, bias_64, y_l);
            LASX_ADDWL_W_H_2_128SV(src_u0, src_u1, src_v0, src_v1, u, v);
            u      = __lasx_xvadd_w(u, bias_128);
            v      = __lasx_xvadd_w(v, bias_128);
            y_l    = __lasx_xvsrai_w(y_l, 7);
            u      = __lasx_xvsrai_w(u, 8);
            v      = __lasx_xvsrai_w(v, 8);
            u      = __lasx_xvadd_w(u, headroom);
            v      = __lasx_xvadd_w(v, headroom);
            WRITE_YUV2RGBL_8(y_l, u, v, count, r, g, b, y, target, Y1, Y2, U, V);
            i += 8;
        }
        for (; count < len_count; count++) {
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

#define YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                                \
static void name ## ext ## _X_lasx(SwsContext *c, const int16_t *lumFilter,            \
                                   const int16_t **lumSrc, int lumFilterSize,          \
                                   const int16_t *chrFilter, const int16_t **chrUSrc,  \
                                   const int16_t **chrVSrc, int chrFilterSize,         \
                                   const int16_t **alpSrc, uint8_t *dest, int dstW,    \
                                   int y)                                              \
{                                                                                      \
    name ## base ## _X_template_lasx(c, lumFilter, lumSrc, lumFilterSize,              \
                                     chrFilter, chrUSrc, chrVSrc, chrFilterSize,       \
                                     alpSrc, dest, dstW, y, fmt, hasAlpha);            \
}

#define YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                               \
YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                                        \
static void name ## ext ## _2_lasx(SwsContext *c, const int16_t *buf[2],               \
                                   const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                                   const int16_t *abuf[2], uint8_t *dest, int dstW,    \
                                   int yalpha, int uvalpha, int y)                     \
{                                                                                      \
    name ## base ## _2_template_lasx(c, buf, ubuf, vbuf, abuf, dest,                   \
                                     dstW, yalpha, uvalpha, y, fmt, hasAlpha);         \
}

#define YUV2RGBWRAPPER(name, base, ext, fmt, hasAlpha)                                 \
YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                                       \
static void name ## ext ## _1_lasx(SwsContext *c, const int16_t *buf0,                 \
                                   const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                                   const int16_t *abuf0, uint8_t *dest, int dstW,      \
                                   int uvalpha, int y)                                 \
{                                                                                      \
    name ## base ## _1_template_lasx(c, buf0, ubuf, vbuf, abuf0, dest,                 \
                                     dstW, uvalpha, y, fmt, hasAlpha);                 \
}


#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
#endif
YUV2RGBWRAPPER(yuv2rgb,, x32_1,  AV_PIX_FMT_RGB32_1, 0)
YUV2RGBWRAPPER(yuv2rgb,, x32,    AV_PIX_FMT_RGB32,   0)
#endif
YUV2RGBWRAPPER(yuv2, rgb, rgb24, AV_PIX_FMT_RGB24,     0)
YUV2RGBWRAPPER(yuv2, rgb, bgr24, AV_PIX_FMT_BGR24,     0)
YUV2RGBWRAPPER(yuv2rgb,,  16,    AV_PIX_FMT_RGB565,    0)
YUV2RGBWRAPPER(yuv2rgb,,  15,    AV_PIX_FMT_RGB555,    0)
YUV2RGBWRAPPER(yuv2rgb,,  12,    AV_PIX_FMT_RGB444,    0)
YUV2RGBWRAPPER(yuv2rgb,,   8,    AV_PIX_FMT_RGB8,      0)
YUV2RGBWRAPPER(yuv2rgb,,   4,    AV_PIX_FMT_RGB4,      0)
YUV2RGBWRAPPER(yuv2rgb,,   4b,   AV_PIX_FMT_RGB4_BYTE, 0)

// This function is copied from libswscale/output.c
static av_always_inline void yuv2rgb_write_full(SwsContext *c,
    uint8_t *dest, int i, int R, int A, int G, int B,
    int y, enum AVPixelFormat target, int hasAlpha, int err[4])
{
    int isrgb8 = target == AV_PIX_FMT_BGR8 || target == AV_PIX_FMT_RGB8;

    if ((R | G | B) & 0xC0000000) {
        R = av_clip_uintp2(R, 30);
        G = av_clip_uintp2(G, 30);
        B = av_clip_uintp2(B, 30);
    }

    switch(target) {
    case AV_PIX_FMT_ARGB:
        dest[0] = hasAlpha ? A : 255;
        dest[1] = R >> 22;
        dest[2] = G >> 22;
        dest[3] = B >> 22;
        break;
    case AV_PIX_FMT_RGB24:
        dest[0] = R >> 22;
        dest[1] = G >> 22;
        dest[2] = B >> 22;
        break;
    case AV_PIX_FMT_RGBA:
        dest[0] = R >> 22;
        dest[1] = G >> 22;
        dest[2] = B >> 22;
        dest[3] = hasAlpha ? A : 255;
        break;
    case AV_PIX_FMT_ABGR:
        dest[0] = hasAlpha ? A : 255;
        dest[1] = B >> 22;
        dest[2] = G >> 22;
        dest[3] = R >> 22;
        break;
    case AV_PIX_FMT_BGR24:
        dest[0] = B >> 22;
        dest[1] = G >> 22;
        dest[2] = R >> 22;
        break;
    case AV_PIX_FMT_BGRA:
        dest[0] = B >> 22;
        dest[1] = G >> 22;
        dest[2] = R >> 22;
        dest[3] = hasAlpha ? A : 255;
        break;
    case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB8:
    {
        int r,g,b;

        switch (c->dither) {
        default:
        case SWS_DITHER_AUTO:
        case SWS_DITHER_ED:
            R >>= 22;
            G >>= 22;
            B >>= 22;
            R += (7*err[0] + 1*c->dither_error[0][i] + 5*c->dither_error[0][i+1] + 3*c->dither_error[0][i+2])>>4;
            G += (7*err[1] + 1*c->dither_error[1][i] + 5*c->dither_error[1][i+1] + 3*c->dither_error[1][i+2])>>4;
            B += (7*err[2] + 1*c->dither_error[2][i] + 5*c->dither_error[2][i+1] + 3*c->dither_error[2][i+2])>>4;
            c->dither_error[0][i] = err[0];
            c->dither_error[1][i] = err[1];
            c->dither_error[2][i] = err[2];
            r = R >> (isrgb8 ? 5 : 7);
            g = G >> (isrgb8 ? 5 : 6);
            b = B >> (isrgb8 ? 6 : 7);
            r = av_clip(r, 0, isrgb8 ? 7 : 1);
            g = av_clip(g, 0, isrgb8 ? 7 : 3);
            b = av_clip(b, 0, isrgb8 ? 3 : 1);
            err[0] = R - r*(isrgb8 ? 36 : 255);
            err[1] = G - g*(isrgb8 ? 36 : 85);
            err[2] = B - b*(isrgb8 ? 85 : 255);
            break;
        case SWS_DITHER_A_DITHER:
            if (isrgb8) {
  /* see http://pippin.gimp.org/a_dither/ for details/origin */
#define A_DITHER(u,v)   (((((u)+((v)*236))*119)&0xff))
                r = (((R >> 19) + A_DITHER(i,y)  -96)>>8);
                g = (((G >> 19) + A_DITHER(i + 17,y) - 96)>>8);
                b = (((B >> 20) + A_DITHER(i + 17*2,y) -96)>>8);
                r = av_clip_uintp2(r, 3);
                g = av_clip_uintp2(g, 3);
                b = av_clip_uintp2(b, 2);
            } else {
                r = (((R >> 21) + A_DITHER(i,y)-256)>>8);
                g = (((G >> 19) + A_DITHER(i + 17,y)-256)>>8);
                b = (((B >> 21) + A_DITHER(i + 17*2,y)-256)>>8);
                r = av_clip_uintp2(r, 1);
                g = av_clip_uintp2(g, 2);
                b = av_clip_uintp2(b, 1);
            }
            break;
        case SWS_DITHER_X_DITHER:
            if (isrgb8) {
  /* see http://pippin.gimp.org/a_dither/ for details/origin */
#define X_DITHER(u,v)   (((((u)^((v)*237))*181)&0x1ff)/2)
                r = (((R >> 19) + X_DITHER(i,y) - 96)>>8);
                g = (((G >> 19) + X_DITHER(i + 17,y) - 96)>>8);
                b = (((B >> 20) + X_DITHER(i + 17*2,y) - 96)>>8);
                r = av_clip_uintp2(r, 3);
                g = av_clip_uintp2(g, 3);
                b = av_clip_uintp2(b, 2);
            } else {
                r = (((R >> 21) + X_DITHER(i,y)-256)>>8);
                g = (((G >> 19) + X_DITHER(i + 17,y)-256)>>8);
                b = (((B >> 21) + X_DITHER(i + 17*2,y)-256)>>8);
                r = av_clip_uintp2(r, 1);
                g = av_clip_uintp2(g, 2);
                b = av_clip_uintp2(b, 1);
            }

            break;
        }

        if(target == AV_PIX_FMT_BGR4_BYTE) {
            dest[0] = r + 2*g + 8*b;
        } else if(target == AV_PIX_FMT_RGB4_BYTE) {
            dest[0] = b + 2*g + 8*r;
        } else if(target == AV_PIX_FMT_BGR8) {
            dest[0] = r + 8*g + 64*b;
        } else if(target == AV_PIX_FMT_RGB8) {
            dest[0] = b + 4*g + 32*r;
        } else
            av_assert2(0);
        break; }
    }
}

#define YUVTORGB_SETUP                                           \
    int y_offset   = c->yuv2rgb_y_offset;                        \
    int y_coeff    = c->yuv2rgb_y_coeff;                         \
    int v2r_coe    = c->yuv2rgb_v2r_coeff;                       \
    int v2g_coe    = c->yuv2rgb_v2g_coeff;                       \
    int u2g_coe    = c->yuv2rgb_u2g_coeff;                       \
    int u2b_coe    = c->yuv2rgb_u2b_coeff;                       \
    __m256i offset = __lasx_xvldrepl_w(&y_offset, 0);            \
    __m256i coeff  = __lasx_xvldrepl_w(&y_coeff, 0);             \
    __m256i v2r    = __lasx_xvldrepl_w(&v2r_coe, 0);             \
    __m256i v2g    = __lasx_xvldrepl_w(&v2g_coe, 0);             \
    __m256i u2g    = __lasx_xvldrepl_w(&u2g_coe, 0);             \
    __m256i u2b    = __lasx_xvldrepl_w(&u2b_coe, 0);             \


#define YUVTORGB(y, u, v, R, G, B, offset, coeff,              \
                 y_temp, v2r, v2g, u2g, u2b)                   \
{                                                              \
     y = __lasx_xvsub_w(y, offset);                            \
     y = __lasx_xvmul_w(y, coeff);                             \
     y = __lasx_xvadd_w(y, y_temp);                            \
     R = __lasx_xvmadd_w(y, v, v2r);                           \
     v = __lasx_xvmadd_w(y, v, v2g);                           \
     G = __lasx_xvmadd_w(v, u, u2g);                           \
     B = __lasx_xvmadd_w(y, u, u2b);                           \
}

#define WRITE_FULL_L_A(r, g, b, a, c, dest, i, R, A, G, B,                    \
                       y, target, hasAlpha, err)                              \
{                                                                             \
    R = __lasx_xvpickve2gr_w(r, 0);                                           \
    G = __lasx_xvpickve2gr_w(g, 0);                                           \
    B = __lasx_xvpickve2gr_w(b, 0);                                           \
    A = __lasx_xvpickve2gr_w(a, 0);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);     \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 1);                                           \
    G = __lasx_xvpickve2gr_w(g, 1);                                           \
    B = __lasx_xvpickve2gr_w(b, 1);                                           \
    A = __lasx_xvpickve2gr_w(a, 1);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 1, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 2);                                           \
    G = __lasx_xvpickve2gr_w(g, 2);                                           \
    B = __lasx_xvpickve2gr_w(b, 2);                                           \
    A = __lasx_xvpickve2gr_w(a, 2);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 2, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 3);                                           \
    G = __lasx_xvpickve2gr_w(g, 3);                                           \
    B = __lasx_xvpickve2gr_w(b, 3);                                           \
    A = __lasx_xvpickve2gr_w(a, 3);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 3, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
}

#define WRITE_FULL_H_A(r, g, b, a, c, dest, i, R, A, G, B,                    \
                       y, target, hasAlpha, err)                              \
{                                                                             \
    R = __lasx_xvpickve2gr_w(r, 4);                                           \
    G = __lasx_xvpickve2gr_w(g, 4);                                           \
    B = __lasx_xvpickve2gr_w(b, 4);                                           \
    A = __lasx_xvpickve2gr_w(a, 4);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);     \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 5);                                           \
    G = __lasx_xvpickve2gr_w(g, 5);                                           \
    B = __lasx_xvpickve2gr_w(b, 5);                                           \
    A = __lasx_xvpickve2gr_w(a, 5);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 1, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 6);                                           \
    G = __lasx_xvpickve2gr_w(g, 6);                                           \
    B = __lasx_xvpickve2gr_w(b, 6);                                           \
    A = __lasx_xvpickve2gr_w(a, 6);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 2, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 7);                                           \
    G = __lasx_xvpickve2gr_w(g, 7);                                           \
    B = __lasx_xvpickve2gr_w(b, 7);                                           \
    A = __lasx_xvpickve2gr_w(a, 7);                                           \
    if (A & 0x100)                                                            \
        A = av_clip_uint8(A);                                                 \
    yuv2rgb_write_full(c, dest, i + 3, R, A, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
}


#define WRITE_FULL_L(r, g, b, c, dest, i, R, G, B,                            \
                     y, target, hasAlpha, err)                                \
{                                                                             \
    R = __lasx_xvpickve2gr_w(r, 0);                                           \
    G = __lasx_xvpickve2gr_w(g, 0);                                           \
    B = __lasx_xvpickve2gr_w(b, 0);                                           \
    yuv2rgb_write_full(c, dest, i, R, 0, G, B, y, target, hasAlpha, err);     \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 1);                                           \
    G = __lasx_xvpickve2gr_w(g, 1);                                           \
    B = __lasx_xvpickve2gr_w(b, 1);                                           \
    yuv2rgb_write_full(c, dest, i + 1, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 2);                                           \
    G = __lasx_xvpickve2gr_w(g, 2);                                           \
    B = __lasx_xvpickve2gr_w(b, 2);                                           \
    yuv2rgb_write_full(c, dest, i + 2, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 3);                                           \
    G = __lasx_xvpickve2gr_w(g, 3);                                           \
    B = __lasx_xvpickve2gr_w(b, 3);                                           \
    yuv2rgb_write_full(c, dest, i + 3, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
}

#define WRITE_FULL_H(r, g, b, c, dest, i, R, G, B,                            \
                     y, target, hasAlpha, err)                                \
{                                                                             \
    R = __lasx_xvpickve2gr_w(r, 4);                                           \
    G = __lasx_xvpickve2gr_w(g, 4);                                           \
    B = __lasx_xvpickve2gr_w(b, 4);                                           \
    yuv2rgb_write_full(c, dest, i, R, 0, G, B, y, target, hasAlpha, err);     \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 5);                                           \
    G = __lasx_xvpickve2gr_w(g, 5);                                           \
    B = __lasx_xvpickve2gr_w(b, 5);                                           \
    yuv2rgb_write_full(c, dest, i + 1, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 6);                                           \
    G = __lasx_xvpickve2gr_w(g, 6);                                           \
    B = __lasx_xvpickve2gr_w(b, 6);                                           \
    yuv2rgb_write_full(c, dest, i + 2, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
    R = __lasx_xvpickve2gr_w(r, 7);                                           \
    G = __lasx_xvpickve2gr_w(g, 7);                                           \
    B = __lasx_xvpickve2gr_w(b, 7);                                           \
    yuv2rgb_write_full(c, dest, i + 3, R, 0, G, B, y, target, hasAlpha, err); \
    dest += step;                                                             \
}

static av_always_inline void
yuv2rgb_full_X_template_lasx(SwsContext *c, const int16_t *lumFilter,
                             const int16_t **lumSrc, int lumFilterSize,
                             const int16_t *chrFilter, const int16_t **chrUSrc,
                             const int16_t **chrVSrc, int chrFilterSize,
                             const int16_t **alpSrc, uint8_t *dest,
                             int dstW, int y, enum AVPixelFormat target, int hasAlpha)
{
    int i, j, B, G, R, A;
    int step       = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    int err[4]     = {0};
    int a_temp     = 1 << 18;
    int templ      = 1 << 9;
    int tempc      = templ - (128 << 19);
    int ytemp      = 1 << 21;
    int len        = dstW - 15;
    __m256i y_temp = __lasx_xvldrepl_w(&ytemp, 0);
    YUVTORGB_SETUP

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;

    for (i = 0; i < len; i += 16) {
        __m256i l_src, u_src, v_src;
        __m256i y_l, y_h, u_l, u_h, v_l, v_h, temp;
        __m256i R_l, R_h, G_l, G_h, B_l, B_h;

        y_l = y_h = __lasx_xvldrepl_w(&templ, 0);
        u_l = u_h = v_l = v_h = __lasx_xvldrepl_w(&tempc, 0);
        for (j = 0; j < lumFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((lumFilter + j), 0);
            l_src = LASX_LD((lumSrc[j] + i));
            LASX_MADDWL_W_H_128SV(y_l, l_src, temp, y_l);
            LASX_MADDWH_W_H_128SV(y_h, l_src, temp, y_h);
        }
        for (j = 0; j < chrFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((chrFilter + j), 0);
            u_src = LASX_LD((chrUSrc[j] + i));
            v_src = LASX_LD((chrVSrc[j] + i));
            LASX_MADDWL_W_H_2_128SV(u_l, u_src, temp, v_l, v_src, temp, u_l, v_l);
            LASX_MADDWH_W_H_2_128SV(u_h, u_src, temp, v_h, v_src, temp, u_h, v_h);
        }
        y_l = __lasx_xvsrai_w(y_l, 10);
        y_h = __lasx_xvsrai_w(y_h, 10);
        u_l = __lasx_xvsrai_w(u_l, 10);
        u_h = __lasx_xvsrai_w(u_h, 10);
        v_l = __lasx_xvsrai_w(v_l, 10);
        v_h = __lasx_xvsrai_w(v_h, 10);
        YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);
        YUVTORGB(y_h, u_h, v_h, R_h, G_h, B_h, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);

        if (hasAlpha) {
            __m256i a_src, a_l, a_h;

            a_l = a_h = __lasx_xvldrepl_w(&a_temp, 0);
            for (j = 0; j < lumFilterSize; j++) {
                temp  = __lasx_xvldrepl_h(lumFilter + j, 0);
                a_src = LASX_LD((alpSrc[j] + i));
                LASX_MADDWL_W_H_128SV(a_l, a_src, temp, a_l);
                LASX_MADDWH_W_H_128SV(a_h, a_src, temp, a_h);
            }
            a_h = __lasx_xvsrai_w(a_h, 19);
            a_l = __lasx_xvsrai_w(a_l, 19);
            WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_L_A(R_h, G_h, B_h, a_h, c, dest, i + 4, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 8, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_h, G_h, B_h, a_h, c, dest, i + 12, R, A, G, B,
                           y, target, hasAlpha, err);
        } else {
            WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_L(R_h, G_h, B_h, c, dest, i + 4, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 8, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_h, G_h, B_h, c, dest, i + 12, R, G, B,
                           y, target, hasAlpha, err);
        }
    }
    if (dstW - i >= 8) {
        __m256i l_src, u_src, v_src;
        __m256i y_l, u_l, v_l, temp;
        __m256i R_l, G_l, B_l;

        y_l = __lasx_xvldrepl_w(&templ, 0);
        u_l = v_l = __lasx_xvldrepl_w(&tempc, 0);
        for (j = 0; j < lumFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((lumFilter + j), 0);
            l_src = LASX_LD((lumSrc[j] + i));
            l_src = __lasx_xvpermi_d(l_src, 0xD8);
            LASX_MADDWL_W_H_128SV(y_l, l_src, temp, y_l);
        }
        for (j = 0; j < chrFilterSize; j++) {
            temp  = __lasx_xvldrepl_h((chrFilter + j), 0);
            u_src = LASX_LD((chrUSrc[j] + i));
            v_src = LASX_LD((chrVSrc[j] + i));
            u_src = __lasx_xvpermi_d(u_src, 0xD8);
            v_src = __lasx_xvpermi_d(v_src, 0xD8);
            LASX_MADDWL_W_H_2_128SV(u_l, u_src, temp, v_l, v_src, temp, u_l, v_l);
        }
        y_l = __lasx_xvsrai_w(y_l, 10);
        u_l = __lasx_xvsrai_w(u_l, 10);
        v_l = __lasx_xvsrai_w(v_l, 10);
        YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);

        if (hasAlpha) {
            __m256i a_src, a_l;

            a_l = __lasx_xvldrepl_w(&a_temp, 0);
            for (j = 0; j < lumFilterSize; j++) {
                temp  = __lasx_xvldrepl_h(lumFilter + j, 0);
                a_src = LASX_LD((alpSrc[j] + i));
                a_src = __lasx_xvpermi_d(a_src, 0xD8);
                LASX_MADDWL_W_H_128SV(a_l, a_src, temp, a_l);
            }
            a_l = __lasx_xvsrai_w(a_l, 19);
            WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                           y, target, hasAlpha, err);
        } else {
            WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                         y, target, hasAlpha, err);
        }
        i += 8;
    }
    for (; i < dstW; i++) {
        int Y = templ;
        int V, U = V = tempc;

        A = 0;
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
        if (hasAlpha) {
            A = 1 << 18;
            for (j = 0; j < lumFilterSize; j++) {
                A += alpSrc[j][i] * lumFilter[j];
            }
            A >>= 19;
            if (A & 0x100)
                A = av_clip_uint8(A);
        }
        Y -= y_offset;
        Y *= y_coeff;
        Y += ytemp;
        R  = (unsigned)Y + V * v2r_coe;
        G  = (unsigned)Y + V * v2g_coe + U * u2g_coe;
        B  = (unsigned)Y + U * u2b_coe;
        yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
        dest += step;
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}

static av_always_inline void
yuv2rgb_full_2_template_lasx(SwsContext *c, const int16_t *buf[2],
                             const int16_t *ubuf[2], const int16_t *vbuf[2],
                             const int16_t *abuf[2], uint8_t *dest, int dstW,
                             int yalpha, int uvalpha, int y,
                             enum AVPixelFormat target, int hasAlpha)
{
    const int16_t *buf0  = buf[0],  *buf1  = buf[1],
                  *ubuf0 = ubuf[0], *ubuf1 = ubuf[1],
                  *vbuf0 = vbuf[0], *vbuf1 = vbuf[1],
                  *abuf0 = hasAlpha ? abuf[0] : NULL,
                  *abuf1 = hasAlpha ? abuf[1] : NULL;
    int yalpha1  = 4096 - yalpha;
    int uvalpha1 = 4096 - uvalpha;
    int uvtemp   = 128 << 19;
    int atemp    = 1 << 18;
    int err[4]   = {0};
    int ytemp    = 1 << 21;
    int len      = dstW - 15;
    int i, R, G, B, A;
    int step = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    __m256i v_uvalpha1 = __lasx_xvldrepl_w(&uvalpha1, 0);
    __m256i v_yalpha1  = __lasx_xvldrepl_w(&yalpha1, 0);
    __m256i v_uvalpha  = __lasx_xvldrepl_w(&uvalpha, 0);
    __m256i v_yalpha   = __lasx_xvldrepl_w(&yalpha, 0);
    __m256i uv         = __lasx_xvldrepl_w(&uvtemp, 0);
    __m256i a_bias     = __lasx_xvldrepl_w(&atemp, 0);
    __m256i y_temp     = __lasx_xvldrepl_w(&ytemp, 0);
    YUVTORGB_SETUP

    av_assert2(yalpha  <= 4096U);
    av_assert2(uvalpha <= 4096U);

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;

    for (i = 0; i < len; i += 16) {
        __m256i b0, b1, ub0, ub1, vb0, vb1;
        __m256i y0_l, y0_h, y1_l, y1_h, u0_l, u0_h;
        __m256i v0_l, v0_h, u1_l, u1_h, v1_l, v1_h;
        __m256i y_l, y_h, v_l, v_h, u_l, u_h;
        __m256i R_l, R_h, G_l, G_h, B_l, B_h;

        b0   = LASX_LD((buf0 + i));
        b1   = LASX_LD((buf1 + i));
        ub0  = LASX_LD((ubuf0 + i));
        ub1  = LASX_LD((ubuf1 + i));
        vb0  = LASX_LD((vbuf0 + i));
        vb1  = LASX_LD((vbuf1 + i));
        LASX_UNPCK_L_W_H_2(b0, b1, y0_l, y1_l);
        LASX_UNPCK_L_W_H_4(ub0, ub1, vb0, vb1, u0_l, u1_l, v0_l, v1_l);
        b0   = __lasx_xvpermi_d(b0, 0x4E);
        b1   = __lasx_xvpermi_d(b1, 0x4E);
        ub0  = __lasx_xvpermi_d(ub0, 0x4E);
        ub1  = __lasx_xvpermi_d(ub1, 0x4E);
        vb0  = __lasx_xvpermi_d(vb0, 0x4E);
        vb1  = __lasx_xvpermi_d(vb1, 0x4E);
        LASX_UNPCK_L_W_H_2(b0, b1, y0_h, y1_h);
        LASX_UNPCK_L_W_H_4(ub0, ub1, vb0, vb1, u0_h, u1_h, v0_h, v1_h);
        y0_l = __lasx_xvmul_w(y0_l, v_yalpha1);
        y0_h = __lasx_xvmul_w(y0_h, v_yalpha1);
        u0_l = __lasx_xvmul_w(u0_l, v_uvalpha1);
        u0_h = __lasx_xvmul_w(u0_h, v_uvalpha1);
        v0_l = __lasx_xvmul_w(v0_l, v_uvalpha1);
        v0_h = __lasx_xvmul_w(v0_h, v_uvalpha1);
        y_l  = __lasx_xvmadd_w(y0_l, v_yalpha, y1_l);
        y_h  = __lasx_xvmadd_w(y0_h, v_yalpha, y1_h);
        u_l  = __lasx_xvmadd_w(u0_l, v_uvalpha, u1_l);
        u_h  = __lasx_xvmadd_w(u0_h, v_uvalpha, u1_h);
        v_l  = __lasx_xvmadd_w(v0_l, v_uvalpha, v1_l);
        v_h  = __lasx_xvmadd_w(v0_h, v_uvalpha, v1_h);
        u_l  = __lasx_xvsub_w(u_l, uv);
        u_h  = __lasx_xvsub_w(u_h, uv);
        v_l  = __lasx_xvsub_w(v_l, uv);
        v_h  = __lasx_xvsub_w(v_h, uv);
        y_l  = __lasx_xvsrai_w(y_l, 10);
        y_h  = __lasx_xvsrai_w(y_h, 10);
        u_l  = __lasx_xvsrai_w(u_l, 10);
        u_h  = __lasx_xvsrai_w(u_h, 10);
        v_l  = __lasx_xvsrai_w(v_l, 10);
        v_h  = __lasx_xvsrai_w(v_h, 10);
        YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);
        YUVTORGB(y_h, u_h, v_h, R_h, G_h, B_h, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);

        if (hasAlpha) {
            __m256i a0, a1, a0_l, a0_h;
            __m256i a_l, a_h, a1_l, a1_h;

            a0  = LASX_LD((abuf0 + i));
            a1  = LASX_LD((abuf1 + i));
            LASX_UNPCK_L_W_H_2(a0, a1, a0_l, a1_l);
            a0  = __lasx_xvpermi_d(a0, 0x4E);
            a1  = __lasx_xvpermi_d(a1, 0x4E);
            LASX_UNPCK_L_W_H_2(a0, a1, a0_h, a1_h);
            a_l = __lasx_xvmadd_w(a_bias, a0_l, v_yalpha1);
            a_h = __lasx_xvmadd_w(a_bias, a0_h, v_yalpha1);
            a_l = __lasx_xvmadd_w(a_l, v_yalpha, a1_l);
            a_h = __lasx_xvmadd_w(a_h, v_yalpha, a1_h);
            a_l = __lasx_xvsrai_w(a_l, 19);
            a_h = __lasx_xvsrai_w(a_h, 19);
            WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_L_A(R_h, G_h, B_h, a_h, c, dest, i + 8, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_h, G_h, B_h, a_h, c, dest, i + 12, R, A, G, B,
                           y, target, hasAlpha, err);
        } else {
            WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_L(R_h, G_h, B_h, c, dest, i + 8, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_h, G_h, B_h, c, dest, i + 12, R, G, B,
                         y, target, hasAlpha, err);
        }
    }
    if (dstW - i >= 8) {
        __m256i b0, b1, ub0, ub1, vb0, vb1;
        __m256i y0_l, y1_l, u0_l;
        __m256i v0_l, u1_l, v1_l;
        __m256i y_l, u_l, v_l;
        __m256i R_l, G_l, B_l;

        b0   = LASX_LD((buf0 + i));
        b1   = LASX_LD((buf1 + i));
        ub0  = LASX_LD((ubuf0 + i));
        ub1  = LASX_LD((ubuf1 + i));
        vb0  = LASX_LD((vbuf0 + i));
        vb1  = LASX_LD((vbuf1 + i));
        LASX_UNPCK_L_W_H_2(b0, b1, y0_l, y1_l);
        LASX_UNPCK_L_W_H_4(ub0, ub1, vb0, vb1, u0_l, u1_l, v0_l, v1_l);
        y0_l = __lasx_xvmul_w(y0_l, v_yalpha1);
        u0_l = __lasx_xvmul_w(u0_l, v_uvalpha1);
        v0_l = __lasx_xvmul_w(v0_l, v_uvalpha1);
        y_l  = __lasx_xvmadd_w(y0_l, v_yalpha, y1_l);
        u_l  = __lasx_xvmadd_w(u0_l, v_uvalpha, u1_l);
        v_l  = __lasx_xvmadd_w(v0_l, v_uvalpha, v1_l);
        u_l  = __lasx_xvsub_w(u_l, uv);
        v_l  = __lasx_xvsub_w(v_l, uv);
        y_l  = __lasx_xvsrai_w(y_l, 10);
        u_l  = __lasx_xvsrai_w(u_l, 10);
        v_l  = __lasx_xvsrai_w(v_l, 10);
        YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                 y_temp, v2r, v2g, u2g, u2b);

        if (hasAlpha) {
            __m256i a0, a1, a0_l;
            __m256i a_l, a1_l;

            a0  = LASX_LD((abuf0 + i));
            a1  = LASX_LD((abuf1 + i));
            LASX_UNPCK_L_W_H_2(a0, a1, a0_l, a1_l);
            a_l = __lasx_xvmadd_w(a_bias, a0_l, v_yalpha1);
            a_l = __lasx_xvmadd_w(a_l, v_yalpha, a1_l);
            a_l = __lasx_xvsrai_w(a_l, 19);
            WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                           y, target, hasAlpha, err);
            WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                           y, target, hasAlpha, err);
        } else {
            WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                         y, target, hasAlpha, err);
            WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                         y, target, hasAlpha, err);
        }
        i += 8;
    }
    for (; i < dstW; i++){
        int Y = ( buf0[i] * yalpha1  +  buf1[i] * yalpha         ) >> 10; //FIXME rounding
        int U = (ubuf0[i] * uvalpha1 + ubuf1[i] * uvalpha- uvtemp) >> 10;
        int V = (vbuf0[i] * uvalpha1 + vbuf1[i] * uvalpha- uvtemp) >> 10;

        A = 0;
        if (hasAlpha){
            A = (abuf0[i] * yalpha1 + abuf1[i] * yalpha + atemp) >> 19;
            if (A & 0x100)
                A = av_clip_uint8(A);
        }

        Y -= y_offset;
        Y *= y_coeff;
        Y += ytemp;
        R  = (unsigned)Y + V * v2r_coe;
        G  = (unsigned)Y + V * v2g_coe + U * u2g_coe;
        B  = (unsigned)Y + U * u2b_coe;
        yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
        dest += step;
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}

static av_always_inline void
yuv2rgb_full_1_template_lasx(SwsContext *c, const int16_t *buf0,
                             const int16_t *ubuf[2], const int16_t *vbuf[2],
                             const int16_t *abuf0, uint8_t *dest, int dstW,
                             int uvalpha, int y, enum AVPixelFormat target,
                             int hasAlpha)
{
    const int16_t *ubuf0 = ubuf[0], *vbuf0 = vbuf[0];
    int i, B, G, R, A;
    int step = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    int err[4]     = {0};
    int ytemp      = 1 << 21;
    int bias_int   = 64;
    int len        = dstW - 15;
    __m256i bias   = __lasx_xvldrepl_w(&bias_int, 0);
    __m256i y_temp = __lasx_xvldrepl_w(&ytemp, 0);
    YUVTORGB_SETUP

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;
    if (uvalpha < 2048) {
        int uvtemp = 128 << 7;
        __m256i uv = __lasx_xvldrepl_w(&uvtemp, 0);

        for (i = 0; i < len; i += 16) {
            __m256i b, ub, vb, ub_l, ub_h, vb_l, vb_h;
            __m256i y_l, y_h, u_l, u_h, v_l, v_h;
            __m256i R_l, R_h, G_l, G_h, B_l, B_h;

            b   = LASX_LD((buf0 + i));
            ub  = LASX_LD((ubuf0 + i));
            vb  = LASX_LD((vbuf0 + i));
            LASX_UNPCK_L_W_H(b, y_l);
            LASX_UNPCK_L_W_H_2(ub, vb, ub_l, vb_l);
            b   = __lasx_xvpermi_d(b, 0x4E);
            ub  = __lasx_xvpermi_d(ub, 0x4E);
            vb  = __lasx_xvpermi_d(vb, 0x4E);
            LASX_UNPCK_L_W_H(b, y_h);
            LASX_UNPCK_L_W_H_2(ub, vb, ub_h, vb_h);
            y_l = __lasx_xvslli_w(y_l, 2);
            y_h = __lasx_xvslli_w(y_h, 2);
            u_l = __lasx_xvsub_w(ub_l, uv);
            u_h = __lasx_xvsub_w(ub_h, uv);
            v_l = __lasx_xvsub_w(vb_l, uv);
            v_h = __lasx_xvsub_w(vb_h, uv);
            u_l = __lasx_xvslli_w(u_l, 2);
            u_h = __lasx_xvslli_w(u_h, 2);
            v_l = __lasx_xvslli_w(v_l, 2);
            v_h = __lasx_xvslli_w(v_h, 2);
            YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);
            YUVTORGB(y_h, u_h, v_h, R_h, G_h, B_h, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);

            if(hasAlpha) {
                __m256i a_src;
                __m256i a_l, a_h;

                a_src = LASX_LD((abuf0 + i));
                a_src = __lasx_xvpermi_d(a_src, 0xD8);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_l);
                a_src = __lasx_xvpermi_d(a_src, 0xB1);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_h);
                a_l   = __lasx_xvsrai_w(a_l, 7);
                a_h   = __lasx_xvsrai_w(a_h, 7);
                WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_L_A(R_h, G_h, B_h, a_h, c, dest, i + 8, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_h, G_h, B_h, a_h, c, dest, i + 12, R, A, G, B,
                               y, target, hasAlpha, err);
            } else {
                WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_L(R_h, G_h, B_h, c, dest, i + 8, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_h, G_h, B_h, c, dest, i + 12, R, G, B,
                             y, target, hasAlpha, err);
            }
        }
        if (dstW - i >= 8) {
            __m256i b, ub, vb, ub_l, vb_l;
            __m256i y_l, u_l, v_l;
            __m256i R_l, G_l, B_l;

            b   = LASX_LD((buf0 + i));
            ub  = LASX_LD((ubuf0 + i));
            vb  = LASX_LD((vbuf0 + i));
            LASX_UNPCK_L_W_H(b, y_l);
            LASX_UNPCK_L_W_H_2(ub, vb, ub_l, vb_l);
            y_l = __lasx_xvslli_w(y_l, 2);
            u_l = __lasx_xvsub_w(ub_l, uv);
            v_l = __lasx_xvsub_w(vb_l, uv);
            u_l = __lasx_xvslli_w(u_l, 2);
            v_l = __lasx_xvslli_w(v_l, 2);
            YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);

            if(hasAlpha) {
                __m256i a_src;
                __m256i a_r, a_l;

                a_src = LASX_LD((abuf0 + i));
                a_src = __lasx_xvpermi_d(a_src, 0xD8);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_l);
                a_l   = __lasx_xvsrai_w(a_r, 7);
                WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                               y, target, hasAlpha, err);
            } else {
                WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                             y, target, hasAlpha, err);
            }
            i += 8;
        }
        for (; i < dstW; i++) {
            int Y = buf0[i] << 2;
            int U = (ubuf0[i] - uvtemp) << 2;
            int V = (vbuf0[i] - uvtemp) << 2;

            A = 0;
            if(hasAlpha) {
                A = (abuf0[i] + 64) >> 7;
                if (A & 0x100)
                    A = av_clip_uint8(A);
            }
            Y -= y_offset;
            Y *= y_coeff;
            Y += ytemp;
            R  = (unsigned)Y + V * v2r_coe;
            G  = (unsigned)Y + V * v2g_coe + U * u2g_coe;
            B  = (unsigned)Y + U * u2b_coe;
            yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
            dest += step;
        }
    } else {
        const int16_t *ubuf1 = ubuf[1], *vbuf1 = vbuf[1];
        int uvtemp = 128 << 8;
        __m256i uv = __lasx_xvldrepl_w(&uvtemp, 0);

        for (i = 0; i < len; i += 16) {
            __m256i b, ub0, ub1, vb0, vb1;
            __m256i y_l, y_h, u_l, u_h, v_l, v_h;
            __m256i R_l, R_h, G_l, G_h, B_l, B_h;

            b   = LASX_LD((buf0 + i));
            ub0 = LASX_LD((ubuf0 + i));
            vb0 = LASX_LD((vbuf0 + i));
            ub1 = LASX_LD((ubuf1 + i));
            vb1 = LASX_LD((vbuf1 + i));
            LASX_UNPCK_L_W_H(b, y_l);
            b   = __lasx_xvpermi_d(b, 0X4E);
            LASX_UNPCK_L_W_H(b, y_h);
            y_l = __lasx_xvslli_w(y_l, 2);
            y_h = __lasx_xvslli_w(y_h, 2);
            ub0 = __lasx_xvpermi_d(ub0, 0xD8);
            vb0 = __lasx_xvpermi_d(vb0, 0xD8);
            ub1 = __lasx_xvpermi_d(ub1, 0xD8);
            vb1 = __lasx_xvpermi_d(vb1, 0xD8);
            LASX_ADDWL_W_H_2_128SV(ub0, ub1, vb0, vb1, u_l, v_l);
            LASX_ADDWH_W_H_2_128SV(ub0, ub1, vb0, vb1, u_h, v_h);
            u_l = __lasx_xvsub_w(u_l, uv);
            u_h = __lasx_xvsub_w(u_h, uv);
            v_l = __lasx_xvsub_w(v_l, uv);
            v_h = __lasx_xvsub_w(v_h, uv);
            u_l = __lasx_xvslli_w(u_l, 1);
            u_h = __lasx_xvslli_w(u_h, 1);
            v_l = __lasx_xvslli_w(v_l, 1);
            v_h = __lasx_xvslli_w(v_h, 1);
            YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);
            YUVTORGB(y_h, u_h, v_h, R_h, G_h, B_h, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);

            if(hasAlpha) {
                __m256i a_src;
                __m256i a_l, a_h;

                a_src = LASX_LD((abuf0 + i));
                a_src = __lasx_xvpermi_d(a_src, 0xD8);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_l);
                a_src = __lasx_xvpermi_d(a_src, 0xB1);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_h);
                a_l   = __lasx_xvsrai_w(a_l, 7);
                a_h   = __lasx_xvsrai_w(a_h, 7);
                WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_L_A(R_h, G_h, B_h, a_h, c, dest, i + 8, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_h, G_h, B_h, a_h, c, dest, i + 12, R, A, G, B,
                               y, target, hasAlpha, err);
            } else {
                WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_L(R_h, G_h, B_h, c, dest, i + 8, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_h, G_h, B_h, c, dest, i + 12, R, G, B,
                             y, target, hasAlpha, err);
            }
        }
        if (dstW - i >= 8) {
            __m256i b, ub0, ub1, vb0, vb1;
            __m256i y_l, u_l, v_l;
            __m256i R_l, G_l, B_l;

            b   = LASX_LD((buf0 + i));
            ub0 = LASX_LD((ubuf0 + i));
            vb0 = LASX_LD((vbuf0 + i));
            ub1 = LASX_LD((ubuf1 + i));
            vb1 = LASX_LD((vbuf1 + i));
            LASX_UNPCK_L_W_H(b, y_l);
            y_l = __lasx_xvslli_w(y_l, 2);
            ub0 = __lasx_xvpermi_d(ub0, 0xD8);
            vb0 = __lasx_xvpermi_d(vb0, 0xD8);
            ub1 = __lasx_xvpermi_d(ub1, 0xD8);
            vb1 = __lasx_xvpermi_d(vb1, 0xD8);
            LASX_ADDWL_W_H_2_128SV(ub0, ub1, vb0, vb1, u_l, v_l);
            u_l = __lasx_xvsub_w(u_l, uv);
            v_l = __lasx_xvsub_w(v_l, uv);
            u_l = __lasx_xvslli_w(u_l, 1);
            v_l = __lasx_xvslli_w(v_l, 1);
            YUVTORGB(y_l, u_l, v_l, R_l, G_l, B_l, offset, coeff,
                     y_temp, v2r, v2g, u2g, u2b);

            if(hasAlpha) {
                __m256i a_src;
                __m256i a_l;

                a_src = LASX_LD((abuf0 + i));
                a_src = __lasx_xvpermi_d(a_src, 0xD8);
                LASX_ADDW_W_W_H_128SV(bias, a_src, a_l);
                a_l   = __lasx_xvsrai_w(a_l, 7);
                WRITE_FULL_L_A(R_l, G_l, B_l, a_l, c, dest, i, R, A, G, B,
                               y, target, hasAlpha, err);
                WRITE_FULL_H_A(R_l, G_l, B_l, a_l, c, dest, i + 4, R, A, G, B,
                               y, target, hasAlpha, err);
            } else {
                WRITE_FULL_L(R_l, G_l, B_l, c, dest, i, R, G, B,
                             y, target, hasAlpha, err);
                WRITE_FULL_H(R_l, G_l, B_l, c, dest, i + 4, R, G, B,
                             y, target, hasAlpha, err);
            }
            i += 8;
        }
        for (; i < dstW; i++) {
            int Y = buf0[i] << 2;
            int U = (ubuf0[i] + ubuf1[i] - uvtemp) << 1;
            int V = (vbuf0[i] + vbuf1[i] - uvtemp) << 1;

            A = 0;
            if(hasAlpha) {
                A = (abuf0[i] + 64) >> 7;
                if (A & 0x100)
                    A = av_clip_uint8(A);
            }
            Y -= y_offset;
            Y *= y_coeff;
            Y += ytemp;
            R  = (unsigned)Y + V * v2r_coe;
            G  = (unsigned)Y + V * v2g_coe + U * u2g_coe;
            B  = (unsigned)Y + U * u2b_coe;
            yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
            dest += step;
        }
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}
#if CONFIG_SMALL
YUV2RGBWRAPPER(yuv2, rgb_full, bgra32_full, AV_PIX_FMT_BGRA,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, abgr32_full, AV_PIX_FMT_ABGR,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, rgba32_full, AV_PIX_FMT_RGBA,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, argb32_full, AV_PIX_FMT_ARGB,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
#else
#if CONFIG_SWSCALE_ALPHA
YUV2RGBWRAPPER(yuv2, rgb_full, bgra32_full, AV_PIX_FMT_BGRA,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, abgr32_full, AV_PIX_FMT_ABGR,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, rgba32_full, AV_PIX_FMT_RGBA,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, argb32_full, AV_PIX_FMT_ARGB,  1)
#endif
YUV2RGBWRAPPER(yuv2, rgb_full, bgrx32_full, AV_PIX_FMT_BGRA,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, xbgr32_full, AV_PIX_FMT_ABGR,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgbx32_full, AV_PIX_FMT_RGBA,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, xrgb32_full, AV_PIX_FMT_ARGB,  0)
#endif
YUV2RGBWRAPPER(yuv2, rgb_full, bgr24_full,  AV_PIX_FMT_BGR24, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb24_full,  AV_PIX_FMT_RGB24, 0)

YUV2RGBWRAPPER(yuv2, rgb_full, bgr4_byte_full,  AV_PIX_FMT_BGR4_BYTE, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb4_byte_full,  AV_PIX_FMT_RGB4_BYTE, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, bgr8_full,   AV_PIX_FMT_BGR8,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb8_full,   AV_PIX_FMT_RGB8,  0)
#undef yuvTorgb
#undef yuvTorgb_setup


av_cold void ff_sws_init_output_loongarch(SwsContext *c)
{

    if(c->flags & SWS_FULL_CHR_H_INT) {
        switch (c->dstFormat) {
        case AV_PIX_FMT_RGBA:
#if CONFIG_SMALL
            c->yuv2packedX = yuv2rgba32_full_X_lasx;
            c->yuv2packed2 = yuv2rgba32_full_2_lasx;
            c->yuv2packed1 = yuv2rgba32_full_1_lasx;
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
                c->yuv2packedX = yuv2rgba32_full_X_lasx;
                c->yuv2packed2 = yuv2rgba32_full_2_lasx;
                c->yuv2packed1 = yuv2rgba32_full_1_lasx;
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packedX = yuv2rgbx32_full_X_lasx;
                c->yuv2packed2 = yuv2rgbx32_full_2_lasx;
                c->yuv2packed1 = yuv2rgbx32_full_1_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_ARGB:
#if CONFIG_SMALL
            c->yuv2packedX = yuv2argb32_full_X_lasx;
            c->yuv2packed2 = yuv2argb32_full_2_lasx;
            c->yuv2packed1 = yuv2argb32_full_1_lasx;
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
                c->yuv2packedX = yuv2argb32_full_X_lasx;
                c->yuv2packed2 = yuv2argb32_full_2_lasx;
                c->yuv2packed1 = yuv2argb32_full_1_lasx;
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packedX = yuv2xrgb32_full_X_lasx;
                c->yuv2packed2 = yuv2xrgb32_full_2_lasx;
                c->yuv2packed1 = yuv2xrgb32_full_1_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_BGRA:
#if CONFIG_SMALL
            c->yuv2packedX = yuv2bgra32_full_X_lasx;
            c->yuv2packed2 = yuv2bgra32_full_2_lasx;
            c->yuv2packed1 = yuv2bgra32_full_1_lasx;
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
                c->yuv2packedX = yuv2bgra32_full_X_lasx;
                c->yuv2packed2 = yuv2bgra32_full_2_lasx;
                c->yuv2packed1 = yuv2bgra32_full_1_lasx;
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packedX = yuv2bgrx32_full_X_lasx;
                c->yuv2packed2 = yuv2bgrx32_full_2_lasx;
                c->yuv2packed1 = yuv2bgrx32_full_1_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_ABGR:
#if CONFIG_SMALL
            c->yuv2packedX = yuv2abgr32_full_X_lasx;
            c->yuv2packed2 = yuv2abgr32_full_2_lasx;
            c->yuv2packed1 = yuv2abgr32_full_1_lasx;
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
                c->yuv2packedX = yuv2abgr32_full_X_lasx;
                c->yuv2packed2 = yuv2abgr32_full_2_lasx;
                c->yuv2packed1 = yuv2abgr32_full_1_lasx;
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packedX = yuv2xbgr32_full_X_lasx;
                c->yuv2packed2 = yuv2xbgr32_full_2_lasx;
                c->yuv2packed1 = yuv2xbgr32_full_1_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_RGB24:
            c->yuv2packedX = yuv2rgb24_full_X_lasx;
            c->yuv2packed2 = yuv2rgb24_full_2_lasx;
            c->yuv2packed1 = yuv2rgb24_full_1_lasx;
            break;
        case AV_PIX_FMT_BGR24:
            c->yuv2packedX = yuv2bgr24_full_X_lasx;
            c->yuv2packed2 = yuv2bgr24_full_2_lasx;
            c->yuv2packed1 = yuv2bgr24_full_1_lasx;
            break;
        case AV_PIX_FMT_BGR4_BYTE:
            c->yuv2packedX = yuv2bgr4_byte_full_X_lasx;
            c->yuv2packed2 = yuv2bgr4_byte_full_2_lasx;
            c->yuv2packed1 = yuv2bgr4_byte_full_1_lasx;
            break;
        case AV_PIX_FMT_RGB4_BYTE:
            c->yuv2packedX = yuv2rgb4_byte_full_X_lasx;
            c->yuv2packed2 = yuv2rgb4_byte_full_2_lasx;
            c->yuv2packed1 = yuv2rgb4_byte_full_1_lasx;
            break;
        case AV_PIX_FMT_BGR8:
            c->yuv2packedX = yuv2bgr8_full_X_lasx;
            c->yuv2packed2 = yuv2bgr8_full_2_lasx;
            c->yuv2packed1 = yuv2bgr8_full_1_lasx;
            break;
        case AV_PIX_FMT_RGB8:
            c->yuv2packedX = yuv2rgb8_full_X_lasx;
            c->yuv2packed2 = yuv2rgb8_full_2_lasx;
            c->yuv2packed1 = yuv2rgb8_full_1_lasx;
            break;
    }
    } else {
        switch (c->dstFormat) {
        case AV_PIX_FMT_RGB32:
        case AV_PIX_FMT_BGR32:
#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packed1 = yuv2rgbx32_1_lasx;
                c->yuv2packed2 = yuv2rgbx32_2_lasx;
                c->yuv2packedX = yuv2rgbx32_X_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_RGB32_1:
        case AV_PIX_FMT_BGR32_1:
#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
            if (c->needAlpha) {
            } else
#endif /* CONFIG_SWSCALE_ALPHA */
            {
                c->yuv2packed1 = yuv2rgbx32_1_1_lasx;
                c->yuv2packed2 = yuv2rgbx32_1_2_lasx;
                c->yuv2packedX = yuv2rgbx32_1_X_lasx;
            }
#endif /* !CONFIG_SMALL */
            break;
        case AV_PIX_FMT_RGB24:
            c->yuv2packed1 = yuv2rgb24_1_lasx;
            c->yuv2packed2 = yuv2rgb24_2_lasx;
            c->yuv2packedX = yuv2rgb24_X_lasx;
            break;
        case AV_PIX_FMT_BGR24:
            c->yuv2packed1 = yuv2bgr24_1_lasx;
            c->yuv2packed2 = yuv2bgr24_2_lasx;
            c->yuv2packedX = yuv2bgr24_X_lasx;
            break;
        case AV_PIX_FMT_RGB565LE:
        case AV_PIX_FMT_RGB565BE:
        case AV_PIX_FMT_BGR565LE:
        case AV_PIX_FMT_BGR565BE:
            c->yuv2packed1 = yuv2rgb16_1_lasx;
            c->yuv2packed2 = yuv2rgb16_2_lasx;
            c->yuv2packedX = yuv2rgb16_X_lasx;
            break;
        case AV_PIX_FMT_RGB555LE:
        case AV_PIX_FMT_RGB555BE:
        case AV_PIX_FMT_BGR555LE:
        case AV_PIX_FMT_BGR555BE:
            c->yuv2packed1 = yuv2rgb15_1_lasx;
            c->yuv2packed2 = yuv2rgb15_2_lasx;
            c->yuv2packedX = yuv2rgb15_X_lasx;
            break;
        case AV_PIX_FMT_RGB444LE:
        case AV_PIX_FMT_RGB444BE:
        case AV_PIX_FMT_BGR444LE:
        case AV_PIX_FMT_BGR444BE:
            c->yuv2packed1 = yuv2rgb12_1_lasx;
            c->yuv2packed2 = yuv2rgb12_2_lasx;
            c->yuv2packedX = yuv2rgb12_X_lasx;
            break;
        case AV_PIX_FMT_RGB8:
        case AV_PIX_FMT_BGR8:
            c->yuv2packed1 = yuv2rgb8_1_lasx;
            c->yuv2packed2 = yuv2rgb8_2_lasx;
            c->yuv2packedX = yuv2rgb8_X_lasx;
            break;
        case AV_PIX_FMT_RGB4:
        case AV_PIX_FMT_BGR4:
            c->yuv2packed1 = yuv2rgb4_1_lasx;
            c->yuv2packed2 = yuv2rgb4_2_lasx;
            c->yuv2packedX = yuv2rgb4_X_lasx;
            break;
        case AV_PIX_FMT_RGB4_BYTE:
        case AV_PIX_FMT_BGR4_BYTE:
            c->yuv2packed1 = yuv2rgb4b_1_lasx;
            c->yuv2packed2 = yuv2rgb4b_2_lasx;
            c->yuv2packedX = yuv2rgb4b_X_lasx;
            break;
        }
    }
}
