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

static const uint8_t ff_hevc_mask_arr[16 * 2] __attribute__((aligned(0x40))) = {
    /* 8 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
    0, 1, 1, 2, 2, 3, 3, 4, 16, 17, 17, 18, 18, 19, 19, 20
};

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

static void hevc_vt_bi_8t_8w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 const int8_t *filter,
                                 int32_t height)
{
    int32_t loop_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src2_stride_2x = (src2_stride << 1);
    const int32_t src2_stride_4x = (src2_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    const int32_t src2_stride_3x = src2_stride_2x + src2_stride;
    __m128i src0, src1, src2, src3, src4, src5;
    __m128i src6, src7, src8, src9, src10;
    __m128i in0, in1, in2, in3;
    __m128i src10_r, src32_r, src54_r, src76_r, src98_r;
    __m128i src21_r, src43_r, src65_r, src87_r, src109_r;
    __m128i dst0_r, dst1_r, dst2_r, dst3_r;
    __m128i filt0, filt1, filt2, filt3;
    __m128i const_vec;

    src0_ptr -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    LSX_DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6,
                  filt0, filt1, filt2, filt3);

    LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0,
                  src0_ptr + src_stride_2x, 0, src0_ptr + src_stride_3x, 0, src0, src1,
                  src2, src3);
    src0_ptr += src_stride_4x;
    LSX_DUP2_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0, src4, src5);
    src6 = __lsx_vld(src0_ptr + src_stride_2x, 0);
    src0_ptr += src_stride_3x;
    LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0, src1,
                  src2, src3);
    LSX_DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
    src6 =  __lsx_vxori_b(src6, 128);
    LSX_DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
                  src10_r, src32_r, src54_r, src21_r);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src0_ptr, 0, src0_ptr + src_stride, 0,
                      src0_ptr + src_stride_2x, 0, src0_ptr + src_stride_3x, 0,
                      src7, src8, src9, src10);
        src0_ptr += src_stride_4x;
        LSX_DUP4_ARG2(__lsx_vld, src1_ptr, 0, src1_ptr + src2_stride, 0,
                      src1_ptr + src2_stride_2x, 0, src1_ptr + src2_stride_3x, 0,
                      in0, in1, in2, in3);
        src1_ptr += src2_stride_4x;
        LSX_DUP4_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src9, 128, src10, 128, src7,
                      src8, src9, src10);
        LSX_DUP4_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src9, src8, src10, src9,
                      src76_r, src87_r, src98_r, src109_r);

        LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                      filt1, dst0_r, src54_r, filt2, dst0_r, src76_r, filt3, dst0_r,
                      dst0_r, dst0_r, dst0_r);
        LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                      filt1, dst1_r, src65_r, filt2, dst1_r, src87_r, filt3, dst1_r,
                      dst1_r, dst1_r, dst1_r);
        LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src32_r, filt0, dst2_r, src54_r,
                      filt1, dst2_r, src76_r, filt2, dst2_r, src98_r, filt3, dst2_r,
                      dst2_r, dst2_r, dst2_r);
        LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src43_r, filt0, dst3_r, src65_r,
                      filt1, dst3_r, src87_r, filt2, dst3_r, src109_r, filt3, dst3_r,
                      dst3_r, dst3_r, dst3_r);

        __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0_r, dst1_r, dst2_r, dst3_r, 7,
                                &dst0_r, &dst1_r, &dst2_r, &dst3_r);

        LSX_DUP2_ARG2(__lsx_vpickev_b, dst1_r, dst0_r, dst3_r, dst2_r, dst0_r, dst1_r);
        __lsx_vstelm_d(dst0_r, dst, 0, 0);
        __lsx_vstelm_d(dst0_r, dst + dst_stride, 0, 1);
        __lsx_vstelm_d(dst1_r, dst + dst_stride_2x, 0, 0);
        __lsx_vstelm_d(dst1_r, dst + dst_stride_3x, 0, 1);
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

static void hevc_vt_bi_8t_16multx2mult_lsx(uint8_t *src0_ptr,
                                           int32_t src_stride,
                                           int16_t *src1_ptr,
                                           int32_t src2_stride,
                                           uint8_t *dst,
                                           int32_t dst_stride,
                                           const int8_t *filter,
                                           int32_t height, int32_t width)
{
    uint8_t *src0_ptr_tmp;
    int16_t *src1_ptr_tmp;
    uint8_t *dst_tmp;
    uint32_t loop_cnt;
    uint32_t cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t src2_stride_2x = (src2_stride << 1);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i in0, in1, in2, in3;
    __m128i src10_r, src32_r, src54_r, src76_r;
    __m128i src21_r, src43_r, src65_r, src87_r;
    __m128i dst0_r, dst1_r;
    __m128i src10_l, src32_l, src54_l, src76_l;
    __m128i src21_l, src43_l, src65_l, src87_l;
    __m128i dst0_l, dst1_l;
    __m128i filt0, filt1, filt2, filt3;
    __m128i const_vec;

    src0_ptr -= src_stride_3x;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    LSX_DUP4_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filter, 4, filter, 6,
                  filt0, filt1, filt2, filt3);

    for (cnt = (width >> 4); cnt--;) {
        src0_ptr_tmp = src0_ptr;
        src1_ptr_tmp = src1_ptr;
        dst_tmp = dst;

        LSX_DUP4_ARG2(__lsx_vld, src0_ptr_tmp, 0, src0_ptr_tmp + src_stride, 0,
                      src0_ptr_tmp + src_stride_2x, 0, src0_ptr_tmp + src_stride_3x,
                      0, src0, src1, src2, src3);
        src0_ptr_tmp += src_stride_4x;
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr_tmp, 0, src0_ptr_tmp + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src0_ptr_tmp + src_stride_2x, 0);
        src0_ptr_tmp += src_stride_3x;
        LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128,
                      src0, src1, src2, src3);
        LSX_DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);

        LSX_DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src5, src4, src2, src1,
                      src10_r, src32_r, src54_r, src21_r);
        LSX_DUP2_ARG2(__lsx_vilvl_b, src4, src3, src6, src5, src43_r, src65_r);
        LSX_DUP4_ARG2(__lsx_vilvh_b, src1, src0, src3, src2, src5, src4, src2, src1,
                      src10_l, src32_l, src54_l, src21_l);
        LSX_DUP2_ARG2(__lsx_vilvh_b, src4, src3, src6, src5, src43_l, src65_l);

        for (loop_cnt = (height >> 1); loop_cnt--;) {
            LSX_DUP2_ARG2(__lsx_vld, src0_ptr_tmp, 0, src0_ptr_tmp + src_stride, 0,
                          src7, src8);
            src0_ptr_tmp += src_stride_2x;
            LSX_DUP2_ARG2(__lsx_vld, src1_ptr_tmp, 0, src1_ptr_tmp + src2_stride, 0,
                          in0, in1);
            LSX_DUP2_ARG2(__lsx_vld, src1_ptr_tmp, 16, src1_ptr_tmp + src2_stride,
                          16, in2, in3);
            src1_ptr_tmp += src2_stride_2x;
            LSX_DUP2_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src7, src8);

            LSX_DUP2_ARG2(__lsx_vilvl_b, src7, src6, src8, src7, src76_r, src87_r);
            LSX_DUP2_ARG2(__lsx_vilvh_b, src7, src6, src8, src7, src76_l, src87_l);

            LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src10_r, filt0, dst0_r, src32_r,
                          filt1, dst0_r, src54_r, filt2, dst0_r, src76_r, filt3, dst0_r,
                          dst0_r, dst0_r, dst0_r);
            LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src21_r, filt0, dst1_r, src43_r,
                          filt1, dst1_r, src65_r, filt2, dst1_r, src87_r, filt3, dst1_r,
                          dst1_r, dst1_r, dst1_r);
            LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src10_l, filt0, dst0_l, src32_l,
                          filt1, dst0_l, src54_l, filt2, dst0_l, src76_l, filt3, dst0_l,
                          dst0_l, dst0_l, dst0_l);
            LSX_DUP4_ARG3(__lsx_dp2add_h_b, const_vec, src21_l, filt0, dst1_l, src43_l,
                          filt1, dst1_l, src65_l, filt2, dst1_l, src87_l, filt3, dst1_l,
                          dst1_l, dst1_l, dst1_l);

            __lsx_hevc_bi_rnd_clip4(in0, in1, in2, in3, dst0_r, dst1_r, dst0_l, dst1_l,
                                    7, &dst0_r, &dst1_r, &dst0_l, &dst1_l);

            LSX_DUP2_ARG2(__lsx_vpickev_b, dst0_l, dst0_r, dst1_l, dst1_r, dst0_r, dst1_r);

            __lsx_vst(dst0_r, dst_tmp, 0);
            __lsx_vst(dst1_r, dst_tmp + dst_stride, 0);
            dst_tmp += dst_stride_2x;

            src10_r = src32_r;
            src32_r = src54_r;
            src54_r = src76_r;
            src21_r = src43_r;
            src43_r = src65_r;
            src65_r = src87_r;
            src10_l = src32_l;
            src32_l = src54_l;
            src54_l = src76_l;
            src21_l = src43_l;
            src43_l = src65_l;
            src65_l = src87_l;
            src6 = src8;
        }

        src0_ptr += 16;
        src1_ptr += 16;
        dst += 16;
    }
}

static void hevc_vt_bi_8t_16w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter,
                                  int32_t height)
{
    hevc_vt_bi_8t_16multx2mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                   dst, dst_stride, filter, height, 16);
}

static void hevc_vt_bi_8t_24w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter,
                                  int32_t height)
{
    hevc_vt_bi_8t_16multx2mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                   dst, dst_stride, filter, height, 16);
    hevc_vt_bi_8t_8w_lsx(src0_ptr + 16, src_stride, src1_ptr + 16, src2_stride,
                         dst + 16, dst_stride, filter, height);
}

static void hevc_vt_bi_8t_32w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter,
                                  int32_t height)
{
    hevc_vt_bi_8t_16multx2mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                   dst, dst_stride, filter, height, 32);
}

static void hevc_vt_bi_8t_48w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter,
                                  int32_t height)
{
    hevc_vt_bi_8t_16multx2mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                   dst, dst_stride, filter, height, 48);
}

static void hevc_vt_bi_8t_64w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter,
                                  int32_t height)
{
    hevc_vt_bi_8t_16multx2mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                   dst, dst_stride, filter, height, 64);
}

static void hevc_hv_bi_8t_8multx1mult_lsx(uint8_t *src0_ptr,
                                          int32_t src_stride,
                                          int16_t *src1_ptr,
                                          int32_t src2_stride,
                                          uint8_t *dst,
                                          int32_t dst_stride,
                                          const int8_t *filter_x,
                                          const int8_t *filter_y,
                                          int32_t height, int32_t width)
{
    uint32_t loop_cnt;
    uint32_t cnt;
    uint8_t *src0_ptr_tmp;
    int16_t *src1_ptr_tmp;
    uint8_t *dst_tmp;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    __m128i out;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;
    __m128i in0, tmp;
    __m128i filt0, filt1, filt2, filt3;
    __m128i filt_h0, filt_h1, filt_h2, filt_h3;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);
    __m128i mask1, mask2, mask3;
    __m128i filter_vec, const_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m128i dst0_r, dst0_l;
    __m128i dst10_r, dst32_r, dst54_r, dst76_r;
    __m128i dst10_l, dst32_l, dst54_l, dst76_l;

    src0_ptr -= src_stride_3x + 3;
    const_vec = __lsx_vreplgr2vr_h(8192); // 128 << 6

    LSX_DUP4_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filter_x, 4, filter_x, 6,
                  filt0, filt1, filt2, filt3);
    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);

    LSX_DUP4_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filter_vec, 2,
                  filter_vec, 3, filt_h0, filt_h1, filt_h2, filt_h3);

    LSX_DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);

    for (cnt = width >> 3; cnt--;) {
        src0_ptr_tmp = src0_ptr;
        dst_tmp = dst;
        src1_ptr_tmp = src1_ptr;

        LSX_DUP4_ARG2(__lsx_vld, src0_ptr_tmp, 0, src0_ptr_tmp + src_stride, 0,
                      src0_ptr_tmp + src_stride_2x, 0, src0_ptr_tmp + src_stride_3x, 0,
                      src0, src1, src2, src3);
        src0_ptr_tmp += src_stride_4x;
        LSX_DUP2_ARG2(__lsx_vld, src0_ptr_tmp, 0, src0_ptr_tmp + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src0_ptr_tmp + src_stride_2x, 0);
        src0_ptr_tmp += src_stride_3x;
        LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128,
                      src0, src1, src2, src3);
        LSX_DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);

        /* row 0 row 1 row 2 row 3 */
        LSX_DUP4_ARG3(__lsx_vshuf_b, src0, src0, mask0, src0, src0, mask1, src0, src0,
                      mask2, src0, src0, mask3, vec0, vec1, vec2, vec3);
        LSX_DUP4_ARG3(__lsx_vshuf_b, src1, src1, mask0, src1, src1, mask1, src1, src1,
                      mask2, src1, src1, mask3, vec4, vec5, vec6, vec7);
        LSX_DUP4_ARG3(__lsx_vshuf_b, src2, src2, mask0, src2, src2, mask1, src2, src2,
                      mask2, src2, src2, mask3, vec8, vec9, vec10, vec11);
        LSX_DUP4_ARG3(__lsx_vshuf_b, src3, src3, mask0, src3, src3, mask1, src3, src3,
                      mask2, src3, src3, mask3, vec12, vec13, vec14, vec15);
        dst0 = __lsx_hevc_filt_8tap_h(vec0, vec1, vec2, vec3, filt0, filt1, filt2,
                                      filt3);
        dst1 = __lsx_hevc_filt_8tap_h(vec4, vec5, vec6, vec7, filt0, filt1, filt2,
                                      filt3);
        dst2 = __lsx_hevc_filt_8tap_h(vec8, vec9, vec10, vec11, filt0, filt1, filt2,
                                      filt3);
        dst3 = __lsx_hevc_filt_8tap_h(vec12, vec13, vec14, vec15, filt0, filt1,
                                      filt2, filt3);

        LSX_DUP4_ARG3(__lsx_vshuf_b, src4, src4, mask0, src4, src4, mask1, src4, src4,
                      mask2, src4, src4, mask3, vec0, vec1, vec2, vec3);
        LSX_DUP4_ARG3(__lsx_vshuf_b, src5, src5, mask0, src5, src5, mask1, src5, src5,
                      mask2, src5, src5, mask3, vec4, vec5, vec6, vec7);
        LSX_DUP4_ARG3(__lsx_vshuf_b, src6, src6, mask0, src6, src6, mask1, src6, src6,
                      mask2, src6, src6, mask3, vec8, vec9, vec10, vec11);
        dst4 = __lsx_hevc_filt_8tap_h(vec0, vec1, vec2, vec3, filt0, filt1, filt2,
                                      filt3);
        dst5 = __lsx_hevc_filt_8tap_h(vec4, vec5, vec6, vec7, filt0, filt1, filt2,
                                      filt3);
        dst6 = __lsx_hevc_filt_8tap_h(vec8, vec9, vec10, vec11, filt0, filt1, filt2,
                                      filt3);

        for (loop_cnt = height; loop_cnt--;) {
            src7 = __lsx_vld(src0_ptr_tmp, 0);
            src7 = __lsx_vxori_b(src7, 128);
            src0_ptr_tmp += src_stride;

            in0 = __lsx_vld(src1_ptr_tmp, 0);
            src1_ptr_tmp += src2_stride;

            LSX_DUP4_ARG3(__lsx_vshuf_b, src7, src7, mask0, src7, src7, mask1, src7,
                          src7, mask2, src7, src7, mask3, vec0, vec1, vec2, vec3);
            dst7 = __lsx_hevc_filt_8tap_h(vec0, vec1, vec2, vec3, filt0, filt1,
                                          filt2, filt3);
            LSX_DUP4_ARG2(__lsx_vilvl_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                          dst10_r, dst32_r, dst54_r, dst76_r);
            LSX_DUP4_ARG2(__lsx_vilvh_h, dst1, dst0, dst3, dst2, dst5, dst4, dst7, dst6,
                          dst10_l, dst32_l, dst54_l, dst76_l);

            dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst0_l = __lsx_hevc_filt_8tap_w(dst10_l, dst32_l, dst54_l, dst76_l,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst0_r = __lsx_vsrli_w(dst0_r, 6);
            dst0_l = __lsx_vsrli_w(dst0_l, 6);

            tmp = __lsx_vpickev_h(dst0_l, dst0_r);
            LSX_DUP2_ARG2(__lsx_vsadd_h, tmp, in0, tmp, const_vec, tmp, tmp);
            tmp = __lsx_vsrari_h(tmp, 7);
            tmp = __lsx_clamp255_h(tmp);
            out = __lsx_vpickev_b(tmp, tmp);
            __lsx_vstelm_d(out, dst_tmp, 0, 0);
            dst_tmp += dst_stride;

            dst0 = dst1;
            dst1 = dst2;
            dst2 = dst3;
            dst3 = dst4;
            dst4 = dst5;
            dst5 = dst6;
            dst6 = dst7;
        }

        src0_ptr += 8;
        dst += 8;
        src1_ptr += 8;
    }
}

