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
#include <stdlib.h>
#include <malloc.h>

#include "util.h"
#include "bptc_decode.h"

#include <png.h>

void usage(void);
void parseStartupOptions(int argc, const char * argv[]);

void usage(void) {
  printf("Usage : decode [options] <input file> -w <width> -h <height> [<output file>]\n");
  exit(0);
}

void parseStartupOptions(int argc, const char * argv[])
{
  return;
}

int main(int argc, char **argv) {

  char *input_fname = NULL, *output_fname = NULL;

  uint8_t *bptc_raw = NULL , *out = NULL;
  int i = 0, width = 0, height = 0;

  if(argc < 6)
    usage();

  for(i=1 ; i < argc ; i++) {

    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
        case 'w':
          width = atoi(argv[i+1]);
          i++;
          break;
        case 'h':
          height = atoi(argv[i+1]);
          i++;
          break;
      }
    }

    else {
      if(!input_fname)
        input_fname = argv[i];
      else {
        if(!output_fname)
        output_fname = argv[i];
        else {
          usage();
          return 1;
        }
      }
    }
  }
  /* define the default output file name is not set */
  if(!output_fname)
    output_fname = "dump.png";

  bptc_raw = load_binary_file(input_fname);

  out = (uint8_t*) malloc(width*height*4);

  if(bptc_raw && out)
    bptc_decode_image(bptc_raw, width, height, out);

  write_png_file(output_fname, width, height, 8, PNG_COLOR_TYPE_RGBA, out);

  return 0;
}
