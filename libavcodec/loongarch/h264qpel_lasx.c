/*
 * Loongson LASX optimized h264qpel
 *
 * Copyright (c) 2020 Loongson Technology Corporation Limited
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

#include "h264qpel_lasx.h"
#include "libavutil/loongarch/generic_macros_lasx.h"
#include "libavutil/attributes.h"

static const uint8_t luma_mask_arr[16 * 6] __attribute__((aligned(0x40))) = {
    /* 8 width cases */
    0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 5, 10, 6, 11, 7, 12,
    0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 5, 10, 6, 11, 7, 12,
    1, 4, 2, 5, 3, 6, 4, 7, 5, 8, 6, 9, 7, 10, 8, 11,
    1, 4, 2, 5, 3, 6, 4, 7, 5, 8, 6, 9, 7, 10, 8, 11,
    2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10,
    2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10
};

#define AVC_HORZ_FILTER_SH(in0, in1, mask0, mask1, mask2)  \
( {                                                        \
    __m256i out0_m;                                        \
    __m256i tmp0_m;                                        \
                                                           \
    tmp0_m = __lasx_xvshuf_b(in1, in0, mask0);             \
    out0_m = __lasx_xvhaddw_h_b(tmp0_m, tmp0_m);           \
    tmp0_m = __lasx_xvshuf_b(in1, in0, mask1);             \
    out0_m = __lasx_xvdp2add_h_b(out0_m, minus5b, tmp0_m); \
    tmp0_m = __lasx_xvshuf_b(in1, in0, mask2);             \
    out0_m = __lasx_xvdp2add_h_b(out0_m, plus20b, tmp0_m); \
                                                           \
    out0_m;                                                \
} )

#define AVC_DOT_SH3_SH(in0, in1, in2, coeff0, coeff1, coeff2)  \
( {                                                            \
    __m256i out0_m;                                            \
                                                               \
    out0_m = __lasx_xvdp2_h_b(in0, coeff0);                    \
    out0_m = __lasx_xvdp2add_h_b(out0_m, in1, coeff1);         \
    out0_m = __lasx_xvdp2add_h_b(out0_m, in2, coeff2);         \
                                                               \
    out0_m;                                                    \
} )

static av_always_inline
void avc_luma_hv_qrt_and_aver_dst_16x16_lasx(const uint8_t *src_x,
                                             const uint8_t *src_y,
                                             uint8_t *dst, int32_t stride)
{
    const int16_t filt_const0 = 0xfb01;
    const int16_t filt_const1 = 0x1414;
    const int16_t filt_const2 = 0x1fb;
    uint32_t loop_cnt;
    int32_t stride_x2 = stride << 1;
    int32_t stride_x3 = stride_x2 + stride;
    int32_t stride_x4 = stride << 2;
    __m256i tmp0, tmp1;
    __m256i src_hz0, src_hz1, src_hz2, src_hz3, mask0, mask1, mask2;
    __m256i src_vt0, src_vt1, src_vt2, src_vt3, src_vt4, src_vt5, src_vt6;
    __m256i src_vt7, src_vt8;
    __m256i src_vt10_h, src_vt21_h, src_vt32_h, src_vt43_h, src_vt54_h;
    __m256i src_vt65_h, src_vt76_h, src_vt87_h, filt0, filt1, filt2;
    __m256i hz_out0, hz_out1, hz_out2, hz_out3, vt_out0, vt_out1, vt_out2;
    __m256i vt_out3, out0, out1, out2, out3;
    __m256i minus5b = __lasx_xvldi(0xFB);
    __m256i plus20b = __lasx_xvldi(20);

    filt0 = __lasx_xvldrepl_h(&filt_const0, 0);
    filt1 = __lasx_xvldrepl_h(&filt_const1, 0);
    filt2 = __lasx_xvldrepl_h(&filt_const2, 0);

    mask0 = LASX_LD(luma_mask_arr);
    LASX_LD_2(luma_mask_arr + 32, 32, mask1, mask2);
    src_vt0 = LASX_LD(src_y);
    LASX_LD_4(src_y + stride, stride, src_vt1, src_vt2, src_vt3, src_vt4);
    src_y += stride_x4 + stride;

    LASX_XORI_B_128(src_vt0);
    LASX_XORI_B_4_128(src_vt1, src_vt2, src_vt3, src_vt4);

    for (loop_cnt = 4; loop_cnt--;) {
        LASX_LD_4(src_x, stride, src_hz0, src_hz1, src_hz2, src_hz3);
        src_x  += stride_x4;
        src_hz0 = __lasx_xvpermi_d(src_hz0, 0x94);
        src_hz1 = __lasx_xvpermi_d(src_hz1, 0x94);
        src_hz2 = __lasx_xvpermi_d(src_hz2, 0x94);
        src_hz3 = __lasx_xvpermi_d(src_hz3, 0x94);
        LASX_XORI_B_4_128(src_hz0, src_hz1, src_hz2, src_hz3);

        hz_out0 = AVC_HORZ_FILTER_SH(src_hz0, src_hz0, mask0, mask1, mask2);
        hz_out1 = AVC_HORZ_FILTER_SH(src_hz1, src_hz1, mask0, mask1, mask2);
        hz_out2 = AVC_HORZ_FILTER_SH(src_hz2, src_hz2, mask0, mask1, mask2);
        hz_out3 = AVC_HORZ_FILTER_SH(src_hz3, src_hz3, mask0, mask1, mask2);
        LASX_SRARI_H_4(hz_out0, hz_out1, hz_out2, hz_out3,
                      hz_out0, hz_out1, hz_out2, hz_out3, 5);
        LASX_SAT_H_4(hz_out0, hz_out1, hz_out2, hz_out3,
                      hz_out0, hz_out1, hz_out2, hz_out3, 7);

        LASX_LD_4(src_y, stride, src_vt5, src_vt6, src_vt7, src_vt8);
        src_y += stride_x4;

        LASX_XORI_B_4_128(src_vt5, src_vt6, src_vt7, src_vt8);
        LASX_ILVL_B_4(src_vt1, src_vt0, src_vt2, src_vt1, src_vt3, src_vt2,
                      src_vt4, src_vt3, src_vt10_h, src_vt21_h, src_vt32_h,
                      src_vt43_h);
        LASX_ILVL_B_4(src_vt5, src_vt4, src_vt6, src_vt5, src_vt7, src_vt6,
                      src_vt8, src_vt7, src_vt54_h, src_vt65_h, src_vt76_h,
                      src_vt87_h);
        vt_out0 = AVC_DOT_SH3_SH(src_vt10_h, src_vt32_h, src_vt54_h, filt0,
                                 filt1, filt2);
        vt_out1 = AVC_DOT_SH3_SH(src_vt21_h, src_vt43_h, src_vt65_h, filt0,
                                 filt1, filt2);
        vt_out2 = AVC_DOT_SH3_SH(src_vt32_h, src_vt54_h, src_vt76_h, filt0,
                                 filt1, filt2);
        vt_out3 = AVC_DOT_SH3_SH(src_vt43_h, src_vt65_h, src_vt87_h, filt0,
                                 filt1, filt2);
        LASX_SRARI_H_4(vt_out0, vt_out1, vt_out2, vt_out3,
                       vt_out0, vt_out1, vt_out2, vt_out3, 5);
        LASX_SAT_H_4(vt_out0, vt_out1, vt_out2, vt_out3,
                      vt_out0, vt_out1, vt_out2, vt_out3, 7);

        LASX_ADD_H_4(hz_out0, vt_out0, hz_out1, vt_out1, hz_out2, vt_out2,
                     hz_out3, vt_out3, out0, out1, out2, out3);
        LASX_SRARI_H_4(out0, out1, out2, out3, out0, out1, out2, out3, 1);
        LASX_SAT_H_4(out0, out1, out2, out3, out0, out1, out2, out3, 7);

        LASX_PICKEV_XORI128_B(out0, out1, tmp0);
        LASX_PICKEV_XORI128_B(out2, out3, tmp1);

        LASX_LD_4(dst, stride, out0, out1, out2, out3);
        LASX_ILVL_D_2(out1, out0, out3, out2, out0, out1);
        tmp0 = __lasx_xvavgr_bu(out0, tmp0);
        tmp1 = __lasx_xvavgr_bu(out1, tmp1);

        *(int64_t*)dst               = __lasx_xvpickve2gr_d(tmp0, 0);
        *(int64_t*)(dst + stride)    = __lasx_xvpickve2gr_d(tmp0, 1);
        *(int64_t*)(dst + stride_x2) = __lasx_xvpickve2gr_d(tmp1, 0);
        *(int64_t*)(dst + stride_x3) = __lasx_xvpickve2gr_d(tmp1, 1);

        *(int64_t*)(dst + 8)             = __lasx_xvpickve2gr_d(tmp0, 2);
        *(int64_t*)(dst + 8 + stride)    = __lasx_xvpickve2gr_d(tmp0, 3);
        *(int64_t*)(dst + 8 + stride_x2) = __lasx_xvpickve2gr_d(tmp1, 2);
        *(int64_t*)(dst + 8 + stride_x3) = __lasx_xvpickve2gr_d(tmp1, 3);

        dst    += stride_x4;
        src_vt0 = src_vt4;
        src_vt1 = src_vt5;
        src_vt2 = src_vt6;
        src_vt3 = src_vt7;
        src_vt4 = src_vt8;
    }
}

