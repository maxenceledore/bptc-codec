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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bptc_encode.h"
#include "bptc_encode_tile_analyser.h"
#include "util.h"


void
bptc_encode_image(uint8_t *pixels, uint8_t components, uint8_t size,
                  int width, int height, uint8_t *bptc_output) {

  uint32_t i = 0, nr_blocks = 0, bw = BPTC_BLOCK_X, bh = BPTC_BLOCK_Y;
  block_data bd;
  uint8_t is_float = 0;

  if(!width || !height)
    return;

  nr_blocks = width*height/(bw*bh);

  for(i=0 ; i < nr_blocks; i++) {

    uint8_t *p = pixels;

    p = ub4x4_block_array2d_adress(p, width, height, size, i);

    tile_analysis(p, &bd);

    bptc_encode_tile(bptc_output+(i*16),
                     bd,
                     is_float);
  }
}


uint8_t bptc_encode_tile(uint8_t *block, block_data bd,
                         bool floating_type) {

  int i = 0;

  if(!block)
    return;

//  if(has_alpha && floating_type)
//    return;

  memset(block, 0, 16);
  uint8_t mode =bd.mode;

  bd.color_bits = block_properties_table[mode][COLOR_BITS];

  bptc_encode_mode(block, mode);
  bptc_encode_partitioning(block, mode, 0 /* partTypeId */);
  bptc_encode_swap_bits(block, mode, 0);
  bptc_encode_idx_select_mode4(block, 0);
  bptc_encode_endpoints(block, bd);

}


uint8_t
bptc_encode_mode(uint8_t *block, uint8_t mode) {

  switch(mode) {
    case 0:
      block[0] = 128;
      break;
    case 1:
      block[0] = 64;
      break;
    case 2:
      block[0] = 32;
      break;
    case 3:
      block[0] = 16;
      break;
    case 4:
      block[0] = 8;
      break;
    case 5:
      block[0] = 4;
      break;
    case 6:
      block[0] = 2;
      break;
    case 7:
      block[0] = 1;
      break;
    default:
      return 0;
  }
  return mode;
}

int8_t
bptc_encode_partitioning(uint8_t *block, uint8_t mode, uint8_t partTypeId) {

  if(partTypeId > 63)
    return 1;

  switch(mode) {
    case 0:
      /* mode 0 has 4 partitions bits and so can't
       * hold a partition id greater than 15.
       */
      if(partTypeId > 15)
        return -1;
      block[1] |= (partTypeId & 0x0f) << 4;
      break;
    case 1:
      block[0] |= (partTypeId & 0x20) >> 5;
      block[1] |= (partTypeId & 0x1f) << 3;
      break;
    case 2:
      block[0] |= (partTypeId & 0x30) >> 4;
      block[1] |= (partTypeId & 0x0f) << 4;
      break;
    case 3:
      block[0] |= (partTypeId & 0x38) >> 3;
      block[1] |= (partTypeId & 0x07) << 5;
      break;
    case 7:
      block[0] |= (partTypeId) << 1;
      break;
    default:
      return 1;
  }
  return 0;
}


int8_t
bptc_encode_swap_bits(uint8_t *block, uint8_t mode, uint8_t swapMode) {

  if(swapMode > 3)
    return 1;

  switch(mode) {
    case 4:
      block[0] |= swapMode << 2;
      break;
    case 5:
      block[0] |= swapMode << 3;
      break;
    default:
      return 1;
  }
  return 0;
}


int8_t
bptc_encode_idx_select_mode4(uint8_t *block, uint8_t select) {

  return;
}


int8_t
bptc_encode_endpoints(uint8_t *block, block_data bd) {

  uint8_t colorBits = bd.color_bits;
  uint8_t ColorMSBitsMask = 255 << (8 - colorBits);

  switch(1) {
    case 0:
    insert_bits(block, 6, colorBits, (bd.endpR[0][0] >> 8-colorBits));
    insert_bits(block,10, colorBits, (bd.endpR[0][1] >> 8-colorBits));
    insert_bits(block,14, colorBits, (bd.endpR[1][0] >> 8-colorBits));
    insert_bits(block,18, colorBits, (bd.endpR[1][1] >> 8-colorBits));
    insert_bits(block,22, colorBits, (bd.endpR[2][0] >> 8-colorBits));
    insert_bits(block,26, colorBits, (bd.endpR[2][1] >> 8-colorBits));
    insert_bits(block,30, colorBits, (bd.endpG[0][0] >> 8-colorBits));
    insert_bits(block,34, colorBits, (bd.endpG[0][1] >> 8-colorBits));
    insert_bits(block,38, colorBits, (bd.endpG[1][0] >> 8-colorBits));
    insert_bits(block,42, colorBits, (bd.endpG[1][1] >> 8-colorBits));
    insert_bits(block,46, colorBits, (bd.endpG[2][0] >> 8-colorBits));
    insert_bits(block,50, colorBits, (bd.endpG[2][1] >> 8-colorBits));
    insert_bits(block,54, colorBits, (bd.endpB[0][0] >> 8-colorBits));
    insert_bits(block,58, colorBits, (bd.endpB[0][1] >> 8-colorBits));
    insert_bits(block,62, colorBits, (bd.endpB[1][0] >> 8-colorBits));
    insert_bits(block,66, colorBits, (bd.endpB[1][1] >> 8-colorBits));
    insert_bits(block,70, colorBits, (bd.endpB[2][0] >> 8-colorBits));
    insert_bits(block,74, colorBits, (bd.endpB[2][1] >> 8-colorBits));
    break;
    case 1:
    insert_bits(block, 9, colorBits, (bd.endpR[0][0] >> 8-colorBits));
    insert_bits(block,15, colorBits, (bd.endpR[0][1] >> 8-colorBits));
    insert_bits(block,21, colorBits, (bd.endpR[1][0] >> 8-colorBits));
    insert_bits(block,27, colorBits, (bd.endpR[1][1] >> 8-colorBits));
    insert_bits(block,33, colorBits, (bd.endpG[0][0] >> 8-colorBits));
    insert_bits(block,39, colorBits, (bd.endpG[0][1] >> 8-colorBits));
    insert_bits(block,45, colorBits, (bd.endpG[1][0] >> 8-colorBits));
    insert_bits(block,51, colorBits, (bd.endpG[1][1] >> 8-colorBits));
    insert_bits(block,57, colorBits, (bd.endpB[0][0] >> 8-colorBits));
    insert_bits(block,63, colorBits, (bd.endpB[0][1] >> 8-colorBits));
    insert_bits(block,69, colorBits, (bd.endpB[1][0] >> 8-colorBits));
    insert_bits(block,75, colorBits, (bd.endpB[1][1] >> 8-colorBits));
    break;
    case 3:
    break;
    case 6:
    block[0] |= (bd.endpR[0][0] & ColorMSBitsMask) >> 2;
    block[1] |= (bd.endpR[0][0] & ColorMSBitsMask) << 6;
    block[1] |= (bd.endpR[0][1] & ColorMSBitsMask) >> 1;
    break;
    case 2:
    case 4:
    case 5:
    case 7:
    default:
      return 1;
  }
  return 0;
}


int8_t
bptc_encode_indices(uint8_t *block, block_data bd) {

  switch(bd.mode) {
    case 6:
    break;
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
    default:
      return 1;
  }
  return 0;
}
