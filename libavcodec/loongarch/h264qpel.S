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

.macro lsx_put_h264_qpel8_v_lowpass in0, in1, in2, in3, in4, in5, in6
    vilvl.b       vr7,    \in3,   \in2
    vilvl.b       vr8,    \in4,   \in3
    vilvl.b       vr9,    \in4,   \in1
    vilvl.b       vr10,   \in5,   \in2
    vilvl.b       vr11,   \in5,   \in0
    vilvl.b       vr12,   \in6,   \in1

    vhaddw.hu.bu  vr7,    vr7,    vr7
    vhaddw.hu.bu  vr8,    vr8,    vr8
    vhaddw.hu.bu  vr9,    vr9,    vr9
    vhaddw.hu.bu  vr10,   vr10,   vr10
    vhaddw.hu.bu  vr11,   vr11,   vr11
    vhaddw.hu.bu  vr12,   vr12,   vr12

    vmul.h        vr7,    vr7,    vr20
    vmul.h        vr8,    vr8,    vr20
    vmul.h        vr9,    vr9,    vr21
    vmul.h        vr10,   vr10,   vr21

    vssub.h       vr7,    vr7,    vr9
    vssub.h       vr8,    vr8,    vr10
    vsadd.h       vr7,    vr7,    vr11
    vsadd.h       vr8,    vr8,    vr12
    vsadd.h       vr7,    vr7,    vr22
    vsadd.h       vr8,    vr8,    vr22

    vilvh.b       vr13,   \in3,   \in2
    vilvh.b       vr14,   \in4,   \in3
    vilvh.b       vr15,   \in4,   \in1
    vilvh.b       vr16,   \in5,   \in2
    vilvh.b       vr17,   \in5,   \in0
    vilvh.b       vr18,   \in6,   \in1

    vhaddw.hu.bu  vr13,   vr13,   vr13
    vhaddw.hu.bu  vr14,   vr14,   vr14
    vhaddw.hu.bu  vr15,   vr15,   vr15
    vhaddw.hu.bu  vr16,   vr16,   vr16
    vhaddw.hu.bu  vr17,   vr17,   vr17
    vhaddw.hu.bu  vr18,   vr18,   vr18

    vmul.h        vr13,   vr13,   vr20
    vmul.h        vr14,   vr14,   vr20
    vmul.h        vr15,   vr15,   vr21
    vmul.h        vr16,   vr16,   vr21

    vssub.h       vr13,   vr13,   vr15
    vssub.h       vr14,   vr14,   vr16
    vsadd.h       vr13,   vr13,   vr17
    vsadd.h       vr14,   vr14,   vr18
    vsadd.h       vr13,   vr13,   vr22
    vsadd.h       vr14,   vr14,   vr22

    vssrani.bu.h  vr13,   vr7,    5
    vssrani.bu.h  vr14,   vr8,    5
.endm

