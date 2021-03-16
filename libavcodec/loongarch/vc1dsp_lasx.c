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
    t2 = __lasx_xvreplgr2vr_w(con_4);
    LASX_DP2ADD_W_H(t2, temp0, const_1, t1);
    LASX_DP2ADD_W_H(t2, temp0, const_2, t2);
    LASX_DP2_W_H(temp1, const_3, t3);
    LASX_DP2_W_H(temp1, const_4, t4);

    t5 = __lasx_xvadd_w(t1, t3);
    t6 = __lasx_xvadd_w(t2, t4);
    t7 = __lasx_xvsub_w(t2, t4);
    t8 = __lasx_xvsub_w(t1, t3);

    LASX_ILVH_H_2_128SV(in1, in0, in3, in2, temp0, temp1);
    LASX_DP2_W_H(const_5, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_6, t1);
    LASX_DP2_W_H(const_7, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_8, t2);
    LASX_DP2_W_H(const_9, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_10, t3);
    LASX_DP2_W_H(const_11, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_12, t4);

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
    t3    = __lasx_xvreplgr2vr_w(con_64);
    LASX_DP2ADD_W_H(t3, temp0, const_1, t1);
    LASX_DP2ADD_W_H(t3, temp0, const_2, t2);
    LASX_DP2_W_H(temp1, const_3, t3);
    LASX_DP2_W_H(temp1, const_4, t4);

    t5    = __lasx_xvadd_w(t1, t3);
    t6    = __lasx_xvadd_w(t2, t4);
    t7    = __lasx_xvsub_w(t2, t4);
    t8    = __lasx_xvsub_w(t1, t3);

    LASX_ILVH_H_2_128SV(in2, in0, in3, in1, temp0, temp1);
    LASX_DP2_W_H(const_5, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_6, t1);
    LASX_DP2_W_H(const_7, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_8, t2);
    LASX_DP2_W_H(const_9, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_10, t3);
    LASX_DP2_W_H(const_11, temp0, temp2);
    LASX_DP2ADD_W_H(temp2, temp1, const_12, t4);

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

    const_dc = __lasx_xvreplgr2vr_h(dc);
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

    LASX_DP2_W_H(temp2, const_1, temp0);
    LASX_DP2_W_H(temp2, const_2, temp1);
    t1    = __lasx_xvadd_w(temp0, const_7);
    t2    = __lasx_xvadd_w(temp1, const_7);
    temp0 = __lasx_xvpickev_w(t2, t1);
    temp1 = __lasx_xvpickod_w(t2, t1);
    t3    = __lasx_xvadd_w(temp0, temp1);
    t4    = __lasx_xvsub_w(temp0, temp1);
    t4    = __lasx_xvpermi_d(t4, 0xB1);

    LASX_DP4_D_H(temp3, const_3, t1);
    LASX_DP4_D_H(temp3, const_4, t2);
    LASX_DP4_D_H(temp3, const_5, temp0);
    LASX_DP4_D_H(temp3, const_6, temp1);

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
    LASX_DP2ADD_W_H(const_64, temp0, const_8, t1);
    LASX_DP2ADD_W_H(const_64, temp0, const_9, t2);
    LASX_DP2_W_H(temp1, const_10, t3);
    LASX_DP2_W_H(temp1, const_11, t4);
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
    const_dc = __lasx_xvreplgr2vr_h(dc);

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
    const_dc = __lasx_xvreplgr2vr_h(dc);

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

    LASX_DP2ADD_W_H(const_5, temp0, const_1, t1);
    LASX_DP2ADD_W_H(const_5, temp0, const_2, t2);
    LASX_DP2_W_H(temp1, const_3, t3);
    LASX_DP2_W_H(temp1, const_4, t4);

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
    LASX_DP2ADD_W_H(const_6, temp2, const_7, t1);
    LASX_DP2_W_H(temp3, const_8, t2);
    t3    = __lasx_xvadd_w(t1, t2);
    t4    = __lasx_xvsub_w(t1, t2);
    t4    = __lasx_xvpermi_d(t4, 0x4E);

    LASX_DP2_W_H(temp1, const_9, t1);
    LASX_DP2_W_H(temp1, const_10, t2);
    LASX_DP2_W_H(temp1, const_11, temp2);
    LASX_DP2_W_H(temp1, const_12, temp3);

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
    const_dc = __lasx_xvreplgr2vr_h(dc);

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
    LASX_DP2_W_H(temp1, const_1, t1);
    LASX_DP2_W_H(temp2, const_2, t2);
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
    LASX_DP2ADD_W_H(const_64, temp1, const_1, t1);
    LASX_DP2_W_H(temp2, const_2, t2);
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

