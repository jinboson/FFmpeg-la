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

#include "libavutil/loongarch/loongson_intrinsics.h"
#include "hevcdsp_lsx.h"
#include "hevc_macros_lsx.h"

static const uint8_t ff_hevc_mask_arr[16 * 2] __attribute__((aligned(0x40))) = {
    /* 8 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
    /* 4 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 16, 17, 17, 18, 18, 19, 19, 20
};

static void hevc_copy_4w_lsx(uint8_t *src, int32_t src_stride,
                             int16_t *dst, int32_t dst_stride,
                             int32_t height)
{
    __m128i zero = {0};
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    if (2 == height) {
        __m128i src0, src1;
        __m128i in0;

        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
        src0 = __lsx_vilvl_w(src1, src0);
        in0 = __lsx_vilvl_b(zero, src0);
        in0 = __lsx_vslli_h(in0, 6);
        __lsx_vstelm_d(in0, dst, 0, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 0, 1);
    } else if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0, in1;

        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        DUP2_ARG2(__lsx_vilvl_w, src1, src0, src3, src2, src0, src1);
        DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vstelm_d(in0, dst, 0, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(in1, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(in1, dst + dst_stride_3x, 0, 1);
    } else if (0 == (height & 0x07)) {
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i in0, in1, in2, in3;
        uint32_t loop_cnt;
        for (loop_cnt = (height >> 3); loop_cnt--;) {
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
            src += src_stride_4x;
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,src + src_stride_2x,
                      0, src + src_stride_3x, 0, src4, src5, src6, src7);
            src += src_stride_4x;

            DUP4_ARG2(__lsx_vilvl_w, src1, src0, src3, src2, src5, src4, src7, src6,
                      src0, src1, src2, src3);
            DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0, in1, in2, in3);
            DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);

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
    int32_t src_stride_3x = src_stride_2x + src_stride;

    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0, in1, in2, in3);
        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in4, in5, in6, in7);
        DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        DUP4_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in6, 6, in7, 6, in4, in5, in6, in7);

        DUP4_ARG2(__lsx_vpickve2gr_du, in0, 0, in1, 0, in2, 0, in3, 0, out0_m,
                  out1_m, out2_m, out3_m);
        DUP4_ARG2(__lsx_vpickve2gr_du, in4, 0, in5, 0, in6, 0, in7, 0, out4_m,
                  out5_m, out6_m, out7_m);

        DUP4_ARG2(__lsx_vpickve2gr_wu, in0, 2, in1, 2, in2, 2, in3, 2, out8_m,
                  out9_m, out10_m, out11_m);
        DUP4_ARG2(__lsx_vpickve2gr_wu, in4, 2, in5, 2, in6, 2, in7, 2, out12_m,
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
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    if (2 == height) {
        __m128i src0, src1;
        __m128i in0, in1;

        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
        DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0, dst, 0);
        __lsx_vst(in1, dst + dst_stride, 0);
    } else if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0, in1, in2, in3;

        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0, in1, in2, in3);
        DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        __lsx_vst(in0, dst, 0);
        __lsx_vst(in1, dst + dst_stride, 0);
        __lsx_vst(in2, dst + dst_stride_2x, 0);
        __lsx_vst(in3, dst + dst_stride_3x, 0);
    } else if (6 == height) {
        __m128i src0, src1, src2, src3, src4, src5;
        __m128i in0, in1, in2, in3, in4, in5;

        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3, in0,
                  in1, in2, in3);
        DUP2_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, in4, in5);
        DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
        DUP2_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in4, in5);
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
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src0, src1, src2, src3);
            src += src_stride_4x;
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                      0, src + src_stride_3x, 0, src4, src5, src6, src7);
            src += src_stride_4x;

            DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0, in1, in2, in3);
            DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in4, in5, in6, in7);
            DUP4_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in2, 6, in3, 6, in0, in1, in2, in3);
            DUP4_ARG2(__lsx_vslli_h, in4, 6, in5, 6, in6, 6, in7, 6, in4, in5, in6, in7);
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
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0, in1, in0_r, in1_r, in2_r, in3_r;

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP2_ARG2(__lsx_vilvh_w, src1, src0, src3, src2, src0, src1);
        DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(in0, dst, 16, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 16, 1);
        __lsx_vstelm_d(in1, dst + dst_stride_2x, 16, 0);
        __lsx_vstelm_d(in1, dst + dst_stride_3x, 16, 1);
        dst += dst_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP2_ARG2(__lsx_vilvh_w, src5, src4, src7, src6, src0, src1);
        DUP2_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, in0, in1);
        DUP2_ARG2(__lsx_vslli_h, in0, 6, in1, 6, in0, in1);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(in0, dst, 16, 0);
        __lsx_vstelm_d(in0, dst + dst_stride, 16, 1);
        __lsx_vstelm_d(in1, dst + dst_stride_2x, 16, 0);
        __lsx_vstelm_d(in1, dst + dst_stride_3x, 16, 1);
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
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    if (4 == height) {
        __m128i src0, src1, src2, src3;
        __m128i in0_r, in1_r, in2_r, in3_r;
        __m128i in0_l, in1_l, in2_l, in3_l;

        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_l, dst + dst_stride, 16);
        __lsx_vst(in2_l, dst + dst_stride_2x, 16);
        __lsx_vst(in3_l, dst + dst_stride_3x, 16);
   } else if (12 == height) {
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i src8, src9, src10, src11;
        __m128i in0_r, in1_r, in2_r, in3_r;
        __m128i in0_l, in1_l, in2_l, in3_l;

        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src8, src9, src10, src11);

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_l, dst + dst_stride, 16);
        __lsx_vst(in2_l, dst + dst_stride_2x, 16);
        __lsx_vst(in3_l, dst + dst_stride_3x, 16);
        dst += dst_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_l, dst + dst_stride, 16);
        __lsx_vst(in2_l, dst + dst_stride_2x, 16);
        __lsx_vst(in3_l, dst + dst_stride_3x, 16);
        dst += dst_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src8, zero, src9, zero, src10, zero, src11,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src8, zero, src9, zero, src10, zero, src11,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_l, dst + dst_stride, 16);
        __lsx_vst(in2_l, dst + dst_stride_2x, 16);
        __lsx_vst(in3_l, dst + dst_stride_3x, 16);
    } else if (0 == (height & 0x07)) {
        uint32_t loop_cnt;
        __m128i src0, src1, src2, src3, src4, src5, src6, src7;
        __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

        for (loop_cnt = (height >> 3); loop_cnt--;) {
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                      src + src_stride_3x, 0, src0, src1, src2, src3);
            src += src_stride_4x;
            DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                      src + src_stride_3x, 0, src4, src5, src6, src7);
            src += src_stride_4x;
            DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
            DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
            DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
            DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
            __lsx_vst(in0_r, dst, 0);
            __lsx_vst(in1_r, dst + dst_stride, 0);
            __lsx_vst(in2_r, dst + dst_stride_2x, 0);
            __lsx_vst(in3_r, dst + dst_stride_3x, 0);
            __lsx_vst(in0_l, dst, 16);
            __lsx_vst(in1_l, dst + dst_stride, 16);
            __lsx_vst(in2_l, dst + dst_stride_2x, 16);
            __lsx_vst(in3_l, dst + dst_stride_3x, 16);
            dst += dst_stride_4x;

            DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_r, in1_r, in2_r, in3_r);
            DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                      in0_l, in1_l, in2_l, in3_l);
            DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
            DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
            __lsx_vst(in0_r, dst, 0);
            __lsx_vst(in1_r, dst + dst_stride, 0);
            __lsx_vst(in2_r, dst + dst_stride_2x, 0);
            __lsx_vst(in3_r, dst + dst_stride_3x, 0);
            __lsx_vst(in0_l, dst, 16);
            __lsx_vst(in1_l, dst + dst_stride, 16);
            __lsx_vst(in2_l, dst + dst_stride_2x, 16);
            __lsx_vst(in3_l, dst + dst_stride_3x, 16);
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
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        DUP4_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src + src_stride_2x,
                  16, src + src_stride_3x, 16, src4, src5, src6, src7);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in1_r, dst + dst_stride, 0);
        __lsx_vst(in2_r, dst + dst_stride_2x, 0);
        __lsx_vst(in3_r, dst + dst_stride_3x, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_l, dst + dst_stride, 16);
        __lsx_vst(in2_l, dst + dst_stride_2x, 16);
        __lsx_vst(in3_l, dst + dst_stride_3x, 16);
        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        __lsx_vst(in0_r, dst, 32);
        __lsx_vst(in1_r, dst + dst_stride, 32);
        __lsx_vst(in2_r, dst + dst_stride_2x, 32);
        __lsx_vst(in3_r, dst + dst_stride_3x, 32);
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
    int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i zero = {0};
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0_r, in1_r, in2_r, in3_r, in0_l, in1_l, in2_l, in3_l;

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src2, src4, src6);
        DUP4_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src + src_stride_2x,
                  16, src + src_stride_3x, 16, src1, src3, src5, src7);
        src += src_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        dst += dst_stride;
        __lsx_vst(in2_r, dst, 0);
        __lsx_vst(in2_l, dst, 16);
        __lsx_vst(in3_r, dst, 32);
        __lsx_vst(in3_l, dst, 48);
        dst += dst_stride;

        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        dst += dst_stride;
        __lsx_vst(in2_r, dst, 0);
        __lsx_vst(in2_l, dst, 16);
        __lsx_vst(in3_r, dst, 32);
        __lsx_vst(in3_l, dst, 48);
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
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
        src2 = __lsx_vld(src, 32);
        src += src_stride;
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src3, src4);
        src5 = __lsx_vld(src, 32);
        src += src_stride;
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src6, src7);
        src8 = __lsx_vld(src, 32);
        src += src_stride;
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src9, src10);
        src11 = __lsx_vld(src, 32);
        src += src_stride;

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                  in0_l, in1_l, in2_l, in3_l);
        DUP2_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, in4_r, in5_r);
        DUP2_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, in4_l, in5_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in4_r, 6, in5_r, 6, in4_l, 6, in5_l, 6, in4_r,
                  in5_r, in4_l, in5_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        __lsx_vst(in2_r, dst, 64);
        __lsx_vst(in2_l, dst, 80);
        dst += dst_stride;
        __lsx_vst(in3_r, dst, 0);
        __lsx_vst(in3_l, dst, 16);
        __lsx_vst(in4_r, dst, 32);
        __lsx_vst(in4_l, dst, 48);
        __lsx_vst(in5_r, dst, 64);
        __lsx_vst(in5_l, dst, 80);
        dst += dst_stride;

        DUP4_ARG2(__lsx_vilvl_b, zero, src6, zero, src7, zero, src8, zero, src9,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src6, zero, src7, zero, src8, zero, src9,
                  in0_l, in1_l, in2_l, in3_l);
        DUP2_ARG2(__lsx_vilvl_b, zero, src10, zero, src11, in4_r, in5_r);
        DUP2_ARG2(__lsx_vilvh_b, zero, src10, zero, src11, in4_l, in5_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in4_r, 6, in5_r, 6, in4_l, 6, in5_l, 6, in4_r,
                  in5_r, in4_l, in5_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        __lsx_vst(in2_r, dst, 64);
        __lsx_vst(in2_l, dst, 80);
        dst += dst_stride;
        __lsx_vst(in3_r, dst, 0);
        __lsx_vst(in3_l, dst, 16);
        __lsx_vst(in4_r, dst, 32);
        __lsx_vst(in4_l, dst, 48);
        __lsx_vst(in5_r, dst, 64);
        __lsx_vst(in5_l, dst, 80);
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
        DUP4_ARG2(__lsx_vld, src, 0, src, 16, src, 32, src, 48, src0, src1, src2, src3);
        src += src_stride;
        DUP4_ARG2(__lsx_vld, src, 0, src, 16, src, 32, src, 48, src4, src5, src6, src7);
        src += src_stride;

        DUP4_ARG2(__lsx_vilvl_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                      in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                      in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        __lsx_vst(in2_r, dst, 64);
        __lsx_vst(in2_l, dst, 80);
        __lsx_vst(in3_r, dst, 96);
        __lsx_vst(in3_l, dst, 112);
        dst += dst_stride;

        DUP4_ARG2(__lsx_vilvl_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_r, in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, zero, src6, zero, src7,
                  in0_l, in1_l, in2_l, in3_l);
        DUP4_ARG2(__lsx_vslli_h, in0_r, 6, in1_r, 6, in2_r, 6, in3_r, 6, in0_r,
                  in1_r, in2_r, in3_r);
        DUP4_ARG2(__lsx_vslli_h, in0_l, 6, in1_l, 6, in2_l, 6, in3_l, 6, in0_l,
                  in1_l, in2_l, in3_l);
        __lsx_vst(in0_r, dst, 0);
        __lsx_vst(in0_l, dst, 16);
        __lsx_vst(in1_r, dst, 32);
        __lsx_vst(in1_l, dst, 48);
        __lsx_vst(in2_r, dst, 64);
        __lsx_vst(in2_l, dst, 80);
        __lsx_vst(in3_r, dst, 96);
        __lsx_vst(in3_l, dst, 112);
        dst += dst_stride;
    }
}

static void hevc_hz_8t_4w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3;
    __m128i vec0, vec1, vec2, vec3;
    __m128i dst0, dst1, dst2, dst3;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 16);

    src -= 3;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src4, src5, src6, src7);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128,
                  src0, src1, src2, src3);
        DUP4_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src6, 128, src7, 128,
                  src4, src5, src6, src7);
        DUP4_ARG3(__lsx_vshuf_b, src1, src0, mask0, src1, src0, mask1, src1, src0,
                  mask2, src1, src0, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
                  vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);
        DUP4_ARG3(__lsx_vshuf_b, src3, src2, mask0, src3, src2, mask1, src3, src2,
                  mask2, src3, src2, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst1, vec1, filt1, dst1,
                  vec2, filt2, dst1, vec3, filt3, dst1, dst1, dst1, dst1);
        DUP4_ARG3(__lsx_vshuf_b, src5, src4, mask0, src5, src4, mask1, src5, src4,
                  mask2, src5, src4, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst2, vec1, filt1, dst2,
                  vec2, filt2, dst2, vec3, filt3, dst2, dst2, dst2, dst2);
        DUP4_ARG3(__lsx_vshuf_b, src7, src6, mask0, src7, src6, mask1, src7, src6,
                  mask2, src7, src6, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3,
                  vec2, filt2, dst3, vec3, filt3, dst3, dst3, dst3, dst3);

        __lsx_vstelm_d(dst0, dst, 0, 0);
        __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst1, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst1, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;
        __lsx_vstelm_d(dst2, dst, 0, 0);
        __lsx_vstelm_d(dst2, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst3, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst3, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;
    }
}

static void hevc_hz_8t_8w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i src0, src1, src2, src3;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3;
    __m128i vec0, vec1, vec2, vec3;
    __m128i dst0, dst1, dst2, dst3;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= 3;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);

        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
                  mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
                  vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);
        DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
                  mask2, src1, src1, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst1, vec1, filt1, dst1,
                  vec2, filt2, dst1, vec3, filt3, dst1, dst1, dst1, dst1);
        DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
                  mask2, src2, src2, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst2, vec1, filt1, dst2,
                  vec2, filt2, dst2, vec3, filt3, dst2, dst2, dst2, dst2);
        DUP4_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, src3, src3,
                  mask2, src3, src3, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3,
                  vec2, filt2, dst3, vec3, filt3, dst3, dst3, dst3, dst3);

        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst + dst_stride, 0);
        __lsx_vst(dst2, dst + dst_stride_2x, 0);
        __lsx_vst(dst3, dst + dst_stride_3x, 0);
        dst += dst_stride_4x;
    }
}

static void hevc_hz_8t_12w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i mask0, mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5;
    __m128i filt0, filt1, filt2, filt3, dst0, dst1, dst2, dst3, dst4, dst5;
    __m128i const_vec;

    src -= 3;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);
    mask4 = __lsx_vld(ff_hevc_mask_arr, 16);
    DUP2_ARG2(__lsx_vaddi_bu, mask4, 2, mask4, 4, mask5, mask6);
    mask7 = __lsx_vaddi_bu(mask4, 6);

    for (loop_cnt = 4; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src0, src1, src2, src3);
        DUP4_ARG2(__lsx_vld, src, 8, src + src_stride, 8, src + src_stride_2x, 8,
                  src + src_stride_3x, 8, src4, src5, src6, src7);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);
        DUP4_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src6, 128, src7, 128, src4,
                  src5, src6, src7);

        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src1, src1, mask0, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src3, src3, mask0, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src4, mask4, src7, src6, mask4, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, const_vec, vec1, filt0,
                  const_vec, vec2, filt0, const_vec, vec3, filt0, dst0, dst1, dst2,
                  dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, const_vec, vec5, filt0,
                  dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask1, src1, src1, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask1, src3, src3, mask1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src4, mask5, src7, src6, mask5, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt1, dst1, vec1, filt1, dst2,
                  vec2, filt1, dst3, vec3, filt1, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt1, dst5, vec5, filt1, dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask2, src1, src1, mask2, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask2, src3, src3, mask2, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src4, mask6, src7, src6, mask6, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt2, dst1, vec1, filt2, dst2,
                  vec2, filt2, dst3, vec3, filt2, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt2, dst5, vec5, filt2, dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask3, src1, src1, mask3, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask3, src3, src3, mask3, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src4, mask7, src7, src6, mask7, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt3, dst1, vec1, filt3, dst2,
                  vec2, filt3, dst3, vec3, filt3, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt3, dst5, vec5, filt3, dst4, dst5);

        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst + dst_stride, 0);
        __lsx_vst(dst2, dst + dst_stride_2x, 0);
        __lsx_vst(dst3, dst + dst_stride_3x, 0);

        __lsx_vstelm_d(dst4, dst, 16, 0);
        __lsx_vstelm_d(dst4, dst + dst_stride, 16, 1);
        __lsx_vstelm_d(dst5, dst + dst_stride_2x, 16, 0);
        __lsx_vstelm_d(dst5, dst + dst_stride_3x, 16, 1);
        dst += dst_stride_4x;
    }

}

static void hevc_hz_8t_16w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    __m128i src0, src1, src2, src3;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3;
    __m128i vec0, vec1, vec2, vec3;
    __m128i dst0, dst1, dst2, dst3;
    __m128i const_vec;
    __m128i mask0;

    src -= 3;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6,
              filt0, filt1, filt2, filt3);

    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);

    for (loop_cnt = (height >> 1); loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src2);
        DUP2_ARG2(__lsx_vld, src, 8, src + src_stride, 8, src1, src3);
        src += src_stride_2x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);

        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src1, src1, mask0, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src3, src3, mask0, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, const_vec, vec1, filt0,
                  const_vec, vec2, filt0, const_vec, vec3, filt0, dst0, dst1, dst2,
                  dst3);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask1, src1, src1, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask1, src3, src3, mask1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt1, dst1, vec1, filt1, dst2,
                  vec2, filt1, dst3, vec3, filt1, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask2, src1, src1, mask2, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask2, src3, src3, mask2, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt2, dst1, vec1, filt2, dst2,
                  vec2, filt2, dst3, vec3, filt2, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask3, src1, src1, mask3, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask3, src3, src3, mask3, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt3, dst1, vec1, filt3, dst2,
                  vec2, filt3, dst3, vec3, filt3, dst0, dst1, dst2, dst3);

        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst2, dst + dst_stride, 0);
        __lsx_vst(dst1, dst, 16);
        __lsx_vst(dst3, dst + dst_stride, 16);
        dst += dst_stride_2x;
    }
}

static void hevc_hz_8t_24w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    __m128i src0, src1, src2, src3;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask0, 6, mask0, 8, mask1, mask2,
              mask3, mask4);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 10, mask0, 12, mask5, mask6);
    mask7 = __lsx_vaddi_bu(mask0, 14);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (loop_cnt = (height >> 1); loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
        src += src_stride;
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src2, src3);
        src += src_stride;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);

        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src1, src0, mask4, src1, src1,
                  mask0, src2, src2, mask0, vec0, vec1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src3, src2, mask4, src3, src3, mask0, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, const_vec, vec1, filt0,
                  const_vec, vec2, filt0, const_vec, vec3, filt0, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, const_vec, vec5, filt0,
                  dst4, dst5);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask1, src1, src0, mask5, src1, src1,
                  mask1, src2, src2, mask1, vec0, vec1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src3, src2, mask5, src3, src3, mask1, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt1, dst1, vec1, filt1, dst2,
                  vec2, filt1, dst3, vec3, filt1, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt1, dst5, vec5, filt1, dst4, dst5);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask2, src1, src0, mask6, src1, src1,
                  mask2, src2, src2, mask2, vec0, vec1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src3, src2, mask6, src3, src3, mask2, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt2, dst1, vec1, filt2, dst2,
                  vec2, filt2, dst3, vec3, filt2, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt2, dst5, vec5, filt2, dst4, dst5);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask3, src1, src0, mask7, src1, src1,
                  mask3, src2, src2, mask3, vec0, vec1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src3, src2, mask7, src3, src3, mask3, vec4, vec5);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt3, dst1, vec1, filt3, dst2,
                  vec2, filt3, dst3, vec3, filt3, dst0, dst1, dst2, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt3, dst5, vec5, filt3, dst4, dst5);

        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst, 16);
        __lsx_vst(dst2, dst, 32);
        dst += dst_stride;
        __lsx_vst(dst3, dst, 0);
        __lsx_vst(dst4, dst, 16);
        __lsx_vst(dst5, dst, 32);
        dst += dst_stride;
    }
}

static void hevc_hz_8t_32w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    __m128i src0, src1, src2;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3;
    __m128i dst0, dst1, dst2, dst3;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2,  filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask0, 6, mask0, 8, mask1, mask2,
              mask3, mask4);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 10, mask0, 12, mask5, mask6);
    mask7 = __lsx_vaddi_bu(mask0, 14);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (loop_cnt = height; loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
        src2 = __lsx_vld(src, 24);
        src += src_stride;
        DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
        src2 = __lsx_vxori_b(src2, 128);

        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
                  mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
                  vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);

        DUP4_ARG3(__lsx_vshuf_b, src1, src0, mask4, src1, src0, mask5, src1, src0,
                  mask6, src1, src0, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst1, vec1, filt1, dst1,
                  vec2, filt2, dst1, vec3, filt3, dst1, dst1, dst1, dst1);
        DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
                  mask2, src1, src1, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst2, vec1, filt1, dst2,
                  vec2, filt2, dst2, vec3, filt3, dst2, dst2, dst2, dst2);
        DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
                  mask2, src2, src2, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3,
                  vec2, filt2, dst3, vec3, filt3, dst3, dst3, dst3, dst3);

        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst, 16);
        __lsx_vst(dst2, dst, 32);
        __lsx_vst(dst3, dst, 48);
        dst += dst_stride;
    }
}

static void hevc_hz_8t_48w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    __m128i src0, src1, src2, src3;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask0, 6, mask0, 8, mask1, mask2,
              mask3, mask4);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 10, mask0, 12, mask5, mask6);
    mask7 = __lsx_vaddi_bu(mask0, 14);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (loop_cnt = height; loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
        src2 = __lsx_vld(src, 32);
        src3 = __lsx_vld(src, 40);
        src += src_stride;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);

        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src1, src0, mask4, src1, src1,
                  mask0, src2, src1, mask4, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, const_vec, vec1, filt0,
                  const_vec, vec2, filt0, const_vec, vec3, filt0, dst0, dst1, dst2,
                  dst3);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask1, src1, src0, mask5, src1, src1,
                  mask1, src2, src1, mask5, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt1, dst1, vec1, filt1, dst2,
                  vec2, filt1, dst3, vec3, filt1, dst0, dst1, dst2, dst3);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask2, src1, src0, mask6, src1, src1,
                  mask2, src2, src1, mask6, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt2, dst1, vec1, filt2, dst2,
                  vec2, filt2, dst3, vec3, filt2, dst0, dst1, dst2, dst3);
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask3, src1, src0, mask7, src1, src1,
                  mask3, src2, src1, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt3, dst1, vec1, filt3, dst2,
                  vec2, filt3, dst3, vec3, filt3, dst0, dst1, dst2, dst3);
        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst, 16);
        __lsx_vst(dst2, dst, 32);
        __lsx_vst(dst3, dst, 48);

        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src3, src3, mask0, vec4, vec5);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, const_vec, vec5, filt0,
                  dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask1, src3, src3, mask1, vec4, vec5);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt1, dst5, vec5, filt1, dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask2, src3, src3, mask2, vec4, vec5);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt2, dst5, vec5, filt2, dst4, dst5);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask3, src3, src3, mask3, vec4, vec5);
        DUP2_ARG3(__lsx_vdp2add_h_b, dst4, vec4, filt3, dst5, vec5, filt3, dst4, dst5);
        __lsx_vst(dst4, dst, 64);
        __lsx_vst(dst5, dst, 80);
        dst += dst_stride;
    }
}

static void hevc_hz_8t_64w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    uint32_t loop_cnt;
    __m128i src0, src1, src2, src3, src4;
    __m128i filt0, filt1, filt2, filt3;
    __m128i mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i const_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask0, 6, mask0, 8, mask1, mask2,
              mask3, mask4);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 10, mask0, 12, mask5, mask6)
    mask7 = __lsx_vaddi_bu(mask0, 14);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (loop_cnt = height; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src, 16,  src, 32, src, 48, src0, src1, src2, src3);
        src4 = __lsx_vld(src, 56);
        src += src_stride;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);
        src4 = __lsx_vxori_b(src4, 128);

        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
                  mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
                  vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);
        __lsx_vst(dst0, dst, 0);

        DUP4_ARG3(__lsx_vshuf_b, src1, src0, mask4, src1, src0, mask5, src1, src0,
                  mask6, src1, src0, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst1, vec1, filt1, dst1,
                  vec2, filt2, dst1, vec3, filt3, dst1, dst1, dst1, dst1);
        __lsx_vst(dst1, dst, 16);

        DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
                  mask2, src1, src1, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst2, vec1, filt1, dst2,
                  vec2, filt2, dst2, vec3, filt3, dst2, dst2, dst2, dst2);
        __lsx_vst(dst2, dst, 32);

        DUP4_ARG3(__lsx_vshuf_b, src2, src1, mask4, src2, src1, mask5, src2, src1,
                  mask6, src2, src1, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3,
                  vec2, filt2, dst3, vec3, filt3, dst3, dst3, dst3, dst3);
        __lsx_vst(dst3, dst, 48);

        DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
                  mask2, src2, src2, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst4, vec1, filt1, dst4,
                  vec2, filt2, dst4, vec3, filt3, dst4, dst4, dst4, dst4);
        __lsx_vst(dst4, dst, 64);

        DUP4_ARG3(__lsx_vshuf_b, src3, src2, mask4, src3, src2, mask5, src3, src2,
                  mask6, src3, src2, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst5, vec1, filt1, dst5,
                  vec2, filt2, dst5, vec3, filt3, dst5, dst5, dst5, dst5);
        __lsx_vst(dst5, dst, 80);

        DUP4_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, src3, src3,
                  mask2, src3, src3, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst6, vec1, filt1, dst6,
                  vec2, filt2, dst6, vec3, filt3, dst6, dst6, dst6, dst6);
        __lsx_vst(dst6, dst, 96);

        DUP4_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, src4, src4,
                  mask2, src4, src4, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst7, vec1, filt1, dst7,
                  vec2, filt2, dst7, vec3, filt3, dst7, dst7, dst7, dst7);
        __lsx_vst(dst7, dst, 112);
        dst += dst_stride;
    }
}

static void hevc_vt_8t_4w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter, int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i src9, src10, src11, src12, src13, src14;
    __m128i src10_r, src32_r, src54_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src65_r, src87_r, src109_r;
    __m128i src1110_r, src1211_r, src1312_r, src1413_r;
    __m128i src2110, src4332, src6554, src8776, src10998;
    __m128i src12111110, src14131312;
    __m128i dst10, dst32, dst54, dst76;
    __m128i filt0, filt1, filt2, filt3;
    __m128i const_vec;

    src -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
              src10_r, src32_r, src54_r, src21_r);
    DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);
    DUP2_ARG2(__lsx_vilvl_d, src21_r, src10_r, src43_r, src32_r, src2110, src4332);
    src6554 = __lsx_vilvl_d(src65_r, src54_r);
    DUP2_ARG2(__lsx_vxori_b, src2110, 128, src4332, 128, src2110, src4332);
    src6554 = __lsx_vxori_b(src6554, 128);

    for (loop_cnt = (height >> 3); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x,
                  0, src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0,  src + src_stride_2x,
                  0, src + src_stride_3x, 0, src11, src12, src13, src14);
        src += src_stride_4x;

        DUP4_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src9, src8, src10, src9,
                  src76_r, src87_r, src98_r, src109_r);
        DUP4_ARG2(__lsx_vilvl_b, src11, src10, src12, src11, src13, src12, src14,
                  src13, src1110_r, src1211_r, src1312_r, src1413_r);
        DUP4_ARG2(__lsx_vilvl_d, src87_r, src76_r, src109_r, src98_r, src1211_r,
                  src1110_r, src1413_r, src1312_r, src8776, src10998, src12111110,
                  src14131312);
        DUP4_ARG2(__lsx_vxori_b, src8776, 128, src10998, 128, src12111110, 128,
                  src14131312, 128, src8776, src10998, src12111110, src14131312);

        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src2110, filt0, dst10, src4332, filt1,
                  dst10, src6554, filt2, dst10, src8776, filt3, dst10, dst10, dst10,
                  dst10);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src4332, filt0, dst32, src6554, filt1,
                  dst32, src8776, filt2, dst32, src10998, filt3, dst32, dst32,
                  dst32, dst32);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src6554, filt0, dst54, src8776, filt1,
                  dst54, src10998, filt2, dst54, src12111110, filt3, dst54, dst54,
                  dst54, dst54);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src8776, filt0, dst76, src10998,
                  filt1, dst76, src12111110, filt2, dst76, src14131312, filt3, dst76,
                  dst76, dst76, dst76);

        __lsx_vstelm_d(dst10, dst, 0, 0);
        __lsx_vstelm_d(dst10, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst32, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst32, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;
        __lsx_vstelm_d(dst54, dst, 0, 0);
        __lsx_vstelm_d(dst54, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst76, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst76, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;

        src2110 = src10998;
        src4332 = src12111110;
        src6554 = src14131312;
        src6 = src14;
    }
}

static void hevc_vt_8t_8w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter, int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i src10_r, src32_r, src54_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src65_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i const_vec;
    __m128i filt0, filt1, filt2, filt3;

    src -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6,
              filt0, filt1, filt2, filt3);

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
              src1, src2, src3);
    DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 = __lsx_vxori_b(src6, 128);
    DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
              src10_r, src32_r, src54_r, src21_r);
    DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128,
                  src7, src8, src9, src10);
        DUP4_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src9, src8, src10, src9,
                  src76_r, src87_r, src98_r, src109_r);

        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                  filt1, dst0_r, src54_r, filt2, dst0_r, src76_r, filt3, dst0_r,
                  dst0_r, dst0_r, dst0_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                  filt1, dst1_r, src65_r, filt2, dst1_r, src87_r, filt3, dst1_r,
                  dst1_r, dst1_r, dst1_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst2_r, src54_r,
                  filt1, dst2_r, src76_r, filt2, dst2_r, src98_r, filt3, dst2_r,
                  dst2_r, dst2_r, dst2_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst3_r, src65_r,
                  filt1, dst3_r, src87_r, filt2, dst3_r, src109_r, filt3, dst3_r,
                  dst3_r, dst3_r, dst3_r);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst1_r, dst + dst_stride, 0);
        __lsx_vst(dst2_r, dst + dst_stride_2x, 0);
        __lsx_vst(dst3_r, dst + dst_stride_3x, 0);
        dst += dst_stride_4x;

        src10_r = src54_r;
        src32_r = src76_r;
        src54_r = src98_r;
        src21_r = src65_r;
        src43_r = src87_r;
        src65_r = src109_r;
        src6 = src10;
    }
}

static void hevc_vt_8t_12w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i src10_r, src32_r, src54_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src65_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i src10_l, src32_l, src54_l, src76_l, src98_l;
    __m128i src21_l, src43_l, src65_l, src87_l, src109_l;
    __m128i src2110, src4332, src6554, src8776, src10998;
    __m128i dst0_l, dst1_l;
    __m128i const_vec;
    __m128i filt0, filt1, filt2, filt3;

    src -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);
    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
              src1, src2, src3);
    DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 = __lsx_vxori_b(src6, 128);
    DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
              src10_r, src32_r, src54_r, src21_r);
    DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);
    DUP4_ARG2(__lsx_vilvh_b, src1, src0, src3, src2, src5, src4, src2, src1,
              src10_l, src32_l, src54_l, src21_l);
    DUP2_ARG2(__lsx_vilvh_b, src4, src3, src6, src5, src43_l, src65_l);
    DUP2_ARG2(__lsx_vilvl_d, src21_l, src10_l, src43_l, src32_l, src2110, src4332);
    src6554 = __lsx_vilvl_d(src65_l, src54_l);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src+ src_stride_2x, 0,
                  src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128, src7,
                  src8, src9, src10);
        DUP4_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src9, src8, src10, src9,
                  src76_r, src87_r, src98_r, src109_r);
        DUP4_ARG2(__lsx_vilvh_b, src7, src6, src8, src7, src9, src8, src10, src9,
                  src76_l, src87_l, src98_l, src109_l);
        DUP2_ARG2(__lsx_vilvl_d, src87_l, src76_l, src109_l, src98_l, src8776, src10998);

        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                  filt1, dst0_r, src54_r, filt2, dst0_r, src76_r, filt3, dst0_r,
                  dst0_r, dst0_r, dst0_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                  filt1, dst1_r, src65_r, filt2, dst1_r, src87_r, filt3, dst1_r,
                  dst1_r, dst1_r, dst1_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst2_r, src54_r,
                  filt1, dst2_r, src76_r, filt2, dst2_r, src98_r, filt3, dst2_r,
                  dst2_r, dst2_r, dst2_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst3_r, src65_r,
                  filt1, dst3_r, src87_r, filt2, dst3_r, src109_r, filt3, dst3_r,
                  dst3_r, dst3_r, dst3_r);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src2110, filt0, dst0_l, src4332,
                  filt1, dst0_l, src6554, filt2, dst0_l, src8776, filt3, dst0_l,
                  dst0_l, dst0_l, dst0_l);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src4332, filt0, dst1_l, src6554,
                  filt1, dst1_l, src8776, filt2, dst1_l, src10998, filt3, dst1_l,
                  dst1_l, dst1_l, dst1_l);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst1_r, dst + dst_stride, 0);
        __lsx_vst(dst2_r, dst + dst_stride_2x, 0);
        __lsx_vst(dst3_r, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(dst0_l, dst, 16, 0);
        __lsx_vstelm_d(dst0_l, dst + dst_stride, 16, 1);
        __lsx_vstelm_d(dst1_l, dst + dst_stride_2x, 16, 0);
        __lsx_vstelm_d(dst1_l, dst + dst_stride_3x, 16, 1);
        dst += dst_stride_4x;

        src10_r = src54_r;
        src32_r = src76_r;
        src54_r = src98_r;
        src21_r = src65_r;
        src43_r = src87_r;
        src65_r = src109_r;
        src2110 = src6554;
        src4332 = src8776;
        src6554 = src10998;
        src6 = src10;
    }
}

static void hevc_vt_8t_16multx4mult_lsx(uint8_t *src,
                                        int32_t src_stride,
                                        int16_t *dst,
                                        int32_t dst_stride,
                                        const int8_t *filter,
                                        int32_t height,
                                        int32_t width)
{
    uint8_t *src_tmp;
    int16_t *dst_tmp;
    int32_t loop_cnt, cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = (src_stride << 1) + src_stride;
    int32_t dst_stride_3x = (dst_stride << 1) + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i src10_r, src32_r, src54_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src65_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i src10_l, src32_l, src54_l, src76_l, src98_l;
    __m128i src21_l, src43_l, src65_l, src87_l, src109_l;
    __m128i dst0_l, dst1_l, dst2_l, dst3_l;
    __m128i const_vec;
    __m128i filt0, filt1, filt2, filt3;

    src -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6, filt0,
              filt1, filt2, filt3);

    for (cnt = width >> 4; cnt--;) {
        src_tmp = src;
        dst_tmp = dst;

        DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src_tmp + src_stride_2x,
                  0, src_tmp + src_stride_3x, 0, src0, src1, src2, src3);
        src_tmp += src_stride_4x;
        DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src_tmp + src_stride_2x, 0);
        src_tmp += src_stride_3x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);
        DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);
        DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
                  src10_r, src32_r, src54_r, src21_r);
        DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);
        DUP4_ARG2(__lsx_vilvh_b, src1, src0, src3, src2, src5, src4, src2, src1,
                  src10_l, src32_l, src54_l, src21_l);
        DUP2_ARG2(__lsx_vilvh_b, src4, src3, src6, src5, src43_l, src65_l);

        for (loop_cnt = (height >> 2); loop_cnt--;) {
            DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                      src_tmp + src_stride_2x, 0, src_tmp + src_stride_3x, 0,
                      src7, src8, src9, src10);
            src_tmp += src_stride_4x;
            DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128,
                      src7, src8, src9, src10);
            DUP4_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src9, src8, src10, src9,
                      src76_r, src87_r, src98_r, src109_r);
            DUP4_ARG2(__lsx_vilvh_b, src7, src6, src8, src7, src9, src8, src10, src9,
                      src76_l, src87_l, src98_l, src109_l);

            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                      filt1, dst0_r, src54_r, filt2, dst0_r, src76_r, filt3,
                      dst0_r, dst0_r, dst0_r, dst0_r);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                      filt1, dst1_r, src65_r, filt2, dst1_r, src87_r, filt3,
                      dst1_r, dst1_r, dst1_r, dst1_r);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst2_r, src54_r,
                      filt1, dst2_r, src76_r, filt2, dst2_r, src98_r, filt3,
                      dst2_r, dst2_r, dst2_r, dst2_r);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst3_r, src65_r,
                      filt1, dst3_r, src87_r, filt2, dst3_r, src109_r, filt3,
                      dst3_r, dst3_r, dst3_r, dst3_r);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src10_l, filt0, dst0_l, src32_l,
                      filt1, dst0_l, src54_l, filt2, dst0_l, src76_l, filt3,
                      dst0_l, dst0_l, dst0_l, dst0_l);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src21_l, filt0, dst1_l, src43_l,
                      filt1, dst1_l, src65_l, filt2, dst1_l, src87_l, filt3,
                      dst1_l, dst1_l, dst1_l, dst1_l);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src32_l, filt0, dst2_l, src54_l,
                      filt1, dst2_l, src76_l, filt2, dst2_l, src98_l, filt3,
                      dst2_l, dst2_l, dst2_l, dst2_l);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, src43_l, filt0, dst3_l, src65_l,
                      filt1, dst3_l, src87_l, filt2, dst3_l, src109_l, filt3,
                      dst3_l, dst3_l, dst3_l, dst3_l);

            __lsx_vst(dst0_r, dst_tmp, 0);
            __lsx_vst(dst1_r, dst_tmp + dst_stride, 0);
            __lsx_vst(dst2_r, dst_tmp + dst_stride_2x, 0);
            __lsx_vst(dst3_r, dst_tmp + dst_stride_3x, 0);
            __lsx_vst(dst0_l, dst_tmp, 16);
            __lsx_vst(dst1_l, dst_tmp + dst_stride, 16);
            __lsx_vst(dst2_l, dst_tmp + dst_stride_2x, 16);
            __lsx_vst(dst3_l, dst_tmp + dst_stride_3x, 16);
            dst_tmp += dst_stride_4x;

            src10_r = src54_r;
            src32_r = src76_r;
            src54_r = src98_r;
            src21_r = src65_r;
            src43_r = src87_r;
            src65_r = src109_r;
            src10_l = src54_l;
            src32_l = src76_l;
            src54_l = src98_l;
            src21_l = src65_l;
            src43_l = src87_l;
            src65_l = src109_l;
            src6 = src10;
        }
        src += 16;
        dst += 16;
    }
}

static void hevc_vt_8t_16w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    hevc_vt_8t_16multx4mult_lsx(src, src_stride, dst, dst_stride,
                                filter, height, 16);
}

static void hevc_vt_8t_24w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    hevc_vt_8t_16multx4mult_lsx(src, src_stride, dst, dst_stride,
                                filter, height, 16);
    hevc_vt_8t_8w_lsx(src + 16, src_stride, dst + 16, dst_stride,
                      filter, height);
}

static void hevc_vt_8t_32w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    hevc_vt_8t_16multx4mult_lsx(src, src_stride, dst, dst_stride,
                                filter, height, 32);
}

static void hevc_vt_8t_48w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    hevc_vt_8t_16multx4mult_lsx(src, src_stride, dst, dst_stride,
                                filter, height, 48);
}

static void hevc_vt_8t_64w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter, int32_t height)
{
    hevc_vt_8t_16multx4mult_lsx(src, src_stride, dst, dst_stride,
                                filter, height, 64);
}

static void hevc_hv_8t_4w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter_x, const int8_t *filter_y,
                              int32_t height)
{
    uint32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i filt0, filt1, filt2, filt3;
    __m128i filt_h0, filt_h1, filt_h2, filt_h3;
    __m128i mask1, mask2, mask3;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;
    __m128i dst30, dst41, dst52, dst63, dst66, dst97, dst108;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i dst10_r, dst32_r, dst54_r, dst76_r, dst98_r;
    __m128i dst21_r, dst43_r, dst65_r, dst87_r, dst109_r;
    __m128i mask0;

    mask0 = __lsx_vld(ff_hevc_mask_arr, 16);

    src -= src_stride_3x + 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filter_x, 4,
              filter_x, 6, filt0, filt1, filt2, filt3);
    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);

    DUP4_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filter_vec, 2,
              filter_vec, 3, filt_h0, filt_h1, filt_h2, filt_h3);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0, src1,
              src2, src3);
    DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 = __lsx_vxori_b(src6, 128);

    DUP4_ARG3(__lsx_vshuf_b, src3, src0, mask0, src3, src0, mask1, src3, src0,
              mask2, src3, src0, mask3, vec0, vec1, vec2, vec3);
    DUP4_ARG3(__lsx_vshuf_b, src4, src1, mask0, src4, src1, mask1, src4, src1,
              mask2, src4, src1, mask3, vec4, vec5, vec6, vec7);
    DUP4_ARG3(__lsx_vshuf_b, src5, src2, mask0, src5, src2, mask1, src5, src2,
              mask2, src5, src2, mask3, vec8, vec9, vec10, vec11);
    DUP4_ARG3(__lsx_vshuf_b, src6, src3, mask0, src6, src3, mask1, src6, src3,
              mask2, src6, src3, mask3, vec12, vec13, vec14, vec15);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst30, vec1, filt1, dst30,
              vec2, filt2, dst30, vec3, filt3, dst30, dst30, dst30, dst30);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst41, vec5, filt1, dst41,
              vec6, filt2, dst41, vec7, filt3, dst41, dst41, dst41, dst41);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst52, vec9, filt1, dst52,
              vec10, filt2, dst52, vec11, filt3, dst52, dst52, dst52, dst52);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec12, filt0, dst63, vec13, filt1, dst63,
              vec14, filt2, dst63, vec15, filt3, dst63, dst63, dst63, dst63);

    DUP2_ARG2(__lsx_vilvl_h, dst41, dst30, dst52, dst41, dst10_r, dst21_r);
    DUP2_ARG2(__lsx_vilvh_h, dst41, dst30, dst52, dst41, dst43_r, dst54_r);
    dst32_r = __lsx_vilvl_h(dst63, dst52);
    dst65_r = __lsx_vilvh_h(dst63, dst52);
    dst66 = __lsx_vreplvei_d(dst63, 1);

    for (loop_cnt = height >> 2; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128,
                  src7, src8, src9, src10);

        DUP4_ARG3(__lsx_vshuf_b, src9, src7, mask0, src9, src7, mask1, src9, src7,
                  mask2, src9, src7, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vshuf_b, src10, src8, mask0, src10, src8, mask1, src10, src8,
                  mask2, src10, src8, mask3, vec4, vec5, vec6, vec7);

        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst97, vec1, filt1,
                  dst97, vec2, filt2, dst97, vec3, filt3, dst97, dst97, dst97, dst97);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst108, vec5, filt1,
                  dst108, vec6, filt2, dst108, vec7, filt3, dst108, dst108, dst108, dst108);

        DUP2_ARG2(__lsx_vilvl_h, dst97, dst66, dst108, dst97, dst76_r, dst87_r);
        dst109_r = __lsx_vilvh_h(dst108, dst97);
        dst66 = __lsx_vreplvei_d(dst97, 1);
        dst98_r = __lsx_vilvl_h(dst66, dst108);

        dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r,
                                        filt_h0, filt_h1, filt_h2, filt_h3);
        dst1_r = __lsx_hevc_filt_8tap_w(dst21_r, dst43_r, dst65_r, dst87_r,
                                        filt_h0, filt_h1, filt_h2, filt_h3);
        dst2_r = __lsx_hevc_filt_8tap_w(dst32_r, dst54_r, dst76_r, dst98_r,
                                        filt_h0, filt_h1, filt_h2, filt_h3);
        dst3_r = __lsx_hevc_filt_8tap_w(dst43_r, dst65_r, dst87_r, dst109_r,
                                        filt_h0, filt_h1, filt_h2, filt_h3);
        DUP4_ARG2(__lsx_vsrli_w, dst0_r, 6, dst1_r, 6, dst2_r, 6, dst3_r, 6,
                  dst0_r, dst1_r, dst2_r, dst3_r);
        DUP2_ARG2(__lsx_vpickev_h, dst1_r, dst0_r, dst3_r, dst2_r, dst0_r, dst2_r);
        __lsx_vstelm_d(dst0_r, dst, 0, 0);
        __lsx_vstelm_d(dst0_r, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst2_r, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst2_r, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;

        dst10_r = dst54_r;
        dst32_r = dst76_r;
        dst54_r = dst98_r;
        dst21_r = dst65_r;
        dst43_r = dst87_r;
        dst65_r = dst109_r;
        dst66 = __lsx_vreplvei_d(dst108, 1);
    }
}

static void hevc_hv_8t_8multx1mult_lsx(uint8_t *src,
                                       int32_t src_stride,
                                       int16_t *dst,
                                       int32_t dst_stride,
                                       const int8_t *filter_x,
                                       const int8_t *filter_y,
                                       int32_t height,
                                       int32_t width)
{
    uint32_t loop_cnt, cnt;
    uint8_t *src_tmp;
    int16_t *dst_tmp;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i filt0, filt1, filt2, filt3;
    __m128i filt_h0, filt_h1, filt_h2, filt_h3;
    __m128i mask1, mask2, mask3;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i dst0_r, dst0_l;
    __m128i dst10_r, dst32_r, dst54_r, dst76_r;
    __m128i dst10_l, dst32_l, dst54_l, dst76_l;
    __m128i mask0 = {0x403030202010100, 0x807070606050504};

    src -= src_stride_3x + 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filter_x, 4,
              filter_x, 6, filt0, filt1, filt2, filt3);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);

    DUP4_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filter_vec, 2,
              filter_vec, 3, filt_h0, filt_h1, filt_h2, filt_h3);

    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (cnt = width >> 3; cnt--;) {
        src_tmp = src;
        dst_tmp = dst;
        DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src_tmp + src_stride_2x,
                  0, src_tmp + src_stride_3x, 0, src0, src1, src2, src3);
        src_tmp += src_stride_4x;
        DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src_tmp + src_stride_2x, 0);
        src_tmp += src_stride_3x;
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);
        DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);

        /* row 0 row 1 row 2 row 3 */
        DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
                  mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
                  mask2, src1, src1, mask3, vec4, vec5, vec6, vec7);
        DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
                  mask2, src2, src2, mask3, vec8, vec9, vec10, vec11);
        DUP4_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, src3, src3,
                  mask2, src3, src3, mask3, vec12, vec13, vec14, vec15);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
                  vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst1, vec5, filt1, dst1,
                  vec6, filt2, dst1, vec7, filt3, dst1, dst1, dst1, dst1);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst2, vec9, filt1, dst2,
                  vec10, filt2, dst2, vec11, filt3, dst2, dst2, dst2, dst2);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec12, filt0, dst3, vec13, filt1,
                  dst3, vec14, filt2, dst3, vec15, filt3, dst3, dst3, dst3, dst3);

        /* row 4 row 5 row 6 */
        DUP4_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, src4, src4,
                  mask2, src4, src4, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, src5, src5,
                  mask2, src5, src5, mask3, vec4, vec5, vec6, vec7);
        DUP4_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, src6, src6,
                  mask2, src6, src6, mask3, vec8, vec9, vec10, vec11);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst4, filt1, vec1, dst4,
                  vec2, filt2, dst4, vec3, filt3, dst4, dst4, dst4, dst4);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst5, vec5, filt1, dst5,
                  vec6, filt2, dst5, vec7, filt3, dst5, dst5, dst5, dst5);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst6, vec9, filt1, dst6,
                  vec10, filt2, dst6, vec11, filt3, dst6, dst6, dst6, dst6);

        for (loop_cnt = height; loop_cnt--;) {
            src7 = __lsx_vld(src_tmp, 0);
            src7 = __lsx_vxori_b(src7, 128);
            src_tmp += src_stride;

            DUP4_ARG3(__lsx_vshuf_b, src7, src7, mask0, src7, src7, mask1, src7,
                      src7, mask2, src7, src7, mask3, vec0, vec1, vec2, vec3);
            DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst7, vec1, filt1,
                      dst7, vec2, filt2, dst7, vec3, filt3, dst7, dst7, dst7, dst7);

            DUP4_ARG2(__lsx_vilvl_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                      dst10_r, dst32_r, dst54_r, dst76_r);
            DUP4_ARG2(__lsx_vilvh_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                      dst10_l, dst32_l, dst54_l, dst76_l);

            dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst0_l = __lsx_hevc_filt_8tap_w(dst10_l, dst32_l, dst54_l, dst76_l,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst0_r = __lsx_vsrli_w(dst0_r, 6);
            dst0_l = __lsx_vsrli_w(dst0_l, 6);

            dst0_r = __lsx_vpickev_h(dst0_l, dst0_r);
            __lsx_vst(dst0_r, dst_tmp, 0);
            dst_tmp += dst_stride;

            dst0 = dst1;
            dst1 = dst2;
            dst2 = dst3;
            dst3 = dst4;
            dst4 = dst5;
            dst5 = dst6;
            dst6 = dst7;
        }
        src += 8;
        dst += 8;
    }
}

