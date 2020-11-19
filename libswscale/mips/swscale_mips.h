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

void ff_hscale_8_to_19_msa(SwsContext *c, int16_t *_dst, int dstW,
                           const uint8_t *src, const int16_t *filter,
                           const int32_t *filterPos, int filterSize);

void ff_hscale_16_to_19_msa(SwsContext *c, int16_t *_dst, int dstW,
                            const uint8_t *_src, const int16_t *filter,
                            const int32_t *filterPos, int filterSize);

void ff_hscale_16_to_15_msa(SwsContext *c, int16_t *dst, int dstW,
                            const uint8_t *_src, const int16_t *filter,
                            const int32_t *filterPos, int filterSize);

void ff_yuv2planeX_8_msa(const int16_t *filter, int filterSize,
                         const int16_t **src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2rgbx32_1_X_msa(SwsContext *c, const int16_t *lumFilter,
                        const int16_t **lumSrc, int lumFilterSize,
                        const int16_t *chrFilter, const int16_t **chrUSrc,
                        const int16_t **chrVSrc, int chrFilterSize,
                        const int16_t **alpSrc, uint8_t *dest, int dstW,
                        int y);

void yuv2rgbx32_1_2_msa(SwsContext *c, const int16_t *buf[2],
                        const int16_t *ubuf[2], const int16_t *vbuf[2],
                        const int16_t *abuf[2], uint8_t *dest, int dstW,
                        int yalpha, int uvalpha, int y);

void yuv2rgbx32_1_1_msa(SwsContext *c, const int16_t *buf0,
                        const int16_t *ubuf[2], const int16_t *vbuf[2],
                        const int16_t *abuf0, uint8_t *dest, int dstW,
                        int uvalpha, int y);

void yuv2rgbx32_X_msa(SwsContext *c, const int16_t *lumFilter,
                      const int16_t **lumSrc, int lumFilterSize,
                      const int16_t *chrFilter, const int16_t **chrUSrc,
                      const int16_t **chrVSrc, int chrFilterSize,
                      const int16_t **alpSrc, uint8_t *dest, int dstW,
                      int y);

void yuv2rgbx32_2_msa(SwsContext *c, const int16_t *buf[2],
                      const int16_t *ubuf[2], const int16_t *vbuf[2],
                      const int16_t *abuf[2], uint8_t *dest, int dstW,
                      int yalpha, int uvalpha, int y);

void yuv2rgbx32_1_msa(SwsContext *c, const int16_t *buf0,
                      const int16_t *ubuf[2], const int16_t *vbuf[2],
                      const int16_t *abuf0, uint8_t *dest, int dstW,
                      int uvalpha, int y);

void yuv2rgb16_2_msa(SwsContext *c, const int16_t *buf[2],
                     const int16_t *ubuf[2], const int16_t *vbuf[2],
                     const int16_t *abuf[2], uint8_t *dest, int dstW,
                     int yalpha, int uvalpha, int y);

void yuv2rgb16_1_msa(SwsContext *c, const int16_t *buf0,
                     const int16_t *ubuf[2], const int16_t *vbuf[2],
                     const int16_t *abuf0, uint8_t *dest, int dstW,
                     int uvalpha, int y);

void yuv2rgb16_X_msa(SwsContext *c, const int16_t *lumFilter,
                     const int16_t **lumSrc, int lumFilterSize,
                     const int16_t *chrFilter, const int16_t **chrUSrc,
                     const int16_t **chrVSrc, int chrFilterSize,
                     const int16_t **alpSrc, uint8_t *dest, int dstW,
                     int y);

void yuv2bgra32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2bgra32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2bgra32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2abgr32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2abgr32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2abgr32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2rgba32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2rgba32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2rgba32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2argb32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2argb32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2argb32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2bgrx32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2bgrx32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2bgrx32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2xbgr32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2xbgr32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2xbgr32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2rgbx32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2rgbx32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2rgbx32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2xrgb32_full_2_msa(SwsContext *c, const int16_t *buf[2],
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf[2], uint8_t *dest, int dstW,
                           int yalpha, int uvalpha, int y);

void yuv2xrgb32_full_1_msa(SwsContext *c, const int16_t *buf0,
                           const int16_t *ubuf[2], const int16_t *vbuf[2],
                           const int16_t *abuf0, uint8_t *dest, int dstW,
                           int uvalpha, int y);

void yuv2xrgb32_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                           const int16_t **lumSrc, int lumFilterSize,
                           const int16_t *chrFilter, const int16_t **chrUSrc,
                           const int16_t **chrVSrc, int chrFilterSize,
                           const int16_t **alpSrc, uint8_t *dest, int dstW,
                           int y);

void yuv2bgr24_full_2_msa(SwsContext *c, const int16_t *buf[2],
                          const int16_t *ubuf[2], const int16_t *vbuf[2],
                          const int16_t *abuf[2], uint8_t *dest, int dstW,
                          int yalpha, int uvalpha, int y);

void yuv2bgr24_full_1_msa(SwsContext *c, const int16_t *buf0,
                          const int16_t *ubuf[2], const int16_t *vbuf[2],
                          const int16_t *abuf0, uint8_t *dest, int dstW,
                          int uvalpha, int y);

void yuv2bgr24_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                          const int16_t **lumSrc, int lumFilterSize,
                          const int16_t *chrFilter, const int16_t **chrUSrc,
                          const int16_t **chrVSrc, int chrFilterSize,
                          const int16_t **alpSrc, uint8_t *dest, int dstW,
                          int y);

