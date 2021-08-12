/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Hecai Yuan <yuanhecai@loongson.cn>
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
#include "libavcodec/vp8dsp.h"
#include "libavutil/loongarch/generic_macros_lsx.h"
#include "vp8dsp_loongarch.h"

static const uint8_t mc_filt_mask_arr[16 * 3] = {
    /* 8 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
    /* 4 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 16, 17, 17, 18, 18, 19, 19, 20,
    /* 4 width cases */
    8, 9, 9, 10, 10, 11, 11, 12, 24, 25, 25, 26, 26, 27, 27, 28
};

static const int8_t subpel_filters_lsx[7][8] = {
    {-6, 123, 12, -1, 0, 0, 0, 0},
    {2, -11, 108, 36, -8, 1, 0, 0},     /* New 1/4 pel 6 tap filter */
    {-9, 93, 50, -6, 0, 0, 0, 0},
    {3, -16, 77, 77, -16, 3, 0, 0},     /* New 1/2 pel 6 tap filter */
    {-6, 50, 93, -9, 0, 0, 0, 0},
    {1, -8, 36, 108, -11, 2, 0, 0},     /* New 1/4 pel 6 tap filter */
    {-1, 12, 123, -6, 0, 0, 0, 0},
};

#define DPADD_SH3_SH(in0, in1, in2, coeff0, coeff1, coeff2)         \
( {                                                                 \
    __m128i out0_m;                                                 \
                                                                    \
    out0_m = __lsx_dp2_h_b(in0, coeff0);                            \
    out0_m = __lsx_dp2add_h_b(out0_m, in1, coeff1);                 \
    out0_m = __lsx_dp2add_h_b(out0_m, in2, coeff2);                 \
                                                                    \
    out0_m;                                                         \
} )

#define VSHF_B3_SB(in0, in1, in2, in3, in4, in5, mask0, mask1, mask2,  \
                out0, out1, out2)                                      \
{                                                                      \
    LSX_DUP2_ARG3(__lsx_vshuf_b, in1, in0, mask0, in3, in2, mask1,     \
                  out0, out1);                                         \
    out2 = __lsx_vshuf_b(in5, in4, mask2);                             \
}

#define HORIZ_6TAP_FILT(src0, src1, mask0, mask1, mask2,                 \
                        filt_h0, filt_h1, filt_h2)                       \
( {                                                                      \
    __m128i vec0_m, vec1_m, vec2_m;                                      \
    __m128i hz_out_m;                                                    \
                                                                         \
    VSHF_B3_SB(src0, src1, src0, src1, src0, src1, mask0, mask1, mask2,  \
               vec0_m, vec1_m, vec2_m);                                  \
    hz_out_m = DPADD_SH3_SH(vec0_m, vec1_m, vec2_m,                      \
                            filt_h0, filt_h1, filt_h2);                  \
                                                                         \
    hz_out_m = __lsx_vsrari_h(hz_out_m, 7);                              \
    hz_out_m = __lsx_vsat_h(hz_out_m, 7);                                \
                                                                         \
    hz_out_m;                                                            \
} )

