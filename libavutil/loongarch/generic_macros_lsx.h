/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * All rights reserved.
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
 *                Xiwei Gu   <guxiwei-hf@loongson.cn>
 *                Lu Wang    <wanglu@loongson.cn>
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
 *
 */

#ifndef AVUTIL_LOONGARCH_GENERIC_MACROS_LSX_H
#define AVUTIL_LOONGARCH_GENERIC_MACROS_LSX_H

/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * All rights reserved.
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
 *                Xiwei Gu   <guxiwei-hf@loongson.cn>
 *                Lu Wang    <wanglu@loongson.cn>
 *
 * This file is maintained in LSOM project, don't change it directly.
 * You can get the latest version of this header from: ***
 *
 */

#ifndef GENERIC_MACROS_LSX_H
#define GENERIC_MACROS_LSX_H

#include <lsxintrin.h>

/**
 * MAJOR version: Macro usage changes.
 * MINOR version: Add new functions, or bug fix.
 * MICRO version: Comment changes or implementation changes.
 */
#define LSOM_LSX_VERSION_MAJOR 0
#define LSOM_LSX_VERSION_MINOR 4
#define LSOM_LSX_VERSION_MICRO 0

#define LSX_DUP2_ARG1(_LSX_INS, _IN0, _IN1, _OUT0, _OUT1) \
{ \
    _OUT0 = _LSX_INS(_IN0); \
    _OUT1 = _LSX_INS(_IN1); \
}

#define LSX_DUP2_ARG2(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1) \
{ \
    _OUT0 = _LSX_INS(_IN0, _IN1); \
    _OUT1 = _LSX_INS(_IN2, _IN3); \
}

#define LSX_DUP2_ARG3(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _OUT0, _OUT1) \
{ \
    _OUT0 = _LSX_INS(_IN0, _IN1, _IN2); \
    _OUT1 = _LSX_INS(_IN3, _IN4, _IN5); \
}

#define LSX_DUP4_ARG1(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LSX_DUP2_ARG1(_LSX_INS, _IN0, _IN1, _OUT0, _OUT1); \
    LSX_DUP2_ARG1(_LSX_INS, _IN2, _IN3, _OUT2, _OUT3); \
}

#define LSX_DUP4_ARG2(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _IN6, _IN7, \
                      _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LSX_DUP2_ARG2(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1); \
    LSX_DUP2_ARG2(_LSX_INS, _IN4, _IN5, _IN6, _IN7, _OUT2, _OUT3); \
}

#define LSX_DUP4_ARG3(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _IN6, _IN7, \
                      _IN8, _IN9, _IN10, _IN11, _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LSX_DUP2_ARG3(_LSX_INS, _IN0, _IN1, _IN2, _IN3, _IN4,  _IN5,  _OUT0, _OUT1); \
    LSX_DUP2_ARG3(_LSX_INS, _IN6, _IN7, _IN8, _IN9, _IN10, _IN11, _OUT2, _OUT3); \
}

/*
 * =============================================================================
 * Description : Dot product & addition of byte vector elements
 * Arguments   : Inputs  - in_c, in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from in_h are multiplied by
 *               signed byte elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 *               Then the results plus to signed half word elements from in_c.
 * Example     : out = __lsx_dp2add_h_b(in_c, in_h, in_l)
 *        in_c : 1,2,3,4, 1,2,3,4
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1
 *         out : 23,40,41,26, 23,40,41,26
 * =============================================================================
 */
