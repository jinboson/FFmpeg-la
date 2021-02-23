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

#ifndef AVCODEC_LOONGARCH_IDCTDSP_LOONGARCH_H
#define AVCODEC_LOONGARCH_IDCTDSP_LOONGARCH_H

#include <stdint.h>
#include <stddef.h>
#include "libavcodec/h264.h"

void ff_simple_idct_lasx(int16_t *block);
void ff_simple_idct_put_lasx(uint8_t *dest, ptrdiff_t stride_dst, int16_t *block);
void ff_simple_idct_add_lasx(uint8_t *dest, ptrdiff_t stride_dst, int16_t *block);

#endif /* AVCODEC_LOONGARCH_IDCTDSP_LOONGARCH_H */
