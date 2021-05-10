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
#define LSOM_LSX_VERSION_MINOR 1
#define LSOM_LSX_VERSION_MICRO 1

/*
 * =============================================================================
 * Description : Dot product & addition of byte vector elements
 * Arguments   : Inputs  - in_h, in_l
 *               Outputs - out
 *               Retrun Type - halfword
 * Details     : Signed byte elements from in0 are iniplied with
 *               signed byte elements from in0 producing a result
 *               twice the size of input i.e. signed halfword.
 *               Then this iniplication results of adjacent odd-even elements
 *               are added to the out vector
 *               (2 signed halfword results)
 * Example     : __lsx_dp2add_h_b(in_h, in_l, out)
 *         out : 1,2,3,4, 1,2,3,4
 *        in_h : 1,2,3,4, 5,6,7,8, 1,2,3,4, 5,6,7,8
 *        in_l : 8,7,6,5, 4,3,2,1, 8,7,6,5, 4,3,2,1
 *         out : 23,40,41,26, 23,40,41,26
 * =============================================================================
 */
static inline __m128i __lsx_dp2add_h_b(__m128i in_h, __m128i in_l, __m128i out)
{
    out = __lsx_vmaddwev_h_b(out, in_h, in_l);
    out = __lsx_vmaddwod_h_b(out, in_h, in_l);
    return out;
}

/*
 * =============================================================================
 * Description : LSX duplicate macros repeat multiple groups of the same
 *               operation '_LSX_INS'. This is used for some easy instructions.
 *               The output values are passed by return.
 * Arguments   : Inputs  - _LSX_INS
 *                       - _IN0, _IN1, ~
 *               Outputs - _OUT0, _OUT1, ~
 * Details     : General form LSX_DUPn_ARGm,
 *               n: _LSX_INS duplicate times
 *               m: number of _LSX_INS parameters
 * =============================================================================
 */
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

#endif /* GENERIC_MACROS_LSX_H */
#endif /* AVUTIL_LOONGARCH_GENERIC_MACROS_LSX_H */
