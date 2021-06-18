/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Jin Bo <jinbo@loongson.cn>
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

#include "libavcodec/vp9dsp.h"
#include "libavutil/loongarch/generic_macros_lsx.h"
#include "vp9dsp_loongarch.h"

#define VP9_DCT_CONST_BITS   14
#define ALLOC_ALIGNED(align) __attribute__ ((aligned(align)))
#define ROUND_POWER_OF_TWO(value, n) (((value) + (1 << ((n) - 1))) >> (n))

static const int32_t cospi_1_64 = 16364;
static const int32_t cospi_2_64 = 16305;
static const int32_t cospi_3_64 = 16207;
static const int32_t cospi_4_64 = 16069;
static const int32_t cospi_5_64 = 15893;
static const int32_t cospi_6_64 = 15679;
static const int32_t cospi_7_64 = 15426;
static const int32_t cospi_8_64 = 15137;
static const int32_t cospi_9_64 = 14811;
static const int32_t cospi_10_64 = 14449;
static const int32_t cospi_11_64 = 14053;
static const int32_t cospi_12_64 = 13623;
static const int32_t cospi_13_64 = 13160;
static const int32_t cospi_14_64 = 12665;
static const int32_t cospi_15_64 = 12140;
static const int32_t cospi_16_64 = 11585;
static const int32_t cospi_17_64 = 11003;
static const int32_t cospi_18_64 = 10394;
static const int32_t cospi_19_64 = 9760;
static const int32_t cospi_20_64 = 9102;
static const int32_t cospi_21_64 = 8423;
static const int32_t cospi_22_64 = 7723;
static const int32_t cospi_23_64 = 7005;
static const int32_t cospi_24_64 = 6270;
static const int32_t cospi_25_64 = 5520;
static const int32_t cospi_26_64 = 4756;
static const int32_t cospi_27_64 = 3981;
static const int32_t cospi_28_64 = 3196;
static const int32_t cospi_29_64 = 2404;
static const int32_t cospi_30_64 = 1606;
static const int32_t cospi_31_64 = 804;

static const int32_t sinpi_1_9 = 5283;
static const int32_t sinpi_2_9 = 9929;
static const int32_t sinpi_3_9 = 13377;
static const int32_t sinpi_4_9 = 15212;

#define BUTTERFLY_4(RTYPE, in0, in1, in2, in3, out0, out1, out2, out3)  \
{                                                                       \
    out0 = (__m128i)((RTYPE)in0 + (RTYPE)in3);                          \
    out1 = (__m128i)((RTYPE)in1 + (RTYPE)in2);                          \
                                                                        \
    out2 = (__m128i)((RTYPE)in1 - (RTYPE)in2);                          \
    out3 = (__m128i)((RTYPE)in0 - (RTYPE)in3);                          \
}


#define VP9_DOTP_CONST_PAIR(reg0, reg1, cnst0, cnst1, out0, out1)  \
{                                                                  \
    __m128i k0_m = __lsx_vreplgr2vr_h(cnst0);                      \
    __m128i s0_m, s1_m, s2_m, s3_m;                                \
                                                                   \
    s0_m = __lsx_vreplgr2vr_h(cnst1);                              \
    k0_m = __lsx_vpackev_h(s0_m, k0_m);                            \
                                                                   \
    s1_m = __lsx_vilvl_h(__lsx_vneg_h(reg1), reg0);                \
    s0_m = __lsx_vilvh_h(__lsx_vneg_h(reg1), reg0);                \
    s3_m = __lsx_vilvl_h(reg0, reg1);                              \
    s2_m = __lsx_vilvh_h(reg0, reg1);                              \
    LSX_DUP2_ARG2(__lsx_dp2_w_h, s1_m, k0_m, s0_m, k0_m, s1_m,     \
                  s0_m);                                           \
    LSX_DUP2_ARG2(__lsx_vsrari_w, s1_m, VP9_DCT_CONST_BITS,        \
                  s0_m, VP9_DCT_CONST_BITS, s1_m, s0_m);           \
    out0 = __lsx_vpickev_h(s0_m, s1_m);                            \
    LSX_DUP2_ARG2(__lsx_dp2_w_h, s3_m, k0_m, s2_m, k0_m, s1_m,     \
                  s0_m);                                           \
    LSX_DUP2_ARG2(__lsx_vsrari_w, s1_m, VP9_DCT_CONST_BITS,        \
                  s0_m, VP9_DCT_CONST_BITS, s1_m, s0_m);           \
    out1 = __lsx_vpickev_h(s0_m, s1_m);                            \
}

#define VP9_SET_COSPI_PAIR(c0_h, c1_h)    \
( {                                       \
    __m128i out0_m, r0_m, r1_m;           \
                                          \
    r0_m = __lsx_vreplgr2vr_h(c0_h);      \
    r1_m = __lsx_vreplgr2vr_h(c1_h);      \
    out0_m = __lsx_vpackev_h(r1_m, r0_m); \
                                          \
    out0_m;                               \
} )