void ff_put_vp8_epel16_v6_lsx(uint8_t *dst, ptrdiff_t dst_stride,
                              uint8_t *src, ptrdiff_t src_stride,
                              int height, int mx, int my)
{
    uint32_t loop_cnt;
    const int8_t *filter = subpel_filters_lsx[my - 1];
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i src10_l, src32_l, src54_l, src76_l, src21_l, src43_l, src65_l, src87_l;
    __m128i src10_h, src32_h, src54_h, src76_h, src21_h, src43_h, src65_h, src87_h;
    __m128i filt0, filt1, filt2;
    __m128i tmp0, tmp1, tmp2, tmp3;

    ptrdiff_t src_stride2 = src_stride << 1;
    ptrdiff_t src_stride3 = src_stride2 + src_stride;
    ptrdiff_t src_stride4 = src_stride2 << 1;

    LSX_DUP2_ARG2(__lsx_vldrepl_h, filter, 0, filter, 2, filt0, filt1);
    filt2 = __lsx_vldrepl_h(filter, 4);

    LSX_DUP4_ARG2(__lsx_vld, src - src_stride2, 0, src - src_stride, 0, src, 0,
                  src + src_stride, 0, src0, src1, src2, src3);
    src4 = __lsx_vld(src + src_stride2, 0);
    src += src_stride3;

    LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                  src1, src2, src3);
    src4 = __lsx_vxori_b(src4, 128);

    LSX_DUP4_ARG2(__lsx_vilvl_b, src1, src0, src3, src2, src4, src3, src2, src1,
                  src10_l, src32_l, src43_l, src21_l);
    LSX_DUP4_ARG2(__lsx_vilvh_b, src1, src0, src3, src2, src4, src3, src2, src1,
                  src10_h, src32_h, src43_h, src21_h);

    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride2, 0,
                      src + src_stride3, 0, src5, src6, src7, src8);
        src += src_stride4;
        LSX_DUP4_ARG2(__lsx_vxori_b, src5, 128, src6, 128, src7, 128, src8, 128,
                      src5, src6, src7, src8);

        LSX_DUP4_ARG2(__lsx_vilvl_b, src5, src4, src6, src5, src7, src6, src8, src7,
                      src54_l, src65_l, src76_l, src87_l);
        LSX_DUP4_ARG2(__lsx_vilvh_b, src5, src4, src6, src5, src7, src6, src8, src7,
                      src54_h, src65_h, src76_h, src87_h);

        tmp0 = DPADD_SH3_SH(src10_l, src32_l, src54_l, filt0, filt1, filt2);
        tmp1 = DPADD_SH3_SH(src21_l, src43_l, src65_l, filt0, filt1, filt2);
        tmp2 = DPADD_SH3_SH(src10_h, src32_h, src54_h, filt0, filt1, filt2);
        tmp3 = DPADD_SH3_SH(src21_h, src43_h, src65_h, filt0, filt1, filt2);

        LSX_DUP2_ARG3(__lsx_vssrarni_b_h, tmp2, tmp0, 7, tmp3, tmp1, 7, tmp0, tmp1);
        LSX_DUP2_ARG2(__lsx_vxori_b, tmp0, 128, tmp1, 128, tmp0, tmp1);
        __lsx_vst(tmp0, dst, 0);
        dst += dst_stride;
        __lsx_vst(tmp1, dst, 0);
        dst += dst_stride;

        tmp0 = DPADD_SH3_SH(src32_l, src54_l, src76_l, filt0, filt1, filt2);
        tmp1 = DPADD_SH3_SH(src43_l, src65_l, src87_l, filt0, filt1, filt2);
        tmp2 = DPADD_SH3_SH(src32_h, src54_h, src76_h, filt0, filt1, filt2);
        tmp3 = DPADD_SH3_SH(src43_h, src65_h, src87_h, filt0, filt1, filt2);

        LSX_DUP2_ARG3(__lsx_vssrarni_b_h, tmp2, tmp0, 7, tmp3, tmp1, 7, tmp0, tmp1);
        LSX_DUP2_ARG2(__lsx_vxori_b, tmp0, 128, tmp1, 128, tmp0, tmp1);
        __lsx_vst(tmp0, dst, 0);
        dst += dst_stride;
        __lsx_vst(tmp1, dst, 0);
        dst += dst_stride;

        src10_l = src54_l;
        src32_l = src76_l;
        src21_l = src65_l;
        src43_l = src87_l;
        src10_h = src54_h;
        src32_h = src76_h;
        src21_h = src65_h;
        src43_h = src87_h;
        src4 = src8;
    }
}

