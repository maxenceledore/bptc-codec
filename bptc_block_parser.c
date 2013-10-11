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
#include <stdlib.h>
#include "bptc_block_parser.h"
#include "util.h"

void
bptc_parse_block(uint8_t *block, block_data *bd) {

  int i = 0;

  if(!bd)
    return;

  bd->mode = get_block_mode(block[0]);

  bd->rotation_bits = extract_rotation_bits(block[0], bd->mode);

  bd->partition_bits = extract_partition_bits(block, bd->mode);

  bd->partition_table = get_partition(bd->mode, bd->partition_bits);

  bd->color_bits = query_bitfield_size(bd->mode, COLOR_BITS);
  bd->alpha_bits = query_bitfield_size(bd->mode, ALPHA_BITS);

  bd->idx_selection_bit = extract_index_selection_bit(block, bd->mode);

  bd->index_bits = query_bitfield_size(bd->mode, IDX_BITS_PER_ELT);

  extract_endpoints_bits(block, bd);
  extract_indices(block, bd->mode, bd->partition_table, bd->index_table);
}


uint8_t
get_block_mode(uint8_t byte0) {

  int mode_number = 0, i;
  uint8_t mask = 0x80;

  for(i=0; i<8; i++) {
    if(byte0 & mask)
      return mode_number;
    byte0 <<= 1;
    mode_number++;
  }
}

uint8_t
query_bitfield_size(uint8_t mode, uint8_t property) {

    return block_properties_table[mode][property];
}

uint8_t
query_bitfield_position(uint8_t mode, uint8_t property) {

  uint8_t bf_pos = 0;

  while(--property) {
    switch(property) {
      case PARTITION_BITS:
          bf_pos += block_properties_table[mode][PARTITION_BITS];
        break;
      case ROTATION_IDX_BITS:
          bf_pos += block_properties_table[mode][ROTATION_IDX_BITS];
        break;
      case IDX_SELECT_BITS:
          bf_pos += block_properties_table[mode][IDX_SELECT_BITS];
        break;
      case COLOR_BITS:
          bf_pos +=
          block_properties_table[mode][COLOR_BITS]*
          block_properties_table[mode][NR_SUBSETS]*2*3;
        break;
      case ALPHA_BITS:
          bf_pos +=
          block_properties_table[mode][ALPHA_BITS]*
          block_properties_table[mode][NR_SUBSETS]*2;
        break;
      case IDX_BITS_PER_ELT:
          bf_pos +=
          block_properties_table[mode][IDX_BITS_PER_ELT]*16;
          break;
      case SECONDARY_IDX_BITS_PER_ELT:
          bf_pos +=
          block_properties_table[mode][SECONDARY_IDX_BITS_PER_ELT]*16;
          break;
      default:
        return bf_pos;
    }
  }

  return bf_pos;
}


const uint8_t*
get_partition(uint8_t mode, uint8_t pbits) {
  
  if(mode > 3 && mode < 7)
    return NULL;

  if(mode == 0 || mode == 2)
    return partition_table_3_subsets+(pbits*16);

  return partition_table_2_subsets+(pbits*16);
}


/* rotation bits are a pair of bits and only
 * setup with modes 4 and 5 which both don't define any
 * partition bits. So they are always encoded following 
 * immediately the block mode bit in the same byte as
 * the position of the bit mode for mode 4 and 5 let enough
 * space for it.
 */
uint8_t
extract_rotation_bits(uint8_t modeset_byte, uint8_t mode) {

  if(mode == 4)
    return (uint8_t)((modeset_byte & 0x12) >> 2);

  if(mode == 5)
    return (uint8_t)((modeset_byte & 0x18) >> 3);

  return 0;
}

uint8_t
extract_partition_bits(uint8_t *block, uint8_t mode) {

  /* modes 4, 5 and 6 have only one subset 
   * (zero partition bits)
   */
  if(mode > 3 && mode < 7)
    return 0;

  if(mode == 0)
    return (uint8_t)(block[1] & 0xF0);

  if(mode == 1)
    return (uint8_t)( ((block[0] & 0x01) << 5) | ((block[1] & 0xF8) >> 3));

  if(mode == 2)
    return (uint8_t)( ((block[0] & 0x03) << 4) | ((block[1] & 0xF0) >> 4));

  if(mode == 3)
    return (uint8_t)( ((block[0] & 0x07) << 3) | ((block[1] & 0xE0) >> 5));

  if(mode == 7)
    return (uint8_t)( (block[0] & 0x74) >> 1);
}


