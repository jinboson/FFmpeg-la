/*
 * Copyright (C) 2023 Loongson Technology Co. Ltd.
 * Contributed by jinbo(jinbo@loongson.cn)
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

#define YUV2RGB_LOAD_COE                              \
    /* Load x_offset */                               \
    v8i16 y_offset = (v8i16)__msa_fill_d(c->yOffset); \
    v8i16 u_offset = (v8i16)__msa_fill_d(c->uOffset); \
    v8i16 v_offset = (v8i16)__msa_fill_d(c->vOffset); \
    /*Load x_coeff */                                 \
    v8i16 ug_coeff = (v8i16)__msa_fill_d(c->ugCoeff); \
    v8i16 vg_coeff = (v8i16)__msa_fill_d(c->vgCoeff); \
    v8i16 y_coeff  = (v8i16)__msa_fill_d(c->yCoeff);  \
    v8i16 ub_coeff = (v8i16)__msa_fill_d(c->ubCoeff); \
    v8i16 vr_coeff = (v8i16)__msa_fill_d(c->vrCoeff); \

#define LOAD_YUV_16                                      \
    m_y1   = LD_SB(py_1);                                \
    m_y2   = LD_SB(py_2);                                \
    m_u    = LD_SB(pu);                                  \
    m_v    = LD_SB(pv);                                  \
    m_u    = __msa_ilvr_b((v16i8)m_u, (v16i8)m_u);       \
    m_v    = __msa_ilvr_b((v16i8)m_v, (v16i8)m_v);       \
    m_u_l  = __msa_ilvl_b(zero, (v16i8)m_u);             \
    m_u    = __msa_ilvr_b(zero, (v16i8)m_u);             \
    m_v_l  = __msa_ilvl_b(zero, (v16i8)m_v);             \
    m_v    = __msa_ilvr_b(zero, (v16i8)m_v);             \
    m_y1_l = __msa_ilvl_b(zero, (v16i8)m_y1);            \
    m_y1   = __msa_ilvr_b(zero, (v16i8)m_y1);            \
    m_y2_l = __msa_ilvl_b(zero, (v16i8)m_y2);            \
    m_y2   = __msa_ilvr_b(zero, (v16i8)m_y2);            \

#define MUH_H(in0, in1, in2, in3, out0)                  \
{                                                        \
    v4i32 tmp0, tmp1;                                    \
    tmp0 = in0 * in1;                                    \
    tmp1 = in2 * in3;                                    \
    out0 = __msa_pckod_h((v8i16)tmp1, (v8i16)tmp0);      \
}

#define YUV2RGB(y1, y2, u, v, r1, g1, b1, r2, g2, b2)   \
{                                                       \
    y1 = (v16i8)__msa_slli_h((v8i16)y1, 3);             \
    y2 = (v16i8)__msa_slli_h((v8i16)y2, 3);             \
    u  = (v16i8)__msa_slli_h((v8i16)u, 3);              \
    v  = (v16i8)__msa_slli_h((v8i16)v, 3);              \
    y1 = (v16i8)__msa_subv_h((v8i16)y1, y_offset);      \
    y2 = (v16i8)__msa_subv_h((v8i16)y2, y_offset);      \
    u  = (v16i8)__msa_subv_h((v8i16)u, u_offset);       \
    v  = (v16i8)__msa_subv_h((v8i16)v, v_offset);       \
    UNPCK_SH_SW(y_coeff, y_coeff_r, y_coeff_l);         \
    UNPCK_SH_SW(y1, tmp_r, tmp_l);                      \
    MUH_H(tmp_r, y_coeff_r, tmp_l, y_coeff_l, y_1_r);   \
    UNPCK_SH_SW(y2, tmp_r, tmp_l);                      \
    MUH_H(tmp_r, y_coeff_r, tmp_l, y_coeff_l, y_2_r);   \
    UNPCK_SH_SW(ug_coeff, ug_coeff_r, ug_coeff_l);      \
    UNPCK_SH_SW(ub_coeff, ub_coeff_r, ub_coeff_l);      \
    UNPCK_SH_SW(u, tmp_r, tmp_l);                       \
    MUH_H(tmp_r, ug_coeff_r, tmp_l, ug_coeff_l, u2g_r); \
    MUH_H(tmp_r, ub_coeff_r, tmp_l, ub_coeff_l, u2b_r); \
    UNPCK_SH_SW(vr_coeff, vr_coeff_r, vr_coeff_l);      \
    UNPCK_SH_SW(vg_coeff, vg_coeff_r, vg_coeff_l);      \
    UNPCK_SH_SW(v, tmp_r, tmp_l);                       \
    MUH_H(tmp_r, vr_coeff_r, tmp_l, vr_coeff_l, v2r_r); \
    MUH_H(tmp_r, vg_coeff_r, tmp_l, vg_coeff_l, v2g_r); \
    r1    = __msa_adds_s_h(y_1_r, v2r_r);               \
    v2g_r = __msa_adds_s_h(v2g_r, u2g_r);               \
    g1    = __msa_adds_s_h(y_1_r, v2g_r);               \
    b1    = __msa_adds_s_h(y_1_r, u2b_r);               \
    r2    = __msa_adds_s_h(y_2_r, v2r_r);               \
    g2    = __msa_adds_s_h(y_2_r, v2g_r);               \
    b2    = __msa_adds_s_h(y_2_r, u2b_r);               \
    CLIP_SH4_0_255(r1, g1, b1, r2);                     \
    CLIP_SH2_0_255(g2, b2);                             \
}