static av_always_inline void
avc_luma_hv_qrt_16x16_lasx(const uint8_t *src_x, const uint8_t *src_y, uint8_t *dst,
                           int32_t stride)
{
    const int16_t filt_const0 = 0xfb01;
    const int16_t filt_const1 = 0x1414;
    const int16_t filt_const2 = 0x1fb;
    uint32_t loop_cnt;
    int32_t stride_x2 = stride << 1;
    int32_t stride_x3 = stride_x2 + stride;
    int32_t stride_x4 = stride << 2;
    __m256i tmp0, tmp1;
    __m256i src_hz0, src_hz1, src_hz2, src_hz3, mask0, mask1, mask2;
    __m256i src_vt0, src_vt1, src_vt2, src_vt3, src_vt4, src_vt5, src_vt6;
    __m256i src_vt7, src_vt8;
    __m256i src_vt10_h, src_vt21_h, src_vt32_h, src_vt43_h, src_vt54_h;
    __m256i src_vt65_h, src_vt76_h, src_vt87_h, filt0, filt1, filt2;
    __m256i hz_out0, hz_out1, hz_out2, hz_out3, vt_out0, vt_out1, vt_out2;
    __m256i vt_out3, out0, out1, out2, out3;
    __m256i minus5b = __lasx_xvldi(0xFB);
    __m256i plus20b = __lasx_xvldi(20);

    filt0 = __lasx_xvldrepl_h(&filt_const0, 0);
    filt1 = __lasx_xvldrepl_h(&filt_const1, 0);
    filt2 = __lasx_xvldrepl_h(&filt_const2, 0);

    mask0 = LASX_LD(luma_mask_arr);
    LASX_LD_2(luma_mask_arr + 32, 32, mask1, mask2);
    src_vt0 = LASX_LD(src_y);
    LASX_LD_4(src_y + stride, stride, src_vt1, src_vt2, src_vt3, src_vt4);
    src_y += stride_x4 + stride;

    LASX_XORI_B_128(src_vt0);
    LASX_XORI_B_4_128(src_vt1, src_vt2, src_vt3, src_vt4);

    for (loop_cnt = 4; loop_cnt--;) {
        LASX_LD_4(src_x, stride, src_hz0, src_hz1, src_hz2, src_hz3);
        src_x  += stride_x4;
        src_hz0 = __lasx_xvpermi_d(src_hz0, 0x94);
        src_hz1 = __lasx_xvpermi_d(src_hz1, 0x94);
        src_hz2 = __lasx_xvpermi_d(src_hz2, 0x94);
        src_hz3 = __lasx_xvpermi_d(src_hz3, 0x94);
        LASX_XORI_B_4_128(src_hz0, src_hz1, src_hz2, src_hz3);

        hz_out0 = AVC_HORZ_FILTER_SH(src_hz0, src_hz0, mask0, mask1, mask2);
        hz_out1 = AVC_HORZ_FILTER_SH(src_hz1, src_hz1, mask0, mask1, mask2);
        hz_out2 = AVC_HORZ_FILTER_SH(src_hz2, src_hz2, mask0, mask1, mask2);
        hz_out3 = AVC_HORZ_FILTER_SH(src_hz3, src_hz3, mask0, mask1, mask2);
        LASX_SRARI_H_4(hz_out0, hz_out1, hz_out2, hz_out3,
                       hz_out0, hz_out1, hz_out2, hz_out3, 5);
        LASX_SAT_H_4(hz_out0, hz_out1, hz_out2, hz_out3,
                     hz_out0, hz_out1, hz_out2, hz_out3, 7);

        LASX_LD_4(src_y, stride, src_vt5, src_vt6, src_vt7, src_vt8);
        src_y += stride_x4;

        LASX_XORI_B_4_128(src_vt5, src_vt6, src_vt7, src_vt8);
        LASX_ILVL_B_4(src_vt1, src_vt0, src_vt2, src_vt1, src_vt3, src_vt2,
                      src_vt4, src_vt3, src_vt10_h, src_vt21_h, src_vt32_h,
                      src_vt43_h);
        LASX_ILVL_B_4(src_vt5, src_vt4, src_vt6, src_vt5, src_vt7, src_vt6,
                      src_vt8, src_vt7, src_vt54_h, src_vt65_h, src_vt76_h,
                      src_vt87_h);
        vt_out0 = AVC_DOT_SH3_SH(src_vt10_h, src_vt32_h, src_vt54_h, filt0,
                                 filt1, filt2);
        vt_out1 = AVC_DOT_SH3_SH(src_vt21_h, src_vt43_h, src_vt65_h, filt0,
                                 filt1, filt2);
        vt_out2 = AVC_DOT_SH3_SH(src_vt32_h, src_vt54_h, src_vt76_h, filt0,
                                 filt1, filt2);
        vt_out3 = AVC_DOT_SH3_SH(src_vt43_h, src_vt65_h, src_vt87_h, filt0,
                                 filt1, filt2);
        LASX_SRARI_H_4(vt_out0, vt_out1, vt_out2, vt_out3,
                       vt_out0, vt_out1, vt_out2, vt_out3, 5);
        LASX_SAT_H_4(vt_out0, vt_out1, vt_out2, vt_out3,
                      vt_out0, vt_out1, vt_out2, vt_out3, 7);

        LASX_ADD_H_4(hz_out0, vt_out0, hz_out1, vt_out1, hz_out2, vt_out2,
                     hz_out3, vt_out3, out0, out1, out2, out3);
        LASX_SRARI_H_4(out0, out1, out2, out3, out0, out1, out2, out3, 1);
        LASX_SAT_H_4(out0, out1, out2, out3, out0, out1, out2, out3, 7);

        LASX_PICKEV_XORI128_B(out0, out1, tmp0);
        LASX_PICKEV_XORI128_B(out2, out3, tmp1);
        *(int64_t*)dst               = __lasx_xvpickve2gr_d(tmp0, 0);
        *(int64_t*)(dst + stride)    = __lasx_xvpickve2gr_d(tmp0, 1);
        *(int64_t*)(dst + stride_x2) = __lasx_xvpickve2gr_d(tmp1, 0);
        *(int64_t*)(dst + stride_x3) = __lasx_xvpickve2gr_d(tmp1, 1);

        *(int64_t*)(dst + 8)             = __lasx_xvpickve2gr_d(tmp0, 2);
        *(int64_t*)(dst + 8 + stride)    = __lasx_xvpickve2gr_d(tmp0, 3);
        *(int64_t*)(dst + 8 + stride_x2) = __lasx_xvpickve2gr_d(tmp1, 2);
        *(int64_t*)(dst + 8 + stride_x3) = __lasx_xvpickve2gr_d(tmp1, 3);

        dst    += stride_x4;
        src_vt0 = src_vt4;
        src_vt1 = src_vt5;
        src_vt2 = src_vt6;
        src_vt3 = src_vt7;
        src_vt4 = src_vt8;
    }
}

