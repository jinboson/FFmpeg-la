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
#include "libavutil/mips/generic_macros_msa.h"
#include "libavutil/intreadwrite.h"

void ff_hscale_8_to_15_msa(SwsContext *c, int16_t *dst, int dstW,
                           const uint8_t *src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize)
{
    int i;

    if (filterSize == 8) {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };

            src0 = LD_V(v8i16, src + filterPos[i]);
            filter0 = LD_V(v8i16, filter + (i << 3));
            src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
            out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
            out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
            val += (__msa_copy_s_w((v4i32)out0, 0) +
                    __msa_copy_s_w((v4i32)out0, 2));
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else if (filterSize == 4) {
        int len = dstW & (~1);

        for (i = 0; i < len; i += 2) {
            v8i16 src1, src2, src3;
            v8i16 filter0;
            v4i32 out0;
            int val1 = 0;
            int val2 = 0;
            v16i8 zero = {0};

            src1 = LD_V(v8i16, src + filterPos[i]);
            src2 = LD_V(v8i16, src + filterPos[i + 1]);
            filter0 = LD_V(v8i16, filter + (i << 2));
            src1 = (v8i16)__msa_ilvr_b(zero, (v16i8)src1);
            src2 = (v8i16)__msa_ilvr_b(zero, (v16i8)src2);
            src3 = (v8i16)__msa_ilvr_d((v2i64)src2, (v2i64)src1);
            out0 = (v4i32)__msa_dotp_s_w(src3, filter0);
            val1 = __msa_copy_s_w(out0, 0) + __msa_copy_s_w(out0, 1);
            val2 = __msa_copy_s_w(out0, 2) + __msa_copy_s_w(out0, 3);
            dst[i] = FFMIN(val1 >> 7, (1 << 15) - 1);
            dst[i + 1] = FFMIN(val2 >> 7, (1 << 15) - 1);
        }
        if (i < dstW) {
           int val = 0;
           uint8_t *srcPos = src + filterPos[i];
           int16_t *filterStart = filter + filterSize * i;

           for (int j = 0; j < 4; j++) {
               val += ((int)srcPos[j]) * filterStart[j];
           }
           dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else if (filterSize > 8) {
        int len  = filterSize >> 3;
        int part = len << 3;

        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;
            int j, val = 0;

            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + (j << 3));
                filter0 = LD_V(v8i16, filterStart + (j << 3));
                src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
                out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
                out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
                val += (__msa_copy_s_w((v4i32)out0, 0) +
                        __msa_copy_s_w((v4i32)out0, 2));
            }
            for (j = part; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;

            for (int j = 0; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 7, (1 << 15) - 1);
        }
    }
}

void ff_hscale_8_to_19_msa(SwsContext *c, int16_t *_dst, int dstW,
                           const uint8_t *src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize)
{
    int i;
    int32_t *dst = (int32_t *) _dst;

    if (filterSize == 8) {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };

            src0 = LD_V(v8i16, src + filterPos[i]);
            filter0 = LD_V(v8i16, filter + (i << 3));
            src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
            out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
            out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
            val += (__msa_copy_s_w((v4i32)out0, 0) +
                    __msa_copy_s_w((v4i32)out0, 2));
            dst[i] = FFMIN(val >> 3, (1 << 19) - 1);
        }
    } else if (filterSize == 4) {
        int len = dstW & (~1);

        for (i = 0; i < len; i += 2) {
            v8i16 src1, src2, src3;
            v8i16 filter0;
            v4i32 out0;
            int val1 = 0;
            int val2 = 0;
            v16i8 zero = {0};

            src1 = LD_V(v8i16, src + filterPos[i]);
            src2 = LD_V(v8i16, src + filterPos[i + 1]);
            filter0 = LD_V(v8i16, filter + (i << 2));
            src1 = (v8i16)__msa_ilvr_b(zero, (v16i8)src1);
            src2 = (v8i16)__msa_ilvr_b(zero, (v16i8)src2);
            src3 = (v8i16)__msa_ilvr_d((v2i64)src2, (v2i64)src1);
            out0 = (v4i32)__msa_dotp_s_w(src3, filter0);
            val1 = __msa_copy_s_w(out0, 0) + __msa_copy_s_w(out0, 1);
            val2 = __msa_copy_s_w(out0, 2) + __msa_copy_s_w(out0, 3);
            dst[i] = FFMIN(val1 >> 3, (1 << 19) - 1);
            dst[i + 1] = FFMIN(val2 >> 3, (1 << 19) - 1);
        }
        if (i < dstW) {
           int val = 0;
           uint8_t *srcPos = src + filterPos[i];
           int16_t *filterStart = filter + filterSize * i;

           for (int j = 0; j < 4; j++) {
               val += ((int)srcPos[j]) * filterStart[j];
           }
           dst[i] = FFMIN(val >> 3, (1 << 19) - 1);
        }
    } else if (filterSize > 8) {
        int len  = filterSize >> 3;
        int part = len << 3;

        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0, out0;
            v16i8 zero = { 0 };
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;
            int j, val = 0;

            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + (j << 3));
                filter0 = LD_V(v8i16, filterStart + (j << 3));
                src0 = (v8i16)__msa_ilvr_b(zero, (v16i8)src0);
                out0 = (v8i16)__msa_dotp_s_w(src0, filter0);
                out0 = (v8i16)__msa_hadd_s_d((v4i32)out0, (v4i32)out0);
                val += (__msa_copy_s_w((v4i32)out0, 0) +
                        __msa_copy_s_w((v4i32)out0, 2));
            }
            for (j = part; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 3, (1 << 19) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            uint8_t *srcPos = src + filterPos[i];
            int16_t *filterStart = filter + filterSize * i;

            for (int j = 0; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> 3, (1 << 19) - 1);
        }
    }
}

void ff_hscale_16_to_19_msa(SwsContext *c, int16_t *_dst, int dstW,
                            const uint8_t *_src, const int16_t *filter,
                            const int32_t *filterPos, int filterSize)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(c->srcFormat);
    int i;
    int32_t *dst        = (int32_t *) _dst;
    const uint16_t *src = (const uint16_t *) _src;
    int bits            = desc->comp[0].depth - 1;
    int sh              = bits - 4;

    if ((isAnyRGB(c->srcFormat) || c->srcFormat == AV_PIX_FMT_PAL8)
         && desc->comp[0].depth<16) {
        sh = 9;
    } else if (desc->flags & AV_PIX_FMT_FLAG_FLOAT) {
        sh = 11;
    }
    if (filterSize == 8) {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            v8i16 src0, filter0;
            v4i32 src_l, src_r, filter_l, filter_r, out_l, out_r, out;
            v8i16 zero = { 0 };

            src0 = LD_V(v8i16, src + filterPos[i]);
            filter0 = LD_V(v8i16, filter + (i << 3));
            src_r = (v4i32)__msa_ilvr_h(zero, (v8i16)src0);
            src_l = (v4i32)__msa_ilvl_h(zero, (v8i16)src0);
            UNPCK_SH_SW(filter0, filter_r, filter_l);
            out_r = (v4i32)__msa_dotp_s_d(src_r, filter_r);
            out_l = (v4i32)__msa_dotp_s_d(src_l, filter_l);
            out   = (v4i32)__msa_addv_w(out_r, out_l);
            val += (__msa_copy_s_w(out, 0) +
                    __msa_copy_s_w(out, 2));
            dst[i] = FFMIN(val >> sh, (1 << 19) - 1);
        }
    } else if (filterSize == 4) {
        int len = dstW & (~1);

        for (i = 0; i < len; i += 2) {
            v8i16 src1, src2, filter0;
            v4i32 src1_r, src2_r, filter_r, filter_l;
            v4i32 out1, out2;
            int val1 = 0;
            int val2 = 0;
            v8i16 zero = {0};

            src1     = LD_V(v8i16, src + filterPos[i]);
            src2     = LD_V(v8i16, src + filterPos[i + 1]);
            filter0  = LD_V(v8i16, filter + (i << 2));
            src1_r   = (v4i32)__msa_ilvr_h(zero, src1);
            src2_r   = (v4i32)__msa_ilvr_h(zero, src2);
            UNPCK_SH_SW(filter0, filter_r, filter_l);
            out1     = (v4i32)__msa_dotp_s_d(src1_r, filter_r);
            out2     = (v4i32)__msa_dotp_s_d(src2_r, filter_l);
            val1     = __msa_copy_s_w(out1, 0) + __msa_copy_s_w(out1, 2);
            val2     = __msa_copy_s_w(out2, 0) + __msa_copy_s_w(out2, 2);
            dst[i]   = FFMIN(val1 >> sh, (1 << 19) - 1);
            dst[i + 1] = FFMIN(val2 >> sh, (1 << 19) - 1);
        }
        if (i < dstW) {
           int val = 0;
           uint8_t *srcPos = src + filterPos[i];
           int16_t *filterStart = filter + filterSize * i;

           for (int j = 0; j < 4; j++) {
               val += ((int)srcPos[j]) * filterStart[j];
           }
           dst[i] = FFMIN(val >> sh, (1 << 19) - 1);
        }
    } else if (filterSize > 8) {
        int len  = filterSize >> 3;
        int part = len << 3;

        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0;
            v4i32 src_r, src_l, filter_r, filter_l, out_r, out_l, out;
            v8i16 zero = { 0 };
            uint16_t *srcPos = src + filterPos[i];
            int16_t  *filterStart = filter + filterSize * i;
            int j, val = 0;

            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + (j << 3));
                filter0 = LD_V(v8i16, filterStart + (j << 3));
                src_r = (v4i32)__msa_ilvr_h(zero, (v8i16)src0);
                src_l = (v4i32)__msa_ilvl_h(zero, (v8i16)src0);
                UNPCK_SH_SW(filter0, filter_r, filter_l);
                out_r = (v4i32)__msa_dotp_s_d(src_r, filter_r);
                out_l = (v4i32)__msa_dotp_s_d(src_l, filter_l);
                out   = (v4i32)__msa_addv_w(out_r, out_l);
                val  += (__msa_copy_s_w(out, 0) +
                         __msa_copy_s_w(out, 2));
            }
            for (j = part; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> sh, (1 << 19) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            uint16_t *srcPos = src + filterPos[i];
            int16_t  *filterStart = filter + filterSize * i;

            for (int j = 0; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> sh, (1 << 19) - 1);
        }
    }
}

