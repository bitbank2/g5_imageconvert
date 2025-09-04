PNG_ROOT = PNGdec/src

all: imgcvt
CFLAGS = -D__LINUX__ -DPNG_MAX_BUFFERED_PIXELS=2501*8 -I $(PNG_ROOT) -Wall -O2

imgcvt: main.o PNGdec.o adler32.o crc32.o infback.o inffast.o inflate.o inftrees.o zutil.o
	$(CC) main.o PNGdec.o adler32.o crc32.o infback.o inffast.o inflate.o inftrees.o zutil.o -o $@
	strip $@

main.o: main.cpp Makefile Group5.h
	$(CXX) $(CFLAGS) -c main.cpp

PNGdec.o: $(PNG_ROOT)/PNGdec.cpp $(PNG_ROOT)/png.inl $(PNG_ROOT)/PNGdec.h
	$(CXX) $(CFLAGS) -c $(PNG_ROOT)/PNGdec.cpp

adler32.o: $(PNG_ROOT)/adler32.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/adler32.c

crc32.o: $(PNG_ROOT)/crc32.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/crc32.c

infback.o: $(PNG_ROOT)/infback.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/infback.c

inffast.o: $(PNG_ROOT)/inffast.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/inffast.c

inflate.o: $(PNG_ROOT)/inflate.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/inflate.c

inftrees.o: $(PNG_ROOT)/inftrees.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/inftrees.c

zutil.o: $(PNG_ROOT)/zutil.c
	$(CC) $(CFLAGS) -c $(PNG_ROOT)/zutil.c

clean:
	rm -rf *.o imgcvt