#define VP9_ADDBLK_ST8x4_UB(dst, dst_stride, in0, in1, in2, in3)   \
{                                                                  \
    uint8_t *dst_m = (uint8_t *) (dst);                            \
    __m128i dst0_m, dst1_m, dst2_m, dst3_m;                        \
    __m128i tmp0_m, tmp1_m;                                        \
    __m128i res0_m, res1_m, res2_m, res3_m;                        \
    __m128i zero_m = __lsx_vldi(0);                                \
    LSX_DUP4_ARG2(__lsx_vld,                                       \
                  dst_m, 0,                                        \
                  dst_m + dst_stride, 0,                           \
                  dst_m + 2 * dst_stride, 0,                       \
                  dst_m + 3 * dst_stride, 0,                       \
                  dst0_m, dst1_m, dst2_m, dst3_m);                 \
    LSX_DUP4_ARG2(__lsx_vilvl_b,                                   \
                  zero_m, dst0_m, zero_m, dst1_m, zero_m, dst2_m,  \
                  zero_m, dst3_m, res0_m, res1_m, res2_m, res3_m); \
    LSX_DUP4_ARG2(__lsx_vadd_h,                                    \
                  res0_m, in0, res1_m, in1, res2_m, in2, res3_m,   \
                  in3, res0_m, res1_m, res2_m, res3_m);            \
    LSX_DUP4_ARG1(__lsx_clamp255_h,                                \
                  res0_m, res1_m, res2_m, res3_m,                  \
                  res0_m, res1_m, res2_m, res3_m);                 \
    LSX_DUP2_ARG2(__lsx_vpickev_b,                                 \
                  res1_m, res0_m, res3_m, res2_m, tmp0_m, tmp1_m); \
    __lsx_vstelm_d(tmp0_m, dst_m, 0, 0);                           \
    __lsx_vstelm_d(tmp0_m, dst_m + dst_stride, 0, 1);              \
    __lsx_vstelm_d(tmp1_m, dst_m + 2 * dst_stride, 0, 0);          \
    __lsx_vstelm_d(tmp1_m, dst_m + 3 * dst_stride, 0, 1);          \
}

static void vp9_idct_butterfly_transpose_store(int16_t *tmp_buf,
                                               int16_t *tmp_eve_buf,
                                               int16_t *tmp_odd_buf,
                                               int16_t *dst)
{
    __m128i vec0, vec1, vec2, vec3, loc0, loc1, loc2, loc3;
    __m128i m0, m1, m2, m3, m4, m5, m6, m7, n0, n1, n2, n3, n4, n5, n6, n7;

    /* FINAL BUTTERFLY : Dependency on Even & Odd */
    vec0 = __lsx_vld(tmp_odd_buf, 0);
    vec1 = __lsx_vld(tmp_odd_buf, 9 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 14 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 6 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 0);
    loc1 = __lsx_vld(tmp_eve_buf, 8 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 4 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 12 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h,
                 loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                 m0, m4, m2, m6);

    #define SUB(a, b) __lsx_vsub_h(a, b)

    __lsx_vst(SUB(loc0, vec3), tmp_buf, 31 * 8 * 2);
    __lsx_vst(SUB(loc1, vec2), tmp_buf, 23 * 8 * 2);
    __lsx_vst(SUB(loc2, vec1), tmp_buf, 27 * 8 * 2);
    __lsx_vst(SUB(loc3, vec0), tmp_buf, 19 * 8 * 2);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 4 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 13 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 10 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 3 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 2 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 10 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 6 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 14 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h,
                 loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                 m1, m5, m3, m7);

    __lsx_vst(SUB(loc0, vec3), tmp_buf, 29 * 8 * 2);
    __lsx_vst(SUB(loc1, vec2), tmp_buf, 21 * 8 * 2);
    __lsx_vst(SUB(loc2, vec1), tmp_buf, 25 * 8 * 2);
    __lsx_vst(SUB(loc3, vec0), tmp_buf, 17 * 8 * 2);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 2 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 11 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 12 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 7 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 1 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 9 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 5 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 13 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h,
                 loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                 n0, n4, n2, n6);

    __lsx_vst(SUB(loc0, vec3), tmp_buf, 30 * 8 * 2);
    __lsx_vst(SUB(loc1, vec2), tmp_buf, 22 * 8 * 2);
    __lsx_vst(SUB(loc2, vec1), tmp_buf, 26 * 8 * 2);
    __lsx_vst(SUB(loc3, vec0), tmp_buf, 18 * 8 * 2);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 5 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 15 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 8 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 1 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 3 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 11 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 7 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 15 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h,
                 loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                 n1, n5, n3, n7);

    __lsx_vst(SUB(loc0, vec3), tmp_buf, 28 * 8 * 2);
    __lsx_vst(SUB(loc1, vec2), tmp_buf, 20 * 8 * 2);
    __lsx_vst(SUB(loc2, vec1), tmp_buf, 24 * 8 * 2);
    __lsx_vst(SUB(loc3, vec0), tmp_buf, 16 * 8 * 2);

    /* Transpose : 16 vectors */
    /* 1st & 2nd 8x8 */
    TRANSPOSE8x8_H(m0, n0, m1, n1, m2, n2, m3, n3,
                   m0, n0, m1, n1, m2, n2, m3, n3);
    __lsx_vst(m0, dst, 0);
    __lsx_vst(n0, dst, 32 * 2);
    __lsx_vst(m1, dst, 32 * 4);
    __lsx_vst(n1, dst, 32 * 6);
    __lsx_vst(m2, dst, 32 * 8);
    __lsx_vst(n2, dst, 32 * 10);
    __lsx_vst(m3, dst, 32 * 12);
    __lsx_vst(n3, dst, 32 * 14);

    TRANSPOSE8x8_H(m4, n4, m5, n5, m6, n6, m7, n7,
                   m4, n4, m5, n5, m6, n6, m7, n7);

    __lsx_vst(m4, dst, 16);
    __lsx_vst(n4, dst, 16 + 32 * 2);
    __lsx_vst(m5, dst, 16 + 32 * 4);
    __lsx_vst(n5, dst, 16 + 32 * 6);
    __lsx_vst(m6, dst, 16 + 32 * 8);
    __lsx_vst(n6, dst, 16 + 32 * 10);
    __lsx_vst(m7, dst, 16 + 32 * 12);
    __lsx_vst(n7, dst, 16 + 32 * 14);

    /* 3rd & 4th 8x8 */
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 16 * 16, tmp_buf, 16 * 17,
                  tmp_buf, 16 * 18, tmp_buf, 16 * 19,
                  m0, n0, m1, n1);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 16 * 20, tmp_buf, 16 * 21,
                  tmp_buf, 16 * 22, tmp_buf, 16 * 23,
                  m2, n2, m3, n3);

    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 16 * 24, tmp_buf, 16 * 25,
                  tmp_buf, 16 * 26, tmp_buf, 16 * 27,
                  m4, n4, m5, n5);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 16 * 28, tmp_buf, 16 * 29,
                  tmp_buf, 16 * 30, tmp_buf, 16 * 31,
                  m6, n6, m7, n7);

    TRANSPOSE8x8_H(m0, n0, m1, n1, m2, n2, m3, n3,
                   m0, n0, m1, n1, m2, n2, m3, n3);

    __lsx_vst(m0, dst, 32);
    __lsx_vst(n0, dst, 32 + 32 * 2);
    __lsx_vst(m1, dst, 32 + 32 * 4);
    __lsx_vst(n1, dst, 32 + 32 * 6);
    __lsx_vst(m2, dst, 32 + 32 * 8);
    __lsx_vst(n2, dst, 32 + 32 * 10);
    __lsx_vst(m3, dst, 32 + 32 * 12);
    __lsx_vst(n3, dst, 32 + 32 * 14);

    TRANSPOSE8x8_H(m4, n4, m5, n5, m6, n6, m7, n7,
                   m4, n4, m5, n5, m6, n6, m7, n7);

    __lsx_vst(m4, dst, 48);
    __lsx_vst(n4, dst, 48 + 32 * 2);
    __lsx_vst(m5, dst, 48 + 32 * 4);
    __lsx_vst(n5, dst, 48 + 32 * 6);
    __lsx_vst(m6, dst, 48 + 32 * 8);
    __lsx_vst(n6, dst, 48 + 32 * 10);
    __lsx_vst(m7, dst, 48 + 32 * 12);
    __lsx_vst(n7, dst, 48 + 32 * 14);
}

