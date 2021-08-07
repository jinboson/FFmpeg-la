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

static void hevc_sao_edge_filter_0degree_4width_lsx(uint8_t *dst,
                                                    int32_t dst_stride,
                                                    uint8_t *src,
                                                    int32_t src_stride,
                                                    int16_t *sao_offset_val,
                                                    int32_t height)
{
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i cmp_minus10, cmp_minus11, diff_minus10, diff_minus11;
    __m128i sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i src_minus10, src_minus11, src_plus10, offset, src0, dst0;
    __m128i const1 = __lsx_vldi(1);
    __m128i zero = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src -= 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src_minus10, src_minus11);

    for (height -= 2; height; height -= 2) {
        src += src_stride_2x;
        src_minus10 = __lsx_vpickev_d(src_minus11, src_minus10);
        src0 = __lsx_vshuf_b(zero, src_minus10, shuf1);
        src_plus10 = __lsx_vshuf_b(zero, src_minus10, shuf2);

        LSX_DUP2_ARG2(__lsx_vseq_b, src0, src_minus10, src0, src_plus10, cmp_minus10,
                      cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src0, src_minus10, src0, src_plus10, cmp_minus10,
                      cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                      const1, cmp_minus11, diff_minus10, diff_minus11);

        offset = __lsx_vadd_b(diff_minus10, diff_minus11);
        offset = __lsx_vaddi_bu(offset, 2);

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src_minus10, src_minus11);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                      offset, offset, offset);
        src0 = __lsx_vxori_b(src0, 128);
        dst0 = __lsx_vsadd_b(src0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        __lsx_vstelm_w(dst0, dst, 0, 0);
        __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
        dst += dst_stride_2x;
    }

    src_minus10 = __lsx_vpickev_d(src_minus11, src_minus10);
    src0 = __lsx_vshuf_b(zero, src_minus10, shuf1);
    src_plus10 = __lsx_vshuf_b(zero, src_minus10, shuf2);

    LSX_DUP2_ARG2(__lsx_vseq_b, src0, src_minus10, src0, src_plus10, cmp_minus10,
                  cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src0, src_minus10, src0, src_plus10, cmp_minus10,
                  cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                      const1, cmp_minus11, diff_minus10, diff_minus11);

    offset = __lsx_vadd_b(diff_minus10, diff_minus11);
    offset = __lsx_vaddi_bu(offset, 2);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    src0 = __lsx_vxori_b(src0, 128);
    dst0 = __lsx_vsadd_b(src0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_w(dst0, dst, 0, 0);
    __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
}

static void hevc_sao_edge_filter_0degree_8width_lsx(uint8_t *dst,
                                                    int32_t dst_stride,
                                                    uint8_t *src,
                                                    int32_t src_stride,
                                                    int16_t *sao_offset_val,
                                                    int32_t height)
{
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i cmp_minus10, cmp_minus11, diff_minus10, diff_minus11;
    __m128i src0, src1, dst0, src_minus10, src_minus11, src_plus10, src_plus11;
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i zeros = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src -= 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src_minus10, src_minus11);

    for (height -= 2; height; height -= 2) {
        src += src_stride_2x;
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf1, zeros, src_minus11,
                      shuf1, src0, src1);
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11,
                      shuf2, src_plus10, src_plus11);
        LSX_DUP2_ARG2(__lsx_vpickev_d, src_minus11, src_minus10, src_plus11, src_plus10,
                      src_minus10, src_plus10);
        src0 = __lsx_vpickev_d(src1, src0);

        LSX_DUP2_ARG2(__lsx_vseq_b, src0, src_minus10, src0, src_plus10, cmp_minus10,
                      cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src0, src_minus10, src0, src_plus10, cmp_minus10,
                      cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                      const1, cmp_minus11, diff_minus10, diff_minus11);

        offset = __lsx_vadd_b(diff_minus10, diff_minus11);
        offset = __lsx_vaddi_bu(offset, 2);

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src, 0, src + src_stride, 0, src_minus10, src_minus11);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                      offset, offset, offset);
        src0 = __lsx_vxori_b(src0, 128);
        dst0 = __lsx_vsadd_b(src0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        __lsx_vstelm_d(dst0, dst, 0, 0);
        __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
        dst += dst_stride_2x;
    }

    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf1, zeros, src_minus11, shuf1,
                  src0, src1);
    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11, shuf2,
                  src_plus10, src_plus11);
    LSX_DUP2_ARG2(__lsx_vpickev_d, src_minus11, src_minus10, src_plus11, src_plus10,
                  src_minus10, src_plus10);
    src0 =  __lsx_vpickev_d(src1, src0);

    LSX_DUP2_ARG2(__lsx_vseq_b, src0, src_minus10, src0, src_plus10, cmp_minus10,
                  cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src0, src_minus10, src0, src_plus10, cmp_minus10,
                  cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                      const1, cmp_minus11, diff_minus10, diff_minus11);

    offset = __lsx_vadd_b(diff_minus10, diff_minus11);
    offset = __lsx_vaddi_bu(offset, 2);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    src0 = __lsx_vxori_b(src0, 128);
    dst0 = __lsx_vsadd_b(src0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_d(dst0, dst, 0, 0);
    __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
}

