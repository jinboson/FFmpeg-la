/*
 * Loongson LASX/LSX optimized h264dsp
 *
 * Copyright (c) 2022 Loongson Technology Corporation Limited
 * Contributed by Hao Chen <chenhao@loongson.cn>
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

const vec_shuf
.rept 2
.byte 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3
.endr
endconst

.macro AVC_LPF_P1_OR_Q1 _in0, _in1, _in2, _in3, _in4, _in5, _out, _tmp0, _tmp1
    vavgr.hu       \_tmp0,   \_in0,  \_in1
    vslli.h        \_tmp1,   \_in2,  1
    vsub.h         \_tmp0,   \_tmp0, \_tmp1
    vavg.h         \_tmp0,   \_in3,  \_tmp0
    vclip.h        \_tmp0,   \_tmp0, \_in4,  \_in5
    vadd.h         \_out,    \_in2,  \_tmp0
.endm

.macro AVC_LPF_P0Q0 _in0, _in1, _in2, _in3, _in4, _in5, _out0,   \
                    _out1, _tmp0, _tmp1
    vsub.h         \_tmp0,   \_in0,  \_in1
    vsub.h         \_tmp1,   \_in2,  \_in3
    vslli.h        \_tmp0,   \_tmp0, 2
    vaddi.hu       \_tmp1,   \_tmp1, 4
    vadd.h         \_tmp0,   \_tmp0, \_tmp1
    vsrai.h        \_tmp0,   \_tmp0, 3
    vclip.h        \_tmp0,   \_tmp0, \_in4,  \_in5
    vadd.h         \_out0,   \_in1,  \_tmp0
    vsub.h         \_out1,   \_in0,  \_tmp0
    vclip255.h     \_out0,   \_out0
    vclip255.h     \_out1,   \_out1
.endm

function ff_h264_h_lpf_luma_8_lsx
    slli.d          t0,     a1,    1   //img_width_2x
    slli.d          t1,     a1,    2   //img_width_4x
    slli.d          t2,     a1,    3   //img_width_8x
    addi.d          sp,     sp,    -64
    fst.d           f24,    sp,    0
    fst.d           f25,    sp,    8
    fst.d           f26,    sp,    16
    fst.d           f27,    sp,    24
    fst.d           f28,    sp,    32
    fst.d           f29,    sp,    40
    fst.d           f30,    sp,    48
    fst.d           f31,    sp,    56
    la.local        t4,     vec_shuf
    add.d           t3,     t0,    a1  //img_width_3x
    vldrepl.w       vr0,    a4,    0   //tmp_vec0
    vld             vr1,    t4,    0  //tc_vec
    vshuf.b         vr1,    vr0,   vr0,   vr1   //tc_vec
    vslti.b         vr2,    vr1,   0
    vxori.b         vr2,    vr2,   255
    vandi.b         vr2,    vr2,   1    //bs_vec
    vsetnez.v       $fcc0,  vr2
    bceqz           $fcc0,  .END_LUMA_8
    vldi            vr0,    0            //zero
    addi.d          t4,     a0,    -4    //src
    vslt.bu         vr3,    vr0,   vr2   //is_bs_greater_than0
    add.d           t5,     t4,    t2    //src_tmp
    vld             vr4,    t4,    0    //row0
    vldx            vr5,    t4,    a1   //row1
    vldx            vr6,    t4,    t0   //row2
    vldx            vr7,    t4,    t3   //row3
    add.d           t6,     t4,    t1   // src += img_width_4x
    vld             vr8,    t6,    0    //row4
    vldx            vr9,    t6,    a1   //row5
    vldx            vr10,   t6,    t0   //row6
    vldx            vr11,   t6,    t3   //row7
    vld             vr12,   t5,    0    //row8
    vldx            vr13,   t5,    a1   //row9
    vldx            vr14,   t5,    t0   //row10
    vldx            vr15,   t5,    t3   //row11
    add.d           t6,     t5,    t1   // src_tmp += img_width_4x
    vld             vr16,   t6,    0    //row12
    vldx            vr17,   t6,    a1   //row13
    vldx            vr18,   t6,    t0   //row14
    vldx            vr19,   t6,    t3   //row15
    LSX_TRANSPOSE16X8_B vr4, vr5, vr6, vr7, vr8, vr9, vr10, vr11,        \
                        vr12, vr13, vr14, vr15, vr16, vr17, vr18, vr19,  \
                        vr10, vr11, vr12, vr13, vr14, vr15, vr16, vr17,  \
                        vr20, vr21, vr22, vr23, vr24, vr25, vr26, vr27
    //vr10: p3_org, vr11: p2_org, vr12: p1_org, vr13: p0_org
    //vr14: q0_org, vr15: q1_org, vr16: q2_org, vr17: q3_org
    vabsd.bu        vr20,   vr13,  vr14    //p0_asub_q0
    vabsd.bu        vr21,   vr12,  vr13    //p1_asub_p0
    vabsd.bu        vr22,   vr15,  vr14    //q1_asub_q0

    vreplgr2vr.b    vr4,    a2          //alpha
    vreplgr2vr.b    vr5,    a3          //beta

    vslt.bu         vr6,    vr20,  vr4   //is_less_than_alpha
    vslt.bu         vr7,    vr21,  vr5   //is_less_than_beta
    vand.v          vr8,    vr6,   vr7   //is_less_than
    vslt.bu         vr7,    vr22,  vr5   //is_less_than_beta
    vand.v          vr8,    vr7,   vr8   //is_less_than
    vand.v          vr8,    vr8,   vr3   //is_less_than
    vsetnez.v       $fcc0,  vr8
    bceqz           $fcc0,  .END_LUMA_8
    vneg.b          vr9,    vr1          //neg_tc_h
    vsllwil.hu.bu   vr18,   vr1,   0     //tc_h.0
    vexth.hu.bu     vr19,   vr1          //tc_h.1
    vexth.h.b       vr2,    vr9          //neg_tc_h.1
    vsllwil.h.b     vr9,    vr9,   0     //neg_tc_h.0

    vsllwil.hu.bu   vr23,   vr12,  0     //p1_org_h.0
    vexth.hu.bu     vr3,    vr12         //p1_org_h.1
    vsllwil.hu.bu   vr24,   vr13,  0     //p0_org_h.0
    vexth.hu.bu     vr4,    vr13         //p0_org_h.1
    vsllwil.hu.bu   vr25,   vr14,  0     //q0_org_h.0
    vexth.hu.bu     vr6,    vr14         //q0_org_h.1

    vabsd.bu        vr0,    vr11,  vr13  //p2_asub_p0
    vslt.bu         vr7,    vr0,   vr5
    vand.v          vr7,    vr8,   vr7   //is_less_than_beta
    vsetnez.v       $fcc0,  vr7
    bceqz           $fcc0,  .END_LUMA_BETA
    vsllwil.hu.bu   vr26,   vr11,  0   //p2_org_h.0
    vexth.hu.bu     vr0,    vr11       //p2_org_h.1
    AVC_LPF_P1_OR_Q1 vr24, vr25, vr23, vr26, vr9, vr18, vr27, vr28, vr29  //vr27: p1_h.0
    AVC_LPF_P1_OR_Q1 vr4, vr6, vr3, vr0, vr2, vr19, vr28, vr29, vr30 //vr28: p1_h.1
    vpickev.b       vr27,   vr28,  vr27
    vbitsel.v       vr12,   vr12,  vr27,  vr7
    vandi.b         vr7,    vr7,   1
    vadd.b          vr1,    vr1,   vr7
.END_LUMA_BETA:
    vabsd.bu        vr26,   vr16,  vr14  //q2_asub_q0
    vslt.bu         vr7,    vr26,  vr5
    vand.v          vr7,    vr7,   vr8
    vsllwil.hu.bu   vr27,   vr15,  0     //q1_org_h.0
    vexth.hu.bu     vr26,   vr15         //q1_org_h.1
    vsetnez.v       $fcc0,  vr7
    bceqz           $fcc0,  .END_LUMA_BETA_SEC
    vsllwil.hu.bu   vr28,   vr16,  0     //q2_org_h.0
    vexth.hu.bu     vr0,    vr16         //q2_org_h.1
    AVC_LPF_P1_OR_Q1 vr24, vr25, vr27, vr28, vr9, vr18, vr29, vr30, vr31  //vr29: q1_h.0
    AVC_LPF_P1_OR_Q1 vr4, vr6, vr26, vr0, vr2, vr19, vr22, vr30, vr31  //vr22:q1_h.1
    vpickev.b       vr29,   vr22,  vr29
    vbitsel.v       vr15,   vr15,  vr29,  vr7
    vandi.b         vr7,    vr7,   1
    vadd.b          vr1,    vr1,   vr7
.END_LUMA_BETA_SEC:
    vneg.b          vr22,   vr1    //neg_thresh_h
    vsllwil.h.b     vr28,   vr22,  0  //neg_thresh_h.0
    vexth.h.b       vr29,   vr22     //neg_thresh_h.1
    vsllwil.hu.bu   vr18,   vr1,   0  //tc_h.0
    vexth.hu.bu     vr1,    vr1       //tc_h.1
    AVC_LPF_P0Q0 vr25, vr24, vr23, vr27, vr28, vr18, vr30, vr31, vr0, vr2
    AVC_LPF_P0Q0 vr6, vr4, vr3, vr26, vr29, vr1, vr20, vr21, vr0, vr2
    vpickev.b       vr30,   vr20,  vr30  //p0_h
    vpickev.b       vr31,   vr21,  vr31  //q0_h
    vbitsel.v       vr13,   vr13,  vr30,  vr8  //p0_org
    vbitsel.v       vr14,   vr14,  vr31,  vr8  //q0_org
    //vr10: p3_org, vr11: p2_org, vr12: p1_org, vr13: p0_org
    //vr14: q0_org, vr15: q1_org, vr16: q2_org, vr17: q3_org

    vilvl.b         vr4,    vr12,  vr10   // row0.0
    vilvl.b         vr5,    vr16,  vr14   // row0.1
    vilvl.b         vr6,    vr13,  vr11   // row2.0
    vilvl.b         vr7,    vr17,  vr15   // row2.1

    vilvh.b         vr8,    vr12,  vr10   // row1.0
    vilvh.b         vr9,    vr16,  vr14   // row1.1
    vilvh.b         vr10,   vr13,  vr11   // row3.0
    vilvh.b         vr11,   vr17,  vr15   // row3.1

    vilvl.b         vr12,   vr6,   vr4    // row4.0
    vilvl.b         vr13,   vr7,   vr5    // row4.1
    vilvl.b         vr14,   vr10,  vr8    // row6.0
    vilvl.b         vr15,   vr11,  vr9    // row6.1

    vilvh.b         vr16,   vr6,   vr4    // row5.0
    vilvh.b         vr17,   vr7,   vr5    // row5.1
    vilvh.b         vr18,   vr10,  vr8    // row7.0
    vilvh.b         vr19,   vr11,  vr9    // row7.1

    vilvl.w         vr4,    vr13,  vr12   // row4: 0, 4, 1, 5
    vilvh.w         vr5,    vr13,  vr12   // row4: 2, 6, 3, 7
    vilvl.w         vr6,    vr17,  vr16   // row5: 0, 4, 1, 5
    vilvh.w         vr7,    vr17,  vr16   // row5: 2, 6, 3, 7

    vilvl.w         vr8,    vr15,  vr14   // row6: 0, 4, 1, 5
    vilvh.w         vr9,    vr15,  vr14   // row6: 2, 6, 3, 7
    vilvl.w         vr10,   vr19,  vr18   // row7: 0, 4, 1, 5
    vilvh.w         vr11,   vr19,  vr18   // row7: 2, 6, 3, 7

    vbsrl.v         vr20,   vr4,   8
    vbsrl.v         vr21,   vr5,   8
    vbsrl.v         vr22,   vr6,   8
    vbsrl.v         vr23,   vr7,   8

    vbsrl.v         vr24,   vr8,   8
    vbsrl.v         vr25,   vr9,   8
    vbsrl.v         vr26,   vr10,  8
    vbsrl.v         vr27,   vr11,  8

    fst.d           f4,     t4,    0
    fstx.d          f20,    t4,    a1
    fstx.d          f5,     t4,    t0
    fstx.d          f21,    t4,    t3
    add.d           t4,     t4,    t1
    fst.d           f6,     t4,    0
    fstx.d          f22,    t4,    a1
    fstx.d          f7,     t4,    t0
    fstx.d          f23,    t4,    t3
    add.d           t4,     t4,    t1
    fst.d           f8,     t4,    0
    fstx.d          f24,    t4,    a1
    fstx.d          f9,     t4,    t0
    fstx.d          f25,    t4,    t3
    add.d           t4,     t4,    t1
    fst.d           f10,    t4,    0
    fstx.d          f26,    t4,    a1
    fstx.d          f11,    t4,    t0
    fstx.d          f27,    t4,    t3

.END_LUMA_8:
    fld.d           f24,    sp,    0
    fld.d           f25,    sp,    8
    fld.d           f26,    sp,    16
    fld.d           f27,    sp,    24
    fld.d           f28,    sp,    32
    fld.d           f29,    sp,    40
    fld.d           f30,    sp,    48
    fld.d           f31,    sp,    56
    addi.d          sp,     sp,    64
endfunc

function ff_h264_v_lpf_luma_8_lsx
    slli.d          t0,     a1,    1   //img_width_2x
    la.local        t4,     vec_shuf
    vldrepl.w       vr0,    a4,    0   //tmp_vec0
    vld             vr1,    t4,    0  //tc_vec
    add.d           t1,     t0,    a1  //img_width_3x
    vshuf.b         vr1,    vr0,   vr0,   vr1   //tc_vec
    addi.d          sp,     sp,    -24
    fst.d           f24,    sp,    0
    fst.d           f25,    sp,    8
    fst.d           f26,    sp,    16
    vslti.b         vr2,    vr1,   0
    vxori.b         vr2,    vr2,   255
    vandi.b         vr2,    vr2,   1    //bs_vec
    vsetnez.v       $fcc0,  vr2
    bceqz           $fcc0,  .END_V_LUMA_8
    sub.d           t2,     a0,    t1   //data - img_width_3x
    vreplgr2vr.b    vr4,    a2          //alpha
    vreplgr2vr.b    vr5,    a3          //beta
    vldi            vr0,    0           //zero
    vld             vr10,   t2,    0    //p2_org
    vldx            vr11,   t2,    a1   //p1_org
    vldx            vr12,   t2,    t0   //p0_org
    vld             vr13,   a0,    0    //q0_org
    vldx            vr14,   a0,    a1   //q1_org

    vslt.bu         vr0,    vr0,   vr2   //is_bs_greater_than0
    vabsd.bu        vr16,   vr11,  vr12  //p1_asub_p0
    vabsd.bu        vr15,   vr12,  vr13  //p0_asub_q0
    vabsd.bu        vr17,   vr14,  vr13  //q1_asub_q0

    vslt.bu         vr6,    vr15,  vr4   //is_less_than_alpha
    vslt.bu         vr7,    vr16,  vr5   //is_less_than_beta
    vand.v          vr8,    vr6,   vr7   //is_less_than
    vslt.bu         vr7,    vr17,  vr5   //is_less_than_beta
    vand.v          vr8,    vr7,   vr8
    vand.v          vr8,    vr8,   vr0  //is_less_than

    vsetnez.v       $fcc0,  vr8
    bceqz           $fcc0,  .END_V_LUMA_8
    vldx            vr15,   a0,    t0    //q2_org
    vneg.b          vr0,    vr1          //neg_tc_h
    vsllwil.h.b     vr18,   vr1,   0     //tc_h.0
    vexth.h.b       vr19,   vr1          //tc_h.1
    vsllwil.h.b     vr9,    vr0,   0     //neg_tc_h.0
    vexth.h.b       vr2,    vr0          //neg_tc_h.1

    vsllwil.hu.bu   vr16,   vr11,  0     //p1_org_h.0
    vexth.hu.bu     vr17,   vr11         //p1_org_h.1
    vsllwil.hu.bu   vr20,   vr12,  0     //p0_org_h.0
    vexth.hu.bu     vr21,   vr12         //p0_org_h.1
    vsllwil.hu.bu   vr22,   vr13,  0     //q0_org_h.0
    vexth.hu.bu     vr23,   vr13         //q0_org_h.1

    vabsd.bu        vr0,    vr10,  vr12  //p2_asub_p0
    vslt.bu         vr7,    vr0,   vr5   //is_less_than_beta
    vand.v          vr7,    vr7,   vr8   //is_less_than_beta

    vsetnez.v       $fcc0,  vr8
    bceqz           $fcc0,  .END_V_LESS_BETA
    vsllwil.hu.bu   vr3,    vr10,  0   //p2_org_h.0
    vexth.hu.bu     vr4,    vr10       //p2_org_h.1
    AVC_LPF_P1_OR_Q1 vr20, vr22, vr16, vr3, vr9, vr18, vr24, vr0, vr26
    AVC_LPF_P1_OR_Q1 vr21, vr23, vr17, vr4, vr2, vr19, vr25, vr0, vr26
    vpickev.b       vr24,   vr25,  vr24
    vbitsel.v       vr24,   vr11,  vr24,   vr7
    addi.d          t3,     t2,    16
    vstx            vr24,   t2,    a1
    vandi.b         vr7,    vr7,   1
    vadd.b          vr1,    vr7,   vr1
.END_V_LESS_BETA:
    vabsd.bu        vr0,    vr15,  vr13   //q2_asub_q0
    vslt.bu         vr7,    vr0,   vr5    //is_less_than_beta
    vand.v          vr7,    vr7,   vr8    //is_less_than_beta
    vsllwil.hu.bu   vr3,    vr14,  0     //q1_org_h.0
    vexth.hu.bu     vr4,    vr14         //q1_org_h.1

    vsetnez.v       $fcc0,  vr7
    bceqz           $fcc0,  .END_V_LESS_BETA_SEC
    vsllwil.hu.bu   vr11,   vr15,  0     //q2_org_h.0
    vexth.hu.bu     vr15,   vr15         //q2_org_h.1
    AVC_LPF_P1_OR_Q1 vr20, vr22, vr3, vr11, vr9, vr18, vr24, vr0, vr26
    AVC_LPF_P1_OR_Q1 vr21, vr23, vr4, vr15, vr2, vr19, vr25, vr0, vr26
    vpickev.b       vr24,   vr25,  vr24
    vbitsel.v       vr24,   vr14,  vr24,   vr7
    vstx            vr24,   a0,    a1
    vandi.b         vr7,    vr7,   1
    vadd.b          vr1,    vr1,   vr7
.END_V_LESS_BETA_SEC:
    vneg.b          vr0,    vr1
    vsllwil.h.b     vr9,    vr0,   0    //neg_thresh_h.0
    vexth.h.b       vr2,    vr0         //neg_thresh_h.1
    vsllwil.hu.bu   vr18,   vr1,   0    //tc_h.0
    vexth.hu.bu     vr19,   vr1         //tc_h.1
    AVC_LPF_P0Q0 vr22, vr20, vr16, vr3, vr9, vr18, vr11, vr15, vr0, vr26
    AVC_LPF_P0Q0 vr23, vr21, vr17, vr4, vr2, vr19, vr10, vr14, vr0, vr26
    vpickev.b       vr11,   vr10,  vr11  //p0_h
    vpickev.b       vr15,   vr14,  vr15  //q0_h
    vbitsel.v       vr11,   vr12,  vr11,   vr8  //p0_h
    vbitsel.v       vr15,   vr13,  vr15,   vr8  //q0_h
    vstx            vr11,   t2,    t0
    vst             vr15,   a0,    0
.END_V_LUMA_8:
    fld.d           f24,    sp,    0
    fld.d           f25,    sp,    8
    fld.d           f26,    sp,    16
    addi.d          sp,     sp,    24
endfunc

.macro AVC_LPF_P0P1P2_OR_Q0Q1Q2 _in0, _in1, _in2, _in3, _in4, _in5  \
                                _out0, _out1, _out2, _tmp0, _const3
    vadd.h          \_tmp0,  \_in1,  \_in2
    vadd.h          \_tmp0,  \_tmp0, \_in3
    vslli.h         \_out2,  \_in0,  1
    vslli.h         \_out0,  \_tmp0, 1
    vadd.h          \_out0,  \_out0, \_in4
    vadd.h          \_out1,  \_in4,  \_tmp0
    vadd.h          \_out0,  \_out0, \_in5
    vmadd.h         \_out2,  \_in4,  \_const3
    vsrar.h         \_out0,  \_out0, \_const3
    vadd.h          \_out2,  \_out2, \_tmp0
    vsrari.h        \_out1,  \_out1, 2
    vsrar.h         \_out2,  \_out2, \_const3
.endm

.macro AVC_LPF_P0_OR_Q0 _in0, _in1, _in2, _out0, _tmp0
    vslli.h         \_tmp0,  \_in2,  1
    vadd.h          \_out0,  \_in0,  \_in1
    vadd.h          \_out0,  \_out0, \_tmp0
    vsrari.h        \_out0,  \_out0, 2
.endm

////LSX optimization is enough for this function.
function ff_h264_h_lpf_luma_intra_8_lsx
    slli.d          t0,     a1,    1   //img_width_2x
    slli.d          t1,     a1,    2   //img_width_4x
    addi.d          t4,     a0,    -4   //src
    addi.d          sp,     sp,    -64
    fst.d           f24,    sp,    0
    fst.d           f25,    sp,    8
    fst.d           f26,    sp,    16
    fst.d           f27,    sp,    24
    fst.d           f28,    sp,    32
    fst.d           f29,    sp,    40
    fst.d           f30,    sp,    48
    fst.d           f31,    sp,    56
    add.d           t2,     t0,    a1   //img_width_3x
    add.d           t5,     t4,    t1
    vld             vr0,    t4,    0    //row0
    vldx            vr1,    t4,    a1   //row1
    vldx            vr2,    t4,    t0   //row2
    vldx            vr3,    t4,    t2   //row3
    add.d           t6,     t5,    t1
    vld             vr4,    t5,    0    //row4
    vldx            vr5,    t5,    a1   //row5
    vldx            vr6,    t5,    t0   //row6
    vldx            vr7,    t5,    t2   //row7
    add.d           t7,     t6,    t1
    vld             vr8,    t6,    0    //row8
    vldx            vr9,    t6,    a1   //row9
    vldx            vr10,   t6,    t0   //row10
    vldx            vr11,   t6,    t2   //row11
    vld             vr12,   t7,    0    //row12
    vldx            vr13,   t7,    a1   //row13
    vldx            vr14,   t7,    t0   //row14
    vldx            vr15,   t7,    t2   //row15
    LSX_TRANSPOSE16X8_B vr0, vr1, vr2, vr3, vr4, vr5, vr6, vr7,       \
                        vr8, vr9, vr10, vr11, vr12, vr13, vr14, vr15, \
                        vr0, vr1, vr2, vr3, vr4, vr5, vr6, vr7,       \
                        vr16, vr17, vr18, vr19, vr20, vr21, vr22, vr23
    // vr0: p3_org, vr1: p2_org, vr2: p1_org, vr3: p0_org
    // vr4: q0_org, vr5: q1_org, vr6: q2_org, vr7: q3_org

    vreplgr2vr.b    vr16,   a2    //alpha_in
    vreplgr2vr.b    vr17,   a3    //beta_in
    vabsd.bu        vr10,   vr3,   vr4    //p0_asub_q0
    vabsd.bu        vr11,   vr2,   vr3    //p1_asub_p0
    vabsd.bu        vr12,   vr5,   vr4    //q1_asub_q0

    vslt.bu         vr8,    vr10,  vr16  //is_less_than_alpha
    vslt.bu         vr9,    vr11,  vr17  //is_less_than_beta
    vand.v          vr18,   vr8,   vr9   //is_less_than
    vslt.bu         vr9,    vr12,  vr17  //is_less_than_beta
    vand.v          vr18,   vr18,  vr9   //is_less_than

    vsetnez.v       $fcc0,  vr18
    bceqz           $fcc0,  .END_H_INTRA_8
    vsrli.b         vr16,   vr16,  2     //less_alpha_shift2_add2
    vaddi.bu        vr16,   vr16,  2
    vslt.bu         vr16,   vr10,  vr16
    vsllwil.hu.bu   vr10,   vr2,   0   //p1_org_h.0
    vexth.hu.bu     vr11,   vr2        //p1_org_h.1
    vsllwil.hu.bu   vr12,   vr3,   0   //p0_org_h.0
    vexth.hu.bu     vr13,   vr3        //p0_org_h.1

    vsllwil.hu.bu   vr14,   vr4,   0   //q0_org_h.0
    vexth.hu.bu     vr15,   vr4        //q0_org_h.1
    vsllwil.hu.bu   vr19,   vr5,   0   //q1_org_h.0
    vexth.hu.bu     vr20,   vr5        //q1_org_h.1

    vabsd.bu        vr21,   vr1,   vr3  //p2_asub_p0
    vslt.bu         vr9,    vr21,  vr17  //is_less_than_beta
    vand.v          vr9,    vr9,   vr16
    vxori.b         vr22,   vr9,   0xff  //negate_is_less_than_beta
    vand.v          vr9,    vr9,   vr18
    vand.v          vr22,   vr22,  vr18

    vsetnez.v       $fcc0,  vr9
    bceqz           $fcc0,  .END_H_INTRA_LESS_BETA
    vsllwil.hu.bu   vr23,   vr1,   0   //p2_org_h.0
    vexth.hu.bu     vr24,   vr1        //p2_org_h.1
    vsllwil.hu.bu   vr25,   vr0,   0   //p3_org_h.0
    vexth.hu.bu     vr26,   vr0        //p3_org_h.1
    vldi            vr27,   0x403

    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr25, vr12, vr14, vr10, vr23, vr19, vr28, vr29, vr30, vr31, vr27
    //vr28: p0_h.0   vr29: p1_h.0  vr30: p2_h.0
    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr26, vr13, vr15, vr11, vr24, vr20, vr23, vr25, vr21, vr31, vr27
    //vr23: p0_h.1   vr25: p1_h.1  vr21: p2_h.1
    vpickev.b       vr28,   vr23,  vr28  //p0_h
    vpickev.b       vr29,   vr25,  vr29  //p1_h
    vpickev.b       vr30,   vr21,  vr30  //p2_h
    vbitsel.v       vr3,    vr3,   vr28,   vr9
    vbitsel.v       vr2,    vr2,   vr29,   vr9
    vbitsel.v       vr1,    vr1,   vr30,   vr9
.END_H_INTRA_LESS_BETA:
    AVC_LPF_P0_OR_Q0 vr12, vr19, vr10, vr23, vr25
    AVC_LPF_P0_OR_Q0 vr13, vr20, vr11, vr24, vr25
    //vr23: p0_h.0   vr24: p0_h.1
    vpickev.b       vr23,   vr24,  vr23
    vbitsel.v       vr3,    vr3,   vr23,   vr22

    vabsd.bu        vr21,   vr6,   vr4   //q2_asub_q0
    vslt.bu         vr9,    vr21,  vr17  //is_less_than_beta
    vand.v          vr9,    vr9,   vr16
    vxori.b         vr22,   vr9,   0xff   //negate_is_less_than_beta
    vand.v          vr9,    vr9,   vr18
    vand.v          vr22,   vr22,  vr18

    vsetnez.v       $fcc0,  vr9
    bceqz           $fcc0,  .END_H_INTRA_LESS_BETA_SEC
    vsllwil.hu.bu   vr23,   vr6,   0   //q2_org_h.0
    vexth.hu.bu     vr24,   vr6        //q2_org_h.1
    vsllwil.hu.bu   vr25,   vr7,   0   //q3_org_h.0
    vexth.hu.bu     vr26,   vr7        //q3_org_h.1
    vldi            vr27,   0x403

    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr25, vr14, vr12, vr19, vr23, vr10, vr28, vr29, vr30, vr31, vr27
    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr26, vr15, vr13, vr20, vr24, vr11, vr23, vr25, vr21, vr31, vr27
    vpickev.b       vr28,   vr23,  vr28  //q0_h
    vpickev.b       vr29,   vr25,  vr29  //q1_h
    vpickev.b       vr30,   vr21,  vr30  //q2_h
    vbitsel.v       vr4,    vr4,   vr28,   vr9
    vbitsel.v       vr5,    vr5,   vr29,   vr9
    vbitsel.v       vr6,    vr6,   vr30,   vr9
.END_H_INTRA_LESS_BETA_SEC:
    AVC_LPF_P0_OR_Q0 vr14, vr10, vr19, vr23, vr25
    AVC_LPF_P0_OR_Q0 vr15, vr11, vr20, vr24, vr25
    vpickev.b       vr23,   vr24,  vr23
    vbitsel.v       vr4,    vr4,   vr23,   vr22

    vilvl.b         vr14,   vr2,   vr0   // row0.0
    vilvl.b         vr15,   vr6,   vr4   // row0.1
    vilvl.b         vr16,   vr3,   vr1   // row2.0
    vilvl.b         vr17,   vr7,   vr5   // row2.1

    vilvh.b         vr18,   vr2,   vr0   // row1.0
    vilvh.b         vr19,   vr6,   vr4   // row1.1
    vilvh.b         vr20,   vr3,   vr1   // row3.0
    vilvh.b         vr21,   vr7,   vr5   // row3.1

    vilvl.b         vr2,    vr16,   vr14    // row4.0
    vilvl.b         vr3,    vr17,   vr15    // row4.1
    vilvl.b         vr4,    vr20,   vr18    // row6.0
    vilvl.b         vr5,    vr21,   vr19    // row6.1

    vilvh.b         vr6,    vr16,   vr14    // row5.0
    vilvh.b         vr7,    vr17,   vr15    // row5.1
    vilvh.b         vr8,    vr20,   vr18    // row7.0
    vilvh.b         vr9,    vr21,   vr19    // row7.1

    vilvl.w         vr14,   vr3,    vr2   // row4: 0, 4, 1, 5
    vilvh.w         vr15,   vr3,    vr2   // row4: 2, 6, 3, 7
    vilvl.w         vr16,   vr7,    vr6   // row5: 0, 4, 1, 5
    vilvh.w         vr17,   vr7,    vr6   // row5: 2, 6, 3, 7

    vilvl.w         vr18,   vr5,    vr4   // row6: 0, 4, 1, 5
    vilvh.w         vr19,   vr5,    vr4   // row6: 2, 6, 3, 7
    vilvl.w         vr20,   vr9,    vr8   // row7: 0, 4, 1, 5
    vilvh.w         vr21,   vr9,    vr8   // row7: 2, 6, 3, 7

    vbsrl.v         vr0,    vr14,   8
    vbsrl.v         vr1,    vr15,   8
    vbsrl.v         vr2,    vr16,   8
    vbsrl.v         vr3,    vr17,   8

    vbsrl.v         vr4,    vr18,   8
    vbsrl.v         vr5,    vr19,   8
    vbsrl.v         vr6,    vr20,   8
    vbsrl.v         vr7,    vr21,   8

    fst.d           f14,    t4,    0
    fstx.d          f0,     t4,    a1
    fstx.d          f15,    t4,    t0
    fstx.d          f1,     t4,    t2
    fst.d           f16,    t5,    0
    fstx.d          f2,     t5,    a1
    fstx.d          f17,    t5,    t0
    fstx.d          f3,     t5,    t2
    fst.d           f18,    t6,    0
    fstx.d          f4,     t6,    a1
    fstx.d          f19,    t6,    t0
    fstx.d          f5,     t6,    t2
    fst.d           f20,    t7,    0
    fstx.d          f6,     t7,    a1
    fstx.d          f21,    t7,    t0
    fstx.d          f7,     t7,    t2
.END_H_INTRA_8:
    fld.d           f24,    sp,    0
    fld.d           f25,    sp,    8
    fld.d           f26,    sp,    16
    fld.d           f27,    sp,    24
    fld.d           f28,    sp,    32
    fld.d           f29,    sp,    40
    fld.d           f30,    sp,    48
    fld.d           f31,    sp,    56
    addi.d          sp,     sp,    64
endfunc

//LSX optimization is enough for this function.
function ff_h264_v_lpf_luma_intra_8_lsx
    slli.d          t0,     a1,    1   //img_width_2x
    add.d           t1,     t0,    a1  //img_width_3x
    addi.d          sp,     sp,    -64
    fst.d           f24,    sp,    0
    fst.d           f25,    sp,    8
    fst.d           f26,    sp,    16
    fst.d           f27,    sp,    24
    fst.d           f28,    sp,    32
    fst.d           f29,    sp,    40
    fst.d           f30,    sp,    48
    fst.d           f31,    sp,    56
    sub.d           t4,     a0,    t1  //src - img_width_3x

    vld             vr0,    a0,    0   //q0_org
    vldx            vr1,    a0,    a1  //q1_org
    vldx            vr2,    t4,    a1  //p1_org
    vldx            vr3,    t4,    t0  //p0_org

    vreplgr2vr.b    vr4,    a2   //alpha
    vreplgr2vr.b    vr5,    a3   //beta

    vabsd.bu        vr6,    vr3,   vr0   //p0_asub_q0
    vabsd.bu        vr7,    vr2,   vr3   //p1_asub_p0
    vabsd.bu        vr8,    vr1,   vr0   //q1_asub_q0

    vslt.bu         vr9,    vr6,   vr4  //is_less_than_alpha
    vslt.bu         vr10,   vr7,   vr5  //is_less_than_beta
    vand.v          vr11,   vr9,   vr10  //is_less_than
    vslt.bu         vr10,   vr8,   vr5
    vand.v          vr11,   vr10,  vr11

    vsetnez.v       $fcc0,  vr11
    bceqz           $fcc0,  .END_V_INTRA_8

    vld             vr12,   t4,    0   //p2_org
    vldx            vr13,   a0,    t0  //q2_org
    vsrli.b         vr14,   vr4,   2   //is_alpha_shift2_add2
    vsllwil.hu.bu   vr15,   vr2,   0  //p1_org_h.0
    vexth.hu.bu     vr16,   vr2       //p1_org_h.1
    vaddi.bu        vr14,   vr14,  2
    vsllwil.hu.bu   vr17,   vr3,   0  //p0_org_h.0
    vexth.hu.bu     vr18,   vr3       //p0_org_h.1
    vslt.bu         vr14,   vr6,   vr14
    vsllwil.hu.bu   vr19,   vr0,   0  //q0_org_h.0
    vexth.hu.bu     vr20,   vr0       //q0_org_h.1
    vsllwil.hu.bu   vr21,   vr1,   0  //q1_org_h.0
    vexth.hu.bu     vr22,   vr1       //q1_org_h.1

    vabsd.bu        vr23,   vr12,  vr3  //p2_asub_p0
    vslt.bu         vr10,   vr23,  vr5  //is_less_than_beta
    vand.v          vr10,   vr10,  vr14
    vxori.b         vr23,   vr10,  0xff //negate_is_less_than_beta
    vand.v          vr10,   vr10,  vr11
    vand.v          vr23,   vr23,  vr11

    vsetnez.v       $fcc0,  vr10
    bceqz           $fcc0,  .END_V_INTRA_LESS_BETA
    sub.d           t5,     t4,    a1
    vld             vr24,   t5,    0  //p3_org
    vsllwil.hu.bu   vr26,   vr12,  0  //p2_org_h.0
    vexth.hu.bu     vr27,   vr12      //p2_org_h.1
    vsllwil.hu.bu   vr28,   vr24,  0  //p3_org_h.0
    vexth.hu.bu     vr29,   vr24      //p3_org_h.1
    vldi            vr4,    0x403

    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr28, vr17, vr19, vr15, vr26, vr21, vr25, vr30, vr31, vr24, vr4
    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr29, vr18, vr20, vr16, vr27, vr22, vr6, vr7, vr8, vr24, vr4

    vpickev.b       vr25,   vr6,   vr25  //p0_h
    vpickev.b       vr30,   vr7,   vr30  //p1_h
    vpickev.b       vr31,   vr8,   vr31  //p2_h

    vbitsel.v       vr3,    vr3,   vr25,   vr10
    vbitsel.v       vr2,    vr2,   vr30,   vr10
    vbitsel.v       vr12,   vr12,  vr31,   vr10

    vstx            vr2,    t4,    a1
    vst             vr12,   t4,    0
.END_V_INTRA_LESS_BETA:
    AVC_LPF_P0_OR_Q0 vr17, vr21, vr15, vr24, vr30
    AVC_LPF_P0_OR_Q0 vr18, vr22, vr16, vr25, vr30
    vpickev.b       vr24,   vr25,  vr24
    vbitsel.v       vr3,    vr3,   vr24,   vr23
    vstx            vr3,    t4,    t0

    vabsd.bu        vr23,   vr13,  vr0   //q2_asub_q0
    vslt.bu         vr10,   vr23,  vr5   //is_less_than_beta
    vand.v          vr10,   vr10,  vr14
    vxori.b         vr23,   vr10,  0xff  //negate_is_less_than_beta
    vand.v          vr10,   vr10,  vr11
    vand.v          vr23,   vr23,  vr11

    vsetnez.v       $fcc0,  vr10
    bceqz           $fcc0,  .END_V_INTRA_LESS_BETA_SEC
    vldx            vr24,   a0,    t1  //q3_org

    vsllwil.hu.bu   vr26,   vr13,  0  //q2_org_h.0
    vexth.hu.bu     vr27,   vr13      //q2_org_h.1
    vsllwil.hu.bu   vr28,   vr24,  0  //q3_org_h.0
    vexth.hu.bu     vr29,   vr24      //q3_org_h.1
    vldi            vr4,    0x403

    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr28, vr19, vr17, vr21, vr26, vr15, vr25, vr30, vr31, vr24, vr4
    AVC_LPF_P0P1P2_OR_Q0Q1Q2 vr29, vr20, vr18, vr22, vr27, vr16, vr6, vr7, vr8, vr24, vr4

    vpickev.b       vr25,   vr6,   vr25
    vpickev.b       vr30,   vr7,   vr30
    vpickev.b       vr31,   vr8,   vr31

    vbitsel.v       vr0,    vr0,   vr25,   vr10
    vbitsel.v       vr1,    vr1,   vr30,   vr10
    vbitsel.v       vr13,   vr13,  vr31,   vr10
    vstx            vr1,    a0,    a1
    vstx            vr13,   a0,    t0
.END_V_INTRA_LESS_BETA_SEC:
    AVC_LPF_P0_OR_Q0 vr19, vr15, vr21, vr24, vr30
    AVC_LPF_P0_OR_Q0 vr20, vr16, vr22, vr25, vr30
    vpickev.b       vr24,   vr25,  vr24
    vbitsel.v       vr0,    vr0,   vr24,   vr23
    vst             vr0,    a0,    0
.END_V_INTRA_8:
    fld.d           f24,    sp,    0
    fld.d           f25,    sp,    8
    fld.d           f26,    sp,    16
    fld.d           f27,    sp,    24
    fld.d           f28,    sp,    32
    fld.d           f29,    sp,    40
    fld.d           f30,    sp,    48
    fld.d           f31,    sp,    56
    addi.d          sp,     sp,    64
endfunc

function ff_h264_h_lpf_chroma_intra_8_lsx
    addi.d          t4,     a0,    -2
    slli.d          t0,     a1,    1   //img_2x
    slli.d          t2,     a1,    2   //img_4x
    add.d           t1,     t0,    a1  //img_3x

    add.d           t5,     t4,    t2
    fld.s           f0,     t4,    0   //row0
    fldx.s          f1,     t4,    a1  //row1
    fldx.s          f2,     t4,    t0  //row2
    fldx.s          f3,     t4,    t1  //row3
    fld.s           f4,     t5,    0   //row4
    fldx.s          f5,     t5,    a1  //row5
    fldx.s          f6,     t5,    t0  //row6
    fldx.s          f7,     t5,    t1  //row7

    vilvl.b         vr8,    vr2,   vr0  //p1_org
    vilvl.b         vr9,    vr3,   vr1  //p0_org
    vilvl.b         vr10,   vr6,   vr4  //q0_org
    vilvl.b         vr11,   vr7,   vr5  //q1_org

    vilvl.b         vr0,    vr9,   vr8
    vilvl.b         vr1,    vr11,  vr10
    vilvl.w         vr2,    vr1,   vr0
    vilvh.w         vr3,    vr1,   vr0

    vilvl.d         vr8,    vr2,   vr2   //p1_org
    vilvh.d         vr9,    vr2,   vr2   //p0_org
    vilvl.d         vr10,   vr3,   vr3   //q0_org
    vilvh.d         vr11,   vr3,   vr3   //q1_org

    vreplgr2vr.b    vr0,    a2   //alpha
    vreplgr2vr.b    vr1,    a3   //beta

    vabsd.bu        vr2,    vr9,   vr10  //p0_asub_q0
    vabsd.bu        vr3,    vr8,   vr9   //p1_asub_p0
    vabsd.bu        vr4,    vr11,  vr10  //q1_asub_q0

    vslt.bu         vr5,    vr2,   vr0  //is_less_than_alpha
    vslt.bu         vr6,    vr3,   vr1  //is_less_than_beta
    vand.v          vr7,    vr5,   vr6   //is_less_than
    vslt.bu         vr6,    vr4,   vr1
    vand.v          vr7,    vr7,   vr6

    vsetnez.v       $fcc0,  vr7
    bceqz           $fcc0,  .END_H_CHROMA_INTRA_8

    vexth.hu.bu     vr12,   vr8   //p1_org_h
    vexth.hu.bu     vr13,   vr9   //p0_org_h
    vexth.hu.bu     vr14,   vr10  //q0_org_h
    vexth.hu.bu     vr15,   vr11  //q1_org_h

    AVC_LPF_P0_OR_Q0 vr13, vr15, vr12, vr16, vr18
    AVC_LPF_P0_OR_Q0 vr14, vr12, vr15, vr17, vr18

    vpickev.b       vr18,   vr16,   vr16
    vpickev.b       vr19,   vr17,   vr17
    vbitsel.v       vr9,    vr9,    vr18,   vr7
    vbitsel.v       vr10,   vr10,   vr19,   vr7
.END_H_CHROMA_INTRA_8:
    vilvl.b         vr11,   vr10,   vr9
    addi.d          t4,     t4,     1
    vstelm.h        vr11,   t4,     0,      0
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      1
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      2
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      3
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      4
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      5
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      6
    add.d           t4,     t4,     a1
    vstelm.h        vr11,   t4,     0,      7
endfunc

function ff_h264_v_lpf_chroma_intra_8_lsx
    slli.d          t0,     a1,     1    //img_width_2x
    sub.d           t2,     a0,     a1
    sub.d           t1,     a0,     t0   //data - img_width_2x

    vreplgr2vr.b    vr0,    a2
    vreplgr2vr.b    vr1,    a3

    vld             vr2,    t1,     0   //p1_org
    vldx            vr3,    t1,     a1  //p0_org
    vld             vr4,    a0,     0   //q0_org
    vldx            vr5,    a0,     a1  //q1_org

    vabsd.bu        vr6,    vr3,    vr4  //p0_asub_q0
    vabsd.bu        vr7,    vr2,    vr3  //p1_asub_p0
    vabsd.bu        vr8,    vr5,    vr4  //q1_asub_q0

    vslt.bu         vr9,    vr6,    vr0  //is_less_than_alpha
    vslt.bu         vr10,   vr7,    vr1  //is_less_than_beta
    vand.v          vr11,   vr9,    vr10  //is_less_than
    vslt.bu         vr10,   vr8,    vr1
    vand.v          vr11,   vr10,   vr11

    vsetnez.v       $fcc0,  vr11
    bceqz           $fcc0,  .END_V_CHROMA_INTRA_8

    vsllwil.hu.bu   vr6,    vr2,    0  //p1_org_h.0
    vsllwil.hu.bu   vr8,    vr3,    0  //p0_org_h.0
    vsllwil.hu.bu   vr13,   vr4,    0  //q0_org_h.0
    vsllwil.hu.bu   vr15,   vr5,    0  //q1_org_h.0

    AVC_LPF_P0_OR_Q0 vr8, vr15, vr6, vr17, vr23
    AVC_LPF_P0_OR_Q0 vr13, vr6, vr15, vr18, vr23

    vpickev.b       vr19,   vr17,   vr17
    vpickev.b       vr20,   vr18,   vr18
    vbitsel.v       vr3,    vr3,    vr19,   vr11
    vbitsel.v       vr4,    vr4,    vr20,   vr11

    vstelm.d        vr3,    t2,     0,      0
    vstelm.d        vr4,    a0,     0,      0
.END_V_CHROMA_INTRA_8:
endfunc

function ff_biweight_h264_pixels16_8_lsx
    slli.d          t0,     a2,     1
    slli.d          t2,     a2,     2
    add.d           t1,     t0,     a2
    addi.d          a7,     a7,     1
    ori             a7,     a7,     1
    sll.d           a7,     a7,     a4
    addi.d          a4,     a4,     1

    vreplgr2vr.b    vr0,    a6    //tmp0
    vreplgr2vr.b    vr1,    a5    //tmp1
    vreplgr2vr.h    vr8,    a7    //offset
    vreplgr2vr.h    vr9,    a4    //denom
    vilvh.b         vr20,   vr1,    vr0    //wgt

    add.d           t4,     a1,     t2
    vld             vr0,    a1,     0
    vldx            vr1,    a1,     a2
    vldx            vr2,    a1,     t0
    vldx            vr3,    a1,     t1
    vld             vr4,    t4,     0
    vldx            vr5,    t4,     a2
    vldx            vr6,    t4,     t0
    vldx            vr7,    t4,     t1

    add.d           t5,     a0,     t2
    vld             vr10,   a0,     0
    vldx            vr11,   a0,     a2
    vldx            vr12,   a0,     t0
    vldx            vr13,   a0,     t1
    vld             vr14,   t5,     0
    vldx            vr15,   t5,     a2
    vldx            vr16,   t5,     t0
    vldx            vr17,   t5,     t1

    vilvl.b         vr18,   vr10,   vr0
    vilvl.b         vr19,   vr11,   vr1
    vilvl.b         vr21,   vr12,   vr2
    vilvl.b         vr22,   vr13,   vr3
    vilvh.b         vr0,    vr10,   vr0
    vilvh.b         vr1,    vr11,   vr1
    vilvh.b         vr2,    vr12,   vr2
    vilvh.b         vr3,    vr13,   vr3

    vilvl.b         vr10,   vr14,   vr4
    vilvl.b         vr11,   vr15,   vr5
    vilvl.b         vr12,   vr16,   vr6
    vilvl.b         vr13,   vr17,   vr7
    vilvh.b         vr14,   vr14,   vr4
    vilvh.b         vr15,   vr15,   vr5
    vilvh.b         vr16,   vr16,   vr6
    vilvh.b         vr17,   vr17,   vr7

    vmov            vr4,    vr8
    vmov            vr5,    vr8
    vmov            vr6,    vr8
    vmov            vr7,    vr8
    vmaddwev.h.bu.b vr4,    vr18,   vr20
    vmaddwev.h.bu.b vr5,    vr19,   vr20
    vmaddwev.h.bu.b vr6,    vr21,   vr20
    vmaddwev.h.bu.b vr7,    vr22,   vr20
    vmaddwod.h.bu.b vr4,    vr18,   vr20
    vmaddwod.h.bu.b vr5,    vr19,   vr20
    vmaddwod.h.bu.b vr6,    vr21,   vr20
    vmaddwod.h.bu.b vr7,    vr22,   vr20
    vmov            vr18,   vr8
    vmov            vr19,   vr8
    vmov            vr21,   vr8
    vmov            vr22,   vr8
    vmaddwev.h.bu.b vr18,   vr0,    vr20
    vmaddwev.h.bu.b vr19,   vr1,    vr20
    vmaddwev.h.bu.b vr21,   vr2,    vr20
    vmaddwev.h.bu.b vr22,   vr3,    vr20
    vmaddwod.h.bu.b vr18,   vr0,    vr20
    vmaddwod.h.bu.b vr19,   vr1,    vr20
    vmaddwod.h.bu.b vr21,   vr2,    vr20
    vmaddwod.h.bu.b vr22,   vr3,    vr20
    vmov            vr0,    vr8
    vmov            vr1,    vr8
    vmov            vr2,    vr8
    vmov            vr3,    vr8
    vmaddwev.h.bu.b vr0,    vr10,   vr20
    vmaddwev.h.bu.b vr1,    vr11,   vr20
    vmaddwev.h.bu.b vr2,    vr12,   vr20
    vmaddwev.h.bu.b vr3,    vr13,   vr20
    vmaddwod.h.bu.b vr0,    vr10,   vr20
    vmaddwod.h.bu.b vr1,    vr11,   vr20
    vmaddwod.h.bu.b vr2,    vr12,   vr20
    vmaddwod.h.bu.b vr3,    vr13,   vr20
    vmov            vr10,   vr8
    vmov            vr11,   vr8
    vmov            vr12,   vr8
    vmov            vr13,   vr8
    vmaddwev.h.bu.b vr10,   vr14,   vr20
    vmaddwev.h.bu.b vr11,   vr15,   vr20
    vmaddwev.h.bu.b vr12,   vr16,   vr20
    vmaddwev.h.bu.b vr13,   vr17,   vr20
    vmaddwod.h.bu.b vr10,   vr14,   vr20
    vmaddwod.h.bu.b vr11,   vr15,   vr20
    vmaddwod.h.bu.b vr12,   vr16,   vr20
    vmaddwod.h.bu.b vr13,   vr17,   vr20

    vssran.bu.h     vr4,    vr4,    vr9
    vssran.bu.h     vr5,    vr5,    vr9
    vssran.bu.h     vr6,    vr6,    vr9
    vssran.bu.h     vr7,    vr7,    vr9
    vssran.bu.h     vr18,   vr18,   vr9
    vssran.bu.h     vr19,   vr19,   vr9
    vssran.bu.h     vr21,   vr21,   vr9
    vssran.bu.h     vr22,   vr22,   vr9
    vssran.bu.h     vr0,    vr0,    vr9
    vssran.bu.h     vr1,    vr1,    vr9
    vssran.bu.h     vr2,    vr2,    vr9
    vssran.bu.h     vr3,    vr3,    vr9
    vssran.bu.h     vr10,   vr10,   vr9
    vssran.bu.h     vr11,   vr11,   vr9
    vssran.bu.h     vr12,   vr12,   vr9
    vssran.bu.h     vr13,   vr13,   vr9

    vilvl.d         vr4,    vr18,   vr4
    vilvl.d         vr5,    vr19,   vr5
    vilvl.d         vr6,    vr21,   vr6
    vilvl.d         vr7,    vr22,   vr7
    vilvl.d         vr0,    vr10,   vr0
    vilvl.d         vr1,    vr11,   vr1
    vilvl.d         vr2,    vr12,   vr2
    vilvl.d         vr3,    vr13,   vr3

    vst             vr4,    a0,     0
    vstx            vr5,    a0,     a2
    vstx            vr6,    a0,     t0
    vstx            vr7,    a0,     t1
    vst             vr0,    t5,     0
    vstx            vr1,    t5,     a2
    vstx            vr2,    t5,     t0
    vstx            vr3,    t5,     t1

    addi.d          t6,     zero,   16
    bne             a3,     t6,     .END_BIWEIGHT_PIXELS16
    add.d           t4,     t4,     t2
    add.d           t5,     t5,     t2
    vld             vr0,    t4,     0
    vldx            vr1,    t4,     a2
    vldx            vr2,    t4,     t0
    vldx            vr3,    t4,     t1
    add.d           t6,     t4,     t2
    add.d           t7,     t5,     t2
    vld             vr4,    t6,     0
    vldx            vr5,    t6,     a2
    vldx            vr6,    t6,     t0
    vldx            vr7,    t6,     t1

    vld             vr10,   t5,     0
    vldx            vr11,   t5,     a2
    vldx            vr12,   t5,     t0
    vldx            vr13,   t5,     t1
    vld             vr14,   t7,     0
    vldx            vr15,   t7,     a2
    vldx            vr16,   t7,     t0
    vldx            vr17,   t7,     t1

    vilvl.b         vr18,   vr10,   vr0
    vilvl.b         vr19,   vr11,   vr1
    vilvl.b         vr21,   vr12,   vr2
    vilvl.b         vr22,   vr13,   vr3
    vilvh.b         vr0,    vr10,   vr0
    vilvh.b         vr1,    vr11,   vr1
    vilvh.b         vr2,    vr12,   vr2
    vilvh.b         vr3,    vr13,   vr3

    vilvl.b         vr10,   vr14,   vr4
    vilvl.b         vr11,   vr15,   vr5
    vilvl.b         vr12,   vr16,   vr6
    vilvl.b         vr13,   vr17,   vr7
    vilvh.b         vr14,   vr14,   vr4
    vilvh.b         vr15,   vr15,   vr5
    vilvh.b         vr16,   vr16,   vr6
    vilvh.b         vr17,   vr17,   vr7

    vmov            vr4,    vr8
    vmov            vr5,    vr8
    vmov            vr6,    vr8
    vmov            vr7,    vr8
    vmaddwev.h.bu.b vr4,    vr18,   vr20
    vmaddwev.h.bu.b vr5,    vr19,   vr20
    vmaddwev.h.bu.b vr6,    vr21,   vr20
    vmaddwev.h.bu.b vr7,    vr22,   vr20
    vmaddwod.h.bu.b vr4,    vr18,   vr20
    vmaddwod.h.bu.b vr5,    vr19,   vr20
    vmaddwod.h.bu.b vr6,    vr21,   vr20
    vmaddwod.h.bu.b vr7,    vr22,   vr20
    vmov            vr18,   vr8
    vmov            vr19,   vr8
    vmov            vr21,   vr8
    vmov            vr22,   vr8
    vmaddwev.h.bu.b vr18,   vr0,    vr20
    vmaddwev.h.bu.b vr19,   vr1,    vr20
    vmaddwev.h.bu.b vr21,   vr2,    vr20
    vmaddwev.h.bu.b vr22,   vr3,    vr20
    vmaddwod.h.bu.b vr18,   vr0,    vr20
    vmaddwod.h.bu.b vr19,   vr1,    vr20
    vmaddwod.h.bu.b vr21,   vr2,    vr20
    vmaddwod.h.bu.b vr22,   vr3,    vr20
    vmov            vr0,    vr8
    vmov            vr1,    vr8
    vmov            vr2,    vr8
    vmov            vr3,    vr8
    vmaddwev.h.bu.b vr0,    vr10,   vr20
    vmaddwev.h.bu.b vr1,    vr11,   vr20
    vmaddwev.h.bu.b vr2,    vr12,   vr20
    vmaddwev.h.bu.b vr3,    vr13,   vr20
    vmaddwod.h.bu.b vr0,    vr10,   vr20
    vmaddwod.h.bu.b vr1,    vr11,   vr20
    vmaddwod.h.bu.b vr2,    vr12,   vr20
    vmaddwod.h.bu.b vr3,    vr13,   vr20
    vmov            vr10,   vr8
    vmov            vr11,   vr8
    vmov            vr12,   vr8
    vmov            vr13,   vr8
    vmaddwev.h.bu.b vr10,   vr14,   vr20
    vmaddwev.h.bu.b vr11,   vr15,   vr20
    vmaddwev.h.bu.b vr12,   vr16,   vr20
    vmaddwev.h.bu.b vr13,   vr17,   vr20
    vmaddwod.h.bu.b vr10,   vr14,   vr20
    vmaddwod.h.bu.b vr11,   vr15,   vr20
    vmaddwod.h.bu.b vr12,   vr16,   vr20
    vmaddwod.h.bu.b vr13,   vr17,   vr20

    vssran.bu.h     vr4,    vr4,    vr9
    vssran.bu.h     vr5,    vr5,    vr9
    vssran.bu.h     vr6,    vr6,    vr9
    vssran.bu.h     vr7,    vr7,    vr9
    vssran.bu.h     vr18,   vr18,   vr9
    vssran.bu.h     vr19,   vr19,   vr9
    vssran.bu.h     vr21,   vr21,   vr9
    vssran.bu.h     vr22,   vr22,   vr9
    vssran.bu.h     vr0,    vr0,    vr9
    vssran.bu.h     vr1,    vr1,    vr9
    vssran.bu.h     vr2,    vr2,    vr9
    vssran.bu.h     vr3,    vr3,    vr9
    vssran.bu.h     vr10,   vr10,   vr9
    vssran.bu.h     vr11,   vr11,   vr9
    vssran.bu.h     vr12,   vr12,   vr9
    vssran.bu.h     vr13,   vr13,   vr9

    vilvl.d         vr4,    vr18,   vr4
    vilvl.d         vr5,    vr19,   vr5
    vilvl.d         vr6,    vr21,   vr6
    vilvl.d         vr7,    vr22,   vr7
    vilvl.d         vr0,    vr10,   vr0
    vilvl.d         vr1,    vr11,   vr1
    vilvl.d         vr2,    vr12,   vr2
    vilvl.d         vr3,    vr13,   vr3

    vst             vr4,    t5,     0
    vstx            vr5,    t5,     a2
    vstx            vr6,    t5,     t0
    vstx            vr7,    t5,     t1
    vst             vr0,    t7,     0
    vstx            vr1,    t7,     a2
    vstx            vr2,    t7,     t0
    vstx            vr3,    t7,     t1
.END_BIWEIGHT_PIXELS16:
endfunc

function ff_biweight_h264_pixels16_8_lasx
    slli.d           t0,     a2,     1
    slli.d           t2,     a2,     2
    add.d            t1,     t0,     a2
    addi.d           a7,     a7,     1
    ori              a7,     a7,     1
    sll.d            a7,     a7,     a4
    addi.d           a4,     a4,     1

    xvreplgr2vr.b    xr0,    a6    //tmp0
    xvreplgr2vr.b    xr1,    a5    //tmp1
    xvreplgr2vr.h    xr8,    a7    //offset
    xvreplgr2vr.h    xr9,    a4    //denom
    xvilvh.b         xr20,   xr1,    xr0    //wgt

    add.d            t4,     a1,     t2
    vld              vr0,    a1,     0
    vldx             vr1,    a1,     a2
    vldx             vr2,    a1,     t0
    vldx             vr3,    a1,     t1
    vld              vr4,    t4,     0
    vldx             vr5,    t4,     a2
    vldx             vr6,    t4,     t0
    vldx             vr7,    t4,     t1

    add.d            t5,     a0,     t2
    vld              vr10,   a0,     0
    vldx             vr11,   a0,     a2
    vldx             vr12,   a0,     t0
    vldx             vr13,   a0,     t1
    vld              vr14,   t5,     0
    vldx             vr15,   t5,     a2
    vldx             vr16,   t5,     t0
    vldx             vr17,   t5,     t1

    xvpermi.q        xr1,    xr0,    0x20
    xvpermi.q        xr3,    xr2,    0x20
    xvpermi.q        xr5,    xr4,    0x20
    xvpermi.q        xr7,    xr6,    0x20

    xvpermi.q        xr11,   xr10,   0x20
    xvpermi.q        xr13,   xr12,   0x20
    xvpermi.q        xr15,   xr14,   0x20
    xvpermi.q        xr17,   xr16,   0x20

    xvilvl.b         xr0,    xr11,   xr1   //vec0
    xvilvl.b         xr2,    xr13,   xr3   //vec2
    xvilvl.b         xr4,    xr15,   xr5   //vec4
    xvilvl.b         xr6,    xr17,   xr7   //vec6

    xvilvh.b         xr10,   xr11,   xr1   //vec1
    xvilvh.b         xr12,   xr13,   xr3   //vec2
    xvilvh.b         xr14,   xr15,   xr5   //vec5
    xvilvh.b         xr16,   xr17,   xr7   //vec7

    xmov             xr1,    xr8
    xmov             xr3,    xr8
    xmov             xr5,    xr8
    xmov             xr7,    xr8
    xvmaddwev.h.bu.b xr1,    xr0,   xr20
    xvmaddwev.h.bu.b xr3,    xr2,   xr20
    xvmaddwev.h.bu.b xr5,    xr4,   xr20
    xvmaddwev.h.bu.b xr7,    xr6,   xr20
    xvmaddwod.h.bu.b xr1,    xr0,   xr20
    xvmaddwod.h.bu.b xr3,    xr2,   xr20
    xvmaddwod.h.bu.b xr5,    xr4,   xr20
    xvmaddwod.h.bu.b xr7,    xr6,   xr20
    xmov             xr11,   xr8
    xmov             xr13,   xr8
    xmov             xr15,   xr8
    xmov             xr17,   xr8
    xvmaddwev.h.bu.b xr11,   xr10,  xr20
    xvmaddwev.h.bu.b xr13,   xr12,  xr20
    xvmaddwev.h.bu.b xr15,   xr14,  xr20
    xvmaddwev.h.bu.b xr17,   xr16,  xr20
    xvmaddwod.h.bu.b xr11,   xr10,  xr20
    xvmaddwod.h.bu.b xr13,   xr12,  xr20
    xvmaddwod.h.bu.b xr15,   xr14,  xr20
    xvmaddwod.h.bu.b xr17,   xr16,  xr20

    xvssran.bu.h     xr1,    xr1,   xr9   //vec0
    xvssran.bu.h     xr3,    xr3,   xr9   //vec2
    xvssran.bu.h     xr5,    xr5,   xr9   //vec4
    xvssran.bu.h     xr7,    xr7,   xr9   //vec6
    xvssran.bu.h     xr11,   xr11,  xr9   //vec1
    xvssran.bu.h     xr13,   xr13,  xr9   //vec3
    xvssran.bu.h     xr15,   xr15,  xr9   //vec5
    xvssran.bu.h     xr17,   xr17,  xr9   //vec7

    xvilvl.d         xr0,    xr11,   xr1
    xvilvl.d         xr2,    xr13,   xr3
    xvilvl.d         xr4,    xr15,   xr5
    xvilvl.d         xr6,    xr17,   xr7

    xvpermi.d        xr1,    xr0,    0x4E
    xvpermi.d        xr3,    xr2,    0x4E
    xvpermi.d        xr5,    xr4,    0x4E
    xvpermi.d        xr7,    xr6,    0x4E
    vst              vr0,    a0,     0
    vstx             vr1,    a0,     a2
    vstx             vr2,    a0,     t0
    vstx             vr3,    a0,     t1
    vst              vr4,    t5,     0
    vstx             vr5,    t5,     a2
    vstx             vr6,    t5,     t0
    vstx             vr7,    t5,     t1

    addi.d          t6,     zero,   16
    bne             a3,     t6,     .END_BIWEIGHT_PIXELS16_LASX
    add.d           t4,     t4,     t2
    add.d           t5,     t5,     t2
    vld             vr0,    t4,     0
    vldx            vr1,    t4,     a2
    vldx            vr2,    t4,     t0
    vldx            vr3,    t4,     t1
    add.d           t6,     t4,     t2
    add.d           t7,     t5,     t2
    vld             vr4,    t6,     0
    vldx            vr5,    t6,     a2
    vldx            vr6,    t6,     t0
    vldx            vr7,    t6,     t1

    vld             vr10,   t5,     0
    vldx            vr11,   t5,     a2
    vldx            vr12,   t5,     t0
    vldx            vr13,   t5,     t1
    vld             vr14,   t7,     0
    vldx            vr15,   t7,     a2
    vldx            vr16,   t7,     t0
    vldx            vr17,   t7,     t1

    xvpermi.q        xr1,    xr0,    0x20
    xvpermi.q        xr3,    xr2,    0x20
    xvpermi.q        xr5,    xr4,    0x20
    xvpermi.q        xr7,    xr6,    0x20

    xvpermi.q        xr11,   xr10,   0x20
    xvpermi.q        xr13,   xr12,   0x20
    xvpermi.q        xr15,   xr14,   0x20
    xvpermi.q        xr17,   xr16,   0x20

    xvilvl.b         xr0,    xr11,   xr1   //vec0
    xvilvl.b         xr2,    xr13,   xr3   //vec2
    xvilvl.b         xr4,    xr15,   xr5   //vec4
    xvilvl.b         xr6,    xr17,   xr7   //vec6

    xvilvh.b         xr10,   xr11,   xr1   //vec1
    xvilvh.b         xr12,   xr13,   xr3   //vec2
    xvilvh.b         xr14,   xr15,   xr5   //vec5
    xvilvh.b         xr16,   xr17,   xr7   //vec7

    xmov             xr1,    xr8
    xmov             xr3,    xr8
    xmov             xr5,    xr8
    xmov             xr7,    xr8
    xvmaddwev.h.bu.b xr1,    xr0,   xr20
    xvmaddwev.h.bu.b xr3,    xr2,   xr20
    xvmaddwev.h.bu.b xr5,    xr4,   xr20
    xvmaddwev.h.bu.b xr7,    xr6,   xr20
    xvmaddwod.h.bu.b xr1,    xr0,   xr20
    xvmaddwod.h.bu.b xr3,    xr2,   xr20
    xvmaddwod.h.bu.b xr5,    xr4,   xr20
    xvmaddwod.h.bu.b xr7,    xr6,   xr20
    xmov             xr11,   xr8
    xmov             xr13,   xr8
    xmov             xr15,   xr8
    xmov             xr17,   xr8
    xvmaddwev.h.bu.b xr11,   xr10,  xr20
    xvmaddwev.h.bu.b xr13,   xr12,  xr20
    xvmaddwev.h.bu.b xr15,   xr14,  xr20
    xvmaddwev.h.bu.b xr17,   xr16,  xr20
    xvmaddwod.h.bu.b xr11,   xr10,  xr20
    xvmaddwod.h.bu.b xr13,   xr12,  xr20
    xvmaddwod.h.bu.b xr15,   xr14,  xr20
    xvmaddwod.h.bu.b xr17,   xr16,  xr20

    xvssran.bu.h     xr1,    xr1,   xr9   //vec0
    xvssran.bu.h     xr3,    xr3,   xr9   //vec2
    xvssran.bu.h     xr5,    xr5,   xr9   //vec4
    xvssran.bu.h     xr7,    xr7,   xr9   //vec6
    xvssran.bu.h     xr11,   xr11,  xr9   //vec1
    xvssran.bu.h     xr13,   xr13,  xr9   //vec3
    xvssran.bu.h     xr15,   xr15,  xr9   //vec5
    xvssran.bu.h     xr17,   xr17,  xr9   //vec7

    xvilvl.d         xr0,    xr11,   xr1
    xvilvl.d         xr2,    xr13,   xr3
    xvilvl.d         xr4,    xr15,   xr5
    xvilvl.d         xr6,    xr17,   xr7

    xvpermi.d        xr1,    xr0,    0x4E
    xvpermi.d        xr3,    xr2,    0x4E
    xvpermi.d        xr5,    xr4,    0x4E
    xvpermi.d        xr7,    xr6,    0x4E

    vst              vr0,    t5,     0
    vstx             vr1,    t5,     a2
    vstx             vr2,    t5,     t0
    vstx             vr3,    t5,     t1
    vst              vr4,    t7,     0
    vstx             vr5,    t7,     a2
    vstx             vr6,    t7,     t0
    vstx             vr7,    t7,     t1
.END_BIWEIGHT_PIXELS16_LASX:
endfunc

function ff_biweight_h264_pixels8_8_lsx
    slli.d           t0,     a2,     1
    slli.d           t2,     a2,     2
    add.d            t1,     t0,     a2
    addi.d           a7,     a7,     1
    ori              a7,     a7,     1
    sll.d            a7,     a7,     a4
    addi.d           a4,     a4,     1
    addi.d           t3,     zero,   8

    vreplgr2vr.b     vr0,    a6    //tmp0
    vreplgr2vr.b     vr1,    a5    //tmp1
    vreplgr2vr.h     vr8,    a7    //offset
    vreplgr2vr.h     vr9,    a4    //denom
    vilvh.b          vr20,   vr1,    vr0    //wgt

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f10,    a0,     0   //src10
    fldx.d           f11,    a0,     a2  //src11
    fldx.d           f12,    a0,     t0  //src12
    fldx.d           f13,    a0,     t1  //src13

    vilvl.d          vr4,    vr1,    vr0  //src0
    vilvl.d          vr5,    vr3,    vr2  //src2
    vilvl.d          vr6,    vr11,   vr10 //dst0
    vilvl.d          vr7,    vr13,   vr12 //dst2

    vilvl.b          vr0,    vr6,    vr4  //vec0.0
    vilvh.b          vr1,    vr6,    vr4  //vec0.1
    vilvl.b          vr2,    vr7,    vr5  //vec1.0
    vilvh.b          vr3,    vr7,    vr5  //vec1.1

    vmov             vr4,    vr8
    vmov             vr5,    vr8
    vmov             vr6,    vr8
    vmov             vr7,    vr8
    vmaddwev.h.bu.b  vr4,    vr0,    vr20
    vmaddwev.h.bu.b  vr5,    vr1,    vr20
    vmaddwev.h.bu.b  vr6,    vr2,    vr20
    vmaddwev.h.bu.b  vr7,    vr3,    vr20
    vmaddwod.h.bu.b  vr4,    vr0,    vr20
    vmaddwod.h.bu.b  vr5,    vr1,    vr20
    vmaddwod.h.bu.b  vr6,    vr2,    vr20
    vmaddwod.h.bu.b  vr7,    vr3,    vr20

    vssran.bu.h      vr0,    vr4,   vr9   //vec0
    vssran.bu.h      vr1,    vr5,   vr9   //vec0
    vssran.bu.h      vr2,    vr6,   vr9   //vec1
    vssran.bu.h      vr3,    vr7,   vr9   //vec1

    vilvl.d          vr4,    vr1,   vr0
    vilvl.d          vr6,    vr3,   vr2

    vbsrl.v          vr5,    vr4,   8
    vbsrl.v          vr7,    vr6,   8

    fst.d            f4,     a0,    0
    fstx.d           f5,     a0,    a2
    fstx.d           f6,     a0,    t0
    fstx.d           f7,     a0,    t1

    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS8
    addi.d           t3,     zero,   16
    add.d            a1,     a1,     t2
    add.d            a0,     a0,     t2

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f10,    a0,     0   //src10
    fldx.d           f11,    a0,     a2  //src11
    fldx.d           f12,    a0,     t0  //src12
    fldx.d           f13,    a0,     t1  //src13

    vilvl.d          vr4,    vr1,    vr0  //src0
    vilvl.d          vr5,    vr3,    vr2  //src2
    vilvl.d          vr6,    vr11,   vr10 //dst0
    vilvl.d          vr7,    vr13,   vr12 //dst2

    vilvl.b          vr0,    vr6,    vr4  //vec0.0
    vilvh.b          vr1,    vr6,    vr4  //vec0.1
    vilvl.b          vr2,    vr7,    vr5  //vec1.0
    vilvh.b          vr3,    vr7,    vr5  //vec1.1

    vmov             vr4,    vr8
    vmov             vr5,    vr8
    vmov             vr6,    vr8
    vmov             vr7,    vr8
    vmaddwev.h.bu.b  vr4,    vr0,    vr20
    vmaddwev.h.bu.b  vr5,    vr1,    vr20
    vmaddwev.h.bu.b  vr6,    vr2,    vr20
    vmaddwev.h.bu.b  vr7,    vr3,    vr20
    vmaddwod.h.bu.b  vr4,    vr0,    vr20
    vmaddwod.h.bu.b  vr5,    vr1,    vr20
    vmaddwod.h.bu.b  vr6,    vr2,    vr20
    vmaddwod.h.bu.b  vr7,    vr3,    vr20

    vssran.bu.h      vr0,    vr4,   vr9   //vec0
    vssran.bu.h      vr1,    vr5,   vr9   //vec0
    vssran.bu.h      vr2,    vr6,   vr9   //vec1
    vssran.bu.h      vr3,    vr7,   vr9   //vec1

    vilvl.d          vr4,    vr1,   vr0
    vilvl.d          vr6,    vr3,   vr2

    vbsrl.v          vr5,    vr4,   8
    vbsrl.v          vr7,    vr6,   8

    fst.d            f4,     a0,    0
    fstx.d           f5,     a0,    a2
    fstx.d           f6,     a0,    t0
    fstx.d           f7,     a0,    t1
    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS8
    add.d            a1,     a1,     t2
    add.d            a0,     a0,     t2
    add.d            t4,     a1,     t2
    add.d            t5,     a0,     t2

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f4,     t4,     0   //src4
    fldx.d           f5,     t4,     a2  //src5
    fldx.d           f6,     t4,     t0  //src6
    fldx.d           f7,     t4,     t1  //src7
    fld.d            f10,    a0,     0   //src10
    fldx.d           f11,    a0,     a2  //src11
    fldx.d           f12,    a0,     t0  //src12
    fldx.d           f13,    a0,     t1  //src13
    fld.d            f14,    t5,     0   //src10
    fldx.d           f15,    t5,     a2  //src11
    fldx.d           f16,    t5,     t0  //src12
    fldx.d           f17,    t5,     t1  //src13

    vilvl.d          vr0,    vr1,    vr0  //src0
    vilvl.d          vr2,    vr3,    vr2  //src2
    vilvl.d          vr4,    vr5,    vr4  //src4
    vilvl.d          vr6,    vr7,    vr6  //src6
    vilvl.d          vr10,   vr11,   vr10 //dst0
    vilvl.d          vr12,   vr13,   vr12 //dst2
    vilvl.d          vr14,   vr15,   vr14 //dst4
    vilvl.d          vr16,   vr17,   vr16 //dst6

    vilvl.b          vr1,    vr10,   vr0  //vec0.0
    vilvh.b          vr3,    vr10,   vr0  //vec0.1
    vilvl.b          vr5,    vr12,   vr2  //vec2.0
    vilvh.b          vr7,    vr12,   vr2  //vec2.1
    vilvl.b          vr11,   vr14,   vr4  //vec4.0
    vilvh.b          vr13,   vr14,   vr4  //vec4.1
    vilvl.b          vr15,   vr16,   vr6  //vec6.0
    vilvh.b          vr17,   vr16,   vr6  //vec6.1

    vmov             vr0,    vr8
    vmov             vr2,    vr8
    vmov             vr4,    vr8
    vmov             vr6,    vr8
    vmaddwev.h.bu.b  vr0,    vr1,    vr20
    vmaddwev.h.bu.b  vr2,    vr3,    vr20
    vmaddwev.h.bu.b  vr4,    vr5,    vr20
    vmaddwev.h.bu.b  vr6,    vr7,    vr20
    vmaddwod.h.bu.b  vr0,    vr1,    vr20
    vmaddwod.h.bu.b  vr2,    vr3,    vr20
    vmaddwod.h.bu.b  vr4,    vr5,    vr20
    vmaddwod.h.bu.b  vr6,    vr7,    vr20

    vmov             vr10,   vr8
    vmov             vr12,   vr8
    vmov             vr14,   vr8
    vmov             vr16,   vr8

    vmaddwev.h.bu.b  vr10,   vr11,   vr20
    vmaddwev.h.bu.b  vr12,   vr13,   vr20
    vmaddwev.h.bu.b  vr14,   vr15,   vr20
    vmaddwev.h.bu.b  vr16,   vr17,   vr20
    vmaddwod.h.bu.b  vr10,   vr11,   vr20
    vmaddwod.h.bu.b  vr12,   vr13,   vr20
    vmaddwod.h.bu.b  vr14,   vr15,   vr20
    vmaddwod.h.bu.b  vr16,   vr17,   vr20

    vssran.bu.h      vr1,    vr0,   vr9   //vec0
    vssran.bu.h      vr3,    vr2,   vr9   //vec0
    vssran.bu.h      vr5,    vr4,   vr9   //vec2
    vssran.bu.h      vr7,    vr6,   vr9   //vec2

    vssran.bu.h      vr11,   vr10,  vr9   //vec4
    vssran.bu.h      vr13,   vr12,  vr9   //vec4
    vssran.bu.h      vr15,   vr14,  vr9   //vec6
    vssran.bu.h      vr17,   vr16,  vr9   //vec6

    vilvl.d          vr0,    vr3,   vr1
    vilvl.d          vr2,    vr7,   vr5
    vilvl.d          vr10,   vr13,  vr11
    vilvl.d          vr12,   vr17,  vr15

    vbsrl.v          vr1,    vr0,   8
    vbsrl.v          vr3,    vr2,   8
    vbsrl.v          vr11,   vr10,  8
    vbsrl.v          vr13,   vr12,  8

    fst.d            f0,     a0,    0
    fstx.d           f1,     a0,    a2
    fstx.d           f2,     a0,    t0
    fstx.d           f3,     a0,    t1
    fst.d            f10,    t5,    0
    fstx.d           f11,    t5,    a2
    fstx.d           f12,    t5,    t0
    fstx.d           f13,    t5,    t1
.END_BIWEIGHT_H264_PIXELS8:
endfunc

function ff_biweight_h264_pixels8_8_lasx
    slli.d           t0,     a2,     1
    slli.d           t2,     a2,     2
    add.d            t1,     t0,     a2
    addi.d           a7,     a7,     1
    ori              a7,     a7,     1
    sll.d            a7,     a7,     a4
    addi.d           a4,     a4,     1
    addi.d           t3,     zero,   8

    xvreplgr2vr.b    xr0,    a6    //tmp0
    xvreplgr2vr.b    xr1,    a5    //tmp1
    xvreplgr2vr.h    xr8,    a7    //offset
    xvreplgr2vr.h    xr9,    a4    //denom
    xvilvh.b         xr20,   xr1,    xr0    //wgt

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f10,    a0,     0   //src10
    fldx.d           f11,    a0,     a2  //src11
    fldx.d           f12,    a0,     t0  //src12
    fldx.d           f13,    a0,     t1  //src13

    vilvl.d          vr4,    vr1,    vr0  //src0
    vilvl.d          vr5,    vr3,    vr2  //src2
    vilvl.d          vr6,    vr11,   vr10 //dst0
    vilvl.d          vr7,    vr13,   vr12 //dst2

    xvpermi.q        xr5,    xr4,    0x20
    xvpermi.q        xr7,    xr6,    0x20

    xvilvl.b         xr0,    xr7,    xr5
    xvilvh.b         xr1,    xr7,    xr5

    xmov             xr2,    xr8
    xmov             xr3,    xr8
    xvmaddwev.h.bu.b xr2,    xr0,    xr20
    xvmaddwev.h.bu.b xr3,    xr1,    xr20
    xvmaddwod.h.bu.b xr2,    xr0,    xr20
    xvmaddwod.h.bu.b xr3,    xr1,    xr20

    xvssran.bu.h     xr4,    xr2,   xr9   //vec0
    xvssran.bu.h     xr5,    xr3,   xr9   //vec2

    xvilvl.d         xr0,    xr5,   xr4
    xvpermi.d        xr2,    xr0,   0x4E
    vbsrl.v          vr1,    vr0,   8
    vbsrl.v          vr3,    vr2,   8

    fst.d            f0,     a0,    0
    fstx.d           f1,     a0,    a2
    fstx.d           f2,     a0,    t0
    fstx.d           f3,     a0,    t1

    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS8_LASX
    addi.d           t3,     zero,   16
    add.d            a1,     a1,     t2
    add.d            a0,     a0,     t2

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f10,    a0,     0   //src10
    fldx.d           f11,    a0,     a2  //src11
    fldx.d           f12,    a0,     t0  //src12
    fldx.d           f13,    a0,     t1  //src13

    vilvl.d          vr4,    vr1,    vr0  //src0
    vilvl.d          vr5,    vr3,    vr2  //src2
    vilvl.d          vr6,    vr11,   vr10 //dst0
    vilvl.d          vr7,    vr13,   vr12 //dst2

    xvpermi.q        xr5,    xr4,    0x20
    xvpermi.q        xr7,    xr6,    0x20

    xvilvl.b         xr0,    xr7,    xr5
    xvilvh.b         xr1,    xr7,    xr5

    xmov             xr2,    xr8
    xmov             xr3,    xr8
    xvmaddwev.h.bu.b xr2,    xr0,    xr20
    xvmaddwev.h.bu.b xr3,    xr1,    xr20
    xvmaddwod.h.bu.b xr2,    xr0,    xr20
    xvmaddwod.h.bu.b xr3,    xr1,    xr20

    xvssran.bu.h     xr4,    xr2,   xr9   //vec0
    xvssran.bu.h     xr5,    xr3,   xr9   //vec2

    xvilvl.d         xr0,    xr5,   xr4
    xvpermi.d        xr2,    xr0,   0x4E
    vbsrl.v          vr1,    vr0,   8
    vbsrl.v          vr3,    vr2,   8

    fst.d            f0,     a0,    0
    fstx.d           f1,     a0,    a2
    fstx.d           f2,     a0,    t0
    fstx.d           f3,     a0,    t1
    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS8_LASX
    add.d            a1,     a1,     t2
    add.d            a0,     a0,     t2
    add.d            t4,     a1,     t2
    add.d            t5,     a0,     t2

    fld.d            f0,     a1,     0   //src0
    fldx.d           f1,     a1,     a2  //src1
    fldx.d           f2,     a1,     t0  //src2
    fldx.d           f3,     a1,     t1  //src3
    fld.d            f4,     t4,     0   //src4
    fldx.d           f5,     t4,     a2  //src5
    fldx.d           f6,     t4,     t0  //src6
    fldx.d           f7,     t4,     t1  //src7
    fld.d            f10,    a0,     0   //dst0
    fldx.d           f11,    a0,     a2  //dst1
    fldx.d           f12,    a0,     t0  //dst2
    fldx.d           f13,    a0,     t1  //dst3
    fld.d            f14,    t5,     0   //dst4
    fldx.d           f15,    t5,     a2  //dst5
    fldx.d           f16,    t5,     t0  //dst6
    fldx.d           f17,    t5,     t1  //dst7

    vilvl.d          vr0,    vr1,    vr0  //src0
    vilvl.d          vr2,    vr3,    vr2  //src2
    vilvl.d          vr4,    vr5,    vr4  //src4
    vilvl.d          vr6,    vr7,    vr6  //src6
    vilvl.d          vr10,   vr11,   vr10 //dst0
    vilvl.d          vr12,   vr13,   vr12 //dst2
    vilvl.d          vr14,   vr15,   vr14 //dst4
    vilvl.d          vr16,   vr17,   vr16 //dst6

    xvpermi.q        xr2,    xr0,    0x20
    xvpermi.q        xr6,    xr4,    0x20
    xvpermi.q        xr12,   xr10,   0x20
    xvpermi.q        xr16,   xr14,   0x20

    xvilvl.b         xr0,    xr12,   xr2
    xvilvh.b         xr1,    xr12,   xr2
    xvilvl.b         xr10,   xr16,   xr6
    xvilvh.b         xr11,   xr16,   xr6

    xmov             xr2,    xr8
    xmov             xr3,    xr8
    xmov             xr12,   xr8
    xmov             xr13,   xr8
    xvmaddwev.h.bu.b xr2,    xr0,    xr20
    xvmaddwev.h.bu.b xr3,    xr1,    xr20
    xvmaddwev.h.bu.b xr12,   xr10,   xr20
    xvmaddwev.h.bu.b xr13,   xr11,   xr20
    xvmaddwod.h.bu.b xr2,    xr0,    xr20
    xvmaddwod.h.bu.b xr3,    xr1,    xr20
    xvmaddwod.h.bu.b xr12,   xr10,   xr20
    xvmaddwod.h.bu.b xr13,   xr11,   xr20

    xvssran.bu.h     xr4,    xr2,   xr9   //vec0
    xvssran.bu.h     xr5,    xr3,   xr9   //vec2
    xvssran.bu.h     xr14,   xr12,  xr9   //vec0
    xvssran.bu.h     xr15,   xr13,  xr9   //vec2

    xvilvl.d         xr0,    xr5,   xr4
    xvilvl.d         xr10,   xr15,  xr14
    xvpermi.d        xr2,    xr0,   0x4E
    xvpermi.d        xr12,   xr10,  0x4E
    vbsrl.v          vr1,    vr0,   8
    vbsrl.v          vr3,    vr2,   8
    vbsrl.v          vr11,   vr10,  8
    vbsrl.v          vr13,   vr12,  8

    fst.d            f0,     a0,    0
    fstx.d           f1,     a0,    a2
    fstx.d           f2,     a0,    t0
    fstx.d           f3,     a0,    t1
    fst.d            f10,    t5,    0
    fstx.d           f11,    t5,    a2
    fstx.d           f12,    t5,    t0
    fstx.d           f13,    t5,    t1
.END_BIWEIGHT_H264_PIXELS8_LASX:
endfunc

//LSX optimization is enough for this function.
function ff_biweight_h264_pixels4_8_lsx
    slli.d           t0,     a2,     1
    slli.d           t2,     a2,     2
    add.d            t1,     t0,     a2
    addi.d           a7,     a7,     1
    ori              a7,     a7,     1
    sll.d            a7,     a7,     a4
    addi.d           a4,     a4,     1
    addi.d           t3,     zero,   4

    vreplgr2vr.b     vr0,    a6    //tmp0
    vreplgr2vr.b     vr1,    a5    //tmp1
    vreplgr2vr.h     vr8,    a7    //offset
    vreplgr2vr.h     vr9,    a4    //denom
    vilvh.b          vr20,   vr1,    vr0    //wgt

    fld.s            f0,     a1,     0
    fldx.s           f1,     a1,     a2
    fld.s            f10,    a0,     0
    fldx.s           f11,    a0,     a2
    vilvl.w          vr2,    vr1,    vr0
    vilvl.w          vr12,   vr11,   vr10
    vilvl.b          vr0,    vr12,   vr2

    vmov             vr1,    vr8
    vmaddwev.h.bu.b  vr1,    vr0,    vr20
    vmaddwod.h.bu.b  vr1,    vr0,    vr20

    vssran.bu.h      vr1,    vr1,    vr9   //vec0
    vbsrl.v          vr2,    vr1,    4
    fst.s            f1,     a0,     0
    fstx.s           f2,     a0,     a2

    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS4
    addi.d           t3,     zero,   8
    fldx.s           f0,     a1,     t0
    fldx.s           f1,     a1,     t1
    fldx.s           f10,    a0,     t0
    fldx.s           f11,    a0,     t1
    vilvl.w          vr2,    vr1,    vr0
    vilvl.w          vr12,   vr11,   vr10
    vilvl.b          vr0,    vr12,   vr2

    vmov             vr1,    vr8
    vmaddwev.h.bu.b  vr1,    vr0,    vr20
    vmaddwod.h.bu.b  vr1,    vr0,    vr20

    vssran.bu.h      vr1,    vr1,    vr9   //vec0
    vbsrl.v          vr2,    vr1,    4
    fstx.s           f1,     a0,     t0
    fstx.s           f2,     a0,     t1

    blt              a3,     t3,    .END_BIWEIGHT_H264_PIXELS4
    add.d            a1,     a1,     t2
    add.d            a0,     a0,     t2
    fld.s            f0,     a1,     0
    fldx.s           f1,     a1,     a2
    fldx.s           f2,     a1,     t0
    fldx.s           f3,     a1,     t1
    fld.s            f10,    a0,     0
    fldx.s           f11,    a0,     a2
    fldx.s           f12,    a0,     t0
    fldx.s           f13,    a0,     t1
    vilvl.w          vr4,    vr1,    vr0
    vilvl.w          vr5,    vr3,    vr2
    vilvl.w          vr14,   vr11,   vr10
    vilvl.w          vr15,   vr13,   vr12

    vilvl.b          vr0,    vr14,   vr4
    vilvl.b          vr10,   vr15,   vr5

    vmov             vr1,    vr8
    vmov             vr11,   vr8
    vmaddwev.h.bu.b  vr1,    vr0,    vr20
    vmaddwev.h.bu.b  vr11,   vr10,   vr20
    vmaddwod.h.bu.b  vr1,    vr0,    vr20
    vmaddwod.h.bu.b  vr11,   vr10,   vr20

    vssran.bu.h      vr0,    vr1,    vr9   //vec0
    vssran.bu.h      vr10,   vr11,   vr9   //vec0
    vbsrl.v          vr2,    vr0,    4
    vbsrl.v          vr12,   vr10,   4

    fst.s            f0,     a0,     0
    fstx.s           f2,     a0,     a2
    fstx.s           f10,    a0,     t0
    fstx.s           f12,    a0,     t1
.END_BIWEIGHT_H264_PIXELS4:
endfunc