static void put_vc1_mspel_mc_h_v_lasx(uint8_t *dst, const uint8_t *src,
                                      ptrdiff_t stride, int hmode, int vmode,
                                      int rnd)
{
    __m256i in0, in1, in2, in3;
    __m256i t0, t1, t2, t3, t4, t5, t6, t7;
    __m256i temp0, temp1, const_para1_2, const_para0_3;
    __m256i const_r, const_sh;
    __m256i sh = {0x0000000400000000, 0x0000000500000001, 0x0000000600000002, 0x0000000700000003};
    static const uint8_t para_value[][4] = {{4, 3, 53, 18},
                                            {1, 1, 9, 9},
                                            {3, 4, 18, 53}};
    static const int shift_value[] = {0, 5, 1, 5};
    int shift = (shift_value[hmode] + shift_value[vmode]) >> 1;
    int r     = (1 << (shift - 1)) + rnd - 1;
    const uint8_t *para_v = para_value[vmode - 1];

    const_r  = __lasx_xvreplgr2vr_h(r);
    const_sh = __lasx_xvreplgr2vr_h(shift);
    src -= 1, src -= stride;
    const_para0_3 = __lasx_xvldrepl_h(para_v, 0);
    const_para1_2 = __lasx_xvldrepl_h(para_v, 2);
    LASX_LD_4(src, stride, in0, in1, in2, in3);
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    in3   = __lasx_xvpermi_d(in3, 0xD8);
    LASX_ILVL_B_2_128SV(in2, in1, in3, in0, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t0);
    LASX_DP2SUB_H_BU(t0, temp1, const_para0_3, t0);
    src  += (stride << 2);
    in0   = LASX_LD(src);
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    LASX_ILVL_B_2_128SV(in3, in2, in0, in1, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t1);
    LASX_DP2SUB_H_BU(t1, temp1, const_para0_3, t1);
    src  += stride;
    in1   = LASX_LD(src);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    LASX_ILVL_B_2_128SV(in0, in3, in1, in2, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t2);
    LASX_DP2SUB_H_BU(t2, temp1, const_para0_3, t2);
    src  += stride;
    in2   = LASX_LD(src);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    LASX_ILVL_B_2_128SV(in1, in0, in2, in3, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t3);
    LASX_DP2SUB_H_BU(t3, temp1, const_para0_3, t3);
    src  += stride;
    in3   = LASX_LD(src);
    in3   = __lasx_xvpermi_d(in3, 0xD8);
    LASX_ILVL_B_2_128SV(in2, in1, in3, in0, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t4);
    LASX_DP2SUB_H_BU(t4, temp1, const_para0_3, t4);
    src  += stride;
    in0   = LASX_LD(src);
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    LASX_ILVL_B_2_128SV(in3, in2, in0, in1, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t5);
    LASX_DP2SUB_H_BU(t5, temp1, const_para0_3, t5);
    src  += stride;
    in1   = LASX_LD(src);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    LASX_ILVL_B_2_128SV(in0, in3, in1, in2, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t6);
    LASX_DP2SUB_H_BU(t6, temp1, const_para0_3, t6);
    src  += stride;
    in2   = LASX_LD(src);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    LASX_ILVL_B_2_128SV(in1, in0, in2, in3, temp0, temp1);
    LASX_DP2_H_BU(temp0, const_para1_2, t7);
    LASX_DP2SUB_H_BU(t7, temp1, const_para0_3, t7);
    LASX_ADD_H_8(t0, const_r, t1, const_r, t2, const_r, t3, const_r,
                 t4, const_r, t5, const_r, t6, const_r, t7, const_r,
                 t0, t1, t2, t3, t4, t5, t6, t7);
    t0    = __lasx_xvsra_h(t0, const_sh);
    t1    = __lasx_xvsra_h(t1, const_sh);
    t2    = __lasx_xvsra_h(t2, const_sh);
    t3    = __lasx_xvsra_h(t3, const_sh);
    t4    = __lasx_xvsra_h(t4, const_sh);
    t5    = __lasx_xvsra_h(t5, const_sh);
    t6    = __lasx_xvsra_h(t6, const_sh);
    t7    = __lasx_xvsra_h(t7, const_sh);
    LASX_TRANSPOSE8x8_H_128SV(t0, t1, t2, t3, t4, t5, t6, t7,
                              t0, t1, t2, t3, t4, t5, t6, t7);
    para_v  = para_value[hmode - 1];
    const_para0_3 = __lasx_xvldrepl_h(para_v, 0);
    const_para1_2 = __lasx_xvldrepl_h(para_v, 2);
    const_para0_3 = __lasx_vext2xv_h_b(const_para0_3);
    const_para1_2 = __lasx_vext2xv_h_b(const_para1_2);
    r       = 64 - rnd;
    const_r = __lasx_xvreplgr2vr_w(r);
    in0     = __lasx_xvpermi_d(t0, 0x72);
    in1     = __lasx_xvpermi_d(t1, 0x72);
    in2     = __lasx_xvpermi_d(t2, 0x72);
    t0      = __lasx_xvpermi_d(t0, 0xD8);
    t1      = __lasx_xvpermi_d(t1, 0xD8);
    t2      = __lasx_xvpermi_d(t2, 0xD8);
    t3      = __lasx_xvpermi_d(t3, 0xD8);
    t4      = __lasx_xvpermi_d(t4, 0xD8);
    t5      = __lasx_xvpermi_d(t5, 0xD8);
    t6      = __lasx_xvpermi_d(t6, 0xD8);
    t7      = __lasx_xvpermi_d(t7, 0xD8);
    LASX_ILVL_H_2_128SV(t2, t1, t3, t0, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t0);
    LASX_DP2SUB_W_H(t0, temp1, const_para0_3, t0);
    LASX_ILVL_H_2_128SV(t3, t2, t4, t1, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t1);
    LASX_DP2SUB_W_H(t1, temp1, const_para0_3, t1);
    LASX_ILVL_H_2_128SV(t4, t3, t5, t2, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t2);
    LASX_DP2SUB_W_H(t2, temp1, const_para0_3, t2);
    LASX_ILVL_H_2_128SV(t5, t4, t6, t3, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t3);
    LASX_DP2SUB_W_H(t3, temp1, const_para0_3, t3);
    LASX_ILVL_H_2_128SV(t6, t5, t7, t4, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t4);
    LASX_DP2SUB_W_H(t4, temp1, const_para0_3, t4);
    LASX_ILVL_H_2_128SV(t7, t6, in0, t5, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t5);
    LASX_DP2SUB_W_H(t5, temp1, const_para0_3, t5);
    LASX_ILVL_H_2_128SV(in0, t7, in1, t6, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t6);
    LASX_DP2SUB_W_H(t6, temp1, const_para0_3, t6);
    LASX_ILVL_H_2_128SV(in1, in0, in2, t7, temp0, temp1);
    LASX_DP2_W_H(temp0, const_para1_2, t7);
    LASX_DP2SUB_W_H(t7, temp1, const_para0_3, t7);
    t0    = __lasx_xvadd_w(t0, const_r);
    t1    = __lasx_xvadd_w(t1, const_r);
    t2    = __lasx_xvadd_w(t2, const_r);
    t3    = __lasx_xvadd_w(t3, const_r);
    t4    = __lasx_xvadd_w(t4, const_r);
    t5    = __lasx_xvadd_w(t5, const_r);
    t6    = __lasx_xvadd_w(t6, const_r);
    t7    = __lasx_xvadd_w(t7, const_r);
    LASX_SRAI_W_8(t0, t1, t2, t3, t4, t5, t6, t7,
                  t0, t1, t2, t3, t4, t5, t6, t7, 7);
    LASX_TRANSPOSE8x8_W(t0, t1, t2, t3, t4, t5, t6, t7,
                        t0, t1, t2, t3, t4, t5, t6, t7);
    LASX_CLIP_W_0_255_4(t0, t1, t2, t3, t0, t1, t2, t3);
    LASX_CLIP_W_0_255_4(t4, t5, t6, t7, t4, t5, t6, t7);
    LASX_PCKEV_H_4_128SV(t1, t0, t3, t2, t5, t4, t7, t6,
                         t0, t1, t2, t3);
    LASX_PCKEV_B_2_128SV(t1, t0, t3, t2, t0, t1);
    t0 = __lasx_xvperm_w(t0, sh);
    t1 = __lasx_xvperm_w(t1, sh);
    LASX_ST_D_4(t0, 0, 1, 2, 3, dst, stride);
    dst += (stride << 2);
    LASX_ST_D_4(t1, 0, 1, 2, 3, dst, stride);
}