static void hevc_hv_8t_8w_lsx(uint8_t *src, int32_t src_stride,
                              int16_t *dst, int32_t dst_stride,
                              const int8_t *filter_x, const int8_t *filter_y,
                              int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 8);
}

static void hevc_hv_8t_12w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    uint32_t loop_cnt;
    uint8_t *src_tmp;
    int16_t *dst_tmp;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i mask0, mask1, mask2, mask3, mask4, mask5, mask6, mask7;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;
    __m128i filt0, filt1, filt2, filt3, filt_h0, filt_h1, filt_h2, filt_h3;
    __m128i filter_vec, const_vec;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i dst30, dst41, dst52, dst63, dst66, dst97, dst108;
    __m128i dst10_r, dst32_r, dst54_r, dst76_r, dst98_r, dst21_r, dst43_r;
    __m128i dst65_r, dst87_r, dst109_r, dst10_l, dst32_l, dst54_l, dst76_l;
    __m128i dst0_r, dst0_l, dst1_r, dst2_r, dst3_r;

    src -= src_stride_3x + 3;
    DUP4_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filter_x, 4,
              filter_x, 6, filt0, filt1, filt2, filt3);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);

    DUP4_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filter_vec, 2,
              filter_vec, 3, filt_h0, filt_h1, filt_h2, filt_h3);

    mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    src_tmp = src;
    dst_tmp = dst;

    DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
              src_tmp + src_stride_2x, 0, src_tmp + src_stride_3x, 0,
              src0, src1, src2, src3);
    src_tmp += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src_tmp + src_stride_2x, 0);
    src_tmp += src_stride_3x;
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0, src1,
              src2, src3);
    DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 = __lsx_vxori_b(src6, 128);

    /* row 0 row 1 row 2 row 3 */
    DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
              mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
    DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
              mask2, src1, src1, mask3, vec4, vec5, vec6, vec7);
    DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
              mask2, src2, src2, mask3, vec8, vec9, vec10, vec11);
    DUP4_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, src3, src3,
              mask2, src3, src3, mask3, vec12, vec13, vec14, vec15);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0,
              vec2, filt2, dst0, vec3, filt3, dst0, dst0, dst0, dst0);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst1, vec5, filt1, dst1,
              vec6, filt2, dst1, vec7, filt3, dst1, dst1, dst1, dst1);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst2, vec9, filt1, dst2,
              vec10, filt2, dst2, vec11, filt3, dst2, dst2, dst2, dst2);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec12, filt0, dst3, vec13, filt1, dst3,
              vec14, filt2, dst3, vec15, filt3, dst3, dst3, dst3, dst3);

    /* row 4 row 5 row 6 */
    DUP4_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, src4, src4,
              mask2, src4, src4, mask3, vec0, vec1, vec2, vec3);
    DUP4_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, src5, src5,
              mask2, src5, src5, mask3, vec4, vec5, vec6, vec7);
    DUP4_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, src6, src6,
              mask2, src6, src6, mask3, vec8, vec9, vec10, vec11);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst4, vec1, filt1, dst4,
              vec2, filt2, dst4, vec3, filt3, dst4, dst4, dst4, dst4);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst5, vec5, filt1, dst5,
              vec6, filt2, dst5, vec7, filt3, dst5, dst5, dst5, dst5);
    dst6 = const_vec;
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst6, vec9, filt1, dst6,
              vec10, filt2, dst6, vec11, filt3, dst6, dst6, dst6, dst6);

    for (loop_cnt = height; loop_cnt--;) {
        src7 = __lsx_vld(src_tmp, 0);
        src7 = __lsx_vxori_b(src7, 128);
        src_tmp += src_stride;

        DUP4_ARG3(__lsx_vshuf_b, src7, src7, mask0, src7, src7, mask1, src7, src7,
                  mask2, src7, src7, mask3, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst7, vec1, filt1, dst7,
                  vec2, filt2, dst7, vec3, filt3, dst7, dst7, dst7, dst7);
        DUP4_ARG2(__lsx_vilvl_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                  dst10_r, dst32_r, dst54_r, dst76_r);
        DUP4_ARG2(__lsx_vilvh_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                  dst10_l, dst32_l, dst54_l, dst76_l);
        dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        dst0_l = __lsx_hevc_filt_8tap_w(dst10_l, dst32_l, dst54_l, dst76_l, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        dst0_r = __lsx_vsrli_w(dst0_r, 6);
        dst0_l = __lsx_vsrli_w(dst0_l, 6);

        dst0_r = __lsx_vpickev_h(dst0_l, dst0_r);
        __lsx_vst(dst0_r, dst_tmp, 0);
        dst_tmp += dst_stride;

        dst0 = dst1;
        dst1 = dst2;
        dst2 = dst3;
        dst3 = dst4;
        dst4 = dst5;
        dst5 = dst6;
        dst6 = dst7;
    }
    src += 8;
    dst += 8;

    mask4 = __lsx_vld(ff_hevc_mask_arr, 16);
    DUP2_ARG2(__lsx_vaddi_bu, mask4, 2, mask4, 4, mask5, mask6);
    mask7 = __lsx_vaddi_bu(mask4, 6);

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src += src_stride_4x;
    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
              src1, src2, src3)
    DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 = __lsx_vxori_b(src6, 128);

    DUP4_ARG3(__lsx_vshuf_b, src3, src0, mask4, src3, src0, mask5, src3, src0,
              mask6, src3, src0, mask7, vec0, vec1, vec2, vec3);
    DUP4_ARG3(__lsx_vshuf_b, src4, src1, mask4, src4, src1, mask5, src4, src1,
              mask6, src4, src1, mask7, vec4, vec5, vec6, vec7);
    DUP4_ARG3(__lsx_vshuf_b, src5, src2, mask4, src5, src2, mask5, src5, src2,
              mask6, src5, src2, mask7, vec8, vec9, vec10, vec11);
    DUP4_ARG3(__lsx_vshuf_b, src6, src3, mask4, src6, src3, mask5, src6, src3,
              mask6, src6, src3, mask7, vec12, vec13, vec14, vec15);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst30, vec1, filt1, dst30,
              vec2, filt2, dst30, vec3, filt3, dst30, dst30, dst30, dst30);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst41, vec5, filt1, dst41,
              vec6, filt2, dst41, vec7, filt3, dst41, dst41, dst41, dst41);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst52, vec9, filt1, dst52,
              vec10, filt2, dst52, vec11, filt3, dst52, dst52, dst52, dst52);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec12, filt0, dst63, vec13, filt1, dst63,
              vec14, filt2, dst63, vec15, filt3, dst63, dst63, dst63, dst63);

    DUP2_ARG2(__lsx_vilvl_h, dst41, dst30, dst52, dst41, dst10_r, dst21_r);
    DUP2_ARG2(__lsx_vilvh_h, dst41, dst30, dst52, dst41, dst43_r, dst54_r);
    dst32_r = __lsx_vilvl_h(dst63, dst52);
    dst65_r = __lsx_vilvh_h(dst63, dst52);

    dst66 = __lsx_vreplvei_d(dst63, 1);

    for (loop_cnt = height >> 2; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128, src7,
                  src8, src9, src10);

        DUP4_ARG3(__lsx_vshuf_b, src9, src7, mask4, src9, src7, mask5, src9, src7,
                  mask6, src9, src7, mask7, vec0, vec1, vec2, vec3);
        DUP4_ARG3(__lsx_vshuf_b, src10, src8, mask4, src10, src8, mask5, src10,
                  src8, mask6, src10, src8, mask7, vec4, vec5, vec6, vec7);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst97, vec1, filt1,
                  dst97, vec2, filt2, dst97, vec3, filt3, dst97, dst97, dst97, dst97);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst108, vec5, filt1, dst108,
                  vec6, filt2, dst108, vec7, filt3, dst108, dst108, dst108, dst108);

        DUP2_ARG2(__lsx_vilvl_h, dst97, dst66, dst108, dst97, dst76_r, dst87_r);
        dst109_r = __lsx_vilvh_h(dst108, dst97);
        dst66 = __lsx_vreplvei_d(dst97, 1);
        dst98_r = __lsx_vilvl_h(dst66, dst108);

        dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        dst1_r = __lsx_hevc_filt_8tap_w(dst21_r, dst43_r, dst65_r, dst87_r, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        dst2_r = __lsx_hevc_filt_8tap_w(dst32_r, dst54_r, dst76_r, dst98_r, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        dst3_r = __lsx_hevc_filt_8tap_w(dst43_r, dst65_r, dst87_r, dst109_r, filt_h0,
                                        filt_h1, filt_h2, filt_h3);
        DUP4_ARG2(__lsx_vsrli_w, dst0_r, 6, dst1_r, 6, dst2_r, 6, dst3_r, 6,
                  dst0_r, dst1_r, dst2_r, dst3_r);
        DUP2_ARG2(__lsx_vpickev_h, dst1_r, dst0_r, dst3_r, dst2_r, dst0_r, dst2_r);
        __lsx_vstelm_d(dst0_r, dst, 0, 0);
        __lsx_vstelm_d(dst0_r, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst2_r, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst2_r, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;

        dst10_r = dst54_r;
        dst32_r = dst76_r;
        dst54_r = dst98_r;
        dst21_r = dst65_r;
        dst43_r = dst87_r;
        dst65_r = dst109_r;
        dst66 = __lsx_vreplvei_d(dst108, 1);
    }
}

static void hevc_hv_8t_16w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 16);
}