static inline __m128i __lsx_dp2add_h_b(__m128i in_c, __m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmaddwev_h_b(in_c, in_h, in_l);
    out = __lsx_vmaddwod_h_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product & addition of byte vector elements
 * Arguments   : Inputs  - in_c, in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Unsigned byte elements from in_h are multiplied by
 *               unsigned byte elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 *               The results plus to signed half word elements from in_c.
 * Example     : out = __lsx_dp2add_h_b(in_c, in_h, in_l)
 *        in_c : 1,2,3,4, 1,2,3,4
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1
 *         out : 23,40,41,26, 23,40,41,26
 * =============================================================================
 */
static inline __m128i __lsx_dp2add_h_bu(__m128i in_c, __m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmaddwev_h_bu(in_c, in_h, in_l);
    out = __lsx_vmaddwod_h_bu(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of byte vector elements
 * Arguments   : Inputs  - in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from in_h are multiplied by
 *               signed byte elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 * Example     : out = __lsx_dp2_h_b(in_h, in_l)
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1
 *         out : 22,38,38,22, 22,38,38,22
 * =============================================================================
 */
static inline __m128i __lsx_dp2_h_b(__m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmulwev_h_b(in_h, in_l);
    out = __lsx_vmaddwod_h_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of byte vector elements
 * Arguments   : Inputs  - in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Unsigned byte elements from in_h are multiplied by
 *               signed byte elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 * Example     : out = __lsx_dp2_h_bu_b(in_h, in_l)
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,-1
 *         out : 22,38,38,22, 22,38,38,6
 * =============================================================================
 */
static inline __m128i __lsx_dp2_h_bu_b(__m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmulwev_h_bu_b(in_h, in_l);
    out = __lsx_vmaddwod_h_bu_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of byte vector elements
 * Arguments   : Inputs  - in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from in_h are multiplied by
 *               signed byte elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 * Example     : out = __lsx_dp2_w_h(in_h, in_l)
 *        in_h : 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1
 *         out : 22,38,38,22
 * =============================================================================
 */
static inline __m128i __lsx_dp2_w_h(__m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmulwev_w_h(in_h, in_l);
    out = __lsx_vmaddwod_w_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product & addition of half word vector elements
 * Arguments   : Inputs  - in_c, in_h, in_l
 *               Outputs - out
 *               Retrun Type - __m128i
 * Details     : Signed half word elements from in_h are multiplied by
 *               signed half word elements from in_l, and then added adjacent to
 *               each other to get results with the twice size of input.
 *               Then the results plus to signed word elements from in_c.
 * Example     : out = __lsx_dp2add_h_b(in_c, in_h, in_l)
 *        in_c : 1,2,3,4
 *        in_h : 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1
 *         out : 23,40,41,26
 * =============================================================================
 */
static inline __m128i __lsx_dp2add_w_h(__m128i in_c, __m128i in_h, __m128i in_l)
{
    __m128i out;

    out = __lsx_vmaddwev_w_h(in_c, in_h, in_l);
    out = __lsx_vmaddwod_w_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Set each element of vector between 0 and 255
 * Arguments   : Inputs  - _in
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from _in are clamped between 0 and 255.
 * Example     : out = __lsx_clamp255_h(_in)
 *         _in : -8,255,280,249, -8,255,280,249
 *         out : 0,255,255,249, 0,255,255,249
 * =============================================================================
 */
static inline __m128i __lsx_clamp255_h(__m128i _in)
{
    __m128i out;

    out = __lsx_vmaxi_h(_in, 0);
    out = __lsx_vsat_hu(out, 7);
    return out;
}

/*
 * =============================================================================
 * Description : Set each element of vector between 0 and 255
 * Arguments   : Inputs  - _in
 *               Outputs - out
 *               Retrun Type - word
 * Details     : Signed byte elements from _in are clamped between 0 and 255.
 * Example     : out = __lsx_clamp255_w(_in)
 *         _in : -8,255,280,249
 *         out : 0,255,255,249
 * =============================================================================
 */
static inline __m128i __lsx_clamp255_w(__m128i _in)
{
    __m128i out;

    out = __lsx_vmaxi_w(_in, 0);
    out = __lsx_vsat_wu(out, 7);
    return out;
}

/*
 * =============================================================================
 * Description : Transposes 4x4 block with word elements in vectors
 * Arguments   : Inputs  - in0, in1, in2, in3
 *               Outputs - out0, out1, out2, out3
 * Details     :
 * Example     :
 *               1, 2, 3, 4            1, 5, 9,13
 *               5, 6, 7, 8    to      2, 6,10,14
 *               9,10,11,12  =====>    3, 7,11,15
 *              13,14,15,16            4, 8,12,16
 * =============================================================================
 */
#define TRANSPOSE4x4_W(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                          \
    __m128i _t0, _t1, _t2, _t3;                                            \
                                                                           \
    _t0   = __lsx_vilvl_w(_in1, _in0);                                     \
    _t1   = __lsx_vilvh_w(_in1, _in0);                                     \
    _t2   = __lsx_vilvl_w(_in3, _in2);                                     \
    _t3   = __lsx_vilvh_w(_in3, _in2);                                     \
    _out0 = __lsx_vilvl_d(_t2, _t0);                                       \
    _out1 = __lsx_vilvh_d(_t2, _t0);                                       \
    _out2 = __lsx_vilvl_d(_t3, _t1);                                       \
    _out3 = __lsx_vilvh_d(_t3, _t1);                                       \
}

/*
 * =============================================================================
 * Description : Transposes 8x8 block with half word elements in vectors
 * Arguments   : Inputs  - in0, in1, in2, in3, in4, in5, in6, in7
 *               Outputs - out0, out1, out2, out3, out4, out5, out6, out7
 * Details     :
 * Example     :
 *              00,01,02,03,04,05,06,07           00,10,20,30,40,50,60,70
 *              10,11,12,13,14,15,16,17           01,11,21,31,41,51,61,71
 *              20,21,22,23,24,25,26,27           02,12,22,32,42,52,62,72
 *              30,31,32,33,34,35,36,37    to     03,13,23,33,43,53,63,73
 *              40,41,42,43,44,45,46,47  ======>  04,14,24,34,44,54,64,74
 *              50,51,52,53,54,55,56,57           05,15,25,35,45,55,65,75
 *              60,61,62,63,64,65,66,67           06,16,26,36,46,56,66,76
 *              70,71,72,73,74,75,76,77           07,17,27,37,47,57,67,77
 * =============================================================================
 */
#define TRANSPOSE8x8_H(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,         \
                       _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7) \
{                                                                              \
    __m128i _s0, _s1, _t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;                  \
                                                                               \
    _s0 = __lsx_vilvl_h(_in6, _in4);                                           \
    _s1 = __lsx_vilvl_h(_in7, _in5);                                           \
    _t0 = __lsx_vilvl_h(_s1, _s0);                                             \
    _t1 = __lsx_vilvh_h(_s1, _s0);                                             \
    _s0 = __lsx_vilvh_h(_in6, _in4);                                           \
    _s1 = __lsx_vilvh_h(_in7, _in5);                                           \
    _t2 = __lsx_vilvl_h(_s1, _s0);                                             \
    _t3 = __lsx_vilvh_h(_s1, _s0);                                             \
    _s0 = __lsx_vilvl_h(_in2, _in0);                                           \
    _s1 = __lsx_vilvl_h(_in3, _in1);                                           \
    _t4 = __lsx_vilvl_h(_s1, _s0);                                             \
    _t5 = __lsx_vilvh_h(_s1, _s0);                                             \
    _s0 = __lsx_vilvh_h(_in2, _in0);                                           \
    _s1 = __lsx_vilvh_h(_in3, _in1);                                           \
    _t6 = __lsx_vilvl_h(_s1, _s0);                                             \
    _t7 = __lsx_vilvh_h(_s1, _s0);                                             \
                                                                               \
    _out0 = __lsx_vpickev_d(_t0, _t4);                                         \
    _out2 = __lsx_vpickev_d(_t1, _t5);                                         \
    _out4 = __lsx_vpickev_d(_t2, _t6);                                         \
    _out6 = __lsx_vpickev_d(_t3, _t7);                                         \
    _out1 = __lsx_vpickod_d(_t0, _t4);                                         \
    _out3 = __lsx_vpickod_d(_t1, _t5);                                         \
    _out5 = __lsx_vpickod_d(_t2, _t6);                                         \
    _out7 = __lsx_vpickod_d(_t3, _t7);                                         \
}

/*
 * =============================================================================
 * Description : Butterfly of 4 input vectors
 * Arguments   : Inputs  - in0, in1, in2, in3
 *               Outputs - out0, out1, out2, out3
 * Details     : Butterfly operation
 * Example     :
 *               out0 = in0 + in3;
 *               out1 = in1 + in2;
 *               out2 = in1 - in2;
 *               out3 = in0 - in3;
 * =============================================================================
 */
#define BUTTERFLY_4_B(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                         \
    _out0 = __lsx_vadd_b(_in0, _in3);                                     \
    _out1 = __lsx_vadd_b(_in1, _in2);                                     \
    _out2 = __lsx_vsub_b(_in1, _in2);                                     \
    _out3 = __lsx_vsub_b(_in0, _in3);                                     \
}
#define BUTTERFLY_4_H(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                         \
    _out0 = __lsx_vadd_h(_in0, _in3);                                     \
    _out1 = __lsx_vadd_h(_in1, _in2);                                     \
    _out2 = __lsx_vsub_h(_in1, _in2);                                     \
    _out3 = __lsx_vsub_h(_in0, _in3);                                     \
}
#define BUTTERFLY_4_W(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                         \
    _out0 = __lsx_vadd_w(_in0, _in3);                                     \
    _out1 = __lsx_vadd_w(_in1, _in2);                                     \
    _out2 = __lsx_vsub_w(_in1, _in2);                                     \
    _out3 = __lsx_vsub_w(_in0, _in3);                                     \
}
#define BUTTERFLY_4_D(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                         \
    _out0 = __lsx_vadd_d(_in0, _in3);                                     \
    _out1 = __lsx_vadd_d(_in1, _in2);                                     \
    _out2 = __lsx_vsub_d(_in1, _in2);                                     \
    _out3 = __lsx_vsub_d(_in0, _in3);                                     \
}

/*
 * =============================================================================
 * Description : Print out elements in vector.
 * Arguments   : Inputs  - RTYPE, _element_num, _in0, _enter
 *               Outputs -
 * Details     : Print out '_element_num' elements in 'RTYPE' vector '_in0', if
 *               '_enter' is TRUE, prefix "\nVP:" will be added first.
 * Example     : VECT_PRINT(v4i32,4,in0,1); // in0: 1,2,3,4
 *               VP:1,2,3,4,
 * =============================================================================
 */
#define VECT_PRINT(RTYPE, element_num, in0, enter)    \
{                                                     \
    RTYPE _tmp0 = (RTYPE)in0;                         \
    int _i = 0;                                       \
    if (enter)                                        \
        printf("\nVP:");                              \
    for(_i = 0; _i < element_num; _i++)               \
        printf("%d,",_tmp0[_i]);                      \
}

#endif /* GENERIC_MACROS_LSX_H */
#endif /* AVUTIL_LOONGARCH_GENERIC_MACROS_LSX_H */