#define PUT_VC1_MSPEL_MC_LASX(hmode, vmode)                                   \
void ff_put_vc1_mspel_mc ## hmode ## vmode ## _lasx(uint8_t *dst,             \
                                                const uint8_t *src,           \
                                                ptrdiff_t stride, int rnd)    \
{                                                                             \
    put_vc1_mspel_mc_h_v_lasx(dst, src, stride, hmode, vmode, rnd);           \
}                                                                             \
void ff_put_vc1_mspel_mc ## hmode ## vmode ## _16_lasx(uint8_t *dst,          \
                                                   const uint8_t *src,        \
                                                   ptrdiff_t stride, int rnd) \
{                                                                             \
    put_vc1_mspel_mc_h_v_lasx(dst, src, stride, hmode, vmode, rnd);           \
    put_vc1_mspel_mc_h_v_lasx(dst + 8, src + 8, stride, hmode, vmode, rnd);   \
    dst += 8 * stride, src += 8 * stride;                                     \
    put_vc1_mspel_mc_h_v_lasx(dst, src, stride, hmode, vmode, rnd);           \
    put_vc1_mspel_mc_h_v_lasx(dst + 8, src + 8, stride, hmode, vmode, rnd);   \
}

PUT_VC1_MSPEL_MC_LASX(1, 1);
PUT_VC1_MSPEL_MC_LASX(1, 2);
PUT_VC1_MSPEL_MC_LASX(1, 3);

