/*
 * Loongson LASX optimized h264idct
 *
 * Copyright (c) 2022 Loongson Technology Corporation Limited
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
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

#include "loongson_asm.S"

/*
 * #define FUNC2(a, b, c)  FUNC3(a, b, c)
 * #define FUNCC(a) FUNC2(a, BIT_DEPTH, _c)
 * void FUNCC(ff_h264_idct_dc_add)(uint8_t *_dst, int16_t *_block, int stride)
 * LSX optimization is enough for this function.
 */
function ff_h264_idct_dc_add_8_lsx
    vldrepl.h     vr4,    a1,    0
    add.d         t2,     a2,    a2
    add.d         t3,     t2,    a2
    fld.s         f0,     a0,    0
    fldx.s        f1,     a0,    a2
    fldx.s        f2,     a0,    t2
    fldx.s        f3,     a0,    t3
    st.h          zero,   a1,    0

    vsrari.h      vr4,    vr4,   6
    vilvl.w       vr0,    vr1,   vr0
    vilvl.w       vr1,    vr3,   vr2
    vsllwil.hu.bu vr0,    vr0,   0
    vsllwil.hu.bu vr1,    vr1,   0
    vadd.h        vr0,    vr0,   vr4
    vadd.h        vr1,    vr1,   vr4
    vssrarni.bu.h vr1,    vr0,   0

    vbsrl.v       vr2,    vr1,   4
    vbsrl.v       vr3,    vr1,   8
    vbsrl.v       vr4,    vr1,   12
    fst.s         f1,     a0,    0
    fstx.s        f2,     a0,    a2
    fstx.s        f3,     a0,    t2
    fstx.s        f4,     a0,    t3
endfunc

/*
 * #define FUNC2(a, b, c)  FUNC3(a, b, c)
 * #define FUNCC(a) FUNC2(a, BIT_DEPTH, _c)
 * void FUNCC(ff_h264_idct8_dc_add)(uint8_t *_dst, int16_t *_block, int stride)
 */
function ff_h264_idct8_dc_add_8_lsx
    vldrepl.h     vr8,    a1,    0
    add.d         t2,     a2,    a2
    add.d         t3,     t2,    a2
    add.d         t4,     t3,    a2
    add.d         t5,     t4,    a2
    add.d         t6,     t5,    a2
    add.d         t7,     t6,    a2

    fld.d         f0,     a0,    0
    fldx.d        f1,     a0,    a2
    fldx.d        f2,     a0,    t2
    fldx.d        f3,     a0,    t3
    fldx.d        f4,     a0,    t4
    fldx.d        f5,     a0,    t5
    fldx.d        f6,     a0,    t6
    fldx.d        f7,     a0,    t7
    st.h          zero,   a1,    0

    vsrari.h      vr8,    vr8,   6
    vsllwil.hu.bu vr0,    vr0,   0
    vsllwil.hu.bu vr1,    vr1,   0
    vsllwil.hu.bu vr2,    vr2,   0
    vsllwil.hu.bu vr3,    vr3,   0
    vsllwil.hu.bu vr4,    vr4,   0
    vsllwil.hu.bu vr5,    vr5,   0
    vsllwil.hu.bu vr6,    vr6,   0
    vsllwil.hu.bu vr7,    vr7,   0
    vadd.h        vr0,    vr0,   vr8
    vadd.h        vr1,    vr1,   vr8
    vadd.h        vr2,    vr2,   vr8
    vadd.h        vr3,    vr3,   vr8
    vadd.h        vr4,    vr4,   vr8
    vadd.h        vr5,    vr5,   vr8
    vadd.h        vr6,    vr6,   vr8
    vadd.h        vr7,    vr7,   vr8
    vssrarni.bu.h vr1,    vr0,   0
    vssrarni.bu.h vr3,    vr2,   0
    vssrarni.bu.h vr5,    vr4,   0
    vssrarni.bu.h vr7,    vr6,   0

    vbsrl.v       vr0,    vr1,   8
    vbsrl.v       vr2,    vr3,   8
    vbsrl.v       vr4,    vr5,   8
    vbsrl.v       vr6,    vr7,   8
    fst.d         f1,     a0,    0
    fstx.d        f0,     a0,    a2
    fstx.d        f3,     a0,    t2
    fstx.d        f2,     a0,    t3
    fstx.d        f5,     a0,    t4
    fstx.d        f4,     a0,    t5
    fstx.d        f7,     a0,    t6
    fstx.d        f6,     a0,    t7
endfunc
function ff_h264_idct8_dc_add_8_lasx
    xvldrepl.h    xr8,    a1,    0
    add.d         t2,     a2,    a2
    add.d         t3,     t2,    a2
    add.d         t4,     t3,    a2
    add.d         t5,     t4,    a2
    add.d         t6,     t5,    a2
    add.d         t7,     t6,    a2

    fld.d         f0,     a0,    0
    fldx.d        f1,     a0,    a2
    fldx.d        f2,     a0,    t2
    fldx.d        f3,     a0,    t3
    fldx.d        f4,     a0,    t4
    fldx.d        f5,     a0,    t5
    fldx.d        f6,     a0,    t6
    fldx.d        f7,     a0,    t7
    st.h          zero,   a1,    0

    xvsrari.h     xr8,    xr8,   6
    xvpermi.q     xr1,    xr0,   0x20
    xvpermi.q     xr3,    xr2,   0x20
    xvpermi.q     xr5,    xr4,   0x20
    xvpermi.q     xr7,    xr6,   0x20
    xvsllwil.hu.bu xr1,   xr1,   0
    xvsllwil.hu.bu xr3,   xr3,   0
    xvsllwil.hu.bu xr5,   xr5,   0
    xvsllwil.hu.bu xr7,   xr7,   0
    xvadd.h       xr1,    xr1,   xr8
    xvadd.h       xr3,    xr3,   xr8
    xvadd.h       xr5,    xr5,   xr8
    xvadd.h       xr7,    xr7,   xr8

    xvssrarni.bu.h xr3,   xr1,   0
    xvssrarni.bu.h xr7,   xr5,   0

    xvpermi.q     xr1,    xr3,   0x11
    xvpermi.q     xr5,    xr7,   0x11
    xvbsrl.v      xr0,    xr1,   8
    xvbsrl.v      xr2,    xr3,   8
    xvbsrl.v      xr4,    xr5,   8
    xvbsrl.v      xr6,    xr7,   8

    fst.d         f3,     a0,    0
    fstx.d        f1,     a0,    a2
    fstx.d        f2,     a0,    t2
    fstx.d        f0,     a0,    t3
    fstx.d        f7,     a0,    t4
    fstx.d        f5,     a0,    t5
    fstx.d        f6,     a0,    t6
    fstx.d        f4,     a0,    t7
endfunc