static void vp9_idct8x32_column_even_process_store(int16_t *tmp_buf,
                                                   int16_t *tmp_eve_buf)
{
    __m128i vec0, vec1, vec2, vec3, loc0, loc1, loc2, loc3;
    __m128i reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
    __m128i stp0, stp1, stp2, stp3, stp4, stp5, stp6, stp7;
    __m128i zero = __lsx_vldi(0);

    /* Even stage 1 */
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 0, tmp_buf, 32 * 8,
                  tmp_buf, 32 * 16, tmp_buf, 32 * 24,
                  reg0, reg1, reg2, reg3);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 32 * 32, tmp_buf, 32 * 40,
                  tmp_buf, 32 * 48, tmp_buf, 32 * 56,
                  reg4, reg5, reg6, reg7);

    __lsx_vst(zero, tmp_buf, 0);
    __lsx_vst(zero, tmp_buf, 32 * 8);
    __lsx_vst(zero, tmp_buf, 32 * 16);
    __lsx_vst(zero, tmp_buf, 32 * 24);
    __lsx_vst(zero, tmp_buf, 32 * 32);
    __lsx_vst(zero, tmp_buf, 32 * 40);
    __lsx_vst(zero, tmp_buf, 32 * 48);
    __lsx_vst(zero, tmp_buf, 32 * 56);

    tmp_buf += (2 * 32);

    VP9_DOTP_CONST_PAIR(reg1, reg7, cospi_28_64, cospi_4_64, reg1, reg7);
    VP9_DOTP_CONST_PAIR(reg5, reg3, cospi_12_64, cospi_20_64, reg5, reg3);
    BUTTERFLY_4(v8i16, reg1, reg7, reg3, reg5, vec1, vec3, vec2, vec0);
    VP9_DOTP_CONST_PAIR(vec2, vec0, cospi_16_64, cospi_16_64, loc2, loc3);

    loc1 = vec3;
    loc0 = vec1;

    VP9_DOTP_CONST_PAIR(reg0, reg4, cospi_16_64, cospi_16_64, reg0, reg4);
    VP9_DOTP_CONST_PAIR(reg2, reg6, cospi_24_64, cospi_8_64, reg2, reg6);
    BUTTERFLY_4(v8i16, reg4, reg0, reg2, reg6, vec1, vec3, vec2, vec0);
    BUTTERFLY_4(v8i16, vec0, vec1, loc1, loc0, stp3, stp0, stp7, stp4);
    BUTTERFLY_4(v8i16, vec2, vec3, loc3, loc2, stp2, stp1, stp6, stp5);

    /* Even stage 2 */
    /* Load 8 */
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 0, tmp_buf, 32 * 8,
                  tmp_buf, 32 * 16, tmp_buf, 32 * 24,
                  reg0, reg1, reg2, reg3);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_buf, 32 * 32, tmp_buf, 32 * 40,
                  tmp_buf, 32 * 48, tmp_buf, 32 * 56,
                  reg4, reg5, reg6, reg7);

    __lsx_vst(zero, tmp_buf, 0);
    __lsx_vst(zero, tmp_buf, 32 * 8);
    __lsx_vst(zero, tmp_buf, 32 * 16);
    __lsx_vst(zero, tmp_buf, 32 * 24);
    __lsx_vst(zero, tmp_buf, 32 * 32);
    __lsx_vst(zero, tmp_buf, 32 * 40);
    __lsx_vst(zero, tmp_buf, 32 * 48);
    __lsx_vst(zero, tmp_buf, 32 * 56);

    VP9_DOTP_CONST_PAIR(reg0, reg7, cospi_30_64, cospi_2_64, reg0, reg7);
    VP9_DOTP_CONST_PAIR(reg4, reg3, cospi_14_64, cospi_18_64, reg4, reg3);
    VP9_DOTP_CONST_PAIR(reg2, reg5, cospi_22_64, cospi_10_64, reg2, reg5);
    VP9_DOTP_CONST_PAIR(reg6, reg1, cospi_6_64, cospi_26_64, reg6, reg1);

    vec0 = __lsx_vadd_h(reg0, reg4);
    reg0 = __lsx_vsub_h(reg0, reg4);
    reg4 = __lsx_vadd_h(reg6, reg2);
    reg6 = __lsx_vsub_h(reg6, reg2);
    reg2 = __lsx_vadd_h(reg1, reg5);
    reg1 = __lsx_vsub_h(reg1, reg5);
    reg5 = __lsx_vadd_h(reg7, reg3);
    reg7 = __lsx_vsub_h(reg7, reg3);
    reg3 = vec0;

    vec1 = reg2;
    reg2 = __lsx_vadd_h(reg3, reg4);
    reg3 = __lsx_vsub_h(reg3, reg4);
    reg4 = __lsx_vsub_h(reg5, vec1);
    reg5 = __lsx_vadd_h(reg5, vec1);

    VP9_DOTP_CONST_PAIR(reg7, reg0, cospi_24_64, cospi_8_64, reg0, reg7);
    VP9_DOTP_CONST_PAIR(__lsx_vneg_h(reg6), reg1, cospi_24_64, cospi_8_64,
                        reg6, reg1);

    vec0 = __lsx_vsub_h(reg0, reg6);
    reg0 = __lsx_vadd_h(reg0, reg6);
    vec1 = __lsx_vsub_h(reg7, reg1);
    reg7 = __lsx_vadd_h(reg7, reg1);

    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_16_64, cospi_16_64, reg6, reg1);
    VP9_DOTP_CONST_PAIR(reg4, reg3, cospi_16_64, cospi_16_64, reg3, reg4);

    /* Even stage 3 : Dependency on Even stage 1 & Even stage 2 */
    /* Store 8 */
    BUTTERFLY_4(v8i16, stp0, stp1, reg7, reg5, loc1, loc3, loc2, loc0);
    __lsx_vst(loc1, tmp_eve_buf, 0);
    __lsx_vst(loc3, tmp_eve_buf, 16);
    __lsx_vst(loc2, tmp_eve_buf, 14 * 16);
    __lsx_vst(loc0, tmp_eve_buf, 14 * 16 + 16);
    BUTTERFLY_4(v8i16, stp2, stp3, reg4, reg1, loc1, loc3, loc2, loc0);
    __lsx_vst(loc1, tmp_eve_buf, 2 * 16);
    __lsx_vst(loc3, tmp_eve_buf, 2 * 16 + 16);
    __lsx_vst(loc2, tmp_eve_buf, 12 * 16);
    __lsx_vst(loc0, tmp_eve_buf, 12 * 16 + 16);

    /* Store 8 */
    BUTTERFLY_4(v8i16, stp4, stp5, reg6, reg3, loc1, loc3, loc2, loc0);
    __lsx_vst(loc1, tmp_eve_buf, 4 * 16);
    __lsx_vst(loc3, tmp_eve_buf, 4 * 16 + 16);
    __lsx_vst(loc2, tmp_eve_buf, 10 * 16);
    __lsx_vst(loc0, tmp_eve_buf, 10 * 16 + 16);

    BUTTERFLY_4(v8i16, stp6, stp7, reg2, reg0, loc1, loc3, loc2, loc0);
    __lsx_vst(loc1, tmp_eve_buf, 6 * 16);
    __lsx_vst(loc3, tmp_eve_buf, 6 * 16 + 16);
    __lsx_vst(loc2, tmp_eve_buf, 8 * 16);
    __lsx_vst(loc0, tmp_eve_buf, 8 * 16 + 16);
}