/* put_pixels8_8_inline_asm: dst = src */
static av_always_inline void
put_pixels8_8_inline_asm(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    double tmp[8];
    __asm__ volatile (
        "ld.d       %[tmp0],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp1],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp2],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp3],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp4],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp5],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp6],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp7],    %[src],    0x0         \n\t"

        "st.d       %[tmp0],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp1],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp2],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp3],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp4],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp5],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp6],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp7],    %[dst],    0x0         \n\t"
        : [tmp0]"=&r"(tmp[0]),        [tmp1]"=&r"(tmp[1]),
          [tmp2]"=&r"(tmp[2]),        [tmp3]"=&r"(tmp[3]),
          [tmp4]"=&r"(tmp[4]),        [tmp5]"=&r"(tmp[5]),
          [tmp6]"=&r"(tmp[6]),        [tmp7]"=&r"(tmp[7]),
          [dst]"+&r"(dst),            [src]"+&r"(src)
        : [stride]"r"(stride)
        : "memory"
    );
}

/* avg_pixels8_8_lsx   : dst = avg(src, dst)
 * put_pixels8_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels8_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
avg_pixels8_8_lsx(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    uint8_t *tmp = dst;
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr1,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr2,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr3,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr4,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr5,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr6,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr7,    %[src],  0          \n\t"

        "vld     $vr8,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr9,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr10,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr11,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr12,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr13,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr14,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr15,   %[tmp],  0          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0       \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1       \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2       \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3       \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4       \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5       \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6       \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7       \n\t"

        "vstelm.d  $vr0,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr1,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr2,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr3,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr4,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr5,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr6,  %[dst],  0,  0      \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vstelm.d  $vr7,  %[dst],  0,  0      \n\t"
        : [dst]"+&r"(dst), [tmp]"+&r"(tmp), [src]"+&r"(src)
        : [stride]"r"(stride)
        : "memory"
    );
}

/* avg_pixels8_8_lsx   : dst = avg(src, dst)
 * put_pixels8_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels8_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
put_pixels8_l2_8_lsx(uint8_t *dst, const uint8_t *src, const uint8_t *half,
                     ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr1,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr2,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr3,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr4,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr5,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr6,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr7,    %[src],   0             \n\t"

        "vld     $vr8,    %[half],  0x00          \n\t"
        "vld     $vr9,    %[half],  0x08          \n\t"
        "vld     $vr10,   %[half],  0x10          \n\t"
        "vld     $vr11,   %[half],  0x18          \n\t"
        "vld     $vr12,   %[half],  0x20          \n\t"
        "vld     $vr13,   %[half],  0x28          \n\t"
        "vld     $vr14,   %[half],  0x30          \n\t"
        "vld     $vr15,   %[half],  0x38          \n\t"

        "vavgr.bu $vr0,   $vr8,     $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,     $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,    $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,    $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,    $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,    $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,    $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,    $vr7          \n\t"

        "vstelm.d  $vr0,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr1,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr2,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr3,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr4,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr5,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr6,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr7,  %[dst],   0,  0         \n\t"
        : [dst]"+&r"(dst), [half]"+&r"(half), [src]"+&r"(src)
        : [srcStride]"r"(srcStride), [dstStride]"r"(dstStride)
        : "memory"
    );
}

/* avg_pixels8_8_lsx   : dst = avg(src, dst)
 * put_pixels8_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels8_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
avg_pixels8_l2_8_lsx(uint8_t *dst, const uint8_t *src, const uint8_t *half,
                     ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    uint8_t *tmp = dst;
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr1,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr2,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr3,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr4,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr5,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr6,    %[src],   0             \n\t"
        "add.d   %[src],  %[src],   %[srcStride]  \n\t"
        "vld     $vr7,    %[src],   0             \n\t"

        "vld     $vr8,    %[half],  0x00          \n\t"
        "vld     $vr9,    %[half],  0x08          \n\t"
        "vld     $vr10,   %[half],  0x10          \n\t"
        "vld     $vr11,   %[half],  0x18          \n\t"
        "vld     $vr12,   %[half],  0x20          \n\t"
        "vld     $vr13,   %[half],  0x28          \n\t"
        "vld     $vr14,   %[half],  0x30          \n\t"
        "vld     $vr15,   %[half],  0x38          \n\t"

        "vavgr.bu $vr0,   $vr8,     $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,     $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,    $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,    $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,    $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,    $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,    $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,    $vr7          \n\t"

        "vld     $vr8,    %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr9,    %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr10,   %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr11,   %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr12,   %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr13,   %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr14,   %[tmp],   0             \n\t"
        "add.d   %[tmp],  %[tmp],   %[dstStride]  \n\t"
        "vld     $vr15,   %[tmp],   0             \n\t"

        "vavgr.bu $vr0,   $vr8,     $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,     $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,    $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,    $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,    $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,    $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,    $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,    $vr7          \n\t"

        "vstelm.d  $vr0,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr1,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr2,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr3,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr4,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr5,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr6,  %[dst],   0,  0         \n\t"
        "add.d   %[dst],  %[dst],   %[dstStride]  \n\t"
        "vstelm.d  $vr7,  %[dst],   0,  0         \n\t"
        : [dst]"+&r"(dst), [tmp]"+&r"(tmp), [half]"+&r"(half), [src]"+&r"(src)
        : [dstStride]"r"(dstStride), [srcStride]"r"(srcStride)
        : "memory"
    );
}

/* put_pixels16_8_lsx: dst = src */
static av_always_inline void
put_pixels16_8_lsx(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    __asm__ volatile (
        "vld     $vr0,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr1,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr2,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr3,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr4,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr5,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr6,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr7,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"

        "vst     $vr0,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr1,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr2,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr3,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr4,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr5,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr6,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr7,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"

        "vld     $vr0,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr1,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr2,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr3,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr4,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr5,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr6,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr7,    %[src],  0          \n\t"

        "vst     $vr0,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr1,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr2,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr3,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr4,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr5,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr6,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr7,    %[dst],  0          \n\t"
        : [dst]"+&r"(dst),            [src]"+&r"(src)
        : [stride]"r"(stride)
        : "memory"
    );
}

