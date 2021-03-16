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

#include "libavutil/loongarch/generic_macros_lasx.h"
#include "idctdsp_loongarch.h"

#define LASX_TRANSPOSE4x16(in_0, in_1, in_2, in_3, out_0, out_1, out_2, out_3)  \
{                                                                               \
    __m256i temp_0, temp_1, temp_2, temp_3;                                     \
    __m256i temp_4, temp_5, temp_6, temp_7;                                     \
    temp_0 = __lasx_xvpermi_q(in_2, in_0, 0x20);                                \
    temp_1 = __lasx_xvpermi_q(in_2, in_0, 0x31);                                \
    temp_2 = __lasx_xvpermi_q(in_3, in_1, 0x20);                                \
    temp_3 = __lasx_xvpermi_q(in_3, in_1, 0x31);                                \
    LASX_ILVLH_H_128SV(temp_1, temp_0, temp_5, temp_4);                         \
    LASX_ILVLH_H_128SV(temp_3, temp_2, temp_7, temp_6);                         \
    LASX_ILVLH_W_128SV(temp_6, temp_4, out_1, out_0);                           \
    LASX_ILVLH_W_128SV(temp_7, temp_5, out_3, out_2);                           \
}

#define LASX_IDCTROWCONDDC                                                      \
    const_val  = 16383 * ((1 << 19) / 16383);                                   \
    const_val1 = __lasx_xvinsgr2vr_w(const_val0, const_val, 0);                 \
    const_val1 = __lasx_xvreplve0_w(const_val1);                                \
    LASX_LD_4(block, 16, in0, in1, in2, in3);                                   \
    LASX_TRANSPOSE4x16(in0, in1, in2, in3, in0, in1, in2, in3);                 \
    a0 = __lasx_xvpermi_d(in0, 0xD8);                                           \
    a0 = __lasx_vext2xv_w_h(a0);                                                \
    temp  = __lasx_xvslli_w(a0, 3);                                             \
    a1 = __lasx_xvpermi_d(in0, 0x8D);                                           \
    a1 = __lasx_vext2xv_w_h(a1);                                                \
    a2 = __lasx_xvpermi_d(in1, 0xD8);                                           \
    a2 = __lasx_vext2xv_w_h(a2);                                                \
    a3 = __lasx_xvpermi_d(in1, 0x8D);                                           \
    a3 = __lasx_vext2xv_w_h(a3);                                                \
    b0 = __lasx_xvpermi_d(in2, 0xD8);                                           \
    b0 = __lasx_vext2xv_w_h(b0);                                                \
    b1 = __lasx_xvpermi_d(in2, 0x8D);                                           \
    b1 = __lasx_vext2xv_w_h(b1);                                                \
    b2 = __lasx_xvpermi_d(in3, 0xD8);                                           \
    b2 = __lasx_vext2xv_w_h(b2);                                                \
    b3 = __lasx_xvpermi_d(in3, 0x8D);                                           \
    b3 = __lasx_vext2xv_w_h(b3);                                                \
    select_vec = a0 | a1 | a2 | a3 | b0 | b1 | b2 | b3;                         \
    select_vec = __lasx_xvslti_wu(select_vec, 1);                               \
                                                                                \
    w2    = __lasx_xvrepl128vei_h(w1, 2);                                       \
    w3    = __lasx_xvrepl128vei_h(w1, 3);                                       \
    w4    = __lasx_xvrepl128vei_h(w1, 4);                                       \
    w5    = __lasx_xvrepl128vei_h(w1, 5);                                       \
    w6    = __lasx_xvrepl128vei_h(w1, 6);                                       \
    w7    = __lasx_xvrepl128vei_h(w1, 7);                                       \
    w1    = __lasx_xvrepl128vei_h(w1, 1);                                       \
                                                                                \
    /* part of FUNC6(idctRowCondDC) */                                          \
    LASX_MADDWL_W_H_128SV(const_val0, in0, w4, temp0);                          \
    LASX_MULWL_W_H_2_128SV(in1, w2, in1, w6, temp1, temp2);                     \
    a0    = __lasx_xvadd_w(temp0, temp1);                                       \
    a1    = __lasx_xvadd_w(temp0, temp2);                                       \
    a2    = __lasx_xvsub_w(temp0, temp2);                                       \
    a3    = __lasx_xvsub_w(temp0, temp1);                                       \
                                                                                \
    LASX_ILVH_H_2_128SV(in1, in0, w3, w1, temp0, temp1);                        \
    LASX_DP2_W_H(temp0, temp1, b0);                                             \
    temp1 = __lasx_xvneg_h(w7);                                                 \
    LASX_ILVL_H_128SV(temp1, w3, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b1);                                             \
    temp1 = __lasx_xvneg_h(w1);                                                 \
    LASX_ILVL_H_128SV(temp1, w5, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b2);                                             \
    temp1 = __lasx_xvneg_h(w5);                                                 \
    LASX_ILVL_H_128SV(temp1, w7, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b3);                                             \
                                                                                \
    /* if (AV_RAN64A(row + 4)) */                                               \
    LASX_ILVL_H_2_128SV(in3, in2, w6, w4, temp0, temp1);                        \
    LASX_DP2ADD_W_H(a0, temp0, temp1, a0);                                      \
    LASX_ILVL_H_128SV(w2, w4, temp1);                                           \
    LASX_DP2SUB_W_H(a1, temp0, temp1, a1);                                      \
    temp1 = __lasx_xvneg_h(w4);                                                 \
    LASX_ILVL_H_128SV(w2, temp1, temp2);                                        \
    LASX_DP2ADD_W_H(a2, temp0, temp2, a2);                                      \
    temp1 = __lasx_xvneg_h(w6);                                                 \
    LASX_ILVL_H_128SV(temp1, w4, temp2);                                        \
    LASX_DP2ADD_W_H(a3, temp0, temp2, a3);                                      \
                                                                                \
    LASX_ILVH_H_2_128SV(in3, in2, w7, w5, temp0, temp1);                        \
    LASX_DP2ADD_W_H(b0, temp0, temp1, b0);                                      \
    LASX_ILVL_H_2_128SV(w5, w1, w3, w7, temp1, temp2);                          \
    LASX_DP2SUB_W_H(b1, temp0, temp1, b1);                                      \
    LASX_DP2ADD_W_H(b2, temp0, temp2, b2);                                      \
    temp1 = __lasx_xvneg_h(w1);                                                 \
    LASX_ILVL_H_128SV(temp1, w3, temp2);                                        \
    LASX_DP2ADD_W_H(b3, temp0, temp2, b3);                                      \
                                                                                \
    temp0 = __lasx_xvadd_w(a0, b0);                                             \
    temp1 = __lasx_xvadd_w(a1, b1);                                             \
    temp2 = __lasx_xvadd_w(a2, b2);                                             \
    temp3 = __lasx_xvadd_w(a3, b3);                                             \
    a0    = __lasx_xvsub_w(a0, b0);                                             \
    a1    = __lasx_xvsub_w(a1, b1);                                             \
    a2    = __lasx_xvsub_w(a2, b2);                                             \
    a3    = __lasx_xvsub_w(a3, b3);                                             \
    LASX_SRAI_W_8(temp0, temp1, temp2, temp3, a0, a1, a2, a3,                   \
                  temp0, temp1, temp2, temp3, a0, a1, a2, a3, 11);              \
    in0   = __lasx_xvbitsel_v(temp0, temp, select_vec);                         \
    in1   = __lasx_xvbitsel_v(temp1, temp, select_vec);                         \
    in2   = __lasx_xvbitsel_v(temp2, temp, select_vec);                         \
    in3   = __lasx_xvbitsel_v(temp3, temp, select_vec);                         \
    a0    = __lasx_xvbitsel_v(a0, temp, select_vec);                            \
    a1    = __lasx_xvbitsel_v(a1, temp, select_vec);                            \
    a2    = __lasx_xvbitsel_v(a2, temp, select_vec);                            \
    a3    = __lasx_xvbitsel_v(a3, temp, select_vec);                            \
    in0   = __lasx_xvpickev_h(in1, in0);                                        \
    in1   = __lasx_xvpickev_h(in3, in2);                                        \
    in2   = __lasx_xvpickev_h(a2, a3);                                          \
    in3   = __lasx_xvpickev_h(a0, a1);                                          \
    in0   = __lasx_xvpermi_d(in0, 0xD8);                                        \
    in1   = __lasx_xvpermi_d(in1, 0xD8);                                        \
    in2   = __lasx_xvpermi_d(in2, 0xD8);                                        \
    in3   = __lasx_xvpermi_d(in3, 0xD8);                                        \


