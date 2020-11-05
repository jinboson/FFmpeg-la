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
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(c->dstFormat);

        if (c->srcBpc == 8) {
            if (c->dstBpc <= 14) {
                c->hyScale = c->hcScale = ff_hscale_8_to_15_msa;
            } else {
                c->hyScale = c->hcScale = ff_hscale_8_to_19_msa;
            }
        } else {
            c->hyScale = c->hcScale = c->dstBpc > 14 ? ff_hscale_16_to_19_msa
                                                     : ff_hscale_16_to_15_msa;
        }
        switch (c->srcFormat) {
        case AV_PIX_FMT_GBRAP:
        case AV_PIX_FMT_GBRP:
            {
                 c->readChrPlanar = planar_rgb_to_uv_msa;
                 c->readLumPlanar = planar_rgb_to_y_msa;
            }
            break;
        }
        if (c->dstBpc == 8)
            c->yuv2planeX = ff_yuv2planeX_8_msa;
        if (c->dstFormat == AV_PIX_FMT_P010LE || c->dstFormat == AV_PIX_FMT_P010BE) {
        } else if (is16BPS(c->dstFormat)) {
            c->yuv2planeX = isBE(c->dstFormat) ? yuv2planeX_16BE_msa  : yuv2planeX_16LE_msa;
            c->yuv2plane1 = isBE(c->dstFormat) ? yuv2plane1_16BE_msa  : yuv2plane1_16LE_msa;
        } else if (isNBPS(c->dstFormat)) {
            if (desc->comp[0].depth == 9) {
                c->yuv2planeX = isBE(c->dstFormat) ? yuv2planeX_9BE_msa  : yuv2planeX_9LE_msa;
                c->yuv2plane1 = isBE(c->dstFormat) ? yuv2plane1_9BE_msa  : yuv2plane1_9LE_msa;
            } else if (desc->comp[0].depth == 10) {
                c->yuv2planeX = isBE(c->dstFormat) ? yuv2planeX_10BE_msa  : yuv2planeX_10LE_msa;
                c->yuv2plane1 = isBE(c->dstFormat) ? yuv2plane1_10BE_msa  : yuv2plane1_10LE_msa;
            } else if (desc->comp[0].depth == 12) {
                c->yuv2planeX = isBE(c->dstFormat) ? yuv2planeX_12BE_msa  : yuv2planeX_12LE_msa;
                c->yuv2plane1 = isBE(c->dstFormat) ? yuv2plane1_12BE_msa  : yuv2plane1_12LE_msa;
            } else if (desc->comp[0].depth == 14) {
                c->yuv2planeX = isBE(c->dstFormat) ? yuv2planeX_14BE_msa  : yuv2planeX_14LE_msa;
                c->yuv2plane1 = isBE(c->dstFormat) ? yuv2plane1_14BE_msa  : yuv2plane1_14LE_msa;
            } else
                av_assert0(0);
        }
        if (c->flags & SWS_FULL_CHR_H_INT) {
            switch (c->dstFormat) {
            case AV_PIX_FMT_BGRA:
#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packedX = yuv2bgrx32_full_X_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            }
        } else {
            switch (c->dstFormat) {
            case AV_PIX_FMT_RGB32:
            case AV_PIX_FMT_BGR32:
#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packed1 = yuv2rgbx32_1_msa;
                    c->yuv2packed2 = yuv2rgbx32_2_msa;
                    c->yuv2packedX = yuv2rgbx32_X_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_RGB32_1:
            case AV_PIX_FMT_BGR32_1:
#if CONFIG_SMALL
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packed1 = yuv2rgbx32_1_1_msa;
                    c->yuv2packed2 = yuv2rgbx32_1_2_msa;
                    c->yuv2packedX = yuv2rgbx32_1_X_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_RGB565LE:
            case AV_PIX_FMT_RGB565BE:
            case AV_PIX_FMT_BGR565LE:
            case AV_PIX_FMT_BGR565BE:
                c->yuv2packed1 = yuv2rgb16_1_msa;
                c->yuv2packed2 = yuv2rgb16_2_msa;
                c->yuv2packedX = yuv2rgb16_X_msa;
                break;
            }
        }
    }
#endif /* #if HAVE_MSA */
}
