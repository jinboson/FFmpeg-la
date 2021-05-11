/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Lu Wang <wanglu@loongson.cn>
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

#include "libavutil/loongarch/generic_macros_lsx.h"
#include "libavcodec/loongarch/hevcdsp_lsx.h"

static void hevc_copy_4w_lsx(uint8_t *src, int32_t src_stride,
                             int16_t *dst, int32_t dst_stride,
                             int32_t height)
{
    __m128i zero = {0};
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;

    if (2 == height) {
        __m128i src0, src1;
        __m128i in0;

        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
        src0 = __lsx_vilvl_w(src1, src0);
        in0 = __lsx_vilvl_b(zero, src0);
        in0 = __lsx_vslli_h(in0, 6);
        __lsx_vstelm_d(in0, dst, 0, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 0, 1);
    } else if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0, in1;

        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
        LSX_DUP2_ARG2(__lsx_vilvl_w, src1, src0, src3, src2, src0, src1);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        LSX_DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vstelm_d(in0, dst, 0, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(in1, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(in1, dst + dst_stride_3x, 0, 1);
    } else if (0 == (height & 0x07)) {
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i in0, in1, in2, in3;
        uint32_t loop_cnt;
        for (loop_cnt = (height >> 3); loop_cnt--;) {
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,
                          src + src_stride_2x, 0, src + src_stride_3x, 0,
                          src0, src1, src2, src3);
            src += src_stride_4x;
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,src + src_stride_2x,
                          0, src + src_stride_3x, 0, src4, src5, src6, src7);
            src += src_stride_4x;

            LSX_DUP4_ARG2(__lsx_vilvl_w, src1, src0, src3, src2, src5, src4, src7, src6,
                          src0, src1, src2, src3);
            LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                          in0, in1, in2, in3);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1,
                          in2, in3);

            __lsx_vstelm_d(in0, dst, 0, 0);
            __lsx_vstelm_d(in0, dst + dst_stride, 0, 1);
            __lsx_vstelm_d(in1, dst + dst_stride_2x, 0, 0);
            __lsx_vstelm_d(in1, dst + dst_stride_3x, 0, 1);
            dst += dst_stride_4x;
            __lsx_vstelm_d(in2, dst, 0, 0);
            __lsx_vstelm_d(in2, dst + dst_stride, 0, 1);
            __lsx_vstelm_d(in3, dst + dst_stride_2x, 0, 0);
            __lsx_vstelm_d(in3, dst + dst_stride_3x, 0, 1);
            dst += dst_stride_4x;
        }
    }
}

static void hevc_copy_6w_lsx(uint8_t *src, int32_t src_stride,
                             int16_t *dst, int32_t dst_stride,
                             int32_t height)
{
    uint32_t loop_cnt;
    uint64_t out0_m, out1_m, out2_m, out3_m;
    uint64_t out4_m, out5_m, out6_m, out7_m;
    uint32_t out8_m, out9_m, out10_m, out11_m;
    uint32_t out12_m, out13_m, out14_m, out15_m;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;

    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in4, in5, in6, in7);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in6, 6, in7, 6, in4, in5, in6, in7);

        LSX_DUP4_ARG2(__lsx_vpickve2gr_du, in0, 0, in1, 0, in2, 0, in3, 0, out0_m,
                      out1_m, out2_m, out3_m);
        LSX_DUP4_ARG2(__lsx_vpickve2gr_du, in4, 0, in5, 0, in6, 0, in7, 0, out4_m,
                      out5_m, out6_m, out7_m);

        LSX_DUP4_ARG2(__lsx_vpickve2gr_wu, in0, 2, in1, 2, in2, 2, in3, 2, out8_m,
                      out9_m, out10_m, out11_m);
        LSX_DUP4_ARG2(__lsx_vpickve2gr_wu, in4, 2, in5, 2, in6, 2, in7, 2, out12_m,
                      out13_m, out14_m, out15_m);

        *(uint64_t *)dst = out0_m;
        *(uint32_t *)(dst + 4) = out8_m;
        dst += dst_stride;
        *(uint64_t *)dst = out1_m;
        *(uint32_t *)(dst + 4) = out9_m;
        dst += dst_stride;
        *(uint64_t *)dst = out2_m;
        *(uint32_t *)(dst + 4) = out10_m;
        dst += dst_stride;
        *(uint64_t *)dst = out3_m;
        *(uint32_t *)(dst + 4) = out11_m;
        dst += dst_stride;
        *(uint64_t *)dst = out4_m;
        *(uint32_t *)(dst + 4) = out12_m;
        dst += dst_stride;
        *(uint64_t *)dst = out5_m;
        *(uint32_t *)(dst + 4) = out13_m;
        dst += dst_stride;
        *(uint64_t *)dst = out6_m;
        *(uint32_t *)(dst + 4) = out14_m;
        dst += dst_stride;
        *(uint64_t *)dst = out7_m;
        *(uint32_t *)(dst + 4) = out15_m;
        dst += dst_stride;
    }
}

