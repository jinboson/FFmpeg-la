/*
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

#ifndef AVCODEC_LOONGARCH_H264QPEL_LSX_H
#define AVCODEC_LOONGARCH_H264QPEL_LSX_H

#include <stdint.h>
#include <stddef.h>
#include "libavcodec/h264.h"

void put_h264_qpel8_hv_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                   ptrdiff_t dstStride, ptrdiff_t srcStride);
void put_h264_qpel8_h_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t dstStride, ptrdiff_t srcStride);
void put_h264_qpel8_v_lowpass_lsx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t dstStride, ptrdiff_t srcStride);
void put_pixels16_l2_8_lsx(uint8_t *dst, const uint8_t *src, uint8_t *half,
                           ptrdiff_t dstStride, ptrdiff_t srcStride);
void avg_h264_qpel8_v_lowpass_lsx(uint8_t *dst, uint8_t *src, int dstStride,
                                  int srcStride);

void ff_put_h264_qpel16_mc00_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc10_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc20_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc30_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc01_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc11_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc13_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc31_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc33_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc03_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc02_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc22_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_put_h264_qpel16_mc21_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride);
void ff_put_h264_qpel16_mc12_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride);
void ff_put_h264_qpel16_mc32_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride);
void ff_put_h264_qpel16_mc23_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride);

void ff_avg_h264_qpel16_mc00_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc10_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc30_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc33_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc11_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc31_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc13_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc20_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t dst_stride);
void ff_avg_h264_qpel16_mc02_lsx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride);
#endif  // #ifndef AVCODEC_LOONGARCH_H264QPEL_LSX_H
