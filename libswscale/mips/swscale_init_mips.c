/*
 * Copyright (C) 2020 Loongson Technology Co. Ltd.
 * Contributed by Gu Xiwei(guxiwei-hf@loongson.cn)
 * All rights reserved.
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

#include "swscale_mips.h"
#include "libavutil/mips/cpu.h"

av_cold void ff_sws_init_swscale_mips(SwsContext *c)
{
    int cpu_flags = av_get_cpu_flags();
#if HAVE_MSA
    if (have_msa(cpu_flags)) {
        if (c->srcBpc == 8 && c->dstBpc <= 14)
            c->hyScale = c->hcScale = ff_hscale_8_to_15_msa;
        if (c->dstBpc == 8)
            c->yuv2planeX = ff_yuv2planeX_8_msa;
    }
#endif /* #if HAVE_MSA */
}
