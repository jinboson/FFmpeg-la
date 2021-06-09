/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
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

#include "libavutil/loongarch/cpu.h"
#include "libavutil/attributes.h"
#include "libavcodec/vp9dsp.h"
#include "vp9dsp_loongarch.h"

#if HAVE_LSX
static av_cold void vp9dsp_mc_init_lsx(VP9DSPContext *dsp, int bpp)
{
    if (bpp == 8) {
#define init_subpel1(idx1, idx2, idxh, idxv, sz, dir, type)  \
    dsp->mc[idx1][FILTER_8TAP_SMOOTH ][idx2][idxh][idxv] =   \
        ff_##type##_8tap_smooth_##sz##dir##_lsx;             \
    dsp->mc[idx1][FILTER_8TAP_REGULAR][idx2][idxh][idxv] =   \
        ff_##type##_8tap_regular_##sz##dir##_lsx;            \
    dsp->mc[idx1][FILTER_8TAP_SHARP  ][idx2][idxh][idxv] =   \
        ff_##type##_8tap_sharp_##sz##dir##_lsx;

#define init_subpel2(idx, idxh, idxv, dir, type)      \
    init_subpel1(0, idx, idxh, idxv, 64, dir, type);  \
    init_subpel1(1, idx, idxh, idxv, 32, dir, type);  \
    init_subpel1(2, idx, idxh, idxv, 16, dir, type);  \
    init_subpel1(3, idx, idxh, idxv,  8, dir, type);  \
    init_subpel1(4, idx, idxh, idxv,  4, dir, type)

#define init_subpel3(idx, type)         \
    init_subpel2(idx, 1, 0, h, type)    \
    init_subpel2(idx, 0, 1, v, type);   \
    init_subpel2(idx, 1, 1, hv, type);

    init_subpel3(0, put);

#undef init_subpel1
#undef init_subpel2
#undef init_subpel3
    }
}

static av_cold void vp9dsp_init_lsx(VP9DSPContext *dsp, int bpp)
{
    vp9dsp_mc_init_lsx(dsp, bpp);
}
#endif /* #if HAVE_LSX */

av_cold void ff_vp9dsp_init_loongarch(VP9DSPContext *dsp, int bpp)
{
    int cpu_flags = av_get_cpu_flags();

    if (have_lsx(cpu_flags))
        vp9dsp_init_lsx(dsp, bpp);
}