#define LASX_IDCTCOLS                                                           \
    /* part of FUNC6(idctSparaseCol) */                                         \
    LASX_TRANSPOSE4x16(in0, in1, in2, in3, in0, in1, in2, in3);                 \
    LASX_MADDWL_W_H_128SV(const_val1, in0, w4, temp0);                          \
    LASX_MULWL_W_H_2_128SV(in1, w2, in1, w6, temp1, temp2);                     \
    a0    = __lasx_xvadd_w(temp0, temp1);                                       \
    a1    = __lasx_xvadd_w(temp0, temp2);                                       \
    a2    = __lasx_xvsub_w(temp0, temp2);                                       \
    a3    = __lasx_xvsub_w(temp0, temp1);                                       \
                                                                                \
    LASX_ILVH_H_2_128SV(in1, in0, w3, w1, temp0, temp1);                        \
    LASX_DP2_W_H(temp0, temp1, b0);                                             \
    temp1 = __lasx_xvneg_h(w7);                                                 \
    LASX_ILVL_H_128SV(temp1, w3, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b1);                                             \
    temp1 = __lasx_xvneg_h(w1);                                                 \
    LASX_ILVL_H_128SV(temp1, w5, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b2);                                             \
    temp1 = __lasx_xvneg_h(w5);                                                 \
    LASX_ILVL_H_128SV(temp1, w7, temp2);                                        \
    LASX_DP2_W_H(temp0, temp2, b3);                                             \
                                                                                \
    /* if (AV_RAN64A(row + 4)) */                                               \
    LASX_ILVL_H_2_128SV(in3, in2, w6, w4, temp0, temp1);                        \
    LASX_DP2ADD_W_H(a0, temp0, temp1, a0);                                      \
    LASX_ILVL_H_128SV(w2, w4, temp1);                                           \
    LASX_DP2SUB_W_H(a1, temp0, temp1, a1);                                      \
    temp1 = __lasx_xvneg_h(w4);                                                 \
    LASX_ILVL_H_128SV(w2, temp1, temp2);                                        \
    LASX_DP2ADD_W_H(a2, temp0, temp2, a2);                                      \
    temp1 = __lasx_xvneg_h(w6);                                                 \
    LASX_ILVL_H_128SV(temp1, w4, temp2);                                        \
    LASX_DP2ADD_W_H(a3, temp0, temp2, a3);                                      \
                                                                                \
    LASX_ILVH_H_2_128SV(in3, in2, w7, w5, temp0, temp1);                        \
    LASX_DP2ADD_W_H(b0, temp0, temp1, b0);                                      \
    LASX_ILVL_H_2_128SV(w5, w1, w3, w7, temp1, temp2);                          \
    LASX_DP2SUB_W_H(b1, temp0, temp1, b1);                                      \
    LASX_DP2ADD_W_H(b2, temp0, temp2, b2);                                      \
    temp1 = __lasx_xvneg_h(w1);                                                 \
    LASX_ILVL_H_128SV(temp1, w3, temp2);                                        \
    LASX_DP2ADD_W_H(b3, temp0, temp2, b3);                                      \
                                                                                \
    temp0 = __lasx_xvadd_w(a0, b0);                                             \
    temp1 = __lasx_xvadd_w(a1, b1);                                             \
    temp2 = __lasx_xvadd_w(a2, b2);                                             \
    temp3 = __lasx_xvadd_w(a3, b3);                                             \
    a3    = __lasx_xvsub_w(a3, b3);                                             \
    a2    = __lasx_xvsub_w(a2, b2);                                             \
    a1    = __lasx_xvsub_w(a1, b1);                                             \
    a0    = __lasx_xvsub_w(a0, b0);                                             \
    LASX_SRAI_W_8(temp0, temp1, temp2, temp3, a0, a1, a2, a3,                   \
                  temp0, temp1, temp2, temp3, a0, a1, a2, a3, 20);              \
    in0   = __lasx_xvpickev_h(temp1, temp0);                                    \
    in1   = __lasx_xvpickev_h(temp3, temp2);                                    \
    in2   = __lasx_xvpickev_h(a2, a3);                                          \
    in3   = __lasx_xvpickev_h(a0, a1);                                          \


