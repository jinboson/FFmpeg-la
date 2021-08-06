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
#include "hevcdsp_lsx.h"
#include "hevc_macros_lsx.h"

static const uint8_t ff_hevc_mask_arr[16 * 2] __attribute__((aligned(0x40))) = {
    /* 8 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
    /* 4 width cases */
    0, 1, 1, 2, 2, 3, 3, 4, 16, 17, 17, 18, 18, 19, 19, 20
};

static av_always_inline
void hevc_hv_uniwgt_8t_8multx2mult_lsx(uint8_t *src,
                                       int32_t src_stride,
                                       uint8_t *dst,
                                       int32_t dst_stride,
                                       const int8_t *filter_x,
                                       const int8_t *filter_y,
                                       int32_t height,
                                       int32_t weight,
                                       int32_t offset,
                                       int32_t rnd_val,
                                       int32_t width)
{
    uint32_t loop_cnt, cnt;
    uint8_t *src_tmp;
    uint8_t *dst_tmp;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;

    __m128i src0, src1, src2, src3, src4, src5, src6, src7, src8;
    __m128i filt0, filt1, filt2, filt3;
    __m128i filt_h0, filt_h1, filt_h2, filt_h3;
    __m128i mask1, mask2, mask3;
    __m128i filter_vec;
    __m128i vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7;
    __m128i vec8, vec9, vec10, vec11, vec12, vec13, vec14, vec15;
    __m128i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8;
    __m128i dst0_r, dst0_l, dst1_r, dst1_l;
    __m128i dst10_r, dst32_r, dst54_r, dst76_r;
    __m128i dst10_l, dst32_l, dst54_l, dst76_l;
    __m128i dst21_r, dst43_r, dst65_r, dst87_r;
    __m128i dst21_l, dst43_l, dst65_l, dst87_l;
    __m128i weight_vec, offset_vec, rnd_vec, const_128, denom_vec;
    __m128i mask0 = __lsx_vld(ff_hevc_mask_arr, 0);

    src -= (src_stride_3x + 3);
    weight_vec = __lsx_vreplgr2vr_w(weight);
    offset_vec = __lsx_vreplgr2vr_w(offset);
    rnd_vec = __lsx_vreplgr2vr_w(rnd_val);
    denom_vec = __lsx_vsubi_wu(rnd_vec, 6);

    const_128 = __lsx_vldi(0x880);
    const_128 = __lsx_vmul_w(weight_vec, const_128);
    denom_vec = __lsx_vsrar_w(const_128, denom_vec);
    offset_vec = __lsx_vadd_w(denom_vec, offset_vec);

    LSX_DUP4_ARG2(__lsx_vldrepl_h, filter_x, 0, filter_x, 2, filter_x, 4, filter_x, 6,
                  filt0, filt1, filt2, filt3);
    filter_vec = __lsx_vld(filter_y, 0);
    filter_vec = __lsx_vsllwil_h_b(filter_vec, 0);
    LSX_DUP4_ARG2(__lsx_vreplvei_w, filter_vec, 0, filter_vec, 1, filter_vec, 2,
                  filter_vec, 3, filt_h0, filt_h1, filt_h2, filt_h3);
    LSX_DUP2_ARG2(__lsx_vaddi_bu, mask0, 2, mask0, 4, mask1, mask2);
    mask3 = __lsx_vaddi_bu(mask0, 6);

    for (cnt = width >> 3; cnt--;) {
        src_tmp = src;
        dst_tmp = dst;

        LSX_DUP4_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0,
                      src_tmp + src_stride_2x, 0, src_tmp + src_stride_3x, 0, src0,
                      src1, src2, src3);
        src_tmp += src_stride_4x;
        LSX_DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src4, src5);
        src6 = __lsx_vld(src_tmp + src_stride_2x, 0);
        src_tmp += src_stride_3x;
        LSX_DUP4_ARG2(__lsx_vxori_b, src0, 128, src1, 128, src2, 128, src3, 128, src0,
                      src1, src2, src3);
        LSX_DUP2_ARG2(__lsx_vxori_b, src4, 128, src5, 128, src4, src5);
        src6 = __lsx_vxori_b(src6, 128);

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

        LSX_DUP4_ARG2(__lsx_vilvl_h, dst1, dst0, dst3, dst2, dst5, dst4, dst2, dst1,
                      dst10_r, dst32_r, dst54_r, dst21_r);
        LSX_DUP2_ARG2(__lsx_vilvl_h, dst4, dst3, dst6, dst5, dst43_r, dst65_r);
        LSX_DUP4_ARG2(__lsx_vilvh_h, dst1, dst0, dst3, dst2, dst5, dst4, dst2, dst1,
                      dst10_l, dst32_l, dst54_l, dst21_l);
        LSX_DUP2_ARG2(__lsx_vilvh_h, dst4, dst3, dst6, dst5, dst43_l, dst65_l);

        for (loop_cnt = height >> 1; loop_cnt--;) {
            LSX_DUP2_ARG2(__lsx_vld, src_tmp, 0, src_tmp + src_stride, 0, src7, src8);
            src_tmp += src_stride_2x;
            LSX_DUP2_ARG2(__lsx_vxori_b, src7, 128, src8, 128, src7, src8);
            LSX_DUP4_ARG3(__lsx_vshuf_b, src7, src7, mask0, src7, src7, mask1, src7,
                          src7, mask2, src7, src7, mask3, vec0, vec1, vec2, vec3);
            dst7 = __lsx_hevc_filt_8tap_h(vec0, vec1, vec2, vec3, filt0, filt1,
                                          filt2, filt3);
            dst76_r = __lsx_vilvl_h(dst7, dst6);
            dst76_l = __lsx_vilvh_h(dst7, dst6);
            dst0_r = __lsx_hevc_filt_8tap_w(dst10_r, dst32_r, dst54_r, dst76_r,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst0_l = __lsx_hevc_filt_8tap_w(dst10_l, dst32_l, dst54_l, dst76_l,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            LSX_DUP2_ARG2(__lsx_vsrai_w, dst0_r, 6, dst0_l, 6, dst0_r, dst0_l);

            /* row 8 */
            LSX_DUP4_ARG3(__lsx_vshuf_b, src8, src8, mask0, src8, src8, mask1, src8,
                          src8, mask2, src8, src8, mask3, vec0, vec1, vec2, vec3);
            dst8 = __lsx_hevc_filt_8tap_h(vec0, vec1, vec2, vec3, filt0, filt1,
                                          filt2, filt3);

            dst87_r = __lsx_vilvl_h(dst8, dst7);
            dst87_l = __lsx_vilvh_h(dst8, dst7);
            dst1_r = __lsx_hevc_filt_8tap_w(dst21_r, dst43_r, dst65_r, dst87_r,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            dst1_l = __lsx_hevc_filt_8tap_w(dst21_l, dst43_l, dst65_l, dst87_l,
                                            filt_h0, filt_h1, filt_h2, filt_h3);
            LSX_DUP2_ARG2(__lsx_vsrai_w, dst1_r, 6, dst1_l, 6, dst1_r, dst1_l);

            LSX_DUP2_ARG2(__lsx_vmul_w, dst0_r, weight_vec, dst0_l, weight_vec, dst0_r,
                          dst0_l);
            LSX_DUP2_ARG2(__lsx_vmul_w, dst1_r, weight_vec, dst1_l, weight_vec, dst1_r,
                          dst1_l);
            LSX_DUP4_ARG2(__lsx_vsrar_w, dst0_r, rnd_vec, dst1_r, rnd_vec, dst0_l,
                          rnd_vec, dst1_l, rnd_vec, dst0_r, dst1_r, dst0_l, dst1_l);

            LSX_DUP2_ARG2(__lsx_vadd_w, dst0_r, offset_vec, dst0_l, offset_vec, dst0_r,
                          dst0_l);
            LSX_DUP2_ARG2(__lsx_vadd_w, dst1_r, offset_vec, dst1_l, offset_vec, dst1_r,
                          dst1_l);
            LSX_DUP4_ARG1(__lsx_clamp255_w, dst0_r, dst1_r, dst0_l, dst1_l, dst0_r,
                          dst1_r, dst0_l, dst1_l);
            LSX_DUP2_ARG2(__lsx_vpickev_h, dst0_l, dst0_r, dst1_l, dst1_r, dst0_r, dst1_r);
            dst0_r = __lsx_vpickev_b(dst1_r, dst0_r);

            __lsx_vstelm_d(dst0_r, dst_tmp, 0, 0);
            __lsx_vstelm_d(dst0_r, dst_tmp + dst_stride, 0, 1);
            dst_tmp += dst_stride_2x;

            dst10_r = dst32_r;
            dst32_r = dst54_r;
            dst54_r = dst76_r;
            dst10_l = dst32_l;
            dst32_l = dst54_l;
            dst54_l = dst76_l;
            dst21_r = dst43_r;
            dst43_r = dst65_r;
            dst65_r = dst87_r;
            dst21_l = dst43_l;
            dst43_l = dst65_l;
            dst65_l = dst87_l;
            dst6 = dst8;
        }

        src += 8;
        dst += 8;
    }
}

static void hevc_hv_uniwgt_8t_8w_lsx(uint8_t *src,
                                     int32_t src_stride,
                                     uint8_t *dst,
                                     int32_t dst_stride,
                                     const int8_t *filter_x,
                                     const int8_t *filter_y,
                                     int32_t height,
                                     int32_t weight,
                                     int32_t offset,
                                     int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 8);
}