/* avg_pixels16_8_lsx    : dst = avg(src, dst)
 * put_pixels16_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels16_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
avg_pixels16_8_lsx(uint8_t *dst, const uint8_t *src, ptrdiff_t stride)
{
    uint8_t *tmp = dst;
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr1,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr2,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr3,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr4,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr5,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr6,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr7,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"

        "vld     $vr8,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr9,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr10,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr11,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr12,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr13,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr14,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr15,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0       \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1       \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2       \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3       \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4       \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5       \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6       \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7       \n\t"

        "vst     $vr0,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr1,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr2,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr3,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr4,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr5,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr6,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr7,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"

        /* h8~h15 */
        "vld     $vr0,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr1,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr2,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr3,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr4,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr5,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr6,    %[src],  0          \n\t"
        "add.d   %[src],  %[src],  %[stride]  \n\t"
        "vld     $vr7,    %[src],  0          \n\t"

        "vld     $vr8,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr9,    %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr10,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr11,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr12,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr13,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr14,   %[tmp],  0          \n\t"
        "add.d   %[tmp],  %[tmp],  %[stride]  \n\t"
        "vld     $vr15,   %[tmp],  0          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0       \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1       \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2       \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3       \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4       \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5       \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6       \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7       \n\t"

        "vst     $vr0,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr1,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr2,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr3,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr4,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr5,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr6,    %[dst],  0          \n\t"
        "add.d   %[dst],  %[dst],  %[stride]  \n\t"
        "vst     $vr7,    %[dst],  0          \n\t"
        : [dst]"+&r"(dst), [tmp]"+&r"(tmp), [src]"+&r"(src)
        : [stride]"r"(stride)
        : "memory"
    );
}

/* avg_pixels16_8_lsx   : dst = avg(src, dst)
 * put_pixels16_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels16_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
put_pixels16_l2_8_lsx(uint8_t *dst, const uint8_t *src, uint8_t *half,
                      ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr1,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr2,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr3,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr4,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr5,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr6,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr7,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"

        "vld     $vr8,    %[half], 0x00          \n\t"
        "vld     $vr9,    %[half], 0x10          \n\t"
        "vld     $vr10,   %[half], 0x20          \n\t"
        "vld     $vr11,   %[half], 0x30          \n\t"
        "vld     $vr12,   %[half], 0x40          \n\t"
        "vld     $vr13,   %[half], 0x50          \n\t"
        "vld     $vr14,   %[half], 0x60          \n\t"
        "vld     $vr15,   %[half], 0x70          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vst     $vr0,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr1,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr2,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr3,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr4,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr5,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr6,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr7,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"

        /* h8~h15 */
        "vld     $vr0,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr1,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr2,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr3,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr4,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr5,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr6,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr7,    %[src],  0             \n\t"

        "vld     $vr8,    %[half], 0x80          \n\t"
        "vld     $vr9,    %[half], 0x90          \n\t"
        "vld     $vr10,   %[half], 0xa0          \n\t"
        "vld     $vr11,   %[half], 0xb0          \n\t"
        "vld     $vr12,   %[half], 0xc0          \n\t"
        "vld     $vr13,   %[half], 0xd0          \n\t"
        "vld     $vr14,   %[half], 0xe0          \n\t"
        "vld     $vr15,   %[half], 0xf0          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vst     $vr0,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr1,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr2,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr3,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr4,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr5,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr6,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr7,    %[dst],  0             \n\t"
        : [dst]"+&r"(dst), [half]"+&r"(half), [src]"+&r"(src)
        : [dstStride]"r"(dstStride), [srcStride]"r"(srcStride)
        : "memory"
    );
}

/* avg_pixels16_8_lsx    : dst = avg(src, dst)
 * put_pixels16_l2_8_lsx: dst = avg(src, half) , half stride is 8.
 * avg_pixels16_l2_8_lsx: dst = avg(avg(src, half), dst) , half stride is 8.*/
