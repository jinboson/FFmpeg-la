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

#include "vc1dsp_loongarch.h"
#include "libavutil/loongarch/generic_macros_lasx.h"

void ff_vc1_inv_trans_8x8_lasx(int16_t block[64])
{
    int32_t con_4    = 4;
    int32_t con_64   = 64;
    __m256i in0, in1, in2, in3;
    __m256i temp0, temp1, temp2, temp3, t1, t2, t3, t4, t5, t6, t7, t8;
    __m256i const_1  = {0x000c000c000c000c, 0x000c000c000c000c, 0x000c000c000c000c, 0x000c000c000c000c};
    __m256i const_2  = {0xfff4000cfff4000c, 0xfff4000cfff4000c, 0xfff4000cfff4000c, 0xfff4000cfff4000c};
    __m256i const_3  = {0x0006001000060010, 0x0006001000060010, 0x0006001000060010, 0x0006001000060010};
    __m256i const_4  = {0xfff00006fff00006, 0xfff00006fff00006, 0xfff00006fff00006, 0xfff00006fff00006};
    __m256i const_5  = {0x000f0010000f0010, 0x000f0010000f0010, 0x000f0010000f0010, 0x000f0010000f0010};
    __m256i const_6  = {0x0004000900040009, 0x0004000900040009, 0x0004000900040009, 0x0004000900040009};
    __m256i const_7  = {0xfffc000ffffc000f, 0xfffc000ffffc000f, 0xfffc000ffffc000f, 0xfffc000ffffc000f};
    __m256i const_8  = {0xfff7fff0fff7fff0, 0xfff7fff0fff7fff0, 0xfff7fff0fff7fff0, 0xfff7fff0fff7fff0};
    __m256i const_9  = {0xfff00009fff00009, 0xfff00009fff00009, 0xfff00009fff00009, 0xfff00009fff00009};
    __m256i const_10 = {0x000f0004000f0004, 0x000f0004000f0004, 0x000f0004000f0004, 0x000f0004000f0004};
    __m256i const_11 = {0xfff70004fff70004, 0xfff70004fff70004, 0xfff70004fff70004, 0xfff70004fff70004};
    __m256i const_12 = {0xfff0000ffff0000f, 0xfff0000ffff0000f, 0xfff0000ffff0000f, 0xfff0000ffff0000f};

    LASX_LD_4(block, 16, in0, in1, in2, in3);
    in0 = __lasx_xvpermi_d(in0, 0xD8);
    in1 = __lasx_xvpermi_d(in1, 0xD8);
    in2 = __lasx_xvpermi_d(in2, 0xD8);
    in3 = __lasx_xvpermi_d(in3, 0xD8);
    /* first loops */
    LASX_ILVL_H_2_128SV(in2, in0, in3, in1, temp0, temp1);
    t2 = __lasx_xvldrepl_w(&con_4, 0);
    t1 = __lasx_xvdp2add_w_h(t2, temp0, const_1);
    t2 = __lasx_xvdp2add_w_h(t2, temp0, const_2);
    t3 = __lasx_xvdp2_w_h(temp1, const_3);
    t4 = __lasx_xvdp2_w_h(temp1, const_4);

    t5 = __lasx_xvadd_w(t1, t3);
    t6 = __lasx_xvadd_w(t2, t4);
    t7 = __lasx_xvsub_w(t2, t4);
    t8 = __lasx_xvsub_w(t1, t3);

    LASX_ILVH_H_2_128SV(in1, in0, in3, in2, temp0, temp1);
    temp2 = __lasx_xvdp2_w_h(const_5, temp0);
    t1    = __lasx_xvdp2add_w_h(temp2, temp1, const_6);
    temp2 = __lasx_xvdp2_w_h(const_7, temp0);
    t2    = __lasx_xvdp2add_w_h(temp2, temp1, const_8);
    temp2 = __lasx_xvdp2_w_h(const_9, temp0);
    t3    = __lasx_xvdp2add_w_h(temp2, temp1, const_10);
    temp2 = __lasx_xvdp2_w_h(const_11, temp0);
    t4    = __lasx_xvdp2add_w_h(temp2, temp1, const_12);

    temp0 = __lasx_xvadd_w(t1, t5);
    temp1 = __lasx_xvadd_w(t6, t2);
    temp2 = __lasx_xvadd_w(t7, t3);
    temp3 = __lasx_xvadd_w(t8, t4);
    in0   = __lasx_xvsub_w(t8, t4);
    in1   = __lasx_xvsub_w(t7, t3);
    in2   = __lasx_xvsub_w(t6, t2);
    in3   = __lasx_xvsub_w(t5, t1);
    LASX_SRAI_W_8(temp0, temp1, temp2, temp3, in0, in1, in2, in3,
                  temp0, temp1, temp2, temp3, in0, in1, in2, in3, 3);

    /* second loops */
    temp0 = __lasx_xvpackev_h(temp1, temp0);
    temp1 = __lasx_xvpackev_h(temp3, temp2);
    temp2 = __lasx_xvpackev_h(in1, in0);
    temp3 = __lasx_xvpackev_h(in3, in2);
    LASX_ILVL_W_2_128SV(temp1, temp0, temp3, temp2, t1, t3);
    LASX_ILVH_W_2_128SV(temp1, temp0, temp3, temp2, t2, t4);
    in0   = __lasx_xvpermi_q(t3, t1, 0x20);
    in1   = __lasx_xvpermi_q(t3, t1, 0x31);
    in2   = __lasx_xvpermi_q(t4, t2, 0x20);
    in3   = __lasx_xvpermi_q(t4, t2, 0x31);
    LASX_ILVL_H_2_128SV(in1, in0, in3, in2, temp0, temp1);
    t3    = __lasx_xvldrepl_w(&con_64, 0);
    t1    = __lasx_xvdp2add_w_h(t3, temp0, const_1);
    t2    = __lasx_xvdp2add_w_h(t3, temp0, const_2);
    t3    = __lasx_xvdp2_w_h(temp1, const_3);
    t4    = __lasx_xvdp2_w_h(temp1, const_4);

    t5    = __lasx_xvadd_w(t1, t3);
    t6    = __lasx_xvadd_w(t2, t4);
    t7    = __lasx_xvsub_w(t2, t4);
    t8    = __lasx_xvsub_w(t1, t3);

    LASX_ILVH_H_2_128SV(in2, in0, in3, in1, temp0, temp1);
    temp2 = __lasx_xvdp2_w_h(const_5, temp0);
    t1    = __lasx_xvdp2add_w_h(temp2, temp1, const_6);
    temp2 = __lasx_xvdp2_w_h(const_7, temp0);
    t2    = __lasx_xvdp2add_w_h(temp2, temp1, const_8);
    temp2 = __lasx_xvdp2_w_h(const_9, temp0);
    t3    = __lasx_xvdp2add_w_h(temp2, temp1, const_10);
    temp2 = __lasx_xvdp2_w_h(const_11, temp0);
    t4    = __lasx_xvdp2add_w_h(temp2, temp1, const_12);

    temp0 = __lasx_xvadd_w(t5, t1);
    temp1 = __lasx_xvadd_w(t6, t2);
    temp2 = __lasx_xvadd_w(t7, t3);
    temp3 = __lasx_xvadd_w(t8, t4);
    in0   = __lasx_xvsub_w(t8, t4);
    in1   = __lasx_xvsub_w(t7, t3);
    in2   = __lasx_xvsub_w(t6, t2);
    in3   = __lasx_xvsub_w(t5, t1);
    in0   = __lasx_xvaddi_wu(in0, 1);
    in1   = __lasx_xvaddi_wu(in1, 1);
    in2   = __lasx_xvaddi_wu(in2, 1);
    in3   = __lasx_xvaddi_wu(in3, 1);
    LASX_SRAI_W_8(temp0, temp1, temp2, temp3, in0, in1, in2, in3,
                  temp0, temp1, temp2, temp3, in0, in1, in2, in3, 7);
    t1 = __lasx_xvpickev_h(temp1, temp0);
    t2 = __lasx_xvpickev_h(temp3, temp2);
    t3 = __lasx_xvpickev_h(in1, in0);
    t4 = __lasx_xvpickev_h(in3, in2);
    in0 = __lasx_xvpermi_d(t1, 0xD8);
    in1 = __lasx_xvpermi_d(t2, 0xD8);
    in2 = __lasx_xvpermi_d(t3, 0xD8);
    in3 = __lasx_xvpermi_d(t4, 0xD8);
    LASX_ST_4(in0, in1, in2, in3, block, 16);
}