void ff_hscale_16_to_15_msa(SwsContext *c, int16_t *dst, int dstW,
                           const uint8_t *_src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize)
{
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(c->srcFormat);
    int i;
    const uint16_t *src = (const uint16_t *) _src;
    int sh              = desc->comp[0].depth - 1;

    if (sh < 15) {
        sh = isAnyRGB(c->srcFormat) || c->srcFormat==AV_PIX_FMT_PAL8 ? 13 :
                      (desc->comp[0].depth - 1);
    } else if (desc->flags && AV_PIX_FMT_FLAG_FLOAT) {
        sh = 15;
    }
    if (filterSize == 8) {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            v8i16 src0, filter0;
            v4i32 src_l, src_r, filter_l, filter_r, out_l, out_r, out;
            v8i16 zero = { 0 };

            src0 = LD_V(v8i16, src + filterPos[i]);
            filter0 = LD_V(v8i16, filter + (i << 3));
            src_r = (v4i32)__msa_ilvr_h(zero, (v8i16)src0);
            src_l = (v4i32)__msa_ilvl_h(zero, (v8i16)src0);
            UNPCK_SH_SW(filter0, filter_r, filter_l);
            out_r = (v4i32)__msa_dotp_s_d(src_r, filter_r);
            out_l = (v4i32)__msa_dotp_s_d(src_l, filter_l);
            out   = (v4i32)__msa_addv_w(out_r, out_l);
            val += (__msa_copy_s_w(out, 0) +
                    __msa_copy_s_w(out, 2));
            dst[i] = FFMIN(val >> sh, (1 << 15) - 1);
        }
    } else if (filterSize == 4) {
        int len = dstW & (~1);

        for (i = 0; i < len; i += 2) {
            v8i16 src1, src2, filter0;
            v4i32 src1_r, src2_r, filter_r, filter_l;
            v4i32 out1, out2;
            int val1 = 0;
            int val2 = 0;
            v8i16 zero = {0};

            src1     = LD_V(v8i16, src + filterPos[i]);
            src2     = LD_V(v8i16, src + filterPos[i + 1]);
            filter0  = LD_V(v8i16, filter + (i << 2));
            src1_r   = (v4i32)__msa_ilvr_h(zero, src1);
            src2_r   = (v4i32)__msa_ilvr_h(zero, src2);
            UNPCK_SH_SW(filter0, filter_r, filter_l);
            out1     = (v4i32)__msa_dotp_s_d(src1_r, filter_r);
            out2     = (v4i32)__msa_dotp_s_d(src2_r, filter_l);
            val1     = __msa_copy_s_w(out1, 0) + __msa_copy_s_w(out1, 2);
            val2     = __msa_copy_s_w(out2, 0) + __msa_copy_s_w(out2, 2);
            dst[i]   = FFMIN(val1 >> sh, (1 << 15) - 1);
            dst[i + 1] = FFMIN(val2 >> sh, (1 << 15) - 1);
        }
        if (i < dstW) {
           int val = 0;
           uint8_t *srcPos = src + filterPos[i];
           int16_t *filterStart = filter + filterSize * i;

           for (int j = 0; j < 4; j++) {
               val += ((int)srcPos[j]) * filterStart[j];
           }
           dst[i] = FFMIN(val >> sh, (1 << 15) - 1);
        }
    } else if (filterSize > 8) {
        int len  = filterSize >> 3;
        int part = len << 3;

        for (i = 0; i < dstW; i++) {
            v8i16 src0, filter0;
            v4i32 src_r, src_l, filter_r, filter_l, out_r, out_l, out;
            v8i16 zero = { 0 };
            uint16_t *srcPos = src + filterPos[i];
            int16_t  *filterStart = filter + filterSize * i;
            int j, val = 0;

            for (j = 0; j < len; j++) {
                src0 = LD_V(v8i16, srcPos + (j << 3));
                filter0 = LD_V(v8i16, filterStart + (j << 3));
                src_r = (v4i32)__msa_ilvr_h(zero, (v8i16)src0);
                src_l = (v4i32)__msa_ilvl_h(zero, (v8i16)src0);
                UNPCK_SH_SW(filter0, filter_r, filter_l);
                out_r = (v4i32)__msa_dotp_s_d(src_r, filter_r);
                out_l = (v4i32)__msa_dotp_s_d(src_l, filter_l);
                out   = (v4i32)__msa_addv_w(out_r, out_l);
                val  += (__msa_copy_s_w(out, 0) +
                         __msa_copy_s_w(out, 2));
            }
            for (j = part; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> sh, (1 << 15) - 1);
        }
    } else {
        for (i = 0; i < dstW; i++) {
            int val = 0;
            uint16_t *srcPos = src + filterPos[i];
            int16_t  *filterStart = filter + filterSize * i;

            for (int j = 0; j < filterSize; j++) {
                val += ((int)srcPos[j]) * filterStart[j];
            }
            dst[i] = FFMIN(val >> sh, (1 << 15) - 1);
        }
    }
}
void ff_yuv2planeX_8_msa(const int16_t *filter, int filterSize,
                         const int16_t **src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset)
{
    int i;
    int len  = dstW >> 3;
    int part = len << 3;
    for (i = 0; i < len; i++) {
        int j;
        v8i16 src0, flags;
        v4i32 src_l, src_r, filter0;
        v4i32 val_r = { dither[(i * 8 + 0 + offset) & 7] << 12,
                        dither[(i * 8 + 1 + offset) & 7] << 12,
                        dither[(i * 8 + 2 + offset) & 7] << 12,
                        dither[(i * 8 + 3 + offset) & 7] << 12 };
        v4i32 val_l = { dither[(i * 8 + 4 + offset) & 7] << 12,
                        dither[(i * 8 + 5 + offset) & 7] << 12,
                        dither[(i * 8 + 6 + offset) & 7] << 12,
                        dither[(i * 8 + 7 + offset) & 7] << 12 };
        v8i16 zero = { 0 };

        for (j = 0; j < filterSize; j++) {
            src0 = LD_V(v8i16, &src[j][i * 8]);
            filter0 = __msa_fill_w(filter[j]);
            flags = __msa_clt_s_h(src0, zero);
            ILVRL_H2_SW(flags, src0, src_r, src_l);
            val_r += src_r * filter0;
            val_l += src_l * filter0;
        }
        val_r >>= 19;
        val_l >>= 19;
        CLIP_SW2_0_255(val_r, val_l);
        src0 = __msa_pckev_h((v8i16)val_l, (v8i16)val_r);
        src0 = (v8i16)__msa_pckev_b((v16i8)src0, (v16i8)src0);
        SD(__msa_copy_s_d((v2i64)src0, 0), dest + i * 8);
    }
    for (i = part; i < dstW; i++) {
        int val = dither[(i + offset) & 7] << 12;
        int j;
        for (j = 0; j< filterSize; j++)
            val += src[j][i] * filter[j];

        dest[i] = av_clip_uint8(val >> 19);
    }
}