static av_always_inline void
avg_pixels16_l2_8_lsx(uint8_t *dst, const uint8_t *src, uint8_t *half,
                      ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    uint8_t *tmp = dst;
    __asm__ volatile (
        /* h0~h7 */
        "vld     $vr0,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr1,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr2,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr3,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr4,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr5,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr6,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr7,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"

        "vld     $vr8,    %[half], 0x00          \n\t"
        "vld     $vr9,    %[half], 0x10          \n\t"
        "vld     $vr10,   %[half], 0x20          \n\t"
        "vld     $vr11,   %[half], 0x30          \n\t"
        "vld     $vr12,   %[half], 0x40          \n\t"
        "vld     $vr13,   %[half], 0x50          \n\t"
        "vld     $vr14,   %[half], 0x60          \n\t"
        "vld     $vr15,   %[half], 0x70          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vld     $vr8,    %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr9,    %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr10,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr11,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr12,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr13,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr14,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr15,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vst     $vr0,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr1,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr2,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr3,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr4,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr5,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr6,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr7,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"

        /* h8~h15 */
        "vld     $vr0,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr1,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr2,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr3,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr4,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr5,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr6,    %[src],  0             \n\t"
        "add.d   %[src],  %[src],  %[srcStride]  \n\t"
        "vld     $vr7,    %[src],  0             \n\t"

        "vld     $vr8,    %[half], 0x80          \n\t"
        "vld     $vr9,    %[half], 0x90          \n\t"
        "vld     $vr10,   %[half], 0xa0          \n\t"
        "vld     $vr11,   %[half], 0xb0          \n\t"
        "vld     $vr12,   %[half], 0xc0          \n\t"
        "vld     $vr13,   %[half], 0xd0          \n\t"
        "vld     $vr14,   %[half], 0xe0          \n\t"
        "vld     $vr15,   %[half], 0xf0          \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vld     $vr8,    %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr9,    %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr10,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr11,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr12,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr13,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr14,   %[tmp],  0             \n\t"
        "add.d   %[tmp],  %[tmp],  %[dstStride]  \n\t"
        "vld     $vr15,   %[tmp],  0             \n\t"

        "vavgr.bu $vr0,   $vr8,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr9,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr10,   $vr2          \n\t"
        "vavgr.bu $vr3,   $vr11,   $vr3          \n\t"
        "vavgr.bu $vr4,   $vr12,   $vr4          \n\t"
        "vavgr.bu $vr5,   $vr13,   $vr5          \n\t"
        "vavgr.bu $vr6,   $vr14,   $vr6          \n\t"
        "vavgr.bu $vr7,   $vr15,   $vr7          \n\t"

        "vst     $vr0,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr1,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr2,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr3,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr4,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr5,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr6,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr7,    %[dst],  0             \n\t"
        : [dst]"+&r"(dst), [tmp]"+&r"(tmp), [half]"+&r"(half), [src]"+&r"(src)
        : [dstStride]"r"(dstStride), [srcStride]"r"(srcStride)
        : "memory"
    );
}

#define QPEL8_H_LOWPASS                            \
    LASX_LD_2(src - 2, 1, src00, src01);           \
    LASX_LD_4(src, 1, src02, src03, src04, src05); \
    src += srcStride;                              \
    LASX_LD_2(src - 2, 1, src10, src11)            \
    LASX_LD_4(src, 1, src12, src13, src14, src15); \
    src += srcStride;                              \
    src00 = __lasx_xvpermi_q(src00, src10, 0x02);  \
    src01 = __lasx_xvpermi_q(src01, src11, 0x02);  \
    src02 = __lasx_xvpermi_q(src02, src12, 0x02);  \
    src03 = __lasx_xvpermi_q(src03, src13, 0x02);  \
    src04 = __lasx_xvpermi_q(src04, src14, 0x02);  \
    src05 = __lasx_xvpermi_q(src05, src15, 0x02);  \
    src00 = __lasx_xvilvl_b(zero, src00);          \
    src01 = __lasx_xvilvl_b(zero, src01);          \
    src02 = __lasx_xvilvl_b(zero, src02);          \
    src02 = __lasx_xvsaddw_hu_hu_bu(src02, src03); \
    src01 = __lasx_xvsaddw_hu_hu_bu(src01, src04); \
    src00 = __lasx_xvsaddw_hu_hu_bu(src00, src05); \
    src02 = __lasx_xvmul_h(src02, h_20);           \
    src01 = __lasx_xvmul_h(src01, h_5);            \
    src02 = __lasx_xvssub_h(src02, src01);         \
    src02 = __lasx_xvsadd_h(src02, src00);         \
    src02 = __lasx_xvsadd_h(src02, h_16);          \
    src02 = __lasx_xvssrani_bu_h(src02, src02, 5); \