void ff_vc1_inv_trans_8x8_dc_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    int dc = block[0];
    uint8_t *dst   = dest + stride;
    __m256i in0, in1, const_dc, temp0;
    __m256i zero   = {0};

    dc = (3 * dc +  1) >> 1;
    dc = (3 * dc + 16) >> 5;

    const_dc = __lasx_xvldrepl_h(&dc, 0);
    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
    dest = dst + stride;
    dst  = dest + stride;
    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
    dest = dst + stride;
    dst  = dest + stride;
    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
    dest = dst + stride;
    dst  = dest + stride;
    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
}

void ff_vc1_inv_trans_8x4_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    uint8_t *dst = dest;
    __m256i shift    = {0x0000000400000000, 0x0000000500000001, 0x0000000600000002, 0x0000000700000003};
    __m256i const_64 = {0x0000004000000040, 0x0000004000000040, 0x0000004000000040, 0x0000004000000040};
    __m256i const_1  = {0x00060010000C000C, 0x00060010000C000C, 0x00060010000C000C, 0x00060010000C000C};
    __m256i const_2  = {0xFFF00006FFF4000C, 0xFFF00006FFF4000C, 0xFFF00006FFF4000C, 0xFFF00006FFF4000C};
    __m256i const_3  = {0x0004000F00090010, 0x0004000F00090010, 0x0004000F00090010, 0x0004000F00090010};
    __m256i const_4  = {0xFFF7FFFCFFF0000F, 0xFFF7FFFCFFF0000F, 0xFFF7FFFCFFF0000F, 0xFFF7FFFCFFF0000F};
    __m256i const_5  = {0x000FFFF000040009, 0x000FFFF000040009, 0x000FFFF000040009, 0x000FFFF000040009};
    __m256i const_6  = {0xFFF0FFF7000F0004, 0xFFF0FFF7000F0004, 0xFFF0FFF7000F0004, 0xFFF0FFF7000F0004};
    __m256i const_7  = {0x0000000000000004, 0x0000000000000004, 0x0000000000000004, 0x0000000000000004};
    __m256i const_8  = {0x0011001100110011, 0x0011001100110011, 0x0011001100110011, 0x0011001100110011};
    __m256i const_9  = {0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011};
    __m256i const_10 = {0x000A0016000A0016, 0x000A0016000A0016, 0x000A0016000A0016, 0x000A0016000A0016};
    __m256i const_11 = {0x0016FFF60016FFF6, 0x0016FFF60016FFF6, 0x0016FFF60016FFF6, 0x0016FFF60016FFF6};
    __m256i in0, in1;
    __m256i temp0, temp1, temp2, temp3, t1, t2, t3, t4;

    LASX_LD_2(block, 16, in0, in1);
    /* first loops */
    temp0 = __lasx_xvpermi_d(in0, 0xB1);
    temp1 = __lasx_xvpermi_d(in1, 0xB1);
    LASX_ILVL_H_2_128SV(temp0, in0, temp1, in1, temp0, temp1);
    temp2 = __lasx_xvpickev_w(temp1, temp0);
    temp3 = __lasx_xvpickod_w(temp1, temp0);

    temp0 = __lasx_xvdp2_w_h(temp2, const_1);
    temp1 = __lasx_xvdp2_w_h(temp2, const_2);
    t1    = __lasx_xvadd_w(temp0, const_7);
    t2    = __lasx_xvadd_w(temp1, const_7);
    temp0 = __lasx_xvpickev_w(t2, t1);
    temp1 = __lasx_xvpickod_w(t2, t1);
    t3    = __lasx_xvadd_w(temp0, temp1);
    t4    = __lasx_xvsub_w(temp0, temp1);
    t4    = __lasx_xvpermi_d(t4, 0xB1);

    t1    = __lasx_xvdp4_d_h(temp3, const_3);
    t2    = __lasx_xvdp4_d_h(temp3, const_4);
    temp0 = __lasx_xvdp4_d_h(temp3, const_5);
    temp1 = __lasx_xvdp4_d_h(temp3, const_6);

    temp2 = __lasx_xvpickev_w(t2, t1);
    temp3 = __lasx_xvpickev_w(temp1, temp0);

    t1    = __lasx_xvadd_w(temp2, t3);
    t2    = __lasx_xvadd_w(temp3, t4);
    temp0 = __lasx_xvsub_w(t4, temp3);
    temp1 = __lasx_xvsub_w(t3, temp2);
    LASX_SRAI_W_4(t1, t2, temp0, temp1, t1, t2, t3, t4, 3);
    /* second loops */
    temp2 = __lasx_xvpickev_h(t2, t1);
    temp3 = __lasx_xvpickev_h(t4, t3);
    temp3 = __lasx_xvshuf4i_h(temp3, 0x4E);
    temp0 = __lasx_xvpermi_q(temp3, temp2, 0x20);
    temp1 = __lasx_xvpermi_q(temp3, temp2, 0x31);
    t1    = __lasx_xvdp2add_w_h(const_64, temp0, const_8);
    t2    = __lasx_xvdp2add_w_h(const_64, temp0, const_9);
    t3    = __lasx_xvdp2_w_h(temp1, const_10);
    t4    = __lasx_xvdp2_w_h(temp1, const_11);
    temp0 = __lasx_xvadd_w(t1, t3);
    temp1 = __lasx_xvsub_w(t2, t4);
    temp2 = __lasx_xvadd_w(t2, t4);
    temp3 = __lasx_xvsub_w(t1, t3);
    LASX_SRAI_W_4(temp0, temp1, temp2, temp3, t1, t2, t3, t4, 7);

    temp0 = __lasx_xvldrepl_d(dst, 0);
    temp0 = __lasx_vext2xv_wu_bu(temp0);
    dst  += stride;
    temp1 = __lasx_xvldrepl_d(dst, 0);
    temp1 = __lasx_vext2xv_wu_bu(temp1);
    dst  += stride;
    temp2 = __lasx_xvldrepl_d(dst, 0);
    temp2 = __lasx_vext2xv_wu_bu(temp2);
    dst  += stride;
    temp3 = __lasx_xvldrepl_d(dst, 0);
    temp3 = __lasx_vext2xv_wu_bu(temp3);
    t1    = __lasx_xvadd_w(temp0, t1);
    t2    = __lasx_xvadd_w(temp1, t2);
    t3    = __lasx_xvadd_w(temp2, t3);
    t4    = __lasx_xvadd_w(temp3, t4);
    LASX_CLIP_W_0_255_4(t1, t2, t3, t4, t1, t2, t3, t4);
    LASX_PCKEV_H_2_128SV(t2, t1, t4, t3, temp0, temp1);
    temp2 = __lasx_xvpickev_b(temp1, temp0);
    temp0 = __lasx_xvperm_w(temp2, shift);
    LASX_ST_D_4(temp0, 0, 1, 2, 3, dest, stride);
}

