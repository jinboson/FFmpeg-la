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

.macro put_h264_qpel8_v_lowpass_core_lsx in0, in1, in2, in3, in4, in5, in6
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr2,    vr13
    vavgr.bu      vr14,   vr3,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr4,    vr13
    vavgr.bu      vr14,   vr5,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr6,    vr13
    vavgr.bu      vr14,   vr0,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr5,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr1,    vr13
    vavgr.bu      vr14,   vr2,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vavgr.bu      vr13,   vr3,    vr13
    vavgr.bu      vr14,   vr4,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vavgr.bu      vr13,   vr5,    vr13
    vavgr.bu      vr14,   vr6,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vavgr.bu      vr13,   vr0,    vr13
    vavgr.bu      vr14,   vr1,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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

    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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

    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride

    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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

    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride

    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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

    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vavgr.bu      vr13,   vr3,    vr13
    vavgr.bu      vr14,   vr4,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vavgr.bu      vr13,   vr5,    vr13
    vavgr.bu      vr14,   vr6,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vavgr.bu      vr13,   vr0,    vr13
    vavgr.bu      vr14,   vr1,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr5,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vavgr.bu      vr13,   vr2,    vr13
    vavgr.bu      vr14,   vr3,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vavgr.bu      vr13,   vr4,    vr13
    vavgr.bu      vr14,   vr5,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vavgr.bu      vr13,   vr6,    vr13
    vavgr.bu      vr14,   vr0,    vr14
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vavgr.bu      vr13,   vr1,    vr13
    vavgr.bu      vr14,   vr2,    vr14
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 *stride
    vld           vr1,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr2,    t2,     a2
    vldx          vr3,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr5,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr6,    t2,     a2
    vldx          vr0,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr1, vr2, vr3, vr4, vr5, vr6, vr0
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr1,    t2,     t1
    alsl.d        t2,     a2,     t2,    2  // t2 = t2 + 4 * stride
    vld           vr2,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr3, vr4, vr5, vr6, vr0, vr1, vr2
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride

    vldx          vr3,    t2,     a2
    vldx          vr4,    t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx vr5, vr6, vr0, vr1, vr2, vr3, vr4
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr5,    t2,     t1
    alsl.d        t2,     a2,     t2,    2 // t2 = t2 + 4 * stride
    vld           vr6,    t2,     0
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vstx          vr13,   a0,     t0
    vstx          vr14,   a0,     t1
endfunc

.macro lsx_avc_luma_hv_qrt_and_aver_dst_16x16
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
    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vld           vr0,    t8,     0
    vldx          vr1,    t8,     a2
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vavgr.bu      vr13,   vr13,   vr0
    vavgr.bu      vr14,   vr14,   vr1
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr1,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vldx          vr2,    t8,     t1
    vldx          vr3,    t8,     t2
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vavgr.bu      vr13,   vr13,   vr2
    vavgr.bu      vr14,   vr14,   vr3
    add.d         t6,     t4,     zero     // t6 = src + 6 * stride
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2  // dst = dst + 4 * stride
    alsl.d        t8,     a2,     t8,    2

    vldx          vr2,    t4,     a2
    vldx          vr3,    t4,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vld           vr4,    t8,     0
    vldx          vr5,    t8,     a2
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vavgr.bu      vr13,   vr13,   vr4
    vavgr.bu      vr14,   vr14,   vr5
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t4,     t2
    alsl.d        t4,     a2,     t4,    2
    vld           vr5,    t4,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vldx          vr6,    t8,     t1
    vldx          vr0,    t8,     t2
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vavgr.bu      vr13,   vr13,   vr6
    vavgr.bu      vr14,   vr14,   vr0
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
    alsl.d        t8,     a2,     t8,    2
    // t6 = src + 6 * stride + 1
    vld           vr0,    t6,     0
    vldx          vr1,    t6,     a2
    vldx          vr2,    t6,     t1
    vldx          vr3,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr4,    t6,     0
    vldx          vr5,    t6,     a2
    vldx          vr6,    t6,     t1

    put_h264_qpel8_v_lowpass_core_lsx vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vld           vr0,    t8,     0
    vldx          vr1,    t8,     a2
    vavgr.bu      vr13,   vr23,   vr13
    vavgr.bu      vr14,   vr24,   vr14
    vavgr.bu      vr13,   vr13,   vr0
    vavgr.bu      vr14,   vr14,   vr1
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr0,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr1,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vldx          vr2,    t8,     t1
    vldx          vr3,    t8,     t2
    vavgr.bu      vr13,   vr25,   vr13
    vavgr.bu      vr14,   vr26,   vr14
    vavgr.bu      vr13,   vr13,   vr2
    vavgr.bu      vr14,   vr14,   vr3
    vstx          vr13,   a0,     t1
    vstx          vr14,   a0,     t2

    alsl.d        a0,     a2,     a0,    2    // dst = dst + 4 *stride
    alsl.d        t8,     a2,     t8,    2

    vldx          vr2,    t6,     a2
    vldx          vr3,    t6,     t1
    put_h264_qpel8_v_lowpass_core_lsx vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vld           vr4,    t8,     0
    vldx          vr5,    t8,     a2
    vavgr.bu      vr13,   vr27,   vr13
    vavgr.bu      vr14,   vr28,   vr14
    vavgr.bu      vr13,   vr13,   vr4
    vavgr.bu      vr14,   vr14,   vr5
    vst           vr13,   a0,     0
    vstx          vr14,   a0,     a2

    vldx          vr4,    t6,     t2
    alsl.d        t6,     a2,     t6,    2
    vld           vr5,    t6,     0
    put_h264_qpel8_v_lowpass_core_lsx vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vldx          vr6,    t8,     t1
    vldx          vr0,    t8,     t2
    vavgr.bu      vr13,   vr29,   vr13
    vavgr.bu      vr14,   vr30,   vr14
    vavgr.bu      vr13,   vr13,   vr6
    vavgr.bu      vr14,   vr14,   vr0
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
.endm