static av_always_inline void
put_h264_qpel8_h_lowpass_lasx(uint8_t *dst, const uint8_t *src, int dstStride,
                              int srcStride)
{
    __m256i src00, src01, src02, src03, src04, src05;
    __m256i src10, src11, src12, src13, src14, src15;
    __m256i zero = {0};
    __m256i h_20 = {20};
    __m256i h_5  = {5};
    __m256i h_16 = {16};

    zero = __lasx_xvreplve0_h(zero);
    h_20 = __lasx_xvreplve0_h(h_20);
    h_5  = __lasx_xvreplve0_h(h_5);
    h_16 = __lasx_xvreplve0_h(h_16);

    QPEL8_H_LOWPASS
    LASX_ST_D_2(src02, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_H_LOWPASS
    LASX_ST_D_2(src02, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_H_LOWPASS
    LASX_ST_D_2(src02, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_H_LOWPASS
    LASX_ST_D_2(src02, 0, 2, dst, dstStride);
}

#define QPEL8_V_LOWPASS(src0, src1, src2, src3, src4, src5, src6, \
                        tmp0, tmp1, tmp2, tmp3, tmp4, tmp5)       \
{                                                                 \
    tmp0 = __lasx_xvpermi_q(src0, src1, 0x02);                    \
    tmp1 = __lasx_xvpermi_q(src1, src2, 0x02);                    \
    tmp2 = __lasx_xvpermi_q(src2, src3, 0x02);                    \
    tmp3 = __lasx_xvpermi_q(src3, src4, 0x02);                    \
    tmp4 = __lasx_xvpermi_q(src4, src5, 0x02);                    \
    tmp5 = __lasx_xvpermi_q(src5, src6, 0x02);                    \
    tmp0 = __lasx_xvilvl_b(zero, tmp0);                           \
    tmp1 = __lasx_xvilvl_b(zero, tmp1);                           \
    tmp2 = __lasx_xvilvl_b(zero, tmp2);                           \
    tmp2 = __lasx_xvsaddw_hu_hu_bu(tmp2, tmp3);                   \
    tmp1 = __lasx_xvsaddw_hu_hu_bu(tmp1, tmp4);                   \
    tmp0 = __lasx_xvsaddw_hu_hu_bu(tmp0, tmp5);                   \
    tmp2 = __lasx_xvmul_h(tmp2, h_20);                            \
    tmp1 = __lasx_xvmul_h(tmp1, h_5);                             \
    tmp2 = __lasx_xvssub_h(tmp2, tmp1);                           \
    tmp2 = __lasx_xvsadd_h(tmp2, tmp0);                           \
    tmp2 = __lasx_xvsadd_h(tmp2, h_16);                           \
    tmp2 = __lasx_xvssrani_bu_h(tmp2, tmp2, 5);                   \
}

static av_always_inline void
put_h264_qpel8_v_lowpass_lasx(uint8_t *dst, const uint8_t *src, int dstStride,
                              int srcStride)
{
    __m256i src00, src01, src02, src03, src04, src05, src06;
    __m256i tmp00, tmp01, tmp02, tmp03, tmp04, tmp05;
    __m256i zero = {0};
    __m256i h_20 = {20};
    __m256i h_5  = {5};
    __m256i h_16 = {16};

    zero = __lasx_xvreplve0_h(zero);
    h_20 = __lasx_xvreplve0_h(h_20);
    h_5  = __lasx_xvreplve0_h(h_5);
    h_16 = __lasx_xvreplve0_h(h_16);

    LASX_LD_2(src - (srcStride << 1), srcStride, src00, src01);
    LASX_LD_4(src, srcStride, src02, src03, src04, src05);
    src += srcStride << 2;
    src06 = LASX_LD(src);
    src += srcStride;
    QPEL8_V_LOWPASS(src00, src01, src02, src03, src04, src05, src06,
                    tmp00, tmp01, tmp02, tmp03, tmp04, tmp05);
    LASX_ST_D_2(tmp02, 0, 2, dst, dstStride);
    dst += dstStride << 1;

    LASX_LD_2(src, srcStride, src00, src01);
    src += srcStride << 1;
    QPEL8_V_LOWPASS(src02, src03, src04, src05, src06, src00, src01,
                    tmp00, tmp01, tmp02, tmp03, tmp04, tmp05);
    LASX_ST_D_2(tmp02, 0, 2, dst, dstStride);
    dst += dstStride << 1;

    LASX_LD_2(src, srcStride, src02, src03);
    src += srcStride << 1;
    QPEL8_V_LOWPASS(src04, src05, src06, src00, src01, src02, src03,
                    tmp00, tmp01, tmp02, tmp03, tmp04, tmp05);
    LASX_ST_D_2(tmp02, 0, 2, dst, dstStride);
    dst += dstStride << 1;

    LASX_LD_2(src, srcStride, src04, src05);
    QPEL8_V_LOWPASS(src06, src00, src01, src02, src03, src04, src05,
                    tmp00, tmp01, tmp02, tmp03, tmp04, tmp05);
    LASX_ST_D_2(tmp02, 0, 2, dst, dstStride);
}

#define QPEL8_HV_LOWPASS_H(tmp)                    \
{                                                  \
    LASX_LD_2(src - 2, 1, src00, src01);           \
    LASX_LD_4(src, 1, src02, src03, src04, src05); \
    src += srcStride;                              \
    LASX_LD_2(src - 2, 1, src10, src11)            \
    LASX_LD_4(src, 1, src12, src13, src14, src15); \
    src += srcStride;                              \
    src00 = __lasx_xvpermi_q(src00, src10, 0x02);  \
    src01 = __lasx_xvpermi_q(src01, src11, 0x02);  \
    src02 = __lasx_xvpermi_q(src02, src12, 0x02);  \
    src03 = __lasx_xvpermi_q(src03, src13, 0x02);  \
    src04 = __lasx_xvpermi_q(src04, src14, 0x02);  \
    src05 = __lasx_xvpermi_q(src05, src15, 0x02);  \
    src00 = __lasx_xvilvl_b(zero, src00);          \
    src01 = __lasx_xvilvl_b(zero, src01);          \
    src02 = __lasx_xvilvl_b(zero, src02);          \
    src02 = __lasx_xvsaddw_hu_hu_bu(src02, src03); \
    src01 = __lasx_xvsaddw_hu_hu_bu(src01, src04); \
    src00 = __lasx_xvsaddw_hu_hu_bu(src00, src05); \
    src02 = __lasx_xvmul_h(src02, h_20);           \
    src01 = __lasx_xvmul_h(src01, h_5);            \
    src02 = __lasx_xvssub_h(src02, src01);         \
    tmp  = __lasx_xvsadd_h(src02, src00);          \
}

#define QPEL8_HV_LOWPASS_V(src0, src1, src2, src3,     \
                           src4, src5, temp0, temp1,   \
                           temp2, temp3, temp4, temp5, \
                           out)                        \
{                                                      \
    temp0 = __lasx_xvaddwl_w_h(src2, src3);            \
    temp1 = __lasx_xvaddwh_w_h(src2, src3);            \
    temp2 = __lasx_xvaddwl_w_h(src1, src4);            \
    temp3 = __lasx_xvaddwh_w_h(src1, src4);            \
    temp4 = __lasx_xvaddwl_w_h(src0, src5);            \
    temp5 = __lasx_xvaddwh_w_h(src0, src5);            \
    temp0 = __lasx_xvmul_w(temp0, w_20);               \
    temp1 = __lasx_xvmul_w(temp1, w_20);               \
    temp2 = __lasx_xvmul_w(temp2, w_5);                \
    temp3 = __lasx_xvmul_w(temp3, w_5);                \
    temp0 = __lasx_xvssub_w(temp0, temp2);             \
    temp1 = __lasx_xvssub_w(temp1, temp3);             \
    temp0 = __lasx_xvsadd_w(temp0, temp4);             \
    temp1 = __lasx_xvsadd_w(temp1, temp5);             \
    temp0 = __lasx_xvsadd_w(temp0, w_512);             \
    temp1 = __lasx_xvsadd_w(temp1, w_512);             \
    temp0 = __lasx_xvssrani_hu_w(temp0, temp0, 10);    \
    temp1 = __lasx_xvssrani_hu_w(temp1, temp1, 10);    \
    temp0 = __lasx_xvpackev_d(temp1, temp0);           \
    out   = __lasx_xvssrani_bu_h(temp0, temp0, 0);     \
}

static void put_h264_qpel8_hv_lowpass_lasx(uint8_t *dst, const uint8_t *src,
                                           ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    __m256i src00, src01, src02, src03, src04, src05;
    __m256i src10, src11, src12, src13, src14, src15;
    __m256i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    __m256i tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
    __m256i zero = {0};
    __m256i h_20 = {20};
    __m256i h_5  = {5};
    __m256i w_20 = {20};
    __m256i w_5  = {5};
    __m256i w_512 = {512};

    zero = __lasx_xvreplve0_h(zero);
    h_20 = __lasx_xvreplve0_h(w_20);
    h_5  = __lasx_xvreplve0_h(w_5);
    w_20 = __lasx_xvreplve0_w(w_20);
    w_5  = __lasx_xvreplve0_w(w_5);
    w_512 = __lasx_xvreplve0_w(w_512);

    src -= srcStride << 1;
    QPEL8_HV_LOWPASS_H(tmp0)
    QPEL8_HV_LOWPASS_H(tmp2)
    QPEL8_HV_LOWPASS_H(tmp4)
    QPEL8_HV_LOWPASS_H(tmp6)
    QPEL8_HV_LOWPASS_H(tmp8)
    QPEL8_HV_LOWPASS_H(tmp10)
    QPEL8_HV_LOWPASS_H(tmp12)
    tmp11 = __lasx_xvpermi_q(tmp12, tmp10, 0x21);
    tmp9  = __lasx_xvpermi_q(tmp10, tmp8,  0x21);
    tmp7  = __lasx_xvpermi_q(tmp8,  tmp6,  0x21);
    tmp5  = __lasx_xvpermi_q(tmp6,  tmp4,  0x21);
    tmp3  = __lasx_xvpermi_q(tmp4,  tmp2,  0x21);
    tmp1  = __lasx_xvpermi_q(tmp2,  tmp0,  0x21);

    QPEL8_HV_LOWPASS_V(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, src00, src01,
                       src02, src03, src04, src05, tmp0)
    LASX_ST_D_2(tmp0, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_HV_LOWPASS_V(tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, src00, src01,
                       src02, src03, src04, src05, tmp2)
    LASX_ST_D_2(tmp2, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_HV_LOWPASS_V(tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, src00, src01,
                       src02, src03, src04, src05, tmp4)
    LASX_ST_D_2(tmp4, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    QPEL8_HV_LOWPASS_V(tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, src00, src01,
                       src02, src03, src04, src05, tmp6)
    LASX_ST_D_2(tmp6, 0, 2, dst, dstStride);
}

static av_always_inline void
avg_h264_qpel8_h_lowpass_lasx(uint8_t *dst, const uint8_t *src, int dstStride,
                              int srcStride)
{
    __m256i src00, src01, src02, src03, src04, src05;
    __m256i src10, src11, src12, src13, src14, src15;
    __m256i dst00, dst01;
    __m256i zero = {0};
    __m256i h_20 = {20};
    __m256i h_5  = {5};
    __m256i h_16 = {16};

    zero = __lasx_xvreplve0_h(zero);
    h_20 = __lasx_xvreplve0_h(h_20);
    h_5  = __lasx_xvreplve0_h(h_5);
    h_16 = __lasx_xvreplve0_h(h_16);

    LASX_LD_2(dst, dstStride, dst00, dst01);
    QPEL8_H_LOWPASS
    dst00 = __lasx_xvpermi_q(dst00, dst01, 0x02);
    dst00 = __lasx_xvavgr_bu(dst00, src02);
    LASX_ST_D_2(dst00, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    LASX_LD_2(dst, dstStride, dst00, dst01);
    QPEL8_H_LOWPASS
    dst00 = __lasx_xvpermi_q(dst00, dst01, 0x02);
    dst00 = __lasx_xvavgr_bu(dst00, src02);
    LASX_ST_D_2(dst00, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    LASX_LD_2(dst, dstStride, dst00, dst01);
    QPEL8_H_LOWPASS
    dst00 = __lasx_xvpermi_q(dst00, dst01, 0x02);
    dst00 = __lasx_xvavgr_bu(dst00, src02);
    LASX_ST_D_2(dst00, 0, 2, dst, dstStride);
    dst += dstStride << 1;
    LASX_LD_2(dst, dstStride, dst00, dst01);
    QPEL8_H_LOWPASS
    dst00 = __lasx_xvpermi_q(dst00, dst01, 0x02);
    dst00 = __lasx_xvavgr_bu(dst00, src02);
    LASX_ST_D_2(dst00, 0, 2, dst, dstStride);
}

static av_always_inline void
put_h264_qpel16_h_lowpass_lasx(uint8_t *dst, const uint8_t *src,
                               int dstStride, int srcStride)
{
    put_h264_qpel8_h_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
    src += srcStride << 3;
    dst += dstStride << 3;
    put_h264_qpel8_h_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
}

static av_always_inline void
avg_h264_qpel16_h_lowpass_lasx(uint8_t *dst, const uint8_t *src,
                               int dstStride, int srcStride)
{
    avg_h264_qpel8_h_lowpass_lasx(dst, src, dstStride, srcStride);
    avg_h264_qpel8_h_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
    src += srcStride << 3;
    dst += dstStride << 3;
    avg_h264_qpel8_h_lowpass_lasx(dst, src, dstStride, srcStride);
    avg_h264_qpel8_h_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
}

static void put_h264_qpel16_v_lowpass_lasx(uint8_t *dst, const uint8_t *src,
                                           int dstStride, int srcStride)
{
    put_h264_qpel8_v_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
    src += 8*srcStride;
    dst += 8*dstStride;
    put_h264_qpel8_v_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_lasx(dst+8, src+8, dstStride, srcStride);
}

static void put_h264_qpel16_hv_lowpass_lasx(uint8_t *dst, const uint8_t *src,
                                            ptrdiff_t dstStride, ptrdiff_t srcStride)
{
    put_h264_qpel8_hv_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_hv_lowpass_lasx(dst + 8, src + 8, dstStride, srcStride);
    src += srcStride << 3;
    dst += dstStride << 3;
    put_h264_qpel8_hv_lowpass_lasx(dst, src, dstStride, srcStride);
    put_h264_qpel8_hv_lowpass_lasx(dst + 8, src + 8, dstStride, srcStride);
}

void ff_put_h264_qpel8_mc00_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    /* In mmi optimization, it used function ff_put_pixels8_8_mmi
     * which implemented in hpeldsp_mmi.c */
    put_pixels8_8_inline_asm(dst, src, stride);
}

void ff_put_h264_qpel8_mc10_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_h_lowpass_lasx(half, src, 8, stride);
    /* in qpel8, the stride of half and height of block is 8 */
    put_pixels8_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_put_h264_qpel8_mc20_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    put_h264_qpel8_h_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel8_mc30_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_h_lowpass_lasx(half, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, src+1, half, stride, stride);
}

void ff_put_h264_qpel8_mc01_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_v_lowpass_lasx(half, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_put_h264_qpel8_mc11_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_put_h264_qpel8_mc21_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 64;

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_put_h264_qpel8_mc31_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src + 1, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_put_h264_qpel8_mc02_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    put_h264_qpel8_v_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel8_mc12_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 64;

    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfH, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_put_h264_qpel8_mc22_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    put_h264_qpel8_hv_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel8_mc32_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 64;

    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfH, src + 1, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_put_h264_qpel8_mc03_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_v_lowpass_lasx(half, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, src + stride, half, stride, stride);
}

void ff_put_h264_qpel8_mc13_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_put_h264_qpel8_mc23_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 64;

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_put_h264_qpel8_mc33_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src + 1, 8, stride);
    put_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_avg_h264_qpel8_mc00_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    /* In mmi optimization, it used function ff_avg_pixels8_8_mmi
     * which implemented in hpeldsp_mmi.c */
    avg_pixels8_8_lsx(dst, src, stride);
}

void ff_avg_h264_qpel8_mc10_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_h_lowpass_lasx(half, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_avg_h264_qpel8_mc20_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    avg_h264_qpel8_h_lowpass_lasx(dst, src, stride, stride);
}

void ff_avg_h264_qpel8_mc30_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t half[64];

    put_h264_qpel8_h_lowpass_lasx(half, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, src+1, half, stride, stride);
}

void ff_avg_h264_qpel8_mc11_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_avg_h264_qpel8_mc21_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 64;

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_avg_h264_qpel8_mc31_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src + 1, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_avg_h264_qpel8_mc12_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 64;

    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfH, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_avg_h264_qpel8_mc32_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 64;

    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfH, src + 1, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_avg_h264_qpel8_mc13_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_avg_h264_qpel8_mc23_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t temp[128];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 64;

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_hv_lowpass_lasx(halfHV, src, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfHV, stride, 8);
}

