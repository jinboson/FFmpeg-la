/*
 * Copyright (C) 2021 Loongson Technology Co. Ltd.
 * Contributed by Hao Chen(chenhao@loongson.cn)
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

#include "swscale_loongarch.h"
#include "libavutil/loongarch/generic_macros_lasx.h"

#define YUV2RGB_LOAD_COE                                     \
    /* Load x_offset */                                      \
    __m256i y_offset = __lasx_xvreplgr2vr_d(c->yOffset);     \
    __m256i u_offset = __lasx_xvreplgr2vr_d(c->uOffset);     \
    __m256i v_offset = __lasx_xvreplgr2vr_d(c->vOffset);     \
    /* Load x_coeff  */                                      \
    __m256i ug_coeff = __lasx_xvreplgr2vr_d(c->ugCoeff);     \
    __m256i vg_coeff = __lasx_xvreplgr2vr_d(c->vgCoeff);     \
    __m256i y_coeff  = __lasx_xvreplgr2vr_d(c->yCoeff);      \
    __m256i ub_coeff = __lasx_xvreplgr2vr_d(c->ubCoeff);     \
    __m256i vr_coeff = __lasx_xvreplgr2vr_d(c->vrCoeff);     \

#define LOAD_YUV_16                                          \
    m_y  = LASX_LD(py + (w << 4));                           \
    m_u  = __lasx_xvldrepl_d(pu + (w << 3), 0);              \
    m_v  = __lasx_xvldrepl_d(pv + (w << 3), 0);              \
    LASX_UNPCK_L_HU_BU(m_y, m_y);                            \
    LASX_UNPCK_L_HU_BU_2(m_u, m_v, m_u, m_v);                \

/* YUV2RGB method
 * The conversion method is as follows:
 * R = Y' * y_coeff + V' * vr_coeff
 * G = Y' * y_coeff + V' * vg_coeff + U' * ug_coeff
 * B = Y' * y_coeff + U' * ub_coeff
 *
 * where X' = X * 8 - x_offset
 *
 */

#define YUV2RGB                                                            \
    m_y = __lasx_xvslli_h(m_y, 3);                                         \
    m_u = __lasx_xvslli_h(m_u, 3);                                         \
    m_v = __lasx_xvslli_h(m_v, 3);                                         \
    m_y = __lasx_xvsub_h(m_y, y_offset);                                   \
    m_u = __lasx_xvsub_h(m_u, u_offset);                                   \
    m_v = __lasx_xvsub_h(m_v, v_offset);                                   \
    m_u = __lasx_xvshuf_h(shuf4, m_u, m_u);                                \
    m_v = __lasx_xvshuf_h(shuf4, m_v, m_v);                                \
    y_1 = __lasx_xvmuh_h(m_y, y_coeff);                                    \
    u2g = __lasx_xvmuh_h(m_u, ug_coeff);                                   \
    r   = __lasx_xvmadd_h(y_1, m_v, vr_coeff);                             \
    uvg = __lasx_xvmadd_h(u2g, m_v, vg_coeff);                             \
    b   = __lasx_xvmadd_h(y_1, m_u, ub_coeff);                             \
    g   = __lasx_xvsadd_h(y_1, uvg);                                       \
    LASX_CLIP_H_0_255_2(r, g, r, g);                                       \
    LASX_CLIP_H_0_255(b, b);                                               \

#define RGB_PACK_16(r, g, b, rgb_l, rgb_h)                                 \
{                                                                          \
    __m256i rg;                                                            \
    LASX_SHUF_B_128SV(g, r, shuf1, rg);                                    \
    LASX_SHUF_B_2_128SV(b, rg, b, rg, shuf2, shuf3, rgb_l, rgb_h);         \
}

#define RGB_STORE(rgb_l, rgb_h, image, w)                                      \
{                                                                              \
    uint8_t *index = image + (w * 48);                                         \
    __lasx_xvstelm_d(rgb_l, (index), 0,  0);                                   \
    __lasx_xvstelm_d(rgb_l, (index), 8,  1);                                   \
    __lasx_xvstelm_d(rgb_h, (index), 16, 0);                                   \
    __lasx_xvstelm_d(rgb_l, (index), 24, 2);                                   \
    __lasx_xvstelm_d(rgb_l, (index), 32, 3);                                   \
    __lasx_xvstelm_d(rgb_h, (index), 40, 2);                                   \
}

#define YUV2RGBFUNC(func_name, dst_type, alpha)                                     \
           int func_name(SwsContext *c, const uint8_t *src[],                       \
                         int srcStride[], int srcSliceY, int srcSliceH,             \
                         uint8_t *dst[], int dstStride[])                           \
{                                                                                   \
    int w, y, h_size, vshift;                                                       \
    __m256i m_y, m_u, m_v;                                                          \
    __m256i y_1, u2g, uvg, rgb_l, rgb_h;                                            \
    __m256i r, g, b;                                                                \
    __m256i shuf1 = {0x1606140412021000, 0x1E0E1C0C1A0A1808,                        \
                     0x1606140412021000, 0x1E0E1C0C1A0A1808};                       \
    __m256i shuf2 = {0x0504120302100100, 0x0A18090816060714,                        \
                     0x0504120302100100, 0x0A18090816060714};                       \
    __m256i shuf3 = {0x0B180C0D1C0E0F1E, 0X0101010101010101,                        \
                     0x0B180C0D1C0E0F1E, 0X0101010101010101};                       \
    __m256i shuf4 = {0x0001000100000000, 0x0003000300020002,                        \
                     0x0005000500040004, 0x0007000700060006};                       \
    YUV2RGB_LOAD_COE                                                                \
                                                                                    \
    h_size = c->dstW >> 4;                                                          \
    vshift = c->srcFormat != AV_PIX_FMT_YUV422P;                                    \
    for (y = 0; y < srcSliceH; y++) {                                               \
        dst_type *image   = dst[0] + (y + srcSliceY) * dstStride[0];                \
        const uint8_t *py = src[0] +               y * srcStride[0];                \
        const uint8_t *pu = src[1] +   (y >> vshift) * srcStride[1];                \
        const uint8_t *pv = src[2] +   (y >> vshift) * srcStride[2];                \
        for (w = 0; w < h_size; w ++) {                                             \

#define END_FUNC()                                                                  \
        }                                                                           \
    }                                                                               \
    return srcSliceH;                                                               \
}

YUV2RGBFUNC(yuv420_rgb24_lasx, uint8_t, 0)
    LOAD_YUV_16
    YUV2RGB
    RGB_PACK_16(r, g, b, rgb_l, rgb_h);
    RGB_STORE(rgb_l, rgb_h, image, w);
    END_FUNC()

YUV2RGBFUNC(yuv420_bgr24_lasx, uint8_t, 0)
    LOAD_YUV_16
    YUV2RGB
    RGB_PACK_16(b, g, r, rgb_l, rgb_h);
    RGB_STORE(rgb_l, rgb_h, image, w);
    END_FUNC()