static void hevc_sao_edge_filter_0degree_16multiple_lsx(uint8_t *dst,
                                                        int32_t dst_stride,
                                                        uint8_t *src,
                                                        int32_t src_stride,
                                                        int16_t *sao_offset_val,
                                                        int32_t width,
                                                        int32_t height)
{
    uint8_t *dst_ptr, *src_minus1;
    int32_t v_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i sao_offset;
    __m128i cmp_minus10, cmp_plus10, diff_minus10, diff_plus10, cmp_minus11;
    __m128i cmp_plus11, diff_minus11, diff_plus11, cmp_minus12, cmp_plus12;
    __m128i diff_minus12, diff_plus12, cmp_minus13, cmp_plus13, diff_minus13;
    __m128i diff_plus13;
    __m128i src10, src11, src12, src13, dst0, dst1, dst2, dst3;
    __m128i src_minus10, src_minus11, src_minus12, src_minus13;
    __m128i offset_mask0, offset_mask1, offset_mask2, offset_mask3;
    __m128i src_zero0, src_zero1, src_zero2, src_zero3;
    __m128i src_plus10, src_plus11, src_plus12, src_plus13;

    sao_offset = __lsx_vld(sao_offset_val, 0);
    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    for (; height; height -= 4) {
        src_minus1 = src - 1;
        LSX_DUP4_ARG2(__lsx_vld, src_minus1, 0, src_minus1 + src_stride, 0,
                      src_minus1 + src_stride_2x, 0, src_minus1 + src_stride_3x, 0,
                      src_minus10, src_minus11, src_minus12, src_minus13);

        for (v_cnt = 0; v_cnt < width; v_cnt += 16) {
            src_minus1 += 16;
            dst_ptr = dst + v_cnt;
            LSX_DUP4_ARG2(__lsx_vld, src_minus1, 0, src_minus1 + src_stride, 0,
                          src_minus1 + src_stride_2x, 0, src_minus1 + src_stride_3x, 0,
                          src10, src11, src12, src13);
            LSX_DUP4_ARG3(__lsx_vshuf_b, src10, src_minus10, shuf1, src11, src_minus11,
                          shuf1, src12, src_minus12, shuf1, src13, src_minus13, shuf1,
                          src_zero0, src_zero1, src_zero2, src_zero3);
            LSX_DUP4_ARG3(__lsx_vshuf_b, src10, src_minus10, shuf2, src11, src_minus11,
                          shuf2, src12, src_minus12, shuf2, src13, src_minus13, shuf2,
                          src_plus10, src_plus11, src_plus12, src_plus13);

            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10,
                          diff_plus10, const1, cmp_plus10, diff_minus11, const1,
                          cmp_minus11, diff_plus11, const1, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus12, const1, cmp_minus12,
                          diff_plus12, const1, cmp_plus12, diff_minus13, const1,
                          cmp_minus13, diff_plus13, const1, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);

            LSX_DUP4_ARG2(__lsx_vadd_b, diff_minus10, diff_plus10, diff_minus11,
                          diff_plus11, diff_minus12, diff_plus12, diff_minus13,
                          diff_plus13, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP4_ARG2(__lsx_vaddi_bu, offset_mask0, 2, offset_mask1, 2, offset_mask2,
                          2, offset_mask3, 2, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask0, sao_offset,
                          sao_offset, offset_mask0, offset_mask0, offset_mask0);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask1, sao_offset,
                          sao_offset, offset_mask1, offset_mask1, offset_mask1);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask2, sao_offset,
                          sao_offset, offset_mask2, offset_mask2, offset_mask2);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask3, sao_offset,
                          sao_offset, offset_mask3, offset_mask3, offset_mask3);

            LSX_DUP4_ARG2(__lsx_vxori_b, src_zero0, 128, src_zero1, 128, src_zero2, 128,
                          src_zero3, 128, src_zero0, src_zero1, src_zero2, src_zero3);
            LSX_DUP4_ARG2(__lsx_vsadd_b, src_zero0, offset_mask0, src_zero1,
                          offset_mask1, src_zero2, offset_mask2, src_zero3, offset_mask3,
                          dst0, dst1, dst2, dst3);
            LSX_DUP4_ARG2(__lsx_vxori_b, dst0, 128, dst1, 128, dst2, 128, dst3, 128,
                          dst0, dst1, dst2, dst3);

            src_minus10 = src10;
            src_minus11 = src11;
            src_minus12 = src12;
            src_minus13 = src13;

            __lsx_vst(dst0, dst_ptr, 0);
            __lsx_vst(dst1, dst_ptr + dst_stride, 0);
            __lsx_vst(dst2, dst_ptr + dst_stride_2x, 0);
            __lsx_vst(dst3, dst_ptr + dst_stride_3x, 0);
        }
        src += src_stride_4x;
        dst += dst_stride_4x;
    }
}