static void hevc_hv_uniwgt_8t_16w_lsx(uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int8_t *filter_x,
                                      const int8_t *filter_y,
                                      int32_t height,
                                      int32_t weight,
                                      int32_t offset,
                                      int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 16);
}

static void hevc_hv_uniwgt_8t_24w_lsx(uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int8_t *filter_x,
                                      const int8_t *filter_y,
                                      int32_t height,
                                      int32_t weight,
                                      int32_t offset,
                                      int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 24);
}

static void hevc_hv_uniwgt_8t_32w_lsx(uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int8_t *filter_x,
                                      const int8_t *filter_y,
                                      int32_t height,
                                      int32_t weight,
                                      int32_t offset,
                                      int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 32);
}

static void hevc_hv_uniwgt_8t_48w_lsx(uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int8_t *filter_x,
                                      const int8_t *filter_y,
                                      int32_t height,
                                      int32_t weight,
                                      int32_t offset,
                                      int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 48);
}

static void hevc_hv_uniwgt_8t_64w_lsx(uint8_t *src,
                                      int32_t src_stride,
                                      uint8_t *dst,
                                      int32_t dst_stride,
                                      const int8_t *filter_x,
                                      const int8_t *filter_y,
                                      int32_t height,
                                      int32_t weight,
                                      int32_t offset,
                                      int32_t rnd_val)
{
    hevc_hv_uniwgt_8t_8multx2mult_lsx(src, src_stride, dst, dst_stride,
                                      filter_x, filter_y, height, weight,
                                      offset, rnd_val, 64);
}