/*
 * void avg_h264_qpel16_mc33(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc33_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t0,     t0,     a2   // t0 = src + stride - 2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t4,     t4,     1
    addi.d        t8,     a0,     0

    lsx_avc_luma_hv_qrt_and_aver_dst_16x16
endfunc

/*
 * void avg_h264_qpel16_mc11(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc11_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t8,     a0,     0

    lsx_avc_luma_hv_qrt_and_aver_dst_16x16
endfunc

/*
 * void avg_h264_qpel16_mc31(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc31_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t4,     t4,     1
    addi.d        t8,     a0,     0

    lsx_avc_luma_hv_qrt_and_aver_dst_16x16
endfunc

/*
 * void avg_h264_qpel16_mc13(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc13_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t0,     t0,     a2
    add.d         t3,     a1,     zero // t3 = src
    sub.d         t4,     a1,     t1   // t4 = src - 2 * stride
    addi.d        t8,     a0,     0

    lsx_avc_luma_hv_qrt_and_aver_dst_16x16
endfunc

/*
 * void avg_h264_qpel16_mc20(uint8_t *dst, const uint8_t *src,
 *                           ptrdiff_t stride)
 */
function ff_avg_h264_qpel16_mc20_lsx
    slli.d        t1,     a2,     1
    add.d         t2,     t1,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        t0,     a1,     -2   // t0 = src - 2
    addi.d        t5,     a0,     0

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

    addi.d        t0,     t0,     8

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vstx          vr0,    a0,     t1
    vstx          vr1,    a0,     t2

    alsl.d        t0,     a2,     t0,    2
    alsl.d        t5,     a2,     t5,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vstx          vr0,    a0,     t1
    vstx          vr1,    a0,     t2

    alsl.d        a1,     a2,     a1,    2
    alsl.d        t0,     a2,     t0,    2
    alsl.d        t5,     a2,     t5,    2
    alsl.d        a0,     a2,     a0,    2

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

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    vssrani.bu.h  vr2,    vr12,   5
    vssrani.bu.h  vr3,    vr13,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    vssrani.bu.h  vr2,    vr14,   5
    vssrani.bu.h  vr3,    vr15,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vstx          vr0,    a0,     t1
    vstx          vr1,    a0,     t2

    alsl.d        t0,     a2,     t0,    2
    alsl.d        t5,     a2,     t5,    2
    alsl.d        a0,     a2,     a0,    2

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vld           vr0,    t5,     0
    vldx          vr1,    t5,     a2
    vssrani.bu.h  vr2,    vr16,   5
    vssrani.bu.h  vr3,    vr17,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr2, vr3
    vldx          vr0,    t5,     t1
    vldx          vr1,    t5,     t2
    vssrani.bu.h  vr2,    vr18,   5
    vssrani.bu.h  vr3,    vr19,   5
    vavgr.bu      vr0,    vr0,    vr2
    vavgr.bu      vr1,    vr1,    vr3
    vstx          vr0,    a0,     t1
    vstx          vr1,    a0,     t2
