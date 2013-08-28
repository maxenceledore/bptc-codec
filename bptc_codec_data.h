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

 /*
  * BPTC codec data
  * Many things needed to decode a BPTC block
  */

#ifndef BPTC_CODEC_DATA_H
#define BPTC_CODEC_DATA_H

#include <stdint.h>

#define BPTC_BLOCK_X     4
#define BPTC_BLOCK_Y     4
#define BLOCK_SIZE_BYTE 16


#define MAX_SUBSETS   3
#define NUM_ENDPOINTS 2
#define NUM_CHANNELS  4

#define NUL_INDICES   (BPTC_BLOCK_X*BPTC_BLOCK_y)


#define RGBA8         0
#define SRGB8_ALPHA8  1
#define RGB_SFLOAT16  2
#define RGB_UFLOAT16  3

#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB                 0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB           0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB           0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB         0x8E8F

typedef uint8_t color_data[MAX_SUBSETS][NUM_ENDPOINTS];

typedef struct {

  uint8_t        blockTotallyOpaque; /* every RGBA pixels are all set to 255 */
  uint8_t        fp16_input;

  uint8_t        mode;
  const uint8_t *block_properties;
  uint8_t        partition_bits;      /* drop it when everything's functional */
  const uint8_t *partition_table;
  uint8_t        rotation_bits;
  uint8_t        idx_selection_bit;
  uint8_t        color_bits;
  uint8_t        alpha_bits;
  color_data     endpR;
  color_data     endpG;
  color_data     endpB;
  color_data     endpA;
  uint8_t        index_bits;
  uint8_t        index_table[16];
  uint8_t        secondaryIndexBits;
  uint8_t        secondaryIndexTable[16];
} block_data;


/* 
 * Indices of below block_properties_table 2nd level referencing
 * to ease queries from it.
 */
#define NR_SUBSETS                 0
#define PARTITION_BITS             1
#define ROTATION_IDX_BITS          2
#define IDX_SELECT_BITS            3
#define COLOR_BITS                 4
#define ALPHA_BITS                 5
#define ENDPOINT_PBITS             6
#define SHARED_PBITS               7
#define IDX_BITS_PER_ELT           8
#define SECONDARY_IDX_BITS_PER_ELT 9

static const uint8_t block_properties_table[8][10] = {
/*    nr   part  rot  idx select color alpha endpt  shared  idx    2nd idx
 * subsets bits  bits    bits    bits  bits  pbits  pbits   [bits per elt]
 */
 { 3,       4,    0,      0,      4,    0,     1,     0,      3,     0 },
 { 2,       6,    0,      0,      6,    0,     0,     1,      3,     0 },
 { 3,       6,    0,      0,      5,    0,     0,     0,      2,     0 },
 { 2,       6,    0,      0,      7,    0,     1,     0,      2,     0 },
 { 1,       0,    2,      1,      5,    6,     0,     0,      2,     3 },
 { 1,       0,    2,      0,      7,    8,     0,     0,      2,     2 },
 { 1,       0,    0,      0,      7,    7,     1,     0,      4,     0 },
 { 2,       6,    0,      0,      5,    5,     1,     0,      2,     0 }};

 
/*
 * Partitions tables
 */

static const uint8_t partition_table_2_subsets[] = {
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,
    0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,
    0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,
    0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1,
    0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,
    0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
    0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,
    0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,
    0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0,
    0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0,
    0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,
    0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,
    0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,
    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,
    0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,
    0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,
    0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,
    0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
    0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,
    0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,
    0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,
    0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,
    0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,
    0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,
    0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,
    0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,
    0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0,
    0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0,
    0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,
    0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,
    0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,
    0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,
    0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,
    0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,
    0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,
    0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1,
    0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,
    0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0,
    0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1,
    0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1,
    0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,
    0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1,
    0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,
    0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,
    0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0,
    0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1
};

