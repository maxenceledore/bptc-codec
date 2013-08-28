ENCODER_BIN = encode
DECODER_BIN = decode

LIBRARIES = -lpng12 -lm

COMMON_SRC =\
util.c\

ENCODER_SRC = bptc_encode.c bptc_runtime_encoder.c bptc_encode_tile_analyser.c
DECODER_SRC = bptc_block_parser.c bptc_decode.c bptc_runtime_decoder.c


all:
	gcc -o $(ENCODER_BIN) $(ENCODER_SRC) $(COMMON_SRC) $(LIBRARIES)
	rm -rf *.o
	gcc -o $(DECODER_BIN) $(DECODER_SRC) $(COMMON_SRC) $(LIBRARIES)
	rm -rf *.o

clean:
	rm -rf *.o
	rm $(DECODER_BIN)
	rm $(ENCODER_BIN)