static void hevc_copy_8w_lsx(uint8_t *src, int32_t src_stride,
                             int16_t *dst, int32_t dst_stride,
                             int32_t height)
{
    __m128i zero = {0};
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;

    if (2 == height) {
        __m128i src0, src1;
        __m128i in0, in1;

        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);

        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        LSX_DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0, dst, 0);
        __lsx_vst(in1, dst + dst_stride, 0);
    } else if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0, in1, in2, in3;

        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        __lsx_vst(in0, dst, 0);
        __lsx_vst(in1, dst + dst_stride, 0);
        __lsx_vst(in2, dst + dst_stride_2x, 0);
        __lsx_vst(in3, dst + dst_stride_3x, 0);
    } else if (6 == height) {
        __m128i src0, src1, src2, src3, src4, src5;
        __m128i in0, in1, in2, in3, in4, in5;

        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3, in0,
                      in1, in2, in3);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, in4, in5);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        LSX_DUP2_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in4, in5);
        __lsx_vst(in0, dst, 0);
        __lsx_vst(in1, dst + dst_stride, 0);
        __lsx_vst(in2, dst + dst_stride_2x, 0);
        __lsx_vst(in3, dst + dst_stride_3x, 0);
        __lsx_vst(in4, dst + dst_stride_4x, 0);
        __lsx_vst(in5, dst + dst_stride_4x + dst_stride, 0);
    } else if (0 == (height & 0x07)) {
        uint32_t loop_cnt;
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i in0, in1, in2, in3, in4, in5, in6, in7;

        for (loop_cnt = (height >> 3); loop_cnt--;) {
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,
                          src + src_stride_2x, 0, src + src_stride_3x, 0, src0,
                          src1, src2, src3);
            src += src_stride_4x;
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,
                          src + src_stride_2x, 0, src + src_stride_3x, 0, src4,
                          src5, src6, src7);
            src += src_stride_4x;

            LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                          in0, in1, in2, in3);
            LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                          in4, in5, in6, in7);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
            LSX_DUP4_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in6, 6, in7, 6, in4, in5, in6, in7);
            __lsx_vst(in0, dst, 0);
            __lsx_vst(in1, dst + dst_stride, 0);
            __lsx_vst(in2, dst + dst_stride_2x, 0);
            __lsx_vst(in3, dst + dst_stride_3x, 0);
            dst += dst_stride_4x;
            __lsx_vst(in4, dst, 0);
            __lsx_vst(in5, dst + dst_stride, 0);
            __lsx_vst(in6, dst + dst_stride_2x, 0);
            __lsx_vst(in7, dst + dst_stride_3x, 0);
            dst += dst_stride_4x;
        }
    }
}