static void simple_idct_lasx(int16_t *block)
{
    int32_t const_val = 1 << 10;
    __m256i w1 = {0x4B42539F58C50000, 0x11A822A332493FFF, 0x4B42539F58C50000, 0x11A822A332493FFF};
    __m256i in0, in1, in2, in3;
    __m256i w2, w3, w4, w5, w6, w7;
    __m256i a0, a1, a2, a3;
    __m256i b0, b1, b2, b3;
    __m256i temp0, temp1, temp2, temp3;
    __m256i const_val0 = __lasx_xvreplgr2vr_w(const_val);
    __m256i const_val1, select_vec, temp;

    LASX_IDCTROWCONDDC
    LASX_IDCTCOLS
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    in3   = __lasx_xvpermi_d(in3, 0xD8);
    LASX_ST_4(in0, in1, in2, in3, block, 16);
}

static void simple_idct_put_lasx(uint8_t *dst, int32_t dst_stride,
                                 int16_t *block)
{
    int32_t const_val = 1 << 10;
    __m256i w1 = {0x4B42539F58C50000, 0x11A822A332493FFF, 0x4B42539F58C50000, 0x11A822A332493FFF};
    __m256i in0, in1, in2, in3;
    __m256i w2, w3, w4, w5, w6, w7;
    __m256i a0, a1, a2, a3;
    __m256i b0, b1, b2, b3;
    __m256i temp0, temp1, temp2, temp3;
    __m256i const_val0 = __lasx_xvreplgr2vr_w(const_val);
    __m256i const_val1, select_vec, temp;

    LASX_IDCTROWCONDDC
    LASX_IDCTCOLS
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    in3   = __lasx_xvpermi_d(in3, 0xD8);
    LASX_CLIP_H_0_255_4(in0, in1, in2, in3, in0, in1, in2, in3);
    LASX_PCKEV_B_2_128SV(in1, in0, in3, in2, in0, in1);
    LASX_ST_D_4(in0, 0, 2, 1, 3, dst, dst_stride);
    dst += (dst_stride << 2);
    LASX_ST_D_4(in1, 0, 2, 1, 3, dst, dst_stride);
}