#define RGB32_PACK(a, r, g, b, rgb_l, rgb_h)            \
{                                                       \
    v16i8 ra, bg;                                       \
    ra    = __msa_ilvev_b((v16i8)r, (v16i8)a);          \
    bg    = __msa_ilvev_b((v16i8)b, (v16i8)g);          \
    rgb_l = __msa_ilvr_h((v8i16)bg, (v8i16)ra);         \
    rgb_h = __msa_ilvl_h((v8i16)bg, (v8i16)ra);         \
}

#define RGB32_STORE(rgb_l, rgb_h, image)                \
{                                                       \
    ST_SH2(rgb_l, rgb_h, image, 4);                     \
}

#define YUV2RGBFUNC32(func_name, dst_type, alpha)                               \
           int func_name(SwsContext *c, const uint8_t *src[],                   \
                         int srcStride[], int srcSliceY, int srcSliceH,         \
                         uint8_t *dst[], int dstStride[])                       \
{                                                                               \
    int x, y, h_size, vshift, res;                                              \
    v16i8 m_y1, m_y2, m_u, m_v;                                                 \
    v16i8 m_u_l, m_v_l, m_y1_l, m_y2_l;                                         \
    v4i32 y_coeff_r, y_coeff_l, ug_coeff_r, ug_coeff_l;                         \
    v4i32 ub_coeff_r, ub_coeff_l, vr_coeff_r, vr_coeff_l;                       \
    v4i32 vg_coeff_r, vg_coeff_l, tmp_r, tmp_l;                                 \
    v8i16 y_1_r, y_2_r, u2g_r, v2g_r, u2b_r, v2r_r, rgb1_l, rgb1_h;             \
    v8i16 rgb2_l, rgb2_h, r1, g1, b1, r2, g2, b2;                               \
    v16u8 a = (v16u8)__msa_fill_b(0xFF);                                        \
    v16i8 zero = __msa_fill_b(0);                                               \
                                                                                \
    YUV2RGB_LOAD_COE                                                            \
                                                                                \
    h_size = c->dstW >> 4;                                                      \
    res = (c->dstW & 15) >> 1;                                                  \
    vshift = c->srcFormat != AV_PIX_FMT_YUV422P;                                \
    for (y = 0; y < srcSliceH; y += 2) {                                        \
        int yd = y + srcSliceY;                                                 \
        dst_type av_unused *r, *g, *b;                                          \
        dst_type *image1    = (dst_type *)(dst[0] + (yd)     * dstStride[0]);   \
        dst_type *image2    = (dst_type *)(dst[0] + (yd + 1) * dstStride[0]);   \
        const uint8_t *py_1 = src[0] +               y * srcStride[0];          \
        const uint8_t *py_2 = py_1   +                   srcStride[0];          \
        const uint8_t *pu   = src[1] +   (y >> vshift) * srcStride[1];          \
        const uint8_t *pv   = src[2] +   (y >> vshift) * srcStride[2];          \
        for(x = 0; x < h_size; x++) {                                           \

#define DEALYUV2RGBREMAIN32                                                     \
            py_1 += 16;                                                         \
            py_2 += 16;                                                         \
            pu += 8;                                                            \
            pv += 8;                                                            \
            image1 += 16;                                                       \
            image2 += 16;                                                       \
        }                                                                       \
        for (x = 0; x < res; x++) {                                             \
            int av_unused U, V, Y;                                              \
            U = pu[0];                                                          \
            V = pv[0];                                                          \
            r = (void *)c->table_rV[V+YUVRGB_TABLE_HEADROOM];                   \
            g = (void *)(c->table_gU[U+YUVRGB_TABLE_HEADROOM]                   \
                       + c->table_gV[V+YUVRGB_TABLE_HEADROOM]);                 \
            b = (void *)c->table_bU[U+YUVRGB_TABLE_HEADROOM];                   \

#define PUTRGB(dst, src)                    \
    Y      = src[0];                        \
    dst[0] = r[Y] + g[Y] + b[Y];            \
    Y      = src[1];                        \
    dst[1] = r[Y] + g[Y] + b[Y];            \

#define ENDRES32                            \
    pu += 1;                                \
    pv += 1;                                \
    py_1 += 2;                              \
    py_2 += 2;                              \
    image1 += 2;                            \
    image2 += 2;                            \

#define END_FUNC()                          \
        }                                   \
    }                                       \
    return srcSliceH;                       \
}

