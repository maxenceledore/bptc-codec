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
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "util.h"
#include "bptc_encode.h"
#include "bptc_codec_data.h"

void usage(void);
void parseStartupOptions(int argc, const char * argv[]);

void usage(void) {
  printf("Usage : encode [options] <input file> <output file>\n"
         "Options :\n"
         "-v : output informations (PSNR evaluation, compression time ...)\n"
         "-m {0...7} : restrict compression to a modes subset\n");
  exit(0);
}

void parseStartupOptions(int argc, const char * argv[])
{
        return;
}

int main(int argc, char **argv) {

  char *input_fname = NULL, *output_fname = NULL;
  FILE *out = NULL;

  uint8_t *bptc_data = NULL , *pixel_data = NULL;
  uint16_t width = 0, height = 0;
  uint8_t components = 0, bpp = 0;

  uint8_t verbose = 0;

  if(argc < 3) {
     usage();
     return;
  }

  input_fname = argv[1];
  output_fname = argv[2];

  if(strstr(input_fname + strlen(input_fname) - 4, ".tga")) {
     uint8_t *p = NULL;
     int i = 0;
     read_tga_file(input_fname, &width, &height, &components, &bpp, &pixel_data);
     for(i=0; i < width*height; i++) {
       uint8_t tmp = 0;
       tmp = pixel_data[i*3+0];
       pixel_data[i*3+0] = pixel_data[i*3+2];;
       pixel_data[i*3+2] = tmp;
  }
     /* There's no unsigned char RGB8 BPTC format. Only (S)RGBA. To simplify things,
      * let's inject now opaque alpha bits to work on RGBA8 from 24 bits RGB tga files.
      */
#if 0
     if(bpp == 24) {
        p = malloc(width * height * 4);
        if(p)
           for(i=0; i < width * height * 3; i+=3) {
              memcpy(p, pixel_data+i, 3);
              p[3] = 255;
              p += 4;
           }
           else
              exit(1);
        free(pixel_data); pixel_data = NULL;
        pixel_data = p;
        components = 4;
        bpp +=0;
     }
#endif
}
  else if(strstr(input_fname + strlen(input_fname) - 4, ".png")) {
     return;
  }
  else if(strstr(input_fname + strlen(input_fname) - 4, ".exr")) {
     return;
  }
  else {
     printf("unknown file type (supported extensions : .tga .png .exr)\n");
     exit(1);
  }

  if(!pixel_data)
    exit(1);

  bptc_data = malloc(width * height);

  if(bptc_data && pixel_data)
     bptc_encode_image(pixel_data, components, bpp / 8, width, height, bptc_data);
  else {
    printf("Error while allocating memory. Abort\n");
    exit(1);
  }

  out = fopen(output_fname, "w");

  if(out) {
    fwrite(bptc_data, 1, width*height, out);
    fclose(out);
    free(pixel_data);
    free(bptc_data);
  }
  else {
    printf("Error while creating output file : %s\n", output_fname);
    exit(1);
  }
    

  return 0;
}