static void hevc_sao_edge_filter_90degree_4width_lsx(uint8_t *dst,
                                                     int32_t dst_stride,
                                                     uint8_t *src,
                                                     int32_t src_stride,
                                                     int16_t *sao_offset_val,
                                                     int32_t height)
{
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i dst0;
    __m128i sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i cmp_minus10, diff_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus10, src_minus11, src10, src11;
    __m128i src_zero0, src_zero1;
    __m128i offset;
    __m128i offset_mask0, offset_mask1;

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    /* load in advance */
    LSX_DUP4_ARG2(__lsx_vld, src - src_stride, 0, src, 0, src + src_stride, 0,
                  src + src_stride_2x, 0, src_minus10, src_minus11, src10, src11);

    for (height -= 2; height; height -= 2) {
        src += src_stride_2x;
        LSX_DUP4_ARG2(__lsx_vilvl_b, src10, src_minus10, src_minus11, src_minus11,
                      src11, src_minus11, src10, src10, src_minus10, src_zero0,
                      src_minus11, src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                      offset, offset, offset);

        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src + src_stride, 0, src + src_stride_2x, 0,
                      src10, src11);

        __lsx_vstelm_w(dst0, dst, 0, 0);
        __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
        dst += dst_stride_2x;
    }

    LSX_DUP4_ARG2(__lsx_vilvl_b, src10, src_minus10, src_minus11, src_minus11, src11,
                  src_minus11, src10, src10, src_minus10, src_zero0, src_minus11,
                  src_zero1);
    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_bu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 = __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_w(dst0, dst, 0, 0);
    __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
}

static void hevc_sao_edge_filter_90degree_8width_lsx(uint8_t *dst,
                                                     int32_t dst_stride,
                                                     uint8_t *src,
                                                     int32_t src_stride,
                                                     int16_t *sao_offset_val,
                                                     int32_t height)
{
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i src_zero0, src_zero1, dst0;
    __m128i cmp_minus10, diff_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus10, src_minus11, src10, src11;
    __m128i offset_mask0, offset_mask1;

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src - src_stride, 0, src, 0, src_minus10, src_minus11);
    LSX_DUP2_ARG2(__lsx_vld, src + src_stride, 0, src + src_stride_2x, 0, src10, src11);

    for (height -= 2; height; height -= 2) {
        src += src_stride_2x;
        LSX_DUP4_ARG2(__lsx_vilvl_b, src10, src_minus10, src_minus11, src_minus11, src11,
                      src_minus11, src10, src10, src_minus10, src_zero0, src_minus11,
                      src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                      offset, offset, offset);

        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src + src_stride, 0, src + src_stride_2x, 0,
                      src10, src11);

        __lsx_vstelm_d(dst0, dst, 0, 0);
        __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
        dst += dst_stride_2x;
    }

    LSX_DUP4_ARG2(__lsx_vilvl_b, src10, src_minus10, src_minus11, src_minus11, src11,
                  src_minus11, src10, src10, src_minus10, src_zero0, src_minus11,
                  src_zero1);
    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 =  __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_d(dst0, dst, 0, 0);
    __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
}

static void hevc_sao_edge_filter_90degree_16multiple_lsx(uint8_t *dst,
                                                         int32_t dst_stride,
                                                         uint8_t *src,
                                                         int32_t src_stride,
                                                         int16_t *
                                                         sao_offset_val,
                                                         int32_t width,
                                                         int32_t height)
{
    uint8_t *src_orig = src;
    uint8_t *dst_orig = dst;
    int32_t h_cnt, v_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i cmp_minus10, cmp_plus10, diff_minus10, diff_plus10, cmp_minus11;
    __m128i cmp_plus11, diff_minus11, diff_plus11, cmp_minus12, cmp_plus12;
    __m128i diff_minus12, diff_plus12, cmp_minus13, cmp_plus13, diff_minus13;
    __m128i diff_plus13;
    __m128i src10, src_minus10, dst0, src11, src_minus11, dst1;
    __m128i src12, dst2, src13, dst3;
    __m128i offset_mask0, offset_mask1, offset_mask2, offset_mask3, sao_offset;

    sao_offset = __lsx_vld(sao_offset_val, 0);
    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    for (v_cnt = 0; v_cnt < width; v_cnt += 16) {
        src = src_orig + v_cnt;
        dst = dst_orig + v_cnt;

        LSX_DUP2_ARG2(__lsx_vld, src - src_stride, 0, src, 0, src_minus10, src_minus11);

        for (h_cnt = (height >> 2); h_cnt--;) {
            LSX_DUP4_ARG2(__lsx_vld, src + src_stride, 0, src + src_stride_2x, 0,
                          src + src_stride_3x, 0, src + src_stride_4x, 0,
                          src10, src11, src12, src13);

            LSX_DUP4_ARG2(__lsx_vseq_b, src_minus11, src_minus10, src_minus11, src10,
                          src10, src_minus11, src10, src11, cmp_minus10, cmp_plus10,
                          cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vseq_b, src11, src10, src11, src12, src12, src11, src12,
                          src13, cmp_minus12, cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_minus11, src_minus10, src_minus11, src10,
                          src10, src_minus11, src10, src11, cmp_minus10, cmp_plus10,
                          cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src11, src10, src11, src12, src12, src11, src12,
                          src13, cmp_minus12, cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10,
                          diff_plus10, const1, cmp_plus10, diff_minus11, const1,
                          cmp_minus11, diff_plus11, const1, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus12, const1, cmp_minus12,
                          diff_plus12, const1, cmp_plus12, diff_minus13, const1,
                          cmp_minus13, diff_plus13, const1, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);

            LSX_DUP4_ARG2(__lsx_vadd_b, diff_minus10, diff_plus10, diff_minus11,
                          diff_plus11, diff_minus12, diff_plus12, diff_minus13,
                          diff_plus13, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP4_ARG2(__lsx_vaddi_bu, offset_mask0, 2, offset_mask1, 2, offset_mask2,
                          2, offset_mask3, 2, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask0, sao_offset,
                          sao_offset, offset_mask0, offset_mask0, offset_mask0);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask1, sao_offset,
                          sao_offset, offset_mask1, offset_mask1, offset_mask1);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask2, sao_offset,
                          sao_offset, offset_mask2, offset_mask2, offset_mask2);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask3, sao_offset,
                          sao_offset, offset_mask3, offset_mask3, offset_mask3);

            src_minus10 = src12;
            LSX_DUP4_ARG2(__lsx_vxori_b, src_minus11, 128, src10, 128, src11, 128,
                          src12, 128, src_minus11, src10, src11, src12);
            LSX_DUP4_ARG2(__lsx_vsadd_b, src_minus11, offset_mask0, src10, offset_mask1,
                          src11, offset_mask2, src12, offset_mask3, dst0, dst1, dst2,
                          dst3);
            LSX_DUP4_ARG2(__lsx_vxori_b, dst0, 128, dst1, 128, dst2, 128, dst3, 128,
                          dst0, dst1, dst2, dst3);
            src_minus11 = src13;

            __lsx_vst(dst0, dst, 0);
            __lsx_vst(dst1, dst + dst_stride, 0);
            __lsx_vst(dst2, dst + dst_stride_2x, 0);
            __lsx_vst(dst3, dst + dst_stride_3x, 0);
            src += src_stride_4x;
            dst += dst_stride_4x;
        }
    }
}

