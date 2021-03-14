/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 * Contributed by Shiyou Yin <yinshiyou-hf@loongson.cn>
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

#include "libavutil/loongarch/generic_macros_lasx.h"
#include "libavcodec/loongarch/hpeldsp_lasx.h"

static inline void
put_pixels8_l2_8_lsx(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
                     int dst_stride, int src_stride1, int src_stride2, int h)
{
    __asm__ volatile (
        "1:                                       \n\t"
        "vld      $vr0,    %[src1], 0             \n\t"
        "add.d    %[src1], %[src1], %[srcStride1] \n\t"
        "vld      $vr1,    %[src1], 0             \n\t"
        "add.d    %[src1], %[src1], %[srcStride1] \n\t"
        "vld      $vr2,    %[src1], 0             \n\t"
        "add.d    %[src1], %[src1], %[srcStride1] \n\t"
        "vld      $vr3,    %[src1], 0             \n\t"
        "add.d    %[src1], %[src1], %[srcStride1] \n\t"

        "vld      $vr4,    %[src2], 0             \n\t"
        "add.d    %[src2], %[src2], %[srcStride2] \n\t"
        "vld      $vr5,    %[src2], 0             \n\t"
        "add.d    %[src2], %[src2], %[srcStride2] \n\t"
        "vld      $vr6,    %[src2], 0             \n\t"
        "add.d    %[src2], %[src2], %[srcStride2] \n\t"
        "vld      $vr7,    %[src2], 0             \n\t"
        "add.d    %[src2], %[src2], %[srcStride2] \n\t"

        "addi.d   %[h],    %[h],    -4            \n\t"

        "vavgr.bu $vr0,    $vr4,    $vr0          \n\t"
        "vavgr.bu $vr1,    $vr5,    $vr1          \n\t"
        "vavgr.bu $vr2,    $vr6,    $vr2          \n\t"
        "vavgr.bu $vr3,    $vr7,    $vr3          \n\t"
        "vstelm.d $vr0,    %[dst],  0,  0         \n\t"
        "add.d    %[dst],  %[dst],  %[dstStride]  \n\t"
        "vstelm.d $vr1,    %[dst],  0,  0         \n\t"
        "add.d    %[dst],  %[dst],  %[dstStride]  \n\t"
        "vstelm.d $vr2,    %[dst],  0,  0         \n\t"
        "add.d    %[dst],  %[dst],  %[dstStride]  \n\t"
        "vstelm.d $vr3,    %[dst],  0,  0         \n\t"
        "add.d    %[dst],  %[dst],  %[dstStride]  \n\t"
        "bnez     %[h],             1b            \n\t"

        : [dst]"+&r"(dst), [src2]"+&r"(src2), [src1]"+&r"(src1),
          [h]"+&r"(h)
        : [dstStride]"r"(dst_stride), [srcStride1]"r"(src_stride1),
          [srcStride2]"r"(src_stride2)
        : "memory"
    );
}

static inline void
put_pixels16_l2_8_lsx(uint8_t *dst, const uint8_t *src1, const uint8_t *src2,
                      int dst_stride, int src_stride1, int src_stride2, int h)
{
    __asm__ volatile (
        "1:                                      \n\t"
        "vld     $vr0,    %[src1], 0             \n\t"
        "add.d   %[src1], %[src1], %[srcStride1] \n\t"
        "vld     $vr1,    %[src1], 0             \n\t"
        "add.d   %[src1], %[src1], %[srcStride1] \n\t"
        "vld     $vr2,    %[src1], 0             \n\t"
        "add.d   %[src1], %[src1], %[srcStride1] \n\t"
        "vld     $vr3,    %[src1], 0             \n\t"
        "add.d   %[src1], %[src1], %[srcStride1] \n\t"

        "vld     $vr4,    %[src2], 0             \n\t"
        "add.d   %[src2], %[src2], %[srcStride2] \n\t"
        "vld     $vr5,    %[src2], 0             \n\t"
        "add.d   %[src2], %[src2], %[srcStride2] \n\t"
        "vld     $vr6,    %[src2], 0             \n\t"
        "add.d   %[src2], %[src2], %[srcStride2] \n\t"
        "vld     $vr7,    %[src2], 0             \n\t"
        "add.d   %[src2], %[src2], %[srcStride2] \n\t"

        "addi.d  %[h],    %[h],    -4            \n\t"

        "vavgr.bu $vr0,   $vr4,    $vr0          \n\t"
        "vavgr.bu $vr1,   $vr5,    $vr1          \n\t"
        "vavgr.bu $vr2,   $vr6,    $vr2          \n\t"
        "vavgr.bu $vr3,   $vr7,    $vr3          \n\t"
        "vst     $vr0,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr1,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr2,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "vst     $vr3,    %[dst],  0             \n\t"
        "add.d   %[dst],  %[dst],  %[dstStride]  \n\t"
        "bnez    %[h],             1b            \n\t"

        : [dst]"+&r"(dst), [src2]"+&r"(src2), [src1]"+&r"(src1),
          [h]"+&r"(h)
        : [dstStride]"r"(dst_stride), [srcStride1]"r"(src_stride1),
          [srcStride2]"r"(src_stride2)
        : "memory"
    );
}