void ff_avg_h264_qpel8_mc33_lasx(uint8_t *dst, const uint8_t *src,
                                 ptrdiff_t stride)
{
    uint8_t halfH[64];
    uint8_t halfV[64];

    put_h264_qpel8_h_lowpass_lasx(halfH, src + stride, 8, stride);
    put_h264_qpel8_v_lowpass_lasx(halfV, src + 1, 8, stride);
    avg_pixels8_l2_8_lsx(dst, halfH, halfV, stride, 8);
}

void ff_put_h264_qpel16_mc00_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    /* In mmi optimization, it used function ff_put_pixels16_8_mmi
     * which implemented in hpeldsp_mmi.c */
    put_pixels16_8_lsx(dst, src, stride);
}

void ff_put_h264_qpel16_mc10_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_h_lowpass_lasx(half, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_put_h264_qpel16_mc20_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    put_h264_qpel16_h_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel16_mc30_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_h_lowpass_lasx(half, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, src+1, half, stride, stride);
}

void ff_put_h264_qpel16_mc01_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_v_lowpass_lasx(half, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_put_h264_qpel16_mc11_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_16x16_lasx(src - 2, src - (stride * 2), dst, stride);
}

void ff_put_h264_qpel16_mc21_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lasx(halfH, src, 16, stride);
    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc31_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_16x16_lasx(src - 2, src - (stride * 2) + 1, dst, stride);
}

