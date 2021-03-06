/*
 * Loongson LASX optimized h264dsp
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
 *                Xiwei  Gu  <guxiwei-hf@loongson.cn>
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

#include "libavutil/loongarch/generic_macros_lasx.h"
#include "h264dsp_lasx.h"

#define AVC_LPF_P1_OR_Q1(p0_or_q0_org_in, q0_or_p0_org_in,   \
                         p1_or_q1_org_in, p2_or_q2_org_in,   \
                         neg_tc_in, tc_in, p1_or_q1_out)     \
{                                                            \
    __m256i clip3, temp;                                     \
                                                             \
    clip3 = __lasx_xvavgr_hu(p0_or_q0_org_in,                \
                             q0_or_p0_org_in);               \
    temp = __lasx_xvslli_h(p1_or_q1_org_in, 1);              \
    clip3 = __lasx_xvsub_h(clip3, temp);                     \
    clip3 = __lasx_xvavg_h(p2_or_q2_org_in, clip3);          \
    LASX_CLIP_H(clip3, neg_tc_in, tc_in);                    \
    p1_or_q1_out = __lasx_xvadd_h(p1_or_q1_org_in, clip3);   \
}

#define AVC_LPF_P0Q0(q0_or_p0_org_in, p0_or_q0_org_in,       \
                     p1_or_q1_org_in, q1_or_p1_org_in,       \
                     neg_threshold_in, threshold_in,         \
                     p0_or_q0_out, q0_or_p0_out)             \
{                                                            \
    __m256i q0_sub_p0, p1_sub_q1, delta;                     \
                                                             \
    q0_sub_p0 = __lasx_xvsub_h(q0_or_p0_org_in,              \
                               p0_or_q0_org_in);             \
    p1_sub_q1 = __lasx_xvsub_h(p1_or_q1_org_in,              \
                               q1_or_p1_org_in);             \
    q0_sub_p0 = __lasx_xvslli_h(q0_sub_p0, 2);               \
    p1_sub_q1 = __lasx_xvaddi_hu(p1_sub_q1, 4);              \
    delta = __lasx_xvadd_h(q0_sub_p0, p1_sub_q1);            \
    delta = __lasx_xvsrai_h(delta, 3);                       \
                                                             \
    LASX_CLIP_H(delta, neg_threshold_in, threshold_in);      \
                                                             \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_org_in, delta);   \
    q0_or_p0_out = __lasx_xvsub_h(q0_or_p0_org_in, delta);   \
                                                             \
    LASX_CLIP_H_0_255(p0_or_q0_out, p0_or_q0_out);           \
    LASX_CLIP_H_0_255(q0_or_p0_out, q0_or_p0_out);           \
}

void ff_h264_h_lpf_luma_8_lasx(uint8_t *data, int img_width,
                                   int alpha_in, int beta_in, int8_t *tc)
{
    int img_width_2x = img_width << 1;
    int img_width_8x = img_width << 3;
    __m256i tmp_vec0, bs_vec;
    __m256i tc_vec = {0x0101010100000000, 0x0303030302020202, 0x0101010100000000, 0x0303030302020202};

    tmp_vec0 = __lasx_xvldrepl_w((uint32_t*)tc, 0);
    tc_vec   = __lasx_xvshuf_b(tmp_vec0, tmp_vec0, tc_vec);
    bs_vec   = __lasx_xvslti_b(tc_vec, 0);
    bs_vec   = __lasx_xvxori_b(bs_vec, 255);
    bs_vec   = __lasx_xvandi_b(bs_vec, 1);

    if (__lasx_xbnz_v(bs_vec)) {
        uint8_t *src = data - 4;
        __m256i p3_org, p2_org, p1_org, p0_org, q0_org, q1_org, q2_org, q3_org;
        __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
        __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
        __m256i is_bs_greater_than0;
        __m256i zero = __lasx_xvldi(0);

        is_bs_greater_than0 = __lasx_xvslt_bu(zero, bs_vec);

        {
            uint8_t *src_tmp = src + img_width_8x;
            __m256i row0, row1, row2, row3, row4, row5, row6, row7;
            __m256i row8, row9, row10, row11, row12, row13, row14, row15;

            LASX_LD_8(src, img_width, row0, row1, row2, row3,
                      row4, row5, row6, row7);
            LASX_LD_8(src_tmp, img_width, row8, row9, row10, row11,
                      row12, row13, row14, row15);

            LASX_TRANSPOSE16x8_B(row0, row1, row2, row3, row4, row5, row6, row7,
                                 row8, row9, row10, row11,
                                 row12, row13, row14, row15,
                                 p3_org, p2_org, p1_org, p0_org,
                                 q0_org, q1_org, q2_org, q3_org);
        }

        p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
        p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
        q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

        alpha = __lasx_xvldrepl_b(&alpha_in, 0);
        beta  = __lasx_xvldrepl_b(&beta_in, 0);

        is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
        is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
        is_less_than       = is_less_than_alpha & is_less_than_beta;
        is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
        is_less_than       = is_less_than_beta & is_less_than;
        is_less_than       = is_less_than & is_bs_greater_than0;

        if (__lasx_xbnz_v(is_less_than)) {
            __m256i neg_tc_h, tc_h, p1_org_h, p0_org_h, q0_org_h, q1_org_h;
            __m256i p2_asub_p0, q2_asub_q0;

            neg_tc_h = __lasx_xvneg_b(tc_vec);
            neg_tc_h = __lasx_vext2xv_h_b(neg_tc_h);
            tc_h     = __lasx_vext2xv_hu_bu(tc_vec);
            p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
            p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
            q0_org_h = __lasx_vext2xv_hu_bu(q0_org);

            p2_asub_p0 = __lasx_xvabsd_bu(p2_org, p0_org);
            is_less_than_beta = __lasx_xvslt_bu(p2_asub_p0, beta);
            is_less_than_beta = is_less_than_beta & is_less_than;

            if (__lasx_xbnz_v(is_less_than_beta)) {
                __m256i p2_org_h, p1_h;

                p2_org_h = __lasx_vext2xv_hu_bu(p2_org);
                AVC_LPF_P1_OR_Q1(p0_org_h, q0_org_h, p1_org_h, p2_org_h,
                                 neg_tc_h, tc_h, p1_h);
                LASX_PCKEV_B(p1_h, p1_h, p1_h);

                p1_org = __lasx_xvbitsel_v(p1_org, p1_h, is_less_than_beta);
                is_less_than_beta = __lasx_xvandi_b(is_less_than_beta, 1);
                tc_vec = __lasx_xvadd_b(tc_vec, is_less_than_beta);
            }

            q2_asub_q0 = __lasx_xvabsd_bu(q2_org, q0_org);
            is_less_than_beta = __lasx_xvslt_bu(q2_asub_q0, beta);
            is_less_than_beta = is_less_than_beta & is_less_than;

            q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

            if (__lasx_xbnz_v(is_less_than_beta)) {
                __m256i q2_org_h, q1_h;

                q2_org_h = __lasx_vext2xv_hu_bu(q2_org);
                AVC_LPF_P1_OR_Q1(p0_org_h, q0_org_h, q1_org_h, q2_org_h,
                                 neg_tc_h, tc_h, q1_h);
                LASX_PCKEV_B(q1_h, q1_h, q1_h);
                q1_org = __lasx_xvbitsel_v(q1_org, q1_h, is_less_than_beta);

                is_less_than_beta = __lasx_xvandi_b(is_less_than_beta, 1);
                tc_vec = __lasx_xvadd_b(tc_vec, is_less_than_beta);
            }

            {
                __m256i neg_thresh_h, p0_h, q0_h;

                neg_thresh_h = __lasx_xvneg_b(tc_vec);
                neg_thresh_h = __lasx_vext2xv_h_b(neg_thresh_h);
                tc_h         = __lasx_vext2xv_hu_bu(tc_vec);

                AVC_LPF_P0Q0(q0_org_h, p0_org_h, p1_org_h, q1_org_h,
                             neg_thresh_h, tc_h, p0_h, q0_h);
                LASX_PCKEV_B_2(p0_h, p0_h, q0_h, q0_h, p0_h, q0_h);
                p0_org = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than);
                q0_org = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than);
            }

            {
                __m256i row0, row1, row2, row3, row4, row5, row6, row7;

                LASX_ILVL_B_4(p1_org, p3_org, p0_org, p2_org, q2_org, q0_org, q3_org,
                              q1_org, row0, row1, row2, row3);
                LASX_ILVLH_B_2(row1, row0, row3, row2, row5, row4, row7, row6);
                LASX_ILVLH_W_2(row6, row4, row7, row5, row1, row0, row3, row2);
                LASX_PCKEV_Q_4(row0, row0, row1, row1, row2, row2, row3, row3,
                               row4, row5, row6, row7);
                LASX_ST_D_2(row4, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row0, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row5, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row1, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row6, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row2, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row7, 2, 3, src, img_width);
                src += img_width_2x;
                LASX_ST_D_2(row3, 2, 3, src, img_width);
            }
        }
    }
}

void ff_h264_v_lpf_luma_8_lasx(uint8_t *data, int img_width,
                                   int alpha_in, int beta_in, int8_t *tc)
{
    int img_width_2x = img_width << 1;
    int img_width_3x = img_width + img_width_2x;
    __m256i tmp_vec0, bs_vec;
    __m256i tc_vec = {0x0101010100000000, 0x0303030302020202, 0x0101010100000000, 0x0303030302020202};

    tmp_vec0 = __lasx_xvldrepl_w((uint32_t*)tc, 0);
    tc_vec   = __lasx_xvshuf_b(tmp_vec0, tmp_vec0, tc_vec);
    bs_vec   = __lasx_xvslti_b(tc_vec, 0);
    bs_vec   = __lasx_xvxori_b(bs_vec, 255);
    bs_vec   = __lasx_xvandi_b(bs_vec, 1);

    if (__lasx_xbnz_v(bs_vec)) {
        __m256i p2_org, p1_org, p0_org, q0_org, q1_org, q2_org;
        __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
        __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
        __m256i p1_org_h, p0_org_h, q0_org_h, q1_org_h;
        __m256i is_bs_greater_than0;
        __m256i zero = __lasx_xvldi(0);

        alpha = __lasx_xvldrepl_b(&alpha_in, 0);
        beta  = __lasx_xvldrepl_b(&beta_in, 0);

        p2_org = LASX_LD(data - img_width_3x);
        p1_org = LASX_LD(data - img_width_2x);
        p0_org = LASX_LD(data - img_width);
        LASX_LD_2(data, img_width, q0_org, q1_org);

        is_bs_greater_than0 = __lasx_xvslt_bu(zero, bs_vec);
        p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
        p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
        q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

        is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
        is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
        is_less_than       = is_less_than_alpha & is_less_than_beta;
        is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
        is_less_than       = is_less_than_beta & is_less_than;
        is_less_than       = is_less_than & is_bs_greater_than0;

        if (__lasx_xbnz_v(is_less_than)) {
            __m256i neg_tc_h, tc_h, p2_asub_p0, q2_asub_q0;

            q2_org = LASX_LD(data + img_width_2x);

            neg_tc_h = __lasx_xvneg_b(tc_vec);
            neg_tc_h = __lasx_vext2xv_h_b(neg_tc_h);
            tc_h     = __lasx_vext2xv_hu_bu(tc_vec);
            p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
            p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
            q0_org_h = __lasx_vext2xv_hu_bu(q0_org);

            p2_asub_p0        = __lasx_xvabsd_bu(p2_org, p0_org);
            is_less_than_beta = __lasx_xvslt_bu(p2_asub_p0, beta);
            is_less_than_beta = is_less_than_beta & is_less_than;

            if (__lasx_xbnz_v(is_less_than_beta)) {
                __m256i p1_h, p2_org_h;

                p2_org_h = __lasx_vext2xv_hu_bu(p2_org);
                AVC_LPF_P1_OR_Q1(p0_org_h, q0_org_h, p1_org_h, p2_org_h,
                                 neg_tc_h, tc_h, p1_h);
                LASX_PCKEV_B(p1_h, p1_h, p1_h);
                p1_h   = __lasx_xvbitsel_v(p1_org, p1_h, is_less_than_beta);
                p1_org = __lasx_xvpermi_q(p1_org, p1_h, 0x30);
                LASX_ST(p1_org, data - img_width_2x);

                is_less_than_beta = __lasx_xvandi_b(is_less_than_beta, 1);
                tc_vec = __lasx_xvadd_b(tc_vec, is_less_than_beta);
            }

            q2_asub_q0 = __lasx_xvabsd_bu(q2_org, q0_org);
            is_less_than_beta = __lasx_xvslt_bu(q2_asub_q0, beta);
            is_less_than_beta = is_less_than_beta & is_less_than;

            q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

            if (__lasx_xbnz_v(is_less_than_beta)) {
                __m256i q1_h, q2_org_h;

                q2_org_h = __lasx_vext2xv_hu_bu(q2_org);
                AVC_LPF_P1_OR_Q1(p0_org_h, q0_org_h, q1_org_h, q2_org_h,
                                 neg_tc_h, tc_h, q1_h);
                LASX_PCKEV_B(q1_h, q1_h, q1_h);
                q1_h = __lasx_xvbitsel_v(q1_org, q1_h, is_less_than_beta);
                q1_org = __lasx_xvpermi_q(q1_org, q1_h, 0x30);
                LASX_ST(q1_org, data + img_width);

                is_less_than_beta = __lasx_xvandi_b(is_less_than_beta, 1);
                tc_vec = __lasx_xvadd_b(tc_vec, is_less_than_beta);

            }

            {
                __m256i neg_thresh_h, p0_h, q0_h;

                neg_thresh_h = __lasx_xvneg_b(tc_vec);
                neg_thresh_h = __lasx_vext2xv_h_b(neg_thresh_h);
                tc_h         = __lasx_vext2xv_hu_bu(tc_vec);

                AVC_LPF_P0Q0(q0_org_h, p0_org_h, p1_org_h, q1_org_h,
                             neg_thresh_h, tc_h, p0_h, q0_h);
                LASX_PCKEV_B_2(p0_h, p0_h, q0_h, q0_h, p0_h, q0_h);
                p0_h = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than);
                q0_h = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than);
                p0_org = __lasx_xvpermi_q(p0_org, p0_h, 0x30);
                q0_org = __lasx_xvpermi_q(q0_org, q0_h, 0x30);
                LASX_ST(p0_org, data - img_width);
                LASX_ST(q0_org, data);
            }
        }
    }
}

void ff_h264_h_lpf_chroma_8_lasx(uint8_t *data, int img_width,
                                 int alpha_in, int beta_in, int8_t *tc)
{
    __m256i tmp_vec0, bs_vec;
    __m256i tc_vec = {0x0303020201010000, 0x0303020201010000, 0x0, 0x0};
    __m256i zero = __lasx_xvldi(0);

    tmp_vec0 = __lasx_xvldrepl_w((uint32_t*)tc, 0);
    tc_vec   = __lasx_xvshuf_b(tmp_vec0, tmp_vec0, tc_vec);
    bs_vec   = __lasx_xvslti_b(tc_vec, 0);
    bs_vec   = __lasx_xvxori_b(bs_vec, 255);
    bs_vec   = __lasx_xvandi_b(bs_vec, 1);
    bs_vec   = __lasx_xvpermi_q(zero, bs_vec, 0x30);

    if (__lasx_xbnz_v(bs_vec)) {
        uint8_t *src = data - 2;
        __m256i p1_org, p0_org, q0_org, q1_org;
        __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
        __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
        __m256i is_bs_greater_than0;

        is_bs_greater_than0 = __lasx_xvslt_bu(zero, bs_vec);

        {
            __m256i row0, row1, row2, row3, row4, row5, row6, row7;

            LASX_LD_8(src, img_width, row0, row1, row2, row3,
                      row4, row5, row6, row7);
            LASX_ILVL_B_4(row2, row0, row3, row1, row6, row4, row7, row5,
                          p1_org, p0_org, q0_org, q1_org);
            LASX_ILVL_B_2(p0_org, p1_org, q1_org, q0_org, row0, row1);
            LASX_ILVLH_W_128SV(row1, row0, row3, row2);
            p1_org = __lasx_xvpermi_d(row2, 0x00);
            p0_org = __lasx_xvpermi_d(row2, 0x55);
            q0_org = __lasx_xvpermi_d(row3, 0x00);
            q1_org = __lasx_xvpermi_d(row3, 0x55);
        }

        p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
        p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
        q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

        alpha = __lasx_xvldrepl_b(&alpha_in, 0);
        beta  = __lasx_xvldrepl_b(&beta_in, 0);

        is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
        is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
        is_less_than       = is_less_than_alpha & is_less_than_beta;
        is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
        is_less_than       = is_less_than_beta & is_less_than;
        is_less_than       = is_less_than & is_bs_greater_than0;

        if (__lasx_xbnz_v(is_less_than)) {
            __m256i p1_org_h, p0_org_h, q0_org_h, q1_org_h;

            p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
            p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
            q0_org_h = __lasx_vext2xv_hu_bu(q0_org);
            q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

            {
                __m256i tc_h, neg_thresh_h, p0_h, q0_h;

                neg_thresh_h = __lasx_xvneg_b(tc_vec);
                neg_thresh_h = __lasx_vext2xv_h_b(neg_thresh_h);
                tc_h         = __lasx_vext2xv_hu_bu(tc_vec);

                AVC_LPF_P0Q0(q0_org_h, p0_org_h, p1_org_h, q1_org_h,
                             neg_thresh_h, tc_h, p0_h, q0_h);
                LASX_PCKEV_B_2(p0_h, p0_h, q0_h, q0_h, p0_h, q0_h);
                p0_org = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than);
                q0_org = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than);
            }

            p0_org = __lasx_xvilvl_b(q0_org, p0_org);
            src = data - 1;
            __lasx_xvstelm_h(p0_org, src, 0, 0);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 1);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 2);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 3);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 4);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 5);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 6);
            src += img_width;
            __lasx_xvstelm_h(p0_org, src, 0, 7);
        }
    }
}

void ff_h264_v_lpf_chroma_8_lasx(uint8_t *data, int img_width,
                                 int alpha_in, int beta_in, int8_t *tc)
{
    int img_width_2x = img_width << 1;
    __m256i tmp_vec0, bs_vec;
    __m256i tc_vec = {0x0303020201010000, 0x0303020201010000, 0x0, 0x0};
    __m256i zero = __lasx_xvldi(0);

    tmp_vec0 = __lasx_xvldrepl_w((uint32_t*)tc, 0);
    tc_vec   = __lasx_xvshuf_b(tmp_vec0, tmp_vec0, tc_vec);
    bs_vec   = __lasx_xvslti_b(tc_vec, 0);
    bs_vec   = __lasx_xvxori_b(bs_vec, 255);
    bs_vec   = __lasx_xvandi_b(bs_vec, 1);
    bs_vec   = __lasx_xvpermi_q(zero, bs_vec, 0x30);

    if (__lasx_xbnz_v(bs_vec)) {
        __m256i p1_org, p0_org, q0_org, q1_org;
        __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
        __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
        __m256i is_bs_greater_than0;

        alpha = __lasx_xvldrepl_b(&alpha_in, 0);
        beta  = __lasx_xvldrepl_b(&beta_in, 0);

        p1_org = LASX_LD(data - img_width_2x);
        p0_org = LASX_LD(data - img_width);
        LASX_LD_2(data, img_width, q0_org, q1_org);

        is_bs_greater_than0 = __lasx_xvslt_bu(zero, bs_vec);
        p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
        p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
        q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

        is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
        is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
        is_less_than       = is_less_than_alpha & is_less_than_beta;
        is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
        is_less_than       = is_less_than_beta & is_less_than;
        is_less_than       = is_less_than & is_bs_greater_than0;

        if (__lasx_xbnz_v(is_less_than)) {
            __m256i p1_org_h, p0_org_h, q0_org_h, q1_org_h;

            p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
            p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
            q0_org_h = __lasx_vext2xv_hu_bu(q0_org);
            q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

            {
                __m256i neg_thresh_h, tc_h, p0_h, q0_h;

                neg_thresh_h = __lasx_xvneg_b(tc_vec);
                neg_thresh_h = __lasx_vext2xv_h_b(neg_thresh_h);
                tc_h         = __lasx_vext2xv_hu_bu(tc_vec);

                AVC_LPF_P0Q0(q0_org_h, p0_org_h, p1_org_h, q1_org_h,
                             neg_thresh_h, tc_h, p0_h, q0_h);
                LASX_PCKEV_B_2(p0_h, p0_h, q0_h, q0_h, p0_h, q0_h);
                p0_h = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than);
                q0_h = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than);
                LASX_ST_D(p0_h, 0, data - img_width);
                LASX_ST_D(q0_h, 0, data);
            }
        }
    }
}

#define AVC_LPF_P0P1P2_OR_Q0Q1Q2(p3_or_q3_org_in, p0_or_q0_org_in,          \
                                 q3_or_p3_org_in, p1_or_q1_org_in,          \
                                 p2_or_q2_org_in, q1_or_p1_org_in,          \
                                 p0_or_q0_out, p1_or_q1_out, p2_or_q2_out)  \
{                                                                           \
    __m256i threshold;                                                      \
    __m256i const2, const3 = __lasx_xvldi(0);                               \
                                                                            \
    const2 = __lasx_xvaddi_hu(const3, 2);                                   \
    const3 = __lasx_xvaddi_hu(const3, 3);                                   \
    threshold = __lasx_xvadd_h(p0_or_q0_org_in, q3_or_p3_org_in);           \
    threshold = __lasx_xvadd_h(p1_or_q1_org_in, threshold);                 \
                                                                            \
    p0_or_q0_out = __lasx_xvslli_h(threshold, 1);                           \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_out, p2_or_q2_org_in);           \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_out, q1_or_p1_org_in);           \
    p0_or_q0_out = __lasx_xvsrar_h(p0_or_q0_out, const3);                   \
                                                                            \
    p1_or_q1_out = __lasx_xvadd_h(p2_or_q2_org_in, threshold);              \
    p1_or_q1_out = __lasx_xvsrar_h(p1_or_q1_out, const2);                   \
                                                                            \
    p2_or_q2_out = __lasx_xvmul_h(p2_or_q2_org_in, const3);                 \
    p2_or_q2_out = __lasx_xvadd_h(p2_or_q2_out, p3_or_q3_org_in);           \
    p2_or_q2_out = __lasx_xvadd_h(p2_or_q2_out, p3_or_q3_org_in);           \
    p2_or_q2_out = __lasx_xvadd_h(p2_or_q2_out, threshold);                 \
    p2_or_q2_out = __lasx_xvsrar_h(p2_or_q2_out, const3);                   \
}

/* data[-u32_img_width] = (uint8_t)((2 * p1 + p0 + q1 + 2) >> 2); */
#define AVC_LPF_P0_OR_Q0(p0_or_q0_org_in, q1_or_p1_org_in,             \
                         p1_or_q1_org_in, p0_or_q0_out)                \
{                                                                      \
    __m256i const2 = __lasx_xvldi(0);                                  \
    const2 = __lasx_xvaddi_hu(const2, 2);                              \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_org_in, q1_or_p1_org_in);   \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_out, p1_or_q1_org_in);      \
    p0_or_q0_out = __lasx_xvadd_h(p0_or_q0_out, p1_or_q1_org_in);      \
    p0_or_q0_out = __lasx_xvsrar_h(p0_or_q0_out, const2);              \
}