void ff_vc1_inv_trans_8x4_dc_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    int dc = block[0];
    uint8_t *dst   = dest + stride;
    __m256i in0, in1, const_dc, temp0;
    __m256i zero = {0};

    dc = (3  * dc + 1) >> 1;
    dc = (17 * dc + 64) >> 7;
    const_dc = __lasx_xvldrepl_h(&dc, 0);

    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
    dest = dst + stride;
    dst  = dest + stride;
    in0   = __lasx_xvldrepl_d(dest, 0);
    in1   = __lasx_xvldrepl_d(dst, 0);
    in0   = __lasx_xvpermi_q(in1, in0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_d(temp0, dest, 0, 0);
    __lasx_xvstelm_d(temp0, dst, 0, 2);
}

void ff_vc1_inv_trans_4x8_dc_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    int dc = block[0];
    uint8_t *dst1 = dest + stride;
    uint8_t *dst2 = dst1 + stride;
    uint8_t *dst3 = dst2 + stride;
    __m256i in0, in1, in2, in3, const_dc, temp0, temp1;
    __m256i zero = {0};

    dc = (17 * dc +  4) >> 3;
    dc = (12 * dc + 64) >> 7;
    const_dc = __lasx_xvldrepl_h(&dc, 0);

    in0   = __lasx_xvldrepl_w(dest, 0);
    in1   = __lasx_xvldrepl_w(dst1, 0);
    in2   = __lasx_xvldrepl_w(dst2, 0);
    in3   = __lasx_xvldrepl_w(dst3, 0);
    LASX_ILVL_W_2_128SV(in1, in0, in3, in2, temp0, temp1);
    in0   = __lasx_xvpermi_q(temp1, temp0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_w(temp0, dest, 0, 0);
    __lasx_xvstelm_w(temp0, dst1, 0, 1);
    __lasx_xvstelm_w(temp0, dst2, 0, 4);
    __lasx_xvstelm_w(temp0, dst3, 0, 5);

    dest = dst3 + stride;
    dst1 = dest + stride;
    dst2 = dst1 + stride;
    dst3 = dst2 + stride;

    in0   = __lasx_xvldrepl_w(dest, 0);
    in1   = __lasx_xvldrepl_w(dst1, 0);
    in2   = __lasx_xvldrepl_w(dst2, 0);
    in3   = __lasx_xvldrepl_w(dst3, 0);
    LASX_ILVL_W_2_128SV(in1, in0, in3, in2, temp0, temp1);
    in0   = __lasx_xvpermi_q(temp1, temp0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_w(temp0, dest, 0, 0);
    __lasx_xvstelm_w(temp0, dst1, 0, 1);
    __lasx_xvstelm_w(temp0, dst2, 0, 4);
    __lasx_xvstelm_w(temp0, dst3, 0, 5);

}

void ff_vc1_inv_trans_4x8_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    uint8_t *dst = dest;
    __m256i in0, in1, in2, in3;
    __m256i temp0, temp1, temp2, temp3, t1, t2, t3, t4;

    __m256i const_1  = {0x0011001100110011, 0x0011001100110011, 0x0011001100110011, 0x0011001100110011};
    __m256i const_2  = {0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011, 0xFFEF0011FFEF0011};
    __m256i const_3  = {0x000A0016000A0016, 0x000A0016000A0016, 0x000A0016000A0016, 0x000A0016000A0016};
    __m256i const_4  = {0x0016FFF60016FFF6, 0x0016FFF60016FFF6, 0x0016FFF60016FFF6, 0x0016FFF60016FFF6};
    __m256i const_5  = {0x0000000400000004, 0x0000000400000004, 0x0000000400000004, 0x0000000400000004};
    __m256i const_6  = {0x0000004000000040, 0x0000004000000040, 0x0000004000000040, 0x0000004000000040};
    __m256i const_7  = {0x000C000C000C000C, 0X000C000C000C000C, 0xFFF4000CFFF4000C, 0xFFF4000CFFF4000C};
    __m256i const_8  = {0x0006001000060010, 0x0006001000060010, 0xFFF00006FFF00006, 0xFFF00006FFF00006};
    __m256i const_9  = {0x0009001000090010, 0x0009001000090010, 0x0004000F0004000F, 0x0004000F0004000F};
    __m256i const_10 = {0xFFF0000FFFF0000F, 0xFFF0000FFFF0000F, 0xFFF7FFFCFFF7FFFC, 0xFFF7FFFCFFF7FFFC};
    __m256i const_11 = {0x0004000900040009, 0x0004000900040009, 0x000FFFF0000FFFF0, 0x000FFFF0000FFFF0};
    __m256i const_12 = {0x000F0004000F0004, 0x000F0004000F0004, 0xFFF0FFF7FFF0FFF7, 0xFFF0FFF7FFF0FFF7};
    __m256i shift    = {0x0000000400000000, 0x0000000600000002, 0x0000000500000001, 0x0000000700000003};

    /* first loops */
    LASX_LD_4(block, 16, in0, in1, in2, in3);
    in0   = __lasx_xvilvl_d(in1, in0);
    in1   = __lasx_xvilvl_d(in3, in2);
    temp0 = __lasx_xvpickev_h(in1, in0);
    temp1 = __lasx_xvpickod_h(in1, in0);
    temp0 = __lasx_xvperm_w(temp0, shift);
    temp1 = __lasx_xvperm_w(temp1, shift);

    t1 = __lasx_xvdp2add_w_h(const_5, temp0, const_1);
    t2 = __lasx_xvdp2add_w_h(const_5, temp0, const_2);
    t3 = __lasx_xvdp2_w_h(temp1, const_3);
    t4 = __lasx_xvdp2_w_h(temp1, const_4);

    temp0 = __lasx_xvadd_w(t1, t3);
    temp1 = __lasx_xvsub_w(t2, t4);
    temp2 = __lasx_xvadd_w(t2, t4);
    temp3 = __lasx_xvsub_w(t1, t3);
    LASX_SRAI_W_4(temp0, temp1, temp2, temp3, temp0, temp1, temp2, temp3, 3);

    /* second loops */
    t1    = __lasx_xvpickev_w(temp1, temp0);
    t2    = __lasx_xvpickev_w(temp3, temp2);
    t1    = __lasx_xvpickev_h(t2, t1);
    t3    = __lasx_xvpickod_w(temp1, temp0);
    t4    = __lasx_xvpickod_w(temp3, temp2);
    temp1 = __lasx_xvpickev_h(t4, t3);
    temp2 = __lasx_xvpermi_q(t1, t1, 0x00);
    temp3 = __lasx_xvpermi_q(t1, t1, 0x11);
    t1    = __lasx_xvdp2add_w_h(const_6, temp2, const_7);
    t2    = __lasx_xvdp2_w_h(temp3, const_8);
    t3    = __lasx_xvadd_w(t1, t2);
    t4    = __lasx_xvsub_w(t1, t2);
    t4    = __lasx_xvpermi_d(t4, 0x4E);

    t1    = __lasx_xvdp2_w_h(temp1, const_9);
    t2    = __lasx_xvdp2_w_h(temp1, const_10);
    temp2 = __lasx_xvdp2_w_h(temp1, const_11);
    temp3 = __lasx_xvdp2_w_h(temp1, const_12);

    temp0 = __lasx_xvpermi_q(t2, t1, 0x20);
    temp1 = __lasx_xvpermi_q(t2, t1, 0x31);
    t1    = __lasx_xvadd_w(temp0, temp1);
    temp0 = __lasx_xvpermi_q(temp3, temp2, 0x20);
    temp1 = __lasx_xvpermi_q(temp3, temp2, 0x31);
    t2    = __lasx_xvadd_w(temp1, temp0);
    temp0 = __lasx_xvadd_w(t1, t3);
    temp1 = __lasx_xvadd_w(t2, t4);
    temp2 = __lasx_xvsub_w(t4, t2);
    temp3 = __lasx_xvsub_w(t3, t1);
    temp2 = __lasx_xvaddi_wu(temp2, 1);
    temp3 = __lasx_xvaddi_wu(temp3, 1);
    LASX_SRAI_W_4(temp0, temp1, temp2, temp3, temp0, temp1, temp2, temp3, 7);

    const_1 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_2 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_3 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_4 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_5 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_6 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_7 = __lasx_xvldrepl_w(dst, 0);
    dst += stride;
    const_8 = __lasx_xvldrepl_w(dst, 0);

    LASX_ILVL_W_4_128SV(const_2, const_1, const_4, const_3, const_5, const_6,
                        const_7, const_8, const_1, const_2, const_3, const_4);
    const_1 = __lasx_vext2xv_wu_bu(const_1);
    const_2 = __lasx_vext2xv_wu_bu(const_2);
    const_3 = __lasx_vext2xv_wu_bu(const_3);
    const_4 = __lasx_vext2xv_wu_bu(const_4);

    temp0   = __lasx_xvadd_w(temp0, const_1);
    temp1   = __lasx_xvadd_w(temp1, const_2);
    temp2   = __lasx_xvadd_w(temp2, const_3);
    temp3   = __lasx_xvadd_w(temp3, const_4);
    LASX_CLIP_W_0_255_4(temp0, temp1, temp2, temp3, temp0, temp1, temp2, temp3);
    LASX_PCKEV_H_2_128SV(temp1, temp0, temp3, temp2, temp0, temp1);
    temp0   = __lasx_xvpickev_b(temp1, temp0);
    LASX_ST_W_8(temp0, 0, 4, 1, 5, 6, 2, 7, 3, dest, stride);
}