/*Copy from libswscale/output.c*/
static av_always_inline void
yuv2rgb_write(uint8_t *_dest, int i, int Y1, int Y2,
              unsigned A1, unsigned A2,
              const void *_r, const void *_g, const void *_b, int y,
              enum AVPixelFormat target, int hasAlpha)
{
    if (target == AV_PIX_FMT_ARGB || target == AV_PIX_FMT_RGBA ||
        target == AV_PIX_FMT_ABGR || target == AV_PIX_FMT_BGRA) {
        uint32_t *dest = (uint32_t *) _dest;
        const uint32_t *r = (const uint32_t *) _r;
        const uint32_t *g = (const uint32_t *) _g;
        const uint32_t *b = (const uint32_t *) _b;

#if CONFIG_SMALL
        int sh = hasAlpha ? ((target ==AV_PIX_FMT_RGB32_1 || target == AV_PIX_FMT_BGR32_1) ? 0 : 24) : 0;
        dest[i * 2 + 0] = r[Y1] + g[Y1] + b[Y1];
        dest[i * 2 + 1] = r[Y2] + g[Y2] + b[Y2];
#else
#if defined(ASSERT_LEVEL) && ASSERT_LEVEL > 1
        int sh = (target == AV_PIX_FMT_RGB32_1 ||
                  target == AV_PIX_FMT_BGR32_1) ? 0 : 24;
        av_assert2((((r[Y1] + g[Y1] + b[Y1]) >> sh) & 0xFF) == 0xFF);
#endif
        dest[i * 2 + 0] = r[Y1] + g[Y1] + b[Y1];
        dest[i * 2 + 1] = r[Y2] + g[Y2] + b[Y2];
#endif

    } else if (target == AV_PIX_FMT_RGB565 || target == AV_PIX_FMT_BGR565 ||
               target == AV_PIX_FMT_RGB555 || target == AV_PIX_FMT_BGR555 ||
               target == AV_PIX_FMT_RGB444 || target == AV_PIX_FMT_BGR444) {
        uint16_t *dest = (uint16_t *) _dest;
        const uint16_t *r = (const uint16_t *) _r;
        const uint16_t *g = (const uint16_t *) _g;
        const uint16_t *b = (const uint16_t *) _b;
        int dr1, dg1, db1, dr2, dg2, db2;

        if (target == AV_PIX_FMT_RGB565 || target == AV_PIX_FMT_BGR565) {
            dr1 = ff_dither_2x2_8[ y & 1     ][0];
            dg1 = ff_dither_2x2_4[ y & 1     ][0];
            db1 = ff_dither_2x2_8[(y & 1) ^ 1][0];
            dr2 = ff_dither_2x2_8[ y & 1     ][1];
            dg2 = ff_dither_2x2_4[ y & 1     ][1];
            db2 = ff_dither_2x2_8[(y & 1) ^ 1][1];
        }

        dest[i * 2 + 0] = r[Y1 + dr1] + g[Y1 + dg1] + b[Y1 + db1];
        dest[i * 2 + 1] = r[Y2 + dr2] + g[Y2 + dg2] + b[Y2 + db2];
    }
}

static av_always_inline void
yuv2rgb_X_msa_template(SwsContext *c, const int16_t *lumFilter,
                       const int16_t **lumSrc, int lumFilterSize,
                       const int16_t *chrFilter, const int16_t **chrUSrc,
                       const int16_t **chrVSrc, int chrFilterSize,
                       const int16_t **alpSrc, uint8_t *dest, int dstW,
                       int y, enum AVPixelFormat target, int hasAlpha)
{
    int i, j;
    int count = 0;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);

    for (i = 0; i < len; i += 8) {
        int t = 1 << 18;
        v8i16 l_src, u_src, v_src;
        v4i32 lumsrc_r, lumsrc_l, usrc, vsrc, temp;
        v4i32 y_r, y_l, u, v;

        y_r = __msa_fill_w(t);
        y_l = y_r;
        u   = y_r;
        v   = y_r;
        for (j = 0; j < lumFilterSize; j++) {
            temp     = __msa_fill_w(lumFilter[j]);
            l_src    = LD_V(v8i16, (lumSrc[j] + i));
            UNPCK_SH_SW(l_src, lumsrc_r, lumsrc_l);       /*can use lsx optimization*/
            y_r      = __msa_maddv_w(lumsrc_r, temp, y_r);
            y_l      = __msa_maddv_w(lumsrc_l, temp, y_l);
        }
        for (j = 0; j < chrFilterSize; j++) {
            u_src = LD_V(v8i16, (chrUSrc[j] + count));
            v_src = LD_V(v8i16, (chrVSrc[j] + count));
            UNPCK_R_SH_SW(u_src, usrc);
            UNPCK_R_SH_SW(v_src, vsrc);
            temp  = __msa_fill_w(chrFilter[j]);
            u     = __msa_maddv_w(usrc, temp, u);
            v     = __msa_maddv_w(vsrc, temp, v);
        }
        y_r = __msa_srai_w(y_r, 19);
        y_l = __msa_srai_w(y_l, 19);
        u   = __msa_srai_w(u, 19);
        v   = __msa_srai_w(v, 19);
        u   = __msa_addv_w(u, headroom);
        v   = __msa_addv_w(v, headroom);
        for (j = 0; j < 2; j++) {
            int Y1, Y2, U, V;
            int m = j * 2;
            int n = j + 2;

            Y1 = y_r[m];
            Y2 = y_r[m + 1];
            U  = u[j];
            V  = v[j];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
            Y1 = y_l[m];
            Y2 = y_l[m + 1];
            U  = u[n];
            V  = v[n];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
        count += 4;
    }
    for (count; count < len_count; count++) {
        int Y1 = 1 << 18;
        int Y2 = 1 << 18;
        int U  = 1 << 18;
        int V  = 1 << 18;

        for (j = 0; j < lumFilterSize; j++) {
            Y1 += lumSrc[j][count * 2]     * lumFilter[j];
            Y2 += lumSrc[j][count * 2 + 1] * lumFilter[j];
        }
        for (j = 0; j < chrFilterSize; j++) {
            U += chrUSrc[j][count] * chrFilter[j];
            V += chrVSrc[j][count] * chrFilter[j];
        }
        Y1 >>= 19;
        Y2 >>= 19;
        U  >>= 19;
        V  >>= 19;
        r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM];
        g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
             c->table_gV[V + YUVRGB_TABLE_HEADROOM]);
        b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

        yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                      r, g, b, y, target, 0);
    }
}

static av_always_inline void
yuv2rgb_2_msa_template(SwsContext *c, const int16_t *buf[2],
                       const int16_t *ubuf[2], const int16_t *vbuf[2],
                       const int16_t *abuf[2], uint8_t *dest, int dstW,
                       int yalpha, int uvalpha, int y,
                       enum AVPixelFormat target, int hasAlpha)
{
    const int16_t *buf0  = buf[0],  *buf1  = buf[1],
                  *ubuf0 = ubuf[0], *ubuf1 = ubuf[1],
                  *vbuf0 = vbuf[0], *vbuf1 = vbuf[1],
                  *abuf0 = NULL,
                  *abuf1 = NULL;
    int yalpha1  = 4096 - yalpha;
    int uvalpha1 = 4096 - uvalpha;
    int i, count = 0;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;
    const void *r, *g, *b;
    v4i32 v_yalpha1  = (v4i32)__msa_fill_w(yalpha1);
    v4i32 v_uvalpha1 = (v4i32)__msa_fill_w(uvalpha1);
    v4i32 v_yalpha   = (v4i32)__msa_fill_w(yalpha);
    v4i32 v_uvalpha  = (v4i32)__msa_fill_w(uvalpha);
    v4i32 headroom   = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);

    for (i = 0; i < len; i += 8) {
        v8i16 src_y, src_u, src_v;
        v4i32 y0_r, y0_l, u0, v0;
        v4i32 y1_r, y1_l, u1, v1;
        v4i32 y_r, y_l, u, v;

        src_y = LD_V(v8i16, buf0 + i);
        src_u = LD_V(v8i16, ubuf0 + count);
        src_v = LD_V(v8i16, vbuf0 + count);
        UNPCK_SH_SW(src_y, y0_r, y0_l);
        UNPCK_R_SH_SW(src_u, u0);
        UNPCK_R_SH_SW(src_v, v0);
        src_y = LD_V(v8i16, buf1  + i);
        src_u = LD_V(v8i16, ubuf1 + count);
        src_v = LD_V(v8i16, vbuf1 + count);
        UNPCK_SH_SW(src_y, y1_r, y1_l);
        UNPCK_R_SH_SW(src_u, u1);
        UNPCK_R_SH_SW(src_v, v1);
        y0_r = __msa_mulv_w(y0_r, v_yalpha1);
        y0_l = __msa_mulv_w(y0_l, v_yalpha1);
        u0   = __msa_mulv_w(u0, v_uvalpha1);
        v0   = __msa_mulv_w(v0, v_uvalpha1);
        y_r  = __msa_maddv_w(y1_r, v_yalpha, y0_r);
        y_l  = __msa_maddv_w(y1_l, v_yalpha, y0_l);
        u    = __msa_maddv_w(u1, v_uvalpha, u0);
        v    = __msa_maddv_w(v1, v_uvalpha, v0);
        y_r  = __msa_srai_w(y_r, 19);
        y_l  = __msa_srai_w(y_l, 19);
        u    = __msa_srai_w(u, 19);
        v    = __msa_srai_w(v, 19);
        u    = __msa_addv_w(u, headroom);
        v    = __msa_addv_w(v, headroom);
        for (int j = 0; j < 2; j++) {
            int Y1, Y2, U, V;
            int m = j * 2;
            int n = j + 2;

            Y1 = y_r[m];
            Y2 = y_r[m + 1];
            U  = u[j];
            V  = v[j];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
            Y1 = y_l[m];
            Y2 = y_l[m + 1];
            U  = u[n];
            V  = v[n];
            r  =  c->table_rV[V];
            g  = (c->table_gU[U] + c->table_gV[V]);
            b  =  c->table_bU[U];

            yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
        count += 4;
    }

    for (count; count < len_count; count++) {
        int Y1 = (buf0[count * 2]     * yalpha1  +
                  buf1[count * 2]     * yalpha)  >> 19;
        int Y2 = (buf0[count * 2 + 1] * yalpha1  +
                  buf1[count * 2 + 1] * yalpha) >> 19;
        int U  = (ubuf0[count] * uvalpha1 + ubuf1[count] * uvalpha) >> 19;
        int V  = (vbuf0[count] * uvalpha1 + vbuf1[count] * uvalpha) >> 19;

        r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
        g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
             c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
        b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

        yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                      r, g, b, y, target, 0);
    }
}

