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

#ifndef AVUTIL_LOONGARCH_GENERIC_MACROS_LASX_H
#define AVUTIL_LOONGARCH_GENERIC_MACROS_LASX_H

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

#ifndef GENERIC_MACROS_LASX_H
#define GENERIC_MACROS_LASX_H

#include <stdint.h>
#include <lasxintrin.h>

/**
 * MAJOR version: Macro usage changes.
 * MINOR version: Add new functions, or bug fix.
 * MICRO version: Comment changes or implementation changes.
 */
#define LSOM_LASX_VERSION_MAJOR 0
#define LSOM_LASX_VERSION_MINOR 7
#define LSOM_LASX_VERSION_MICRO 0

#define LASX_DUP2_ARG1(_LASX_INS, _IN0, _IN1, _OUT0, _OUT1) \
{ \
    _OUT0 = _LASX_INS(_IN0); \
    _OUT1 = _LASX_INS(_IN1); \
}

#define LASX_DUP2_ARG2(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1) \
{ \
    _OUT0 = _LASX_INS(_IN0, _IN1); \
    _OUT1 = _LASX_INS(_IN2, _IN3); \
}

#define LASX_DUP2_ARG3(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _OUT0, _OUT1) \
{ \
    _OUT0 = _LASX_INS(_IN0, _IN1, _IN2); \
    _OUT1 = _LASX_INS(_IN3, _IN4, _IN5); \
}

#define LASX_DUP4_ARG1(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LASX_DUP2_ARG1(_LASX_INS, _IN0, _IN1, _OUT0, _OUT1); \
    LASX_DUP2_ARG1(_LASX_INS, _IN2, _IN3, _OUT2, _OUT3); \
}

#define LASX_DUP4_ARG2(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _IN6, _IN7, \
                       _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LASX_DUP2_ARG2(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _OUT0, _OUT1); \
    LASX_DUP2_ARG2(_LASX_INS, _IN4, _IN5, _IN6, _IN7, _OUT2, _OUT3); \
}

#define LASX_DUP4_ARG3(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _IN4, _IN5, _IN6, _IN7, \
                       _IN8, _IN9, _IN10, _IN11, _OUT0, _OUT1, _OUT2, _OUT3) \
{ \
    LASX_DUP2_ARG3(_LASX_INS, _IN0, _IN1, _IN2, _IN3, _IN4,  _IN5,  _OUT0, _OUT1); \
    LASX_DUP2_ARG3(_LASX_INS, _IN6, _IN7, _IN8, _IN9, _IN10, _IN11, _OUT2, _OUT3); \
}

/*
 * =============================================================================
 * Description : Dot product of byte vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Return Type - signed halfword
 * Details     : Unsigned byte elements from in_h are multiplied with
 *               unsigned byte elements from in_l producing a result
 *               twice the size of input i.e. signed halfword.
 *               Then this multiplied results of adjacent odd-even elements
 *               are added to the out vector
 * Example     : See out = __lasx_xvdp2_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2_h_bu(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_h_bu(in_h, in_l);
    out = __lasx_xvmaddwod_h_bu(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of byte vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Return Type - signed halfword
 * Details     : Signed byte elements from in_h are multiplied with
 *               signed byte elements from in_l producing a result
 *               twice the size of input i.e. signed halfword.
 *               Then this iniplication results of adjacent odd-even elements
 *               are added to the out vector
 * Example     : See out = __lasx_xvdp2_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2_h_b(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_h_b(in_h, in_l);
    out = __lasx_xvmaddwod_h_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Signed halfword elements from in_h are multiplied with
 *               signed halfword elements from in_l producing a result
 *               twice the size of input i.e. signed word.
 *               Then this multiplied results of adjacent odd-even elements
 *               are added to the out vector.
 * Example     : out = __lasx_xvdp2_w_h(in_h, in_l)
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1
 *         out : 22,38,38,22, 22,38,38,22
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2_w_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_w_h(in_h, in_l);
    out = __lasx_xvmaddwod_w_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of word vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Retrun Type - signed double
 * Details     : Signed word elements from in_h are multiplied with
 *               signed word elements from in_l producing a result
 *               twice the size of input i.e. signed double word.
 *               Then this multiplied results of adjacent odd-even elements
 *               are added to the out vector.
 * Example     : See out = __lasx_xvdp2_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2_d_w(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_d_w(in_h, in_l);
    out = __lasx_xvmaddwod_d_w(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Unsigned halfword elements from in_h are multiplied with
 *               signed halfword elements from in_l producing a result
 *               twice the size of input i.e. unsigned word.
 *               Multiplication result of adjacent odd-even elements
 *               are added to the out vector
 * Example     : See out = __lasx_xvdp2_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2_w_hu_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_w_hu_h(in_h, in_l);
    out = __lasx_xvmaddwod_w_hu_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product & addition of byte vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from in_h are multiplied with
 *               signed byte elements from in_l producing a result
 *               twice the size of input i.e. signed halfword.
 *               Then this multiplied results of adjacent odd-even elements
 *               are added to the in_c vector.
 * Example     : See out = __lasx_xvdp2add_w_h(in_c, in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2add_h_b(__m256i in_c,__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmaddwev_h_b(in_c, in_h, in_l);
    out = __lasx_xvmaddwod_h_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 *               Return Type - per RTYPE
 * Details     : Signed halfword elements from in_h are multiplied with
 *               signed halfword elements from in_l producing a result
 *               twice the size of input i.e. signed word.
 *               Multiplication result of adjacent odd-even elements
 *               are added to the in_c vector.
 * Example     : out = __lasx_xvdp2add_w_h(in_c, in_h, in_l)
 *        in_c : 1,2,3,4, 1,2,3,4
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8,
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1,
 *         out : 23,40,41,26, 23,40,41,26
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2add_w_h(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmaddwev_w_h(in_c, in_h, in_l);
    out = __lasx_xvmaddwod_w_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Unsigned halfword elements from in_h are multiplied with
 *               unsigned halfword elements from in_l producing a result
 *               twice the size of input i.e. signed word.
 *               Multiplication result of adjacent odd-even elements
 *               are added to the in_c vector.
 * Example     : See out = __lasx_xvdp2add_w_h(in_c, in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2add_w_hu(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmaddwev_w_hu(in_c, in_h, in_l);
    out = __lasx_xvmaddwod_w_hu(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Unsigned halfword elements from in_h are multiplied with
 *               signed halfword elements from in_l producing a result
 *               twice the size of input i.e. signed word.
 *               Multiplication result of adjacent odd-even elements
 *               are added to the in_c vector
 * Example     : See out = __lasx_xvdp2add_w_h(in_c, in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2add_w_hu_h(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmaddwev_w_hu_h(in_c, in_h, in_l);
    out = __lasx_xvmaddwod_w_hu_h(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : Vector Unsigned Dot Product and Subtract
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 *               Return Type - signed halfword
 * Details     : Unsigned byte elements from in_h are multiplied with
 *               unsigned byte elements from in_l producing a result
 *               twice the size of input i.e. signed halfword.
 *               Multiplication result of adjacent odd-even elements
 *               are added together and subtracted from double width elements
 *               in_c vector.
 * Example     : See out = __lasx_xvdp2sub_w_h(in_c, in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2sub_h_bu(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_h_bu(in_h, in_l);
    out = __lasx_xvmaddwod_h_bu(out, in_h, in_l);
    out = __lasx_xvsub_h(in_c, out);
    return out;
}

/*
 * =============================================================================
 * Description : Vector Signed Dot Product and Subtract
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Signed halfword elements from in_h are multiplied with
 *               Signed halfword elements from in_l producing a result
 *               twice the size of input i.e. signed word.
 *               Multiplication result of adjacent odd-even elements
 *               are added together and subtracted from double width elements
 *               in_c vector.
 * Example     : out = __lasx_xvdp2sub_w_h(in_c, in_h, in_l)
 *        in_c : 0,0,0,0, 0,0,0,0
 *        in_h : 3,1,3,0, 0,0,0,1, 0,0,1,1, 0,0,0,1
 *        in_l : 2,1,1,0, 1,0,0,0, 0,0,1,0, 1,0,0,1
 *         out : -7,-3,0,0, 0,-1,0,-1
 * =============================================================================
 */
