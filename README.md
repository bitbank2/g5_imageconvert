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
The G5 convert tool is written in C++ and uses POSIX functions for file I/O and memory allocation. This means that it should build without issue on Linux, MacOS and Windows.
It uses my PNGdec library for decoding PNG images and includes this as a submodule. There are no external dependencies, so just run make and you're
ready to use it. Start by cloning this repo recursively:<br>
git clone --recursive https://github.com/bitbank2/g5_imageconvert<br>
If you forgot to type the --recursive parameter, don't worry. After you clone it, do the following:<br>
git clone https://github.com/bitbank2/g5_imageconvert<br>
cd g5_imageconvert<br>
git submodule init<br>
git submodule update<br>
Now you can simply type 'make' and you'll be ready to use it.<br>

# How well does it compress images?
FAX (Group4) compression was designed to compress black text on a white page. G5 is the same. It compresses images in two ways - one dimensionally
with run length encoding (RLE) and two dimensionally by compressing the points that change color on the current line relative to similar points
on the line above. This works quite well for images with large areas of constant color with straight or curved edges (like fonts). This doesn't 
perform well on dithered patterns because it uses more bits to encode alternating colors that would be used by uncompressed data. For your average
non-dithered 1-bit image, you'll usually get between 4 and 10 to 1 compression. The codec both compresses and decompresses images very quickly
so the extra CPU cycles needed to decompress font images is insignificant compared to the overall font drawing.

# Image Examples
<b>1 Bit Example</b><br>
<br>
![Example PNG 1](/bear_193x80.png?raw=true "Example PNG 1")
<br>
This is a 193x80 1-bit per pixel image which compresses to a 1082 byte PNG
file (uncompressed size is 2000 bytes, so PNG is not able to compress it
very much. The following command line converts it into a G5 image, ready
to include in your project:<br>
./imgcvt bear_193x80.png bear_193x80.h BW<br>
The result is a 534 byte G5 file. So in this case, G5 is a more effective
image compression algorithm compared to PNG, yet it takes many fewer CPU
cycles to decode the G5 image.<br>
<br>
<b>2 Bit Example</b><br>
<br>
![Example PNG 2](/smiley_hearts.png?raw=true "Example PNG 2")
<br>
This image is a 128x128 2-bit (4 color) PNG. The uncompressed size is 4096
bytes and the PNG file compresses to 1472 bytes. The 2-plane G5 image is
only 362 bytes. Again, G5 compresses this type of image much better than PNG.<br>
When used within your project, you can easily draw it at any scale and
any position with a single line of code:<br>
![Use case](/g5_4clr.jpg?raw=true "Use case")
<br>

