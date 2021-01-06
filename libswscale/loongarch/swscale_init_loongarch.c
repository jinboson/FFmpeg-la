/*
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 * Contributed by Hao Chen(chenhao@loongson.cn)
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

#include "swscale_loongarch.h"
#include "libavutil/loongarch/cpu.h"

av_cold void ff_sws_init_swscale_loongarch(SwsContext *c)
{
    int cpu_flags = av_get_cpu_flags();
    if (have_lasx(cpu_flags)) {
        if (c->srcBpc == 8) {
            if (c->dstBpc <= 14) {
                c->hyScale = c->hcScale = ff_hscale_8_to_15_lasx;
            } else {
                c->hyScale = c->hcScale = ff_hscale_8_to_19_lasx;
            }
        } else {
            c->hyScale = c->hcScale = c->dstBpc > 14 ? ff_hscale_16_to_19_lasx
                                                     : ff_hscale_16_to_15_lasx;
        }
    }
}