static void simple_idct_add_lasx(uint8_t *dst, int32_t dst_stride,
                                 int16_t *block)
{
    int32_t const_val = 1 << 10;
    uint8_t *dst1 = dst;
    __m256i w1 = {0x4B42539F58C50000, 0x11A822A332493FFF, 0x4B42539F58C50000, 0x11A822A332493FFF};
    __m256i sh = {0x0003000200010000, 0x000B000A00090008, 0x0007000600050004, 0x000F000E000D000B};
    __m256i in0, in1, in2, in3;
    __m256i w2, w3, w4, w5, w6, w7;
    __m256i a0, a1, a2, a3;
    __m256i b0, b1, b2, b3;
    __m256i temp0, temp1, temp2, temp3;
    __m256i const_val0 = __lasx_xvreplgr2vr_w(const_val);
    __m256i const_val1, select_vec, temp;

    LASX_IDCTROWCONDDC
    LASX_IDCTCOLS
    a0    = __lasx_xvldrepl_d(dst1, 0);
    a0    = __lasx_vext2xv_hu_bu(a0);
    dst1 += dst_stride;
    a1    = __lasx_xvldrepl_d(dst1, 0);
    a1    = __lasx_vext2xv_hu_bu(a1);
    dst1 += dst_stride;
    a2    = __lasx_xvldrepl_d(dst1, 0);
    a2    = __lasx_vext2xv_hu_bu(a2);
    dst1 += dst_stride;
    a3    = __lasx_xvldrepl_d(dst1, 0);
    a3    = __lasx_vext2xv_hu_bu(a3);
    dst1 += dst_stride;
    b0    = __lasx_xvldrepl_d(dst1, 0);
    b0    = __lasx_vext2xv_hu_bu(b0);
    dst1 += dst_stride;
    b1    = __lasx_xvldrepl_d(dst1, 0);
    b1    = __lasx_vext2xv_hu_bu(b1);
    dst1 += dst_stride;
    b2    = __lasx_xvldrepl_d(dst1, 0);
    b2    = __lasx_vext2xv_hu_bu(b2);
    dst1 += dst_stride;
    b3    = __lasx_xvldrepl_d(dst1, 0);
    b3    = __lasx_vext2xv_hu_bu(b3);
    temp0 = __lasx_xvshuf_h(sh, a1, a0);
    temp1 = __lasx_xvshuf_h(sh, a3, a2);
    temp2 = __lasx_xvshuf_h(sh, b1, b0);
    temp3 = __lasx_xvshuf_h(sh, b3, b2);
    in0   = __lasx_xvadd_h(temp0, in0);
    in1   = __lasx_xvadd_h(temp1, in1);
    in2   = __lasx_xvadd_h(temp2, in2);
    in3   = __lasx_xvadd_h(temp3, in3);
    in0   = __lasx_xvpermi_d(in0, 0xD8);
    in1   = __lasx_xvpermi_d(in1, 0xD8);
    in2   = __lasx_xvpermi_d(in2, 0xD8);
    in3   = __lasx_xvpermi_d(in3, 0xD8);
    LASX_CLIP_H_0_255_4(in0, in1, in2, in3, in0, in1, in2, in3);
    LASX_PCKEV_B_2_128SV(in1, in0, in3, in2, in0, in1);
    LASX_ST_D_4(in0, 0, 2, 1, 3, dst, dst_stride);
    dst += (dst_stride << 2);
    LASX_ST_D_4(in1, 0, 2, 1, 3, dst, dst_stride);
}

void ff_simple_idct_lasx(int16_t *block)
{
    simple_idct_lasx(block);
}

void ff_simple_idct_put_lasx(uint8_t *dst, ptrdiff_t dst_stride, int16_t *block)
{
    simple_idct_put_lasx(dst, dst_stride, block);
}

void ff_simple_idct_add_lasx(uint8_t *dst, ptrdiff_t dst_stride, int16_t *block)
{
    simple_idct_add_lasx(dst, dst_stride, block);
}