void ff_put_h264_qpel16_mc02_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    put_h264_qpel16_v_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel16_mc12_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lasx(halfH, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc22_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    put_h264_qpel16_hv_lowpass_lasx(dst, src, stride, stride);
}

void ff_put_h264_qpel16_mc32_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lasx(halfH, src + 1, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc03_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_v_lowpass_lasx(half, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, src+stride, half, stride, stride);
}

void ff_put_h264_qpel16_mc13_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_16x16_lasx(src + stride - 2, src - (stride * 2), dst,
                               stride);
}

void ff_put_h264_qpel16_mc23_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lasx(halfH, src + stride, 16, stride);
    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_put_h264_qpel16_mc33_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_16x16_lasx(src + stride - 2, src - (stride * 2) + 1, dst,
                               stride);
}

void ff_avg_h264_qpel16_mc00_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    /* In mmi optimization, it used function ff_avg_pixels16_8_mmi
     * which implemented in hpeldsp_mmi.c */
    avg_pixels16_8_lsx(dst, src, stride);
}

void ff_avg_h264_qpel16_mc10_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_h_lowpass_lasx(half, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_avg_h264_qpel16_mc20_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avg_h264_qpel16_h_lowpass_lasx(dst, src, stride, stride);
}

void ff_avg_h264_qpel16_mc30_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_h_lowpass_lasx(half, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, src+1, half, stride, stride);
}

void ff_avg_h264_qpel16_mc01_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_v_lowpass_lasx(half, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, src, half, stride, stride);
}

void ff_avg_h264_qpel16_mc11_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_and_aver_dst_16x16_lasx(src - 2,
                                            src - (stride * 2),
                                            dst, stride);
}

void ff_avg_h264_qpel16_mc21_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lasx(halfH, src, 16, stride);
    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_avg_h264_qpel16_mc31_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_and_aver_dst_16x16_lasx(src - 2,
                                            src - (stride * 2) +
                                            sizeof(uint8_t),
                                            dst, stride);
}

void ff_avg_h264_qpel16_mc12_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lasx(halfH, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_avg_h264_qpel16_mc32_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfHV = temp;
    uint8_t *const halfH  = temp + 256;

    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    put_h264_qpel16_v_lowpass_lasx(halfH, src + 1, 16, stride);
    avg_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_avg_h264_qpel16_mc03_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t half[256];

    put_h264_qpel16_v_lowpass_lasx(half, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, src + stride, half, stride, stride);
}

void ff_avg_h264_qpel16_mc13_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_and_aver_dst_16x16_lasx(src + stride - 2,
                                            src - (stride * 2),
                                            dst, stride);
}

void ff_avg_h264_qpel16_mc23_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    uint8_t temp[512];
    uint8_t *const halfH  = temp;
    uint8_t *const halfHV = temp + 256;

    put_h264_qpel16_h_lowpass_lasx(halfH, src + stride, 16, stride);
    put_h264_qpel16_hv_lowpass_lasx(halfHV, src, 16, stride);
    avg_pixels16_l2_8_lsx(dst, halfH, halfHV, stride, 16);
}

void ff_avg_h264_qpel16_mc33_lasx(uint8_t *dst, const uint8_t *src,
                                  ptrdiff_t stride)
{
    avc_luma_hv_qrt_and_aver_dst_16x16_lasx(src + stride - 2,
                                            src - (stride * 2) +
                                            sizeof(uint8_t),
                                            dst, stride);
}
