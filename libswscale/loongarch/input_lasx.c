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
#include "libavutil/loongarch/generic_macros_lasx.h"

void planar_rgb_to_uv_lasx(uint8_t *_dstU, uint8_t *_dstV, const uint8_t *src[4],
                           int width, int32_t *rgb2yuv)
{
    int i;
    uint16_t *dstU   = (uint16_t *)_dstU;
    uint16_t *dstV   = (uint16_t *)_dstV;
    int set          = 0x4001 << (RGB2YUV_SHIFT - 7);
    int len          = width - 15;
    int32_t tem_ru   = rgb2yuv[RU_IDX], tem_gu = rgb2yuv[GU_IDX], tem_bu = rgb2yuv[BU_IDX];
    int32_t tem_rv   = rgb2yuv[RV_IDX], tem_gv = rgb2yuv[GV_IDX], tem_bv = rgb2yuv[BV_IDX];
    int shift        = RGB2YUV_SHIFT - 6;
    const uint8_t *src0 = src[0], *src1 = src[1], *src2 = src[2];
    __m256i ru, gu, bu, rv, gv, bv;
    __m256i mask = {0x0D0C090805040100, 0x1D1C191815141110, 0x0D0C090805040100, 0x1D1C191815141110};
    __m256i temp = __lasx_xvreplgr2vr_w(set);
    __m256i sra  = __lasx_xvreplgr2vr_w(shift);

    ru = __lasx_xvreplgr2vr_w(tem_ru);
    gu = __lasx_xvreplgr2vr_w(tem_gu);
    bu = __lasx_xvreplgr2vr_w(tem_bu);
    rv = __lasx_xvreplgr2vr_w(tem_rv);
    gv = __lasx_xvreplgr2vr_w(tem_gv);
    bv = __lasx_xvreplgr2vr_w(tem_bv);
    for (i = 0; i < len; i += 16) {
        __m256i _g, _b, _r;
        __m256i g_l, g_h, b_l, b_h, r_l, r_h;
        __m256i v_l, v_h, u_l, u_h, u_lh, v_lh;

        _g   = LASX_LD((src0 + i));
        _b   = LASX_LD((src1 + i));
        _r   = LASX_LD((src2 + i));
        LASX_UNPCK_L_WU_BU(_g, g_l);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_l, r_l);
        _g   = __lasx_xvpermi_d(_g, 0x01);
        _b   = __lasx_xvpermi_d(_b, 0x01);
        _r   = __lasx_xvpermi_d(_r, 0x01);
        LASX_UNPCK_L_WU_BU(_g, g_h);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_h, r_h);
        u_l  = __lasx_xvmadd_w(temp, ru, r_l);
        u_h  = __lasx_xvmadd_w(temp, ru, r_h);
        v_l  = __lasx_xvmadd_w(temp, rv, r_l);
        v_h  = __lasx_xvmadd_w(temp, rv, r_h);
        u_l  = __lasx_xvmadd_w(u_l, gu, g_l);
        u_l  = __lasx_xvmadd_w(u_l, bu, b_l);
        u_h  = __lasx_xvmadd_w(u_h, gu, g_h);
        u_h  = __lasx_xvmadd_w(u_h, bu, b_h);
        v_l  = __lasx_xvmadd_w(v_l, gv, g_l);
        v_l  = __lasx_xvmadd_w(v_l, bv, b_l);
        v_h  = __lasx_xvmadd_w(v_h, gv, g_h);
        v_h  = __lasx_xvmadd_w(v_h, bv, b_h);
        u_l  = __lasx_xvsra_w(u_l, sra);
        u_h  = __lasx_xvsra_w(u_h, sra);
        v_l  = __lasx_xvsra_w(v_l, sra);
        v_h  = __lasx_xvsra_w(v_h, sra);
        LASX_SHUF_B_2_128SV(u_h, u_l, v_h, v_l, mask, mask, u_lh, v_lh);
        u_lh = __lasx_xvpermi_d(u_lh, 0xD8);
        v_lh = __lasx_xvpermi_d(v_lh, 0xD8);
        __lasx_xvst(u_lh, (dstU + i), 0);
        __lasx_xvst(v_lh, (dstV + i), 0);
    }
    if (width - i >= 8) {
        __m256i _g, _b, _r;
        __m256i g_l, b_l, r_l;
        __m256i v_l, u_l, u, v;

        _g  = __lasx_xvldrepl_d((src0 + i), 0);
        _b  = __lasx_xvldrepl_d((src1 + i), 0);
        _r  = __lasx_xvldrepl_d((src2 + i), 0);
        LASX_UNPCK_L_WU_BU(_g, g_l);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_l, r_l);
        u_l = __lasx_xvmadd_w(temp, ru, r_l);
        v_l = __lasx_xvmadd_w(temp, rv, r_l);
        u_l = __lasx_xvmadd_w(u_l, gu, g_l);
        u_l = __lasx_xvmadd_w(u_l, bu, b_l);
        v_l = __lasx_xvmadd_w(v_l, gv, g_l);
        v_l = __lasx_xvmadd_w(v_l, bv, b_l);
        u_l = __lasx_xvsra_w(u_l, sra);
        v_l = __lasx_xvsra_w(v_l, sra);
        LASX_SHUF_B_2_128SV(u_l, u_l, v_l, v_l, mask, mask, u, v);
        __lasx_xvstelm_d(u, (dstU + i), 0, 0);
        __lasx_xvstelm_d(u, (dstU + i), 8, 2);
        __lasx_xvstelm_d(v, (dstV + i), 0, 0);
        __lasx_xvstelm_d(v, (dstV + i), 8, 2);
        i += 8;
    }
    for (; i < width; i++) {
        int g = src[0][i];
        int b = src[1][i];
        int r = src[2][i];

        dstU[i] = (tem_ru * r + tem_gu * g + tem_bu * b + set) >> shift;
        dstV[i] = (tem_rv * r + tem_gv * g + tem_bv * b + set) >> shift;
    }
}