void ff_put_vp8_epel8_h6v6_lsx(uint8_t *dst, ptrdiff_t dst_stride,
                               uint8_t *src, ptrdiff_t src_stride,
                               int height, int mx, int my)
{
    uint32_t loop_cnt;
    const int8_t *filter_horiz = subpel_filters_lsx[mx - 1];
    const int8_t *filter_vert = subpel_filters_lsx[my - 1];
    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i filt_hz0, filt_hz1, filt_hz2;
    __m128i mask0, mask1, mask2, filt_vt0, filt_vt1, filt_vt2;
    __m128i hz_out0, hz_out1, hz_out2, hz_out3, hz_out4, hz_out5, hz_out6;
    __m128i hz_out7, hz_out8, out0, out1, out2, out3, out4, out5, out6, out7;
    __m128i tmp0, tmp1, tmp2, tmp3;

    ptrdiff_t src_stride2 = src_stride << 1;
    ptrdiff_t src_stride3 = src_stride2 + src_stride;
    ptrdiff_t src_stride4 = src_stride2 << 1;

    mask0 = __lsx_vld(mc_filt_mask_arr, 0);
    src -= (2 + src_stride2);

    /* rearranging filter */
    LSX_DUP2_ARG2(__lsx_vldrepl_h, filter_horiz, 0, filter_horiz, 2,
                  filt_hz0, filt_hz1);
    filt_hz2 = __lsx_vldrepl_h(filter_horiz, 4);

    LSX_DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);

    LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride2, 0,
                  src + src_stride3, 0, src0, src1, src2, src3);
    src += src_stride4;
    src4 = __lsx_vld(src, 0);
    src +=  src_stride;

    LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128,
                  src0 ,src1, src2, src3);
    src4 = __lsx_vxori_b(src4, 128);

    hz_out0 = HORIZ_6TAP_FILT(src0, src0, mask0, mask1, mask2, filt_hz0,
                              filt_hz1, filt_hz2);
    hz_out1 = HORIZ_6TAP_FILT(src1, src1, mask0, mask1, mask2, filt_hz0,
                              filt_hz1, filt_hz2);
    hz_out2 = HORIZ_6TAP_FILT(src2, src2, mask0, mask1, mask2, filt_hz0,
                              filt_hz1, filt_hz2);
    hz_out3 = HORIZ_6TAP_FILT(src3, src3, mask0, mask1, mask2, filt_hz0,
                              filt_hz1, filt_hz2);
    hz_out4 = HORIZ_6TAP_FILT(src4, src4, mask0, mask1, mask2, filt_hz0,
                              filt_hz1, filt_hz2);

    LSX_DUP2_ARG2(__lsx_vldrepl_h, filter_vert, 0, filter_vert, 2,
                  filt_vt0, filt_vt1);
    filt_vt2 = __lsx_vldrepl_h(filter_vert, 4);

    LSX_DUP2_ARG2(__lsx_vpackev_b, hz_out1, hz_out0, hz_out3, hz_out2, out0, out1);
    LSX_DUP2_ARG2(__lsx_vpackev_b, hz_out2, hz_out1, hz_out4, hz_out3, out3, out4);
    for (loop_cnt = (height >> 2); loop_cnt--;) {
        LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride2, 0,
                      src + src_stride3, 0, src5, src6, src7, src8);
        src += src_stride4;

        LSX_DUP4_ARG2(__lsx_vxori_b, src5, 128, src6, 128, src7, 128, src8, 128,
                      src5, src6, src7, src8);

        hz_out5 = HORIZ_6TAP_FILT(src5, src5, mask0, mask1, mask2, filt_hz0,
                                  filt_hz1, filt_hz2);
        out2 = __lsx_vpackev_b(hz_out5, hz_out4);
        tmp0 = DPADD_SH3_SH(out0, out1, out2,filt_vt0, filt_vt1, filt_vt2);

        hz_out6 = HORIZ_6TAP_FILT(src6, src6, mask0, mask1, mask2, filt_hz0,
                                  filt_hz1, filt_hz2);
        out5 = __lsx_vpackev_b(hz_out6, hz_out5);
        tmp1 = DPADD_SH3_SH(out3, out4, out5, filt_vt0, filt_vt1, filt_vt2);

        hz_out7 = HORIZ_6TAP_FILT(src7, src7, mask0, mask1, mask2, filt_hz0,
                                  filt_hz1, filt_hz2);

        out7 = __lsx_vpackev_b(hz_out7, hz_out6);
        tmp2 = DPADD_SH3_SH(out1, out2, out7, filt_vt0, filt_vt1, filt_vt2);

        hz_out8 = HORIZ_6TAP_FILT(src8, src8, mask0, mask1, mask2, filt_hz0,
                                  filt_hz1, filt_hz2);
        out6 = __lsx_vpackev_b(hz_out8, hz_out7);
        tmp3 = DPADD_SH3_SH(out4, out5, out6, filt_vt0, filt_vt1, filt_vt2);

        LSX_DUP2_ARG3(__lsx_vssrarni_b_h, tmp1, tmp0, 7, tmp3, tmp2, 7, tmp0, tmp1);
        LSX_DUP2_ARG2(__lsx_vxori_b, tmp0, 128, tmp1, 128, tmp0, tmp1);
        __lsx_vstelm_d(tmp0, dst, 0, 0);

        dst += dst_stride;
        __lsx_vstelm_d(tmp0, dst, 0, 1);
        dst += dst_stride;
        __lsx_vstelm_d(tmp1, dst, 0, 0);
        dst += dst_stride;
        __lsx_vstelm_d(tmp1, dst, 0, 1);
        dst += dst_stride;

        hz_out4 = hz_out8;
        out0 = out2;
        out1 = out7;
        out3 = out5;
        out4 = out6;
    }
}

