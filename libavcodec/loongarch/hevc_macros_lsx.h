/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
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

#ifndef AVCODEC_LOONGARCH_HEVC_MACROS_LSX_H
#define AVCODEC_LOONGARCH_HEVC_MACROS_LSX_H

static inline __m128i __lsx_hevc_filt_8tap_h(__m128i in0, __m128i in1, __m128i in2,
                                             __m128i in3, __m128i filt0, __m128i filt1,
                                             __m128i filt2, __m128i filt3)
{
    __m128i out_m;

    out_m = __lsx_vdp2_h_b(in0, filt0);
    out_m = __lsx_vdp2add_h_b(out_m, in1, filt1);
    out_m = __lsx_vdp2add_h_b(out_m, in2, filt2);
    out_m = __lsx_vdp2add_h_b(out_m, in3, filt3);
    return out_m;
}

static inline __m128i __lsx_hevc_filt_8tap_w(__m128i in0, __m128i in1, __m128i in2,
                                             __m128i in3, __m128i filt0, __m128i filt1,
                                             __m128i filt2, __m128i filt3)
{
    __m128i out_m;

    out_m = __lsx_vdp2_w_h(in0, filt0);
    out_m = __lsx_vdp2add_w_h(out_m, in1, filt1);
    out_m = __lsx_vdp2add_w_h(out_m, in2, filt2);
    out_m = __lsx_vdp2add_w_h(out_m, in3, filt3);
    return out_m;
}

static inline __m128i __lsx_hevc_filt_4tap_h(__m128i in0, __m128i in1, __m128i filt0,
                                             __m128i filt1)
{
    __m128i out_m;

    out_m = __lsx_vdp2_h_b(in0, filt0);
    out_m = __lsx_vdp2add_h_b(out_m, in1, filt1);
    return out_m;
}

static inline __m128i __lsx_hevc_filt_4tap_w(__m128i in0, __m128i in1, __m128i filt0,
                                             __m128i filt1)
{
    __m128i out_m;

    out_m = __lsx_vdp2_w_h(in0, filt0);
    out_m = __lsx_vdp2add_w_h(out_m, in1, filt1);
    return out_m;
}

#endif  /* AVCODEC_LOONGARCH_HEVC_MACROS_LSX_H */
