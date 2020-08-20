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
    int i, j;
    if (filterSize >= 8) {
        int len  = filterSize >> 3;
        int part = len << 3;
        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;
            int val = 0;
            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + j * 8);
                filter0 = LD_V(v8i16, filterStart + j * 8);
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
    } else if (filterSize == 4 && (dstW & 0x01) == 0) {
        for (i = 0; i < dstW; i += 2) {
            v8i16 src1, src2, src3;
            v8i16 filter0;
            v4i32 out0;
            int val1 = 0;
            int val2 = 0;
            v16i8 zero = {0};
            src1 = LD_V(v8i16, src + filterPos[i]);
            src2 = LD_V(v8i16, src + filterPos[i + 1]);
            filter0 = LD_V(v8i16, filter + i * 4);
            src1 = (v8i16)__msa_ilvr_b(zero, (v16i8)src1);
            src2 = (v8i16)__msa_ilvr_b(zero, (v16i8)src2);
            src3 = (v8i16)__msa_ilvr_d((v2i64)src2, (v2i64)src1);
            out0 = (v4i32)__msa_dotp_s_w(src3, filter0);
            val1 = __msa_copy_s_w(out0, 0) + __msa_copy_s_w(out0, 1);
            val2 = __msa_copy_s_w(out0, 2) + __msa_copy_s_w(out0, 3);
            dst[i] = FFMIN(val1 >> 7, (1 << 15) - 1);
            dst[i + 1] = FFMIN(val2 >> 7, (1 << 15) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++){
            int val = 0;
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;
            for (j = 0; j < filterSize; j++) {
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