endfunc

.macro LSX_QPEL8_HV_LOWPASS_H out0, out1
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
    vsadd.h       \out0,  vr2,    vr10
    vsadd.h       \out1,  vr3,    vr11
.endm

.macro LSX_QPEL8_HV_LOWPASS_V in0, in1, in2, in3, in4, in5, in6, out0, out1, out2, out3
    vilvl.h       vr0,    \in2,   \in3
    vilvl.h       vr1,    \in3,   \in4  // tmp0
    vilvl.h       vr2,    \in1,   \in4
    vilvl.h       vr3,    \in2,   \in5  // tmp2
    vilvl.h       vr4,    \in0,   \in5
    vilvl.h       vr5,    \in1,   \in6  // tmp4
    vhaddw.w.h    vr0,    vr0,    vr0
    vhaddw.w.h    vr1,    vr1,    vr1
    vhaddw.w.h    vr2,    vr2,    vr2
    vhaddw.w.h    vr3,    vr3,    vr3
    vhaddw.w.h    vr4,    vr4,    vr4
    vhaddw.w.h    vr5,    vr5,    vr5
    vmul.w        vr0,    vr0,    vr22
    vmul.w        vr1,    vr1,    vr22
    vmul.w        vr2,    vr2,    vr23
    vmul.w        vr3,    vr3,    vr23
    vssub.w       vr0,    vr0,    vr2
    vssub.w       vr1,    vr1,    vr3
    vsadd.w       vr0,    vr0,    vr4
    vsadd.w       vr1,    vr1,    vr5
    vsadd.w       \out0,  vr0,    vr24
    vsadd.w       \out1,  vr1,    vr24

    vilvh.h       vr0,    \in2,   \in3
    vilvh.h       vr1,    \in3,   \in4  // tmp0
    vilvh.h       vr2,    \in1,   \in4
    vilvh.h       vr3,    \in2,   \in5  // tmp2
    vilvh.h       vr4,    \in0,   \in5
    vilvh.h       vr5,    \in1,   \in6  // tmp4
    vhaddw.w.h    vr0,    vr0,    vr0
    vhaddw.w.h    vr1,    vr1,    vr1
    vhaddw.w.h    vr2,    vr2,    vr2
    vhaddw.w.h    vr3,    vr3,    vr3
    vhaddw.w.h    vr4,    vr4,    vr4
    vhaddw.w.h    vr5,    vr5,    vr5
    vmul.w        vr0,    vr0,    vr22
    vmul.w        vr1,    vr1,    vr22
    vmul.w        vr2,    vr2,    vr23
    vmul.w        vr3,    vr3,    vr23
    vssub.w       vr0,    vr0,    vr2
    vssub.w       vr1,    vr1,    vr3
    vsadd.w       vr0,    vr0,    vr4
    vsadd.w       vr1,    vr1,    vr5
    vsadd.w       \out2,  vr0,    vr24
    vsadd.w       \out3,  vr1,    vr24

    vssrani.hu.w  \out2,  \out0,  10
    vssrani.hu.w  \out3,  \out1,  10
    vssrani.bu.h  \out3,  \out2,  0
.endm

