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
#include "libavcodec/loongarch/hevc_macros_lsx.h"

static av_always_inline
void __lsx_hevc_bi_rnd_clip2(__m128i in0, __m128i in1, __m128i vec0, __m128i vec1,
                             int rnd_val, __m128i *out0, __m128i *out1)
{
    *out0 = __lsx_vsadd_h(vec0, in0);
    *out1 = __lsx_vsadd_h(vec1, in1);
    *out0 = __lsx_vsrari_h(*out0, rnd_val);
    *out1 = __lsx_vsrari_h(*out1, rnd_val);
    *out0 = __lsx_clamp255_h(*out0);
    *out1 = __lsx_clamp255_h(*out1);
}

static av_always_inline
void __lsx_hevc_bi_rnd_clip4(__m128i in0, __m128i in1, __m128i in2, __m128i in3,
                             __m128i vec0, __m128i vec1, __m128i vec2,
                             __m128i vec3, int rnd_val, __m128i *out0,
                             __m128i *out1, __m128i *out2, __m128i *out3)
{
    __lsx_hevc_bi_rnd_clip2(in0, in1, vec0, vec1, rnd_val, out0, out1);
    __lsx_hevc_bi_rnd_clip2(in2, in3, vec2, vec3, rnd_val, out2, out3);
}


static void hevc_bi_copy_12w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src2_stride_2x = (src2_stride << 1);
    const int32_t src2_stride_4x = (src2_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    const int32_t src2_stride_3x = src2_stride_2x + src2_stride;
    __m128i out0, out1, out2;
    __m128i src0, src1, src2, src3;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5;

    for (loop_cnt = 4; loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0,
                      src0_ptr + src_stride_2x, 0, src0_ptr + src_stride_3x, 0,
                      src0, src1, src2, src3);
        src0_ptr += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr + src2_stride, 0,
                      src1_ptr + src2_stride_2x, 0, src1_ptr + src2_stride_3x, 0,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 16, src1_ptr + src2_stride, 16,
                      src1_ptr + src2_stride_2x, 16, src1_ptr + src2_stride_3x, 16,
                      in4, in5, in6, in7);

        src1_ptr += src2_stride_4x;
        LSX_DUP2_ARG2(__lsx_vilvl_d, in5, in4, in7, in6, in4, in5);
        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0, dst1, dst2, dst3)
        LSX_DUP2_ARG2(__lsx_vilvh_w, src1, src0, src3, src2, src0, src1);
        LSX_DUP2_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, dst4, dst5)
        __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0, dst1, dst2, dst3,
                                7, &dst0, &dst1, &dst2, &dst3);
        __lsx_hevc_bi_rnd_clip2(in4, in5, dst4, dst5, 7, &dst4, &dst5);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst1, dst0, dst3, dst2, out0, out1);
        out2 = __lsx_vpickev_b(dst5, dst4);
        __lsx_vstelm_d(out0, dst, 0, 0);
        __lsx_vstelm_d(out0, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(out1, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(out1, dst + dst_stride_3x, 0, 1);
        __lsx_vstelm_w(out2, dst, 8, 0);
        __lsx_vstelm_w(out2, dst + dst_stride, 8, 1);
        __lsx_vstelm_w(out2, dst + dst_stride_2x, 8, 2);
        __lsx_vstelm_w(out2, dst + dst_stride_3x, 8, 3);
        dst += dst_stride_4x;
    }
}