YUV2RGBFUNC32(yuv420_rgba32_msa, uint32_t, 0)
    LOAD_YUV_16
    YUV2RGB(m_y1, m_y2, m_u, m_v, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(r1, g1, b1, a, rgb1_l, rgb1_h);
    RGB32_PACK(r2, g2, b2, a, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1);
    RGB32_STORE(rgb2_l, rgb2_h, image2);
    YUV2RGB(m_y1_l, m_y2_l, m_u_l, m_v_l, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(r1, g1, b1, a, rgb1_l, rgb1_h);
    RGB32_PACK(r2, g2, b2, a, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1 + 8);
    RGB32_STORE(rgb2_l, rgb2_h, image2 + 8);
    DEALYUV2RGBREMAIN32
    PUTRGB(image1, py_1);
    PUTRGB(image2, py_2);
    ENDRES32
    END_FUNC()

YUV2RGBFUNC32(yuv420_bgra32_msa, uint32_t, 0)
    LOAD_YUV_16
    YUV2RGB(m_y1, m_y2, m_u, m_v, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(b1, g1, r1, a, rgb1_l, rgb1_h);
    RGB32_PACK(b2, g2, r2, a, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1);
    RGB32_STORE(rgb2_l, rgb2_h, image2);
    YUV2RGB(m_y1_l, m_y2_l, m_u_l, m_v_l, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(b1, g1, r1, a, rgb1_l, rgb1_h);
    RGB32_PACK(b2, g2, r2, a, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1 + 8);
    RGB32_STORE(rgb2_l, rgb2_h, image2 + 8);
    DEALYUV2RGBREMAIN32
    PUTRGB(image1, py_1);
    PUTRGB(image2, py_2);
    ENDRES32
    END_FUNC()

YUV2RGBFUNC32(yuv420_argb32_msa, uint32_t, 0)
    LOAD_YUV_16
    YUV2RGB(m_y1, m_y2, m_u, m_v, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(a, r1, g1, b1, rgb1_l, rgb1_h);
    RGB32_PACK(a, r2, g2, b2, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1);
    RGB32_STORE(rgb2_l, rgb2_h, image2);
    YUV2RGB(m_y1_l, m_y2_l, m_u_l, m_v_l, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(a, r1, g1, b1, rgb1_l, rgb1_h);
    RGB32_PACK(a, r2, g2, b2, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1 + 8);
    RGB32_STORE(rgb2_l, rgb2_h, image2 + 8);
    DEALYUV2RGBREMAIN32
    PUTRGB(image1, py_1);
    PUTRGB(image2, py_2);
    ENDRES32
    END_FUNC()

YUV2RGBFUNC32(yuv420_abgr32_msa, uint32_t, 0)
    LOAD_YUV_16
    YUV2RGB(m_y1, m_y2, m_u, m_v, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(a, b1, g1, r1, rgb1_l, rgb1_h);
    RGB32_PACK(a, b2, g2, r2, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1);
    RGB32_STORE(rgb2_l, rgb2_h, image2);
    YUV2RGB(m_y1_l, m_y2_l, m_u_l, m_v_l, r1, g1, b1, r2, g2, b2);
    RGB32_PACK(a, b1, g1, r1, rgb1_l, rgb1_h);
    RGB32_PACK(a, b2, g2, r2, rgb2_l, rgb2_h);
    RGB32_STORE(rgb1_l, rgb1_h, image1 + 8);
    RGB32_STORE(rgb2_l, rgb2_h, image2 + 8);
    DEALYUV2RGBREMAIN32
    PUTRGB(image1, py_1);
    PUTRGB(image2, py_2);
    ENDRES32
    END_FUNC()
