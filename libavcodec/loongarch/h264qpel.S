/*
 * Loongson LSX optimized h264qpel
 *
 * Copyright (c) 2022 Loongson Technology Corporation Limited
 * Contributed by Hecai Yuan <yuanhecai@loongson.cn>
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
 * void put_h264_qpel16_mc00(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc00_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    slli.d        t2,     t0,     1

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a2
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1
    add.d         a1,     a1,     t2

    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2
    vstx          vr2,    a0,     t0
    vstx          vr3,    a0,     t1
    add.d         a0,     a0,     t2
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2
    vstx          vr6,    a0,     t0
    vstx          vr7,    a0,     t1
    add.d         a0,     a0,     t2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a2
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2
    vstx          vr2,    a0,     t0
    vstx          vr3,    a0,     t1
    add.d         a0,     a0,     t2
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2
    vstx          vr6,    a0,     t0
    vstx          vr7,    a0,     t1
endfunc

.macro LSX_QPEL8_H_LOWPASS out0, out1
    vld           vr0,    a1,     -2
    add.d         t0,     a1,     a2
    vld           vr1,    t0,     -2
    vbsrl.v       vr2,    vr0,    1
    vbsrl.v       vr3,    vr1,    1
    vbsrl.v       vr4,    vr0,    2
    vbsrl.v       vr5,    vr1,    2
    vbsrl.v       vr6,    vr0,    3
    vbsrl.v       vr7,    vr1,    3
    vbsrl.v       vr8,    vr0,    4
    vbsrl.v       vr9,    vr1,    4
    vbsrl.v       vr10,   vr0,    5
    vbsrl.v       vr11,   vr1,    5

    vilvl.b       vr6,    vr4,    vr6
    vilvl.b       vr7,    vr5,    vr7
    vilvl.b       vr8,    vr2,    vr8
    vilvl.b       vr9,    vr3,    vr9
    vilvl.b       vr10,   vr0,    vr10
    vilvl.b       vr11,   vr1,    vr11

    vhaddw.hu.bu  vr6,    vr6,    vr6
    vhaddw.hu.bu  vr7,    vr7,    vr7
    vhaddw.hu.bu  vr8,    vr8,    vr8
    vhaddw.hu.bu  vr9,    vr9,    vr9
    vhaddw.hu.bu  vr10,   vr10,   vr10
    vhaddw.hu.bu  vr11,   vr11,   vr11

    vmul.h        vr2,    vr6,    vr20
    vmul.h        vr3,    vr7,    vr20
    vmul.h        vr4,    vr8,    vr21
    vmul.h        vr5,    vr9,    vr21
    vssub.h       vr2,    vr2,    vr4
    vssub.h       vr3,    vr3,    vr5
    vsadd.h       vr2,    vr2,    vr10
    vsadd.h       vr3,    vr3,    vr11
    vsadd.h       \out0,  vr2,    vr22
    vsadd.h       \out1,  vr3,    vr22
.endm

/*
 * void put_h264_qpel16_mc10(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc10_lsx
    addi.d        t8,     a1,     0
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    /* h0~h7 */
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8
    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr10,   t8,     0
    vldx          vr11,   t8,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr0,    vr2,    vr10
    vavgr.bu      vr1,    vr3,    vr11
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    vldx          vr12,   t8,     t1
    vldx          vr13,   t8,     t2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr2,    vr4,    vr12
    vavgr.bu      vr3,    vr5,    vr13
    vstx          vr2,    a0,     t1
    vstx          vr3,    a0,     t2

    alsl.d        a0,     a2,     a0,    2
    alsl.d        t8,     a2,     t8,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    vld           vr14,   t8,     0
    vldx          vr15,   t8,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr4,    vr6,    vr14
    vavgr.bu      vr5,    vr7,    vr15
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    vldx          vr16,   t8,     t1
    vldx          vr17,   t8,     t2
    alsl.d        t8,     a2,     t8,    2
    vavgr.bu      vr6,    vr8,    vr16
    vavgr.bu      vr7,    vr9,    vr17
    vstx          vr6,    a0,     t1
    vstx          vr7,    a0,     t2
    alsl.d        a0,     a2,     a0,    2

    addi.d        a1,     t8,     0
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8

    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr10,   t8,     0
    vldx          vr11,   t8,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr0,    vr2,    vr10
    vavgr.bu      vr1,    vr3,    vr11
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    vldx          vr12,   t8,     t1
    vldx          vr13,   t8,     t2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr2,    vr4,    vr12
    vavgr.bu      vr3,    vr5,    vr13
    vstx          vr2,    a0,     t1
    vstx          vr3,    a0,     t2

    alsl.d        a0,     a2,     a0,    2
    alsl.d        t8,     a2,     t8,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    vld           vr14,   t8,     0
    vldx          vr15,   t8,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr4,    vr6,    vr14
    vavgr.bu      vr5,    vr7,    vr15
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    vldx          vr16,   t8,     t1
    vldx          vr17,   t8,     t2
    vavgr.bu      vr6,    vr8,    vr16
    vavgr.bu      vr7,    vr9,    vr17
    vstx          vr6,    a0,     t1
    vstx          vr7,    a0,     t2
