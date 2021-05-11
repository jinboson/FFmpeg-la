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

#ifndef AVCODEC_LOONGARCH_HEVCDSP_LSX_H
#define AVCODEC_LOONGARCH_HEVCDSP_LSX_H

#include "libavcodec/hevcdsp.h"

#define MC(PEL, DIR, WIDTH)                                               \
void ff_hevc_put_hevc_##PEL##_##DIR##WIDTH##_8_lsx(int16_t *dst,          \
                                                   uint8_t *src,          \
                                                   ptrdiff_t src_stride,  \
                                                   int height,            \
                                                   intptr_t mx,           \
                                                   intptr_t my,           \
                                                   int width)

MC(pel, pixels, 4);
MC(pel, pixels, 6);
MC(pel, pixels, 8);
MC(pel, pixels, 12);
MC(pel, pixels, 16);
MC(pel, pixels, 24);
MC(pel, pixels, 32);
MC(pel, pixels, 48);
MC(pel, pixels, 64);

#undef MC

#endif  // #ifndef AVCODEC_LOONGARCH_HEVCDSP_LSX_H
