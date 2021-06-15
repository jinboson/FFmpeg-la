/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Hao Chen <chenhao@loongson.cn>
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

#include "libavcodec/vp9dsp.h"
#include "libavutil/loongarch/generic_macros_lsx.h"
#include "vp9dsp_loongarch.h"

#define LSX_ST_8(_dst0, _dst1, _dst2, _dst3, _dst4,   \
                 _dst5, _dst6, _dst7, _dst, _stride)  \
{                                                     \
    __lsx_vst(_dst0, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst1, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst2, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst3, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst4, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst5, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst6, _dst, 0);                        \
    _dst += _stride;                                  \
    __lsx_vst(_dst7, _dst, 0);                        \
    _dst += _stride;                                  \
}

#define LSX_ST_8X16(_dst0, _dst1, _dst2, _dst3, _dst4,   \
                 _dst5, _dst6, _dst7, _dst, _stride)     \
{                                                        \
    __lsx_vst(_dst0, _dst, 0);                           \
    __lsx_vst(_dst0, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst1, _dst, 0);                           \
    __lsx_vst(_dst1, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst2, _dst, 0);                           \
    __lsx_vst(_dst2, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst3, _dst, 0);                           \
    __lsx_vst(_dst3, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst4, _dst, 0);                           \
    __lsx_vst(_dst4, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst5, _dst, 0);                           \
    __lsx_vst(_dst5, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst6, _dst, 0);                           \
    __lsx_vst(_dst6, _dst, 16);                          \
    _dst += _stride;                                     \
    __lsx_vst(_dst7, _dst, 0);                           \
    __lsx_vst(_dst7, _dst, 16);                          \
    _dst += _stride;                                     \
}

void ff_vert_16x16_lsx(uint8_t *dst, ptrdiff_t dst_stride, const uint8_t *left,
                       const uint8_t *src)
{
    __m128i src0;

    src0 = __lsx_vld(src, 0);
    LSX_ST_8(src0, src0, src0, src0, src0, src0, src0, src0, dst, dst_stride);
    LSX_ST_8(src0, src0, src0, src0, src0, src0, src0, src0, dst, dst_stride);
}

void ff_vert_32x32_lsx(uint8_t *dst, ptrdiff_t dst_stride, const uint8_t *left,
                       const uint8_t *src)
{
    uint32_t row;
    __m128i src0, src1;

    LSX_DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
    for (row = 32; row--;) {
        __lsx_vst(src0, dst, 0);
        __lsx_vst(src1, dst, 16);
        dst += dst_stride;
    }
}

void ff_hor_16x16_lsx(uint8_t *dst, ptrdiff_t dst_stride, const uint8_t *src,
                      const uint8_t *top)
{
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i src8, src9, src10, src11, src12, src13, src14, src15;

    src15 = __lsx_vldrepl_b(src, 0);
    src14 = __lsx_vldrepl_b(src, 1);
    src13 = __lsx_vldrepl_b(src, 2);
    src12 = __lsx_vldrepl_b(src, 3);
    src11 = __lsx_vldrepl_b(src, 4);
    src10 = __lsx_vldrepl_b(src, 5);
    src9  = __lsx_vldrepl_b(src, 6);
    src8  = __lsx_vldrepl_b(src, 7);
    src7  = __lsx_vldrepl_b(src, 8);
    src6  = __lsx_vldrepl_b(src, 9);
    src5  = __lsx_vldrepl_b(src, 10);
    src4  = __lsx_vldrepl_b(src, 11);
    src3  = __lsx_vldrepl_b(src, 12);
    src2  = __lsx_vldrepl_b(src, 13);
    src1  = __lsx_vldrepl_b(src, 14);
    src0  = __lsx_vldrepl_b(src, 15);
    LSX_ST_8(src0, src1, src2, src3, src4, src5, src6, src7, dst, dst_stride);
    LSX_ST_8(src8, src9, src10, src11, src12, src13, src14, src15, dst, dst_stride);
}

void ff_hor_32x32_lsx(uint8_t *dst, ptrdiff_t dst_stride, const uint8_t *src,
                      const uint8_t *top)
{
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i src8, src9, src10, src11, src12, src13, src14, src15;
    __m128i src16, src17, src18, src19, src20, src21, src22, src23;
    __m128i src24, src25, src26, src27, src28, src29, src30, src31;

    src31 = __lsx_vldrepl_b(src, 0);
    src30 = __lsx_vldrepl_b(src, 1);
    src29 = __lsx_vldrepl_b(src, 2);
    src28 = __lsx_vldrepl_b(src, 3);
    src27 = __lsx_vldrepl_b(src, 4);
    src26 = __lsx_vldrepl_b(src, 5);
    src25 = __lsx_vldrepl_b(src, 6);
    src24 = __lsx_vldrepl_b(src, 7);
    src23 = __lsx_vldrepl_b(src, 8);
    src22 = __lsx_vldrepl_b(src, 9);
    src21 = __lsx_vldrepl_b(src, 10);
    src20 = __lsx_vldrepl_b(src, 11);
    src19 = __lsx_vldrepl_b(src, 12);
    src18 = __lsx_vldrepl_b(src, 13);
    src17 = __lsx_vldrepl_b(src, 14);
    src16 = __lsx_vldrepl_b(src, 15);
    src15 = __lsx_vldrepl_b(src, 16);
    src14 = __lsx_vldrepl_b(src, 17);
    src13 = __lsx_vldrepl_b(src, 18);
    src12 = __lsx_vldrepl_b(src, 19);
    src11 = __lsx_vldrepl_b(src, 20);
    src10 = __lsx_vldrepl_b(src, 21);
    src9  = __lsx_vldrepl_b(src, 22);
    src8  = __lsx_vldrepl_b(src, 23);
    src7  = __lsx_vldrepl_b(src, 24);
    src6  = __lsx_vldrepl_b(src, 25);
    src5  = __lsx_vldrepl_b(src, 26);
    src4  = __lsx_vldrepl_b(src, 27);
    src3  = __lsx_vldrepl_b(src, 28);
    src2  = __lsx_vldrepl_b(src, 29);
    src1  = __lsx_vldrepl_b(src, 30);
    src0  = __lsx_vldrepl_b(src, 31);
    LSX_ST_8X16(src0, src1, src2, src3, src4, src5, src6, src7, dst, dst_stride);
    LSX_ST_8X16(src8, src9, src10, src11, src12, src13, src14, src15, dst, dst_stride);
    LSX_ST_8X16(src16, src17, src18, src19, src20, src21, src22, src23, dst, dst_stride);
    LSX_ST_8X16(src24, src25, src26, src27, src28, src29, src30, src31, dst, dst_stride);
}