static void vp9_idct8x32_column_odd_process_store(int16_t *tmp_buf,
                                                  int16_t *tmp_odd_buf)
{
    __m128i vec0, vec1, vec2, vec3, loc0, loc1, loc2, loc3;
    __m128i reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
    __m128i zero = __lsx_vldi(0);

    /* Odd stage 1 */
    reg0 = __lsx_vld(tmp_buf, 32 * 2);
    reg1 = __lsx_vld(tmp_buf, 7 * 32 * 2);
    reg2 = __lsx_vld(tmp_buf, 9 * 32 * 2);
    reg3 = __lsx_vld(tmp_buf, 15 * 32 * 2);
    reg4 = __lsx_vld(tmp_buf, 17 * 32 * 2);
    reg5 = __lsx_vld(tmp_buf, 23 * 32 * 2);
    reg6 = __lsx_vld(tmp_buf, 25 * 32 * 2);
    reg7 = __lsx_vld(tmp_buf, 31 * 32 * 2);

    __lsx_vst(zero, tmp_buf, 32 * 2);
    __lsx_vst(zero, tmp_buf, 7 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 9 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 15 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 17 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 23 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 25 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 31 * 32 * 2);

    VP9_DOTP_CONST_PAIR(reg0, reg7, cospi_31_64, cospi_1_64, reg0, reg7);
    VP9_DOTP_CONST_PAIR(reg4, reg3, cospi_15_64, cospi_17_64, reg3, reg4);
    VP9_DOTP_CONST_PAIR(reg2, reg5, cospi_23_64, cospi_9_64, reg2, reg5);
    VP9_DOTP_CONST_PAIR(reg6, reg1, cospi_7_64, cospi_25_64, reg1, reg6);

    vec0 = __lsx_vadd_h(reg0, reg3);
    reg0 = __lsx_vsub_h(reg0, reg3);
    reg3 = __lsx_vadd_h(reg7, reg4);
    reg7 = __lsx_vsub_h(reg7, reg4);
    reg4 = __lsx_vadd_h(reg1, reg2);
    reg1 = __lsx_vsub_h(reg1, reg2);
    reg2 = __lsx_vadd_h(reg6, reg5);
    reg6 = __lsx_vsub_h(reg6, reg5);
    reg5 = vec0;

    /* 4 Stores */
    LSX_DUP2_ARG2(__lsx_vadd_h, reg5, reg4, reg3, reg2, vec0, vec1);
    __lsx_vst(vec0, tmp_odd_buf, 4 * 16);
    __lsx_vst(vec1, tmp_odd_buf, 4 * 16 + 16);
    LSX_DUP2_ARG2(__lsx_vsub_h, reg5, reg4, reg3, reg2, vec0, vec1);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_24_64, cospi_8_64, vec0, vec1);
    __lsx_vst(vec0, tmp_odd_buf, 0);
    __lsx_vst(vec1, tmp_odd_buf, 16);

    /* 4 Stores */
    VP9_DOTP_CONST_PAIR(reg7, reg0, cospi_28_64, cospi_4_64, reg0, reg7);
    VP9_DOTP_CONST_PAIR(reg6, reg1, -cospi_4_64, cospi_28_64, reg1, reg6);
    BUTTERFLY_4(v8i16, reg0, reg7, reg6, reg1, vec0, vec1, vec2, vec3);
    __lsx_vst(vec0, tmp_odd_buf, 6 * 16);
    __lsx_vst(vec1, tmp_odd_buf, 6 * 16 + 16);
    VP9_DOTP_CONST_PAIR(vec2, vec3, cospi_24_64, cospi_8_64, vec2, vec3);
    __lsx_vst(vec2, tmp_odd_buf, 2 * 16);
    __lsx_vst(vec3, tmp_odd_buf, 2 * 16 + 16);

    /* Odd stage 2 */
    /* 8 loads */
    reg0 = __lsx_vld(tmp_buf, 3 * 32 * 2);
    reg1 = __lsx_vld(tmp_buf, 5 * 32 * 2);
    reg2 = __lsx_vld(tmp_buf, 11 * 32 * 2);
    reg3 = __lsx_vld(tmp_buf, 13 * 32 * 2);
    reg4 = __lsx_vld(tmp_buf, 19 * 32 * 2);
    reg5 = __lsx_vld(tmp_buf, 21 * 32 * 2);
    reg6 = __lsx_vld(tmp_buf, 27 * 32 * 2);
    reg7 = __lsx_vld(tmp_buf, 29 * 32 * 2);

    __lsx_vst(zero, tmp_buf, 3 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 5 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 11 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 13 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 19 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 21 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 27 * 32 * 2);
    __lsx_vst(zero, tmp_buf, 29 * 32 * 2);

    VP9_DOTP_CONST_PAIR(reg1, reg6, cospi_27_64, cospi_5_64, reg1, reg6);
    VP9_DOTP_CONST_PAIR(reg5, reg2, cospi_11_64, cospi_21_64, reg2, reg5);
    VP9_DOTP_CONST_PAIR(reg3, reg4, cospi_19_64, cospi_13_64, reg3, reg4);
    VP9_DOTP_CONST_PAIR(reg7, reg0, cospi_3_64, cospi_29_64, reg0, reg7);

    /* 4 Stores */
    LSX_DUP4_ARG2(__lsx_vsub_h,reg1, reg2, reg6, reg5, reg0, reg3, reg7, reg4,
                  vec0, vec1, vec2, vec3);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_12_64, cospi_20_64, loc0, loc1);
    VP9_DOTP_CONST_PAIR(vec3, vec2, -cospi_20_64, cospi_12_64, loc2, loc3);
    BUTTERFLY_4(v8i16, loc2, loc3, loc1, loc0, vec0, vec1, vec3, vec2);
    __lsx_vst(vec0, tmp_odd_buf, 12 * 16);
    __lsx_vst(vec1, tmp_odd_buf, 12 * 16 + 3 * 16);
    VP9_DOTP_CONST_PAIR(vec3, vec2, -cospi_8_64, cospi_24_64, vec0, vec1);
    __lsx_vst(vec0, tmp_odd_buf, 10 * 16);
    __lsx_vst(vec1, tmp_odd_buf, 10 * 16 + 16);

    /* 4 Stores */
    LSX_DUP4_ARG2(__lsx_vadd_h, reg0, reg3, reg1, reg2, reg5, reg6, reg4, reg7,
                  vec0, vec1, vec2, vec3);
    BUTTERFLY_4(v8i16, vec0, vec3, vec2, vec1, reg0, reg1, reg3, reg2);
    __lsx_vst(reg0, tmp_odd_buf, 13 * 16);
    __lsx_vst(reg1, tmp_odd_buf, 13 * 16 + 16);
    VP9_DOTP_CONST_PAIR(reg3, reg2, -cospi_8_64, cospi_24_64,
                        reg0, reg1);
    __lsx_vst(reg0, tmp_odd_buf, 8 * 16);
    __lsx_vst(reg1, tmp_odd_buf, 8 * 16 + 16);

    /* Odd stage 3 : Dependency on Odd stage 1 & Odd stage 2 */
    /* Load 8 & Store 8 */
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_odd_buf, 0, tmp_odd_buf, 16,
                  tmp_odd_buf, 32, tmp_odd_buf, 48,
                  reg0, reg1, reg2, reg3);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_odd_buf, 8 * 16, tmp_odd_buf, 8 * 16 + 16,
                  tmp_odd_buf, 8 * 16 + 32, tmp_odd_buf, 8 * 16 + 48,
                  reg4, reg5, reg6, reg7);

    LSX_DUP4_ARG2(__lsx_vadd_h, reg0, reg4, reg1, reg5, reg2, reg6, reg3, reg7,
                  loc0, loc1, loc2, loc3);
    __lsx_vst(loc0, tmp_odd_buf, 0);
    __lsx_vst(loc1, tmp_odd_buf, 16);
    __lsx_vst(loc2, tmp_odd_buf, 32);
    __lsx_vst(loc3, tmp_odd_buf, 48);
    LSX_DUP2_ARG2(__lsx_vsub_h, reg0, reg4, reg1, reg5, vec0, vec1);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_16_64, cospi_16_64, loc0, loc1);

    LSX_DUP2_ARG2(__lsx_vsub_h, reg2, reg6, reg3, reg7, vec0, vec1);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_16_64, cospi_16_64, loc2, loc3);
    __lsx_vst(loc0, tmp_odd_buf, 8 * 16);
    __lsx_vst(loc1, tmp_odd_buf, 8 * 16 + 16);
    __lsx_vst(loc2, tmp_odd_buf, 8 * 16 + 32);
    __lsx_vst(loc3, tmp_odd_buf, 8 * 16 + 48);

    /* Load 8 & Store 8 */
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_odd_buf, 4 * 16, tmp_odd_buf, 4 * 16 + 16,
                  tmp_odd_buf, 4 * 16 + 32, tmp_odd_buf, 4 * 16 + 48,
                  reg1, reg2, reg0, reg3);
    LSX_DUP4_ARG2(__lsx_vld,
                  tmp_odd_buf, 12 * 16, tmp_odd_buf, 12 * 16 + 16,
                  tmp_odd_buf, 12 * 16 + 32, tmp_odd_buf, 12 * 16 + 48,
                  reg4, reg5, reg6, reg7);

    LSX_DUP4_ARG2(__lsx_vadd_h, reg0, reg4, reg1, reg5, reg2, reg6, reg3, reg7,
                  loc0, loc1, loc2, loc3);
    __lsx_vst(loc0, tmp_odd_buf, 4 * 16);
    __lsx_vst(loc1, tmp_odd_buf, 4 * 16 + 16);
    __lsx_vst(loc2, tmp_odd_buf, 4 * 16 + 32);
    __lsx_vst(loc3, tmp_odd_buf, 4 * 16 + 48);

    LSX_DUP2_ARG2(__lsx_vsub_h, reg0, reg4, reg3, reg7, vec0, vec1);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_16_64, cospi_16_64, loc0, loc1);

    LSX_DUP2_ARG2(__lsx_vsub_h, reg1, reg5, reg2, reg6, vec0, vec1);
    VP9_DOTP_CONST_PAIR(vec1, vec0, cospi_16_64, cospi_16_64, loc2, loc3);
    __lsx_vst(loc0, tmp_odd_buf, 12 * 16);
    __lsx_vst(loc1, tmp_odd_buf, 12 * 16 + 16);
    __lsx_vst(loc2, tmp_odd_buf, 12 * 16 + 32);
    __lsx_vst(loc3, tmp_odd_buf, 12 * 16 + 48);
}

