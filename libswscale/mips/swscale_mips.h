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

#ifndef SWSCALE_MIPS_SWSCALE_MIPS_H
#define SWSCALE_MIPS_SWSCALE_MIPS_H

#include "libswscale/swscale.h"
#include "libswscale/swscale_internal.h"

void ff_hscale_8_to_15_msa(SwsContext *c, int16_t *dst, int dstW,
                           const uint8_t *src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize);

void ff_yuv2planeX_8_msa(const int16_t *filter, int filterSize,
                         const int16_t **src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

#endif /* SWSCALE_MIPS_SWSCALE_MIPS_H */
