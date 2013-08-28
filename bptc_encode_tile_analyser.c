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


#include <stdio.h>
#include <math.h>

#ifndef MAXFLOAT
#define MAXFLOAT 3.40282347e+38F
#endif

#include "bptc_encode_tile_analyser.h"



void tile_analysis(uint8_t *image_tile_4x4, block_data *bd) {

   uint8_t meanTileColor[4] = {0, 0, 0, 0};
   int i = 0, j =0;

   meanTileColor[0] += image_tile_4x4[0];
   meanTileColor[1] += image_tile_4x4[1];
   meanTileColor[2] += image_tile_4x4[2];

   bd->endpR[0][0] = bd->endpR[0][1] = meanTileColor[0];
   bd->endpR[1][0] = bd->endpR[1][1] = meanTileColor[0];
   bd->endpR[2][0] = bd->endpR[2][1] = meanTileColor[0];
   bd->endpG[0][0] = bd->endpG[0][1] = meanTileColor[1];
   bd->endpG[1][0] = bd->endpG[1][1] = meanTileColor[1];
   bd->endpG[2][0] = bd->endpG[2][1] = meanTileColor[1];
   bd->endpB[0][0] = bd->endpB[0][1] = meanTileColor[2];
   bd->endpB[1][0] = bd->endpB[1][1] = meanTileColor[2];
   bd->endpB[2][0] = bd->endpB[2][1] = meanTileColor[2];

   bd->mode = 0;

   return;
}


uint8_t* search_matching_partition_pattern (const uint8_t *pattern) {
   int i = 0, res = ~0;

   while(i <64) {
      uint8_t * p = (uint8_t*) partition_table_2_subsets+(i*16);
      res = memcmp(p, pattern, 16);
      if (!res)
         return p;
   }
   return NULL;
}


void dumpModeMap(block_data *blocks, uint32_t nbx, uint32_t nby) {
   return;
}


/*
 * Peak Signal Noise Ratio function 
 * 
 */
float PSNR(float max, float EQM) {

   if(max < sqrt(MAXFLOAT)) {
      printf("PSNR : max > sqrt(MAXFLOAT). Aborting eval\n");
      return 0.0f;
   }

   return 10*log10(max*max/EQM);
}


float
EQM_RGBA8(uint8_t *signal, uint8_t *noise, uint32_t w, uint32_t h) {

   uint8_t *signal_ptr = signal, *noise_ptr = noise;

   float sum = 0, v4_diff[4];

   int i = 0, j = 0;

   for(i = 0 ; i < w ; i++)
      for(j = 0 ; j < h ; j++) {

         v4_diff[0] = signal_ptr[0]-noise_ptr[0];
         v4_diff[1] = signal_ptr[1]-noise_ptr[1];
         v4_diff[2] = signal_ptr[2]-noise_ptr[2];
         v4_diff[3] = signal_ptr[3]-noise_ptr[3];

         sum += v4_diff[0]*v4_diff[0]
         +v4_diff[1]*v4_diff[1]
         +v4_diff[2]*v4_diff[2]
         +v4_diff[3]*v4_diff[3];

         signal_ptr+=4;
         noise_ptr+=4;
      }

      return sum / w*h;
}

void RGB2YUV(float *rgb, float *yuv) {

   if(!rgb || !yuv)
      return;

   yuv[0] = .299f*rgb[0] + .58700f*rgb[1] + .114f*rgb[2];
   yuv[1] = .492f*(rgb[2]-yuv[0]);
   yuv[2] = .877f*(rgb[0]-yuv[0]);
}


void YUV2RGB(float *yuv, float *rgb) {

   if(!yuv || !rgb)
      return;

   rgb[0] = yuv[0] + 1.13983f*yuv[2];
   rgb[1] = yuv[0] -  .39465f*yuv[1] - .58060f*yuv[2];
   rgb[2] = yuv[0] + 2.03211f*yuv[1];
}

float U8_to_UNORMfloat (uint8_t u8) {
   return ((float) u8) / 255.f;
}

uint8_t UNORMfloat_to_U8 (float f) {
   if(f > 1.f)
      f = 1.f;
   if(f < .0f)
      f = .0f;

   return (uint8_t)(f*255.f);
}

float veclength(uint8_t *v, uint8_t n) {

}

uint8_t vecdiff(uint8_t *u, uint8_t *v, uint8_t *w, uint8_t n) {

   int i = 0;

   for(i=0 ; i < n; i++)
      w[i] = u[i] - v[i];
}
