#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define PNG_DEBUG 3
#include <png.h>

extern uint8_t
extract_bits(uint8_t *block, uint8_t startBit , uint8_t count);

extern void
insert_bits(uint8_t *block, uint8_t startBit , uint8_t count,
            uint8_t value);

extern void*
load_binary_file(const char *path);

extern int8_t*
ub_pixel_array2d_adress(int8_t *array2d, int width, int height,
                        int x, int y);

extern int8_t*
ub4x4_block_array2d_adress(int8_t *array2d,
                           int width, int height, int nr_cmpnt,
                           int block_n);

extern void
write_tga_file(char* file_name, uint16_t w, uint16_t h, uint8_t bpp, void *image);

extern void
read_tga_file(char* file_name, uint16_t *w, uint16_t *h,
              uint8_t *components, uint8_t *bpp, uint8_t **image);

extern void
write_png_file(char* file_name, int width, int height,
               png_byte bit_depth, png_byte color_type, void *image);

extern void
read_png_file(char* file_name);

#endif /* UTIL_H */