PUT_VC1_MSPEL_MC_LASX(2, 1);
PUT_VC1_MSPEL_MC_LASX(2, 2);
PUT_VC1_MSPEL_MC_LASX(2, 3);

PUT_VC1_MSPEL_MC_LASX(3, 1);
PUT_VC1_MSPEL_MC_LASX(3, 2);
PUT_VC1_MSPEL_MC_LASX(3, 3);

void ff_put_no_rnd_vc1_chroma_mc8_lasx(uint8_t *dst /* align 8 */,
                                       uint8_t *src /* align 1 */,
                                       ptrdiff_t stride, int h, int x, int y)
{
    const int intA = (8 - x) * (8 - y);
    const int intB =     (x) * (8 - y);
    const int intC = (8 - x) *     (y);
    const int intD =     (x) *     (y);
    __m256i src00, src01, src10, src11;
    __m256i A, B, C, D;
    int i;

    av_assert2(x < 8 && y < 8 && x >= 0 && y >= 0);

    A = __lasx_xvreplgr2vr_h(intA);
    B = __lasx_xvreplgr2vr_h(intB);
    C = __lasx_xvreplgr2vr_h(intC);
    D = __lasx_xvreplgr2vr_h(intD);
    for(i = 0; i < h; i++){
        LASX_LD_2(src, 1, src00, src01);
        src += stride;
        LASX_LD_2(src, 1, src10, src11);

        LASX_UNPCK_L_HU_BU_4(src00, src01, src10, src11,
                             src00, src01, src10, src11);
        src00 = __lasx_xvmul_h(src00, A);
        src01 = __lasx_xvmul_h(src01, B);
        src10 = __lasx_xvmul_h(src10, C);
        src11 = __lasx_xvmul_h(src11, D);
        src00 = __lasx_xvadd_h(src00, src01);
        src10 = __lasx_xvadd_h(src10, src11);
        src00 = __lasx_xvadd_h(src00, src10);
        src00 = __lasx_xvaddi_hu(src00, 28);
        src00 = __lasx_xvsrli_h(src00, 6);
        LASX_PCKEV_B_128SV(src00, src00, src00);
        LASX_ST_D(src00, 0, dst);
        dst += stride;
    }
}