static void hevc_hv_8t_24w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 24);
}

static void hevc_hv_8t_32w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 32);
}

static void hevc_hv_8t_48w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 48);
}

static void hevc_hv_8t_64w_lsx(uint8_t *src, int32_t src_stride,
                               int16_t *dst, int32_t dst_stride,
                               const int8_t *filter_x, const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_8t_8multx1mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 64);
}

static void hevc_hz_4t_32w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter,
                               int32_t height)
{
    uint32_t loop_cnt;
    __m128i src0, src1, src2;
    __m128i filt0, filt1;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    __m128i mask1, mask2, mask3;
    __m128i dst0, dst1, dst2, dst3;
    __m128i vec0, vec1, vec2, vec3;
    __m128i const_vec;

    src -= 1;
    DUP2_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filt0, filt1);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 8, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 10);

    for (loop_cnt = height; loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src, 16, src0, src1);
        src2 = __lsx_vld(src, 24);
        src += src_stride;

        DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
        src2 = __lsx_vxori_b(src2, 128);

        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src1, src0, mask2, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask0, src2, src2, mask0, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, const_vec, vec1, filt0,
                  const_vec, vec2, filt0, const_vec, vec3, filt0, dst0, dst1, dst2,
                  dst3);
        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask1, src1, src0, mask3, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask1, src2, src2, mask1, vec2, vec3);
        DUP4_ARG3(__lsx_vdp2add_h_b, dst0, vec0, filt1, dst1, vec1, filt1, dst2,
                  vec2, filt1, dst3, vec3, filt1, dst0, dst1, dst2, dst3);
        __lsx_vst(dst0, dst, 0);
        __lsx_vst(dst1, dst, 16);
        __lsx_vst(dst2, dst, 32);
        __lsx_vst(dst3, dst, 48);
        dst += dst_stride;
    }
}