#define UNI_W_MC_HV(PEL, WIDTH, TAP)                                          \
void ff_hevc_put_hevc_uni_w_##PEL##_hv##WIDTH##_8_lsx(uint8_t *dst,           \
                                                      ptrdiff_t dst_stride,   \
                                                      uint8_t *src,           \
                                                      ptrdiff_t src_stride,   \
                                                      int height,             \
                                                      int denom,              \
                                                      int weight,             \
                                                      int offset,             \
                                                      intptr_t mx,            \
                                                      intptr_t my,            \
                                                      int width)              \
{                                                                             \
    const int8_t *filter_x = ff_hevc_##PEL##_filters[mx - 1];                 \
    const int8_t *filter_y = ff_hevc_##PEL##_filters[my - 1];                 \
    int shift = denom + 14 - 8;                                               \
                                                                              \
    hevc_hv_uniwgt_##TAP##t_##WIDTH##w_lsx(src, src_stride, dst, dst_stride,  \
                                           filter_x, filter_y,  height,       \
                                           weight, offset, shift);            \
}

UNI_W_MC_HV(qpel, 8, 8);
UNI_W_MC_HV(qpel, 16, 8);
UNI_W_MC_HV(qpel, 24, 8);
UNI_W_MC_HV(qpel, 32, 8);
UNI_W_MC_HV(qpel, 48, 8);
UNI_W_MC_HV(qpel, 64, 8);

#undef UNI_W_MC_HV