static void put_vc1_mspel_mc_v_lasx(uint8_t *dst, const uint8_t *src,
                                    ptrdiff_t stride, int vmode, int rnd)
{
    __m256i in0, in1, in2, in3, temp0, temp1, t0;
    __m256i const_para0_3, const_para1_2, const_r, const_sh;
    static const uint16_t para_value[][2] = {{0x0304, 0x1235},
                                            {0x0101, 0x0909},
                                            {0x0403, 0x3512}};
    const uint16_t *para_v = para_value[vmode - 1];
    static const int shift_value[] = {0, 6, 4, 6};
    static int add_value[3];
    ptrdiff_t stride_2x = stride << 1;
    int i = 0;
    add_value[2] = add_value[0] = 31 + rnd, add_value[1] = 7 + rnd;

    const_r  = __lasx_xvreplgr2vr_h(add_value[vmode - 1]);
    const_sh = __lasx_xvreplgr2vr_h(shift_value[vmode]);
    const_para0_3 = __lasx_xvreplgr2vr_h(*para_v);
    const_para1_2 = __lasx_xvreplgr2vr_h(*(para_v + 1));

    LASX_LD_2((src - stride), stride, in0, in1);
    in2 = LASX_LD(src + stride);
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    for (; i < 16; i++) {
        in3 = LASX_LD(src + stride_2x);
        in3 = __lasx_xvpermi_d(in3, 0xD8);
        LASX_ILVL_B_2_128SV(in2, in1, in3, in0, temp0, temp1);
        LASX_DP2_H_BU(temp0, const_para1_2, t0);
        LASX_DP2SUB_H_BU(t0, temp1, const_para0_3, t0);
        LASX_ADD_H(t0, const_r, t0);
        t0 = __lasx_xvsra_h(t0, const_sh);
        LASX_CLIP_H_0_255(t0, t0);
        LASX_PCKEV_B_128SV(t0, t0, t0);
        LASX_ST_D_2(t0, 0, 2, dst, 8);
        dst += stride;
        src += stride;
        in0 = in1;
        in1 = in2;
        in2 = in3;
    }
}

#define PUT_VC1_MSPEL_MC_V_LASX(vmode)                                    \
void ff_put_vc1_mspel_mc0 ## vmode ## _16_lasx(uint8_t *dst,              \
                                               const uint8_t *src,        \
                                               ptrdiff_t stride, int rnd) \
{                                                                         \
    put_vc1_mspel_mc_v_lasx(dst, src, stride, vmode, rnd);                \
}

PUT_VC1_MSPEL_MC_V_LASX(1);
PUT_VC1_MSPEL_MC_V_LASX(2);
PUT_VC1_MSPEL_MC_V_LASX(3);

#define ROW_LASX(in0, in1, in2, in3, out0)                   \
    LASX_ILVL_B_2_128SV(in2, in1, in3, in0, tmp0_m, tmp1_m); \
    LASX_DP2_H_BU(tmp0_m, const_para1_2, out0);              \
    LASX_DP2SUB_H_BU(out0, tmp1_m, const_para0_3, out0);     \
    LASX_ADD_H(out0, const_r, out0);                         \
    out0 = __lasx_xvsra_h(out0, const_sh);                   \
    LASX_CLIP_H_0_255(out0, out0);                           \
    LASX_PCKEV_B(out0, out0, out0);

