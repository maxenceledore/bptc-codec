#ifndef BPTC_DECODE_H
#define BPTC_DECODE_H

#include <stdint.h>
#include "bptc_codec_data.h"

static void
copy_4x4_rgba8(uint8_t *rgba8_image, const uint8_t *rgba8_44_block, int width);

extern void
bptc_decode_image(uint8_t *block_stream, int width, int height,
                  uint8_t *rgba8_image);

extern void
bptc_decode_block(uint8_t *rgba8_image, int width, int height,
                  block_data bd, int block_nr);

extern void
interpolate_color(int8_t *rgba8, block_data bd);

extern void
swap_color(uint8_t rgba[4], uint8_t rotationBits);

#endif /* BPTC_DECODE_H */