static void hevc_copy_12w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0, in1, in0_r, in1_r, in2_r, in3_r;

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                       in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP2_ARG2(__lsx_vilvh_w, src1, src0, src3, src2, src0, src1);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        LSX_DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(in0, dst + 8, 0, 0);
        __lsx_vstelm_d(in0, dst + 8 + dst_stride, 0, 1);
        __lsx_vstelm_d(in1, dst + 8 + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(in1, dst + 8 + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP2_ARG2(__lsx_vilvh_w, src5, src4, src7, src6, src0, src1);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        LSX_DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(in0, dst + 8, 0, 0);
        __lsx_vstelm_d(in0, dst + 8 + dst_stride, 0, 1);
        __lsx_vstelm_d(in1, dst + 8 + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(in1, dst + 8 + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;
    }
}

static void hevc_copy_16w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    __m128i zero = {0};
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;

    if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0_r, in1_r, in2_r, in3_r;
        __m128i in0_l, in1_l, in2_l, in3_l;

        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
        __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
        __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
   } else if (12 == height) {
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i src8, src9, src10, src11;
        __m128i in0_r, in1_r, in2_r, in3_r;
        __m128i in0_l, in1_l, in2_l, in3_l;

        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0,src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src8, src9, src10, src11);

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
        __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
        __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
        dst += dst_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
        __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
        __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
        dst += dst_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src8, zero, src9, zero, src10, zero, src11,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src8, zero, src9, zero, src10, zero, src11,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
        __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
        __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
    } else if (0 == (height & 0x07)) {
        uint32_t loop_cnt;
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

        for (loop_cnt = (height >> 3); loop_cnt--;) {
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,
                          src + src_stride_2x, 0, src + src_stride_3x, 0, src0,
                          src1, src2, src3);
            src += src_stride_4x;
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,
                          src + src_stride_2x, 0, src + src_stride_3x, 0, src4,
                          src5, src6, src7);
            src += src_stride_4x;
            LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                          in0_r, in1_r, in2_r, in3_r);
            LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                          in0_l, in1_l, in2_l, in3_l);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                          in1_r, in2_r, in3_r);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                          in1_l, in2_l, in3_l);
            __lsx_vst(in0_r, dst, 0);
            __lsx_vst(in1_r, dst + dst_stride, 0);
            __lsx_vst(in2_r, dst + dst_stride_2x, 0);
            __lsx_vst(in3_r, dst + dst_stride_3x, 0);
            __lsx_vst(in0_l, dst + 8, 0);
            __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
            __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
            __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
            dst += dst_stride_4x;

            LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                          in0_r, in1_r, in2_r, in3_r);
            LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                          in0_l, in1_l, in2_l, in3_l);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                          in1_r, in2_r, in3_r);
            LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                          in1_l, in2_l, in3_l);
            __lsx_vst(in0_r, dst, 0);
            __lsx_vst(in1_r, dst + dst_stride, 0);
            __lsx_vst(in2_r, dst + dst_stride_2x, 0);
            __lsx_vst(in3_r, dst + dst_stride_3x, 0);
            __lsx_vst(in0_l, dst + 8, 0);
            __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
            __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
            __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
            dst += dst_stride_4x;
        }
    }
}

static void hevc_copy_24w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
        LSX_DUP4_ARG2(__lsx_vld, src + 16, 0, src + 16 + src_stride, 0,
                      src + 16 + src_stride_2x, 0, src + 16 + src_stride_3x, 0,
                      src4, src5, src6, src7);
        src += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_l, dst + 8 + dst_stride, 0);
        __lsx_vst(in2_l, dst + 8 + dst_stride_2x, 0);
        __lsx_vst(in3_l, dst + 8 + dst_stride_3x, 0);
        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        __lsx_vst(in0_r, dst + 16, 0);
        __lsx_vst(in1_r, dst + 16 + dst_stride, 0);
        __lsx_vst(in2_r, dst + 16 + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + 16 + dst_stride_3x, 0);
        dst += dst_stride_4x;
    }
}

static void hevc_copy_32w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src2, src4, src6);
        LSX_DUP4_ARG2(__lsx_vld, src + 16, 0, src + 16 + src_stride, 0,
                      src + 16 + src_stride_2x, 0, src + 16 + src_stride_3x, 0,
                      src1, src3, src5, src7);
        src += src_stride_4x;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        dst += dst_stride;
        __lsx_vst(in2_r, dst, 0);
        __lsx_vst(in2_l, dst + 8, 0);
        __lsx_vst(in3_r, dst + 16, 0);
        __lsx_vst(in3_l, dst + 24, 0);
        dst += dst_stride;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        dst += dst_stride;
        __lsx_vst(in2_r, dst, 0);
        __lsx_vst(in2_l, dst + 8, 0);
        __lsx_vst(in3_r, dst + 16, 0);
        __lsx_vst(in3_l, dst + 24, 0);
        dst += dst_stride;
    }
}