static void hevc_vt_4t_16w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter,
                               int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i src0, src1, src2, src3, src4, src5;
    __m128i src10_r, src32_r, src21_r, src43_r;
    __m128i src10_l, src32_l, src21_l, src43_l;
    __m128i dst0_r, dst1_r, dst0_l, dst1_l;
    __m128i filt0, filt1;
    __m128i const_vec;

    src -= src_stride;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP2_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filt0, filt1);

    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
    src2 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
    src2 = __lsx_vxori_b(src2, 128);
    DUP2_ARG2(__lsx_vilvl_b, src1, src0, src2, src1, src10_r, src21_r);
    DUP2_ARG2(__lsx_vilvh_b, src1, src0, src2, src1, src10_l, src21_l);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src3, src4);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src3, src4);
        DUP2_ARG2(__lsx_vilvl_b, src3, src2, src4, src3, src32_r, src43_r);
        DUP2_ARG2(__lsx_vilvh_b, src3, src2, src4, src3, src32_l, src43_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_l, filt0, dst0_l, src32_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_l, filt0, dst1_l, src43_l,
                  filt1, dst1_l, dst1_l);
        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        dst += dst_stride;

        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src5, src2);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src5, 128, src2, 128, src5, src2);
        DUP2_ARG2(__lsx_vilvl_b, src5, src4, src2, src5, src10_r, src21_r);
        DUP2_ARG2(__lsx_vilvh_b, src5, src4, src2, src5, src10_l, src21_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst0_r, src10_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_l, filt0, dst0_l, src10_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst1_r, src21_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_l, filt0, dst1_l, src21_l,
                  filt1, dst1_l, dst1_l);
        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        dst += dst_stride;
    }
}

