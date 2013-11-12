/*
 * Standalone BPTC encoder-decoder
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bptc_decode.h"
#include "bptc_block_parser.h"
#include "util.h"

static void
copy_4x4_rgba8(uint8_t *rgba8_image, const uint8_t *rgba8_44_block, int width) {

  int i = 0;
  int row_size = 4 * sizeof(uint8_t[4]);

  for(i=0 ; i<4 ; i++) {
    
    memcpy(rgba8_image, rgba8_44_block, row_size);

    rgba8_image += width*4;
    rgba8_44_block += row_size;
  }
}


void
bptc_decode_image(uint8_t *block_stream, int width, int height,
                  uint8_t *rgba8_image) {
  block_data bd;

  int num_block = width*height/(BPTC_BLOCK_X*BPTC_BLOCK_Y);
  int i =0;

  if(!block_stream || !rgba8_image)
    return;

  memset(&bd, 0, sizeof(bd));

  for(i=0; i<(num_block); i++) {

    bptc_parse_block(block_stream+(i*BLOCK_SIZE_BYTE), &bd);

    bptc_decode_block(rgba8_image, width, height, bd, i);
    }
}


void
bptc_decode_block(uint8_t *rgba8_image, int width, int height,
                  block_data bd, int block_nr) {
#if 0
  if(bd.mode != 6)
    return;
#endif
  uint8_t rgba8_tile_4x4[BPTC_BLOCK_X*BPTC_BLOCK_Y*4];

  interpolate_color(rgba8_tile_4x4, bd);

  if(bd.rotation_bits)
    swap_color(rgba8_tile_4x4, bd.rotation_bits);

  copy_4x4_rgba8(ub4x4_block_array2d_adress(rgba8_image, width, height, 4, block_nr),
                   rgba8_tile_4x4,
                   width);
}


void
interpolate_color(int8_t *rgba8, block_data bd) {

  const uint8_t *weight = NULL;
  uint8_t i = 0;

  int subset = 0;

  switch(bd.index_bits) {
    case 2:
       weight = interpolation_factor_2bit;
      break;
    case 3:
       weight = interpolation_factor_3bit;
      break;
    case 4:
       weight = interpolation_factor_4bit;
      break;
    default:
       printf("invalid index_bits count (mode=%i)", bd.mode);
       exit(1);
  }

  for(i=0 ; i < 16 ; i++) {

     int8_t wcf = weight[bd.index_table[i]]; /* weightColorFactor */

    if(bd.partition_bits)
      subset = bd.partition_table[i];

    /* see page 91 of INTEL_ivb_ihd_os_vol1_part1.pdf*/
    rgba8[0] = (bd.endpR[subset][0] * (64 - wcf) + bd.endpR[subset][1] * wcf + 32) >> 6;
    rgba8[1] = (bd.endpG[subset][0] * (64 - wcf) + bd.endpG[subset][1] * wcf + 32) >> 6;
    rgba8[2] = (bd.endpB[subset][0] * (64 - wcf) + bd.endpB[subset][1] * wcf + 32) >> 6;
    if(!bd.alpha_bits)
      rgba8[3] = 255;
    else
       rgba8[3] = (bd.endpA[subset][0] * (64 - wcf) + bd.endpA[subset][1] * wcf + 32) >> 6;

    rgba8 += sizeof(uint8_t[4]);
      }
}


void
old_interpolate_color(int8_t *rgba8, block_data bd) {
   
   const uint8_t *table = NULL;
   uint8_t i = 0;
   
   int subset = 0;
   
   switch(bd.index_bits) {
      case 2:
         table = interpolation_factor_2bit;
         break;
      case 3:
         table = interpolation_factor_3bit;
         break;
      case 4:
         table = interpolation_factor_4bit;
         break;
      default:
         return;
   }
   
   for(i=0 ; i<16 ; i++) {
      
      if(bd.partition_bits)
         subset = bd.partition_table[i];
      
      rgba8[0] = bd.endpR[subset][0];
      rgba8[1] = bd.endpG[subset][0];
      rgba8[2] = bd.endpB[subset][0];
      if(!bd.alpha_bits)
         rgba8[3] = 255;
      else
         rgba8[3] = bd.endpA[0][0];
      
      rgba8 += sizeof(uint8_t[4]);
   }
}


void
swap_color(uint8_t *rgba8, uint8_t rotationBits) {

  uint8_t tmp_chan = 0;
  uint8_t pixel_size = sizeof(uint8_t[4]);
  uint8_t i = 0;

  switch(rotationBits) {
    case 1:
      for(i=0 ; i<16 ; i++) {
        tmp_chan = rgba8[3];
        rgba8[3] = rgba8[0];
        rgba8[0] = tmp_chan;
        rgba8 += pixel_size;
      }
      return;
    case 2:
      for(i=0 ; i<16 ; i++) {
      tmp_chan = rgba8[3];
      rgba8[3] = rgba8[1];
      rgba8[1] = tmp_chan;
        rgba8 += pixel_size;
      }
      return;
    case 3:
      for(i=0 ; i<16 ; i++) {
      tmp_chan = rgba8[2];
      rgba8[2] = rgba8[1];
      rgba8[1] = tmp_chan;
        rgba8 += pixel_size;
      }
      return;
    default:
      return;
  }
}