static void hevc_sao_edge_filter_45degree_4width_lsx(uint8_t *dst,
                                                     int32_t dst_stride,
                                                     uint8_t *src,
                                                     int32_t src_stride,
                                                     int16_t *sao_offset_val,
                                                     int32_t height)
{
    uint8_t *src_orig;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i cmp_minus10, diff_minus10, src_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus11, src10, src11;
    __m128i src_plus0, src_zero0, src_plus1, src_zero1, dst0;
    __m128i offset_mask0, offset_mask1;
    __m128i zeros = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src_orig = src - 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src_orig - src_stride, 0, src_orig, 0, src_minus10,
                  src_minus11);
    LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                  src10, src11);

    for (height -= 2; height; height -= 2) {
        src_orig += src_stride_2x;

        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src10, shuf2, zeros, src11, shuf2,
                      src_plus0, src_plus1);

        LSX_DUP2_ARG2(__lsx_vilvl_b, src_plus0, src_minus10, src_plus1, src_minus11,
                      src_minus10, src_minus11);
        LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                     offset, offset, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x,
                      0, src10, src11);

        __lsx_vstelm_w(dst0, dst, 0, 0);
        __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
        dst += dst_stride_2x;
    }

    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                  src_zero0, src_zero1);
    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src10, shuf2, zeros, src11, shuf2, src_plus0,
                  src_plus1);

    LSX_DUP2_ARG2(__lsx_vilvl_b, src_plus0, src_minus10, src_plus1, src_minus11,
                  src_minus10, src_minus11);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1, src_zero0,
                  src_zero1);
    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 = __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_w(dst0, dst, 0, 0);
    __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
}

static void hevc_sao_edge_filter_45degree_8width_lsx(uint8_t *dst,
                                                     int32_t dst_stride,
                                                     uint8_t *src,
                                                     int32_t src_stride,
                                                     int16_t *sao_offset_val,
                                                     int32_t height)
{
    uint8_t *src_orig;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i cmp_minus10, diff_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus10, src10, src_minus11, src11;
    __m128i src_zero0, src_plus10, src_zero1, src_plus11, dst0;
    __m128i offset_mask0, offset_mask1;
    __m128i zeros = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src_orig = src - 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src_orig - src_stride, 0, src_orig, 0, src_minus10,
                  src_minus11);
    LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                  src10, src11);

    for (height -= 2; height; height -= 2) {
        src_orig += src_stride_2x;

        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src10, shuf2, zeros, src11, shuf2,
                      src_plus10, src_plus11);

        LSX_DUP2_ARG2(__lsx_vilvl_b, src_plus10, src_minus10, src_plus11, src_minus11,
                      src_minus10, src_minus11);
        LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                     offset, offset, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                      src10, src11)
        __lsx_vstelm_d(dst0, dst, 0, 0);
        __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
        dst += dst_stride_2x;
    }

    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                  src_zero0, src_zero1);
    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src10, shuf2, zeros, src11, shuf2, src_plus10,
                  src_plus11);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src_plus10, src_minus10, src_plus11, src_minus11,
                  src_minus10, src_minus11);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1, src_zero0,
                  src_zero1);

    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 = __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    src_minus10 = src10;
    src_minus11 = src11;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                  src10, src11);

    __lsx_vstelm_d(dst0, dst, 0, 0);
    __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
}

