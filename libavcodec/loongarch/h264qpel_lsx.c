/*
 * Loongson LSX optimized h264qpel
 *
 * Copyright (c) 2020 Loongson Technology Corporation Limited
 * Contributed by Hecai Yuan <yuanhecai@loongson.cn>
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

#include "h264qpel_lsx.h"
#include "libavutil/loongarch/loongson_intrinsics.h"
#include "libavutil/attributes.h"

static void put_h264_qpel16_hv_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                           ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    put_h264_qpel8_hv_lowpass_lsx(dst, src, dstStride, srcStride);
    put_h264_qpel8_hv_lowpass_lsx(dst + 8, src + 8, dstStride, srcStride);
    src += srcStride << 3;
    dst += dstStride << 3;
    put_h264_qpel8_hv_lowpass_lsx(dst, src, dstStride, srcStride);
    put_h264_qpel8_hv_lowpass_lsx(dst + 8, src + 8, dstStride, srcStride);
}

void ff_put_h264_qpel16_mc22_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    put_h264_qpel16_hv_lowpass_lsx(dst, src, stride, stride);
}

static void put_h264_qpel16_h_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                          int dstStride, int srcStride)
{
    put_h264_qpel8_h_lowpass_lsx(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_lsx(dst+8, src+8, dstStride, srcStride);
    src += srcStride << 3;
    dst += dstStride << 3;
    put_h264_qpel8_h_lowpass_lsx(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_lsx(dst+8, src+8, dstStride, srcStride);
}

static void put_h264_qpel16_v_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                           int dstStride, int srcStride)
{
    put_h264_qpel8_v_lowpass_lsx(dst, (uint8_t*)src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_lsx(dst+8, (uint8_t*)src+8, dstStride, srcStride);
    src += 8*srcStride;
    dst += 8*dstStride;
    put_h264_qpel8_v_lowpass_lsx(dst, (uint8_t*)src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_lsx(dst+8, (uint8_t*)src+8, dstStride, srcStride);
}

void ff_put_h264_qpel16_mc21_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lsx(halfH, src, 16, stride);
    put_h264_qpel16_hv_lowpass_lsx(halfHV, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc12_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lsx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lsx(halfH, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc32_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lsx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lsx(halfH, src + 1, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc23_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lsx(halfH, src + stride, 16, stride);
    put_h264_qpel16_hv_lowpass_lsx(halfHV, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}