void ff_h264_h_lpf_luma_intra_8_lasx(uint8_t *data, int img_width,
                                     int alpha_in, int beta_in)
{
    int img_width_2x = img_width << 1;
    uint8_t *src = data - 4;
    __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
    __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
    __m256i p3_org, p2_org, p1_org, p0_org, q0_org, q1_org, q2_org, q3_org;
    __m256i zero = __lasx_xvldi(0);

    {
        __m256i row0, row1, row2, row3, row4, row5, row6, row7;
        __m256i row8, row9, row10, row11, row12, row13, row14, row15;

        LASX_LD_8(src, img_width, row0, row1, row2, row3,
                  row4, row5, row6, row7);
        src += img_width << 3;
        LASX_LD_8(src, img_width, row8, row9, row10, row11,
                  row12, row13, row14, row15);

        LASX_TRANSPOSE16x8_B(row0, row1, row2, row3,
                             row4, row5, row6, row7,
                             row8, row9, row10, row11,
                             row12, row13, row14, row15,
                             p3_org, p2_org, p1_org, p0_org,
                             q0_org, q1_org, q2_org, q3_org);
    }

    alpha = __lasx_xvldrepl_b(&alpha_in, 0);
    beta  = __lasx_xvldrepl_b(&beta_in, 0);
    p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
    p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
    q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

    is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
    is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
    is_less_than       = is_less_than_beta & is_less_than_alpha;
    is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
    is_less_than       = is_less_than_beta & is_less_than;
    is_less_than       = __lasx_xvpermi_q(zero, is_less_than, 0x30);

    if (__lasx_xbnz_v(is_less_than)) {
        __m256i p2_asub_p0, q2_asub_q0, p0_h, q0_h, negate_is_less_than_beta;
        __m256i p1_org_h, p0_org_h, q0_org_h, q1_org_h;
        __m256i less_alpha_shift2_add2 = __lasx_xvsrli_b(alpha, 2);

        less_alpha_shift2_add2 = __lasx_xvaddi_bu(less_alpha_shift2_add2, 2);
        less_alpha_shift2_add2 = __lasx_xvslt_bu(p0_asub_q0, less_alpha_shift2_add2);

        p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
        p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
        q0_org_h = __lasx_vext2xv_hu_bu(q0_org);
        q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

        p2_asub_p0               = __lasx_xvabsd_bu(p2_org, p0_org);
        is_less_than_beta        = __lasx_xvslt_bu(p2_asub_p0, beta);
        is_less_than_beta        = is_less_than_beta & less_alpha_shift2_add2;
        negate_is_less_than_beta = __lasx_xvxori_b(is_less_than_beta, 0xff);
        is_less_than_beta        = is_less_than_beta & is_less_than;
        negate_is_less_than_beta = negate_is_less_than_beta & is_less_than;

        /* combine and store */
        if (__lasx_xbnz_v(is_less_than_beta)) {
            __m256i p2_org_h, p3_org_h, p1_h, p2_h;

            p2_org_h   = __lasx_vext2xv_hu_bu(p2_org);
            p3_org_h   = __lasx_vext2xv_hu_bu(p3_org);

            AVC_LPF_P0P1P2_OR_Q0Q1Q2(p3_org_h, p0_org_h, q0_org_h, p1_org_h,
                                     p2_org_h, q1_org_h, p0_h, p1_h, p2_h);

            LASX_PCKEV_B(p0_h, p0_h, p0_h);
            LASX_PCKEV_B_2(p1_h, p1_h, p2_h, p2_h, p1_h, p2_h);
            p0_org = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than_beta);
            p1_org = __lasx_xvbitsel_v(p1_org, p1_h, is_less_than_beta);
            p2_org = __lasx_xvbitsel_v(p2_org, p2_h, is_less_than_beta);
        }

        AVC_LPF_P0_OR_Q0(p0_org_h, q1_org_h, p1_org_h, p0_h);
        /* combine */
        LASX_PCKEV_B(p0_h, p0_h, p0_h);
        p0_org = __lasx_xvbitsel_v(p0_org, p0_h, negate_is_less_than_beta);

        /* if (tmpFlag && (unsigned)ABS(q2-q0) < thresholds->beta_in) */
        q2_asub_q0 = __lasx_xvabsd_bu(q2_org, q0_org);
        is_less_than_beta = __lasx_xvslt_bu(q2_asub_q0, beta);
        is_less_than_beta = is_less_than_beta & less_alpha_shift2_add2;
        negate_is_less_than_beta = __lasx_xvxori_b(is_less_than_beta, 0xff);
        is_less_than_beta = is_less_than_beta & is_less_than;
        negate_is_less_than_beta = negate_is_less_than_beta & is_less_than;

        /* combine and store */
        if (__lasx_xbnz_v(is_less_than_beta)) {
            __m256i q2_org_h, q3_org_h, q1_h, q2_h;

            q2_org_h   = __lasx_vext2xv_hu_bu(q2_org);
            q3_org_h   = __lasx_vext2xv_hu_bu(q3_org);

            AVC_LPF_P0P1P2_OR_Q0Q1Q2(q3_org_h, q0_org_h, p0_org_h, q1_org_h,
                                     q2_org_h, p1_org_h, q0_h, q1_h, q2_h);

            LASX_PCKEV_B(q0_h, q0_h, q0_h);
            LASX_PCKEV_B_2(q1_h, q1_h, q2_h, q2_h, q1_h, q2_h);
            q0_org = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than_beta);
            q1_org = __lasx_xvbitsel_v(q1_org, q1_h, is_less_than_beta);
            q2_org = __lasx_xvbitsel_v(q2_org, q2_h, is_less_than_beta);

        }

        AVC_LPF_P0_OR_Q0(q0_org_h, p1_org_h, q1_org_h, q0_h);

        /* combine */
        LASX_PCKEV_B(q0_h, q0_h, q0_h);
        q0_org = __lasx_xvbitsel_v(q0_org, q0_h, negate_is_less_than_beta);

        /* transpose and store */
        {
            __m256i row0, row1, row2, row3, row4, row5, row6, row7;

            LASX_ILVL_B_4(p1_org, p3_org, p0_org, p2_org, q2_org, q0_org, q3_org,
                          q1_org, row0, row1, row2, row3);
            LASX_ILVLH_B_2(row1, row0, row3, row2, row5, row4, row7, row6);
            LASX_ILVLH_W_2(row6, row4, row7, row5, row1, row0, row3, row2);
            LASX_PCKEV_Q_4(row0, row0, row1, row1, row2, row2, row3, row3,
                           row4, row5, row6, row7);
            src = data - 4;
            LASX_ST_D_2(row4, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row0, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row5, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row1, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row6, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row2, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row7, 2, 3, src, img_width);
            src += img_width_2x;
            LASX_ST_D_2(row3, 2, 3, src, img_width);
        }
    }
}