static void hevc_vt_4t_24w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter,
                               int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_3x = src_stride_2x + src_stride;

    __m128i src0, src1, src2, src3, src4, src5;
    __m128i src6, src7, src8, src9, src10, src11;
    __m128i src10_r, src32_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i src10_l, src32_l, src21_l, src43_l;
    __m128i dst0_l, dst1_l;
    __m128i filt0, filt1;
    __m128i const_vec;

    src -= src_stride;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP2_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filt0, filt1);

    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
    src2 = __lsx_vld(src + src_stride_2x, 0);
    DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
    src2 = __lsx_vxori_b(src2, 128);
    DUP2_ARG2(__lsx_vilvl_b, src1, src0, src2, src1, src10_r, src21_r);
    DUP2_ARG2(__lsx_vilvh_b, src1, src0, src2, src1, src10_l, src21_l);

    DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src6, src7);
    src8 = __lsx_vld(src + src_stride_2x, 16);
    src += src_stride_3x;
    DUP2_ARG2(__lsx_vxori_b, src6, 128, src7, 128, src6, src7);
    src8 = __lsx_vxori_b(src8, 128);
    DUP2_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src76_r, src87_r);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src3, src4);
        DUP2_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src3, src4);
        DUP2_ARG2(__lsx_vilvl_b, src3, src2, src4, src3, src32_r, src43_r);
        DUP2_ARG2(__lsx_vilvh_b, src3, src2, src4, src3, src32_l, src43_l);

        DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src9, src10);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src9, 128, src10, 128, src9, src10);
        DUP2_ARG2(__lsx_vilvl_b, src9, src8, src10, src9, src98_r, src109_r);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_l, filt0, dst0_l, src32_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_l, filt0, dst1_l, src43_l,
                  filt1, dst1_l, dst1_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src76_r, filt0, dst2_r, src98_r,
                  filt1, dst2_r, dst2_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src87_r, filt0, dst3_r, src109_r,
                  filt1, dst3_r, dst3_r);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        __lsx_vst(dst2_r, dst, 32);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        __lsx_vst(dst3_r, dst, 32);
        dst += dst_stride;

        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src5, src2);
        DUP2_ARG2(__lsx_vxori_b, src5, 128, src2, 128, src5, src2);
        DUP2_ARG2(__lsx_vilvl_b, src5, src4, src2, src5, src10_r, src21_r);
        DUP2_ARG2(__lsx_vilvh_b, src5, src4, src2, src5, src10_l, src21_l);

        DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src11, src8);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src11, 128, src8, 128, src11, src8);
        DUP2_ARG2(__lsx_vilvl_b, src11, src10, src8, src11, src76_r, src87_r);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst0_r, src10_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_l, filt0, dst0_l, src10_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst1_r, src21_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_l, filt0, dst1_l, src21_l,
                  filt1, dst1_l, dst1_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src98_r, filt0, dst2_r, src76_r,
                  filt1, dst2_r, dst2_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src109_r, filt0, dst3_r, src87_r,
                  filt1, dst3_r, dst3_r);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        __lsx_vst(dst2_r, dst, 32);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        __lsx_vst(dst3_r, dst, 32);
        dst += dst_stride;
    }
}