void ff_put_pixels8_8_lasx(uint8_t *block, const uint8_t *pixels,
                           ptrdiff_t line_size, int h)
{
    double tmp[8];
    int h_8 = h >> 3;
    int res = h & 7;

    __asm__ volatile (
        "beqz       %[h_8],                2f          \n\t"
        "1:                                            \n\t"
        "ld.d       %[tmp0],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp1],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp2],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp3],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp4],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp5],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp6],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "ld.d       %[tmp7],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"

        "addi.d     %[h_8],     %[h_8],    -1          \n\t"

        "st.d       %[tmp0],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp1],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp2],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp3],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp4],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp5],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp6],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "st.d       %[tmp7],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "bnez       %[h_8],     1b                     \n\t"

        "2:                                            \n\t"
        "beqz       %[res],     4f                     \n\t"
        "3:                                            \n\t"
        "ld.d       %[tmp0],    %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "addi.d     %[res],     %[res],    -1          \n\t"
        "st.d       %[tmp0],    %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "bnez       %[res],     3b                     \n\t"
        "4:                                            \n\t"
        : [tmp0]"=&r"(tmp[0]),        [tmp1]"=&r"(tmp[1]),
          [tmp2]"=&r"(tmp[2]),        [tmp3]"=&r"(tmp[3]),
          [tmp4]"=&r"(tmp[4]),        [tmp5]"=&r"(tmp[5]),
          [tmp6]"=&r"(tmp[6]),        [tmp7]"=&r"(tmp[7]),
          [dst]"+&r"(block),          [src]"+&r"(pixels),
          [h_8]"+&r"(h_8),            [res]"+&r"(res)
        : [stride]"r"(line_size)
        : "memory"
    );
}

void ff_put_pixels16_8_lsx(uint8_t *block, const uint8_t *pixels,
                           ptrdiff_t line_size, int h)
{
    int h_8 = h >> 3;
    int res = h & 7;

    __asm__ volatile (
        "beqz       %[h_8],     2f                     \n\t"
        "1:                                            \n\t"
        "vld        $vr0,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr1,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr2,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr3,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr4,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr5,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr6,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "vld        $vr7,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"

        "addi.d     %[h_8],     %[h_8],    -1          \n\t"

        "vst        $vr0,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr1,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr2,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr3,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr4,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr5,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr6,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "vst        $vr7,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "bnez       %[h_8],     1b                     \n\t"

        "2:                                            \n\t"
        "beqz       %[res],     4f                     \n\t"
        "3:                                            \n\t"
        "vld        $vr0,       %[src],    0x0         \n\t"
        "add.d      %[src],     %[src],    %[stride]   \n\t"
        "addi.d     %[res],     %[res],    -1          \n\t"
        "vst        $vr0,       %[dst],    0x0         \n\t"
        "add.d      %[dst],     %[dst],    %[stride]   \n\t"
        "bnez       %[res],     3b                     \n\t"
        "4:                                            \n\t"
        : [dst]"+&r"(block),          [src]"+&r"(pixels),
          [h_8]"+&r"(h_8),            [res]"+&r"(res)
        : [stride]"r"(line_size)
        : "memory"
    );
}

void ff_put_pixels8_x2_8_lasx(uint8_t *block, const uint8_t *pixels,
                              ptrdiff_t line_size, int h)
{
    put_pixels8_l2_8_lsx(block, pixels, pixels + 1, line_size, line_size,
                         line_size, h);
}

void ff_put_pixels8_y2_8_lasx(uint8_t *block, const uint8_t *pixels,
                              ptrdiff_t line_size, int h)
{
    put_pixels8_l2_8_lsx(block, pixels, pixels + line_size, line_size,
                         line_size, line_size, h);
}

void ff_put_pixels16_x2_8_lasx(uint8_t *block, const uint8_t *pixels,
                               ptrdiff_t line_size, int h)
{
    put_pixels16_l2_8_lsx(block, pixels, pixels + 1, line_size, line_size,
                          line_size, h);
}

void ff_put_pixels16_y2_8_lasx(uint8_t *block, const uint8_t *pixels,
                               ptrdiff_t line_size, int h)
{
    put_pixels16_l2_8_lsx(block, pixels, pixels + line_size, line_size,
                          line_size, line_size, h);
}