static void hevc_bi_copy_16w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src2_stride_2x = (src2_stride << 1);
    const int32_t src2_stride_4x = (src2_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    const int32_t src2_stride_3x = src2_stride_2x + src2_stride;
    __m128i out0, out1, out2, out3;
    __m128i src0, src1, src2, src3;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r, dst0_l, dst1_l, dst2_l, dst3_l;
    __m128i zero = {0};

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0,
                      src0_ptr + src_stride_2x, 0, src0_ptr + src_stride_3x, 0,
                      src0, src1, src2, src3);
        src0_ptr += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr + src2_stride, 0,
                      src1_ptr + src2_stride_2x, 0, src1_ptr + src2_stride_3x, 0,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 16, src1_ptr + src2_stride, 16,
                      src1_ptr + src2_stride_2x, 16, src1_ptr + src2_stride_3x, 16,
                      in4, in5, in6, in7);
        src1_ptr += src2_stride_4x;
        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0_r, dst1_r, dst2_r, dst3_r)
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      dst0_l, dst1_l, dst2_l, dst3_l);
        LSX_DUP4_ARG2(__lsx_vslli_h, dst0_l, 6, dst1_l, 6, dst2_l, 6, dst3_l, 6, dst0_l,
                      dst1_l, dst2_l, dst3_l);

        __lsx_hevc_bi_rnd_clip4(in0, in1, in4, in5, dst0_r, dst1_r, dst0_l,
                                dst1_l, 7, &dst0_r, &dst1_r, &dst0_l, &dst1_l);
        __lsx_hevc_bi_rnd_clip4(in2, in3, in6, in7, dst2_r, dst3_r, dst2_l,
                                dst3_l, 7, &dst2_r, &dst3_r, &dst2_l, &dst3_l);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst0_l, dst0_r, dst1_l, dst1_r, out0, out1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst2_l, dst2_r, dst3_l, dst3_r, out2, out3);
        __lsx_vst(out0, dst, 0);
        __lsx_vst(out1, dst + dst_stride, 0);
        __lsx_vst(out2, dst + dst_stride_2x, 0);
        __lsx_vst(out3, dst + dst_stride_3x, 0);
        dst += dst_stride_4x;
    }
}

static void hevc_bi_copy_24w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src2_stride_2x = (src2_stride << 1);
    const int32_t src2_stride_4x = (src2_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    const int32_t src2_stride_3x = src2_stride_2x + src2_stride;
    __m128i out0, out1, out2, out3, out4, out5;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, zero = {0};
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, dst11;

    for (loop_cnt = 8; loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0,
                      src0_ptr + src_stride_2x, 0, src0_ptr + src_stride_3x, 0,
                      src0, src1, src4, src5);
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 16, src0_ptr + src_stride, 16,
                      src0_ptr + src_stride_2x, 16, src0_ptr + src_stride_3x, 16,
                      src2, src3, src6, src7);
        src0_ptr += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr + src2_stride, 0,
                      src1_ptr + src2_stride_2x, 0, src1_ptr + src2_stride_3x, 0,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 16, src1_ptr + src2_stride, 16,
                      src1_ptr + src2_stride_2x, 16, src1_ptr + src2_stride_3x, 16,
                      in4, in5, in6, in7);
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 32, src1_ptr + src2_stride, 32,
                      src1_ptr + src2_stride_2x, 32, src1_ptr + src2_stride_3x, 32,
                      in8, in9, in10, in11);
        src1_ptr += src2_stride_4x;

        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0, dst2, dst4, dst5)

        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src4, zero, src5,
                      dst1, dst3, dst7, dst9);
        LSX_DUP4_ARG2(__lsx_vslli_h, dst1, 6, dst3, 6, dst7, 6, dst9, 6,
                      dst1, dst3, dst7, dst9);

        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src4, 6, src5, 6, src6, 6, src7, 6,
                      dst6, dst8, dst10, dst11)

        __lsx_hevc_bi_rnd_clip4(in0, in4, in1, in5, dst0, dst1, dst2, dst3, 7,
                                &dst0, &dst1, &dst2, &dst3);
        __lsx_hevc_bi_rnd_clip4(in8, in9, in2, in6, dst4, dst5, dst6, dst7, 7,
                                &dst4, &dst5, &dst6, &dst7);
        __lsx_hevc_bi_rnd_clip4(in3, in7, in10, in11, dst8, dst9, dst10, dst11, 7,
                                &dst8, &dst9, &dst10, &dst11);
        LSX_DUP4_ARG2(__lsx_vpickev_b, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                      out0, out1, out2, out3);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst9, dst8, dst11, dst10, out4, out5);
        __lsx_vst(out0, dst, 0);
        __lsx_vstelm_d(out2, dst, 16, 0);
        __lsx_vst(out1, dst + dst_stride, 0);
        __lsx_vstelm_d(out2, dst + dst_stride, 16, 1);
        __lsx_vst(out3, dst + dst_stride_2x, 0);
        __lsx_vstelm_d(out5, dst + dst_stride_2x, 16, 0);
        __lsx_vst(out4, dst + dst_stride_3x, 0);
        __lsx_vstelm_d(out5, dst + dst_stride_3x, 16, 1);
        dst += dst_stride_4x;
    }
}