uint8_t
extract_index_selection_bit(uint8_t *block, uint8_t mode) {

  if(mode != 4)
    return 0;

  return (uint8_t)( (block[0] & 0x02) >> 1);
}


/* http://www.reedbeta.com/blog/2012/02/12/understanding-bcn-texture-compression-formats/ */

void
extract_endpoints_bits(uint8_t *block, block_data *bd) {

  int i = 0;
  uint8_t endpoints[2*8];
  switch(bd->mode) {
    case 0:
      bd->endpR[0][0] = extract_bits(block,  6 , 4) << 4;
      bd->endpR[0][1] = extract_bits(block, 10 , 4) << 4;
      bd->endpR[1][0] = extract_bits(block, 14 , 4) << 4;
      bd->endpR[1][1] = extract_bits(block, 18 , 4) << 4;
      bd->endpR[2][0] = extract_bits(block, 22 , 4) << 4;
      bd->endpR[2][1] = extract_bits(block, 26 , 4) << 4;
      bd->endpG[0][0] = extract_bits(block, 30 , 4) << 4;
      bd->endpG[0][1] = extract_bits(block, 34 , 4) << 4;
      bd->endpG[1][0] = extract_bits(block, 38 , 4) << 4;
      bd->endpG[1][1] = extract_bits(block, 42 , 4) << 4;
      bd->endpG[2][0] = extract_bits(block, 46 , 4) << 4;
      bd->endpG[2][1] = extract_bits(block, 50 , 4) << 4;
      bd->endpB[0][0] = extract_bits(block, 54 , 4) << 4;
      bd->endpB[0][1] = extract_bits(block, 58 , 4) << 4;
      bd->endpB[1][0] = extract_bits(block, 62 , 4) << 4;
      bd->endpB[1][1] = extract_bits(block, 66 , 4) << 4;
      bd->endpB[2][0] = extract_bits(block, 70 , 4) << 4;
      bd->endpB[2][1] = extract_bits(block, 74 , 4) << 4;
      break;
    case 1:
      bd->endpR[0][0] = extract_bits(block,  9, 6) << 2;
      bd->endpR[0][1] = extract_bits(block, 15, 6) << 2;
      bd->endpR[1][0] = extract_bits(block, 21, 6) << 2;
      bd->endpR[1][1] = extract_bits(block, 27, 6) << 2;
      bd->endpG[0][0] = extract_bits(block, 33, 6) << 2;
      bd->endpG[0][1] = extract_bits(block, 39, 6) << 2;
      bd->endpG[1][0] = extract_bits(block, 45, 6) << 2;
      bd->endpG[1][1] = extract_bits(block, 51, 6) << 2;
      bd->endpB[0][0] = extract_bits(block, 57, 6) << 2;
      bd->endpB[0][1] = extract_bits(block, 63, 6) << 2;
      bd->endpB[1][0] = extract_bits(block, 69, 6) << 2;
      bd->endpB[1][1] = extract_bits(block, 75, 6) << 2;
      break;
    case 2:
      bd->endpR[0][0] = extract_bits(block, 10, 5) << 3;
      bd->endpR[0][1] = extract_bits(block, 15, 5) << 3;
      bd->endpR[1][0] = extract_bits(block, 20, 5) << 3;
      bd->endpR[1][1] = extract_bits(block, 25, 5) << 3;
      bd->endpR[2][0] = extract_bits(block, 30, 5) << 3;
      bd->endpR[2][1] = extract_bits(block, 35, 5) << 3;
      bd->endpG[0][0] = extract_bits(block, 40, 5) << 3;
      bd->endpG[0][1] = extract_bits(block, 45, 5) << 3;
      bd->endpG[1][0] = extract_bits(block, 50, 5) << 3;
      bd->endpG[1][1] = extract_bits(block, 55, 5) << 3;
      bd->endpG[2][0] = extract_bits(block, 60, 5) << 3;
      bd->endpG[2][1] = extract_bits(block, 65, 5) << 3;
      bd->endpB[0][0] = extract_bits(block, 70, 5) << 3;
      bd->endpB[0][1] = extract_bits(block, 75, 5) << 3;
      bd->endpB[1][0] = extract_bits(block, 80, 5) << 3;
      bd->endpB[1][1] = extract_bits(block, 85, 5) << 3;
      bd->endpB[2][0] = extract_bits(block, 90, 5) << 3;
      bd->endpB[2][1] = extract_bits(block, 95, 5) << 3;
      break;
    case 3:
      bd->endpR[0][0] = extract_bits(block, 11, 7) << 1;
      bd->endpR[0][1] = extract_bits(block, 18, 7) << 1;
      bd->endpR[1][0] = extract_bits(block, 25, 7) << 1;
      bd->endpR[1][1] = extract_bits(block, 32, 7) << 1;
      bd->endpG[0][0] = extract_bits(block, 39, 7) << 1;
      bd->endpG[0][1] = extract_bits(block, 46, 7) << 1;
      bd->endpG[1][0] = extract_bits(block, 53, 7) << 1;
      bd->endpG[1][1] = extract_bits(block, 60, 7) << 1;
      bd->endpB[0][0] = extract_bits(block, 67, 7) << 1;
      bd->endpB[0][1] = extract_bits(block, 74, 7) << 1;
      bd->endpB[1][0] = extract_bits(block, 81, 7) << 1;
      bd->endpB[1][1] = extract_bits(block, 88, 7) << 1;
      break;
    case 4:
      bd->endpR[0][0] = extract_bits(block,  9, 5) << 3;
      bd->endpR[0][1] = extract_bits(block, 14, 5) << 3;
      bd->endpG[0][0] = extract_bits(block, 19, 5) << 3;
      bd->endpG[0][1] = extract_bits(block, 24, 5) << 3;
      bd->endpB[0][0] = extract_bits(block, 29, 5) << 3;
      bd->endpB[0][1] = extract_bits(block, 34, 5) << 3;
      bd->endpA[0][0] = extract_bits(block, 39, 6) << 2;
      bd->endpA[0][1] = extract_bits(block, 45, 6) << 2;
      break;
    case 5:
      bd->endpR[0][0] = extract_bits(block,  9, 7) << 1;
      bd->endpR[0][1] = extract_bits(block, 16, 7) << 1;
      bd->endpG[0][0] = extract_bits(block, 21, 7) << 1;
      bd->endpG[0][1] = extract_bits(block, 28, 7) << 1;
      bd->endpB[0][0] = extract_bits(block, 35, 7) << 1;
      bd->endpB[0][1] = extract_bits(block, 42, 7) << 1;
      bd->endpA[0][0] = extract_bits(block, 49, 8);
      bd->endpA[0][1] = extract_bits(block, 57, 8);
      break;
    case 6:
      bd->endpR[0][0] = extract_bits(block,  8, 7) << 1;
      bd->endpR[0][1] = extract_bits(block, 15, 7) << 1;
      bd->endpG[0][0] = extract_bits(block, 22, 7) << 1;
      bd->endpG[0][1] = extract_bits(block, 29, 7) << 1;
      bd->endpB[0][0] = extract_bits(block, 36, 7) << 1;
      bd->endpB[0][1] = extract_bits(block, 43, 7) << 1;
      bd->endpA[0][0] = extract_bits(block, 50, 7) << 1;
      bd->endpA[0][0] = extract_bits(block, 57, 7) << 1;
      break;
    case 7:
      bd->endpR[0][0] = extract_bits(block, 15, 5) << 3;
      bd->endpR[0][1] = extract_bits(block, 20, 5) << 3;
      bd->endpR[1][0] = extract_bits(block, 25, 5) << 3;
      bd->endpR[1][1] = extract_bits(block, 30, 5) << 3;
      bd->endpG[0][0] = extract_bits(block, 35, 5) << 3;
      bd->endpG[0][1] = extract_bits(block, 40, 5) << 3;
      bd->endpG[1][0] = extract_bits(block, 45, 5) << 3;
      bd->endpG[1][1] = extract_bits(block, 50, 5) << 3;
      bd->endpB[0][0] = extract_bits(block, 55, 5) << 3;
      bd->endpB[0][1] = extract_bits(block, 60, 5) << 3;
      bd->endpB[1][0] = extract_bits(block, 65, 5) << 3;
      bd->endpB[1][1] = extract_bits(block, 70, 5) << 3;
      bd->endpA[0][0] = extract_bits(block, 75, 5) << 3;
      bd->endpA[0][1] = extract_bits(block, 80, 5) << 3;
      bd->endpA[1][0] = extract_bits(block, 85, 5) << 3;
      bd->endpA[1][1] = extract_bits(block, 90, 5) << 3;
      break;
    default:
      break;
  }
}


