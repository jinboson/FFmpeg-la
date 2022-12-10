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