static void hevc_vt_4t_32w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter,
                               int32_t height)
{
    int32_t loop_cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_3x = src_stride_2x + src_stride;

    __m128i src0, src1, src2, src3, src4, src5;
    __m128i src6, src7, src8, src9, src10, src11;
    __m128i src10_r, src32_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i src10_l, src32_l, src76_l, src98_l;
    __m128i src21_l, src43_l, src87_l, src109_l;
    __m128i dst0_l, dst1_l, dst2_l, dst3_l;
    __m128i filt0, filt1;
    __m128i const_vec;

    src -= src_stride;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6
    DUP2_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filt0, filt1);

    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
    src2 = __lsx_vld(src + src_stride_2x, 0);
    DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1)
    src2 = __lsx_vxori_b(src2, 128);
    DUP2_ARG2(__lsx_vilvl_b, src1, src0, src2, src1, src10_r, src21_r);
    DUP2_ARG2(__lsx_vilvh_b, src1, src0, src2, src1, src10_l, src21_l);

    DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src6, src7);
    src8 = __lsx_vld(src + src_stride_2x, 16);
    src += src_stride_3x;
    DUP2_ARG2(__lsx_vxori_b, src6, 128, src7, 128, src6, src7);
    src8 = __lsx_vxori_b(src8, 128);
    DUP2_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src76_r, src87_r);
    DUP2_ARG2(__lsx_vilvh_b, src7, src6, src8, src7, src76_l, src87_l);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src3, src4);
        DUP2_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src3, src4);
        DUP2_ARG2(__lsx_vilvl_b, src3, src2, src4, src3, src32_r, src43_r);
        DUP2_ARG2(__lsx_vilvh_b, src3, src2, src4, src3, src32_l, src43_l);

        DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src9, src10);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src9, 128, src10, 128, src9, src10);
        DUP2_ARG2(__lsx_vilvl_b, src9, src8, src10, src9, src98_r, src109_r);
        DUP2_ARG2(__lsx_vilvh_b, src9, src8, src10, src9, src98_l, src109_l);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src10_l, filt0, dst0_l, src32_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src21_l, filt0, dst1_l,src43_l,
                  filt1, dst1_l, dst1_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src76_r, filt0, dst2_r, src98_r,
                  filt1, dst2_r, dst2_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src76_l, filt0, dst2_l, src98_l,
                  filt1, dst2_l, dst2_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src87_r, filt0, dst3_r, src109_r,
                  filt1, dst3_r, dst3_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src87_l, filt0, dst3_l, src109_l,
                  filt1, dst3_l, dst3_l);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        __lsx_vst(dst2_r, dst, 32);
        __lsx_vst(dst2_l, dst, 48);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        __lsx_vst(dst3_r, dst, 32);
        __lsx_vst(dst3_l, dst, 48);
        dst += dst_stride;

        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src5, src2);
        DUP2_ARG2(__lsx_vxori_b, src5, 128, src2, 128, src5, src2);
        DUP2_ARG2(__lsx_vilvl_b, src5, src4, src2, src5, src10_r, src21_r);
        DUP2_ARG2(__lsx_vilvh_b, src5, src4, src2, src5, src10_l, src21_l);

        DUP2_ARG2(__lsx_vld, src, 16, src + src_stride, 16, src11, src8);
        src += src_stride_2x;
        DUP2_ARG2(__lsx_vxori_b, src11, 128, src8, 128, src11, src8);
        DUP2_ARG2(__lsx_vilvl_b, src11, src10, src8, src11, src76_r, src87_r);
        DUP2_ARG2(__lsx_vilvh_b, src11, src10, src8, src11, src76_l, src87_l);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_r, filt0, dst0_r, src10_r,
                  filt1, dst0_r, dst0_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src32_l, filt0, dst0_l, src10_l,
                  filt1, dst0_l, dst0_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_r, filt0, dst1_r, src21_r,
                  filt1, dst1_r, dst1_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src43_l, filt0, dst1_l, src21_l,
                  filt1, dst1_l, dst1_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src98_r, filt0, dst2_r, src76_r,
                  filt1, dst2_r, dst2_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src98_l, filt0, dst2_l, src76_l,
                  filt1, dst2_l, dst2_l);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src109_r, filt0, dst3_r, src87_r,
                  filt1, dst3_r, dst3_r);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, src109_l, filt0, dst3_l, src87_l,
                  filt1, dst3_l, dst3_l);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst0_l, dst, 16);
        __lsx_vst(dst2_r, dst, 32);
        __lsx_vst(dst2_l, dst, 48);
        dst += dst_stride;
        __lsx_vst(dst1_r, dst, 0);
        __lsx_vst(dst1_l, dst, 16);
        __lsx_vst(dst3_r, dst, 32);
        __lsx_vst(dst3_l, dst, 48);
        dst += dst_stride;
    }
}

static void hevc_hv_4t_8x2_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y)
{
    int32_t src_stride_2x = (src_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;

    __m128i src0, src1, src2, src3, src4;
    __m128i filt0, filt1;
    __m128i filt_h0, filt_h1;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    __m128i mask1;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9;
    __m128i dst0, dst1, dst2, dst3, dst4;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l;
    __m128i dst10_r, dst32_r, dst21_r, dst43_r;
    __m128i dst10_l, dst32_l, dst21_l, dst43_l;

    src -= (src_stride + 1);
    DUP2_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filt0, filt1);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    DUP2_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filt_h0, filt_h1);

    mask1 = __lsx_vaddi_bu(mask0, 2);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src4 = __lsx_vld(src + src_stride_4x, 0);
    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
              src1, src2, src3);
    src4 = __lsx_vxori_b(src4, 128);

    DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, vec0, vec1);
    DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, vec2, vec3);
    DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, vec4, vec5);
    DUP2_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, vec6, vec7);
    DUP2_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, vec8, vec9);

    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1,
              dst0, dst0);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst1, vec3, filt1,
              dst1, dst1);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst2, vec5, filt1,
              dst2, dst2);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec6, filt0, dst3, vec7, filt1,
              dst3, dst3);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst4, vec9, filt1,
              dst4, dst4);

    DUP2_ARG2(__lsx_vilvl_h, dst1, dst0, dst2, dst1, dst10_r, dst21_r);
    DUP2_ARG2(__lsx_vilvh_h, dst1, dst0, dst2, dst1, dst10_l, dst21_l);
    DUP2_ARG2(__lsx_vilvl_h, dst3, dst2, dst4, dst3, dst32_r, dst43_r);
    DUP2_ARG2(__lsx_vilvh_h, dst3, dst2, dst4, dst3, dst32_l, dst43_l);

    dst0_r = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
    dst0_l = __lsx_hevc_filt_4tap_w(dst10_l, dst32_l, filt_h0, filt_h1);
    dst1_r = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
    dst1_l = __lsx_hevc_filt_4tap_w(dst21_l, dst43_l, filt_h0, filt_h1);
    DUP4_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst1_r, 6, dst1_l, 6, dst0_r,
              dst0_l, dst1_r, dst1_l);
    DUP2_ARG2(__lsx_vpickev_h, dst0_l, dst0_r, dst1_l, dst1_r, dst0_r, dst1_r);
    __lsx_vst(dst0_r, dst, 0);
    __lsx_vst(dst1_r, dst + dst_stride, 0);
}

static void hevc_hv_4t_8multx4_lsx(uint8_t *src, int32_t src_stride,
                                   int16_t *dst, int32_t dst_stride,
                                   const int8_t *filter_x,
                                   const int8_t *filter_y, int32_t width8mult)
{
    int32_t cnt;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i src0, src1, src2, src3, src4, src5, src6, mask0, mask1;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i filt0, filt1, filt_h0, filt_h1, filter_vec, const_vec;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6;
    __m128i dst10_r, dst32_r, dst54_r, dst21_r, dst43_r, dst65_r;
    __m128i dst10_l, dst32_l, dst54_l, dst21_l, dst43_l, dst65_l;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l, dst2_r, dst2_l, dst3_r, dst3_l;

    src -= (src_stride + 1);
    DUP2_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filt0, filt1);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    DUP2_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filt_h0, filt_h1);

    mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    mask1 = __lsx_vaddi_bu(mask0, 2);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (cnt = width8mult; cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src0, src1, src2, src3);
        src += src_stride_4x;
        DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src + src_stride_2x, 0);
        src += (8 - src_stride_4x);
        DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3)
        DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);

        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, vec4, vec5);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0, dst0);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst1, vec3, filt1, dst1, dst1);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst2, vec5, filt1, dst2, dst2);

        DUP2_ARG2(__lsx_vilvl_h, dst1, dst0, dst2, dst1, dst10_r, dst21_r);
        DUP2_ARG2(__lsx_vilvh_h, dst1, dst0, dst2, dst1, dst10_l, dst21_l);

        DUP2_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, vec4, vec5);
        DUP2_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, vec6, vec7);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst4, vec3, filt1, dst4, dst4);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst5, vec5, filt1, dst5, dst5);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec6, filt0, dst6, vec7, filt1, dst6, dst6);
        DUP2_ARG2(__lsx_vilvl_h, dst3, dst2, dst4, dst3, dst32_r, dst43_r);
        DUP2_ARG2(__lsx_vilvh_h, dst3, dst2, dst4, dst3, dst32_l, dst43_l);
        DUP2_ARG2(__lsx_vilvl_h, dst5, dst4, dst6, dst5, dst54_r, dst65_r);
        DUP2_ARG2(__lsx_vilvh_h, dst5, dst4, dst6, dst5, dst54_l, dst65_l);

        dst0_r = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
        dst0_l = __lsx_hevc_filt_4tap_w(dst10_l, dst32_l, filt_h0, filt_h1);
        dst1_r = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
        dst1_l = __lsx_hevc_filt_4tap_w(dst21_l, dst43_l, filt_h0, filt_h1);

        dst2_r = __lsx_hevc_filt_4tap_w(dst32_r, dst54_r, filt_h0, filt_h1);
        dst2_l = __lsx_hevc_filt_4tap_w(dst32_l, dst54_l, filt_h0, filt_h1);
        dst3_r = __lsx_hevc_filt_4tap_w(dst43_r, dst65_r, filt_h0, filt_h1);
        dst3_l = __lsx_hevc_filt_4tap_w(dst43_l, dst65_l, filt_h0, filt_h1);
        DUP4_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst1_r, 6, dst1_l, 6,
                  dst0_r, dst0_l, dst1_r, dst1_l);
        DUP4_ARG2(__lsx_vsrai_w, dst2_r, 6, dst2_l, 6, dst3_r, 6, dst3_l, 6,
                  dst2_r, dst2_l, dst3_r, dst3_l);
        DUP2_ARG2(__lsx_vpickev_h, dst0_l, dst0_r, dst1_l, dst1_r, dst0_r, dst1_r);
        DUP2_ARG2(__lsx_vpickev_h, dst2_l, dst2_r, dst3_l, dst3_r, dst2_r, dst3_r);

        __lsx_vst(dst0_r, dst, 0);
        __lsx_vst(dst1_r, dst + dst_stride, 0);
        __lsx_vst(dst2_r, dst + dst_stride_2x, 0);
        __lsx_vst(dst3_r, dst + dst_stride_3x, 0);
        dst += 8;
    }
}