.macro put_h264_qpel8_hv_lowpass_core_lsx in0, in1
    vld           vr0,    \in0,  0
    vldx          vr1,    \in0,  a3
    LSX_QPEL8_HV_LOWPASS_H vr12, vr13 // a b$
    vldx          vr0,    \in0,  t1
    vldx          vr1,    \in0,  t2
    LSX_QPEL8_HV_LOWPASS_H vr14, vr15 // c d$

    alsl.d        \in0,   a3,    \in0,   2

    vld           vr0,    \in0,  0
    vldx          vr1,    \in0,  a3
    LSX_QPEL8_HV_LOWPASS_H vr16, vr17 // e f$
    vldx          vr0,    \in0,  t1
    vldx          vr1,    \in0,  t2
    LSX_QPEL8_HV_LOWPASS_H vr18, vr19 // g h$

    LSX_QPEL8_HV_LOWPASS_V vr12, vr13, vr14, vr15, vr16, vr17, vr18, vr6, vr7, vr0, vr1
    vstelm.d      vr1,    \in1,    0,     0
    add.d         \in1,   \in1,    a2
    vstelm.d      vr1,    \in1,    0,     1

    alsl.d        \in0,    a3,    \in0,   2

    // tmp8
    vld           vr0,    \in0,   0
    vldx          vr1,    \in0,   a3
    LSX_QPEL8_HV_LOWPASS_H vr12, vr13
    LSX_QPEL8_HV_LOWPASS_V vr14, vr15, vr16, vr17, vr18, vr19, vr12, vr6, vr7, vr0, vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1

    // tmp10
    vldx          vr0,    \in0,   t1
    vldx          vr1,    \in0,   t2
    LSX_QPEL8_HV_LOWPASS_H vr14, vr15
    LSX_QPEL8_HV_LOWPASS_V vr16, vr17, vr18, vr19, vr12, vr13, vr14, vr6, vr7, vr0, vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1

    // tmp12
    alsl.d        \in0,   a3,     \in0,  2

    vld           vr0,    \in0,   0
    vldx          vr1,    \in0,   a3
    LSX_QPEL8_HV_LOWPASS_H vr16, vr17
    LSX_QPEL8_HV_LOWPASS_V vr18, vr19, vr12, vr13, vr14, vr15, vr16, vr6, vr7, vr0, vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1
.endm

function put_h264_qpel8_hv_lowpass_lsx
    slli.d        t1,     a3,     1
    add.d         t2,     t1,     a3

    addi.d        sp,     sp,     -8
    fst.d         f24,    sp,     0

    vldi          vr20,   0x414   // h_20
    vldi          vr21,   0x405   // h_5
    vldi          vr22,   0x814   // w_20
    vldi          vr23,   0x805   // w_5
    addi.d        t4,     zero,   512
    vreplgr2vr.w  vr24,   t4      // w_512

    addi.d        t0,     a1,     -2   // t0 = src - 2
    sub.d         t0,     t0,     t1   // t0 = t0 - 2 * stride

    put_h264_qpel8_hv_lowpass_core_lsx t0, a0

    fld.d         f24,    sp,     0
    addi.d        sp,     sp,     8
endfunc

/*
 * void put_h264_qpel16_h_lowpass_lsx(uint8_t *dst, const uint8_t *src,
 *                                    ptrdiff_t dstStride, ptrdiff_t srcStride)
 */
function put_h264_qpel8_h_lowpass_lsx
    slli.d        t1,     a3,     1
    add.d         t2,     t1,     a3
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t3,     a1,     zero // t3 = src

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a3
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    alsl.d        a1,     a3,     t0,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a3
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2
endfunc

/*
 * void put_pixels16_l2_8_lsx(uint8_t *dst, const uint8_t *src, uint8_t *half,
 *                            ptrdiff_t dstStride, ptrdiff_t srcStride)
 */
function put_pixels16_l2_8_lsx
    slli.d        t0,     a4,     1
    add.d         t1,     t0,     a4
    slli.d        t2,     t0,     1
    slli.d        t3,     a3,     1
    add.d         t4,     t3,     a3
    slli.d        t5,     t3,     1

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1
    add.d         a1,     a1,     t2

    vld           vr8,    a2,     0x00
    vld           vr9,    a2,     0x10
    vld           vr10,   a2,     0x20
    vld           vr11,   a2,     0x30
    vld           vr12,   a2,     0x40
    vld           vr13,   a2,     0x50
    vld           vr14,   a2,     0x60
    vld           vr15,   a2,     0x70

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a3
    vstx          vr2,    a0,     t3
    vstx          vr3,    a0,     t4
    add.d         a0,     a0,     t5
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a3
    vstx          vr6,    a0,     t3
    vstx          vr7,    a0,     t4
    add.d         a0,     a0,     t5

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vld           vr8,    a2,     0x80
    vld           vr9,    a2,     0x90
    vld           vr10,   a2,     0xa0
    vld           vr11,   a2,     0xb0
    vld           vr12,   a2,     0xc0
    vld           vr13,   a2,     0xd0
    vld           vr14,   a2,     0xe0
    vld           vr15,   a2,     0xf0

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a3
    vstx          vr2,    a0,     t3
    vstx          vr3,    a0,     t4
    add.d         a0,     a0,     t5
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a3
    vstx          vr6,    a0,     t3
    vstx          vr7,    a0,     t4
endfunc

