/*
 * Loongson LSX optimized h264chroma
 *
 * Copyright (c) 2022 Loongson Technology Corporation Limited
 * Contributed by Lu Wang <wanglu@loongson.cn>
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

/* void ff_put_h264_chroma_mc8_lsx(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
                                   int h, int x, int y) */
function ff_put_h264_chroma_mc8_lsx
    li.d             t8,      8
    sub.d            t1,      t8,     a4     // 8-x
    sub.d            t2,      t8,     a5     // 8-y
    mul.d            t3,      t1,     t2     // A
    mul.d            t4,      a4,     t2     // B
    mul.d            t5,      t1,     a5     // C
    mul.d            t6,      a4,     a5     // D
    add.d            t0,      t4,     t5     // E
    vreplgr2vr.b     vr0,     t3
    vreplgr2vr.b     vr1,     t4
    vreplgr2vr.b     vr2,     t5
    vreplgr2vr.b     vr3,     t6
    vreplgr2vr.b     vr4,     t0
    slli.d           t2,      a2,     1
    add.d            t3,      t2,     a2
    slli.d           t4,      a2,     2

    bge              zero,    t6,     .ENDLOOP_D
    move             t1,      a3
    vilvl.b          vr9,     vr1,    vr0
    vilvl.b          vr10,    vr3,    vr2
.LOOP_D:
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    add.d            a1,      a1,     a2
    vld              vr7,     a1,     0
    vld              vr8,     a1,     1
    vilvl.b          vr11,    vr6,    vr5
    vilvl.b          vr12,    vr8,    vr7
    vmulwev.h.bu     vr13,    vr9,    vr11
    vmaddwod.h.bu    vr13,    vr9,    vr11
    vmulwev.h.bu     vr14,    vr10,   vr12
    vmaddwod.h.bu    vr14,    vr10,   vr12
    vadd.h           vr13,    vr13,   vr14
    vsrarni.b.h      vr13,    vr13,   6
    vstelm.d         vr13,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    vilvl.b          vr11,    vr8,    vr7
    vilvl.b          vr12,    vr6,    vr5
    vmulwev.h.bu     vr13,    vr9,    vr11
    vmaddwod.h.bu    vr13,    vr9,    vr11
    vmulwev.h.bu     vr14,    vr10,   vr12
    vmaddwod.h.bu    vr14,    vr10,   vr12
    vadd.h           vr13,    vr13,   vr14
    vsrarni.b.h      vr13,    vr13,   6
    vstelm.d         vr13,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr7,     a1,     0
    vld              vr8,     a1,     1
    vilvl.b          vr11,    vr6,    vr5
    vilvl.b          vr12,    vr8,    vr7
    vmulwev.h.bu     vr13,    vr9,    vr11
    vmaddwod.h.bu    vr13,    vr9,    vr11
    vmulwev.h.bu     vr14,    vr10,   vr12
    vmaddwod.h.bu    vr14,    vr10,   vr12
    vadd.h           vr13,    vr13,   vr14
    vsrarni.b.h      vr13,    vr13,   6
    vstelm.d         vr13,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    vilvl.b          vr11,    vr8,    vr7
    vilvl.b          vr12,    vr6,    vr5
    vmulwev.h.bu     vr13,    vr9,    vr11
    vmaddwod.h.bu    vr13,    vr9,    vr11
    vmulwev.h.bu     vr14,    vr10,   vr12
    vmaddwod.h.bu    vr14,    vr10,   vr12
    vadd.h           vr13,    vr13,   vr14
    vsrarni.b.h      vr13,    vr13,   6
    vstelm.d         vr13,    a0,     0,    0
    add.d            a0,      a0,     a2

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOP_D
    b                .ENDLOOP
.ENDLOOP_D:

    bge              zero,    t0,     .ENDLOOP_E
    move             t1,      a3
    li.d             t7,      1
    slt              t8,      zero,   t5
    maskeqz          t5,      a2,     t8
    masknez          t7,      t7,     t8
    or               t7,      t7,     t5
    vilvl.b          vr7,     vr4,    vr0
.LOOP_E:
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOP_E
    b                .ENDLOOP
.ENDLOOP_E:

    move             t1,      a3
.LOOP:
    vld              vr5,     a1,     0
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vsrarni.b.h      vr7,     vr7,    6
    vilvl.b          vr6,     vr7,    vr6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     a2
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vsrarni.b.h      vr7,     vr7,    6
    vilvl.b          vr6,     vr7,    vr6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     t2
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vsrarni.b.h      vr7,     vr7,    6
    vilvl.b          vr6,     vr7,    vr6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     t3
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vsrarni.b.h      vr7,     vr7,    6
    vilvl.b          vr6,     vr7,    vr6
    vstelm.d         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     t4

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOP
.ENDLOOP:
endfunc

/* void ff_avg_h264_chroma_mc8_lsx(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
                                   int h, int x, int y) */