static inline __m256i __lasx_xvdp2sub_w_h(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_w_h(in_h, in_l);
    out = __lasx_xvmaddwod_w_h(out, in_h, in_l);
    out = __lasx_xvsub_w(in_c, out);
    return out;
}

/*
 * =============================================================================
 * Description : Dot product of halfword vector elements
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 *               Return Type - signed word
 * Details     : Signed halfword elements from in_h are iniplied with
 *               signed halfword elements from in_l producing a result
 *               four times the size of input i.e. signed doubleword.
 *               Then this iniplication results of four adjacent elements
 *               are added together and stored to the out vector.
 * Example     : out = __lasx_xvdp4_d_h(in_h, in_l)
 *        in_h :  3,1,3,0, 0,0,0,1, 0,0,1,-1, 0,0,0,1
 *        in_l : -2,1,1,0, 1,0,0,0, 0,0,1, 0, 1,0,0,1
 *         out : -2,0,1,1
 * =============================================================================
 */
static inline __m256i __lasx_xvdp4_d_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvmulwev_w_h(in_h, in_l);
    out = __lasx_xvmaddwod_w_h(out, in_h, in_l);
    out = __lasx_xvhaddw_d_w(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The high half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are added after the
 *               higher half of the two-fold sign extension (signed byte
 *               to signed halfword) and stored to the out vector.
 * Example     : See out = __lasx_xvaddwh_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvaddwh_h_b(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvilvh_b(in_h, in_l);
    out = __lasx_xvhaddw_h_b(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The high half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are added after the
 *               higher half of the two-fold sign extension (signed halfword
 *               to signed word) and stored to the out vector.
 * Example     : out = __lasx_xvaddwh_w_h(in_h, in_l)
 *        in_h : 3, 0,3,0, 0,0,0,-1, 0,0,1,-1, 0,0,0,1
 *        in_l : 2,-1,1,2, 1,0,0, 0, 1,0,1, 0, 1,0,0,1
 *         out : 1,0,0,-1, 1,0,0, 2
 * =============================================================================
 */
 static inline __m256i __lasx_xvaddwh_w_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvilvh_h(in_h, in_l);
    out = __lasx_xvhaddw_w_h(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are added after the
 *               lower half of the two-fold sign extension (signed byte
 *               to signed halfword) and stored to the out vector.
 * Example     : See out = __lasx_xvaddwl_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvaddwl_h_b(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvilvl_b(in_h, in_l);
    out = __lasx_xvhaddw_h_b(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are added after the
 *               lower half of the two-fold sign extension (signed halfword
 *               to signed word) and stored to the out vector.
 * Example     : out = __lasx_xvaddwl_w_h(in_h, in_l)
 *        in_h : 3, 0,3,0, 0,0,0,-1, 0,0,1,-1, 0,0,0,1
 *        in_l : 2,-1,1,2, 1,0,0, 0, 1,0,1, 0, 1,0,0,1
 *         out : 5,-1,4,2, 1,0,2,-1
 * =============================================================================
 */
static inline __m256i __lasx_xvaddwl_w_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvilvl_h(in_h, in_l);
    out = __lasx_xvhaddw_w_h(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The out vector and the out vector are added after the
 *               lower half of the two-fold zero extension (unsigned byte
 *               to unsigned halfword) and stored to the out vector.
 * Example     : See out = __lasx_xvaddwl_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvaddwl_h_bu(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvilvl_b(in_h, in_l);
    out = __lasx_xvhaddw_hu_bu(out, out);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_l vector after double zero extension (unsigned byte to
 *               signed halfword)，added to the in_h vector.
 * Example     : See out = __lasx_xvaddw_w_w_h(in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvaddw_h_h_bu(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvsllwil_hu_bu(in_l, 0);
    out = __lasx_xvadd_h(in_h, out);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_l vector after double sign extension (signed halfword to
 *               signed word), added to the in_h vector.
 * Example     : out = __lasx_xvaddw_w_w_h(in_h, in_l)
 *        in_h : 0, 1,0,0, -1,0,0,1,
 *        in_l : 2,-1,1,2,  1,0,0,0, 0,0,1,0, 1,0,0,1,
 *         out : 2, 0,1,2, -1,0,1,1,
 * =============================================================================
 */
static inline __m256i __lasx_xvaddw_w_w_h(__m256i in_h, __m256i in_l)
{
    __m256i out;

    out = __lasx_xvsllwil_w_h(in_l, 0);
    out = __lasx_xvadd_w(in_h, out);
    return out;
}

/*
 * =============================================================================
 * Description : Multiplication and addition calculation after expansion
 *               of the lower half of the vector.
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are multiplied after
 *               the lower half of the two-fold sign extension (signed halfword
 *               to signed word), and the result is added to the vector in_c,
 *               then stored to the out vector.
 * Example     : out = __lasx_xvmaddwl_w_h(in_c, in_h, in_l)
 *        in_c : 1,2,3,4, 5,6,7,8
 *        in_h : 1,2,3,4, 1,2,3,4, 5,6,7,8, 5,6,7,8
 *        in_l : 200, 300, 400, 500,  2000, 3000, 4000, 5000,
 *              -200,-300,-400,-500, -2000,-3000,-4000,-5000
 *         out : 201, 602,1203,2004, -995, -1794,-2793,-3992
 * =============================================================================
 */
static inline __m256i __lasx_xvmaddwl_w_h(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i tmp0, tmp1, out;

    tmp0 = __lasx_xvsllwil_w_h(in_h, 0);
    tmp1 = __lasx_xvsllwil_w_h(in_l, 0);
    tmp0 = __lasx_xvmul_w(tmp0, tmp1);
    out  = __lasx_xvadd_w(tmp0, in_c);
    return out;
}

/*
 * =============================================================================
 * Description : Multiplication and addition calculation after expansion
 *               of the higher half of the vector.
 * Arguments   : Inputs - in_c, in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are multiplied after
 *               the higher half of the two-fold sign extension (signed
 *               halfword to signed word), and the result is added to
 *               the vector in_c, then stored to the out vector.
 * Example     : See out = __lasx_xvmaddwl_w_h(in_c, in_h, in_l)
 * =============================================================================
 */
static inline __m256i __lasx_xvmaddwh_w_h(__m256i in_c, __m256i in_h, __m256i in_l)
{
    __m256i tmp0, tmp1, out;

    tmp0 = __lasx_xvilvh_h(in_h, in_h);
    tmp1 = __lasx_xvilvh_h(in_l, in_l);
    tmp0 = __lasx_xvmulwev_w_h(tmp0, tmp1);
    out  = __lasx_xvadd_w(tmp0, in_c);
    return out;
}

/*
 * =============================================================================
 * Description : Multiplication calculation after expansion of the lower
 *               half of the vector.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are multiplied after
 *               the lower half of the two-fold sign extension (signed
 *               halfword to signed word), then stored to the out vector.
 * Example     : out = __lasx_xvmulwl_w_h(in_h, in_l)
 *        in_h : 3,-1,3,0, 0,0,0,-1, 0,0,1,-1, 0,0,0,1
 *        in_l : 2,-1,1,2, 1,0,0, 0, 0,0,1, 0, 1,0,0,1
 *         out : 6,1,3,0, 0,0,1,0
 * =============================================================================
 */
static inline __m256i __lasx_xvmulwl_w_h(__m256i in_h, __m256i in_l)
{
    __m256i tmp0, tmp1, out;

    tmp0 = __lasx_xvsllwil_w_h(in_h, 0);
    tmp1 = __lasx_xvsllwil_w_h(in_l, 0);
    out  = __lasx_xvmul_w(tmp0, tmp1);
    return out;
}

/*
 * =============================================================================
 * Description : Multiplication calculation after expansion of the lower
 *               half of the vector.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector and the in_l vector are multiplied after
 *               the lower half of the two-fold sign extension (signed
 *               halfword to signed word), then stored to the out vector.
 * Example     : out = __lasx_xvmulwh_w_h(in_h, in_l)
 *        in_h : 3,-1,3,0, 0,0,0,-1, 0,0,1,-1, 0,0,0,1
 *        in_l : 2,-1,1,2, 1,0,0, 0, 0,0,1, 0, 1,0,0,1
 *         out : 0,0,0,0, 0,0,0,1
 * =============================================================================
 */
static inline __m256i __lasx_xvmulwh_w_h(__m256i in_h, __m256i in_l)
{
    __m256i tmp0, tmp1, out;

    tmp0 = __lasx_xvilvh_h(in_h, in_h);
    tmp1 = __lasx_xvilvh_h(in_l, in_l);
    out  = __lasx_xvmulwev_w_h(tmp0, tmp1);
    return out;
}

/*
 * =============================================================================
 * Description : The low half of the vector elements are expanded and
 *               added saturately after being doubled.
 * Arguments   : Inputs - in_h, in_l
 *               Output - out
 * Details     : The in_h vector adds the in_l vector saturately after the lower
 *               half of the two-fold zero extension (unsigned byte to unsigned
 *               halfword) and the results are stored to the out vector.
 * Example     : out = __lasx_xvsaddw_hu_hu_bu(in_h, in_l)
 *        in_h : 2,65532,1,2, 1,0,0,0, 0,0,1,0, 1,0,0,1
 *        in_l : 3,6,3,0, 0,0,0,1, 0,0,1,1, 0,0,0,1, 3,18,3,0, 0,0,0,1, 0,0,1,1, 0,0,0,1
 *         out : 5,65535,4,2, 1,0,0,1, 3,18,4,0, 1,0,0,2,
 * =============================================================================
 */
static inline __m256i __lasx_xvsaddw_hu_hu_bu(__m256i in_h, __m256i in_l)
{
    __m256i tmp1, out;
    __m256i zero = {0};

    tmp1 = __lasx_xvilvl_b(zero, in_l);
    out  = __lasx_xvsadd_hu(in_h, tmp1);
    return out;
}

/*
 * =============================================================================
 * Description : Clip all halfword elements of input vector between min & max
 *               out = ((in) < (min)) ? (min) : (((in) > (max)) ? (max) : (in))
 * Arguments   : Inputs  - in    (input vector)
 *                       - min   (min threshold)
 *                       - max   (max threshold)
 *               Outputs - in    (output vector with clipped elements)
 *               Return Type - signed halfword
 * Example     : out = __lasx_xvclip_h(in, min, max)
 *          in : -8,2,280,249, -8,255,280,249, 4,4,4,4, 5,5,5,5
 *         min : 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1
 *         max : 9,9,9,9, 9,9,9,9, 9,9,9,9, 9,9,9,9
 *         out : 1,2,9,9, 1,9,9,9, 4,4,4,4, 5,5,5,5
 * =============================================================================
 */
static inline __m256i __lasx_xvclip_h(__m256i in, __m256i min, __m256i max)
{
    __m256i out;

    out = __lasx_xvmax_h(min, in);
    out = __lasx_xvmin_h(max, out);
    return out;
}

/*
 * =============================================================================
 * Description : Clip all signed halfword elements of input vector
 *               between 0 & 255
 * Arguments   : Inputs  - in   (input vector)
 *               Outputs - out  (output vector with clipped elements)
 *               Return Type - signed halfword
 * Example     : See out = __lasx_xvclamp255_w(in)
 * =============================================================================
 */
static inline __m256i __lasx_xvclip255_h(__m256i in)
{
    __m256i out;

    out = __lasx_xvmaxi_h(in, 0);
    out = __lasx_xvsat_hu(out, 7);
    return out;
}

/*
 * =============================================================================
 * Description : Clip all signed word elements of input vector
 *               between 0 & 255
 * Arguments   : Inputs - in   (input vector)
 *               Output - out  (output vector with clipped elements)
 *               Return Type - signed word
 * Example     : out = __lasx_xvclamp255_w(in)
 *          in : -8,255,280,249, -8,255,280,249
 *         out :  0,255,255,249,  0,255,255,249
 * =============================================================================
 */
static inline __m256i __lasx_xvclip255_w(__m256i in)
{
    __m256i out;

    out = __lasx_xvmaxi_w(in, 0);
    out = __lasx_xvsat_wu(out, 7);
    return out;
}

/*
 * =============================================================================
 * Description : Indexed halfword element values are replicated to all
 *               elements in output vector. If 'indx < 8' use xvsplati_l_*,
 *               if 'indx >= 8' use xvsplati_h_*.
 * Arguments   : Inputs - in, idx
 *               Output - out
 * Details     : Idx element value from in vector is replicated to all
 *               elements in out vector.
 *               Valid index range for halfword operation is 0-7
 * Example     : out = __lasx_xvsplati_l_h(in, idx)
 *          in : 20,10,11,12, 13,14,15,16, 0,0,2,0, 0,0,0,0
 *         idx : 0x02
 *         out : 11,11,11,11, 11,11,11,11, 11,11,11,11, 11,11,11,11
 * =============================================================================
 */
static inline __m256i __lasx_xvsplati_l_h(__m256i in, int idx)
{
    __m256i out;

    out = __lasx_xvpermi_q(in, in, 0x02);
    out = __lasx_xvreplve_h(out, idx);
    return out;
}

/*
 * =============================================================================
 * Description : Indexed halfword element values are replicated to all
 *               elements in output vector. If 'indx < 8' use xvsplati_l_*,
 *               if 'indx >= 8' use xvsplati_h_*.
 * Arguments   : Inputs - in, idx
 *               Output - out
 * Details     : Idx element value from in vector is replicated to all
 *               elements in out vector.
 *               Valid index range for halfword operation is 0-7
 * Example     : out = __lasx_xvsplati_h_h(in, idx)
 *          in : 20,10,11,12, 13,14,15,16, 0,2,0,0, 0,0,0,0
 *         idx : 0x09
 *         out : 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2
 * =============================================================================
 */

static inline __m256i __lasx_xvsplati_h_h(__m256i in, int idx)
{
    __m256i out;

    out = __lasx_xvpermi_q(in, in, 0x13);
    out = __lasx_xvreplve_h(out, idx);
    return out;
}

/*
 * =============================================================================
 * Description : Transpose 2x2 block with quad word elements in vectors
 * Arguments   : Inputs  - _in0, _in1
 *               Outputs - _out0, _out1
 * Example     : LASX_TRANSPOSE2x2_Q
 *        _in0 : 1,2
 *        _in1 : 3,4
 *
 *       _out0 : 1,3
 *       _out1 : 2,4
 * =============================================================================
 */
#define LASX_TRANSPOSE2x2_Q(_in0, _in1, _out0, _out1)                  \
{                                                                      \
    _out0 = __lasx_xvpermi_q(_in0, _in1, 0x02);                        \
    _out1 = __lasx_xvpermi_q(_in0, _in1, 0x13);                        \
}

/*
 * =============================================================================
 * Description : Transpose 4x4 block with double word elements in vectors
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3
 *               Outputs - _out0, _out1, _out2, _out3
 * Example     : LASX_TRANSPOSE4x4_D
 *         _in0 : 1,2,3,4
 *         _in1 : 1,2,3,4
 *         _in2 : 1,2,3,4
 *         _in3 : 1,2,3,4
 *
 *        _out0 : 1,1,1,1
 *        _out1 : 2,2,2,2
 *        _out2 : 3,3,3,3
 *        _out3 : 4,4,4,4
 * =============================================================================
 */
#define LASX_TRANSPOSE4x4_D(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                               \
    __m256i _tmp0, _tmp1, _tmp2, _tmp3;                                         \
    _tmp0 = __lasx_xvilvl_d(_in1, _in0);                                        \
    _tmp1 = __lasx_xvilvh_d(_in1, _in0);                                        \
    _tmp2 = __lasx_xvilvl_d(_in3, _in2);                                        \
    _tmp3 = __lasx_xvilvh_d(_in3, _in2);                                        \
    _out0 = __lasx_xvpermi_q(_tmp2, _tmp0, 0x20);                               \
    _out2 = __lasx_xvpermi_q(_tmp2, _tmp0, 0x31);                               \
    _out1 = __lasx_xvpermi_q(_tmp3, _tmp1, 0x20);                               \
    _out3 = __lasx_xvpermi_q(_tmp3, _tmp1, 0x31);                               \
}

/*
 * =============================================================================
 * Description : Transpose 8x8 block with word elements in vectors
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7
 *               Outputs - _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7
 * Example     : LASX_TRANSPOSE8x8_W
 *         _in0 : 1,2,3,4,5,6,7,8
 *         _in1 : 2,2,3,4,5,6,7,8
 *         _in2 : 3,2,3,4,5,6,7,8
 *         _in3 : 4,2,3,4,5,6,7,8
 *         _in4 : 5,2,3,4,5,6,7,8
 *         _in5 : 6,2,3,4,5,6,7,8
 *         _in6 : 7,2,3,4,5,6,7,8
 *         _in7 : 8,2,3,4,5,6,7,8
 *
 *        _out0 : 1,2,3,4,5,6,7,8
 *        _out1 : 2,2,2,2,2,2,2,2
 *        _out2 : 3,3,3,3,3,3,3,3
 *        _out3 : 4,4,4,4,4,4,4,4
 *        _out4 : 5,5,5,5,5,5,5,5
 *        _out5 : 6,6,6,6,6,6,6,6
 *        _out6 : 7,7,7,7,7,7,7,7
 *        _out7 : 8,8,8,8,8,8,8,8
 * =============================================================================
 */
#define LASX_TRANSPOSE8x8_W(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,         \
                            _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7) \
{                                                                                   \
    __m256i _s0_m, _s1_m;                                                           \
    __m256i _tmp0_m, _tmp1_m, _tmp2_m, _tmp3_m;                                     \
    __m256i _tmp4_m, _tmp5_m, _tmp6_m, _tmp7_m;                                     \
                                                                                    \
    _s0_m   = __lasx_xvilvl_w(_in2, _in0);                                          \
    _s1_m   = __lasx_xvilvl_w(_in3, _in1);                                          \
    _tmp0_m = __lasx_xvilvl_w(_s1_m, _s0_m);                                        \
    _tmp1_m = __lasx_xvilvh_w(_s1_m, _s0_m);                                        \
    _s0_m   = __lasx_xvilvh_w(_in2, _in0);                                          \
    _s1_m   = __lasx_xvilvh_w(_in3, _in1);                                          \
    _tmp2_m = __lasx_xvilvl_w(_s1_m, _s0_m);                                        \
    _tmp3_m = __lasx_xvilvh_w(_s1_m, _s0_m);                                        \
    _s0_m   = __lasx_xvilvl_w(_in6, _in4);                                          \
    _s1_m   = __lasx_xvilvl_w(_in7, _in5);                                          \
    _tmp4_m = __lasx_xvilvl_w(_s1_m, _s0_m);                                        \
    _tmp5_m = __lasx_xvilvh_w(_s1_m, _s0_m);                                        \
    _s0_m   = __lasx_xvilvh_w(_in6, _in4);                                          \
    _s1_m   = __lasx_xvilvh_w(_in7, _in5);                                          \
    _tmp6_m = __lasx_xvilvl_w(_s1_m, _s0_m);                                        \
    _tmp7_m = __lasx_xvilvh_w(_s1_m, _s0_m);                                        \
    _out0 = __lasx_xvpermi_q(_tmp4_m, _tmp0_m, 0x20);                               \
    _out1 = __lasx_xvpermi_q(_tmp5_m, _tmp1_m, 0x20);                               \
    _out2 = __lasx_xvpermi_q(_tmp6_m, _tmp2_m, 0x20);                               \
    _out3 = __lasx_xvpermi_q(_tmp7_m, _tmp3_m, 0x20);                               \
    _out4 = __lasx_xvpermi_q(_tmp4_m, _tmp0_m, 0x31);                               \
    _out5 = __lasx_xvpermi_q(_tmp5_m, _tmp1_m, 0x31);                               \
    _out6 = __lasx_xvpermi_q(_tmp6_m, _tmp2_m, 0x31);                               \
    _out7 = __lasx_xvpermi_q(_tmp7_m, _tmp3_m, 0x31);                               \
}

/*
 * =============================================================================
 * Description : Transpose input 16x8 byte block
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,
 *                         _in8, _in9, _in10, _in11, _in12, _in13, _in14, _in15
 *                         (input 16x8 byte block)
 *               Outputs - _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7
 *                         (output 8x16 byte block)
 * Details     : The rows of the matrix become columns, and the columns become rows.
 * Example     : See LASX_TRANSPOSE16x8_H
 * =============================================================================
 */
#define LASX_TRANSPOSE16x8_B(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,         \
                             _in8, _in9, _in10, _in11, _in12, _in13, _in14, _in15,   \
                             _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7) \
{                                                                                    \
    __m256i _tmp0_m, _tmp1_m, _tmp2_m, _tmp3_m;                                      \
    __m256i _tmp4_m, _tmp5_m, _tmp6_m, _tmp7_m;                                      \
    __m256i _t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;                                  \
                                                                                     \
    _tmp0_m = __lasx_xvilvl_b(_in2, _in0);                                           \
    _tmp1_m = __lasx_xvilvl_b(_in3, _in1);                                           \
    _tmp2_m = __lasx_xvilvl_b(_in6, _in4);                                           \
    _tmp3_m = __lasx_xvilvl_b(_in7, _in5);                                           \
    _tmp4_m = __lasx_xvilvl_b(_in10, _in8);                                          \
    _tmp5_m = __lasx_xvilvl_b(_in11, _in9);                                          \
    _tmp6_m = __lasx_xvilvl_b(_in14, _in12);                                         \
    _tmp7_m = __lasx_xvilvl_b(_in15, _in13);                                         \
    _t0 = __lasx_xvilvl_b(_tmp1_m, _tmp0_m);                                         \
    _t1 = __lasx_xvilvh_b(_tmp1_m, _tmp0_m);                                         \
    _t2 = __lasx_xvilvl_b(_tmp3_m, _tmp2_m);                                         \
    _t3 = __lasx_xvilvh_b(_tmp3_m, _tmp2_m);                                         \
    _t4 = __lasx_xvilvl_b(_tmp5_m, _tmp4_m);                                         \
    _t5 = __lasx_xvilvh_b(_tmp5_m, _tmp4_m);                                         \
    _t6 = __lasx_xvilvl_b(_tmp7_m, _tmp6_m);                                         \
    _t7 = __lasx_xvilvh_b(_tmp7_m, _tmp6_m);                                         \
    _tmp0_m = __lasx_xvilvl_w(_t2, _t0);                                             \
    _tmp2_m = __lasx_xvilvh_w(_t2, _t0);                                             \
    _tmp4_m = __lasx_xvilvl_w(_t3, _t1);                                             \
    _tmp6_m = __lasx_xvilvh_w(_t3, _t1);                                             \
    _tmp1_m = __lasx_xvilvl_w(_t6, _t4);                                             \
    _tmp3_m = __lasx_xvilvh_w(_t6, _t4);                                             \
    _tmp5_m = __lasx_xvilvl_w(_t7, _t5);                                             \
    _tmp7_m = __lasx_xvilvh_w(_t7, _t5);                                             \
    _out0 = __lasx_xvilvl_d(_tmp1_m, _tmp0_m);                                       \
    _out1 = __lasx_xvilvh_d(_tmp1_m, _tmp0_m);                                       \
    _out2 = __lasx_xvilvl_d(_tmp3_m, _tmp2_m);                                       \
    _out3 = __lasx_xvilvh_d(_tmp3_m, _tmp2_m);                                       \
    _out4 = __lasx_xvilvl_d(_tmp5_m, _tmp4_m);                                       \
    _out5 = __lasx_xvilvh_d(_tmp5_m, _tmp4_m);                                       \
    _out6 = __lasx_xvilvl_d(_tmp7_m, _tmp6_m);                                       \
    _out7 = __lasx_xvilvh_d(_tmp7_m, _tmp6_m);                                       \
}

/*
 * =============================================================================
 * Description : Transpose input 16x8 byte block
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,
 *                         _in8, _in9, _in10, _in11, _in12, _in13, _in14, _in15
 *                         (input 16x8 byte block)
 *               Outputs - _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7
 *                         (output 8x16 byte block)
 * Details     : The rows of the matrix become columns, and the columns become rows.
 * Example     : LASX_TRANSPOSE16x8_H
 *        _in0 : 1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in1 : 2,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in2 : 3,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in3 : 4,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in4 : 5,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in5 : 6,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in6 : 7,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in7 : 8,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in8 : 9,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *        _in9 : 1,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in10 : 0,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in11 : 2,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in12 : 3,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in13 : 7,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in14 : 5,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *       _in15 : 6,2,3,4,5,6,7,8,0,0,0,0,0,0,0,0
 *
 *       _out0 : 1,2,3,4,5,6,7,8,9,1,0,2,3,7,5,6
 *       _out1 : 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
 *       _out2 : 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
 *       _out3 : 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
 *       _out4 : 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
 *       _out5 : 6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6
 *       _out6 : 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
 *       _out7 : 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
 * =============================================================================
 */
#define LASX_TRANSPOSE16x8_H(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,         \
                             _in8, _in9, _in10, _in11, _in12, _in13, _in14, _in15,   \
                             _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7) \
   {                                                                                 \
    __m256i _tmp0_m, _tmp1_m, _tmp2_m, _tmp3_m;                                      \
    __m256i _tmp4_m, _tmp5_m, _tmp6_m, _tmp7_m;                                      \
    __m256i _t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;                                  \
                                                                                     \
    _tmp0_m = __lasx_xvilvl_h(_in2, _in0);                                           \
    _tmp1_m = __lasx_xvilvl_h(_in3, _in1);                                           \
    _tmp2_m = __lasx_xvilvl_h(_in6, _in4);                                           \
    _tmp3_m = __lasx_xvilvl_h(_in7, _in5);                                           \
    _tmp4_m = __lasx_xvilvl_h(_in10, _in8);                                          \
    _tmp5_m = __lasx_xvilvl_h(_in11, _in9);                                          \
    _tmp6_m = __lasx_xvilvl_h(_in14, _in12);                                         \
    _tmp7_m = __lasx_xvilvl_h(_in15, _in13);                                         \
    _t0 = __lasx_xvilvl_h(_tmp1_m, _tmp0_m);                                         \
    _t1 = __lasx_xvilvh_h(_tmp1_m, _tmp0_m);                                         \
    _t2 = __lasx_xvilvl_h(_tmp3_m, _tmp2_m);                                         \
    _t3 = __lasx_xvilvh_h(_tmp3_m, _tmp2_m);                                         \
    _t4 = __lasx_xvilvl_h(_tmp5_m, _tmp4_m);                                         \
    _t5 = __lasx_xvilvh_h(_tmp5_m, _tmp4_m);                                         \
    _t6 = __lasx_xvilvl_h(_tmp7_m, _tmp6_m);                                         \
    _t7 = __lasx_xvilvh_h(_tmp7_m, _tmp6_m);                                         \
    _tmp0_m = __lasx_xvilvl_d(_t2, _t0);                                             \
    _tmp2_m = __lasx_xvilvh_d(_t2, _t0);                                             \
    _tmp4_m = __lasx_xvilvl_d(_t3, _t1);                                             \
    _tmp6_m = __lasx_xvilvh_d(_t3, _t1);                                             \
    _tmp1_m = __lasx_xvilvl_d(_t6, _t4);                                             \
    _tmp3_m = __lasx_xvilvh_d(_t6, _t4);                                             \
    _tmp5_m = __lasx_xvilvl_d(_t7, _t5);                                             \
    _tmp7_m = __lasx_xvilvh_d(_t7, _t5);                                             \
    _out0 = __lasx_xvpermi_q(_tmp1_m, _tmp0_m, 0x20);                                \
    _out1 = __lasx_xvpermi_q(_tmp3_m, _tmp2_m, 0x20);                                \
    _out2 = __lasx_xvpermi_q(_tmp5_m, _tmp4_m, 0x20);                                \
    _out3 = __lasx_xvpermi_q(_tmp7_m, _tmp6_m, 0x20);                                \
                                                                                     \
    _tmp0_m = __lasx_xvilvh_h(_in2, _in0);                                           \
    _tmp1_m = __lasx_xvilvh_h(_in3, _in1);                                           \
    _tmp2_m = __lasx_xvilvh_h(_in6, _in4);                                           \
    _tmp3_m = __lasx_xvilvh_h(_in7, _in5);                                           \
    _tmp4_m = __lasx_xvilvh_h(_in10, _in8);                                          \
    _tmp5_m = __lasx_xvilvh_h(_in11, _in9);                                          \
    _tmp6_m = __lasx_xvilvh_h(_in14, _in12);                                         \
    _tmp7_m = __lasx_xvilvh_h(_in15, _in13);                                         \
    _t0 = __lasx_xvilvl_h(_tmp1_m, _tmp0_m);                                         \
    _t1 = __lasx_xvilvh_h(_tmp1_m, _tmp0_m);                                         \
    _t2 = __lasx_xvilvl_h(_tmp3_m, _tmp2_m);                                         \
    _t3 = __lasx_xvilvh_h(_tmp3_m, _tmp2_m);                                         \
    _t4 = __lasx_xvilvl_h(_tmp5_m, _tmp4_m);                                         \
    _t5 = __lasx_xvilvh_h(_tmp5_m, _tmp4_m);                                         \
    _t6 = __lasx_xvilvl_h(_tmp7_m, _tmp6_m);                                         \
    _t7 = __lasx_xvilvh_h(_tmp7_m, _tmp6_m);                                         \
    _tmp0_m = __lasx_xvilvl_d(_t2, _t0);                                             \
    _tmp2_m = __lasx_xvilvh_d(_t2, _t0);                                             \
    _tmp4_m = __lasx_xvilvl_d(_t3, _t1);                                             \
    _tmp6_m = __lasx_xvilvh_d(_t3, _t1);                                             \
    _tmp1_m = __lasx_xvilvl_d(_t6, _t4);                                             \
    _tmp3_m = __lasx_xvilvh_d(_t6, _t4);                                             \
    _tmp5_m = __lasx_xvilvl_d(_t7, _t5);                                             \
    _tmp7_m = __lasx_xvilvh_d(_t7, _t5);                                             \
    _out4 = __lasx_xvpermi_q(_tmp1_m, _tmp0_m, 0x20);                                \
    _out5 = __lasx_xvpermi_q(_tmp3_m, _tmp2_m, 0x20);                                \
    _out6 = __lasx_xvpermi_q(_tmp5_m, _tmp4_m, 0x20);                                \
    _out7 = __lasx_xvpermi_q(_tmp7_m, _tmp6_m, 0x20);                                \
}

/*
 * =============================================================================
 * Description : Transpose 4x4 block with halfword elements in vectors
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3
 *               Outputs - _out0, _out1, _out2, _out3
 *               Return Type - signed halfword
 * Details     : The rows of the matrix become columns, and the columns become rows.
 * Example     : See LASX_TRANSPOSE8x8_H_128SV
 * =============================================================================
 */
#define LASX_TRANSPOSE4x4_H_128SV(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3) \
{                                                                                     \
    __m256i _s0_m, _s1_m;                                                             \
                                                                                      \
    _s0_m = __lasx_xvilvl_h(_in1, _in0);                                              \
    _s1_m = __lasx_xvilvl_h(_in3, _in2);                                              \
    _out0 = __lasx_xvilvl_w(_s1_m, _s0_m);                                            \
    _out2 = __lasx_xvilvh_w(_s1_m, _s0_m);                                            \
    _out1 = __lasx_xvilvh_d(_out0, _out0);                                            \
    _out3 = __lasx_xvilvh_d(_out2, _out2);                                            \
}

/*
 * =============================================================================
 * Description : Transpose input 8x8 byte block
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7
 *                         (input 8x8 byte block)
 *               Outputs - _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7
 *                         (output 8x8 byte block)
 * Example     : See LASX_TRANSPOSE8x8_H_128SV
 * =============================================================================
 */
#define LASX_TRANSPOSE8x8_B_128SV(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7, _out0,\
                                  _out1, _out2, _out3, _out4, _out5, _out6, _out7)      \
{                                                                                       \
    __m256i _tmp0_m, _tmp1_m, _tmp2_m, _tmp3_m;                                         \
    __m256i _tmp4_m, _tmp5_m, _tmp6_m, _tmp7_m;                                         \
    _tmp0_m = __lasx_xvilvl_b(_in2, _in0);                                              \
    _tmp1_m = __lasx_xvilvl_b(_in3, _in1);                                              \
    _tmp2_m = __lasx_xvilvl_b(_in6, _in4);                                              \
    _tmp3_m = __lasx_xvilvl_b(_in7, _in5);                                              \
    _tmp4_m = __lasx_xvilvl_b(_tmp1_m, _tmp0_m);                                        \
    _tmp5_m = __lasx_xvilvh_b(_tmp1_m, _tmp0_m);                                        \
    _tmp6_m = __lasx_xvilvl_b(_tmp3_m, _tmp2_m);                                        \
    _tmp7_m = __lasx_xvilvh_b(_tmp3_m, _tmp2_m);                                        \
    _out0 = __lasx_xvilvl_w(_tmp6_m, _tmp4_m);                                          \
    _out2 = __lasx_xvilvh_w(_tmp6_m, _tmp4_m);                                          \
    _out4 = __lasx_xvilvl_w(_tmp7_m, _tmp5_m);                                          \
    _out6 = __lasx_xvilvh_w(_tmp7_m, _tmp5_m);                                          \
    _out1 = __lasx_xvbsrl_v(_out0, 8);                                                  \
    _out3 = __lasx_xvbsrl_v(_out2, 8);                                                  \
    _out5 = __lasx_xvbsrl_v(_out4, 8);                                                  \
    _out7 = __lasx_xvbsrl_v(_out6, 8);                                                  \
}

/*
 * =============================================================================
 * Description : Transpose 8x8 block with halfword elements in vectors.
 * Arguments   : Inputs  - _in0, _in1, ~
 *               Outputs - _out0, _out1, ~
 * Details     : The rows of the matrix become columns, and the columns become rows.
 * Example     : LASX_TRANSPOSE8x8_H_128SV
 *        _in0 : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        _in1 : 8,2,3,4, 5,6,7,8, 8,2,3,4, 5,6,7,8
 *        _in2 : 8,2,3,4, 5,6,7,8, 8,2,3,4, 5,6,7,8
 *        _in3 : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        _in4 : 9,2,3,4, 5,6,7,8, 9,2,3,4, 5,6,7,8
 *        _in5 : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        _in6 : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        _in7 : 9,2,3,4, 5,6,7,8, 9,2,3,4, 5,6,7,8
 *
 *       _out0 : 1,8,8,1, 9,1,1,9, 1,8,8,1, 9,1,1,9
 *       _out1 : 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2
 *       _out2 : 3,3,3,3, 3,3,3,3, 3,3,3,3, 3,3,3,3
 *       _out3 : 4,4,4,4, 4,4,4,4, 4,4,4,4, 4,4,4,4
 *       _out4 : 5,5,5,5, 5,5,5,5, 5,5,5,5, 5,5,5,5
 *       _out5 : 6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6
 *       _out6 : 7,7,7,7, 7,7,7,7, 7,7,7,7, 7,7,7,7
 *       _out7 : 8,8,8,8, 8,8,8,8, 8,8,8,8, 8,8,8,8
 * =============================================================================
 */
#define LASX_TRANSPOSE8x8_H_128SV(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7, _out0,\
                                  _out1, _out2, _out3, _out4, _out5, _out6, _out7)      \
{                                                                                       \
    __m256i _s0_m, _s1_m;                                                               \
    __m256i _tmp0_m, _tmp1_m, _tmp2_m, _tmp3_m;                                         \
    __m256i _tmp4_m, _tmp5_m, _tmp6_m, _tmp7_m;                                         \
                                                                                        \
    _s0_m   = __lasx_xvilvl_h(_in6, _in4);                                              \
    _s1_m   = __lasx_xvilvl_h(_in7, _in5);                                              \
    _tmp0_m = __lasx_xvilvl_h(_s1_m, _s0_m);                                            \
    _tmp1_m = __lasx_xvilvh_h(_s1_m, _s0_m);                                            \
    _s0_m   = __lasx_xvilvh_h(_in6, _in4);                                              \
    _s1_m   = __lasx_xvilvh_h(_in7, _in5);                                              \
    _tmp2_m = __lasx_xvilvl_h(_s1_m, _s0_m);                                            \
    _tmp3_m = __lasx_xvilvh_h(_s1_m, _s0_m);                                            \
                                                                                        \
    _s0_m   = __lasx_xvilvl_h(_in2, _in0);                                              \
    _s1_m   = __lasx_xvilvl_h(_in3, _in1);                                              \
    _tmp4_m = __lasx_xvilvl_h(_s1_m, _s0_m);                                            \
    _tmp5_m = __lasx_xvilvh_h(_s1_m, _s0_m);                                            \
    _s0_m   = __lasx_xvilvh_h(_in2, _in0);                                              \
    _s1_m   = __lasx_xvilvh_h(_in3, _in1);                                              \
    _tmp6_m = __lasx_xvilvl_h(_s1_m, _s0_m);                                            \
    _tmp7_m = __lasx_xvilvh_h(_s1_m, _s0_m);                                            \
                                                                                        \
    _out0 = __lasx_xvpickev_d(_tmp0_m, _tmp4_m);                                        \
    _out2 = __lasx_xvpickev_d(_tmp1_m, _tmp5_m);                                        \
    _out4 = __lasx_xvpickev_d(_tmp2_m, _tmp6_m);                                        \
    _out6 = __lasx_xvpickev_d(_tmp3_m, _tmp7_m);                                        \
    _out1 = __lasx_xvpickod_d(_tmp0_m, _tmp4_m);                                        \
    _out3 = __lasx_xvpickod_d(_tmp1_m, _tmp5_m);                                        \
    _out5 = __lasx_xvpickod_d(_tmp2_m, _tmp6_m);                                        \
    _out7 = __lasx_xvpickod_d(_tmp3_m, _tmp7_m);                                        \
}

/*
 * =============================================================================
 * Description : Butterfly of 4 input vectors
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3
 *               Outputs - _out0, _out1, _out2, _out3
 * Details     : Butterfly operation
 * Example     : LASX_BUTTERFLY_4
 *               _out0 = _in0 + _in3;
 *               _out1 = _in1 + _in2;
 *               _out2 = _in1 - _in2;
 *               _out3 = _in0 - _in3;
 * =============================================================================
 */
#define LASX_BUTTERFLY_4_B(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3)  \
{                                                                               \
    _out0 = __lasx_xvadd_b(_in0, _in3);                                         \
    _out1 = __lasx_xvadd_b(_in1, _in2);                                         \
    _out2 = __lasx_xvsub_b(_in1, _in2);                                         \
    _out3 = __lasx_xvsub_b(_in0, _in3);                                         \
}
#define LASX_BUTTERFLY_4_H(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3)  \
{                                                                               \
    _out0 = __lasx_xvadd_h(_in0, _in3);                                         \
    _out1 = __lasx_xvadd_h(_in1, _in2);                                         \
    _out2 = __lasx_xvsub_h(_in1, _in2);                                         \
    _out3 = __lasx_xvsub_h(_in0, _in3);                                         \
}
#define LASX_BUTTERFLY_4_W(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3)  \
{                                                                               \
    _out0 = __lasx_xvadd_w(_in0, _in3);                                         \
    _out1 = __lasx_xvadd_w(_in1, _in2);                                         \
    _out2 = __lasx_xvsub_w(_in1, _in2);                                         \
    _out3 = __lasx_xvsub_w(_in0, _in3);                                         \
}
#define LASX_BUTTERFLY_4_D(_in0, _in1, _in2, _in3, _out0, _out1, _out2, _out3)  \
{                                                                               \
    _out0 = __lasx_xvadd_d(_in0, _in3);                                         \
    _out1 = __lasx_xvadd_d(_in1, _in2);                                         \
    _out2 = __lasx_xvsub_d(_in1, _in2);                                         \
    _out3 = __lasx_xvsub_d(_in0, _in3);                                         \
}

/*
 * =============================================================================
 * Description : Butterfly of 8 input vectors
 * Arguments   : Inputs  - _in0, _in1, _in2, _in3, ~
 *               Outputs - _out0, _out1, _out2, _out3, ~
 * Details     : Butterfly operation
 * Example     : LASX_BUTTERFLY_8
 *               _out0 = _in0 + _in7;
 *               _out1 = _in1 + _in6;
 *               _out2 = _in2 + _in5;
 *               _out3 = _in3 + _in4;
 *               _out4 = _in3 - _in4;
 *               _out5 = _in2 - _in5;
 *               _out6 = _in1 - _in6;
 *               _out7 = _in0 - _in7;
 * =============================================================================
 */
#define LASX_BUTTERFLY_8_B(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,        \
                           _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7)\
{                                                                                 \
    _out0 = __lasx_xvadd_b(_in0, _in7);                                           \
    _out1 = __lasx_xvadd_b(_in1, _in6);                                           \
    _out2 = __lasx_xvadd_b(_in2, _in5);                                           \
    _out3 = __lasx_xvadd_b(_in3, _in4);                                           \
    _out4 = __lasx_xvsub_b(_in3, _in4);                                           \
    _out5 = __lasx_xvsub_b(_in2, _in5);                                           \
    _out6 = __lasx_xvsub_b(_in1, _in6);                                           \
    _out7 = __lasx_xvsub_b(_in0, _in7);                                           \
}

#define LASX_BUTTERFLY_8_H(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,        \
                           _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7)\
{                                                                                 \
    _out0 = __lasx_xvadd_h(_in0, _in7);                                           \
    _out1 = __lasx_xvadd_h(_in1, _in6);                                           \
    _out2 = __lasx_xvadd_h(_in2, _in5);                                           \
    _out3 = __lasx_xvadd_h(_in3, _in4);                                           \
    _out4 = __lasx_xvsub_h(_in3, _in4);                                           \
    _out5 = __lasx_xvsub_h(_in2, _in5);                                           \
    _out6 = __lasx_xvsub_h(_in1, _in6);                                           \
    _out7 = __lasx_xvsub_h(_in0, _in7);                                           \
}

#define LASX_BUTTERFLY_8_W(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,        \
                           _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7)\
{                                                                                 \
    _out0 = __lasx_xvadd_w(_in0, _in7);                                           \
    _out1 = __lasx_xvadd_w(_in1, _in6);                                           \
    _out2 = __lasx_xvadd_w(_in2, _in5);                                           \
    _out3 = __lasx_xvadd_w(_in3, _in4);                                           \
    _out4 = __lasx_xvsub_w(_in3, _in4);                                           \
    _out5 = __lasx_xvsub_w(_in2, _in5);                                           \
    _out6 = __lasx_xvsub_w(_in1, _in6);                                           \
    _out7 = __lasx_xvsub_w(_in0, _in7);                                           \
}

#define LASX_BUTTERFLY_8_D(_in0, _in1, _in2, _in3, _in4, _in5, _in6, _in7,        \
                           _out0, _out1, _out2, _out3, _out4, _out5, _out6, _out7)\
{                                                                                 \
    _out0 = __lasx_xvadd_d(_in0, _in7);                                           \
    _out1 = __lasx_xvadd_d(_in1, _in6);                                           \
    _out2 = __lasx_xvadd_d(_in2, _in5);                                           \
    _out3 = __lasx_xvadd_d(_in3, _in4);                                           \
    _out4 = __lasx_xvsub_d(_in3, _in4);                                           \
    _out5 = __lasx_xvsub_d(_in2, _in5);                                           \
    _out6 = __lasx_xvsub_d(_in1, _in6);                                           \
    _out7 = __lasx_xvsub_d(_in0, _in7);                                           \
}


#endif /* GENERIC_MACROS_LASX_H */
#endif /* AVUTIL_LOONGARCH_GENERIC_MACROS_LASX_H */