static void hevc_hv_bi_8t_8w_lsx(uint8_t *src0_ptr,
                                 int32_t src_stride,
                                 int16_t *src1_ptr,
                                 int32_t src2_stride,
                                 uint8_t *dst,
                                 int32_t dst_stride,
                                 const int8_t *filter_x,
                                 const int8_t *filter_y,
                                 int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 8);
}

static void hevc_hv_bi_8t_16w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter_x,
                                  const int8_t *filter_y,
                                  int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 16);
}

static void hevc_hv_bi_8t_24w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter_x,
                                  const int8_t *filter_y,
                                  int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 24);
}

static void hevc_hv_bi_8t_32w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter_x,
                                  const int8_t *filter_y,
                                  int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 32);
}

static void hevc_hv_bi_8t_48w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter_x,
                                  const int8_t *filter_y,
                                  int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 48);
}

static void hevc_hv_bi_8t_64w_lsx(uint8_t *src0_ptr,
                                  int32_t src_stride,
                                  int16_t *src1_ptr,
                                  int32_t src2_stride,
                                  uint8_t *dst,
                                  int32_t dst_stride,
                                  const int8_t *filter_x,
                                  const int8_t *filter_y,
                                  int32_t height)
{
    hevc_hv_bi_8t_8multx1mult_lsx(src0_ptr, src_stride, src1_ptr, src2_stride,
                                  dst, dst_stride, filter_x, filter_y,
                                  height, 64);
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

#define BI_MC(PEL, DIR, WIDTH, TAP, DIR1, FILT_DIR)                          \
void ff_hevc_put_hevc_bi_##PEL##_##DIR##WIDTH##_8_lsx(uint8_t *dst,          \
                                                      ptrdiff_t dst_stride,  \
                                                      uint8_t *src,          \
                                                      ptrdiff_t src_stride,  \
                                                      int16_t *src_16bit,    \
                                                      int height,            \
                                                      intptr_t mx,           \
                                                      intptr_t my,           \
                                                      int width)             \
{                                                                            \
    const int8_t *filter = ff_hevc_##PEL##_filters[FILT_DIR - 1];            \
                                                                             \
    hevc_##DIR1##_bi_##TAP##t_##WIDTH##w_lsx(src, src_stride, src_16bit,     \
                                             MAX_PB_SIZE, dst, dst_stride,   \
                                             filter, height);                \
}

