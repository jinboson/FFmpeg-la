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

void ff_h264_idct_add16_lasx(uint8_t *dst, const int32_t *blk_offset,
                             int16_t *block, int32_t dst_stride,
                             const uint8_t nzc[15 * 8])
{
    int32_t i;

    for (i = 0; i < 16; i++) {
        int32_t nnz = nzc[scan8[i]];

        if (nnz == 1 && ((dctcoef *) block)[i * 16]) {
            ff_h264_idct_dc_add_8_lsx(dst + blk_offset[i],
                                      block + i * 16 * sizeof(pixel),
                                      dst_stride);
	} else if (nnz) {
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

        if (nnz == 1 && ((dctcoef *) block)[cnt * 16]) {
            ff_h264_idct8_dc_add_8_lasx(dst + blk_offset[cnt],
                                        block + cnt * 16 * sizeof(pixel),
                                        dst_stride);
        } else if (nnz) {
            ff_h264_idct8_add_8_lasx(dst + blk_offset[cnt],
                                     block + cnt * 16 * sizeof(pixel),
                                     dst_stride);
        }
    }
}


void ff_h264_idct_add8_lasx(uint8_t **dst, const int32_t *blk_offset,
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

void ff_h264_idct_add8_422_lasx(uint8_t **dst, const int32_t *blk_offset,
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

void ff_h264_idct_add16_intra_lasx(uint8_t *dst, const int32_t *blk_offset,
                                   int16_t *block, int32_t dst_stride,
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
