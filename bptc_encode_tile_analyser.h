/*
 * Standalone BPTC codec
 * Version:  0.1
 *
 * Copyright (C) 2013  Maxence Le Doré   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdint.h>
#include "bptc_codec_data.h"
#include "util.h"

extern void
tile_analysis(uint8_t *image_tile_4x4, block_data *bd);

extern uint8_t* search_matching_partition_pattern (const uint8_t *pattern);

extern void RGB2YUV(float *rgb, float *yuv);
extern void YUV2RGB(float *yuv, float *rgb);
extern void dumpModeMap(block_data *blocks, uint32_t nbx, uint32_t nby);

extern float PSNR(float max, float EQM);
extern float EQM_RGBA8(uint8_t *signal, uint8_t *noise, uint32_t w, uint32_t h);
extern float EQM_RGBA16SF(void);
extern float EQM_RGBA16UF(void);

extern float   U8_to_UNORMfloat (uint8_t u8);
extern uint8_t UNORMfloat_to_U8 (float f);