/*
 * void put_h264_qpel16_mc01(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc01_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    sub.d         t2,     a1,     t0  // t2 = src - 2 * stride
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    vld           vr0,    t2,     0
    vldx          vr1,    t2,     a2
    vldx          vr2,    t2,     t0
    vldx          vr3,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr4,    t2,     0
    vldx          vr5,    t2,     a2
    vldx          vr6,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr2,    vr13
    vavgr.bu      vr14,   vr3,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr4,    vr13
    vavgr.bu      vr14,   vr5,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr6,    vr13
    vavgr.bu      vr14,   vr0,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr5,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr1,    vr13
    vavgr.bu      vr14,   vr2,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vavgr.bu      vr13,   vr3,    vr13
    vavgr.bu      vr14,   vr4,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vavgr.bu      vr13,   vr5,    vr13
    vavgr.bu      vr14,   vr6,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vavgr.bu      vr13,   vr0,    vr13
    vavgr.bu      vr14,   vr1,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr2,    vr13
    vavgr.bu      vr14,   vr3,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1
endfunc

/*
 * void put_h264_qpel16_mc11(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc11_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        sp,     sp,     -64
    fst.d         f24,    sp,     0
    fst.d         f25,    sp,     8
    fst.d         f26,    sp,     16
    fst.d         f27,    sp,     24
    fst.d         f28,    sp,     32
    fst.d         f29,    sp,     40
    fst.d         f30,    sp,     48
    fst.d         f31,    sp,     56

    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride

    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d        a1,     t3,     8
    LSX_QPEL8_H_LOWPASS vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5
    alsl.d        a1,     a2,     a1,    1

    LSX_QPEL8_H_LOWPASS vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5
    alsl.d        a1,     a2,     a1,    1

    LSX_QPEL8_H_LOWPASS vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    vld           vr0,    t4,     0      // t4 = src - 2 * stride
    vldx          vr1,    t4,     a2
    vldx          vr2,    t4,     t1
    vldx          vr3,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr4,    t4,     0
    vldx          vr5,    t4,     a2
    vldx          vr6,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a1,     a2,     t3,    3  // a1 = src + 8 * stride
    addi.d        t5,     a1,     8         // a1 = src + 8 * stride + 8

    LSX_QPEL8_H_LOWPASS vr12, vr13
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr14, vr15
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr16, vr17
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr18, vr19

    addi.d         a1,     t5,     0
    LSX_QPEL8_H_LOWPASS vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5
    alsl.d        a1,     a2,     a1,    1
    LSX_QPEL8_H_LOWPASS vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5
    alsl.d        a1,     a2,     a1,    1

    LSX_QPEL8_H_LOWPASS vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5
    alsl.d        a1,     a2,     a1,    1

    LSX_QPEL8_H_LOWPASS vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    alsl.d        a0,     a2,     a0,    2   // dst = dst + 4 * stride

    // t6 = src + 6 * stride
    vld           vr0,    t6,     0
    vldx          vr1,    t6,     a2
    vldx          vr2,    t6,     t1
    vldx          vr3,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr4,    t6,     0
    vldx          vr5,    t6,     a2
    vldx          vr6,    t6,     t1

    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    fld.d         f24,    sp,     0
    fld.d         f25,    sp,     8
    fld.d         f26,    sp,     16
    fld.d         f27,    sp,     24
    fld.d         f28,    sp,     32
    fld.d         f29,    sp,     40
    fld.d         f30,    sp,     48
    fld.d         f31,    sp,     56
    addi.d        sp,     sp,     64
endfunc

/*
 * void avg_h264_qpel16_mc00(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc00_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    slli.d        t2,     t0,     1
    addi.d        t3,     a0,     0

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

    vld           vr8,    t3,     0
    vldx          vr9,    t3,     a2
    vldx          vr10,   t3,     t0
    vldx          vr11,   t3,     t1
    add.d         t3,     t3,     t2
    vld           vr12,   t3,     0
    vldx          vr13,   t3,     a2
    vldx          vr14,   t3,     t0
    vldx          vr15,   t3,     t1
    add.d         t3,     t3,     t2

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

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

    /* h8~h15 */
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a2
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vld           vr8,    t3,     0
    vldx          vr9,    t3,     a2
    vldx          vr10,   t3,     t0
    vldx          vr11,   t3,     t1
    add.d         t3,     t3,     t2
    vld           vr12,   t3,     0
    vldx          vr13,   t3,     a2
    vldx          vr14,   t3,     t0
    vldx          vr15,   t3,     t1

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

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

