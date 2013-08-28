#ifndef BPTC_ENCODE_H
#define BPTC_ENCODE_H

#include <stdint.h>
#include <stdbool.h>

#include "bptc_codec_data.h"

extern void
bptc_encode_image(uint8_t *pixels, uint8_t components, uint8_t size,
                  int width, int height, uint8_t *bptc_output);

extern uint8_t bptc_encode_tile(uint8_t *block, block_data bd,
                                bool fp_src);

extern uint8_t
bptc_encode_mode(uint8_t *block, uint8_t mode);

extern int8_t
bptc_encode_partitioning(uint8_t *block, uint8_t mode, uint8_t partTypeId);

extern int8_t
bptc_encode_swap_bits(uint8_t *block, uint8_t mode, uint8_t swapMode);

extern int8_t
bptc_encode_idx_select_mode4(uint8_t *block, uint8_t select);

extern int8_t
bptc_encode_endpoints(uint8_t *block, block_data bd);

extern int8_t
bptc_encode_indices(uint8_t *block, block_data bd);

#endif /* BPTC_ENCODE_H */