void planar_rgb_to_y_lasx(uint8_t *_dst, const uint8_t *src[4], int width,
                          int32_t *rgb2yuv)
{
    int i;
    int shift        = (RGB2YUV_SHIFT - 6);
    int set          = 0x801 << (RGB2YUV_SHIFT - 7);
    int len          = width - 15;
    uint16_t *dst    = (uint16_t *)_dst;
    int32_t tem_ry   = rgb2yuv[RY_IDX], tem_gy = rgb2yuv[GY_IDX];
    int32_t tem_by   = rgb2yuv[BY_IDX];
    const uint8_t *src0 = src[0], *src1 = src[1], *src2 = src[2];
    __m256i mask = {0x0D0C090805040100, 0x1D1C191815141110, 0x0D0C090805040100, 0x1D1C191815141110};
    __m256i temp = __lasx_xvreplgr2vr_w(set);
    __m256i sra  = __lasx_xvreplgr2vr_w(shift);
    __m256i ry   = __lasx_xvreplgr2vr_w(tem_ry);
    __m256i gy   = __lasx_xvreplgr2vr_w(tem_gy);
    __m256i by   = __lasx_xvreplgr2vr_w(tem_by);

    for (i = 0; i < len; i += 16) {
        __m256i _g, _b, _r;
        __m256i g_l, g_h, b_l, b_h, r_l, r_h;
        __m256i y_l, y_h, y_lh;

        _g   = LASX_LD((src0 + i));
        _b   = LASX_LD((src1 + i));
        _r   = LASX_LD((src2 + i));
        LASX_UNPCK_L_WU_BU(_g, g_l);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_l, r_l);
        _g   = __lasx_xvpermi_d(_g, 0x01);
        _b   = __lasx_xvpermi_d(_b, 0x01);
        _r   = __lasx_xvpermi_d(_r, 0x01);
        LASX_UNPCK_L_WU_BU(_g, g_h);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_h, r_h);
        y_l  = __lasx_xvmadd_w(temp, ry, r_l);
        y_h  = __lasx_xvmadd_w(temp, ry, r_h);
        y_l  = __lasx_xvmadd_w(y_l, gy, g_l);
        y_l  = __lasx_xvmadd_w(y_l, by, b_l);
        y_h  = __lasx_xvmadd_w(y_h, gy, g_h);
        y_h  = __lasx_xvmadd_w(y_h, by, b_h);
        y_l  = __lasx_xvsra_w(y_l, sra);
        y_h  = __lasx_xvsra_w(y_h, sra);
        LASX_SHUF_B_128SV(y_h, y_l, mask, y_lh);
        y_lh = __lasx_xvpermi_d(y_lh, 0xD8);
        __lasx_xvst(y_lh, (dst + i), 0);
    }
    if (width - i >= 8) {
        __m256i _g, _b, _r;
        __m256i g_l, b_l, r_l;
        __m256i y_l, y;

        _g  = __lasx_xvldrepl_d((src0 + i), 0);
        _b  = __lasx_xvldrepl_d((src1 + i), 0);
        _r  = __lasx_xvldrepl_d((src2 + i), 0);
        LASX_UNPCK_L_WU_BU(_g, g_l);
        LASX_UNPCK_L_WU_BU_2(_b, _r, b_l, r_l);
        y_l = __lasx_xvmadd_w(temp, ry, r_l);
        y_l = __lasx_xvmadd_w(y_l, gy, g_l);
        y_l = __lasx_xvmadd_w(y_l, by, b_l);
        y_l = __lasx_xvsra_w(y_l, sra);
        LASX_SHUF_B_128SV(y_l, y_l, mask, y);
        __lasx_xvstelm_d(y, (dst + i), 0, 0);
        __lasx_xvstelm_d(y, (dst + i), 8, 2);
        i += 8;
    }
    for (; i < width; i++) {
        int g = src[0][i];
        int b = src[1][i];
        int r = src[2][i];

        dst[i] = (tem_ry * r + tem_gy * g + tem_by * b + set) >> shift;
    }
}