.macro put_h264_qpel8_v_lowpass_core_lsx_1 in0, in1, in2, in3, in4, in5, in6
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

    vssrani.bu.h  vr8,    vr7,    5
.endm

/*
 * void put_h264_qpel8_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride,
 *                               int srcStride)
 */
function put_h264_qpel8_v_lowpass_lsx
    slli.d        t0,     a3,     1
    add.d         t1,     t0,     a3
    sub.d         t2,     a1,     t0  // t2 = src - 2 * stride
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    fld.d         f0,     t2,     0
    fldx.d        f1,     t2,     a3
    fldx.d        f2,     t2,     t0
    fldx.d        f3,     t2,     t1
    alsl.d        t2,     a3,     t2,    2  // t2 = t2 + 4 * stride
    fld.d         f4,     t2,     0
    fldx.d        f5,     t2,     a3
    fldx.d        f6,     t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr0, vr1, vr2, vr3, vr4, vr5, vr6
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    fldx.d        f0,     t2,     t1
    alsl.d        t2,     a3,     t2,    2  // t2 = t2 + 4 *stride
    fld.d         f1,     t2,     0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr2, vr3, vr4, vr5, vr6, vr0, vr1
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    fldx.d        f2,     t2,     a3
    fldx.d        f3,     t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr4, vr5, vr6, vr0, vr1, vr2, vr3
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    fldx.d        f4,     t2,     t1
    alsl.d        t2,     a3,     t2,    2 // t2 = t2 + 4 * stride
    fld.d         f5,     t2,     0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr6, vr0, vr1, vr2, vr3, vr4, vr5
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
endfunc

/*
 * void avg_h264_qpel8_v_lowpass(uint8_t *dst, uint8_t *src, int dstStride,
 *                               int srcStride)
 */
function avg_h264_qpel8_v_lowpass_lsx
    slli.d        t0,     a3,     1
    add.d         t1,     t0,     a3
    sub.d         t2,     a1,     t0  // t2 = src - 2 * stride
    addi.d        t3,     a0,     0
    slli.d        t4,     a2,     1
    add.d         t5,     t4,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    fld.d         f0,     t2,     0
    fldx.d        f1,     t2,     a3
    fldx.d        f2,     t2,     t0
    fldx.d        f3,     t2,     t1
    alsl.d        t2,     a3,     t2,    2  // t2 = t2 + 4 * stride
    fld.d         f4,     t2,     0
    fldx.d        f5,     t2,     a3
    fldx.d        f6,     t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr0, vr1, vr2, vr3, vr4, vr5, vr6
    fld.d         f0,     t3,     0
    fldx.d        f1,     t3,     a2
    vilvl.d       vr0,    vr1,    vr0
    vavgr.bu      vr8,    vr8,    vr0
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    fldx.d        f0,     t2,     t1
    alsl.d        t2,     a3,     t2,   2  // t2 = t2 + 4 *stride
    fld.d         f1,     t2,     0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr2, vr3, vr4, vr5, vr6, vr0, vr1
    fldx.d        f2,     t3,     t4
    fldx.d        f3,     t3,     t5
    vilvl.d       vr2,    vr3,    vr2
    vavgr.bu      vr8,    vr8,    vr2
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    alsl.d        t3,     a2,     t3,   2

    fldx.d        f2,     t2,     a3
    fldx.d        f3,     t2,     t0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr4, vr5, vr6, vr0, vr1, vr2, vr3
    fld.d         f4,     t3,     0
    fldx.d        f5,     t3,     a2
    vilvl.d       vr4,    vr5,    vr4
    vavgr.bu      vr8,    vr8,    vr4
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
    add.d         a0,     a0,     a2

    fldx.d        f4,     t2,     t1
    alsl.d        t2,     a3,     t2,   2 // t2 = t2 + 4 * stride
    fld.d         f5,     t2,     0
    put_h264_qpel8_v_lowpass_core_lsx_1 vr6, vr0, vr1, vr2, vr3, vr4, vr5
    fldx.d        f6,     t3,     t4
    fldx.d        f0,     t3,     t5
    vilvl.d       vr6,    vr0,    vr6
    vavgr.bu      vr8,    vr8,    vr6
    vstelm.d      vr8,    a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr8,    a0,     0,    1
endfunc

/*
 * void avg_pixels16_l2_8(uint8_t *dst, const uint8_t *src, uint8_t *half,
 *                        ptrdiff_t dstStride, ptrdiff_t srcStride)
 */
