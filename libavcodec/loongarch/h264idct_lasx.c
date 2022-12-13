/*
 * Loongson LASX optimized h264dsp
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
 *                Xiwei  Gu  <guxiwei-hf@loongson.cn>
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
#include "h264dsp_lasx.h"
#include "libavcodec/bit_depth_template.c"

void ff_h264_idct8_addblk_lasx(uint8_t *dst, int16_t *src,
                               int32_t dst_stride)
{
    __m256i src0, src1, src2, src3, src4, src5, src6, src7;
    __m256i vec0, vec1, vec2, vec3;
    __m256i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    __m256i res0, res1, res2, res3, res4, res5, res6, res7;
    __m256i dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7;
    __m256i zero = __lasx_xvldi(0);
    int32_t dst_stride_2x = dst_stride << 1;
    int32_t dst_stride_4x = dst_stride << 2;
    int32_t dst_stride_3x = dst_stride_2x + dst_stride;

    src[0] += 32;
    DUP4_ARG2(__lasx_xvld, src, 0, src, 16, src, 32, src, 48,
              src0, src1, src2, src3);
    DUP4_ARG2(__lasx_xvld, src, 64, src, 80, src, 96, src, 112,
              src4, src5, src6, src7);
    __lasx_xvst(zero, src, 0);
    __lasx_xvst(zero, src, 32);
    __lasx_xvst(zero, src, 64);
    __lasx_xvst(zero, src, 96);

    vec0 = __lasx_xvadd_h(src0, src4);
    vec1 = __lasx_xvsub_h(src0, src4);
    vec2 = __lasx_xvsrai_h(src2, 1);
    vec2 = __lasx_xvsub_h(vec2, src6);
    vec3 = __lasx_xvsrai_h(src6, 1);
    vec3 = __lasx_xvadd_h(src2, vec3);

    LASX_BUTTERFLY_4_H(vec0, vec1, vec2, vec3, tmp0, tmp1, tmp2, tmp3);

    vec0 = __lasx_xvsrai_h(src7, 1);
    vec0 = __lasx_xvsub_h(src5, vec0);
    vec0 = __lasx_xvsub_h(vec0, src3);
    vec0 = __lasx_xvsub_h(vec0, src7);

    vec1 = __lasx_xvsrai_h(src3, 1);
    vec1 = __lasx_xvsub_h(src1, vec1);
    vec1 = __lasx_xvadd_h(vec1, src7);
    vec1 = __lasx_xvsub_h(vec1, src3);

    vec2 = __lasx_xvsrai_h(src5, 1);
    vec2 = __lasx_xvsub_h(vec2, src1);
    vec2 = __lasx_xvadd_h(vec2, src7);
    vec2 = __lasx_xvadd_h(vec2, src5);

    vec3 = __lasx_xvsrai_h(src1, 1);
    vec3 = __lasx_xvadd_h(src3, vec3);
    vec3 = __lasx_xvadd_h(vec3, src5);
    vec3 = __lasx_xvadd_h(vec3, src1);

    tmp4 = __lasx_xvsrai_h(vec3, 2);
    tmp4 = __lasx_xvadd_h(tmp4, vec0);
    tmp5 = __lasx_xvsrai_h(vec2, 2);
    tmp5 = __lasx_xvadd_h(tmp5, vec1);
    tmp6 = __lasx_xvsrai_h(vec1, 2);
    tmp6 = __lasx_xvsub_h(tmp6, vec2);
    tmp7 = __lasx_xvsrai_h(vec0, 2);
    tmp7 = __lasx_xvsub_h(vec3, tmp7);

    LASX_BUTTERFLY_8_H(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7,
                       res0, res1, res2, res3, res4, res5, res6, res7);
    LASX_TRANSPOSE8x8_H(res0, res1, res2, res3, res4, res5, res6, res7,
                        res0, res1, res2, res3, res4, res5, res6, res7);

    DUP4_ARG1(__lasx_vext2xv_w_h, res0, res1, res2, res3,
              tmp0, tmp1, tmp2, tmp3);
    DUP4_ARG1(__lasx_vext2xv_w_h, res4, res5, res6, res7,
              tmp4, tmp5, tmp6, tmp7);
    vec0 = __lasx_xvadd_w(tmp0, tmp4);
    vec1 = __lasx_xvsub_w(tmp0, tmp4);

    vec2 = __lasx_xvsrai_w(tmp2, 1);
    vec2 = __lasx_xvsub_w(vec2, tmp6);
    vec3 = __lasx_xvsrai_w(tmp6, 1);
    vec3 = __lasx_xvadd_w(vec3, tmp2);

    tmp0 = __lasx_xvadd_w(vec0, vec3);
    tmp2 = __lasx_xvadd_w(vec1, vec2);
    tmp4 = __lasx_xvsub_w(vec1, vec2);
    tmp6 = __lasx_xvsub_w(vec0, vec3);

    vec0 = __lasx_xvsrai_w(tmp7, 1);
    vec0 = __lasx_xvsub_w(tmp5, vec0);
    vec0 = __lasx_xvsub_w(vec0, tmp3);
    vec0 = __lasx_xvsub_w(vec0, tmp7);

    vec1 = __lasx_xvsrai_w(tmp3, 1);
    vec1 = __lasx_xvsub_w(tmp1, vec1);
    vec1 = __lasx_xvadd_w(vec1, tmp7);
    vec1 = __lasx_xvsub_w(vec1, tmp3);

    vec2 = __lasx_xvsrai_w(tmp5, 1);
    vec2 = __lasx_xvsub_w(vec2, tmp1);
    vec2 = __lasx_xvadd_w(vec2, tmp7);
    vec2 = __lasx_xvadd_w(vec2, tmp5);

    vec3 = __lasx_xvsrai_w(tmp1, 1);
    vec3 = __lasx_xvadd_w(tmp3, vec3);
    vec3 = __lasx_xvadd_w(vec3, tmp5);
    vec3 = __lasx_xvadd_w(vec3, tmp1);

    tmp1 = __lasx_xvsrai_w(vec3, 2);
    tmp1 = __lasx_xvadd_w(tmp1, vec0);
    tmp3 = __lasx_xvsrai_w(vec2, 2);
    tmp3 = __lasx_xvadd_w(tmp3, vec1);
    tmp5 = __lasx_xvsrai_w(vec1, 2);
    tmp5 = __lasx_xvsub_w(tmp5, vec2);
    tmp7 = __lasx_xvsrai_w(vec0, 2);
    tmp7 = __lasx_xvsub_w(vec3, tmp7);

    LASX_BUTTERFLY_4_W(tmp0, tmp2, tmp5, tmp7, res0, res1, res6, res7);
    LASX_BUTTERFLY_4_W(tmp4, tmp6, tmp1, tmp3, res2, res3, res4, res5);

    DUP4_ARG2(__lasx_xvsrai_w, res0, 6, res1, 6, res2, 6, res3, 6,
              res0, res1, res2, res3);
    DUP4_ARG2(__lasx_xvsrai_w, res4, 6, res5, 6, res6, 6, res7, 6,
              res4, res5, res6, res7);
    DUP4_ARG2(__lasx_xvpickev_h, res1, res0, res3, res2, res5, res4, res7,
              res6, res0, res1, res2, res3);
    DUP4_ARG2(__lasx_xvpermi_d, res0, 0xd8, res1, 0xd8, res2, 0xd8, res3, 0xd8,
              res0, res1, res2, res3);

    DUP4_ARG2(__lasx_xvldx, dst, 0, dst, dst_stride, dst, dst_stride_2x,
              dst, dst_stride_3x, dst0, dst1, dst2, dst3);
    dst += dst_stride_4x;
    DUP4_ARG2(__lasx_xvldx, dst, 0, dst, dst_stride, dst, dst_stride_2x,
              dst, dst_stride_3x, dst4, dst5, dst6, dst7);
    dst -= dst_stride_4x;
    DUP4_ARG2(__lasx_xvilvl_b, zero, dst0, zero, dst1, zero, dst2, zero, dst3,
              dst0, dst1, dst2, dst3);
    DUP4_ARG2(__lasx_xvilvl_b, zero, dst4, zero, dst5, zero, dst6, zero, dst7,
              dst4, dst5, dst6, dst7);
    DUP4_ARG3(__lasx_xvpermi_q, dst1, dst0, 0x20, dst3, dst2, 0x20, dst5,
              dst4, 0x20, dst7, dst6, 0x20, dst0, dst1, dst2, dst3);
    res0 = __lasx_xvadd_h(res0, dst0);
    res1 = __lasx_xvadd_h(res1, dst1);
    res2 = __lasx_xvadd_h(res2, dst2);
    res3 = __lasx_xvadd_h(res3, dst3);
    DUP4_ARG1(__lasx_xvclip255_h, res0, res1, res2, res3, res0, res1,
              res2, res3);
    DUP2_ARG2(__lasx_xvpickev_b, res1, res0, res3, res2, res0, res1);
    __lasx_xvstelm_d(res0, dst, 0, 0);
    __lasx_xvstelm_d(res0, dst + dst_stride, 0, 2);
    __lasx_xvstelm_d(res0, dst + dst_stride_2x, 0, 1);
    __lasx_xvstelm_d(res0, dst + dst_stride_3x, 0, 3);
    dst += dst_stride_4x;
    __lasx_xvstelm_d(res1, dst, 0, 0);
    __lasx_xvstelm_d(res1, dst + dst_stride, 0, 2);
    __lasx_xvstelm_d(res1, dst + dst_stride_2x, 0, 1);
    __lasx_xvstelm_d(res1, dst + dst_stride_3x, 0, 3);
}

void ff_h264_idct_add16_lasx(uint8_t *dst,
                             const int32_t *blk_offset,
                             int16_t *block, int32_t dst_stride,
                             const uint8_t nzc[15 * 8])
{
    int32_t i;

    for (i = 0; i < 16; i++) {
        int32_t nnz = nzc[scan8[i]];

        if (nnz) {
            if (nnz == 1 && ((dctcoef *) block)[i * 16])
                ff_h264_idct_dc_add_8_lsx(dst + blk_offset[i],
                                               block + i * 16 * sizeof(pixel),
                                               dst_stride);
            else
                ff_h264_idct_add_8_lsx(dst + blk_offset[i],
                                      block + i * 16 * sizeof(pixel),
                                      dst_stride);
        }
    }
}

void ff_h264_idct8_add4_lasx(uint8_t *dst, const int32_t *blk_offset,
                             int16_t *block, int32_t dst_stride,
                             const uint8_t nzc[15 * 8])
{
    int32_t cnt;

    for (cnt = 0; cnt < 16; cnt += 4) {
        int32_t nnz = nzc[scan8[cnt]];

        if (nnz) {
            if (nnz == 1 && ((dctcoef *) block)[cnt * 16])
                ff_h264_idct8_dc_add_8_lasx(dst + blk_offset[cnt],
                                             block + cnt * 16 * sizeof(pixel),
                                             dst_stride);
            else
                ff_h264_idct8_addblk_lasx(dst + blk_offset[cnt],
                                          block + cnt * 16 * sizeof(pixel),
                                          dst_stride);
        }
    }
}


void ff_h264_idct_add8_lasx(uint8_t **dst,
                            const int32_t *blk_offset,
                            int16_t *block, int32_t dst_stride,
                            const uint8_t nzc[15 * 8])
{
    int32_t i;

    for (i = 16; i < 20; i++) {
        if (nzc[scan8[i]])
            ff_h264_idct_add_8_lsx(dst[0] + blk_offset[i],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[0] + blk_offset[i],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
    for (i = 32; i < 36; i++) {
        if (nzc[scan8[i]])
            ff_h264_idct_add_8_lsx(dst[1] + blk_offset[i],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[1] + blk_offset[i],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
}

void ff_h264_idct_add8_422_lasx(uint8_t **dst,
                                const int32_t *blk_offset,
                                int16_t *block, int32_t dst_stride,
                                const uint8_t nzc[15 * 8])
{
    int32_t i;

    for (i = 16; i < 20; i++) {
        if (nzc[scan8[i]])
            ff_h264_idct_add_8_lsx(dst[0] + blk_offset[i],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[0] + blk_offset[i],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
    for (i = 32; i < 36; i++) {
        if (nzc[scan8[i]])
            ff_h264_idct_add_8_lsx(dst[1] + blk_offset[i],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[1] + blk_offset[i],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
    for (i = 20; i < 24; i++) {
        if (nzc[scan8[i + 4]])
            ff_h264_idct_add_8_lsx(dst[0] + blk_offset[i + 4],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[0] + blk_offset[i + 4],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
    for (i = 36; i < 40; i++) {
        if (nzc[scan8[i + 4]])
            ff_h264_idct_add_8_lsx(dst[1] + blk_offset[i + 4],
                                  block + i * 16 * sizeof(pixel),
                                  dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst[1] + blk_offset[i + 4],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
}

void ff_h264_idct_add16_intra_lasx(uint8_t *dst,
                                   const int32_t *blk_offset,
                                   int16_t *block,
                                   int32_t dst_stride,
                                   const uint8_t nzc[15 * 8])
{
    int32_t i;

    for (i = 0; i < 16; i++) {
        if (nzc[scan8[i]])
            ff_h264_idct_add_8_lsx(dst + blk_offset[i],
                                  block + i * 16 * sizeof(pixel), dst_stride);
        else if (((dctcoef *) block)[i * 16])
            ff_h264_idct_dc_add_8_lsx(dst + blk_offset[i],
                                           block + i * 16 * sizeof(pixel),
                                           dst_stride);
    }
}

void ff_h264_deq_idct_luma_dc_lasx(int16_t *dst, int16_t *src,
                                   int32_t de_qval)
{
#define DC_DEST_STRIDE 16

    __m256i src0, src1, src2, src3;
    __m256i vec0, vec1, vec2, vec3;
    __m256i tmp0, tmp1, tmp2, tmp3;
    __m256i hres0, hres1, hres2, hres3;
    __m256i vres0, vres1, vres2, vres3;
    __m256i de_q_vec = __lasx_xvreplgr2vr_w(de_qval);

    DUP4_ARG2(__lasx_xvld, src, 0, src, 8, src, 16, src, 24,
              src0, src1, src2, src3);
    LASX_TRANSPOSE4x4_H(src0, src1, src2, src3, tmp0, tmp1, tmp2, tmp3);
    LASX_BUTTERFLY_4_H(tmp0, tmp2, tmp3, tmp1, vec0, vec3, vec2, vec1);
    LASX_BUTTERFLY_4_H(vec0, vec1, vec2, vec3, hres0, hres3, hres2, hres1);
    LASX_TRANSPOSE4x4_H(hres0, hres1, hres2, hres3,
                        hres0, hres1, hres2, hres3);
    LASX_BUTTERFLY_4_H(hres0, hres1, hres3, hres2, vec0, vec3, vec2, vec1);
    LASX_BUTTERFLY_4_H(vec0, vec1, vec2, vec3, vres0, vres1, vres2, vres3);
    DUP4_ARG1(__lasx_vext2xv_w_h, vres0, vres1, vres2, vres3,
              vres0, vres1, vres2, vres3);
    DUP2_ARG3(__lasx_xvpermi_q, vres1, vres0, 0x20, vres3, vres2, 0x20,
              vres0, vres1);

    vres0 = __lasx_xvmul_w(vres0, de_q_vec);
    vres1 = __lasx_xvmul_w(vres1, de_q_vec);

    vres0 = __lasx_xvsrari_w(vres0, 8);
    vres1 = __lasx_xvsrari_w(vres1, 8);
    vec0 = __lasx_xvpickev_h(vres1, vres0);
    vec0 = __lasx_xvpermi_d(vec0, 0xd8);
    __lasx_xvstelm_h(vec0, dst + 0  * DC_DEST_STRIDE, 0, 0);
    __lasx_xvstelm_h(vec0, dst + 2  * DC_DEST_STRIDE, 0, 1);
    __lasx_xvstelm_h(vec0, dst + 8  * DC_DEST_STRIDE, 0, 2);
    __lasx_xvstelm_h(vec0, dst + 10 * DC_DEST_STRIDE, 0, 3);
    __lasx_xvstelm_h(vec0, dst + 1  * DC_DEST_STRIDE, 0, 4);
    __lasx_xvstelm_h(vec0, dst + 3  * DC_DEST_STRIDE, 0, 5);
    __lasx_xvstelm_h(vec0, dst + 9  * DC_DEST_STRIDE, 0, 6);
    __lasx_xvstelm_h(vec0, dst + 11 * DC_DEST_STRIDE, 0, 7);
    __lasx_xvstelm_h(vec0, dst + 4  * DC_DEST_STRIDE, 0, 8);
    __lasx_xvstelm_h(vec0, dst + 6  * DC_DEST_STRIDE, 0, 9);
    __lasx_xvstelm_h(vec0, dst + 12 * DC_DEST_STRIDE, 0, 10);
    __lasx_xvstelm_h(vec0, dst + 14 * DC_DEST_STRIDE, 0, 11);
    __lasx_xvstelm_h(vec0, dst + 5  * DC_DEST_STRIDE, 0, 12);
    __lasx_xvstelm_h(vec0, dst + 7  * DC_DEST_STRIDE, 0, 13);
    __lasx_xvstelm_h(vec0, dst + 13 * DC_DEST_STRIDE, 0, 14);
    __lasx_xvstelm_h(vec0, dst + 15 * DC_DEST_STRIDE, 0, 15);

#undef DC_DEST_STRIDE
}