void ff_vc1_inv_trans_4x4_dc_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    int dc = block[0];
    uint8_t *dst1 = dest + stride;
    uint8_t *dst2 = dst1 + stride;
    uint8_t *dst3 = dst2 + stride;
    __m256i in0, in1, in2, in3, temp0, temp1, const_dc;
    __m256i zero  = {0};

    dc = (17 * dc +  4) >> 3;
    dc = (17 * dc + 64) >> 7;
    const_dc = __lasx_xvldrepl_h(&dc, 0);

    in0   = __lasx_xvldrepl_w(dest, 0);
    in1   = __lasx_xvldrepl_w(dst1, 0);
    in2   = __lasx_xvldrepl_w(dst2, 0);
    in3   = __lasx_xvldrepl_w(dst3, 0);
    LASX_ILVL_W_2_128SV(in1, in0, in3, in2, temp0, temp1);
    in0   = __lasx_xvpermi_q(temp1, temp0, 0x20);
    LASX_ILVL_B_128SV(zero, in0, temp0);
    temp0 = __lasx_xvadd_h(temp0, const_dc);
    LASX_CLIP_H_0_255(temp0, in0);
    temp0 = __lasx_xvpickev_b(in0, in0);
    __lasx_xvstelm_w(temp0, dest, 0, 0);
    __lasx_xvstelm_w(temp0, dst1, 0, 1);
    __lasx_xvstelm_w(temp0, dst2, 0, 4);
    __lasx_xvstelm_w(temp0, dst3, 0, 5);
}