void yuv2rgb24_full_2_msa(SwsContext *c, const int16_t *buf[2],
                          const int16_t *ubuf[2], const int16_t *vbuf[2],
                          const int16_t *abuf[2], uint8_t *dest, int dstW,
                          int yalpha, int uvalpha, int y);

void yuv2rgb24_full_1_msa(SwsContext *c, const int16_t *buf0,
                          const int16_t *ubuf[2], const int16_t *vbuf[2],
                          const int16_t *abuf0, uint8_t *dest, int dstW,
                          int uvalpha, int y);

void yuv2rgb24_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                          const int16_t **lumSrc, int lumFilterSize,
                          const int16_t *chrFilter, const int16_t **chrUSrc,
                          const int16_t **chrVSrc, int chrFilterSize,
                          const int16_t **alpSrc, uint8_t *dest, int dstW,
                          int y);

void yuv2bgr4_byte_full_2_msa(SwsContext *c, const int16_t *buf[2],
                              const int16_t *ubuf[2], const int16_t *vbuf[2],
                              const int16_t *abuf[2], uint8_t *dest, int dstW,
                              int yalpha, int uvalpha, int y);

void yuv2bgr4_byte_full_1_msa(SwsContext *c, const int16_t *buf0,
                              const int16_t *ubuf[2], const int16_t *vbuf[2],
                              const int16_t *abuf0, uint8_t *dest, int dstW,
                              int uvalpha, int y);

void yuv2bgr4_byte_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                              const int16_t **lumSrc, int lumFilterSize,
                              const int16_t *chrFilter, const int16_t **chrUSrc,
                              const int16_t **chrVSrc, int chrFilterSize,
                              const int16_t **alpSrc, uint8_t *dest, int dstW,
                              int y);

void yuv2rgb4_byte_full_2_msa(SwsContext *c, const int16_t *buf[2],
                              const int16_t *ubuf[2], const int16_t *vbuf[2],
                              const int16_t *abuf[2], uint8_t *dest, int dstW,
                              int yalpha, int uvalpha, int y);

void yuv2rgb4_byte_full_1_msa(SwsContext *c, const int16_t *buf0,
                              const int16_t *ubuf[2], const int16_t *vbuf[2],
                              const int16_t *abuf0, uint8_t *dest, int dstW,
                              int uvalpha, int y);

void yuv2rgb4_byte_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                              const int16_t **lumSrc, int lumFilterSize,
                              const int16_t *chrFilter, const int16_t **chrUSrc,
                              const int16_t **chrVSrc, int chrFilterSize,
                              const int16_t **alpSrc, uint8_t *dest, int dstW,
                              int y);

void yuv2bgr8_full_2_msa(SwsContext *c, const int16_t *buf[2],
                         const int16_t *ubuf[2], const int16_t *vbuf[2],
                         const int16_t *abuf[2], uint8_t *dest, int dstW,
                         int yalpha, int uvalpha, int y);

void yuv2bgr8_full_1_msa(SwsContext *c, const int16_t *buf0,
                         const int16_t *ubuf[2], const int16_t *vbuf[2],
                         const int16_t *abuf0, uint8_t *dest, int dstW,
                         int uvalpha, int y);

void yuv2bgr8_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                         const int16_t **lumSrc, int lumFilterSize,
                         const int16_t *chrFilter, const int16_t **chrUSrc,
                         const int16_t **chrVSrc, int chrFilterSize,
                         const int16_t **alpSrc, uint8_t *dest, int dstW,
                         int y);

void yuv2rgb8_full_2_msa(SwsContext *c, const int16_t *buf[2],
                         const int16_t *ubuf[2], const int16_t *vbuf[2],
                         const int16_t *abuf[2], uint8_t *dest, int dstW,
                         int yalpha, int uvalpha, int y);

void yuv2rgb8_full_1_msa(SwsContext *c, const int16_t *buf0,
                         const int16_t *ubuf[2], const int16_t *vbuf[2],
                         const int16_t *abuf0, uint8_t *dest, int dstW,
                         int uvalpha, int y);

void yuv2rgb8_full_X_msa(SwsContext *c, const int16_t *lumFilter,
                          const int16_t **lumSrc, int lumFilterSize,
                          const int16_t *chrFilter, const int16_t **chrUSrc,
                          const int16_t **chrVSrc, int chrFilterSize,
                          const int16_t **alpSrc, uint8_t *dest, int dstW,
                          int y);

void yuv2plane1_9BE_msa(const int16_t *src, uint8_t *dest, int dstW,
                        const uint8_t *dither, int offset);

void yuv2plane1_9LE_msa(const int16_t *src, uint8_t *dest, int dstW,
                        const uint8_t *dither, int offset);

void yuv2planeX_9BE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                        uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2planeX_9LE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                        uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2plane1_10BE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2plane1_10LE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2planeX_10BE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2planeX_10LE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2plane1_12BE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2plane1_12LE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2planeX_12BE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2planeX_12LE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2plane1_14BE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2plane1_14LE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2planeX_14BE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2planeX_14LE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2plane1_16BE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2plane1_16LE_msa(const int16_t *src, uint8_t *dest, int dstW,
                         const uint8_t *dither, int offset);

void yuv2planeX_16BE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void yuv2planeX_16LE_msa(const int16_t *filter, int filterSize, const int16_t **src,
                         uint8_t *dest, int dstW, const uint8_t *ditherm, int offset);

void planar_rgb_to_uv_msa(uint8_t *_dstU, uint8_t *_dstV, const uint8_t *src[4],
                          int width, int32_t *rgb2yuv);

void planar_rgb_to_y_msa(uint8_t *_dst, const uint8_t *src[4], int width,
                         int32_t *rgb2yuv);

#endif /* SWSCALE_MIPS_SWSCALE_MIPS_H */