static av_always_inline void
yuv2rgb_1_msa_template(SwsContext *c, const int16_t *buf0,
                       const int16_t *ubuf[2], const int16_t *vbuf[2],
                       const int16_t *abuf0, uint8_t *dest, int dstW,
                       int uvalpha, int y, enum AVPixelFormat target,
                       int hasAlpha)
{
    const int16_t *ubuf0 = ubuf[0], *vbuf0 = vbuf[0];
    int i, j;
    const void *r, *g, *b;
    int len = dstW & (~0x07);
    int len_count = (dstW + 1) >> 1;

    if (uvalpha < 2048) {
        int count = 0;
        v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);
        v4i32 bias_64   = (v4i32)__msa_fill_w(64);

        for (i = 0; i < len; i += 8) {
            v8i16 src_y, src_u, src_v;
            v4i32 y_r, y_l, u, v;

            src_y = LD_V(v8i16, buf0 + i);
            src_u = LD_V(v8i16, ubuf0 + count);
            src_v = LD_V(v8i16, vbuf0 + count);
            UNPCK_SH_SW(src_y, y_r, y_l);
            UNPCK_R_SH_SW(src_u, u);
            UNPCK_R_SH_SW(src_v, v);
            y_r = __msa_addv_w(y_r, bias_64);
            y_l = __msa_addv_w(y_l, bias_64);
            u   = __msa_addv_w(u, bias_64);
            v   = __msa_addv_w(v, bias_64);
            y_r = __msa_srai_w(y_r, 7);
            y_l = __msa_srai_w(y_l, 7);
            u   = __msa_srai_w(u, 7);
            v   = __msa_srai_w(v, 7);
            u   = __msa_addv_w(u, headroom);
            v   = __msa_addv_w(v, headroom);
            for (j = 0; j < 2; j++) {
                int Y1, Y2, U, V;
                int m = j * 2;
                int n = j + 2;

                Y1 = y_r[m];
                Y2 = y_r[m + 1];
                U  = u[j];
                V  = v[j];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
                Y1 = y_l[m];
                Y2 = y_l[m + 1];
                U  = u[n];
                V  = v[n];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
            }
            count += 4;
        }
        for (count; count < len_count; count++) {
            int Y1 = (buf0[count * 2    ] + 64) >> 7;
            int Y2 = (buf0[count * 2 + 1] + 64) >> 7;
            int U  = (ubuf0[count]        + 64) >> 7;
            int V  = (vbuf0[count]        + 64) >> 7;

            r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
            g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
                 c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
            b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

            yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
    } else {
        const int16_t *ubuf1 = ubuf[1], *vbuf1 = vbuf[1];
        int count = 0;
        v4i32 headroom  = (v4i32)__msa_fill_w(YUVRGB_TABLE_HEADROOM);
        v4i32 bias_64   = (v4i32)__msa_fill_w(64);
        v4i32 bias_128  = (v4i32)__msa_fill_w(128);

        for (i = 0; i < len; i += 8) {
            v8i16 src_y, src_u, src_v;
            v4i32 y_r, y_l, u0, v0, u1, v1, u, v;

            src_y = LD_V(v8i16, buf0 + i);
            src_u = LD_V(v8i16, ubuf0 + count);
            src_v = LD_V(v8i16, vbuf0 + count);
            UNPCK_SH_SW(src_y, y_r, y_l);
            UNPCK_R_SH_SW(src_u, u0);
            UNPCK_R_SH_SW(src_v, v0);
            src_u = LD_V(v8i16, ubuf1 + count);
            src_v = LD_V(v8i16, vbuf1 + count);
            UNPCK_R_SH_SW(src_u, u1);
            UNPCK_R_SH_SW(src_v, v1);

            u   = __msa_addv_w(u0, u1);
            v   = __msa_addv_w(v0, v1);
            y_r = __msa_addv_w(y_r, bias_64);
            y_l = __msa_addv_w(y_l, bias_64);
            u   = __msa_addv_w(u, bias_128);
            v   = __msa_addv_w(v, bias_128);
            y_r = __msa_srai_w(y_r, 7);
            y_l = __msa_srai_w(y_l, 7);
            u   = __msa_srai_w(u, 8);
            v   = __msa_srai_w(v, 8);
            u   = __msa_addv_w(u, headroom);
            v   = __msa_addv_w(v, headroom);
            for (j = 0; j < 2; j++) {
                int Y1, Y2, U, V;
                int m = j * 2;
                int n = j + 2;

                Y1 = y_r[m];
                Y2 = y_r[m + 1];
                U  = u[j];
                V  = v[j];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + j, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
                Y1 = y_l[m];
                Y2 = y_l[m + 1];
                U  = u[n];
                V  = v[n];
                r  =  c->table_rV[V];
                g  = (c->table_gU[U] + c->table_gV[V]);
                b  =  c->table_bU[U];

                yuv2rgb_write(dest, count + n, Y1, Y2, 0, 0,
                              r, g, b, y, target, 0);
            }
            count += 4;
        }
        for (count; count < len_count; count++) {
            int Y1 = (buf0[count * 2    ]         +  64) >> 7;
            int Y2 = (buf0[count * 2 + 1]         +  64) >> 7;
            int U  = (ubuf0[count] + ubuf1[count] + 128) >> 8;
            int V  = (vbuf0[count] + vbuf1[count] + 128) >> 8;

            r =  c->table_rV[V + YUVRGB_TABLE_HEADROOM],
            g = (c->table_gU[U + YUVRGB_TABLE_HEADROOM] +
                 c->table_gV[V + YUVRGB_TABLE_HEADROOM]),
            b =  c->table_bU[U + YUVRGB_TABLE_HEADROOM];

            yuv2rgb_write(dest, count, Y1, Y2, 0, 0,
                          r, g, b, y, target, 0);
        }
    }
}

#define YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                        \
void name ## ext ## _X_msa(SwsContext *c, const int16_t *lumFilter,            \
                           const int16_t **lumSrc, int lumFilterSize,          \
                           const int16_t *chrFilter, const int16_t **chrUSrc,  \
                           const int16_t **chrVSrc, int chrFilterSize,         \
                           const int16_t **alpSrc, uint8_t *dest, int dstW,    \
                           int y)                                              \
{                                                                              \
    name ## base ## _X_msa_template(c, lumFilter, lumSrc, lumFilterSize,       \
                                    chrFilter, chrUSrc, chrVSrc, chrFilterSize,\
                                    alpSrc, dest, dstW, y, fmt, hasAlpha);     \
}

#define YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                       \
YUV2RGBWRAPPERX(name, base, ext, fmt, hasAlpha)                                \
void name ## ext ## _2_msa(SwsContext *c, const int16_t *buf[2],               \
                           const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                           const int16_t *abuf[2], uint8_t *dest, int dstW,    \
                           int yalpha, int uvalpha, int y)                     \
{                                                                              \
    name ## base ## _2_msa_template(c, buf, ubuf, vbuf, abuf, dest,            \
                                    dstW, yalpha, uvalpha, y, fmt, hasAlpha);  \
}

#define YUV2RGBWRAPPER(name, base, ext, fmt, hasAlpha)                         \
YUV2RGBWRAPPERX2(name, base, ext, fmt, hasAlpha)                               \
void name ## ext ## _1_msa(SwsContext *c, const int16_t *buf0,                 \
                           const int16_t *ubuf[2], const int16_t *vbuf[2],     \
                           const int16_t *abuf0, uint8_t *dest, int dstW,      \
                           int uvalpha, int y)                                 \
{                                                                              \
    name ## base ## _1_msa_template(c, buf0, ubuf, vbuf, abuf0, dest,          \
                                    dstW, uvalpha, y, fmt, hasAlpha);          \
}


#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
#endif
YUV2RGBWRAPPER(yuv2rgb,, x32_1,  AV_PIX_FMT_RGB32_1, 0)
YUV2RGBWRAPPER(yuv2rgb,, x32,    AV_PIX_FMT_RGB32, 0)
#endif
YUV2RGBWRAPPER(yuv2rgb,,  16,    AV_PIX_FMT_RGB565,    0)