static void put_vc1_mspel_mc_h_lasx(uint8_t *dst, const uint8_t *src,
                                    ptrdiff_t stride, int hmode, int rnd)
{
    __m256i in0, in1, in2, in3, in4, in5, in6, in7,
            in8, in9, in10, in11, in12, in13, in14, in15;
    __m256i out0, out1, out2, out3, out4, out5, out6, out7, out8, out9,
            out10, out11, out12, out13, out14, out15, out16, out17, out18;
    __m256i const_para0_3, const_para1_2, const_r, const_sh;
    __m256i tmp0_m, tmp1_m, tmp2_m, tmp3_m;
    __m256i tmp4_m, tmp5_m, tmp6_m, tmp7_m;
    __m256i t0, t1, t2, t3, t4, t5, t6, t7;
    static const uint16_t para_value[][2] = {{0x0304, 0x1235},
                                            {0x0101, 0x0909},
                                            {0x0403, 0x3512}};
    const uint16_t *para_v = para_value[hmode - 1];
    static const int shift_value[] = {0, 6, 4, 6};
    static int add_value[3];
    add_value[2] = add_value[0] = 32 - rnd, add_value[1] = 8 - rnd;

    const_r  = __lasx_xvreplgr2vr_h(add_value[hmode - 1]);
    const_sh = __lasx_xvreplgr2vr_h(shift_value[hmode]);
    const_para0_3 = __lasx_xvreplgr2vr_h(*para_v);
    const_para1_2 = __lasx_xvreplgr2vr_h(*(para_v + 1));
    src -= 1;

    LASX_LD_8(src, stride, in0, in1, in2, in3, in4, in5, in6, in7);
    src += stride << 3;
    LASX_LD_8(src, stride, in8, in9, in10, in11, in12, in13, in14, in15);
    LASX_ILVL_B_8_128SV(in2, in0, in3, in1, in6, in4, in7, in5,
                        in10, in8, in11, in9, in14, in12, in15, in13,
                        tmp0_m, tmp1_m, tmp2_m, tmp3_m,
                        tmp4_m, tmp5_m, tmp6_m, tmp7_m);
    LASX_ILVLH_B_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, t1, t0, t3, t2);
    LASX_ILVLH_B_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, t5, t4, t7, t6);
    LASX_ILVLH_W_2_128SV(t2, t0, t3, t1, tmp2_m, tmp0_m, tmp6_m, tmp4_m);
    LASX_ILVLH_W_2_128SV(t6, t4, t7, t5, tmp3_m, tmp1_m, tmp7_m, tmp5_m);
    LASX_ILVLH_D_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, out1, out0, out3, out2);
    LASX_ILVLH_D_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, out5, out4, out7, out6);

    LASX_ILVH_B_8_128SV(in2, in0, in3, in1, in6, in4, in7, in5,
                        in10, in8, in11, in9, in14, in12, in15, in13,
                        tmp0_m, tmp1_m, tmp2_m, tmp3_m,
                        tmp4_m, tmp5_m, tmp6_m, tmp7_m);
    LASX_ILVLH_B_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, t1, t0, t3, t2);
    LASX_ILVLH_B_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, t5, t4, t7, t6);
    LASX_ILVLH_W_2_128SV(t2, t0, t3, t1, tmp2_m, tmp0_m, tmp6_m, tmp4_m);
    LASX_ILVLH_W_2_128SV(t6, t4, t7, t5, tmp3_m, tmp1_m, tmp7_m, tmp5_m);
    LASX_ILVLH_D_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, out9, out8, out11, out10);
    LASX_ILVLH_D_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m,
                         out13, out12, out15, out14);
    LASX_PCKOD_Q_2(out0, out0, out1, out1, out16, out17);
    LASX_PCKOD_Q(out2, out2, out18);

    out0  = __lasx_xvpermi_d(out0, 0xD8);
    out1  = __lasx_xvpermi_d(out1, 0xD8);
    out2  = __lasx_xvpermi_d(out2, 0xD8);
    out3  = __lasx_xvpermi_d(out3, 0xD8);
    out4  = __lasx_xvpermi_d(out4, 0xD8);
    out5  = __lasx_xvpermi_d(out5, 0xD8);
    out6  = __lasx_xvpermi_d(out6, 0xD8);
    out7  = __lasx_xvpermi_d(out7, 0xD8);
    out8  = __lasx_xvpermi_d(out8, 0xD8);
    out9  = __lasx_xvpermi_d(out9, 0xD8);
    out10 = __lasx_xvpermi_d(out10, 0xD8);
    out11 = __lasx_xvpermi_d(out11, 0xD8);
    out12 = __lasx_xvpermi_d(out12, 0xD8);
    out13 = __lasx_xvpermi_d(out13, 0xD8);
    out14 = __lasx_xvpermi_d(out14, 0xD8);
    out15 = __lasx_xvpermi_d(out15, 0xD8);
    out16 = __lasx_xvpermi_d(out16, 0xD8);
    out17 = __lasx_xvpermi_d(out17, 0xD8);
    out18 = __lasx_xvpermi_d(out18, 0xD8);

    ROW_LASX(out0,  out1,  out2,  out3,  in0);
    ROW_LASX(out1,  out2,  out3,  out4,  in1);
    ROW_LASX(out2,  out3,  out4,  out5,  in2);
    ROW_LASX(out3,  out4,  out5,  out6,  in3);
    ROW_LASX(out4,  out5,  out6,  out7,  in4);
    ROW_LASX(out5,  out6,  out7,  out8,  in5);
    ROW_LASX(out6,  out7,  out8,  out9,  in6);
    ROW_LASX(out7,  out8,  out9,  out10, in7);
    ROW_LASX(out8,  out9,  out10, out11, in8);
    ROW_LASX(out9,  out10, out11, out12, in9);
    ROW_LASX(out10, out11, out12, out13, in10);
    ROW_LASX(out11, out12, out13, out14, in11);
    ROW_LASX(out12, out13, out14, out15, in12);
    ROW_LASX(out13, out14, out15, out16, in13);
    ROW_LASX(out14, out15, out16, out17, in14);
    ROW_LASX(out15, out16, out17, out18, in15);

    LASX_ILVL_B_8_128SV(in2, in0, in3, in1, in6, in4, in7, in5,
                        in10, in8, in11, in9, in14, in12, in15, in13,
                        tmp0_m, tmp1_m, tmp2_m, tmp3_m,
                        tmp4_m, tmp5_m, tmp6_m, tmp7_m);
    LASX_ILVLH_B_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, t1, t0, t3, t2);
    LASX_ILVLH_B_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, t5, t4, t7, t6);
    LASX_ILVLH_W_2_128SV(t2, t0, t3, t1, tmp2_m, tmp0_m, tmp6_m, tmp4_m);
    LASX_ILVLH_W_2_128SV(t6, t4, t7, t5, tmp3_m, tmp1_m, tmp7_m, tmp5_m);
    LASX_ILVLH_D_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, out1, out0, out3, out2);
    LASX_ILVLH_D_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, out5, out4, out7, out6);

    LASX_ILVH_B_8_128SV(in2, in0, in3, in1, in6, in4, in7, in5,
                        in10, in8, in11, in9, in14, in12, in15, in13,
                        tmp0_m, tmp1_m, tmp2_m, tmp3_m,
                        tmp4_m, tmp5_m, tmp6_m, tmp7_m);
    LASX_ILVLH_B_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, t1, t0, t3, t2);
    LASX_ILVLH_B_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m, t5, t4, t7, t6);
    LASX_ILVLH_W_2_128SV(t2, t0, t3, t1, tmp2_m, tmp0_m, tmp6_m, tmp4_m);
    LASX_ILVLH_W_2_128SV(t6, t4, t7, t5, tmp3_m, tmp1_m, tmp7_m, tmp5_m);
    LASX_ILVLH_D_2_128SV(tmp1_m, tmp0_m, tmp3_m, tmp2_m, out9, out8, out11, out10);
    LASX_ILVLH_D_2_128SV(tmp5_m, tmp4_m, tmp7_m, tmp6_m,
                         out13, out12, out15, out14);
    LASX_ST_D_2(out0, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out1, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out2, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out3, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out4, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out5, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out6, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out7, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out8, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out9, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out10, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out11, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out12, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out13, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out14, 0, 1, dst, 8);
    dst += stride;
    LASX_ST_D_2(out15, 0, 1, dst, 8);
}

#define PUT_VC1_MSPEL_MC_H_LASX(hmode)                                    \
void ff_put_vc1_mspel_mc ## hmode ## 0_16_lasx(uint8_t *dst,              \
                                               const uint8_t *src,        \
                                               ptrdiff_t stride, int rnd) \
{                                                                         \
    put_vc1_mspel_mc_h_lasx(dst, src, stride, hmode, rnd);                \
}

PUT_VC1_MSPEL_MC_H_LASX(1);
PUT_VC1_MSPEL_MC_H_LASX(2);
PUT_VC1_MSPEL_MC_H_LASX(3);