function avg_pixels16_l2_8_lsx
    slli.d        t0,     a4,     1
    add.d         t1,     t0,     a4
    slli.d        t2,     t0,     1
    slli.d        t3,     a3,     1
    add.d         t4,     t3,     a3
    slli.d        t5,     t3,     1
    addi.d        t6,     a0,     0

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1
    add.d         a1,     a1,     t2

    vld           vr8,    a2,     0x00
    vld           vr9,    a2,     0x10
    vld           vr10,   a2,     0x20
    vld           vr11,   a2,     0x30
    vld           vr12,   a2,     0x40
    vld           vr13,   a2,     0x50
    vld           vr14,   a2,     0x60
    vld           vr15,   a2,     0x70

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vld           vr8,    t6,     0
    vldx          vr9,    t6,     a3
    vldx          vr10,   t6,     t3
    vldx          vr11,   t6,     t4
    add.d         t6,     t6,     t5
    vld           vr12,   t6,     0
    vldx          vr13,   t6,     a3
    vldx          vr14,   t6,     t3
    vldx          vr15,   t6,     t4
    add.d         t6,     t6,     t5

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7
    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a3
    vstx          vr2,    a0,     t3
    vstx          vr3,    a0,     t4
    add.d         a0,     a0,     t5
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a3
    vstx          vr6,    a0,     t3
    vstx          vr7,    a0,     t4
    add.d         a0,     a0,     t5

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vld           vr8,    a2,     0x80
    vld           vr9,    a2,     0x90
    vld           vr10,   a2,     0xa0
    vld           vr11,   a2,     0xb0
    vld           vr12,   a2,     0xc0
    vld           vr13,   a2,     0xd0
    vld           vr14,   a2,     0xe0
    vld           vr15,   a2,     0xf0

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vld           vr8,    t6,     0
    vldx          vr9,    t6,     a3
    vldx          vr10,   t6,     t3
    vldx          vr11,   t6,     t4
    add.d         t6,     t6,     t5
    vld           vr12,   t6,     0
    vldx          vr13,   t6,     a3
    vldx          vr14,   t6,     t3
    vldx          vr15,   t6,     t4

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vst           vr0,    a0,     0
    vstx          vr1,    a0,     a3
    vstx          vr2,    a0,     t3
    vstx          vr3,    a0,     t4
    add.d         a0,     a0,     t5
    vst           vr4,    a0,     0
    vstx          vr5,    a0,     a3
    vstx          vr6,    a0,     t3
    vstx          vr7,    a0,     t4
endfunc

.macro avg_h264_qpel8_hv_lowpass_core_lsx in0, in1, in2
    vld           vr0,    \in0,  0
    vldx          vr1,    \in0,  a3
    LSX_QPEL8_HV_LOWPASS_H vr12, vr13 // a b
    vldx          vr0,    \in0,  t1
    vldx          vr1,    \in0,  t2
    LSX_QPEL8_HV_LOWPASS_H vr14, vr15 // c d

    alsl.d        \in0,   a3,    \in0,   2

    vld           vr0,    \in0,   0
    vldx          vr1,    \in0,   a3
    LSX_QPEL8_HV_LOWPASS_H vr16, vr17 // e f
    vldx          vr0,    \in0,   t1
    vldx          vr1,    \in0,   t2
    LSX_QPEL8_HV_LOWPASS_H vr18, vr19 // g h

    LSX_QPEL8_HV_LOWPASS_V vr12, vr13, vr14, vr15, vr16, vr17, vr18, vr6, vr7, vr0, vr1
    fld.d         f2,     \in2,    0
    fldx.d        f3,     \in2,    a2
    vilvl.d       vr2,    vr3,     vr2
    vavgr.bu      vr1,    vr2,     vr1
    vstelm.d      vr1,    \in1,    0,     0
    add.d         \in1,   \in1,    a2
    vstelm.d      vr1,    \in1,    0,     1

    alsl.d        \in0,    a3,     \in0,  2

    // tmp8
    vld           vr0,    \in0,   0
    vldx          vr1,    \in0,   a3
    LSX_QPEL8_HV_LOWPASS_H vr12, vr13
    LSX_QPEL8_HV_LOWPASS_V vr14, vr15, vr16, vr17, vr18, vr19, vr12, vr6, vr7, vr0, vr1
    fldx.d        f2,     \in2,    t5
    fldx.d        f3,     \in2,    t6
    vilvl.d       vr2,    vr3,     vr2
    vavgr.bu      vr1,    vr2,     vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1

    alsl.d        \in2,   a2,     \in2,  2

    // tmp10
    vldx          vr0,    \in0,   t1
    vldx          vr1,    \in0,   t2
    LSX_QPEL8_HV_LOWPASS_H vr14, vr15
    LSX_QPEL8_HV_LOWPASS_V vr16, vr17, vr18, vr19, vr12, vr13, vr14, vr6, vr7, vr0, vr1
    fld.d         f2,     \in2,    0
    fldx.d        f3,     \in2,    a2
    vilvl.d       vr2,    vr3,     vr2
    vavgr.bu      vr1,    vr2,     vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1

    // tmp12
    alsl.d        \in0,   a3,     \in0,  2

    vld           vr0,    \in0,   0
    vldx          vr1,    \in0,   a3
    LSX_QPEL8_HV_LOWPASS_H vr16, vr17
    LSX_QPEL8_HV_LOWPASS_V vr18, vr19, vr12, vr13, vr14, vr15, vr16, vr6, vr7, vr0, vr1
    fldx.d        f2,     \in2,   t5
    fldx.d        f3,     \in2,   t6
    vilvl.d       vr2,    vr3,    vr2
    vavgr.bu      vr1,    vr2,    vr1
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     0
    add.d         \in1,   \in1,   a2
    vstelm.d      vr1,    \in1,   0,     1