static void vp9_idct8x32_column_butterfly_addblk(int16_t *tmp_eve_buf,
                                                 int16_t *tmp_odd_buf,
                                                 uint8_t *dst,
                                                 int32_t dst_stride)
{
    __m128i vec0, vec1, vec2, vec3, loc0, loc1, loc2, loc3;
    __m128i m0, m1, m2, m3, m4, m5, m6, m7, n0, n1, n2, n3, n4, n5, n6, n7;

    /* FINAL BUTTERFLY : Dependency on Even & Odd */
    vec0 = __lsx_vld(tmp_odd_buf, 0);
    vec1 = __lsx_vld(tmp_odd_buf, 9 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 14 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 6 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 0);
    loc1 = __lsx_vld(tmp_eve_buf, 8 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 4 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 12 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  m0, m4, m2, m6);
    LSX_DUP4_ARG2(__lsx_vsrari_h, m0, 6, m2, 6, m4, 6, m6, 6, m0, m2, m4, m6);
    VP9_ADDBLK_ST8x4_UB(dst, (4 * dst_stride), m0, m2, m4, m6);

    LSX_DUP4_ARG2(__lsx_vsub_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  m6, m2, m4, m0);
    LSX_DUP4_ARG2(__lsx_vsrari_h, m0, 6, m2, 6, m4, 6, m6, 6, m0, m2, m4, m6);
    VP9_ADDBLK_ST8x4_UB((dst + 19 * dst_stride), (4 * dst_stride),
                        m0, m2, m4, m6);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 4 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 13 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 10 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 3 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 2 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 10 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 6 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 14 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  m1, m5, m3, m7);
    LSX_DUP4_ARG2(__lsx_vsrari_h, m1, 6, m3, 6, m5, 6, m7, 6, m1, m3, m5, m7);
    VP9_ADDBLK_ST8x4_UB((dst + 2 * dst_stride), (4 * dst_stride),
                        m1, m3, m5, m7);

    LSX_DUP4_ARG2(__lsx_vsub_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  m7, m3, m5, m1);
    LSX_DUP4_ARG2(__lsx_vsrari_h, m1, 6, m3, 6, m5, 6, m7, 6, m1, m3, m5, m7);
    VP9_ADDBLK_ST8x4_UB((dst + 17 * dst_stride), (4 * dst_stride),
                        m1, m3, m5, m7);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 2 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 11 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 12 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 7 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 1 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 9 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 5 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 13 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  n0, n4, n2, n6);
    LSX_DUP4_ARG2(__lsx_vsrari_h, n0, 6, n2, 6, n4, 6, n6, 6, n0, n2, n4, n6);
    VP9_ADDBLK_ST8x4_UB((dst + 1 * dst_stride), (4 * dst_stride),
                        n0, n2, n4, n6);
    LSX_DUP4_ARG2(__lsx_vsub_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  n6, n2, n4, n0);
    LSX_DUP4_ARG2(__lsx_vsrari_h, n0, 6, n2, 6, n4, 6, n6, 6, n0, n2, n4, n6);
    VP9_ADDBLK_ST8x4_UB((dst + 18 * dst_stride), (4 * dst_stride),
                        n0, n2, n4, n6);

    /* Load 8 & Store 8 */
    vec0 = __lsx_vld(tmp_odd_buf, 5 * 8 * 2);
    vec1 = __lsx_vld(tmp_odd_buf, 15 * 8 * 2);
    vec2 = __lsx_vld(tmp_odd_buf, 8 * 8 * 2);
    vec3 = __lsx_vld(tmp_odd_buf, 1 * 8 * 2);
    loc0 = __lsx_vld(tmp_eve_buf, 3 * 8 * 2);
    loc1 = __lsx_vld(tmp_eve_buf, 11 * 8 * 2);
    loc2 = __lsx_vld(tmp_eve_buf, 7 * 8 * 2);
    loc3 = __lsx_vld(tmp_eve_buf, 15 * 8 * 2);

    LSX_DUP4_ARG2(__lsx_vadd_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  n1, n5, n3, n7);
    LSX_DUP4_ARG2(__lsx_vsrari_h, n1, 6, n3, 6, n5, 6, n7, 6, n1, n3, n5, n7);
    VP9_ADDBLK_ST8x4_UB((dst + 3 * dst_stride), (4 * dst_stride),
                        n1, n3, n5, n7);
    LSX_DUP4_ARG2(__lsx_vsub_h, loc0, vec3, loc1, vec2, loc2, vec1, loc3, vec0,
                  n7, n3, n5, n1);
    LSX_DUP4_ARG2(__lsx_vsrari_h, n1, 6, n3, 6, n5, 6, n7, 6, n1, n3, n5, n7);
    VP9_ADDBLK_ST8x4_UB((dst + 16 * dst_stride), (4 * dst_stride),
                        n1, n3, n5, n7);
}