function ff_avg_h264_chroma_mc8_lsx
    li.d             t8,      8
    sub.d            t1,      t8,     a4     // 8-x
    sub.d            t2,      t8,     a5     // 8-y
    mul.d            t3,      t1,     t2     // A
    mul.d            t4,      a4,     t2     // B
    mul.d            t5,      t1,     a5     // C
    mul.d            t6,      a4,     a5     // D
    add.d            t0,      t4,     t5     // E
    vreplgr2vr.b     vr0,     t3
    vreplgr2vr.b     vr1,     t4
    vreplgr2vr.b     vr2,     t5
    vreplgr2vr.b     vr3,     t6
    vreplgr2vr.b     vr4,     t0
    slli.d           t2,      a2,     1
    add.d            t3,      t2,     a2
    slli.d           t4,      a2,     2

    bge              zero,    t6,     .ENDLOOPD
    move             t1,      a3
    vilvl.b          vr9,     vr1,    vr0
    vilvl.b          vr10,    vr3,    vr2
.LOOPD:
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    add.d            a1,      a1,     a2
    vld              vr7,     a1,     0
    vld              vr8,     a1,     1
    vld              vr11,    a0,     0
    vilvl.b          vr12,    vr6,    vr5
    vilvl.b          vr13,    vr8,    vr7
    vmulwev.h.bu     vr14,    vr9,    vr12
    vmaddwod.h.bu    vr14,    vr9,    vr12
    vmulwev.h.bu     vr15,    vr10,   vr13
    vmaddwod.h.bu    vr15,    vr10,   vr13
    vadd.h           vr14,    vr14,   vr15
    vsrari.h         vr14,    vr14,   6
    vsllwil.hu.bu    vr11,    vr11,   0
    vadd.h           vr11,    vr14,   vr11
    vsrarni.b.h      vr11,    vr11,   1
    vstelm.d         vr11,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    vld              vr11,    a0,     0
    vilvl.b          vr12,    vr8,    vr7
    vilvl.b          vr13,    vr6,    vr5
    vmulwev.h.bu     vr14,    vr9,    vr12
    vmaddwod.h.bu    vr14,    vr9,    vr12
    vmulwev.h.bu     vr15,    vr10,   vr13
    vmaddwod.h.bu    vr15,    vr10,   vr13
    vadd.h           vr14,    vr14,   vr15
    vsrari.h         vr14,    vr14,   6
    vsllwil.hu.bu    vr11,    vr11,   0
    vadd.h           vr11,    vr14,   vr11
    vsrarni.b.h      vr11,    vr11,   1
    vstelm.d         vr11,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr7,     a1,     0
    vld              vr8,     a1,     1
    vld              vr11,    a0,     0
    vilvl.b          vr12,    vr6,    vr5
    vilvl.b          vr13,    vr8,    vr7
    vmulwev.h.bu     vr14,    vr9,    vr12
    vmaddwod.h.bu    vr14,    vr9,    vr12
    vmulwev.h.bu     vr15,    vr10,   vr13
    vmaddwod.h.bu    vr15,    vr10,   vr13
    vadd.h           vr14,    vr14,   vr15
    vsrari.h         vr14,    vr14,   6
    vsllwil.hu.bu    vr11,    vr11,   0
    vadd.h           vr11,    vr14,   vr11
    vsrarni.b.h      vr11,    vr11,   1
    vstelm.d         vr11,    a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    vld              vr11,    a0,     0
    vilvl.b          vr12,    vr8,    vr7
    vilvl.b          vr13,    vr6,    vr5
    vmulwev.h.bu     vr14,    vr9,    vr12
    vmaddwod.h.bu    vr14,    vr9,    vr12
    vmulwev.h.bu     vr15,    vr10,   vr13
    vmaddwod.h.bu    vr15,    vr10,   vr13
    vadd.h           vr14,    vr14,   vr15
    vsrari.h         vr14,    vr14,   6
    vsllwil.hu.bu    vr11,    vr11,   0
    vadd.h           vr11,    vr14,   vr11
    vsrarni.b.h      vr11,    vr11,   1
    vstelm.d         vr11,    a0,     0,    0
    add.d            a0,      a0,     a2

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOPD
    b                .ENDLOOPELSE
.ENDLOOPD:

    bge              zero,    t0,     .ENDLOOPE
    move             t1,      a3
    li.d             t7,      1
    slt              t8,      zero,   t5
    maskeqz          t5,      a2,     t8
    masknez          t7,      t7,     t8
    or               t7,      t7,     t5
    vilvl.b          vr7,     vr4,    vr0
.LOOPE:
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vld              vr8,     a0,     0
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vld              vr8,     a0,     0
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vld              vr8,     a0,     0
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vld              vr8,     a0,     0
    vilvl.b          vr5,     vr6,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOPE
    b                .ENDLOOPELSE
.ENDLOOPE:

    move             t1,      a3
