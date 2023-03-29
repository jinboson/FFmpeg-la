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
        if (c->flags & SWS_FULL_CHR_H_INT) {
            switch (c->dstFormat) {
            case AV_PIX_FMT_RGBA:
#if CONFIG_SMALL
                c->yuv2packedX = yuv2rgba32_full_X_msa;
                c->yuv2packed2 = yuv2rgba32_full_2_msa;
                c->yuv2packed1 = yuv2rgba32_full_1_msa;
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                    c->yuv2packedX = yuv2rgba32_full_X_msa;
                    c->yuv2packed2 = yuv2rgba32_full_2_msa;
                    c->yuv2packed1 = yuv2rgba32_full_1_msa;
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packedX = yuv2rgbx32_full_X_msa;
                    c->yuv2packed2 = yuv2rgbx32_full_2_msa;
                    c->yuv2packed1 = yuv2rgbx32_full_1_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_ARGB:
#if CONFIG_SMALL
                c->yuv2packedX = yuv2argb32_full_X_msa;
                c->yuv2packed2 = yuv2argb32_full_2_msa;
                c->yuv2packed1 = yuv2argb32_full_1_msa;
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                    c->yuv2packedX = yuv2argb32_full_X_msa;
                    c->yuv2packed2 = yuv2argb32_full_2_msa;
                    c->yuv2packed1 = yuv2argb32_full_1_msa;
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packedX = yuv2xrgb32_full_X_msa;
                    c->yuv2packed2 = yuv2xrgb32_full_2_msa;
                    c->yuv2packed1 = yuv2xrgb32_full_1_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_BGRA:
#if CONFIG_SMALL
                c->yuv2packedX = yuv2bgra32_full_X_msa;
                c->yuv2packed2 = yuv2bgra32_full_2_msa;
                c->yuv2packed1 = yuv2bgra32_full_1_msa;
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                    c->yuv2packedX = yuv2bgra32_full_X_msa;
                    c->yuv2packed2 = yuv2bgra32_full_2_msa;
                    c->yuv2packed1 = yuv2bgra32_full_1_msa;
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packedX = yuv2bgrx32_full_X_msa;
                    c->yuv2packed2 = yuv2bgrx32_full_2_msa;
                    c->yuv2packed1 = yuv2bgrx32_full_1_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_ABGR:
#if CONFIG_SMALL
                c->yuv2packedX = yuv2abgr32_full_X_msa;
                c->yuv2packed2 = yuv2abgr32_full_2_msa;
                c->yuv2packed1 = yuv2abgr32_full_1_msa;
#else
#if CONFIG_SWSCALE_ALPHA
                if (c->needAlpha) {
                    c->yuv2packedX = yuv2abgr32_full_X_msa;
                    c->yuv2packed2 = yuv2abgr32_full_2_msa;
                    c->yuv2packed1 = yuv2abgr32_full_1_msa;
                } else
#endif /* CONFIG_SWSCALE_ALPHA */
                {
                    c->yuv2packedX = yuv2xbgr32_full_X_msa;
                    c->yuv2packed2 = yuv2xbgr32_full_2_msa;
                    c->yuv2packed1 = yuv2xbgr32_full_1_msa;
                }
#endif /* !CONFIG_SMALL */
                break;
            case AV_PIX_FMT_RGB24:
                c->yuv2packedX = yuv2rgb24_full_X_msa;
                c->yuv2packed2 = yuv2rgb24_full_2_msa;
                c->yuv2packed1 = yuv2rgb24_full_1_msa;
                break;
            case AV_PIX_FMT_BGR24:
                c->yuv2packedX = yuv2bgr24_full_X_msa;
                c->yuv2packed2 = yuv2bgr24_full_2_msa;
                c->yuv2packed1 = yuv2bgr24_full_1_msa;
                break;
            case AV_PIX_FMT_BGR4_BYTE:
                c->yuv2packedX = yuv2bgr4_byte_full_X_msa;
                c->yuv2packed2 = yuv2bgr4_byte_full_2_msa;
                c->yuv2packed1 = yuv2bgr4_byte_full_1_msa;
                break;
            case AV_PIX_FMT_RGB4_BYTE:
                c->yuv2packedX = yuv2rgb4_byte_full_X_msa;
                c->yuv2packed2 = yuv2rgb4_byte_full_2_msa;
                c->yuv2packed1 = yuv2rgb4_byte_full_1_msa;
                break;
            case AV_PIX_FMT_BGR8:
                c->yuv2packedX = yuv2bgr8_full_X_msa;
                c->yuv2packed2 = yuv2bgr8_full_2_msa;
                c->yuv2packed1 = yuv2bgr8_full_1_msa;
                break;
            case AV_PIX_FMT_RGB8:
                c->yuv2packedX = yuv2rgb8_full_X_msa;
                c->yuv2packed2 = yuv2rgb8_full_2_msa;
                c->yuv2packed1 = yuv2rgb8_full_1_msa;
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

av_cold SwsFunc ff_yuv2rgb_init_mips(SwsContext *c)
{
    int cpu_flags = av_get_cpu_flags();
#if HAVE_MSA
    if (have_msa(cpu_flags)) {
        switch (c->dstFormat) {
            case AV_PIX_FMT_RGB24:
                return yuv420_rgb24_msa;
            case AV_PIX_FMT_BGR24:
                return yuv420_bgr24_msa;
            case AV_PIX_FMT_RGBA:
                if (CONFIG_SWSCALE_ALPHA && isALPHA(c->srcFormat)) {
                    break;
                } else
                    return yuv420_rgba32_msa;
            case AV_PIX_FMT_ARGB:
                if (CONFIG_SWSCALE_ALPHA && isALPHA(c->srcFormat)) {
                    break;
                } else
                    return yuv420_argb32_msa;
            case AV_PIX_FMT_BGRA:
                if (CONFIG_SWSCALE_ALPHA && isALPHA(c->srcFormat)) {
                    break;
                } else
                    return yuv420_bgra32_msa;
            case AV_PIX_FMT_ABGR:
                if (CONFIG_SWSCALE_ALPHA && isALPHA(c->srcFormat)) {
                    break;
                } else
                    return yuv420_abgr32_msa;
        }
    }
    return NULL;
#endif
}