static void hevc_bi_copy_32w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    __m128i out0, out1, out2, out3;
    __m128i src0, src1, src2, src3;
    __m128i zero = {0};
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;

    for (loop_cnt = (height >> 1); loop_cnt--;) {
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr, 16, src0, src1);
        src0_ptr += src_stride;
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr, 16, src2, src3);
        src0_ptr += src_stride;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr, 16, src1_ptr, 32, src1_ptr, 48,
                      in0, in1, in2, in3);
        src1_ptr += src2_stride;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr, 16, src1_ptr, 32, src1_ptr, 48,
                      in4, in5, in6, in7);
        src1_ptr += src2_stride;

        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0, dst2, dst4, dst6)
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      dst1, dst3, dst5, dst7);
        LSX_DUP4_ARG2(__lsx_vslli_h, dst1, 6, dst3, 6, dst5, 6, dst7, 6, dst1, dst3,
                      dst5, dst7);
        __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0, dst1, dst2, dst3,
                                7, &dst0, &dst1, &dst2, &dst3);
        __lsx_hevc_bi_rnd_clip4(in4, in5, in6, in7, dst4, dst5, dst6, dst7,
                                7, &dst4, &dst5, &dst6, &dst7);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst1, dst0, dst3, dst2, out0, out1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst5, dst4, dst7, dst6, out2, out3);
        __lsx_vst(out0, dst, 0);
        __lsx_vst(out1, dst, 16);
        dst += dst_stride;
        __lsx_vst(out2, dst, 0);
        __lsx_vst(out3, dst, 16);
        dst += dst_stride;
    }
}

static void hevc_bi_copy_48w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    __m128i out0, out1, out2, out3, out4, out5;
    __m128i src0, src1, src2, src3, src4, src5;
    __m128i zero = {0};
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, dst11;

    for (loop_cnt = (height >> 1); loop_cnt--;) {
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr, 16, src0, src1);
        src2 = __lsx_vld(src0_ptr, 32);
        src0_ptr += src_stride;
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr, 16, src3, src4);
        src5 = __lsx_vld(src0_ptr, 32);
        src0_ptr += src_stride;

        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr, 16, src1_ptr, 32, src1_ptr, 48,
                      in0, in1, in2, in3);
        LSX_DUP2_ARG2(__lsx_vld, src1_ptr, 64, src1_ptr, 80, in4, in5);
        src1_ptr += src2_stride;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr, 16, src1_ptr, 32, src1_ptr, 48,
                      in6, in7, in8, in9);
        LSX_DUP2_ARG2(__lsx_vld, src1_ptr, 64, src1_ptr, 80, in10, in11);
        src1_ptr += src2_stride;

        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0, dst2, dst4, dst6);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      dst1, dst3, dst5, dst7);
        LSX_DUP4_ARG2(__lsx_vslli_h, dst1, 6, dst3, 6, dst5, 6, dst7, 6, dst1, dst3,
                      dst5, dst7);
        LSX_DUP2_ARG2(__lsx_vsllwil_hu_bu, src4, 6, src5, 6, dst8, dst10);
        LSX_DUP2_ARG2(__lsx_vilvh_b, zero, src4, zero, src5, dst9, dst11);
        LSX_DUP2_ARG2(__lsx_vslli_h, dst9, 6, dst11, 6, dst9, dst11);


        __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0, dst1, dst2, dst3, 7,
                                &dst0, &dst1, &dst2, &dst3);
        __lsx_hevc_bi_rnd_clip4(in4, in5, in6, in7, dst4, dst5, dst6, dst7, 7,
                                &dst4, &dst5, &dst6, &dst7);
        __lsx_hevc_bi_rnd_clip4(in8, in9, in10, in11, dst8, dst9, dst10, dst11, 7,
                                &dst8, &dst9, &dst10, &dst11);
        LSX_DUP4_ARG2(__lsx_vpickev_b, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                      out0, out1, out2, out3);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst9, dst8, dst11, dst10, out4, out5);
        __lsx_vst(out0, dst, 0);
        __lsx_vst(out1, dst, 16);
        __lsx_vst(out2, dst, 32);
        dst += dst_stride;
        __lsx_vst(out3, dst, 0);
        __lsx_vst(out4, dst, 16);
        __lsx_vst(out5, dst, 32);
        dst += dst_stride;
    }
}

