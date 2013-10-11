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
 * Parts of this code are licensied under the X11 licence from :
 * 
 * http://zarb.org/~gc/html/libpng.html
 * 
 * Copyright 2002-2010 Guillaume Cottenceau.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "bptc_codec_data.h"
#include "util.h"


uint8_t extract_bits(uint8_t *block, uint8_t startBit , uint8_t count) {
   
   int i = 0;
   uint8_t inBytePos = startBit / 8, bitPos = startBit % 8;

   uint32_t bitCountMask = 0;
   
   while(i < count) {
      bitCountMask <<= 1;
      bitCountMask |= 0x1;
      i++;
   };

   return ((bitCountMask << 8-(bitPos+count)) & block[inBytePos]) >> (8-(bitPos+count)) ;
}


int8_t*
ub_pixel_array2d_adress(int8_t *array2d, int width, int height,
                        int x, int y) {
  return array2d + (width *y + x) * 4;
}


int8_t*
ub4x4_block_array2d_adress(int8_t *array2d,
                           int width, int height, int nr_cmpnt,
                           int block_n) {

  int x_block_idx = 0, y_block_idx = 0;
  int8_t* adr = array2d;

  if(width % 4 || height % 4)
    return NULL;

  y_block_idx = (int)floor( (float)block_n / (float)(width/BPTC_BLOCK_Y));

  x_block_idx = block_n % (width/BPTC_BLOCK_X);

  adr += (y_block_idx * BPTC_BLOCK_Y) * (width) * nr_cmpnt;

  adr += (x_block_idx * BPTC_BLOCK_X) * nr_cmpnt;

  return adr;
}


void*
load_binary_file(const char *path) {

  FILE *f = NULL;
  void *m = NULL;
  int size = 0;

  f = fopen(path, "ro");

  if(f) {
    fseek(f,0,SEEK_END);
    size = ftell (f);
    rewind(f);
  }
  else {
    printf("Error while accessing %s\n", path);
    exit(1);
  }

  m = malloc(size);

  if(!m)
    exit(0);

  fread(m, 1, size, f);
  fclose(f);

  return m;
}

/*
 * PNG read/write functions based on GNU libpng API taken from :
 * http://zarb.org/~gc/html/libpng.html
 * Copyright 2002-2010 Guillaume Cottenceau.
 * and
 * http://www.lemoda.net/c/write-png/
 */