static void vp9_idct8x32_1d_columns_addblk_lsx(int16_t *input, uint8_t *dst,
                                               int32_t dst_stride)
{
    int16_t tmp_odd_buf[16 * 8] ALLOC_ALIGNED(16);
    int16_t tmp_eve_buf[16 * 8] ALLOC_ALIGNED(16);

    vp9_idct8x32_column_even_process_store(input, &tmp_eve_buf[0]);
    vp9_idct8x32_column_odd_process_store(input, &tmp_odd_buf[0]);
    vp9_idct8x32_column_butterfly_addblk(&tmp_eve_buf[0], &tmp_odd_buf[0],
                                         dst, dst_stride);
}

static void vp9_idct8x32_1d_columns_lsx(int16_t *input, int16_t *output,
                                        int16_t *tmp_buf)
{
    int16_t tmp_odd_buf[16 * 8] ALLOC_ALIGNED(16);
    int16_t tmp_eve_buf[16 * 8] ALLOC_ALIGNED(16);

    vp9_idct8x32_column_even_process_store(input, &tmp_eve_buf[0]);
    vp9_idct8x32_column_odd_process_store(input, &tmp_odd_buf[0]);
    vp9_idct_butterfly_transpose_store(tmp_buf, &tmp_eve_buf[0],
                                       &tmp_odd_buf[0], output);
}