static void hevc_sao_edge_filter_45degree_16multiple_lsx(uint8_t *dst,
                                                         int32_t dst_stride,
                                                         uint8_t *src,
                                                         int32_t src_stride,
                                                         int16_t *
                                                         sao_offset_val,
                                                         int32_t width,
                                                         int32_t height)
{
    uint8_t *src_orig = src;
    uint8_t *dst_orig = dst;
    int32_t v_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i cmp_minus10, cmp_plus10, diff_minus10, diff_plus10, cmp_minus11;
    __m128i cmp_plus11, diff_minus11, diff_plus11, cmp_minus12, cmp_plus12;
    __m128i diff_minus12, diff_plus12, cmp_minus13, cmp_plus13, diff_minus13;
    __m128i diff_plus13, src_minus14, src_plus13;
    __m128i offset_mask0, offset_mask1, offset_mask2, offset_mask3;
    __m128i src10, src_minus10, dst0, src11, src_minus11, dst1;
    __m128i src12, src_minus12, dst2, src13, src_minus13, dst3;
    __m128i src_zero0, src_plus10, src_zero1, src_plus11, src_zero2, src_plus12;
    __m128i src_zero3, sao_offset;

    sao_offset = __lsx_vld(sao_offset_val, 0);
    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    for (; height; height -= 4) {
        src_orig = src - 1;
        dst_orig = dst;
        LSX_DUP4_ARG2(__lsx_vld, src_orig, 0, src_orig + src_stride, 0,
                      src_orig + src_stride_2x, 0, src_orig + src_stride_3x, 0,
                      src_minus11, src_minus12, src_minus13, src_minus14);

        for (v_cnt = 0; v_cnt < width; v_cnt += 16) {
            src_minus10 = __lsx_vld(src_orig - src_stride, 0);
            LSX_DUP4_ARG2(__lsx_vld, src_orig, 16, src_orig + src_stride, 16,
                          src_orig + src_stride_2x, 16, src_orig + src_stride_3x, 16,
                          src10, src11, src12, src13);
            src_plus13 = __lsx_vld(src + v_cnt + src_stride_4x, 1);
            src_orig += 16;

            LSX_DUP4_ARG3(__lsx_vshuf_b, src10, src_minus11, shuf1, src11, src_minus12,
                          shuf1, src12, src_minus13, shuf1, src13, src_minus14, shuf1,
                          src_zero0, src_zero1, src_zero2, src_zero3);
            LSX_DUP2_ARG3(__lsx_vshuf_b, src11, src_minus12, shuf2, src12, src_minus13,
                          shuf2, src_plus10, src_plus11);
            src_plus12 = __lsx_vshuf_b(src13, src_minus14, shuf2);

            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10,
                          diff_plus10, const1, cmp_plus10, diff_minus11, const1,
                          cmp_minus11, diff_plus11, const1, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus12, const1, cmp_minus12,
                          diff_plus12, const1, cmp_plus12, diff_minus13, const1,
                          cmp_minus13, diff_plus13, const1, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);

            LSX_DUP4_ARG2(__lsx_vadd_b, diff_minus10, diff_plus10, diff_minus11,
                          diff_plus11, diff_minus12, diff_plus12, diff_minus13,
                          diff_plus13, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP4_ARG2(__lsx_vaddi_bu, offset_mask0, 2, offset_mask1, 2, offset_mask2,
                          2, offset_mask3, 2, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);

            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask0, sao_offset,
                          sao_offset, offset_mask0, offset_mask0, offset_mask0);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask1, sao_offset,
                          sao_offset, offset_mask1, offset_mask1, offset_mask1);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask2, sao_offset,
                          sao_offset, offset_mask2, offset_mask2, offset_mask2);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask3, sao_offset,
                          sao_offset, offset_mask3, offset_mask3, offset_mask3);

            LSX_DUP4_ARG2(__lsx_vxori_b, src_zero0, 128, src_zero1, 128, src_zero2, 128,
                          src_zero3, 128, src_zero0, src_zero1, src_zero2, src_zero3);
            LSX_DUP4_ARG2(__lsx_vsadd_b, src_zero0, offset_mask0, src_zero1,
                          offset_mask1, src_zero2, offset_mask2, src_zero3, offset_mask3,
                          dst0, dst1, dst2, dst3);
            LSX_DUP4_ARG2(__lsx_vxori_b, dst0, 128, dst1, 128, dst2, 128, dst3, 128,
                          dst0, dst1, dst2, dst3);

            src_minus11 = src10;
            src_minus12 = src11;
            src_minus13 = src12;
            src_minus14 = src13;

            __lsx_vst(dst0, dst_orig, 0);
            __lsx_vst(dst1, dst_orig + dst_stride, 0);
            __lsx_vst(dst2, dst_orig + dst_stride_2x, 0);
            __lsx_vst(dst3, dst_orig + dst_stride_3x, 0);
            dst_orig += 16;
        }
        src += src_stride_4x;
        dst += dst_stride_4x;
    }
}