static void hevc_hv_4t_8x6_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y)
{
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i filt0, filt1;
    __m128i filt_h0, filt_h1;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    __m128i mask1;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec8, vec9;
    __m128i vec10, vec11, vec12, vec13, vec14, vec15, vec16, vec17;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l, dst2_r, dst2_l, dst3_r, dst3_l;
    __m128i dst4_r, dst4_l, dst5_r, dst5_l;
    __m128i dst10_r, dst32_r, dst10_l, dst32_l;
    __m128i dst21_r, dst43_r, dst21_l, dst43_l;
    __m128i dst54_r, dst54_l, dst65_r, dst65_l;
    __m128i dst76_r, dst76_l, dst87_r, dst87_l;

    src -= (src_stride + 1);
    DUP2_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filt0, filt1);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    DUP2_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filt_h0, filt_h1);

    mask1 = __lsx_vaddi_bu(mask0, 2);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src0, src1, src2, src3);
    src4 = __lsx_vld(src + src_stride_4x, 0);
    src += (src_stride_4x + src_stride);
    DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
              src + src_stride_3x, 0, src5, src6, src7, src8);

    DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
              src1, src2, src3);
    src4 = __lsx_vxori_b(src4, 128);
    DUP4_ARG2(__lsx_vxori_b, src5, 128, src6, 128, src7, 128, src8, 128, src5,
              src6, src7, src8);

    DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src1, src1,
              mask0, src1, src1, mask1, vec0, vec1, vec2, vec3);
    DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1,src3, src3,
              mask0, src3, src3, mask1, vec4, vec5, vec6, vec7);
    DUP4_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, src5, src5,
              mask0, src5, src5, mask1, vec8, vec9, vec10, vec11);
    DUP4_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, src7, src7,
              mask0, src7, src7, mask1, vec12, vec13, vec14, vec15);
    DUP2_ARG3(__lsx_vshuf_b, src8, src8, mask0, src8, src8, mask1, vec16, vec17);

    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1,
              const_vec, vec2, filt0, dst1, vec3, filt1, dst0, dst0, dst1, dst1);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst2, vec5, filt1,
              const_vec, vec6, filt0, dst3, vec7, filt1, dst2, dst2, dst3, dst3);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec8, filt0, dst4, vec9, filt1,
              const_vec, vec10, filt0, dst5, vec11, filt1, dst4, dst4, dst5, dst5);
    DUP4_ARG3(__lsx_vdp2add_h_b, const_vec, vec12, filt0, dst6, vec13, filt1,
              const_vec, vec14, filt0, dst7, vec15, filt1, dst6, dst6, dst7, dst7);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec16, filt0, dst8, vec17, filt1, dst8, dst8);

    DUP4_ARG2(__lsx_vilvl_h, dst1, dst0, dst2, dst1, dst3, dst2, dst4, dst3,
              dst10_r, dst21_r, dst32_r, dst43_r);
    DUP4_ARG2(__lsx_vilvh_h, dst1, dst0, dst2, dst1, dst3, dst2, dst4, dst3,
              dst10_l, dst21_l, dst32_l, dst43_l);
    DUP4_ARG2(__lsx_vilvl_h, dst5, dst4, dst6, dst5, dst7, dst6, dst8, dst7,
              dst54_r, dst65_r, dst76_r, dst87_r);
    DUP4_ARG2(__lsx_vilvh_h, dst5, dst4, dst6, dst5, dst7, dst6, dst8, dst7,
              dst54_l, dst65_l, dst76_l, dst87_l);

    dst0_r = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
    dst0_l = __lsx_hevc_filt_4tap_w(dst10_l, dst32_l, filt_h0, filt_h1);
    dst1_r = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
    dst1_l = __lsx_hevc_filt_4tap_w(dst21_l, dst43_l, filt_h0, filt_h1);
    dst2_r = __lsx_hevc_filt_4tap_w(dst32_r, dst54_r, filt_h0, filt_h1);
    dst2_l = __lsx_hevc_filt_4tap_w(dst32_l, dst54_l, filt_h0, filt_h1);
    dst3_r = __lsx_hevc_filt_4tap_w(dst43_r, dst65_r, filt_h0, filt_h1);
    dst3_l = __lsx_hevc_filt_4tap_w(dst43_l, dst65_l, filt_h0, filt_h1);
    dst4_r = __lsx_hevc_filt_4tap_w(dst54_r, dst76_r, filt_h0, filt_h1);
    dst4_l = __lsx_hevc_filt_4tap_w(dst54_l, dst76_l, filt_h0, filt_h1);
    dst5_r = __lsx_hevc_filt_4tap_w(dst65_r, dst87_r, filt_h0, filt_h1);
    dst5_l = __lsx_hevc_filt_4tap_w(dst65_l, dst87_l, filt_h0, filt_h1);

    DUP4_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst1_r, 6, dst1_l, 6, dst0_r,
              dst0_l, dst1_r, dst1_l);
    DUP4_ARG2(__lsx_vsrai_w, dst2_r, 6, dst2_l, 6, dst3_r, 6, dst3_l, 6, dst2_r,
              dst2_l, dst3_r, dst3_l);
    DUP4_ARG2(__lsx_vsrai_w, dst4_r, 6, dst4_l, 6, dst5_r, 6, dst5_l, 6, dst4_r,
              dst4_l, dst5_r, dst5_l);

    DUP4_ARG2(__lsx_vpickev_h,dst0_l, dst0_r, dst1_l, dst1_r, dst2_l, dst2_r,
              dst3_l, dst3_r, dst0_r, dst1_r, dst2_r, dst3_r);
    DUP2_ARG2(__lsx_vpickev_h, dst4_l, dst4_r, dst5_l, dst5_r, dst4_r, dst5_r);

    __lsx_vst(dst0_r, dst, 0);
    __lsx_vst(dst1_r, dst + dst_stride, 0);
    dst += dst_stride_2x;
    __lsx_vst(dst2_r, dst, 0);
    __lsx_vst(dst3_r, dst + dst_stride, 0);
    dst += dst_stride_2x;
    __lsx_vst(dst4_r, dst, 0);
    __lsx_vst(dst5_r, dst + dst_stride, 0);
}

static void hevc_hv_4t_8multx4mult_lsx(uint8_t *src,
                                       int32_t src_stride,
                                       int16_t *dst,
                                       int32_t dst_stride,
                                       const int8_t *filter_x,
                                       const int8_t *filter_y,
                                       int32_t height,
                                       int32_t width8mult)
{
    uint32_t loop_cnt, cnt;
    uint8_t *src_tmp;
    int16_t *dst_tmp;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i src0, src1, src2, src3, src4, src5, src6;
    __m128i filt0, filt1;
    __m128i filt_h0, filt_h1;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    __m128i mask1;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l, dst2_r, dst2_l, dst3_r, dst3_l;
    __m128i dst10_r, dst32_r, dst54_r, dst21_r, dst43_r, dst65_r;
    __m128i dst10_l, dst32_l, dst54_l, dst21_l, dst43_l, dst65_l;

    src -= (src_stride + 1);
    DUP2_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filt0, filt1);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    DUP2_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filt_h0, filt_h1);

    mask1 = __lsx_vaddi_bu(mask0, 2);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    for (cnt = width8mult; cnt--;) {
        src_tmp = src;
        dst_tmp = dst;

        DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src0, src1);
        src2 = __lsx_vld(src_tmp + src_stride_2x, 0);
        src_tmp += src_stride_3x;

        DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
        src2 = __lsx_vxori_b(src2, 128);

        DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, vec4, vec5);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0, dst0);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst1, vec3, filt1, dst1, dst1);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst2, vec5, filt1, dst2, dst2);

        DUP2_ARG2(__lsx_vilvl_h, dst1, dst0, dst2, dst1, dst10_r, dst21_r);
        DUP2_ARG2(__lsx_vilvh_h, dst1, dst0, dst2, dst1, dst10_l, dst21_l);

        for (loop_cnt = height >> 2; loop_cnt--;) {
            DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                      src_tmp + src_stride_2x, 0, src_tmp + src_stride_3x, 0,
                      src3, src4, src5, src6);
            src_tmp += src_stride_4x;
            DUP4_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src5, 128, src6, 128,
                      src3, src4, src5, src6);

            DUP2_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, vec0, vec1);
            DUP2_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, vec2, vec3);
            DUP2_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, vec4, vec5);
            DUP2_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, vec6, vec7);

            DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1,
                      dst3, dst3);
            DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst4, vec3, filt1,
                      dst4, dst4);
            DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst5, vec5, filt1,
                      dst5, dst5);
            DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec6, filt0, dst6, vec7, filt1,
                      dst6, dst6);

            DUP2_ARG2(__lsx_vilvl_h, dst3, dst2, dst4, dst3, dst32_r, dst43_r);
            DUP2_ARG2(__lsx_vilvh_h, dst3, dst2, dst4, dst3, dst32_l, dst43_l);
            DUP2_ARG2(__lsx_vilvl_h, dst5, dst4, dst6, dst5, dst54_r, dst65_r);
            DUP2_ARG2(__lsx_vilvh_h, dst5, dst4, dst6, dst5, dst54_l, dst65_l);

            dst0_r = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
            dst0_l = __lsx_hevc_filt_4tap_w(dst10_l, dst32_l, filt_h0, filt_h1);
            dst1_r = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
            dst1_l = __lsx_hevc_filt_4tap_w(dst21_l, dst43_l, filt_h0, filt_h1);
            dst2_r = __lsx_hevc_filt_4tap_w(dst32_r, dst54_r, filt_h0, filt_h1);
            dst2_l = __lsx_hevc_filt_4tap_w(dst32_l, dst54_l, filt_h0, filt_h1);
            dst3_r = __lsx_hevc_filt_4tap_w(dst43_r, dst65_r, filt_h0, filt_h1);
            dst3_l = __lsx_hevc_filt_4tap_w(dst43_l, dst65_l, filt_h0, filt_h1);

            DUP4_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst1_r, 6, dst1_l, 6,
                      dst0_r, dst0_l, dst1_r, dst1_l);
            DUP4_ARG2(__lsx_vsrai_w, dst2_r, 6, dst2_l, 6, dst3_r, 6, dst3_l, 6,
                      dst2_r, dst2_l, dst3_r, dst3_l);

            DUP4_ARG2(__lsx_vpickev_h, dst0_l, dst0_r, dst1_l, dst1_r, dst2_l,
                      dst2_r, dst3_l, dst3_r, dst0_r, dst1_r, dst2_r, dst3_r);

            __lsx_vst(dst0_r, dst_tmp, 0);
            __lsx_vst(dst1_r, dst_tmp + dst_stride, 0);
            __lsx_vst(dst2_r, dst_tmp + dst_stride_2x, 0);
            __lsx_vst(dst3_r, dst_tmp + dst_stride_3x, 0);
            dst_tmp += dst_stride_4x;

            dst10_r = dst54_r;
            dst10_l = dst54_l;
            dst21_r = dst65_r;
            dst21_l = dst65_l;
            dst2 = dst6;
        }
        src += 8;
        dst += 8;
    }
}

static void hevc_hv_4t_8w_lsx(uint8_t *src,
                              int32_t src_stride,
                              int16_t *dst,
                              int32_t dst_stride,
                              const int8_t *filter_x,
                              const int8_t *filter_y,
                              int32_t height)
{

    if (2 == height) {
        hevc_hv_4t_8x2_lsx(src, src_stride, dst, dst_stride,
                           filter_x, filter_y);
    } else if (4 == height) {
        hevc_hv_4t_8multx4_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, 1);
    } else if (6 == height) {
        hevc_hv_4t_8x6_lsx(src, src_stride, dst, dst_stride,
                           filter_x, filter_y);
    } else if (0 == (height & 0x03)) {
        hevc_hv_4t_8multx4mult_lsx(src, src_stride, dst, dst_stride,
                                   filter_x, filter_y, height, 1);
    }
}

