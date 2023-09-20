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

#ifndef SWSCALE_MIPS_RGB2RGB_MIPS_H
#define SWSCALE_MIPS_RGB2RGB_MIPS_H

#include "libswscale/rgb2rgb.h"

void ff_interleave_bytes_msa(const uint8_t *src1, const uint8_t *src2,
                             uint8_t *dest, int width, int height,
                             int src1Stride, int src2Stride, int dstStride);

#endif /* SWSCALE_MIPS_RGB2RGB_MIPS_H */