static void hevc_sao_edge_filter_135degree_4width_lsx(uint8_t *dst,
                                                      int32_t dst_stride,
                                                      uint8_t *src,
                                                      int32_t src_stride,
                                                      int16_t *sao_offset_val,
                                                      int32_t height)
{
    uint8_t *src_orig;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);

    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i src_zero0, src_zero1, dst0;
    __m128i cmp_minus10, diff_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus10, src10, src_minus11, src11;
    __m128i offset_mask0, offset_mask1;
    __m128i zeros = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src_orig = src - 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src_orig - src_stride, 0, src_orig, 0, src_minus10,
                  src_minus11);
    LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                  src10, src11);

    for (height -= 2; height; height -= 2) {
        src_orig += src_stride_2x;

        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11,
                      shuf2, src_minus10, src_minus11);

        LSX_DUP2_ARG2(__lsx_vilvl_b, src10, src_minus10, src11, src_minus11, src_minus10,
                      src_minus11);
        LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                     offset, offset, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                      src10, src11);

        __lsx_vstelm_w(dst0, dst, 0, 0);
        __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
        dst += dst_stride_2x;
    }

    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                  src_zero0, src_zero1);
    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11, shuf2,
                  src_minus10, src_minus11);

    LSX_DUP2_ARG2(__lsx_vilvl_b, src10, src_minus10, src11, src_minus11, src_minus10,
                 src_minus11);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1, src_zero0,
                  src_zero1);
    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 = __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_w(dst0, dst, 0, 0);
    __lsx_vstelm_w(dst0, dst + dst_stride, 0, 2);
    dst += dst_stride_2x;
}

static void hevc_sao_edge_filter_135degree_8width_lsx(uint8_t *dst,
                                                      int32_t dst_stride,
                                                      uint8_t *src,
                                                      int32_t src_stride,
                                                      int16_t *sao_offset_val,
                                                      int32_t height)
{
    uint8_t *src_orig;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);

    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i offset, sao_offset = __lsx_vld(sao_offset_val, 0);
    __m128i cmp_minus10, diff_minus10, cmp_minus11, diff_minus11;
    __m128i src_minus10, src10, src_minus11, src11;
    __m128i src_zero0, src_zero1, dst0;
    __m128i offset_mask0, offset_mask1;
    __m128i zeros = {0};

    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);
    src_orig = src - 1;

    /* load in advance */
    LSX_DUP2_ARG2(__lsx_vld, src_orig - src_stride, 0, src_orig, 0, src_minus10,
                  src_minus11);
    LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                  src10, src11);

    for (height -= 2; height; height -= 2) {
        src_orig += src_stride_2x;

        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11,
                      shuf2, src_minus10, src_minus11);

        LSX_DUP2_ARG2(__lsx_vilvl_b, src10, src_minus10, src11, src_minus11, src_minus10,
                      src_minus11);
        LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1,
                      src_zero0, src_zero1);
        LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      diff_minus10, diff_minus11);
        LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                      cmp_minus10, cmp_minus11);
        LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                       const1, cmp_minus11,  diff_minus10, diff_minus11);

        LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                      diff_minus11, offset_mask0, offset_mask1);
        LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                      offset_mask1);
        LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                      offset, dst0);
        LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                     offset, offset, offset);
        dst0 = __lsx_vxori_b(dst0, 128);
        dst0 = __lsx_vsadd_b(dst0, offset);
        dst0 = __lsx_vxori_b(dst0, 128);

        src_minus10 = src10;
        src_minus11 = src11;

        /* load in advance */
        LSX_DUP2_ARG2(__lsx_vld, src_orig + src_stride, 0, src_orig + src_stride_2x, 0,
                      src10, src11);

        __lsx_vstelm_d(dst0, dst, 0, 0);
        __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
        dst += dst_stride_2x;
    }

    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus11, shuf1, zeros, src10, shuf1,
                  src_zero0, src_zero1);
    LSX_DUP2_ARG3(__lsx_vshuf_b, zeros, src_minus10, shuf2, zeros, src_minus11, shuf2,
                  src_minus10, src_minus11);

    LSX_DUP2_ARG2(__lsx_vilvl_b, src10, src_minus10, src11, src_minus11, src_minus10,
                  src_minus11);
    LSX_DUP2_ARG2(__lsx_vilvl_b, src_zero0, src_zero0, src_zero1, src_zero1, src_zero0,
                  src_zero1);
    LSX_DUP2_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  diff_minus10, diff_minus11);
    LSX_DUP2_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero1, src_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_minus11, cmp_minus11,
                  cmp_minus10, cmp_minus11);
    LSX_DUP2_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10, diff_minus11,
                  const1, cmp_minus11, diff_minus10, diff_minus11);

    LSX_DUP2_ARG2(__lsx_vhaddw_hu_bu, diff_minus10, diff_minus10, diff_minus11,
                  diff_minus11, offset_mask0, offset_mask1);
    LSX_DUP2_ARG2(__lsx_vaddi_hu, offset_mask0, 2, offset_mask1, 2, offset_mask0,
                  offset_mask1);
    LSX_DUP2_ARG2(__lsx_vpickev_b, offset_mask1, offset_mask0, src_zero1, src_zero0,
                  offset, dst0);
    LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset, sao_offset, sao_offset,
                  offset, offset, offset);
    dst0 = __lsx_vxori_b(dst0, 128);
    dst0 = __lsx_vsadd_b(dst0, offset);
    dst0 = __lsx_vxori_b(dst0, 128);

    __lsx_vstelm_d(dst0, dst, 0, 0);
    __lsx_vstelm_d(dst0, dst + dst_stride, 0, 1);
}