static void hevc_hv_4t_12w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y,
                               int32_t height)
{
    uint32_t loop_cnt;
    uint8_t *src_tmp;
    int16_t *dst_tmp;
    int32_t src_stride_2x = (src_stride << 1);
    int32_t dst_stride_2x = (dst_stride << 1);
    int32_t src_stride_4x = (src_stride << 2);
    int32_t dst_stride_4x = (dst_stride << 2);
    int32_t src_stride_3x = src_stride_2x + src_stride;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8, src9, src10;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i mask0, mask1, mask2, mask3;
    __m128i filt0, filt1, filt_h0, filt_h1, filter_vec, const_vec;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst10, dst21, dst22, dst73;
    __m128i dst84, dst95, dst106, dst76_r, dst98_r, dst87_r, dst109_r;
    __m128i dst10_r, dst32_r, dst54_r, dst21_r, dst43_r, dst65_r;
    __m128i dst10_l, dst32_l, dst54_l, dst21_l, dst43_l, dst65_l;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l, dst2_r, dst2_l, dst3_r, dst3_l;
    __m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

    src -= (src_stride + 1);
    DUP2_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filt0, filt1);

    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    DUP2_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filt_h0, filt_h1);

    mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    mask1 = __lsx_vaddi_bu(mask0, 2);
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    src_tmp = src;
    dst_tmp = dst;

    DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src0, src1);
    src2 = __lsx_vld(src_tmp + src_stride_2x, 0);
    src_tmp += src_stride_3x;

    DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
    src2 = __lsx_vxori_b(src2, 128);

    DUP2_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, vec0, vec1);
    DUP2_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, vec2, vec3);
    DUP2_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, vec4, vec5);

    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst0, vec1, filt1, dst0, dst0);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst1, vec3, filt1, dst1, dst1);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst2, vec5, filt1, dst2, dst2);

    DUP2_ARG2(__lsx_vilvl_h, dst1, dst0, dst2, dst1, dst10_r, dst21_r);
    DUP2_ARG2(__lsx_vilvh_h, dst1, dst0, dst2, dst1, dst10_l, dst21_l);

    for (loop_cnt = 4; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                  src_tmp + src_stride_2x, 0, src_tmp + src_stride_3x, 0, src3,
                  src4, src5, src6);
        src_tmp += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src5, 128, src6, 128, src3,
                  src4, src5, src6);

        DUP2_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, vec4, vec5);
        DUP2_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, vec6, vec7);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst3, vec1, filt1, dst3, dst3);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst4, vec3, filt1, dst4, dst4);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst5, vec5, filt1, dst5, dst5);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec6, filt0, dst6, vec7, filt1, dst6, dst6);

        DUP2_ARG2(__lsx_vilvl_h, dst3, dst2, dst4, dst3, dst32_r, dst43_r);
        DUP2_ARG2(__lsx_vilvh_h, dst3, dst2, dst4, dst3, dst32_l, dst43_l);
        DUP2_ARG2(__lsx_vilvl_h, dst5, dst4, dst6, dst5, dst54_r, dst65_r);
        DUP2_ARG2(__lsx_vilvh_h, dst5, dst4, dst6, dst5, dst54_l, dst65_l);

        dst0_r = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
        dst0_l = __lsx_hevc_filt_4tap_w(dst10_l, dst32_l, filt_h0, filt_h1);
        dst1_r = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
        dst1_l = __lsx_hevc_filt_4tap_w(dst21_l, dst43_l, filt_h0, filt_h1);
        dst2_r = __lsx_hevc_filt_4tap_w(dst32_r, dst54_r, filt_h0, filt_h1);
        dst2_l = __lsx_hevc_filt_4tap_w(dst32_l, dst54_l, filt_h0, filt_h1);
        dst3_r = __lsx_hevc_filt_4tap_w(dst43_r, dst65_r, filt_h0, filt_h1);
        dst3_l = __lsx_hevc_filt_4tap_w(dst43_l, dst65_l, filt_h0, filt_h1);

        DUP4_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst1_r, 6, dst1_l, 6,
                  dst0_r, dst0_l, dst1_r, dst1_l);
        DUP4_ARG2(__lsx_vsrai_w, dst2_r, 6, dst2_l, 6, dst3_r, 6, dst3_l, 6,
                  dst2_r, dst2_l, dst3_r, dst3_l);
        DUP4_ARG2(__lsx_vpickev_h, dst0_l, dst0_r, dst1_l, dst1_r, dst2_l, dst2_r,
                  dst3_l, dst3_r, dst0_r, dst1_r, dst2_r, dst3_r);
        __lsx_vst(dst0_r, dst_tmp, 0);
        __lsx_vst(dst1_r, dst_tmp + dst_stride, 0);
        __lsx_vst(dst2_r, dst_tmp + dst_stride_2x, 0);
        __lsx_vst(dst3_r, dst_tmp + dst_stride_3x, 0);
        dst_tmp += dst_stride_4x;

        dst10_r = dst54_r;
        dst10_l = dst54_l;
        dst21_r = dst65_r;
        dst21_l = dst65_l;
        dst2 = dst6;
    }

    src += 8;
    dst += 8;

    mask2 = __lsx_vld(ff_hevc_mask_arr, 16);
    mask3 = __lsx_vaddi_bu(mask2, 2);

    DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src0, src1);
    src2 = __lsx_vld(src + src_stride_2x, 0);
    src += src_stride_3x;
    DUP2_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src0, src1);
    src2 = __lsx_vxori_b(src2, 128);
    DUP2_ARG3(__lsx_vshuf_b, src1, src0, mask2, src1, src0, mask3, vec0, vec1);
    DUP2_ARG3(__lsx_vshuf_b, src2, src1, mask2, src2, src1, mask3, vec2, vec3);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst10, vec1, filt1, dst10, dst10);
    DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst21, vec3, filt1, dst21, dst21);
    dst10_r = __lsx_vilvl_h(dst21, dst10);
    dst21_r = __lsx_vilvh_h(dst21, dst10);
    dst22 = __lsx_vreplvei_d(dst21, 1);

    for (loop_cnt = 2; loop_cnt--;) {
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src3, src4, src5, src6);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride_2x, 0,
                  src + src_stride_3x, 0, src7, src8, src9, src10);
        src += src_stride_4x;
        DUP4_ARG2(__lsx_vxori_b, src3, 128, src4, 128, src5, 128, src6, 128, src3,
                  src4, src5, src6);
        DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128, src7,
                  src8, src9, src10);
        DUP2_ARG3(__lsx_vshuf_b, src7, src3, mask2, src7, src3, mask3, vec0, vec1);
        DUP2_ARG3(__lsx_vshuf_b, src8, src4, mask2, src8, src4, mask3, vec2, vec3);
        DUP2_ARG3(__lsx_vshuf_b, src9, src5, mask2, src9, src5, mask3, vec4, vec5);
        DUP2_ARG3(__lsx_vshuf_b, src10, src6, mask2, src10, src6, mask3, vec6, vec7);

        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec0, filt0, dst73, vec1, filt1,
                  dst73, dst73);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec2, filt0, dst84, vec3, filt1,
                  dst84, dst84);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec4, filt0, dst95, vec5, filt1,
                  dst95, dst95);
        DUP2_ARG3(__lsx_vdp2add_h_b, const_vec, vec6, filt0, dst106, vec7, filt1,
                  dst106, dst106);

        DUP2_ARG2(__lsx_vilvl_h, dst73, dst22, dst84, dst73, dst32_r, dst43_r);
        DUP2_ARG2(__lsx_vilvh_h, dst84, dst73, dst95, dst84, dst87_r, dst98_r);
        DUP2_ARG2(__lsx_vilvl_h, dst95, dst84, dst106, dst95, dst54_r, dst65_r);
        dst109_r = __lsx_vilvh_h(dst106, dst95);
        dst22 = __lsx_vreplvei_d(dst73, 1);
        dst76_r = __lsx_vilvl_h(dst22, dst106);

        tmp0 = __lsx_hevc_filt_4tap_w(dst10_r, dst32_r, filt_h0, filt_h1);
        tmp1 = __lsx_hevc_filt_4tap_w(dst21_r, dst43_r, filt_h0, filt_h1);
        tmp2 = __lsx_hevc_filt_4tap_w(dst32_r, dst54_r, filt_h0, filt_h1);
        tmp3 = __lsx_hevc_filt_4tap_w(dst43_r, dst65_r, filt_h0, filt_h1);
        tmp4 = __lsx_hevc_filt_4tap_w(dst54_r, dst76_r, filt_h0, filt_h1);
        tmp5 = __lsx_hevc_filt_4tap_w(dst65_r, dst87_r, filt_h0, filt_h1);
        tmp6 = __lsx_hevc_filt_4tap_w(dst76_r, dst98_r, filt_h0, filt_h1);
        tmp7 = __lsx_hevc_filt_4tap_w(dst87_r, dst109_r, filt_h0, filt_h1);

        DUP4_ARG2(__lsx_vsrai_w, tmp0, 6, tmp1, 6, tmp2, 6, tmp3, 6, tmp0, tmp1, tmp2, tmp3);
        DUP4_ARG2(__lsx_vsrai_w, tmp4, 6, tmp5, 6, tmp6, 6, tmp7, 6, tmp4, tmp5, tmp6, tmp7);
        DUP4_ARG2(__lsx_vpickev_h, tmp1, tmp0, tmp3, tmp2, tmp5, tmp4, tmp7, tmp6,
                  tmp0, tmp1, tmp2, tmp3);

        __lsx_vstelm_d(tmp0, dst, 0, 0);
        __lsx_vstelm_d(tmp0, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(tmp1, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(tmp1, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;
        __lsx_vstelm_d(tmp2, dst, 0, 0);
        __lsx_vstelm_d(tmp2, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(tmp3, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(tmp3, dst + dst_stride_3x, 0, 1);
        dst += dst_stride_4x;

        dst10_r = dst98_r;
        dst21_r = dst109_r;
        dst22 = __lsx_vreplvei_d(dst106, 1);
    }
}

static void hevc_hv_4t_16w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y,
                               int32_t height)
{
    if (4 == height) {
        hevc_hv_4t_8multx4_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, 2);
    } else {
        hevc_hv_4t_8multx4mult_lsx(src, src_stride, dst, dst_stride,
                                   filter_x, filter_y, height, 2);
    }
}

static void hevc_hv_4t_24w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_4t_8multx4mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 3);
}

static void hevc_hv_4t_32w_lsx(uint8_t *src,
                               int32_t src_stride,
                               int16_t *dst,
                               int32_t dst_stride,
                               const int8_t *filter_x,
                               const int8_t *filter_y,
                               int32_t height)
{
    hevc_hv_4t_8multx4mult_lsx(src, src_stride, dst, dst_stride,
                               filter_x, filter_y, height, 4);
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

#define MC(PEL, DIR, WIDTH, TAP, DIR1, FILT_DIR)                          \
void ff_hevc_put_hevc_##PEL##_##DIR##WIDTH##_8_lsx(int16_t *dst,          \
                                                   uint8_t *src,          \
                                                   ptrdiff_t src_stride,  \
                                                   int height,            \
                                                   intptr_t mx,           \
                                                   intptr_t my,           \
                                                   int width)             \
{                                                                         \
    const int8_t *filter = ff_hevc_##PEL##_filters[FILT_DIR - 1];         \
                                                                          \
    hevc_##DIR1##_##TAP##t_##WIDTH##w_lsx(src, src_stride, dst,           \
                                          MAX_PB_SIZE, filter, height);   \
}

MC(qpel, h, 4, 8, hz, mx);
MC(qpel, h, 8, 8, hz, mx);
MC(qpel, h, 12, 8, hz, mx);
MC(qpel, h, 16, 8, hz, mx);
MC(qpel, h, 24, 8, hz, mx);
MC(qpel, h, 32, 8, hz, mx);
MC(qpel, h, 48, 8, hz, mx);
MC(qpel, h, 64, 8, hz, mx);

MC(qpel, v, 4, 8, vt, my);
MC(qpel, v, 8, 8, vt, my);
MC(qpel, v, 12, 8, vt, my);
MC(qpel, v, 16, 8, vt, my);
MC(qpel, v, 24, 8, vt, my);
MC(qpel, v, 32, 8, vt, my);
MC(qpel, v, 48, 8, vt, my);
MC(qpel, v, 64, 8, vt, my);

MC(epel, h, 32, 4, hz, mx);

MC(epel, v, 16, 4, vt, my);
MC(epel, v, 24, 4, vt, my);
MC(epel, v, 32, 4, vt, my);

#undef MC

#define MC_HV(PEL, WIDTH, TAP)                                          \
void ff_hevc_put_hevc_##PEL##_hv##WIDTH##_8_lsx(int16_t *dst,           \
                                                uint8_t *src,           \
                                                ptrdiff_t src_stride,   \
                                                int height,             \
                                                intptr_t mx,            \
                                                intptr_t my,            \
                                                int width)              \
{                                                                       \
    const int8_t *filter_x = ff_hevc_##PEL##_filters[mx - 1];           \
    const int8_t *filter_y = ff_hevc_##PEL##_filters[my - 1];           \
                                                                        \
    hevc_hv_##TAP##t_##WIDTH##w_lsx(src, src_stride, dst, MAX_PB_SIZE,  \
                                          filter_x, filter_y, height);  \
}

MC_HV(qpel, 4, 8);
MC_HV(qpel, 8, 8);
MC_HV(qpel, 12, 8);
MC_HV(qpel, 16, 8);
MC_HV(qpel, 24, 8);
MC_HV(qpel, 32, 8);
MC_HV(qpel, 48, 8);
MC_HV(qpel, 64, 8);

MC_HV(epel, 8, 4);
MC_HV(epel, 12, 4);
MC_HV(epel, 16, 4);
MC_HV(epel, 24, 4);
MC_HV(epel, 32, 4);

#undef MC_HV