static void hevc_bi_copy_64w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 int32_t height)
{
    uint32_t loop_cnt;
    __m128i out0, out1, out2, out3;
    __m128i src0, src1, src2, src3;
    __m128i zero = {0};
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i in0, in1, in2, in3, in4, in5, in6, in7;

    for (loop_cnt = height; loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr, 16, src0_ptr, 32, src0_ptr, 48,
                      src0, src1, src2, src3);
        src0_ptr += src_stride;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr, 16, src1_ptr, 32, src1_ptr, 48,
                      in0, in1, in2, in3);
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 64, src1_ptr, 80, src1_ptr, 96, src1_ptr, 112,
                      in4, in5, in6, in7);
        src1_ptr += src2_stride;

        LSX_DUP4_ARG2(__lsx_vsllwil_hu_bu, src0, 6, src1, 6, src2, 6, src3, 6,
                      dst0, dst2, dst4, dst6);
        LSX_DUP4_ARG2(__lsx_vilvh_b, zero, src0, zero, src1, zero, src2, zero, src3,
                      dst1, dst3, dst5, dst7);
        LSX_DUP4_ARG2(__lsx_vslli_h, dst1, 6, dst3, 6, dst5, 6, dst7, 6, dst1, dst3,
                      dst5, dst7);

        __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0, dst1, dst2, dst3, 7,
                                &dst0, &dst1, &dst2, &dst3);
        __lsx_hevc_bi_rnd_clip4(in4, in5, in6, in7, dst4, dst5, dst6, dst7, 7,
                                &dst4, &dst5, &dst6, &dst7);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst1, dst0, dst3, dst2, out0, out1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, dst5, dst4, dst7, dst6, out2, out3);

        __lsx_vst(out0, dst, 0);
        __lsx_vst(out1, dst, 16);
        __lsx_vst(out2, dst, 32);
        __lsx_vst(out3, dst, 48);
        dst += dst_stride;
    }
}

#define BI_MC_COPY(WIDTH)                                                 \
void ff_hevc_put_hevc_bi_pel_pixels##WIDTH##_8_lsx(uint8_t *dst,          \
                                                   ptrdiff_t dst_stride,  \
                                                   uint8_t *src,          \
                                                   ptrdiff_t src_stride,  \
                                                   int16_t *src_16bit,    \
                                                   int height,            \
                                                   intptr_t mx,           \
                                                   intptr_t my,           \
                                                   int width)             \
{                                                                         \
    hevc_bi_copy_##WIDTH##w_lsx(src, src_stride, src_16bit, MAX_PB_SIZE,  \
                                dst, dst_stride, height);                 \
}

BI_MC_COPY(12);
BI_MC_COPY(16);
BI_MC_COPY(24);
BI_MC_COPY(32);
BI_MC_COPY(48);
BI_MC_COPY(64);

#undef BI_MC_COPY
