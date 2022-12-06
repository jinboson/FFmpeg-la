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
 */
function ff_h264_idct4x4_addblk_dc_lsx
    vldrepl.h     vr4,    a1,    0
    add.d         t2,     a2,    a2
    add.d         t3,     t2,    a2
    vld           vr0,    a0,    0
    vldx          vr1,    a0,    a2
    vldx          vr2,    a0,    t2
    vldx          vr3,    a0,    t3
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