BI_MC(qpel, v, 8, 8, vt, my);
BI_MC(qpel, v, 16, 8, vt, my);
BI_MC(qpel, v, 24, 8, vt, my);
BI_MC(qpel, v, 32, 8, vt, my);
BI_MC(qpel, v, 48, 8, vt, my);
BI_MC(qpel, v, 64, 8, vt, my);

#undef BI_MC

#define BI_MC_HV(PEL, WIDTH, TAP)                                         \
void ff_hevc_put_hevc_bi_##PEL##_hv##WIDTH##_8_lsx(uint8_t *dst,          \
                                                   ptrdiff_t dst_stride,  \
                                                   uint8_t *src,          \
                                                   ptrdiff_t src_stride,  \
                                                   int16_t *src_16bit,    \
                                                   int height,            \
                                                   intptr_t mx,           \
                                                   intptr_t my,           \
                                                   int width)             \
{                                                                         \
    const int8_t *filter_x = ff_hevc_##PEL##_filters[mx - 1];             \
    const int8_t *filter_y = ff_hevc_##PEL##_filters[my - 1];             \
                                                                          \
    hevc_hv_bi_##TAP##t_##WIDTH##w_lsx(src, src_stride, src_16bit,        \
                                       MAX_PB_SIZE, dst, dst_stride,      \
                                       filter_x, filter_y, height);       \
}

BI_MC_HV(qpel, 8, 8);
BI_MC_HV(qpel, 16, 8);
BI_MC_HV(qpel, 24, 8);
BI_MC_HV(qpel, 32, 8);
BI_MC_HV(qpel, 48, 8);
BI_MC_HV(qpel, 64, 8);

#undef BI_MC_HV
