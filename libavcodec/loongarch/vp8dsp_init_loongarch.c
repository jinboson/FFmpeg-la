/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
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

/**
 * @file
 * VP8 compatible video decoder
 */

#include "libavutil/loongarch/cpu.h"
#include "libavcodec/vp8dsp.h"
#include "vp8dsp_loongarch.h"

av_cold void ff_vp8dsp_init_loongarch(VP8DSPContext *dsp)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_lsx(cpu_flags)) {
        dsp->vp8_v_loop_filter16y = ff_vp8_v_loop_filter16_lsx;
        dsp->vp8_h_loop_filter16y = ff_vp8_h_loop_filter16_lsx;
        dsp->vp8_v_loop_filter8uv = ff_vp8_v_loop_filter8uv_lsx;
        dsp->vp8_h_loop_filter8uv = ff_vp8_h_loop_filter8uv_lsx;
    }
}
