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

#ifndef AVUTIL_LOONGARCH_INTMATH_H
#define AVUTIL_LOONGARCH_INTMATH_H

#include <stdint.h>
#include <stdlib.h>
#include "config.h"

#if HAVE_FAST_CLZ

static av_always_inline unsigned ff_loongarch_clz(unsigned _1)
{
    unsigned out;
    __asm__ volatile (
    "clz.w   %[out],  %[in]  \n\t"
    : [out]"=&r"(out)
    : [in]"r"(_1)
    );
    return out;
}

static av_always_inline int ff_loongarch_ctz_w(int _1)
{
    int out;
    __asm__ volatile (
    "ctz.w   %[out],  %[in]        \n\t"
    "andi    %[out],  %[out],  31  \n\t"
    : [out]"=&r"(out)
    : [in]"r"(_1)
    );
    return out;
}

static av_always_inline int ff_loongarch_ctz_d(long long _1)
{
    int out;
    __asm__ volatile (
    "ctz.d   %[out],  %[in]        \n\t"
    "andi    %[out],  %[out],  63  \n\t"
    : [out]"=&r"(out)
    : [in]"r"(_1)
    );
    return out;
}

#define ff_log2(x) (31 - ff_loongarch_clz((x)|1))

#define ff_clz(x) ff_loongarch_clz(x)
#define ff_ctz(x) ff_loongarch_ctz_w(x)
#define ff_ctzll(x)  ff_loongarch_ctz_d(x)

#endif /* HAVE_FAST_CLZ */
#endif /* AVUTIL_LOONGARCH_INTMATH_H */
