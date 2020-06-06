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

#include "rgb2rgb_mips.h"
#include "libavutil/mips/generic_macros_msa.h"

void ff_interleave_bytes_msa(const uint8_t *src1, const uint8_t *src2,
                             uint8_t *dest, int width, int height,
                             int src1Stride, int src2Stride, int dstStride)
{
    int h;
    int len  = width >> 4;
    int part = len << 4;
    for (h = 0; h < height; h++) {
        int w;
        v16u8 src_0, src_1;
        v16u8 dst_0, dst_1;
        for (w = 0; w < len; w++) {
            src_0 = LD_UB(src1 + w * 16);
            src_1 = LD_UB(src2 + w * 16);
            ILVRL_B2_UB(src_1, src_0, dst_0, dst_1);
            ST_UB2(dst_0, dst_1, dest + w * 32, 16);
        }
        for (w = part; w < width; w++) {
            dest[2 * w + 0] = src1[w];
            dest[2 * w + 1] = src2[w];
        }
        dest += dstStride;
        src1 += src1Stride;
        src2 += src2Stride;
    }
}