static void vp9_idct32x32_1_add_lsx(int16_t *input, uint8_t *dst,
                                    int32_t dst_stride)
{
    int32_t i;
    int16_t out;
    __m128i zero = __lsx_vldi(0);
    __m128i dst0, dst1, dst2, dst3, tmp0, tmp1, tmp2, tmp3;
    __m128i res0, res1, res2, res3, res4, res5, res6, res7, vec;

    out = ROUND_POWER_OF_TWO((input[0] * cospi_16_64), VP9_DCT_CONST_BITS);
    out = ROUND_POWER_OF_TWO((out * cospi_16_64), VP9_DCT_CONST_BITS);
    out = ROUND_POWER_OF_TWO(out, 6);
    input[0] = 0;

    vec = __lsx_vreplgr2vr_h(out);

    for (i = 16; i--;) {
        LSX_DUP2_ARG2(__lsx_vld, dst, 0, dst, 16, dst0, dst1);
        LSX_DUP2_ARG2(__lsx_vld, dst + dst_stride, 0,
                      dst + dst_stride, 16, dst2, dst3);

        LSX_DUP4_ARG2(__lsx_vilvl_b,
                      zero, dst0, zero, dst1, zero, dst2, zero, dst3,
                      res0, res1, res2, res3);
        LSX_DUP4_ARG2(__lsx_vilvh_b,
                      zero, dst0, zero, dst1, zero, dst2, zero, dst3,
                      res4, res5, res6, res7);
        LSX_DUP4_ARG2(__lsx_vadd_h,
                      res0, vec, res1, vec, res2, vec, res3, vec,
                      res0, res1, res2, res3);
        LSX_DUP4_ARG2(__lsx_vadd_h,
                      res4, vec, res5, vec, res6, vec, res7, vec,
                      res4, res5, res6, res7);
        LSX_DUP4_ARG1(__lsx_clamp255_h,
                      res0, res1, res2, res3,
                      res0, res1, res2, res3);
        LSX_DUP4_ARG1(__lsx_clamp255_h,
                      res4, res5, res6, res7,
                      res4, res5, res6, res7);
        LSX_DUP4_ARG2(__lsx_vpickev_b,
                      res4, res0, res5, res1, res6, res2, res7, res3,
                      tmp0, tmp1, tmp2, tmp3);

        __lsx_vst(tmp0, dst, 0);
        __lsx_vst(tmp1, dst, 16);
        dst += dst_stride;
        __lsx_vst(tmp2, dst, 0);
        __lsx_vst(tmp3, dst, 16);
        dst += dst_stride;
    }
}