static const uint8_t partition_table_3_subsets[] = {
    0,0,1,1,0,0,1,1,0,2,2,1,2,2,2,2,
    0,0,0,1,0,0,1,1,2,2,1,1,2,2,2,1,
    0,0,0,0,2,0,0,1,2,2,1,1,2,2,1,1,
    0,2,2,2,0,0,2,2,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,0,1,1,2,2,1,1,2,2,
    0,0,1,1,0,0,1,1,0,0,2,2,0,0,2,2,
    0,0,2,2,0,0,2,2,1,1,1,1,1,1,1,1,
    0,0,1,1,0,0,1,1,2,2,1,1,2,2,1,1,
    0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
    0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,
    0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,
    0,0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,
    0,1,1,2,0,1,1,2,0,1,1,2,0,1,1,2,
    0,1,2,2,0,1,2,2,0,1,2,2,0,1,2,2,
    0,0,1,1,0,1,1,2,1,1,2,2,1,2,2,2,
    0,0,1,1,2,0,0,1,2,2,0,0,2,2,2,0,
    0,0,0,1,0,0,1,1,0,1,1,2,1,1,2,2,
    0,1,1,1,0,0,1,1,2,0,0,1,2,2,0,0,
    0,0,0,0,1,1,2,2,1,1,2,2,1,1,2,2,
    0,0,2,2,0,0,2,2,0,0,2,2,1,1,1,1,
    0,1,1,1,0,1,1,1,0,2,2,2,0,2,2,2,
    0,0,0,1,0,0,0,1,2,2,2,1,2,2,2,1,
    0,0,0,0,0,0,1,1,0,1,2,2,0,1,2,2,
    0,0,0,0,1,1,0,0,2,2,1,0,2,2,1,0,
    0,1,2,2,0,1,2,2,0,0,1,1,0,0,0,0,
    0,0,1,2,0,0,1,2,1,1,2,2,2,2,2,2,
    0,1,1,0,1,2,2,1,1,2,2,1,0,1,1,0,
    0,0,0,0,0,1,1,0,1,2,2,1,1,2,2,1,
    0,0,2,2,1,1,0,2,1,1,0,2,0,0,2,2,
    0,1,1,0,0,1,1,0,2,0,0,2,2,2,2,2,
    0,0,1,1,0,1,2,2,0,1,2,2,0,0,1,1,
    0,0,0,0,2,0,0,0,2,2,1,1,2,2,2,1,
    0,0,0,0,0,0,0,2,1,1,2,2,1,2,2,2,
    0,2,2,2,0,0,2,2,0,0,1,2,0,0,1,1,
    0,0,1,1,0,0,1,2,0,0,2,2,0,2,2,2,
    0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,0,
    0,0,0,0,1,1,1,1,2,2,2,2,0,0,0,0,
    0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,
    0,1,2,0,2,0,1,2,1,2,0,1,0,1,2,0,
    0,0,1,1,2,2,0,0,1,1,2,2,0,0,1,1,
    0,0,1,1,1,1,2,2,2,2,0,0,0,0,1,1,
    0,1,0,1,0,1,0,1,2,2,2,2,2,2,2,2,
    0,0,0,0,0,0,0,0,2,1,2,1,2,1,2,1,
    0,0,2,2,1,1,2,2,0,0,2,2,1,1,2,2,
    0,0,2,2,0,0,1,1,0,0,2,2,0,0,1,1,
    0,2,2,0,1,2,2,1,0,2,2,0,1,2,2,1,
    0,1,0,1,2,2,2,2,2,2,2,2,0,1,0,1,
    0,0,0,0,2,1,2,1,2,1,2,1,2,1,2,1,
    0,1,0,1,0,1,0,1,0,1,0,1,2,2,2,2,
    0,2,2,2,0,1,1,1,0,2,2,2,0,1,1,1,
    0,0,0,2,1,1,1,2,0,0,0,2,1,1,1,2,
    0,0,0,0,2,1,1,2,2,1,1,2,2,1,1,2,
    0,2,2,2,0,1,1,1,0,1,1,1,0,2,2,2,
    0,0,0,2,1,1,1,2,1,1,1,2,0,0,0,2,
    0,1,1,0,0,1,1,0,0,1,1,0,2,2,2,2,
    0,0,0,0,0,0,0,0,2,1,1,2,2,1,1,2,
    0,1,1,0,0,1,1,0,2,2,2,2,2,2,2,2,
    0,0,2,2,0,0,1,1,0,0,1,1,0,0,2,2,
    0,0,2,2,1,1,2,2,1,1,2,2,0,0,2,2,
    0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,
    0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,1,
    0,2,2,2,1,2,2,2,0,2,2,2,1,2,2,2,
    0,1,0,1,2,2,2,2,2,2,2,2,2,2,2,2,
    0,1,1,1,2,0,1,1,2,2,0,1,2,2,2,0
};


/* Color interpolation factor tables */

static const uint8_t
interpolation_factor_2bit[4] ={
  0, 21, 43, 64 };

static const uint8_t
interpolation_factor_3bit[8] ={
   0,  9, 18, 27,
  37, 46, 55, 64 };

static const uint8_t
interpolation_factor_4bit[16] ={
   0,  4,  9, 13,
  17, 21, 26, 30,
  34, 38, 43, 47,
  51, 55, 60, 64 };


/* Anchor index values for the second subset of two-subset partitioning */

static const uint8_t anchor_idx_2nd_ss_2_partition[] = {
  15,15,15,15,15,15,15,15,
  15,15,15,15,15,15,15,15,
  15, 2, 8, 2, 2, 8, 8,15,
   2, 8, 2, 2, 8, 8, 2, 2,
  15,15, 6, 8, 2, 8,15,15,
   2, 8, 2, 2, 2,15,15, 6,
   6, 2, 6, 8,15,15, 2, 2,
  15,15,15,15,15, 2, 2,15
};

/* Anchor index values for the second subset of three-subset partitioning */

static const uint8_t anchor_idx_2nd_ss_3_partition[] = {
   3, 3,15,15, 8, 3,15,15,
   8, 8, 6, 6, 6, 5, 3, 3,
   3, 3, 8,15, 3, 3, 6,10,
   5, 8, 8, 6, 8, 5,15,15,
   8,15, 3, 5, 6,10, 8,15,
  15, 3,15, 5,15,15,15,15,
   3,15, 5, 5, 5, 8, 5,10,
   5,10, 8,13,15,12, 3, 3
};

/* Anchor index values for the third subset of three-subset partitioning */

static const uint8_t anchor_idx_3nd_ss_3_partition[] = {
  15, 8, 8, 3,15,15, 3, 8,
  15,15,15,15,15,15,15, 8,
  15, 8,15, 3,15, 8,15, 8,
   3,15, 6,10,15,15,10, 8,
  15, 3,15,10,10, 8, 9,10,
   6,15, 8,15, 3, 6, 6, 8,
  15, 3,15,15,15,15,15,15,
  15,15,15,15, 3,15,15, 8,
};

#endif /* BPTC_CODEC_DATA_H */
