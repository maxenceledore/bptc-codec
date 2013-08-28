#include "bptc_codec_data.h"
#include "bptc_decode.h"

#ifndef BPTC_BLOCK_PARSER_H
#define BPTC_BLOCK_PARSER_H


extern void
bptc_parse_block(uint8_t *block, block_data *bd);

extern uint8_t
get_block_mode(uint8_t byte0);

extern uint8_t
query_bitfield_size(uint8_t mode, uint8_t property);

uint8_t
query_bitfield_position(uint8_t mode, uint8_t property);

extern uint8_t
extract_partition_bits(uint8_t *block, uint8_t mode);

extern const uint8_t*
get_partition(uint8_t mode, uint8_t pbits);

extern uint8_t
extract_rotation_bits(uint8_t modeset_byte, uint8_t mode);

extern uint8_t
extract_index_selection_bit(uint8_t *block, uint8_t mode);

extern void
extract_color_bits(uint8_t *block, block_data *bd);

extern void
extract_indices(uint8_t *block, uint8_t mode, const uint8_t *pTab,
                uint8_t *idxTab);

#endif /* BPTC_BLOCK_PARSER_H */