.endm

function avg_h264_qpel8_hv_lowpass_lsx
    slli.d        t1,     a3,     1
    add.d         t2,     t1,     a3
    slli.d        t5,     a2,     1
    add.d         t6,     a2,     t5

    addi.d        sp,     sp,     -8
    fst.d         f24,    sp,     0

    vldi          vr20,   0x414   // h_20
    vldi          vr21,   0x405   // h_5
    vldi          vr22,   0x814   // w_20
    vldi          vr23,   0x805   // w_5
    addi.d        t4,     zero,   512
    vreplgr2vr.w  vr24,   t4      // w_512

    addi.d        t0,     a1,     -2   // t0 = src - 2
    sub.d         t0,     t0,     t1   // t0 = t0 - 2 * stride
    addi.d        t3,     a0,     0    // t3 = dst

    avg_h264_qpel8_hv_lowpass_core_lsx t0, a0, t3

    fld.d         f24,    sp,     0
    addi.d        sp,     sp,     8
endfunc

function put_pixels8_l2_8_lsx
    slli.d        t0,     a4,     1
    add.d         t1,     t0,     a4
    slli.d        t2,     t0,     1
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vld           vr8,    a2,     0x00
    vld           vr9,    a2,     0x08
    vld           vr10,   a2,     0x10
    vld           vr11,   a2,     0x18
    vld           vr12,   a2,     0x20
    vld           vr13,   a2,     0x28
    vld           vr14,   a2,     0x30
    vld           vr15,   a2,     0x38

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    vstelm.d      vr0,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr1,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr2,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr3,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr4,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr5,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr6,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr7,    a0,     0,     0
endfunc

/*
 * void ff_put_h264_qpel8_mc00(uint8_t *dst, const uint8_t *src,
 *                             ptrdiff_t stride)
 */
function ff_put_h264_qpel8_mc00_lsx
    slli.d        t0,     a2,     1
    add.d         t1,     t0,     a2
    slli.d        t2,     t0,     1
    ld.d          t3,     a1,     0x0
    ldx.d         t4,     a1,     a2
    ldx.d         t5,     a1,     t0
    ldx.d         t6,     a1,     t1
    st.d          t3,     a0,     0x0
    stx.d         t4,     a0,     a2
    stx.d         t5,     a0,     t0
    stx.d         t6,     a0,     t1

    add.d         a1,     a1,     t2
    add.d         a0,     a0,     t2

    ld.d          t3,     a1,     0x0
    ldx.d         t4,     a1,     a2
    ldx.d         t5,     a1,     t0
    ldx.d         t6,     a1,     t1
    st.d          t3,     a0,     0x0
    stx.d         t4,     a0,     a2
    stx.d         t5,     a0,     t0
    stx.d         t6,     a0,     t1
endfunc

/*
 * void ff_avg_h264_qpel8_mc00(uint8_t *dst, const uint8_t *src,
 *                             ptrdiff_t stride)
 */