void ff_vc1_inv_trans_4x4_lasx(uint8_t *dest, ptrdiff_t stride, int16_t *block)
{
    uint8_t *dst = dest + stride;
    __m256i in0, in1, in2, in3;
    __m256i temp0, temp1, temp2, temp3, t1, t2;

    __m256i const_1  = {0x0011001100110011, 0xFFEF0011FFEF0011, 0x0011001100110011, 0xFFEF0011FFEF0011};
    __m256i const_2  = {0x000A0016000A0016, 0x0016FFF60016FFF6, 0x000A0016000A0016, 0x0016FFF60016FFF6};
    __m256i const_64 = {0x0000004000000040, 0x0000004000000040, 0x0000004000000040, 0x0000004000000040};

    LASX_LD_2(block, 16, in0, in1);
    /* first loops */
    temp0 = __lasx_xvilvl_d(in1, in0);
    temp1 = __lasx_xvpickev_h(temp0, temp0);
    temp2 = __lasx_xvpickod_h(temp0, temp0);
    t1    = __lasx_xvdp2_w_h(temp1, const_1);
    t2    = __lasx_xvdp2_w_h(temp2, const_2);
    t1    = __lasx_xvaddi_wu(t1, 4);
    in0   = __lasx_xvadd_w(t1, t2);
    in1   = __lasx_xvsub_w(t1, t2);
    LASX_SRAI_W_2(in0, in1, in0, in1, 3);
    /* second loops */
    temp0   = __lasx_xvpickev_h(in1, in0);
    temp1   = __lasx_xvpermi_q(temp0, temp0, 0x00);
    temp2   = __lasx_xvpermi_q(temp0, temp0, 0x11);
    const_1 = __lasx_xvpermi_d(const_1, 0xD8);
    const_2 = __lasx_xvpermi_d(const_2, 0xD8);
    t1      = __lasx_xvdp2add_w_h(const_64, temp1, const_1);
    t2      = __lasx_xvdp2_w_h(temp2, const_2);
    in0     = __lasx_xvadd_w(t1, t2);
    in1     = __lasx_xvsub_w(t1, t2);
    LASX_SRAI_W_2(in0, in1, in0, in1, 7);
    temp0   = __lasx_xvshuf4i_w(in0, 0x9C);
    temp1   = __lasx_xvshuf4i_w(in1, 0x9C);

    in0     = __lasx_xvldrepl_w(dest, 0);
    in1     = __lasx_xvldrepl_w(dst, 0);
    dst    += stride;
    in2     = __lasx_xvldrepl_w(dst, 0);
    dst    += stride;
    in3     = __lasx_xvldrepl_w(dst, 0);
    temp2   = __lasx_xvilvl_w(in2, in0);
    temp2   = __lasx_vext2xv_wu_bu(temp2);
    temp3   = __lasx_xvilvl_w(in1, in3);
    temp3   = __lasx_vext2xv_wu_bu(temp3);
    temp0   = __lasx_xvadd_w(temp0, temp2);
    temp1   = __lasx_xvadd_w(temp1, temp3);
    LASX_CLIP_W_0_255_2(temp0, temp1, temp0, temp1);
    temp1   = __lasx_xvpickev_h(temp1, temp0);
    temp0   = __lasx_xvpickev_b(temp1, temp1);
    LASX_ST_W_4(temp0, 0, 5, 4, 1, dest, stride);
}