void ff_h264_v_lpf_luma_intra_8_lasx(uint8_t *data, int img_width,
                                     int alpha_in, int beta_in)
{
    int img_width_2x = img_width << 1;
    uint8_t *src = data - img_width_2x;
    __m256i p0_asub_q0, p1_asub_p0, q1_asub_q0, alpha, beta;
    __m256i is_less_than, is_less_than_beta, is_less_than_alpha;
    __m256i p1_org, p0_org, q0_org, q1_org;
    __m256i zero = __lasx_xvldi(0);

    LASX_LD_4(src, img_width, p1_org, p0_org, q0_org, q1_org)
    alpha = __lasx_xvldrepl_b(&alpha_in, 0);
    beta  = __lasx_xvldrepl_b(&beta_in, 0);
    p0_asub_q0 = __lasx_xvabsd_bu(p0_org, q0_org);
    p1_asub_p0 = __lasx_xvabsd_bu(p1_org, p0_org);
    q1_asub_q0 = __lasx_xvabsd_bu(q1_org, q0_org);

    is_less_than_alpha = __lasx_xvslt_bu(p0_asub_q0, alpha);
    is_less_than_beta  = __lasx_xvslt_bu(p1_asub_p0, beta);
    is_less_than       = is_less_than_beta & is_less_than_alpha;
    is_less_than_beta  = __lasx_xvslt_bu(q1_asub_q0, beta);
    is_less_than       = is_less_than_beta & is_less_than;
    is_less_than       = __lasx_xvpermi_q(zero, is_less_than, 0x30);

    if (__lasx_xbnz_v(is_less_than)) {
        __m256i p2_asub_p0, q2_asub_q0, p0_h, q0_h, negate_is_less_than_beta;
        __m256i p1_org_h, p0_org_h, q0_org_h, q1_org_h;
        __m256i p2_org = LASX_LD(src - img_width);
        __m256i q2_org = LASX_LD(data + img_width_2x);
        __m256i less_alpha_shift2_add2 = __lasx_xvsrli_b(alpha, 2);
        less_alpha_shift2_add2 = __lasx_xvaddi_bu(less_alpha_shift2_add2, 2);
        less_alpha_shift2_add2 = __lasx_xvslt_bu(p0_asub_q0, less_alpha_shift2_add2);

        p1_org_h = __lasx_vext2xv_hu_bu(p1_org);
        p0_org_h = __lasx_vext2xv_hu_bu(p0_org);
        q0_org_h = __lasx_vext2xv_hu_bu(q0_org);
        q1_org_h = __lasx_vext2xv_hu_bu(q1_org);

        p2_asub_p0               = __lasx_xvabsd_bu(p2_org, p0_org);
        is_less_than_beta        = __lasx_xvslt_bu(p2_asub_p0, beta);
        is_less_than_beta        = is_less_than_beta & less_alpha_shift2_add2;
        negate_is_less_than_beta = __lasx_xvxori_b(is_less_than_beta, 0xff);
        is_less_than_beta        = is_less_than_beta & is_less_than;
        negate_is_less_than_beta = negate_is_less_than_beta & is_less_than;

        /* combine and store */
        if (__lasx_xbnz_v(is_less_than_beta)) {
            __m256i p2_org_h, p3_org_h, p1_h, p2_h;
            __m256i p3_org = LASX_LD(src - img_width_2x);

            p2_org_h   = __lasx_vext2xv_hu_bu(p2_org);
            p3_org_h   = __lasx_vext2xv_hu_bu(p3_org);

            AVC_LPF_P0P1P2_OR_Q0Q1Q2(p3_org_h, p0_org_h, q0_org_h, p1_org_h,
                                     p2_org_h, q1_org_h, p0_h, p1_h, p2_h);

            LASX_PCKEV_B(p0_h, p0_h, p0_h);
            LASX_PCKEV_B_2(p1_h, p1_h, p2_h, p2_h, p1_h, p2_h);
            p0_org = __lasx_xvbitsel_v(p0_org, p0_h, is_less_than_beta);
            p1_org = __lasx_xvbitsel_v(p1_org, p1_h, is_less_than_beta);
            p2_org = __lasx_xvbitsel_v(p2_org, p2_h, is_less_than_beta);

            LASX_ST(p1_org, src);
            LASX_ST(p2_org, src - img_width);
        }

        AVC_LPF_P0_OR_Q0(p0_org_h, q1_org_h, p1_org_h, p0_h);
        /* combine */
        LASX_PCKEV_B(p0_h, p0_h, p0_h);
        p0_org = __lasx_xvbitsel_v(p0_org, p0_h, negate_is_less_than_beta);
        LASX_ST(p0_org, data - img_width);

        /* if (tmpFlag && (unsigned)ABS(q2-q0) < thresholds->beta_in) */
        q2_asub_q0 = __lasx_xvabsd_bu(q2_org, q0_org);
        is_less_than_beta = __lasx_xvslt_bu(q2_asub_q0, beta);
        is_less_than_beta = is_less_than_beta & less_alpha_shift2_add2;
        negate_is_less_than_beta = __lasx_xvxori_b(is_less_than_beta, 0xff);
        is_less_than_beta = is_less_than_beta & is_less_than;
        negate_is_less_than_beta = negate_is_less_than_beta & is_less_than;

        /* combine and store */
        if (__lasx_xbnz_v(is_less_than_beta)) {
            __m256i q2_org_h, q3_org_h, q1_h, q2_h;
            __m256i q3_org = LASX_LD(data + img_width_2x + img_width);

            q2_org_h   = __lasx_vext2xv_hu_bu(q2_org);
            q3_org_h   = __lasx_vext2xv_hu_bu(q3_org);

            AVC_LPF_P0P1P2_OR_Q0Q1Q2(q3_org_h, q0_org_h, p0_org_h, q1_org_h,
                                     q2_org_h, p1_org_h, q0_h, q1_h, q2_h);

            LASX_PCKEV_B(q0_h, q0_h, q0_h);
            LASX_PCKEV_B_2(q1_h, q1_h, q2_h, q2_h, q1_h, q2_h);
            q0_org = __lasx_xvbitsel_v(q0_org, q0_h, is_less_than_beta);
            q1_org = __lasx_xvbitsel_v(q1_org, q1_h, is_less_than_beta);
            q2_org = __lasx_xvbitsel_v(q2_org, q2_h, is_less_than_beta);

            LASX_ST(q1_org, data + img_width);
            LASX_ST(q2_org, data + img_width_2x);
        }

        AVC_LPF_P0_OR_Q0(q0_org_h, p1_org_h, q1_org_h, q0_h);

        /* combine */
        LASX_PCKEV_B(q0_h, q0_h, q0_h);
        q0_org = __lasx_xvbitsel_v(q0_org, q0_h, negate_is_less_than_beta);

        LASX_ST(q0_org, data);
    }
}
