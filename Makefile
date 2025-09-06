PNGDEC_ROOT = PNGdec/src
PNGENC_ROOT = PNGenc/src

all: imgcvt
CFLAGS = -D__LINUX__ -DPNG_MAX_BUFFERED_PIXELS=2501*8 -I $(PNGDEC_ROOT) -I $(PNGENC_ROOT) -Wall -O2

imgcvt: main.o trees.o PNGenc.o PNGdec.o deflate.o adler32.o crc32.o infback.o inffast.o inflate.o inftrees.o zutil.o
	$(CC) main.o PNGenc.o PNGdec.o adler32.o crc32.o infback.o inffast.o inflate.o deflate.o trees.o inftrees.o zutil.o -o $@
	strip $@

main.o: main.cpp Makefile Group5.h
	$(CXX) $(CFLAGS) -c main.cpp

PNGenc.o: $(PNGENC_ROOT)/PNGenc.cpp $(PNGENC_ROOT)/png.inl $(PNGENC_ROOT)/PNGenc.h
	$(CXX) $(CFLAGS) -c $(PNGENC_ROOT)/PNGenc.cpp

PNGdec.o: $(PNGDEC_ROOT)/PNGdec.cpp $(PNGDEC_ROOT)/png.inl $(PNGDEC_ROOT)/PNGdec.h
	$(CXX) $(CFLAGS) -c $(PNGDEC_ROOT)/PNGdec.cpp

trees.o: $(PNGENC_ROOT)/trees.c
	$(CC) $(CFLAGS) -c $(PNGENC_ROOT)/trees.c

deflate.o: $(PNGENC_ROOT)/deflate.c
	$(CC) $(CFLAGS) -c $(PNGENC_ROOT)/deflate.c

adler32.o: $(PNGDEC_ROOT)/adler32.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/adler32.c

crc32.o: $(PNGDEC_ROOT)/crc32.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/crc32.c

infback.o: $(PNGDEC_ROOT)/infback.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/infback.c

inffast.o: $(PNGDEC_ROOT)/inffast.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/inffast.c

inflate.o: $(PNGDEC_ROOT)/inflate.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/inflate.c

inftrees.o: $(PNGDEC_ROOT)/inftrees.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/inftrees.c

zutil.o: $(PNGDEC_ROOT)/zutil.c
	$(CC) $(CFLAGS) -c $(PNGDEC_ROOT)/zutil.c

clean:
	rm -rf *.o imgcvt