static void hevc_sao_edge_filter_135degree_16multiple_lsx(uint8_t *dst,
                                                          int32_t dst_stride,
                                                          uint8_t *src,
                                                          int32_t src_stride,
                                                          int16_t *sao_offset_val,
                                                          int32_t width,
                                                          int32_t height)
{
    uint8_t *src_orig, *dst_orig;
    int32_t v_cnt;
    const int32_t src_stride_2x = (src_stride << 1);
    const int32_t dst_stride_2x = (dst_stride << 1);
    const int32_t src_stride_4x = (src_stride << 2);
    const int32_t dst_stride_4x = (dst_stride << 2);
    const int32_t src_stride_3x = src_stride_2x + src_stride;
    const int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    __m128i shuf1 = {0x807060504030201, 0x100F0E0D0C0B0A09};
    __m128i shuf2 = {0x908070605040302, 0x11100F0E0D0C0B0A};
    __m128i edge_idx = {0x403000201, 0x0};
    __m128i const1 = __lsx_vldi(1);
    __m128i dst0, dst1, dst2, dst3;
    __m128i cmp_minus10, cmp_minus11, cmp_minus12, cmp_minus13, cmp_plus10;
    __m128i cmp_plus11, cmp_plus12, cmp_plus13, diff_minus10, diff_minus11;
    __m128i diff_minus12, diff_minus13, diff_plus10, diff_plus11, diff_plus12;
    __m128i diff_plus13, src10, src11, src12, src13, src_minus10, src_minus11;
    __m128i src_plus10, src_plus11, src_plus12, src_plus13;
    __m128i src_minus12, src_minus13, src_zero0, src_zero1, src_zero2, src_zero3;
    __m128i offset_mask0, offset_mask1, offset_mask2, offset_mask3, sao_offset;

    sao_offset = __lsx_vld(sao_offset_val, 0);
    sao_offset = __lsx_vpickev_b(sao_offset, sao_offset);

    for (; height; height -= 4) {
        src_orig = src - 1;
        dst_orig = dst;

        LSX_DUP4_ARG2(__lsx_vld, src_orig, 0, src_orig + src_stride, 0,
                      src_orig + src_stride_2x, 0, src_orig + src_stride_3x, 0,
                      src_minus11, src_plus10, src_plus11, src_plus12);

        for (v_cnt = 0; v_cnt < width; v_cnt += 16) {
            src_minus10 = __lsx_vld(src_orig - src_stride, 2);
            LSX_DUP4_ARG2(__lsx_vld, src_orig, 16, src_orig + src_stride, 16,
                          src_orig + src_stride_2x, 16, src_orig + src_stride_3x, 16,
                          src10, src11, src12, src13);
            src_plus13 = __lsx_vld(src_orig + src_stride_4x, 0);
            src_orig += 16;

            LSX_DUP4_ARG3(__lsx_vshuf_b, src10, src_minus11, shuf1, src11, src_plus10,
                          shuf1, src12, src_plus11, shuf1, src13, src_plus12, shuf1,
                          src_zero0, src_zero1, src_zero2, src_zero3);
            src_minus11 = __lsx_vshuf_b(src10, src_minus11, shuf2);
            LSX_DUP2_ARG3(__lsx_vshuf_b, src11, src_plus10, shuf2, src12, src_plus11,
                          shuf2, src_minus12, src_minus13);

            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vseq_b, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero0, src_minus10, src_zero0, src_plus10,
                          src_zero1, src_minus11, src_zero1, src_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vsle_bu, src_zero2, src_minus12, src_zero2, src_plus12,
                          src_zero3, src_minus13, src_zero3, src_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus10, cmp_minus10, cmp_plus10, cmp_plus10,
                          cmp_minus11, cmp_minus11, cmp_plus11, cmp_plus11, cmp_minus10,
                          cmp_plus10, cmp_minus11, cmp_plus11);
            LSX_DUP4_ARG2(__lsx_vnor_v, cmp_minus12, cmp_minus12, cmp_plus12, cmp_plus12,
                          cmp_minus13, cmp_minus13, cmp_plus13, cmp_plus13, cmp_minus12,
                          cmp_plus12, cmp_minus13, cmp_plus13);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus10, const1, cmp_minus10,
                          diff_plus10, const1, cmp_plus10, diff_minus11, const1,
                          cmp_minus11, diff_plus11, const1, cmp_plus11, diff_minus10,
                          diff_plus10, diff_minus11, diff_plus11);
            LSX_DUP4_ARG3(__lsx_vbitsel_v, diff_minus12, const1, cmp_minus12,
                          diff_plus12, const1, cmp_plus12, diff_minus13, const1,
                          cmp_minus13, diff_plus13, const1, cmp_plus13, diff_minus12,
                          diff_plus12, diff_minus13, diff_plus13);

            LSX_DUP4_ARG2(__lsx_vadd_b, diff_minus10, diff_plus10, diff_minus11,
                          diff_plus11, diff_minus12, diff_plus12, diff_minus13,
                          diff_plus13, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);
            LSX_DUP4_ARG2(__lsx_vaddi_bu, offset_mask0, 2, offset_mask1, 2, offset_mask2,
                          2, offset_mask3, 2, offset_mask0, offset_mask1, offset_mask2,
                          offset_mask3);

            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask0, sao_offset,
                          sao_offset, offset_mask0, offset_mask0, offset_mask0);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask1, sao_offset,
                           sao_offset, offset_mask1, offset_mask1, offset_mask1);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask2, sao_offset,
                          sao_offset, offset_mask2, offset_mask2, offset_mask2);
            LSX_DUP2_ARG3(__lsx_vshuf_b, edge_idx, edge_idx, offset_mask3, sao_offset,
                          sao_offset, offset_mask3, offset_mask3, offset_mask3);

            LSX_DUP4_ARG2(__lsx_vxori_b, src_zero0, 128, src_zero1, 128, src_zero2, 128,
                          src_zero3, 128, src_zero0, src_zero1, src_zero2, src_zero3);
            LSX_DUP4_ARG2(__lsx_vsadd_b, src_zero0, offset_mask0, src_zero1,
                          offset_mask1, src_zero2, offset_mask2, src_zero3, offset_mask3,
                          dst0, dst1, dst2, dst3);
            LSX_DUP4_ARG2(__lsx_vxori_b, dst0, 128, dst1, 128, dst2, 128, dst3, 128,
                          dst0, dst1, dst2, dst3);

            src_minus11 = src10;
            src_plus10 = src11;
            src_plus11 = src12;
            src_plus12 = src13;

            __lsx_vst(dst0, dst_orig, 0);
            __lsx_vst(dst1, dst_orig + dst_stride, 0);
            __lsx_vst(dst2, dst_orig + dst_stride_2x, 0);
            __lsx_vst(dst3, dst_orig + dst_stride_3x, 0);
            dst_orig += 16;
        }

        src += src_stride_4x;
        dst += dst_stride_4x;
    }
}