.LOOPELSE:
    vld              vr5,     a1,     0
    vld              vr8,     a0,     0
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vilvl.h          vr6,     vr7,    vr6
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     a2
    vld              vr8,     a0,     0
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vilvl.h          vr6,     vr7,    vr6
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     t2
    vld              vr8,     a0,     0
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vilvl.h          vr6,     vr7,    vr6
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    vldx             vr5,     a1,     t3
    vld              vr8,     a0,     0
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vilvl.h          vr6,     vr7,    vr6
    vsrari.h         vr6,     vr6,    6
    vsllwil.hu.bu    vr8,     vr8,    0
    vadd.h           vr8,     vr6,    vr8
    vsrarni.b.h      vr8,     vr8,    1
    vstelm.d         vr8,     a0,     0,    0
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     t4

    addi.d           t1,      t1,     -4
    blt              zero,    t1,     .LOOPELSE
.ENDLOOPELSE:
endfunc

/* void ff_put_h264_chroma_mc4_lsx(uint8_t *dst, uint8_t *src, ptrdiff_t stride,
                                   int h, int x, int y) */
function ff_put_h264_chroma_mc4_lsx
    li.d             t8,      8
    sub.d            t1,      t8,     a4     // 8-x
    sub.d            t2,      t8,     a5     // 8-y
    mul.d            t3,      t1,     t2     // A
    mul.d            t4,      a4,     t2     // B
    mul.d            t5,      t1,     a5     // C
    mul.d            t6,      a4,     a5     // D
    add.d            t0,      t4,     t5     // E
    slli.d           t8,      a2,     1
    vreplgr2vr.b     vr0,     t3
    vreplgr2vr.b     vr1,     t4
    vreplgr2vr.b     vr2,     t5
    vreplgr2vr.b     vr3,     t6
    vreplgr2vr.b     vr4,     t0

    bge              zero,    t6,     .ENDPUT_D
    move             t1,      a3
    vilvl.b          vr9,     vr1,    vr0
    vilvl.b          vr10,    vr3,    vr2
.PUT_D:
    vld              vr5,     a1,     0
    vld              vr6,     a1,     1
    add.d            a1,      a1,     a2
    vld              vr7,     a1,     0
    vld              vr8,     a1,     1
    add.d            a1,      a1,     a2
    vld              vr11,    a1,     0
    vld              vr12,    a1,     1
    vilvl.b          vr5,     vr6,    vr5
    vilvl.b          vr7,     vr8,    vr7
    vilvl.b          vr13,    vr12,   vr11
    vilvl.d          vr5,     vr7,    vr5
    vilvl.d          vr13,    vr13,   vr7
    vmulwev.h.bu     vr14,    vr9,    vr5
    vmaddwod.h.bu    vr14,    vr9,    vr5
    vmulwev.h.bu     vr15,    vr10,   vr13
    vmaddwod.h.bu    vr15,    vr10,   vr13
    vadd.h           vr14,    vr14,   vr15
    vsrarni.b.h      vr14,    vr14,   6
    vstelm.w         vr14,    a0,     0,    0
    add.d            a0,      a0,     a2
    vstelm.w         vr14,    a0,     0,    1
    add.d            a0,      a0,     a2
    addi.d           t1,      t1,     -2
    blt              zero,    t1,     .PUT_D
    b                .ENDPUT
.ENDPUT_D:

    bge              zero,    t0,     .ENDPUT_E
    move             t1,      a3
    li.d             t7,      1
    slt              t8,      zero,   t5
    maskeqz          t5,      a2,     t8
    masknez          t7,      t7,     t8
    or               t7,      t7,     t5
    vilvl.b          vr7,     vr4,    vr0
.PUT_E:
    vld              vr5,     a1,     0
    vldx             vr6,     a1,     t7
    vilvl.b          vr5,     vr6,    vr5
    add.d            a1,      a1,     a2
    vld              vr8,     a1,     0
    vldx             vr9,     a1,     t7
    vilvl.b          vr8,     vr9,    vr8
    vilvl.d          vr5,     vr8,    vr5
    vmulwev.h.bu     vr6,     vr7,    vr5
    vmaddwod.h.bu    vr6,     vr7,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vstelm.w         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    vstelm.w         vr6,     a0,     0,    1
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     a2
    addi.d           t1,      t1,     -2
    blt              zero,    t1,     .PUT_E
    b                .ENDPUT
.ENDPUT_E:

    move             t1,      a3
.PUT:
    vld              vr5,     a1,     0
    vldx             vr8,     a1,     a2
    vilvl.w          vr5,     vr8,    vr5
    vmulwev.h.bu     vr6,     vr0,    vr5
    vmulwod.h.bu     vr7,     vr0,    vr5
    vsrarni.b.h      vr6,     vr6,    6
    vsrarni.b.h      vr7,     vr7,    6
    vilvl.b          vr6,     vr7,    vr6
    vstelm.w         vr6,     a0,     0,    0
    add.d            a0,      a0,     a2
    vstelm.w         vr6,     a0,     0,    1
    add.d            a0,      a0,     a2
    add.d            a1,      a1,     t8
    addi.d           t1,      t1,     -2
    blt              zero,    t1,     .PUT
.ENDPUT:
endfunc