void
extract_indices(uint8_t *block, uint8_t mode, const uint8_t *pTab,
                uint8_t * idxTab) {

  int i = 0, part1reached = 0, part2reached = 0;

#define PARTITION_REACHED_CHECKS(pTab, part1reached, part2reached)   \
if(0) {                                                              \
}

  switch(mode) {
    case 0:
    case 1:
    case 2:
    case 3:
       part1reached = 0, part2reached = 0;
       memset(idxTab, 0, 16);
       return;
    case 4:
       idxTab[0] =   (block[ 9] & 0x20) >> 5;
       idxTab[1] =  ((block[ 9] & 0x10) | block[9] & 0x08) >> 3;
       idxTab[2] =   (block[ 9] & 0x06) >> 1;
       idxTab[3] = (((block[ 9] & 0x01) << 4) | block[10] & 0x80) >> 7;
       idxTab[4] =   (block[10] & 0x60) >> 5;
       idxTab[5] =   (block[10] & 0x18) >> 3;
       idxTab[6] =   (block[10] & 0x06) >> 1;
       idxTab[7] = (((block[10] & 0x01) << 4) | block[11] & 0x80) >> 7;
       idxTab[8] =    block[11] & 0x60;
       break;
    case 5:
       idxTab[0] =    (block[ 8] & 0x40) >> 6;
       idxTab[1] =    (block[ 8] & 0x30) >> 4;
       idxTab[2] =    (block[ 8] & 0x0c) >> 2;
       idxTab[3] =    (block[ 8] & 0x03);
       idxTab[4] =    (block[ 9] & 0xc0) >> 6;
       idxTab[5] =    (block[ 9] & 0x30) >> 4;
       idxTab[6] =    (block[ 9] & 0x0c) >> 2;
       idxTab[7] =    (block[ 9] & 0x03);
       idxTab[8] =    (block[10] & 0xc0) >> 6;
       idxTab[9] =    (block[10] & 0x30) >> 4;
       idxTab[10] =   (block[10] & 0x0c) >> 2;
       idxTab[11] =   (block[10] & 0x03);
       break;
    case 6:
       idxTab[ 0] =   ((block[ 8] & 0x70) >> 3) | ((block[ 8] & 0x08) >> 3);
       idxTab[ 1] =   ((block[ 8] & 0x07) << 1) | ((block[ 9] & 0x80) >> 7);
       idxTab[ 2] =   ((block[ 9] & 0x70) >> 3) | ((block[ 9] & 0x08) >> 3);
       idxTab[ 3] =   ((block[ 9] & 0x07) << 1) | ((block[10] & 0x80) >> 7);
       idxTab[ 4] =   ((block[10] & 0x70) >> 3) | ((block[10] & 0x08) >> 3);
       idxTab[ 5] =   ((block[10] & 0x07) << 1) | ((block[11] & 0x80) >> 7);
       idxTab[ 6] =   ((block[11] & 0x70) >> 3) | ((block[11] & 0x08) >> 3);
       idxTab[ 7] =   ((block[11] & 0x07) << 1) | ((block[12] & 0x80) >> 7);
       idxTab[ 8] =   ((block[12] & 0x70) >> 3) | ((block[12] & 0x08) >> 3);
       idxTab[ 9] =   ((block[12] & 0x07) << 1) | ((block[13] & 0x80) >> 7);
       idxTab[10] =   ((block[13] & 0x70) >> 3) | ((block[13] & 0x08) >> 3);
       idxTab[11] =   ((block[13] & 0x07) << 1) | ((block[14] & 0x80) >> 7);
       idxTab[12] =   ((block[14] & 0x70) >> 3) | ((block[14] & 0x08) >> 3);
       idxTab[13] =   ((block[14] & 0x07) << 1) | ((block[15] & 0x80) >> 7);
       idxTab[14] =   ((block[15] & 0x70) >> 3) | ((block[15] & 0x08) >> 3);
       idxTab[15] =   ((block[15] & 0x07) << 1) | ((block[16] & 0x80) >> 7);
    case 7:
       /* if(pTab[0] == 1) */ /* every upper left pixel is in partition 0 */
       idxTab[ 0] =    (block[ 8] & 0x20) >> 5;
       break;
    default:
       break;
  }
}