void ff_put_vp8_epel16_h6v6_lsx(uint8_t *dst, ptrdiff_t dst_stride,
                                uint8_t *src, ptrdiff_t src_stride,
                                int height, int mx, int my)
{
    int32_t multiple8_cnt;

    for (multiple8_cnt = 2; multiple8_cnt--;) {
        ff_put_vp8_epel8_h6v6_lsx(dst, dst_stride, src, src_stride, height, mx, my);
        src += 8;
        dst += 8;
    }
}

void ff_put_vp8_pixels16_lsx(uint8_t *dst, ptrdiff_t dst_stride,
                             uint8_t *src, ptrdiff_t src_stride,
                             int height, int mx, int my)
{
    int32_t width = 16;
    int32_t cnt, loop_cnt;
    uint8_t *src_tmp, *dst_tmp;
    __m128i src0, src1, src2, src3, src4, src5, src6, src7;

    ptrdiff_t src_stride2 = src_stride << 1;
    ptrdiff_t src_stride3 = src_stride2 + src_stride;
    ptrdiff_t src_stride4 = src_stride2 << 1;

    ptrdiff_t dst_stride2 = dst_stride << 1;
    ptrdiff_t dst_stride3 = dst_stride2 + dst_stride;
    ptrdiff_t dst_stride4 = dst_stride2 << 1;

    if (0 == height % 8) {
        for (cnt = (width >> 4); cnt--;) {
            src_tmp = src;
            dst_tmp = dst;
            for (loop_cnt = (height >> 3); loop_cnt--;) {
                LSX_DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                              src_tmp + src_stride2, 0, src_tmp + src_stride3, 0,
                              src4, src5, src6, src7);
                src_tmp += src_stride4;

                __lsx_vst(src4, dst_tmp,               0);
                __lsx_vst(src5, dst_tmp + dst_stride,  0);
                __lsx_vst(src6, dst_tmp + dst_stride2, 0);
                __lsx_vst(src7, dst_tmp + dst_stride3, 0);
                dst_tmp += dst_stride4;

                LSX_DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                              src_tmp + src_stride2, 0, src_tmp + src_stride3, 0,
                              src4, src5, src6, src7);
                src_tmp += src_stride4;

                __lsx_vst(src4, dst_tmp,               0);
                __lsx_vst(src5, dst_tmp + dst_stride,  0);
                __lsx_vst(src6, dst_tmp + dst_stride2, 0);
                __lsx_vst(src7, dst_tmp + dst_stride3, 0);
                dst_tmp += dst_stride4;
            }
            src += 16;
            dst += 16;
        }
    } else if (0 == height % 4) {
        for (cnt = (height >> 2); cnt--;) {
            LSX_DUP4_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src + src_stride2, 0,
                          src + src_stride3, 0, src0, src1, src2, src3);
            src += 4 * src_stride4;

            __lsx_vst(src0, dst,               0);
            __lsx_vst(src1, dst + dst_stride,  0);
            __lsx_vst(src2, dst + dst_stride2, 0);
            __lsx_vst(src3, dst + dst_stride3, 0);
            dst += dst_stride4;
       }
    }
}
