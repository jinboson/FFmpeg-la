/*
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

#include "libavutil/loongarch/cpu.h"
#include "h264dsp_lasx.h"

av_cold void ff_h264dsp_init_loongarch(H264DSPContext *c, const int bit_depth,
                                       const int chroma_format_idc)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_lasx(cpu_flags)) {
        if (bit_depth == 8) {
            c->h264_v_loop_filter_luma = ff_h264_v_lpf_luma_8_lasx;
            c->h264_h_loop_filter_luma = ff_h264_h_lpf_luma_8_lasx;
            c->h264_v_loop_filter_luma_intra = ff_h264_v_lpf_luma_intra_8_lasx;
            c->h264_h_loop_filter_luma_intra = ff_h264_h_lpf_luma_intra_8_lasx;
//            c->h264_h_loop_filter_luma_mbaff = ff_h264_h_loop_filter_luma_mbaff_lasx;
//            c->h264_h_loop_filter_luma_mbaff_intra = ff_h264_h_loop_filter_luma_mbaff_intra_lasx;
            c->h264_v_loop_filter_chroma = ff_h264_v_lpf_chroma_8_lasx;

            if (chroma_format_idc <= 1)
                c->h264_h_loop_filter_chroma = ff_h264_h_lpf_chroma_8_lasx;
//            else
//                c->h264_h_loop_filter_chroma = ff_h264_h_loop_filter_chroma422_lasx;
//
//            if (chroma_format_idc > 1)
//                c->h264_h_loop_filter_chroma_mbaff = ff_h264_h_loop_filter_chroma422_mbaff_lasx;
//
//            c->h264_v_loop_filter_chroma_intra = ff_h264_v_lpf_chroma_intra_lasx;
//
//            if (chroma_format_idc <= 1)
//                c->h264_h_loop_filter_chroma_intra = ff_h264_h_lpf_chroma_intra_lasx;
        }
    }
}