// This function is copied from libswscale/output.c
static av_always_inline void yuv2rgb_write_full(SwsContext *c,
    uint8_t *dest, int i, int R, int A, int G, int B,
    int y, enum AVPixelFormat target, int hasAlpha, int err[4])
{
    int isrgb8 = target == AV_PIX_FMT_BGR8 || target == AV_PIX_FMT_RGB8;

    if ((R | G | B) & 0xC0000000) {
        R = av_clip_uintp2(R, 30);
        G = av_clip_uintp2(G, 30);
        B = av_clip_uintp2(B, 30);
    }

    switch(target) {
    case AV_PIX_FMT_ARGB:
        dest[0] = hasAlpha ? A : 255;
        dest[1] = R >> 22;
        dest[2] = G >> 22;
        dest[3] = B >> 22;
        break;
    case AV_PIX_FMT_RGB24:
        dest[0] = R >> 22;
        dest[1] = G >> 22;
        dest[2] = B >> 22;
        break;
    case AV_PIX_FMT_RGBA:
        dest[0] = R >> 22;
        dest[1] = G >> 22;
        dest[2] = B >> 22;
        dest[3] = hasAlpha ? A : 255;
        break;
    case AV_PIX_FMT_ABGR:
        dest[0] = hasAlpha ? A : 255;
        dest[1] = B >> 22;
        dest[2] = G >> 22;
        dest[3] = R >> 22;
        break;
    case AV_PIX_FMT_BGR24:
        dest[0] = B >> 22;
        dest[1] = G >> 22;
        dest[2] = R >> 22;
        break;
    case AV_PIX_FMT_BGRA:
        dest[0] = B >> 22;
        dest[1] = G >> 22;
        dest[2] = R >> 22;
        dest[3] = hasAlpha ? A : 255;
        break;
    case AV_PIX_FMT_BGR4_BYTE:
    case AV_PIX_FMT_RGB4_BYTE:
    case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB8:
    {
        int r,g,b;

        switch (c->dither) {
        default:
        case SWS_DITHER_AUTO:
        case SWS_DITHER_ED:
            R >>= 22;
            G >>= 22;
            B >>= 22;
            R += (7*err[0] + 1*c->dither_error[0][i] + 5*c->dither_error[0][i+1] + 3*c->dither_error[0][i+2])>>4;
            G += (7*err[1] + 1*c->dither_error[1][i] + 5*c->dither_error[1][i+1] + 3*c->dither_error[1][i+2])>>4;
            B += (7*err[2] + 1*c->dither_error[2][i] + 5*c->dither_error[2][i+1] + 3*c->dither_error[2][i+2])>>4;
            c->dither_error[0][i] = err[0];
            c->dither_error[1][i] = err[1];
            c->dither_error[2][i] = err[2];
            r = R >> (isrgb8 ? 5 : 7);
            g = G >> (isrgb8 ? 5 : 6);
            b = B >> (isrgb8 ? 6 : 7);
            r = av_clip(r, 0, isrgb8 ? 7 : 1);
            g = av_clip(g, 0, isrgb8 ? 7 : 3);
            b = av_clip(b, 0, isrgb8 ? 3 : 1);
            err[0] = R - r*(isrgb8 ? 36 : 255);
            err[1] = G - g*(isrgb8 ? 36 : 85);
            err[2] = B - b*(isrgb8 ? 85 : 255);
            break;
        case SWS_DITHER_A_DITHER:
            if (isrgb8) {
  /* see http://pippin.gimp.org/a_dither/ for details/origin */
#define A_DITHER(u,v)   (((((u)+((v)*236))*119)&0xff))
                r = (((R >> 19) + A_DITHER(i,y)  -96)>>8);
                g = (((G >> 19) + A_DITHER(i + 17,y) - 96)>>8);
                b = (((B >> 20) + A_DITHER(i + 17*2,y) -96)>>8);
                r = av_clip_uintp2(r, 3);
                g = av_clip_uintp2(g, 3);
                b = av_clip_uintp2(b, 2);
            } else {
                r = (((R >> 21) + A_DITHER(i,y)-256)>>8);
                g = (((G >> 19) + A_DITHER(i + 17,y)-256)>>8);
                b = (((B >> 21) + A_DITHER(i + 17*2,y)-256)>>8);
                r = av_clip_uintp2(r, 1);
                g = av_clip_uintp2(g, 2);
                b = av_clip_uintp2(b, 1);
            }
            break;
        case SWS_DITHER_X_DITHER:
            if (isrgb8) {
  /* see http://pippin.gimp.org/a_dither/ for details/origin */
#define X_DITHER(u,v)   (((((u)^((v)*237))*181)&0x1ff)/2)
                r = (((R >> 19) + X_DITHER(i,y) - 96)>>8);
                g = (((G >> 19) + X_DITHER(i + 17,y) - 96)>>8);
                b = (((B >> 20) + X_DITHER(i + 17*2,y) - 96)>>8);
                r = av_clip_uintp2(r, 3);
                g = av_clip_uintp2(g, 3);
                b = av_clip_uintp2(b, 2);
            } else {
                r = (((R >> 21) + X_DITHER(i,y)-256)>>8);
                g = (((G >> 19) + X_DITHER(i + 17,y)-256)>>8);
                b = (((B >> 21) + X_DITHER(i + 17*2,y)-256)>>8);
                r = av_clip_uintp2(r, 1);
                g = av_clip_uintp2(g, 2);
                b = av_clip_uintp2(b, 1);
            }

            break;
        }

        if(target == AV_PIX_FMT_BGR4_BYTE) {
            dest[0] = r + 2*g + 8*b;
        } else if(target == AV_PIX_FMT_RGB4_BYTE) {
            dest[0] = b + 2*g + 8*r;
        } else if(target == AV_PIX_FMT_BGR8) {
            dest[0] = r + 8*g + 64*b;
        } else if(target == AV_PIX_FMT_RGB8) {
            dest[0] = b + 4*g + 32*r;
        } else
            av_assert2(0);
        break; }
    }
}

#define yuvTorgb_setup                                           \
    int y_offset = c->yuv2rgb_y_offset;                          \
    int y_coeff  = c->yuv2rgb_y_coeff;                           \
    int v2r_coe  = c->yuv2rgb_v2r_coeff;                         \
    int v2g_coe  = c->yuv2rgb_v2g_coeff;                         \
    int u2g_coe  = c->yuv2rgb_u2g_coeff;                         \
    int u2b_coe  = c->yuv2rgb_u2b_coeff;                         \
    v4i32 offset = __msa_fill_w(y_offset);                       \
    v4i32 coeff  = __msa_fill_w(y_coeff);                        \
    v4i32 v2r    = __msa_fill_w(v2r_coe);                        \
    v4i32 v2g    = __msa_fill_w(v2g_coe);                        \
    v4i32 u2g    = __msa_fill_w(u2g_coe);                        \
    v4i32 u2b    = __msa_fill_w(u2b_coe);                        \


#define yuvTorgb                                                 \
     y_r -= offset;                                              \
     y_l -= offset;                                              \
     y_r *= coeff;                                               \
     y_l *= coeff;                                               \
     y_r += y_temp;                                              \
     y_l += y_temp;                                              \
     R_r = __msa_maddv_w(v_r, v2r, y_r);                         \
     R_l = __msa_maddv_w(v_l, v2r, y_l);                         \
     v_r = __msa_maddv_w(v_r, v2g, y_r);                         \
     v_l = __msa_maddv_w(v_l, v2g, y_l);                         \
     G_r = __msa_maddv_w(u_r, u2g, v_r);                         \
     G_l = __msa_maddv_w(u_l, u2g, v_l);                         \
     B_r = __msa_maddv_w(u_r, u2b, y_r);                         \
     B_l = __msa_maddv_w(u_l, u2b, y_l);                         \


