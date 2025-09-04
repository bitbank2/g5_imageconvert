## G5 Image Conversion Tool

This tool converts PNG and BMP images into and out of my custom, lossless 1-bit per pixel compression format called Group5 (G5). I designed this format
for 1-bit displays like LCDs and Eink. To support three and four color displays (Black/White/Yellow/Red), I added the ability to convert color images
into two separately compressed 1-bit images with colors matched to the ones displayable on Eink screens. The tool also has the ability to generate
C header files of the image data (hex bytes) to make it easier to include in your projects. You can also convert the binary and header files back
into BMP images.

## Why Group5?

CCITT Group 4 2D image compression is a very efficient and elegant 1-bit lossless compression algorithm. It can compress images up to about 20 to 1.
I wanted to use this on highly constrained MCUs and the decoder requires several hundred lines of code along with large lookup tables. I decided
to make a simplified version of G4 (I call it G5) that can run on very constrained embedded systems. To make the decoding simpler, I replaced the
statistical coding of the horizontal mode with a fixed long/short encoding. This reduces the compression effectiveness slightly, but the result
is a decompressor that can be fully implemented in about 250 lines of C code. This new image and font code opens up new use cases
for compact graphics and nice looking fonts on highly constrained systems and is still suitable for more powerful hardware. The original motivation
for G5 was that Last year I was looking into packing more functionality into the ULP (ultra-low power) co-processors inside of Espressif's ESP32-S3
and ESP32-C6 MCUs. These ULP (low speed RISC-V) processors are constrained to use the RTC RAM for both their code and data space. The ULP cannot
access any other memory of the ESP32. On the ESP32-S3, this is only 8K. To fit useful code, along with nice looking fonts into that space is quite
challenging. If I were to use the Group4 algorithm to compress the font data, the decompressor code would overflow the available space. With my
modified image compression algorithm, I was able to fit my program code, font data and the decompressor in that tiny space.

# Building the tool
The G5 convert tool is written in C++ and uses POSIX functions for file I/O. This means that it should build without issue on Linux, MacOS and Windows.
It uses my PNGdec library for decoding PNG images and includes this as a submodule. There are no external dependencies, so just run make and you're
ready to use it.

# How well does it compress images?