void ff_hevc_sao_edge_filter_8_lsx(uint8_t *dst, uint8_t *src,
                                   ptrdiff_t stride_dst,
                                   int16_t *sao_offset_val,
                                   int eo, int width, int height)
{
    ptrdiff_t stride_src = (2 * MAX_PB_SIZE + AV_INPUT_BUFFER_PADDING_SIZE) / sizeof(uint8_t);

    switch (eo) {
    case 0:
        if (width >> 4) {
            hevc_sao_edge_filter_0degree_16multiple_lsx(dst, stride_dst,
                                                        src, stride_src,
                                                        sao_offset_val,
                                                        width - (width & 0x0F),
                                                        height);
            dst += width - (width & 0x0F);
            src += width - (width & 0x0F);
            width &= 0x0F;
        }

        if (width >> 3) {
            hevc_sao_edge_filter_0degree_8width_lsx(dst, stride_dst,
                                                    src, stride_src,
                                                    sao_offset_val, height);
            dst += 8;
            src += 8;
            width &= 0x07;
        }

        if (width) {
            hevc_sao_edge_filter_0degree_4width_lsx(dst, stride_dst,
                                                    src, stride_src,
                                                    sao_offset_val, height);
        }
        break;

    case 1:
        if (width >> 4) {
            hevc_sao_edge_filter_90degree_16multiple_lsx(dst, stride_dst,
                                                         src, stride_src,
                                                         sao_offset_val,
                                                         width - (width & 0x0F),
                                                         height);
            dst += width - (width & 0x0F);
            src += width - (width & 0x0F);
            width &= 0x0F;
        }

        if (width >> 3) {
            hevc_sao_edge_filter_90degree_8width_lsx(dst, stride_dst,
                                                     src, stride_src,
                                                     sao_offset_val, height);
            dst += 8;
            src += 8;
            width &= 0x07;
        }

        if (width) {
            hevc_sao_edge_filter_90degree_4width_lsx(dst, stride_dst,
                                                     src, stride_src,
                                                     sao_offset_val, height);
        }
        break;

    case 2:
        if (width >> 4) {
            hevc_sao_edge_filter_45degree_16multiple_lsx(dst, stride_dst,
                                                         src, stride_src,
                                                         sao_offset_val,
                                                         width - (width & 0x0F),
                                                         height);
            dst += width - (width & 0x0F);
            src += width - (width & 0x0F);
            width &= 0x0F;
        }

        if (width >> 3) {
            hevc_sao_edge_filter_45degree_8width_lsx(dst, stride_dst,
                                                     src, stride_src,
                                                     sao_offset_val, height);
            dst += 8;
            src += 8;
            width &= 0x07;
        }

        if (width) {
            hevc_sao_edge_filter_45degree_4width_lsx(dst, stride_dst,
                                                     src, stride_src,
                                                     sao_offset_val, height);
        }
        break;

    case 3:
        if (width >> 4) {
            hevc_sao_edge_filter_135degree_16multiple_lsx(dst, stride_dst,
                                                          src, stride_src,
                                                          sao_offset_val,
                                                          width - (width & 0x0F),
                                                          height);
            dst += width - (width & 0x0F);
            src += width - (width & 0x0F);
            width &= 0x0F;
        }

        if (width >> 3) {
            hevc_sao_edge_filter_135degree_8width_lsx(dst, stride_dst,
                                                      src, stride_src,
                                                      sao_offset_val, height);
            dst += 8;
            src += 8;
            width &= 0x07;
        }

        if (width) {
            hevc_sao_edge_filter_135degree_4width_lsx(dst, stride_dst,
                                                      src, stride_src,
                                                      sao_offset_val, height);
        }
        break;
    }
}