.macro LSX_QPEL8_H_LOWPASS_1 out0, out1
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
 * void put_h264_qpel16_mc31(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc31_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        sp,     sp,     -64
    fst.d         f24,    sp,     0
    fst.d         f25,    sp,     8
    fst.d         f26,    sp,     16
    fst.d         f27,    sp,     24
    fst.d         f28,    sp,     32
    fst.d         f29,    sp,     40
    fst.d         f30,    sp,     48
    fst.d         f31,    sp,     56

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t4,     t4,     1

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     t0,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    addi.d        a1,     t0,     8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    vld           vr0,    t4,     0      // t4 = src - 2 * stride + 1
    vldx          vr1,    t4,     a2
    vldx          vr2,    t4,     t1
    vldx          vr3,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr4,    t4,     0
    vldx          vr5,    t4,     a2
    vldx          vr6,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a1,     a2,     t0,    3  // a1 = src + 8 * stride
    addi.d        t5,     a1,     8         // a1 = src + 8 * stride + 8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        t5,     a2,     t5,    2

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    alsl.d        a0,     a2,     a0,    2   // dst = dst + 4 * stride

    // t6 = src + 6 * stride + 1
    vld           vr0,    t6,     0
    vldx          vr1,    t6,     a2
    vldx          vr2,    t6,     t1
    vldx          vr3,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr4,    t6,     0
    vldx          vr5,    t6,     a2
    vldx          vr6,    t6,     t1

    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    fld.d         f24,    sp,     0
    fld.d         f25,    sp,     8
    fld.d         f26,    sp,     16
    fld.d         f27,    sp,     24
    fld.d         f28,    sp,     32
    fld.d         f29,    sp,     40
    fld.d         f30,    sp,     48
    fld.d         f31,    sp,     56
    addi.d        sp,     sp,     64
endfunc

/*
 * void put_h264_qpel16_mc33(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc33_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        sp,     sp,     -64
    fst.d         f24,    sp,     0
    fst.d         f25,    sp,     8
    fst.d         f26,    sp,     16
    fst.d         f27,    sp,     24
    fst.d         f28,    sp,     32
    fst.d         f29,    sp,     40
    fst.d         f30,    sp,     48
    fst.d         f31,    sp,     56

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t0,     t0,     a2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t4,     t4,     1

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     t0,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    addi.d        a1,     t0,     8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    vld           vr0,    t4,     0      // t4 = src - 2 * stride + 1
    vldx          vr1,    t4,     a2
    vldx          vr2,    t4,     t1
    vldx          vr3,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr4,    t4,     0
    vldx          vr5,    t4,     a2
    vldx          vr6,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride

    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a1,     a2,     t0,    3  // a1 = src + 8 * stride
    addi.d        t5,     a1,     8         // a1 = src + 8 * stride + 8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        t5,     a2,     t5,    2

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    alsl.d        a0,     a2,     a0,    2   // dst = dst + 4 * stride

    // t6 = src + 6 * stride + 1
    vld           vr0,    t6,     0
    vldx          vr1,    t6,     a2
    vldx          vr2,    t6,     t1
    vldx          vr3,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr4,    t6,     0
    vldx          vr5,    t6,     a2
    vldx          vr6,    t6,     t1

    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    fld.d         f24,    sp,     0
    fld.d         f25,    sp,     8
    fld.d         f26,    sp,     16
    fld.d         f27,    sp,     24
    fld.d         f28,    sp,     32
    fld.d         f29,    sp,     40
    fld.d         f30,    sp,     48
    fld.d         f31,    sp,     56
    addi.d        sp,     sp,     64
endfunc

/*
 * void put_h264_qpel16_mc13(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc13_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        sp,     sp,     -64
    fst.d         f24,    sp,     0
    fst.d         f25,    sp,     8
    fst.d         f26,    sp,     16
    fst.d         f27,    sp,     24
    fst.d         f28,    sp,     32
    fst.d         f29,    sp,     40
    fst.d         f30,    sp,     48
    fst.d         f31,    sp,     56

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t0,     t0,     a2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     t0,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    addi.d        a1,     t0,     8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    vld           vr0,    t4,     0      // t4 = src - 2 * stride + 1
    vldx          vr1,    t4,     a2
    vldx          vr2,    t4,     t1
    vldx          vr3,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr4,    t4,     0
    vldx          vr5,    t4,     a2
    vldx          vr6,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride

    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a1,     a2,     t0,    3  // a1 = src + 8 * stride
    addi.d        t5,     a1,     8         // a1 = src + 8 * stride + 8

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        a1,     a2,     a1,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr23, vr24
    vssrani.bu.h  vr23,   vr12,   5
    vssrani.bu.h  vr24,   vr13,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr25, vr26
    vssrani.bu.h  vr25,   vr14,   5
    vssrani.bu.h  vr26,   vr15,   5

    alsl.d        t5,     a2,     t5,    2

    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    LSX_QPEL8_H_LOWPASS_1 vr27, vr28
    vssrani.bu.h  vr27,   vr16,   5
    vssrani.bu.h  vr28,   vr17,   5
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    LSX_QPEL8_H_LOWPASS_1 vr29, vr30
    vssrani.bu.h  vr29,   vr18,   5
    vssrani.bu.h  vr30,   vr19,   5

    alsl.d        a0,     a2,     a0,    2   // dst = dst + 4 * stride

    // t6 = src + 6 * stride + 1
    vld           vr0,    t6,     0
    vldx          vr1,    t6,     a2
    vldx          vr2,    t6,     t1
    vldx          vr3,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr4,    t6,     0
    vldx          vr5,    t6,     a2
    vldx          vr6,    t6,     t1

    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    fld.d         f24,    sp,     0
    fld.d         f25,    sp,     8
    fld.d         f26,    sp,     16
    fld.d         f27,    sp,     24
    fld.d         f28,    sp,     32
    fld.d         f29,    sp,     40
    fld.d         f30,    sp,     48
    fld.d         f31,    sp,     56
    addi.d        sp,     sp,     64
endfunc

/*
 * void put_h264_qpel16_mc03(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc03_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    sub.d         t2,     a1,     t0  // t2 = src - 2 * stride
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    vld           vr0,    t2,     0
    vldx          vr1,    t2,     a2
    vldx          vr2,    t2,     t0
    vldx          vr3,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr4,    t2,     0
    vldx          vr5,    t2,     a2
    vldx          vr6,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr3,    vr13
    vavgr.bu      vr14,   vr4,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr5,    vr13
    vavgr.bu      vr14,   vr6,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr0,    vr13
    vavgr.bu      vr14,   vr1,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr5,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr2,    vr13
    vavgr.bu      vr14,   vr3,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vavgr.bu      vr13,   vr4,    vr13
    vavgr.bu      vr14,   vr5,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vavgr.bu      vr13,   vr6,    vr13
    vavgr.bu      vr14,   vr0,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vavgr.bu      vr13,   vr1,    vr13
    vavgr.bu      vr14,   vr2,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr3,    vr13
    vavgr.bu      vr14,   vr4,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1
endfunc

/*
 * void avg_h264_qpel16_mc10(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc10_lsx
    addi.d        t0,     a0,     0   // t0 = dst
    addi.d        t1,     a1,     -2  // t1 = src - 2
    addi.d        t4,     t1,     8

    slli.d        t2,     a2,     1
    add.d         t3,     a2,     t2

    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        t1,     a2,     t1,    2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2
    alsl.d        t1,     a2,     t1,    2   // t1 = src + 8 * stride -2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        t1,     a2,     t1,    2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3
endfunc

/*
 * void avg_h264_qpel16_mc30(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc30_lsx
    addi.d        t0,     a0,     0   // t0 = dst
    addi.d        t1,     a1,     -2  // t1 = src - 2
    addi.d        t4,     t1,     8
    addi.d        a1,     a1,     1   // a1 = a1 + 1

    slli.d        t2,     a2,     1
    add.d         t3,     a2,     t2

    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        t1,     a2,     t1,    2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2
    alsl.d        t1,     a2,     t1,    2   // t1 = src + 8 * stride -2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr14, vr15

    alsl.d        t1,     a2,     t1,    2

    vld           vr0,    t1,     0
    vldx          vr1,    t1,     a2
    LSX_QPEL8_H_LOWPASS_1 vr16, vr17
    vldx          vr0,    t1,     t2
    vldx          vr1,    t1,     t3
    LSX_QPEL8_H_LOWPASS_1 vr18, vr19

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3

    alsl.d        t4,     a2,     t4,    2
    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t4,     0
    vldx          vr1,    t4,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a2
    vld           vr12,   t0,     0
    vldx          vr13,   t0,     a2
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t4,     t2
    vldx          vr1,    t4,     t3
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vldx          vr0,    a1,     t2
    vldx          vr1,    a1,     t3
    vldx          vr12,   t0,     t2
    vldx          vr13,   t0,     t3
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vavgr.bu      vr0,    vr0,    vr12
    vavgr.bu      vr1,    vr1,    vr13
    vstx          vr0,    a0,     t2
    vstx          vr1,    a0,     t3
endfunc

/*
 * void put_h264_qpel16_mc02(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_put_h264_qpel16_mc02_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    sub.d         t2,     a1,     t0  // t2 = src - 2 * stride
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    vld           vr0,    t2,     0
    vldx          vr1,    t2,     a2
    vldx          vr2,    t2,     t0
    vldx          vr3,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr4,    t2,     0
    vldx          vr5,    t2,     a2
    vldx          vr6,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr5,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    lsx_put_h264_qpel8_v_lowpass vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    lsx_put_h264_qpel8_v_lowpass vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1
endfunc