function ff_avg_h264_qpel8_mc00_lsx
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

    vstelm.d      vr0,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr1,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr2,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr3,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr4,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr5,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr6,    a0,     0,     0
    add.d         a0,     a0,     a2
    vstelm.d      vr7,    a0,     0,     0
endfunc

function avg_pixels8_l2_8_lsx
    slli.d        t0,     a4,     1
    add.d         t1,     t0,     a4
    slli.d        t2,     t0,     1
    addi.d        t3,     a0,     0
    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a4
    vldx          vr2,    a1,     t0
    vldx          vr3,    a1,     t1
    add.d         a1,     a1,     t2
    vld           vr4,    a1,     0
    vldx          vr5,    a1,     a4
    vldx          vr6,    a1,     t0
    vldx          vr7,    a1,     t1

    vld           vr8,    a2,     0x00
    vld           vr9,    a2,     0x08
    vld           vr10,   a2,     0x10
    vld           vr11,   a2,     0x18
    vld           vr12,   a2,     0x20
    vld           vr13,   a2,     0x28
    vld           vr14,   a2,     0x30
    vld           vr15,   a2,     0x38

    vavgr.bu      vr0,    vr8,    vr0
    vavgr.bu      vr1,    vr9,    vr1
    vavgr.bu      vr2,    vr10,   vr2
    vavgr.bu      vr3,    vr11,   vr3
    vavgr.bu      vr4,    vr12,   vr4
    vavgr.bu      vr5,    vr13,   vr5
    vavgr.bu      vr6,    vr14,   vr6
    vavgr.bu      vr7,    vr15,   vr7

    slli.d        t0,     a3,     1
    add.d         t1,     t0,     a3
    slli.d        t2,     t0,     1
    vld           vr8,    t3,     0
    vldx          vr9,    t3,     a3
    vldx          vr10,   t3,     t0
    vldx          vr11,   t3,     t1
    add.d         t3,     t3,     t2
    vld           vr12,   t3,     0
    vldx          vr13,   t3,     a3
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

    vstelm.d      vr0,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr1,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr2,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr3,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr4,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr5,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr6,    a0,     0,     0
    add.d         a0,     a0,     a3
    vstelm.d      vr7,    a0,     0,     0
endfunc

function avg_h264_qpel8_h_lowpass_lsx
    slli.d        t1,     a3,     1
    add.d         t2,     t1,     a3
    slli.d        t5,     a2,     1
    add.d         t6,     t5,     a2
    vldi          vr20,   0x414
    vldi          vr21,   0x405
    vldi          vr22,   0x410

    addi.d        t0,     a1,     -2   // t0 = src - 2
    add.d         t3,     a1,     zero // t3 = src
    addi.d        t4,     a0,     0    // t4 = dst

    vld           vr0,    t0,     0
    vldx          vr1,    t0,     a3
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    fld.d         f0,     t4,     0
    fldx.d        f1,     t4,     a2
    vilvl.d       vr0,    vr1,    vr0
    vavgr.bu      vr13,   vr13,   vr0
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    vldx          vr0,    t0,     t1
    vldx          vr1,    t0,     t2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    fldx.d        f0,     t4,     t5
    fldx.d        f1,     t4,     t6
    vilvl.d       vr0,    vr1,    vr0
    vavgr.bu      vr13,   vr13,   vr0
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    alsl.d        a1,     a3,     t0,    2
    alsl.d        t4,     a2,     t4,    2

    vld           vr0,    a1,     0
    vldx          vr1,    a1,     a3
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    fld.d         f0,     t4,     0
    fldx.d        f1,     t4,     a2
    vilvl.d       vr0,    vr1,    vr0
    vavgr.bu      vr13,   vr13,   vr0
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
    add.d         a0,     a0,     a2

    vldx          vr0,    a1,     t1
    vldx          vr1,    a1,     t2
    LSX_QPEL8_H_LOWPASS_1 vr12, vr13
    vssrani.bu.h  vr13,   vr12,   5
    fldx.d        f0,     t4,     t5
    fldx.d        f1,     t4,     t6
    vilvl.d       vr0,    vr1,    vr0
    vavgr.bu      vr13,   vr13,   vr0
    vstelm.d      vr13,   a0,     0,    0
    add.d         a0,     a0,     a2
    vstelm.d      vr13,   a0,     0,    1
endfunc