static void vp9_idct32x32_34_colcol_addblk_lsx(int16_t *input, uint8_t *dst,
                                               int32_t dst_stride)
{
    int32_t i;
    int16_t out_arr[32 * 32] ALLOC_ALIGNED(16);
    int16_t *out_ptr = out_arr;
    int16_t tmp_buf[8 * 32] ALLOC_ALIGNED(16);

    for (i = 32; i--;) {
        __asm__ volatile (
            "vxor.v   $vr0,   $vr0,          $vr0   \n\t"
            "vst      $vr0,   %[out_ptr],    0      \n\t"
            "vst      $vr0,   %[out_ptr],    16     \n\t"
            "vst      $vr0,   %[out_ptr],    32     \n\t"
            "vst      $vr0,   %[out_ptr],    48     \n\t"
            :
            : [out_ptr] "r" (out_ptr)
        );
        out_ptr += 32;
    }

    out_ptr = out_arr;

    /* process 8*32 block */
    vp9_idct8x32_1d_columns_lsx(input, out_ptr, &tmp_buf[0]);

    /* transform columns */
    for (i = 0; i < 4; i++) {
        /* process 8*32 block */
        vp9_idct8x32_1d_columns_addblk_lsx((out_ptr + (i << 3)),
                                           (dst + (i << 3)), dst_stride);
    }
}

static void vp9_idct32x32_colcol_addblk_lsx(int16_t *input, uint8_t *dst,
                                            int32_t dst_stride)
{
    int32_t i;
    int16_t out_arr[32 * 32] ALLOC_ALIGNED(16);
    int16_t *out_ptr = out_arr;
    int16_t tmp_buf[8 * 32] ALLOC_ALIGNED(16);

    /* transform rows */
    for (i = 0; i < 4; i++) {
        /* process 8*32 block */
        vp9_idct8x32_1d_columns_lsx((input + (i << 3)), (out_ptr + (i << 8)),
                                    &tmp_buf[0]);
    }

    /* transform columns */
    for (i = 0; i < 4; i++) {
        /* process 8*32 block */
        vp9_idct8x32_1d_columns_addblk_lsx((out_ptr + (i << 3)),
                                           (dst + (i << 3)), dst_stride);
    }
}

void ff_idct_idct_32x32_add_lsx(uint8_t *dst, ptrdiff_t stride,
                                int16_t *block, int eob)
{
    if (eob == 1) {
        vp9_idct32x32_1_add_lsx(block, dst, stride);
    }
    else if (eob <= 34) {
        vp9_idct32x32_34_colcol_addblk_lsx(block, dst, stride);
    }
    else {
        vp9_idct32x32_colcol_addblk_lsx(block, dst, stride);
    }
}