static void hevc_copy_48w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    uint32_t loop_cnt;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i src8, src9, src10, src11;
    __m128i in0_r, in1_r, in2_r, in3_r, in4_r, in5_r;
    __m128i in0_l, in1_l, in2_l, in3_l, in4_l, in5_l;

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + 16, 0, src0, src1);
        src2 = __lsx_vld(src + 32, 0);
        src += src_stride;
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + 16, 0, src3, src4);
        src5 = __lsx_vld(src + 32, 0);
        src += src_stride;
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + 16, 0, src6, src7);
        src8 = __lsx_vld(src + 32, 0);
        src += src_stride;
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + 16, 0, src9, src10);
        src11 = __lsx_vld(src + 32, 0);
        src += src_stride;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, in4_r, in5_r);
        LSX_DUP2_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, in4_l, in5_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in4_r, 6, in5_r, 6, in4_l, 6, in5_l, 6, in4_r,
                      in5_r, in4_l, in5_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        __lsx_vst(in2_r, dst + 32, 0);
        __lsx_vst(in2_l, dst + 40, 0);
        dst += dst_stride;
        __lsx_vst(in3_r, dst, 0);
        __lsx_vst(in3_l, dst + 8, 0);
        __lsx_vst(in4_r, dst + 16, 0);
        __lsx_vst(in4_l, dst + 24, 0);
        __lsx_vst(in5_r, dst + 32, 0);
        __lsx_vst(in5_l, dst + 40, 0);
        dst += dst_stride;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src6, zero, src7, zero, src8, zero, src9,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src6, zero, src7, zero, src8, zero, src9,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP2_ARG2(__lsx_vilvl_b, zero, src10, zero, src11, in4_r, in5_r);
        LSX_DUP2_ARG2(__lsx_vilvh_b, zero, src10, zero, src11, in4_l, in5_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in4_r, 6, in5_r, 6, in4_l, 6, in5_l, 6, in4_r,
                      in5_r, in4_l, in5_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        __lsx_vst(in2_r, dst + 32, 0);
        __lsx_vst(in2_l, dst + 40, 0);
        dst += dst_stride;
        __lsx_vst(in3_r, dst, 0);
        __lsx_vst(in3_l, dst + 8, 0);
        __lsx_vst(in4_r, dst + 16, 0);
        __lsx_vst(in4_l, dst + 24, 0);
        __lsx_vst(in5_r, dst + 32, 0);
        __lsx_vst(in5_l, dst + 40, 0);
        dst += dst_stride;
    }
}

static void hevc_copy_64w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              int32_t height)
{
    uint32_t loop_cnt;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;


    for (loop_cnt = (height >> 1); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + 16, 0, src + 32, 0, src + 48, 0,
                      src0, src1, src2, src3);
        src += src_stride;
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + 16, 0, src + 32, 0, src + 48, 0,
                      src4, src5, src6, src7);
        src += src_stride;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        __lsx_vst(in2_r, dst + 32, 0);
        __lsx_vst(in2_l, dst + 40, 0);
        __lsx_vst(in3_r, dst + 48, 0);
        __lsx_vst(in3_l, dst + 56, 0);
        dst += dst_stride;

        LSX_DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_l, in1_l, in2_l, in3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        LSX_DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst + 8, 0);
        __lsx_vst(in1_r, dst + 16, 0);
        __lsx_vst(in1_l, dst + 24, 0);
        __lsx_vst(in2_r, dst + 32, 0);
        __lsx_vst(in2_l, dst + 40, 0);
        __lsx_vst(in3_r, dst + 48, 0);
        __lsx_vst(in3_l, dst + 56, 0);
        dst += dst_stride;
    }
}

#define MC_COPY(WIDTH)                                                    \
void ff_hevc_put_hevc_pel_pixels##WIDTH##_8_lsx(int16_t *dst,             \
                                                uint8_t *src,             \
                                                ptrdiff_t src_stride,     \
                                                int height,               \
                                                intptr_t mx,              \
                                                intptr_t my,              \
                                                int width)                \
{                                                                         \
    hevc_copy_##WIDTH##w_lsx(src, src_stride, dst, MAX_PB_SIZE, height);  \
}

MC_COPY(4);
MC_COPY(6);
MC_COPY(8);
MC_COPY(12);
MC_COPY(16);
MC_COPY(24);
MC_COPY(32);
MC_COPY(48);
MC_COPY(64);

#undef MC_COPY