endfunc

/*
 * void put_h264_qpel16_mc20(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc20_lsx
    addi.d        t8,     a1,     0
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    /* h0~h7 */
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8
    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    alsl.d        a1,     a2,     a1,    1
    vst           vr2,    a0,     0
    vstx          vr3,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    alsl.d        a1,     a2,     a1,    1
    vstx          vr4,    a0,     t1
    vstx          vr5,    a0,     t2

    alsl.d        a0,     a2,     a0,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    alsl.d        a1,     a2,     a1,    1
    vst           vr6,    a0,     0
    vstx          vr7,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    alsl.d        t8,     a2,     t8,    3
    vstx          vr8,    a0,     t1
    vstx          vr9,    a0,     t2
    alsl.d        a0,     a2,     a0,    2

    addi.d        a1,     t8,     0
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8

    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    alsl.d        a1,     a2,     a1,    1
    vst           vr2,    a0,     0
    vstx          vr3,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    alsl.d        a1,     a2,     a1,    1
    vstx          vr4,    a0,     t1
    vstx          vr5,    a0,     t2

    alsl.d        a0,     a2,     a0,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    alsl.d        a1,     a2,     a1,    1
    vst           vr6,    a0,     0
    vstx          vr7,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    vstx          vr8,    a0,     t1
    vstx          vr9,    a0,     t2
endfunc

/*
 * void put_h264_qpel16_mc30(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc30_lsx
    addi.d        t8,     a1,     0
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    /* h0~h7 */
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8
    addi.d        t7,     t8,     1
    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr10,   t7,     0
    vldx          vr11,   t7,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr0,    vr2,    vr10
    vavgr.bu      vr1,    vr3,    vr11
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    vldx          vr12,   t7,     t1
    vldx          vr13,   t7,     t2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr2,    vr4,    vr12
    vavgr.bu      vr3,    vr5,    vr13
    vstx          vr2,    a0,     t1
    vstx          vr3,    a0,     t2

    alsl.d        a0,     a2,     a0,    2
    alsl.d        t7,     a2,     t7,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    vld           vr14,   t7,     0
    vldx          vr15,   t7,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr4,    vr6,    vr14
    vavgr.bu      vr5,    vr7,    vr15
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    vldx          vr16,   t7,     t1
    vldx          vr17,   t7,     t2
    alsl.d        t8,     a2,     t8,    3
    vavgr.bu      vr6,    vr8,    vr16
    vavgr.bu      vr7,    vr9,    vr17
    vstx          vr6,    a0,     t1
    vstx          vr7,    a0,     t2

    alsl.d        a0,     a2,     a0,    2
    alsl.d        t7,     a2,     t7,    2

    addi.d        a1,     t8,     0
    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t8,     8

    LSX_QPEL8_H_LOWPASS vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr10,   t7,     0
    vldx          vr11,   t7,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr0,    vr2,    vr10
    vavgr.bu      vr1,    vr3,    vr11
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr4, vr5
    vssrani.bu.h  vr4,    vr14,   5
    vssrani.bu.h  vr5,    vr15,   5
    vldx          vr12,   t7,     t1
    vldx          vr13,   t7,     t2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr2,    vr4,    vr12
    vavgr.bu      vr3,    vr5,    vr13
    vstx          vr2,    a0,     t1
    vstx          vr3,    a0,     t2

    alsl.d        a0,     a2,     a0,    2
    alsl.d        t7,     a2,     t7,    2

    LSX_QPEL8_H_LOWPASS vr6, vr7
    vssrani.bu.h  vr6,    vr16,   5
    vssrani.bu.h  vr7,    vr17,   5
    vld           vr14,   t7,     0
    vldx          vr15,   t7,     a2
    alsl.d        a1,     a2,     a1,    1
    vavgr.bu      vr4,    vr6,    vr14
    vavgr.bu      vr5,    vr7,    vr15
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a2

    LSX_QPEL8_H_LOWPASS vr8, vr9
    vssrani.bu.h  vr8,    vr18,   5
    vssrani.bu.h  vr9,    vr19,   5
    vldx          vr16,   t7,     t1
    vldx          vr17,   t7,     t2
    vavgr.bu      vr6,    vr8,    vr16
    vavgr.bu      vr7,    vr9,    vr17
    vstx          vr6,    a0,     t1
    vstx          vr7,    a0,     t2
endfunc