static av_always_inline void
yuv2rgb_full_X_msa_template(SwsContext *c, const int16_t *lumFilter,
                          const int16_t **lumSrc, int lumFilterSize,
                          const int16_t *chrFilter, const int16_t **chrUSrc,
                          const int16_t **chrVSrc, int chrFilterSize,
                          const int16_t **alpSrc, uint8_t *dest,
                          int dstW, int y, enum AVPixelFormat target, int hasAlpha)
{
    int i, j, B, G, R, A;
    int step     = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    int err[4]   = {0};
    int a_temp   = 1 << 18; //init to silence warning
    int templ    = 1 << 9;
    int tempc    = templ - (128 << 19);
    int len      = dstW & (~0x07);
    int ytemp    = 1 << 21;
    v4i32 y_temp = __msa_fill_w(ytemp);
    yuvTorgb_setup

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;

    for (i = 0; i < len; i += 8) {
        int m = i + 4;
        v8i16 l_src, u_src, v_src;
        v4i32 lum_r, lum_l, usrc_r, usrc_l, vsrc_r, vsrc_l;
        v4i32 y_r, y_l, u_r, u_l, v_r, v_l, temp;
        v4i32 R_r, R_l, G_r, G_l, B_r, B_l;

        y_r = y_l = (v4i32)__msa_fill_w(templ);
        u_r = u_l = v_r = v_l = (v4i32)__msa_fill_w(tempc);
        for (j = 0; j < lumFilterSize; j++) {
            temp  = __msa_fill_w(lumFilter[j]);
            l_src = LD_V(v8i16, (lumSrc[j] + i));
            UNPCK_SH_SW(l_src, lum_r, lum_l);
            y_l   = __msa_maddv_w(lum_l, temp, y_l);
            y_r   = __msa_maddv_w(lum_r, temp, y_r);
        }
        for (j = 0; j < chrFilterSize; j++) {
            temp  = __msa_fill_w(chrFilter[j]);
            u_src = LD_V(v8i16, (chrUSrc[j] + i));
            v_src = LD_V(v8i16, (chrVSrc[j] + i));
            UNPCK_SH_SW(u_src, usrc_r, usrc_l);
            UNPCK_SH_SW(v_src, vsrc_r, vsrc_l);
            u_l   = __msa_maddv_w(usrc_l, temp, u_l);
            u_r   = __msa_maddv_w(usrc_r, temp, u_r);
            v_l   = __msa_maddv_w(vsrc_l, temp, v_l);
            v_r   = __msa_maddv_w(vsrc_r, temp, v_r);
        }
        y_r = __msa_srai_w(y_r, 10);
        y_l = __msa_srai_w(y_l, 10);
        u_r = __msa_srai_w(u_r, 10);
        u_l = __msa_srai_w(u_l, 10);
        v_r = __msa_srai_w(v_r, 10);
        v_l = __msa_srai_w(v_l, 10);
        yuvTorgb

        if (hasAlpha) {
            v8i16 a_src;
            v4i32 asrc_r, asrc_l, a_r, a_l;

            a_r = a_l = __msa_fill_w(a_temp);
            for (j = 0; j < lumFilterSize; j++) {
                temp  = __msa_fill_w(lumFilter[j]);
                a_src = LD_V(v8i16, (alpSrc[j] + i));
                UNPCK_SH_SW(a_src, asrc_r, asrc_l);
                a_l   = __msa_maddv_w(asrc_l, temp, a_l);
                a_r   = __msa_maddv_w(asrc_r, temp, a_r);
            }
            a_l = __msa_srai_w(a_l, 19);
            a_r = __msa_srai_w(a_r, 19);
            for (j = 0; j < 4; j++) {
                R = R_r[j];
                G = G_r[j];
                B = B_r[j];
                A = a_r[j];
                if (A & 0x100)
                    A = av_clip_uint8(A);
                yuv2rgb_write_full(c, dest, i + j, R, A, G, B, y, target, hasAlpha, err);
                dest += step;
            }
            for (j = 0; j < 4; j++) {
                R = R_l[j];
                G = G_l[j];
                B = B_l[j];
                A = a_l[j];
                if (A & 0x100)
                    A = av_clip_uint8(A);
                yuv2rgb_write_full(c, dest, m + j, R, A, G, B, y, target, hasAlpha, err);
                dest += step;
            }
        } else {
            for (j = 0; j < 4; j++) {
                R = R_r[j];
                G = G_r[j];
                B = B_r[j];
                yuv2rgb_write_full(c, dest, i + j, R, 0, G, B, y, target, hasAlpha, err);
                dest += step;
            }
            for (j = 0; j < 4; j++) {
                R = R_l[j];
                G = G_l[j];
                B = B_l[j];
                yuv2rgb_write_full(c, dest, m + j, R, 0, G, B, y, target, hasAlpha, err);
                dest += step;
            }
        }
    }
    for (i; i < dstW; i++) {
        int Y = templ;
        int V, U = V = tempc;

        A = 0;
        for (j = 0; j < lumFilterSize; j++) {
            Y += lumSrc[j][i] * lumFilter[j];
        }
        for (j = 0; j < chrFilterSize; j++) {
            U += chrUSrc[j][i] * chrFilter[j];
            V += chrVSrc[j][i] * chrFilter[j];

        }
        Y >>= 10;
        U >>= 10;
        V >>= 10;
        if (hasAlpha) {
            A = 1 << 18;
            for (j = 0; j < lumFilterSize; j++) {
                A += alpSrc[j][i] * lumFilter[j];
            }
            A >>= 19;
            if (A & 0x100)
                A = av_clip_uint8(A);
        }
        Y -= y_offset;
        Y *= y_coeff;
        Y += ytemp;
        R = (unsigned)Y + V * v2r_coe;
        G = (unsigned)Y + V * v2g_coe + U * u2g_coe;
        B = (unsigned)Y + U * u2b_coe;
        yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
        dest += step;
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}

static av_always_inline void
yuv2rgb_full_2_msa_template(SwsContext *c, const int16_t *buf[2],
                     const int16_t *ubuf[2], const int16_t *vbuf[2],
                     const int16_t *abuf[2], uint8_t *dest, int dstW,
                     int yalpha, int uvalpha, int y,
                     enum AVPixelFormat target, int hasAlpha)
{
    const int16_t *buf0  = buf[0],  *buf1  = buf[1],
                  *ubuf0 = ubuf[0], *ubuf1 = ubuf[1],
                  *vbuf0 = vbuf[0], *vbuf1 = vbuf[1],
                  *abuf0 = hasAlpha ? abuf[0] : NULL,
                  *abuf1 = hasAlpha ? abuf[1] : NULL;
    int yalpha1  = 4096 - yalpha;
    int uvalpha1 = 4096 - uvalpha;
    int uvtemp   = 128 << 19;
    int atemp    = 1 << 18;
    int i, j, R, G, B, A;
    int step = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    int err[4] = {0};
    int len = dstW & (~0x07);
    int ytemp   = 1 << 21;
    v4i32 uvalp1 = (v4i32)__msa_fill_w(uvalpha1);
    v4i32 yalp1  = (v4i32)__msa_fill_w(yalpha1);
    v4i32 uvalp  = (v4i32)__msa_fill_w(uvalpha);
    v4i32 yalp   = (v4i32)__msa_fill_w(yalpha);
    v4i32 uv     = (v4i32)__msa_fill_w(uvtemp);
    v4i32 a_bias = (v4i32)__msa_fill_w(atemp);
    v4i32 y_temp = __msa_fill_w(ytemp);
    yuvTorgb_setup

    av_assert2(yalpha  <= 4096U);
    av_assert2(uvalpha <= 4096U);

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;

    for (i = 0; i < len; i += 8) {
        int m = i + 4;
        v8i16 b0, b1, ub0, ub1, vb0, vb1;
        v4i32 b0_r, b0_l, b1_r, b1_l, ub0_r, ub0_l, ub1_r, ub1_l, vb0_r, vb0_l, vb1_r, vb1_l;
        v4i32 y_r, y_l, u_r, u_l, v_r, v_l;
        v4i32 R_r, R_l, G_r, G_l, B_r, B_l;

        b0  = LD_V(v8i16, (buf0 + i));
        b1  = LD_V(v8i16, (buf1 + i));
        ub0 = LD_V(v8i16, (ubuf0 + i));
        ub1 = LD_V(v8i16, (ubuf1 + i));
        vb0 = LD_V(v8i16, (vbuf0 + i));
        vb1 = LD_V(v8i16, (vbuf1 + i));
        UNPCK_SH_SW(b0, b0_r, b0_l);
        UNPCK_SH_SW(b1, b1_r, b1_l);
        UNPCK_SH_SW(ub0, ub0_r, ub0_l);
        UNPCK_SH_SW(ub1, ub1_r, ub1_l);
        UNPCK_SH_SW(vb0, vb0_r, vb0_l);
        UNPCK_SH_SW(vb1, vb1_r, vb1_l);
        y_r = b0_r * yalp1;
        y_l = b0_l * yalp1;
        y_r = __msa_maddv_w(b1_r, yalp, y_r);
        y_l = __msa_maddv_w(b1_l, yalp, y_l);
        u_r = ub0_r * uvalp1;
        u_l = ub0_l * uvalp1;
        u_r = __msa_maddv_w(ub1_r, uvalp, u_r);
        u_l = __msa_maddv_w(ub1_l, uvalp, u_l);
        v_r = vb0_r * uvalp1;
        v_l = vb0_l * uvalp1;
        v_r = __msa_maddv_w(vb1_r, uvalp, v_r);
        v_l = __msa_maddv_w(vb1_l, uvalp, v_l);
        u_r -= uv;
        u_l -= uv;
        v_r -= uv;
        v_l -= uv;
        y_r = __msa_srai_w(y_r, 10);
        y_l = __msa_srai_w(y_l, 10);
        u_r = __msa_srai_w(u_r, 10);
        u_l = __msa_srai_w(u_l, 10);
        v_r = __msa_srai_w(v_r, 10);
        v_l = __msa_srai_w(v_l, 10);
        yuvTorgb

        if (hasAlpha) {
            v8i16 a0, a1;
            v4i32 a_r, a_l, a1_r, a1_l;

            a0 = LD_V(v8i16, (abuf0 + i));
            a1 = LD_V(v8i16, (abuf1 + i));
            UNPCK_SH_SW(a0, a_r, a_l);
            UNPCK_SH_SW(a1, a1_r, a1_l);
            a_r *= yalp1;
            a_l *= yalp1;
            a_r = __msa_maddv_w(a1_r, yalp, a_r);
            a_l = __msa_maddv_w(a1_l, yalp, a_l);
            a_r += a_bias;
            a_l += a_bias;
            a_r = __msa_srai_w(a_r, 19);
            a_l = __msa_srai_w(a_l, 19);
            for (j = 0; j < 4; j++) {
                R = R_r[j];
                G = G_r[j];
                B = B_r[j];
                A = a_r[j];
                if (A & 0x100)
                    A = av_clip_uint8(A);
                yuv2rgb_write_full(c, dest, i + j, R, A, G, B, y, target, hasAlpha, err);
                dest += step;
            }
            for (j = 0; j < 4; j++) {
                R = R_l[j];
                G = G_l[j];
                B = B_l[j];
                A = a_l[j];
                if (A & 0x100)
                    A = av_clip_uint8(A);
                yuv2rgb_write_full(c, dest, m + j, R, A, G, B, y, target, hasAlpha, err);
                dest += step;
            }
        } else {
            for (j = 0; j < 4; j++) {
                R = R_r[j];
                G = G_r[j];
                B = B_r[j];
                yuv2rgb_write_full(c, dest, i + j, R, 0, G, B, y, target, hasAlpha, err);
                dest += step;
            }
            for (j = 0; j < 4; j++) {
                R = R_l[j];
                G = G_l[j];
                B = B_l[j];
                yuv2rgb_write_full(c, dest, m + j, R, 0, G, B, y, target, hasAlpha, err);
                dest += step;
            }
        }
    }
    for (i; i < dstW; i++){
        int Y = ( buf0[i] * yalpha1  +  buf1[i] * yalpha         ) >> 10; //FIXME rounding
        int U = (ubuf0[i] * uvalpha1 + ubuf1[i] * uvalpha- uvtemp) >> 10;
        int V = (vbuf0[i] * uvalpha1 + vbuf1[i] * uvalpha- uvtemp) >> 10;

        A = 0;
        if (hasAlpha){
            A = (abuf0[i] * yalpha1 + abuf1[i] * yalpha + atemp) >> 19;
            if (A & 0x100)
                A = av_clip_uint8(A);
        }

        Y -= y_offset;
        Y *= y_coeff;
        Y += ytemp;
        R = (unsigned)Y + V * v2r_coe;
        G = (unsigned)Y + V * v2g_coe + U * u2g_coe;
        B = (unsigned)Y + U * u2b_coe;
        yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
        dest += step;
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}

static av_always_inline void
yuv2rgb_full_1_msa_template(SwsContext *c, const int16_t *buf0,
                            const int16_t *ubuf[2], const int16_t *vbuf[2],
                            const int16_t *abuf0, uint8_t *dest, int dstW,
                            int uvalpha, int y, enum AVPixelFormat target,
                            int hasAlpha)
{
    const int16_t *ubuf0 = ubuf[0], *vbuf0 = vbuf[0];
    int i, j, B, G, R, A;
    int step = (target == AV_PIX_FMT_RGB24 || target == AV_PIX_FMT_BGR24) ? 3 : 4;
    int err[4] = {0};
    int len = dstW & (~0x07);
    int ytemp    = 1 << 21;
    v4i32 bias   = __msa_fill_w(64);
    v4i32 y_temp = __msa_fill_w(ytemp);
    yuvTorgb_setup

    if(   target == AV_PIX_FMT_BGR4_BYTE || target == AV_PIX_FMT_RGB4_BYTE
       || target == AV_PIX_FMT_BGR8      || target == AV_PIX_FMT_RGB8)
        step = 1;
    if (uvalpha < 2048) {
        int uvtemp = 128 << 7;
        v4i32 uv   = __msa_fill_w(uvtemp);

        for (i = 0; i < len; i += 8) {
            int m = i + 4;
            v8i16 b, ub, vb;
            v4i32 y_r, y_l, u_r, u_l, v_r, v_l;
            v4i32 R_r, R_l, G_r, G_l, B_r, B_l;

            b   = LD_V(v8i16, (buf0 + i));
            ub  = LD_V(v8i16, (ubuf0 + i));
            vb  = LD_V(v8i16, (vbuf0 + i));
            UNPCK_SH_SW(b, y_r, y_l);
            UNPCK_SH_SW(ub, u_r, u_l);
            UNPCK_SH_SW(vb, v_r, v_l);
            y_r = __msa_slli_w(y_r, 2);
            y_l = __msa_slli_w(y_l, 2);
            u_r -= uv;
            u_l -= uv;
            v_r -= uv;
            v_l -= uv;
            u_r = __msa_slli_w(u_r, 2);
            u_l = __msa_slli_w(u_l, 2);
            v_r = __msa_slli_w(v_r, 2);
            v_l = __msa_slli_w(v_l, 2);
            yuvTorgb

            if(hasAlpha) {
                v8i16 a_src;
                v4i32 a_r, a_l;

                a_src = LD_V(v8i16, (abuf0 + i));
                UNPCK_SH_SW(a_src, a_r, a_l);
                a_r += bias;
                a_l += bias;
                a_r = __msa_srai_w(a_r, 7);
                a_l = __msa_srai_w(a_l, 7);
                for (j = 0; j < 4; j++) {
                    R = R_r[j];
                    G = G_r[j];
                    B = B_r[j];
                    A = a_r[j];
                    if (A & 0x100)
                        A = av_clip_uint8(A);
                    yuv2rgb_write_full(c, dest, i + j, R, A, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
                for (j = 0; j < 4; j++) {
                    R = R_l[j];
                    G = G_l[j];
                    B = B_l[j];
                    A = a_l[j];
                    if (A & 0x100)
                        A = av_clip_uint8(A);
                    yuv2rgb_write_full(c, dest, m + j, R, A, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
            } else {
                for (j = 0; j < 4; j++) {
                    R = R_r[j];
                    G = G_r[j];
                    B = B_r[j];
                    yuv2rgb_write_full(c, dest, i + j, R, 0, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
                for (j = 0; j < 4; j++) {
                    R = R_l[j];
                    G = G_l[j];
                    B = B_l[j];
                    yuv2rgb_write_full(c, dest, m + j, R, 0, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
            }
        }
        for (i; i < dstW; i++) {
            int Y = buf0[i] << 2;
            int U = (ubuf0[i] - uvtemp) << 2;
            int V = (vbuf0[i] - uvtemp) << 2;

            A = 0;
            if(hasAlpha) {
                A = (abuf0[i] + 64) >> 7;
                if (A & 0x100)
                    A = av_clip_uint8(A);
            }
            Y -= y_offset;
            Y *= y_coeff;
            Y += ytemp;
            R = (unsigned)Y + V * v2r_coe;
            G = (unsigned)Y + V * v2g_coe + U * u2g_coe;
            B = (unsigned)Y + U * u2b_coe;
            yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
            dest += step;
        }
    } else {
        const int16_t *ubuf1 = ubuf[1], *vbuf1 = vbuf[1];
        int uvtemp = 128 << 8;
        v4i32 uv   = __msa_fill_w(uvtemp);

        for (i = 0; i < len; i += 8) {
            v8i16 b, ub, vb;
            v4i32 y_r, y_l, u_r, u_l, v_r, v_l;
            v4i32 u1_r, u1_l, v1_r, v1_l;
            v4i32 R_r, R_l, G_r, G_l, B_r, B_l;
            int m = i + 4;

            b   = LD_V(v8i16, (buf0 + i));
            ub  = LD_V(v8i16, (ubuf0 + i));
            vb  = LD_V(v8i16, (vbuf0 + i));
            UNPCK_SH_SW(b, y_r, y_l);
            UNPCK_SH_SW(ub, u_r, u_l);
            UNPCK_SH_SW(vb, v_r, v_l);
            y_r = __msa_slli_w(y_r, 2);
            y_l = __msa_slli_w(y_l, 2);
            u_r -= uv;
            u_l -= uv;
            v_r -= uv;
            v_l -= uv;
            ub  = LD_V(v8i16, (ubuf1 + i));
            vb  = LD_V(v8i16, (vbuf1 + i));
            UNPCK_SH_SW(ub, u1_r, u1_l);
            UNPCK_SH_SW(vb, v1_r, v1_l);
            u_r += u1_r;
            u_l += u1_l;
            v_r += v1_r;
            v_l += v1_l;
            u_r = __msa_slli_w(u_r, 1);
            u_l = __msa_slli_w(u_l, 1);
            yuvTorgb

            if(hasAlpha) {
                v8i16 a_src;
                v4i32 a_r, a_l;

                a_src = LD_V(v8i16, (abuf0 + i));
                UNPCK_SH_SW(a_src, a_r, a_l);
                a_r += bias;
                a_l += bias;
                a_r = __msa_srai_w(a_r, 7);
                a_l = __msa_srai_w(a_l, 7);
                for (j = 0; j < 4; j++) {
                    R = R_r[j];
                    G = G_r[j];
                    B = B_r[j];
                    A = a_r[j];
                    if (A & 0x100)
                        A = av_clip_uint8(A);
                    yuv2rgb_write_full(c, dest, i + j, R, A, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
                for (j = 0; j < 4; j++) {
                    R = R_l[j];
                    G = G_l[j];
                    B = B_l[j];
                    A = a_l[j];
                    if (A & 0x100)
                        A = av_clip_uint8(A);
                    yuv2rgb_write_full(c, dest, m + j, R, A, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
            } else {
                for (j = 0; j < 4; j++) {
                    R = R_r[j];
                    G = G_r[j];
                    B = B_r[j];
                    yuv2rgb_write_full(c, dest, i + j, R, 0, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
                for (j = 0; j < 4; j++) {
                    R = R_l[j];
                    G = G_l[j];
                    B = B_l[j];
                    yuv2rgb_write_full(c, dest, m + j, R, 0, G, B, y, target, hasAlpha, err);
                    dest += step;
                }
            }
        }
        for (i; i < dstW; i++) {
            int Y = buf0[i] << 2;
            int U = (ubuf0[i] + ubuf1[i] - uvtemp) << 1;
            int V = (vbuf0[i] + vbuf1[i] - uvtemp) << 1;

            A = 0;
            if(hasAlpha) {
                A = (abuf0[i] + 64) >> 7;
                if (A & 0x100)
                    A = av_clip_uint8(A);
            }
            Y -= y_offset;
            Y *= y_coeff;
            Y += ytemp;
            R = (unsigned)Y + V * v2r_coe;
            G = (unsigned)Y + V * v2g_coe + U * u2g_coe;
            B = (unsigned)Y + U * u2b_coe;
            yuv2rgb_write_full(c, dest, i, R, A, G, B, y, target, hasAlpha, err);
            dest += step;
        }
    }
    c->dither_error[0][i] = err[0];
    c->dither_error[1][i] = err[1];
    c->dither_error[2][i] = err[2];
}
#if CONFIG_SMALL
YUV2RGBWRAPPER(yuv2, rgb_full, bgra32_full, AV_PIX_FMT_BGRA,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, abgr32_full, AV_PIX_FMT_ABGR,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, rgba32_full, AV_PIX_FMT_RGBA,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
YUV2RGBWRAPPER(yuv2, rgb_full, argb32_full, AV_PIX_FMT_ARGB,  CONFIG_SWSCALE_ALPHA && c->needAlpha)
#else
#if CONFIG_SWSCALE_ALPHA
YUV2RGBWRAPPER(yuv2, rgb_full, bgra32_full, AV_PIX_FMT_BGRA,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, abgr32_full, AV_PIX_FMT_ABGR,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, rgba32_full, AV_PIX_FMT_RGBA,  1)
YUV2RGBWRAPPER(yuv2, rgb_full, argb32_full, AV_PIX_FMT_ARGB,  1)
#endif
YUV2RGBWRAPPER(yuv2, rgb_full, bgrx32_full, AV_PIX_FMT_BGRA,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, xbgr32_full, AV_PIX_FMT_ABGR,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgbx32_full, AV_PIX_FMT_RGBA,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, xrgb32_full, AV_PIX_FMT_ARGB,  0)
#endif
YUV2RGBWRAPPER(yuv2, rgb_full, bgr24_full,  AV_PIX_FMT_BGR24, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb24_full,  AV_PIX_FMT_RGB24, 0)

YUV2RGBWRAPPER(yuv2, rgb_full, bgr4_byte_full,  AV_PIX_FMT_BGR4_BYTE, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb4_byte_full,  AV_PIX_FMT_RGB4_BYTE, 0)
YUV2RGBWRAPPER(yuv2, rgb_full, bgr8_full,   AV_PIX_FMT_BGR8,  0)
YUV2RGBWRAPPER(yuv2, rgb_full, rgb8_full,   AV_PIX_FMT_RGB8,  0)
#undef yuvTorgb
#undef yuvTorgb_setup

void planar_rgb_to_uv_msa(uint8_t *_dstU, uint8_t *_dstV, const uint8_t *src[4],
                          int width, int32_t *rgb2yuv)
{
    uint16_t *dstU = (uint16_t *)_dstU;
    uint16_t *dstV = (uint16_t *)_dstV;
    int i;
    int len = width & (~0x07);
    int set = 0x4001<<(RGB2YUV_SHIFT - 7);
    int32_t tem_ru = rgb2yuv[RU_IDX], tem_gu = rgb2yuv[GU_IDX];
    int32_t tem_bu = rgb2yuv[BU_IDX];
    int32_t tem_rv = rgb2yuv[RV_IDX], tem_gv = rgb2yuv[GV_IDX];
    int32_t tem_bv = rgb2yuv[BV_IDX];
    int shift = RGB2YUV_SHIFT - 6;
    v4i32 ru, gu, bu, rv, gv, bv;
    v4i32 temp = __msa_fill_w(set);
    v4i32 sra  = __msa_fill_w(shift);
    v16i8 zero = {0};

    ru = __msa_fill_w(tem_ru);
    gu = __msa_fill_w(tem_gu);
    bu = __msa_fill_w(tem_bu);
    rv = __msa_fill_w(tem_rv);
    gv = __msa_fill_w(tem_gv);
    bv = __msa_fill_w(tem_bv);
    for (i = 0; i < len; i += 8) {
        v16i8 _g, _b, _r;
        v8i16 t_g, t_b, t_r;
        v4i32 g_r, g_l, b_r, b_l, r_r, r_l;
        v4i32 v_r, v_l, u_r, u_l;

        _g  = LD_V(v16i8, (src[0] + i));
        _b  = LD_V(v16i8, (src[1] + i));
        _r  = LD_V(v16i8, (src[2] + i));
        t_g = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_g);
        t_b = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_b);
        t_r = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_r);
        g_r = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_g);
        g_l = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_g);
        b_r = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_b);
        b_l = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_b);
        r_r = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_r);
        r_l = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_r);
        v_r = (v4i32)__msa_mulv_w(r_r, rv);
        v_l = (v4i32)__msa_mulv_w(r_l, rv);
        v_r = (v4i32)__msa_maddv_w(g_r, gv, v_r);
        v_l = (v4i32)__msa_maddv_w(g_l, gv, v_l);
        v_r = (v4i32)__msa_maddv_w(b_r, bv, v_r);
        v_l = (v4i32)__msa_maddv_w(b_l, bv, v_l);
        u_r = (v4i32)__msa_mulv_w(r_r, ru);
        u_l = (v4i32)__msa_mulv_w(r_l, ru);
        u_r = (v4i32)__msa_maddv_w(g_r, gu, u_r);
        u_l = (v4i32)__msa_maddv_w(g_l, gu, u_l);
        u_r = (v4i32)__msa_maddv_w(b_r, bu, u_r);
        u_l = (v4i32)__msa_maddv_w(b_l, bu, u_l);
        v_r = (v4i32)__msa_addv_w(v_r, temp);
        v_l = (v4i32)__msa_addv_w(v_l, temp);
        u_r = (v4i32)__msa_addv_w(u_r, temp);
        u_l = (v4i32)__msa_addv_w(u_l, temp);
        v_r = (v4i32)__msa_sra_w(v_r, sra);
        v_l = (v4i32)__msa_sra_w(v_l, sra);
        u_r = (v4i32)__msa_sra_w(u_r, sra);
        u_l = (v4i32)__msa_sra_w(u_l, sra);
        for (int j = 0; j < 4; j++) {
            int m = i + j;

            dstU[m] = u_r[j];
            dstV[m] = v_r[j];
            dstU[m + 4] = u_l[j];
            dstV[m + 4] = v_l[j];
        }
    }
    for (i; i < width; i++) {
        int g = src[0][i];
        int b = src[1][i];
        int r = src[2][i];

        dstU[i] = (tem_ru * r + tem_gu * g + tem_bu * b + set) >> shift;
        dstV[i] = (tem_rv * r + tem_gv * g + tem_bv * b + set) >> shift;
    }
}

void planar_rgb_to_y_msa(uint8_t *_dst, const uint8_t *src[4], int width,
                         int32_t *rgb2yuv)
{
    uint16_t *dst = (uint16_t *)_dst;
    int32_t tem_ry = rgb2yuv[RY_IDX], tem_gy = rgb2yuv[GY_IDX];
    int32_t tem_by = rgb2yuv[BY_IDX];
    int len    = width & (~0x07);
    int shift  = (RGB2YUV_SHIFT-6);
    int set    = 0x801 << (RGB2YUV_SHIFT - 7);
    int i;
    v4i32 temp = (v4i32)__msa_fill_w(set);
    v4i32 sra  = (v4i32)__msa_fill_w(shift);
    v4i32 ry   = (v4i32)__msa_fill_w(tem_ry);
    v4i32 gy   = (v4i32)__msa_fill_w(tem_gy);
    v4i32 by   = (v4i32)__msa_fill_w(tem_by);
    v16i8 zero = {0};

    for (i = 0; i < len; i += 8) {
        v16i8 _g, _b, _r;
        v8i16 t_g, t_b, t_r;
        v4i32 g_r, g_l, b_r, b_l, r_r, r_l;
        v4i32 out_r, out_l;

        _g    = LD_V(v16i8, src[0] + i);
        _b    = LD_V(v16i8, src[1] + i);
        _r    = LD_V(v16i8, src[2] + i);
        t_g   = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_g);
        t_b   = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_b);
        t_r   = (v8i16)__msa_ilvr_b((v16i8)zero, (v16i8)_r);
        g_r   = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_g);
        g_l   = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_g);
        b_r   = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_b);
        b_l   = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_b);
        r_r   = (v4i32)__msa_ilvr_h((v8i16)zero, (v8i16)t_r);
        r_l   = (v4i32)__msa_ilvl_h((v8i16)zero, (v8i16)t_r);
        out_r = (v4i32)__msa_mulv_w(r_r, ry);
        out_l = (v4i32)__msa_mulv_w(r_l, ry);
        out_r = (v4i32)__msa_maddv_w(g_r, gy, out_r);
        out_l = (v4i32)__msa_maddv_w(g_l, gy, out_l);
        out_r = (v4i32)__msa_maddv_w(b_r, by, out_r);
        out_l = (v4i32)__msa_maddv_w(b_l, by, out_l);
        out_r = (v4i32)__msa_addv_w(out_r, temp);
        out_l = (v4i32)__msa_addv_w(out_l, temp);
        out_r = (v4i32)__msa_sra_w(out_r, sra);
        out_l = (v4i32)__msa_sra_w(out_l, sra);
        for (int j = 0; j < 4; j++) {
            int m = i + j;
            dst[m] = out_r[j];
            dst[m + 4] = out_l[j];
        }
    }
    for (i; i < width; i++) {
        int g = src[0][i];
        int b = src[1][i];
        int r = src[2][i];

        dst[i] = (tem_ry * r + tem_gy * g + tem_by * b + set) >> shift;
    }
}