void
write_png_file(char *file_name, int width, int height,
               png_byte bit_depth, png_byte color_type, void *image)
{
  int x = 0, y = 0;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_bytep *row_pointers = NULL;
  FILE *fp =NULL;

  if(!image)
    return;

  fp = fopen(file_name, "wb");

  if (!fp)
    printf("[write_png_file] File %s could not be opened for writing\n", file_name);

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    printf("[write_png_file] png_create_write_struct failed\n");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    printf("[write_png_file] png_create_info_struct failed\n");

  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[write_png_file] Error during init_io\n");

  png_init_io(png_ptr, fp);

  /* write header */
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[write_png_file] Error during writing header\n");

  png_set_IHDR(png_ptr, info_ptr, width, height,
	       bit_depth, color_type, PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  row_pointers = png_malloc (png_ptr, height * sizeof(png_byte*));
  for (y = 0; y < height; ++y) {
        png_byte *row = 
            png_malloc (png_ptr, sizeof (uint8_t) * width * 4);
        row_pointers[y] = row;
        for(x = 0; x < width; ++x) {
          uint8_t *pixel = ub_pixel_array2d_adress(image, width, height, x, y);
          *row++ = pixel[0];
          *row++ = pixel[1];
          *row++ = pixel[2];
          *row++ = pixel[3];
         }
  }

  png_write_info(png_ptr, info_ptr);

  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[write_png_file] Error during writing bytes\n");

  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  /* end write */
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[write_png_file] Error during end of write\n");

  png_write_end(png_ptr, NULL);

  /* cleanup heap allocation */
  for (y=0; y<height; y++)
    png_free (png_ptr, row_pointers[y]);

  png_free (png_ptr, row_pointers);

  fclose(fp);
}


void read_png_file(char* file_name)
{
  char header[8];    /* 8 is the maximum size that can be checked */
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_bytep *row_pointers = NULL;
  int width = 0, height = 0;
  png_byte color_type, bit_depth;
  int number_of_passes = 0;
  int y = 0;

  FILE *fp = fopen(file_name, "rb");

  if (!fp)
    printf("[read_png_file] File %s could not be opened for reading", file_name);
  /* test for file being a png */
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
    printf("[read_png_file] File %s is not recognized as a PNG file", file_name);

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    printf("[read_png_file] png_create_read_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    printf("[read_png_file] png_create_info_struct failed");

  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[read_png_file] Error during init_io");

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
    printf("[read_png_file] Error during read_image");

  /* untested !!! */
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (y=0; y<height; y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, NULL);
  
  fclose(fp);
}


void
write_tga_file(char* file_name, uint16_t w, uint16_t h, uint8_t bpp, void *image) {

  FILE * d = NULL;
  char fheader[18];

  if(!w || !h || !image)
    return;

  d = fopen(file_name,"w");

  if (!d) {
    printf("Erreur d'écriture ficher\n");
    return;
  }

  memset(fheader, 0, 18);

  fheader[ 0] = 0;
  fheader[ 1] = 0;
  fheader[ 2] = 2; /* RGB ? */
  fheader[ 8] = 0;
  fheader[ 10] = 0;
  memcpy(fheader+12,&w,sizeof(w));
  memcpy(fheader+14,&h,sizeof(h));

  if(bpp == 32) {
     fheader[16] = 32;
     fheader[17] = 8;
  }
  else {
     fheader[16] = 24;
     fheader[17] = 0; /* no flip */
  }

  fwrite(fheader, 1, sizeof(fheader),d);
  fwrite(image, 1,  w*h*bpp/8, d);
  fclose(d);
}


void
read_tga_file(char* file_name, uint16_t *w, uint16_t *h,
              uint8_t *components, uint8_t *bpp, uint8_t **image) {

  /*
   * TODO : far from complete implementation. Specs there :
   * http://www.paulbourke.net/dataformats/tga/
   * also useful :
   * http://nehe.gamedev.net/tutorial/loading_compressed_and_uncompressed_tga's/22001/
   */

  FILE * d = NULL;
  char fheader[18];
  int8_t encoding = 0, hasColorMap = 0, rle = 0;
  uint32_t idFieldSize = 0;
  uint16_t xOffset = 0, yOffset = 0;
  int8_t descriptor = 0;

  if(!image || !file_name)
    return;

  d = fopen(file_name,"r");

  if(!d) {
    printf("Error while accessing %s\n", file_name);
    *image = NULL;
    exit(1);
  }

  fseek(d, 0, SEEK_SET);
  fread(&idFieldSize, 1, 1, d);

  fseek(d, 2, SEEK_SET);

  fread(&rle, 1, 1, d);
  if(rle > 7)
    printf("RLE compression not hanfled yet\n");

  fseek(d, 2, SEEK_SET);
  fread(&encoding, 1, 1, d);

  fseek(d, 8, SEEK_SET);
  fread(&xOffset, 1, 2, d);

  fseek(d, 10, SEEK_SET);
  fread(&yOffset, 1, 2, d);

  fseek(d, 12, SEEK_SET);
  fread(w, 1, 2, d);

  fseek(d, 14, SEEK_SET);
  fread(h, 1, 2, d);

  fseek(d, 16, SEEK_SET);
  fread(bpp, 1, 2, d);

  *image = (uint8_t*) malloc(w[0]*h[0]*3);

  if(!*image)
    return;

  fseek(d, 18+idFieldSize, SEEK_SET);
  fread(*image, 1,  w[0]*h[0]*3, d);

#define HORIZONTAL_FLIP 0x10
#define VERTICAL_FLIP   0x20
#define HAS_ALPHA       0x0F

  fseek(d, 17, SEEK_SET);
  fread(&descriptor, 1, 1, d);
  if(descriptor & HORIZONTAL_FLIP)
     printf("horizontal flip\n");
  if(descriptor & VERTICAL_FLIP)
     printf("vertical flip\n");
  if(descriptor & HAS_ALPHA)
     ;

#undef HORIZONTAL_FLIP
#undef VERTICAL_FLIP
#undef HAS_ALPHA

  fclose(d);
}
