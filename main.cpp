//
// PNG to Group5 Image Compressor tool
// Written by Larry Bank
// Copyright (c) 2025 BitBank Software, Inc.
//
#include <stdio.h>
#define MAX_IMAGE_FLIPS 256
#include "Group5.h"
#include "g5enc.inl"
#include "g5dec.inl"
#include "PNGenc.h"
#include "PNGdec.h"

const char *szPNGErrors[] = {"Success", "Invalid parameter", "Decode error", "Out of memory", "No buffer", "Unsupported feature", "Invalid file",  "Image too large", "Decoder quit early"};
const char *szG5Errors[] = {"Success", "Invalid parameter", "Decode error", "Unsupported feature", "Encode complete", "Decode complete", "Not initialized", "Data overflow", "Max flips exceeded"}
;
PNG png; // static instance of class
PNGENC pngenc;

uint8_t u8Temp[1024];

/* Windows BMP header info (54 bytes) */
uint8_t winbmphdr[54] =
        {0x42,0x4d,
         0,0,0,0,         /* File size */
         0,0,0,0,0x36,4,0,0,0x28,0,0,0,
         0,0,0,0, /* Xsize */
         0,0,0,0, /* Ysize */
         1,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* number of planes, bits per pel */
         0,0,0,0};
// 4 Windows palette entries for converting 3 and 4 color G5 images back into valid Windows BMP files
uint8_t ucBWYRPalette[12] = { // b, g, r
    0x00, 0x00, 0x00, // black
    0xff, 0xff, 0xff, // white
    0xff, 0xff, 0x00, // yellow
    0xff, 0x00, 0x00, // red
};

enum {
    MODE_BW = 0,
    MODE_BWR,
    MODE_BWYR,
    MODE_4GRAY
};
//
// Minimal code to save frames as PNG files
//
void SavePNG(const char *fname, uint8_t *pBitmap, uint8_t *pPalette, int w, int h, int bpp)
{
    FILE * oHandle;
    int i, y, iPitch, iSize;
    uint8_t *s, *pOut;
    uint8_t ucTemp[12];
    
    if (bpp == 1) {
        iPitch = (w+7)/8;
    } else {
        iPitch = (w+1)/2;
        // fix the palette; red and blue are swapped
        for (i=0; i<4; i++) {
            ucTemp[(i*3)+0] = pPalette[(i*3)+2];
            ucTemp[(i*3)+1] = pPalette[(i*3)+1];
            ucTemp[(i*3)+2] = pPalette[(i*3)+0];
        }
    }
    iSize = (w * h * bpp)/8 + 256;
    pOut = (uint8_t *)malloc(iSize); // plenty of space
    i = pngenc.open(pOut, iSize);
    if (i == PNG_SUCCESS) {
        i = pngenc.encodeBegin(w, h, (bpp == 1) ? PNG_PIXEL_GRAYSCALE : PNG_PIXEL_INDEXED, bpp, ucTemp, 9);
        if (i == PNG_SUCCESS) {
            s = pBitmap;
            for (y=0; y<h && i == PNG_SUCCESS; y++) {
                i = pngenc.addLine(s);
                s += iPitch;
            } // for y
            iSize = pngenc.close();
            oHandle = fopen(fname, "w+b");
            if (!oHandle) {
                printf("Error creating file %s\n", fname);
                free(pOut);
                return;
            }
            fwrite(pOut, 1, (size_t)iSize, oHandle);
            fflush(oHandle);
            fclose(oHandle);
            free(pOut);
        } // encodeBegin success
    } // png.open success
} /* SavePNG() */
//
// Minimal code to save frames as Windows BMP files
//
void SaveBMP(const char *fname, uint8_t *pBitmap, uint8_t *pPalette, int cx, int cy, int bpp)
{
FILE * oHandle;
int i, bsize, lsize;
uint32_t *l;
uint8_t *s;
uint8_t ucTemp[1024];

    oHandle = fopen(fname, "w+b");
    if (!oHandle) {
        printf("Error creating file %s\n", fname);
        return;
    }
    if (bpp == 1) {
        bsize = (cx + 7)>>3;
    } else if (bpp == 4) {
        bsize = (cx + 1)>>1;
    } else {
        bsize = (cx * bpp) / 8;
    }
    lsize = (bsize + 3) & 0xfffc; /* Width of each line */
    winbmphdr[26] = 1; // number of planes
    winbmphdr[28] = (uint8_t)bpp;

   /* Write the BMP header */
   l = (uint32_t *)&winbmphdr[2];
    i =(cy * lsize) + 54;
    if (bpp <= 8)
        i += 1024;
   *l = (uint32_t)i; /* Store the file size */
   l = (uint32_t *)&winbmphdr[10]; // pointer to data
    *l = i - (cy * lsize);
    winbmphdr[14] = 0x28;
   l = (uint32_t *)&winbmphdr[18];
   *l = (uint32_t)cx;      /* width */
   *(l+1) = (uint32_t)(-cy);  /* height */
   fwrite(winbmphdr, 1, 54, oHandle);
    if (bpp <= 8) {
    if (pPalette == NULL) {// create a grayscale palette
        int iDelta, iCount = 1<<bpp;
        int iGray = 0;
        iDelta = 255/(iCount-1);
        for (i=0; i<iCount; i++) {
            ucTemp[i*4+0] = (uint8_t)iGray;
            ucTemp[i*4+1] = (uint8_t)iGray;
            ucTemp[i*4+2] = (uint8_t)iGray;
            ucTemp[i*4+3] = 0;
            iGray += iDelta;
        }
    } else {
        for (i=0; i<256; i++) // change palette to WinBMP format
        {
            ucTemp[i*4 + 0] = pPalette[(i*3)+2];
            ucTemp[i*4 + 1] = pPalette[(i*3)+1];
            ucTemp[i*4 + 2] = pPalette[(i*3)+0];
            ucTemp[i*4 + 3] = 0;
        }
    }
    fwrite(ucTemp, 1, 1024, oHandle);
    }
   /* Write the image data */
   for (i=0; i<cy; i++)
    {
        s = &pBitmap[i*bsize];
        if (bpp >= 24) { // swap R/B for Windows BMP byte order
            uint8_t ucTemp[2048];
            int j, iBpp = bpp/8;
            uint8_t *d = ucTemp;
            for (j=0; j<cx; j++) {
                d[0] = s[2]; d[1] = s[1]; d[2] = s[0];
                d += iBpp; s += iBpp;
            }
            fwrite(ucTemp, 1, (size_t)lsize, oHandle);
        } else {
            fwrite(s, 1, (size_t)lsize, oHandle);
        }
    }
    fflush(oHandle);
    fclose(oHandle);
} /* SaveBMP() */
//
// Read a file into memory
//
uint8_t *ReadTheFile(const char *fname, int *pSize)
{
    FILE *infile;
    int iSize;
    uint8_t *pData;
    infile = fopen(fname, "r+b");
    if (infile == NULL) {
        printf("Error opening input file %s\n", fname);
        return NULL;
    }
    fseek(infile, 0, SEEK_END);
    iSize = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    pData = (uint8_t *)malloc(iSize);
    fread(pData, 1, iSize, infile);
    fclose(infile);
    *pSize = iSize;
    return pData;
} /* ReadTheFile() */

//
// Read a Windows BMP file into memory
//
uint8_t * ReadBMP(uint8_t *pTemp, int iSize, int *width, int *height, int *bpp, unsigned char *pPal)
{
    int y, w, h, bits, offset;
    uint8_t *s, *d, *pBitmap;
    int pitch, bytewidth;
    int iDelta;
    
    pBitmap = (uint8_t *)malloc(iSize);
    
    if (pTemp[0] != 'B' || pTemp[1] != 'M' || pTemp[14] < 0x28) {
        free(pBitmap);
        free(pTemp);
        printf("Not a Windows BMP file!\n");
        return NULL;
    }
    w = *(int32_t *)&pTemp[18];
    h = *(int32_t *)&pTemp[22];
    bits = *(int16_t *)&pTemp[26] * *(int16_t *)&pTemp[28];
    if (bits <= 8 && pPal != NULL) { // it has a palette, copy it
        uint8_t *p = pPal;
        for (int i=0; i<(1<<bits); i++)
        {
           *p++ = pTemp[54+i*4];
           *p++ = pTemp[55+i*4];
           *p++ = pTemp[56+i*4];
        }
    }
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    if (bits == 1) {
        bytewidth = (w+7) >> 3;
    } else {
        bytewidth = (w * bits) >> 3;
    }
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
// move up the pixels
    d = pBitmap;
    s = &pTemp[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = &pTemp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    for (y=0; y<h; y++) {
        if (bits == 32) {// need to swap red and blue
            for (int i=0; i<bytewidth; i+=4) {
                d[i] = s[i+2];
                d[i+1] = s[i+1];
                d[i+2] = s[i];
                d[i+3] = s[i+3];
            }
        } else {
            memcpy(d, s, bytewidth);
        }
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    *bpp = bits;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */
//
// Parse a header file generated by this tool
// and turn it back into binary data
//
int ParseHeader(uint8_t *pBMP, int iSize)
{
uint8_t uc, *s, *d, *pEnd;

    printf("Parsing the compressed image data from a header file...\n");
    pEnd = &pBMP[iSize];
    s = d = pBMP;

    while (s < pEnd) {
        if (s[0] == '0' && s[1] == 'x') {
            // high nibble
            if (s[2] <= '9') {
                uc = (s[2] - '0') << 4;
            } else {
                uc = (s[2] - 'a' + 10) << 4;
            }
            // low nibble
            if (s[3] <= '9') {
                uc |= (s[3] - '0');
            } else {
                uc |= (s[3] - 'a' + 10);
            }
            *d++ = uc;
            s += 4;
        } else {
            s++;
        }
    } // while scanning the text
    // Check that the file is valid
    if (*(uint16_t *)pBMP == BB_BITMAP_MARKER || *(uint16_t *)pBMP == BB_BITMAP2_MARKER) {
        return (int)(d - pBMP); // new data size
    }
    return 0; // invalid
} /* ParseHeader() */

//
// Create the comments and const array boilerplate for the hex data bytes
//
void StartHexFile(FILE *f, int iLen, int w, int h, const char *fname, int iMode)
{
    int i;
    char szTemp[256];
    fprintf(f, "//\n// Created with imageconvert, written by Larry Bank\n");
    if (iMode == MODE_BW) {
        fprintf(f, "// %d x %d x 1-bit per pixel\n", w, h);
    } else {
        fprintf(f, "// %d x %d x 2-bits per pixel (split into 2 1-bit planes)\n", w, h);
    }
    fprintf(f, "// compressed image data size = %d bytes\n//\n", iLen);
    strcpy(szTemp, fname);
    i = (int)strlen(szTemp);
    if (szTemp[i-2] == '.') szTemp[i-2] = 0; // get the leaf name for the data
    fprintf(f, "const uint8_t %s[] = {\n", szTemp);
} /* StartHexFile() */
//
// Add N bytes of hex data to the output
// The data will be arranged in rows of 16 bytes each
//
void AddHexBytes(FILE *f, void *pData, int iLen, int bLast)
{
    static int iCount = 0; // number of bytes processed so far
    int i;
    uint8_t *s = (uint8_t *)pData;
    for (i=0; i<iLen; i++) { // process the given data
        fprintf(f, "0x%02x", *s++);
        iCount++;
        if (i < iLen-1 || !bLast) fprintf(f, ",");
        if ((iCount & 15) == 0) fprintf(f, "\n"); // next row of 16
    }
    if (bLast) {
        fprintf(f, "};\n");
    }
} /* AddHexBytes() */
//
// Match the given pixel to black (00), white (01), or red (1x)
//
unsigned char GetBWRPixel(int r, int g, int b)
{
    uint8_t ucOut=0;
    int gr;
    
    gr = (b + r + g*2)>>2; // gray
    // match the color to closest of black/white/red
    if (r > g && r > b) { // red is dominant
        if (gr < 100 && r < 80) {
            // black
        } else {
            if (r-b > 32 && r-g > 32) {
                // is red really dominant?
                ucOut |= 3; // red (can be 2 or 3, but 3 is compatible w/BWYR)
            } else { // yellowish should be white
                // no, use white instead of pink/yellow
                ucOut |= 1;
            }
        }
    } else { // check for white/black
        if (gr >= 100) {
            ucOut |= 1; // white
        } else {
            // black
        }
    }
    return ucOut;
} /* GetBWRPixel() */
//
// Match the given pixel to black (00), white (01), yellow (10), or red (11)
// returns 2 bit value of closest matching color
//
unsigned char GetBWYRPixel(int r, int g, int b)
{
    uint8_t ucOut=0;
    int gr;

    gr = (b + r + g*2)>>2; // gray
    // match the color to closest of black/white/yellow/red
    if (r > b || g > b) { // red or yellow is dominant
        if (gr < 90 && r < 80 && g < 80) {
            // black
        } else {
            if (r-b > 32 && r-g > 32) {
                // is red really dominant?
                ucOut = 3; // red
            } else if (r-b > 32 && g-b > 32) {
                // yes, yellow
                ucOut = 2;
            } else {
                ucOut = 1; // gray/white
            }
        }
    } else { // check for white/black
        if (gr >= 100) {
            ucOut = 1; // white
        } else {
            // black
        }
    }
    return ucOut;
} /* GetBWYRPixel() */
//
// The user passed a file with the wrong bit depth (not 1)
// convert to 1-bpp by converting each color to 0 or 1 based on the gray level
//
void ConvertBpp(uint8_t *pBMP, int iMode, int w, int h, int iBpp, uint8_t *palette)
{
    int gray, r=0, g=0, b=0, x, y, iDelta, iPitch, iDestPitch, iDestBpp;
    uint8_t *s, *d, *pPal, u8, count;

    iDestBpp = (iMode == MODE_BW) ? 1 : 2;
    iPitch = (w * iBpp)/8;
    if (iDestBpp == 1)
        iDestPitch = (w+7)/8;
    else {
        iDestPitch = (w+3)/4;
    }
    iDelta = iBpp/8;
    for (y=0; y<h; y++) {
        s = &pBMP[iPitch * y];
        d = &pBMP[iDestPitch * y]; // overwrite the original data as we change it
        count = 8; // bits in a byte
        u8 = 0; // start with all black
        for (x=0; x<w; x++) { // slower code, but less code :)
            u8 <<= iDestBpp;
            switch (iBpp) {
                case 24:
                case 32:
                    r = s[0];
                    g = s[1];
                    b = s[2];
                    s += iDelta;
                    break;
                case 16:
                    r = s[1] & 0xf8; // red
                    g = ((s[0] | s[1] << 8) >> 3) & 0xfc; // green
                    b = s[0] << 3;
                    s += 2;
                    break;
                case 8:
                    if (palette) {
                        pPal = &palette[s[0] * 3];
                        r = pPal[0];
                        g = pPal[1];
                        b = pPal[2];
                    } else {
                        r = g = b = s[0];
                    }
                    s++;
                    break;
                case 4:
                    if (palette) {
                        if (x & 1) {
                            pPal = &palette[(s[0] & 0xf) * 3];
                            s++;
                        } else {
                            pPal = &palette[(s[0]>>4) * 3];
                        }
                        r = pPal[0];
                        g = pPal[1];
                        b = pPal[2];
                    } else {
                        if (x & 1) {
                            r = g = b = (s[0] & 0xf) | (s[0] << 4);
                        } else {
                            r = g = b = (s[0] >> 4) | (s[0] & 0xf0);
                        }
                    }
                    break;
            } // switch on bpp
            if (iMode == MODE_BW || iMode == MODE_4GRAY) {
                gray = (r + g*2 + b)/4;
                u8 |= gray >> (8-iDestBpp);
            } else if (iMode == MODE_BWR) {
                u8 |= GetBWRPixel(r, g, b); // match the closest colors
            } else {
                u8 |= GetBWYRPixel(r, g, b);
            }
            count -= iDestBpp;
            if (count == 0) { // byte is full, move on
                *d++ = u8;
                u8 = 0;
                count = 8;
            }
        } // for x
        if (count != 8) {
            *d++ = (u8 << count); // store last partial byte
        }
    } // for y
} /* ConvertBpp() */

int main(int argc, const char * argv[]) {
    int w, h, y, rc, bpp = 0;
    uint8_t *s, *d, *pOut, *pData, *pImage;
    int iPitch, iPlaneSize, iOutSize, iDataSize, iMode;
    uint8_t *pPalette;
    G5ENCIMAGE g5enc;
    G5DECIMAGE g5dec;
    BB_BITMAP bbbm, *pBBBM;
    int bHFile; // flag indicating if the output will be a .H file of hex data
    int bPNGFile; // flag indicating if the output file should be PNG
    const char *szModes[] = {"BW", "BWR", "BWYR", "4GRAY", NULL};
    
    printf("Group5 image conversion tool\n");
    if (argc != 4) {
        printf("Usage: ./pngconvert <PNG or BMP image> <g5 compressed image> <mode>\n");
        printf("or ./pngconvert <G5 image> <PNG or BMP image> <mode>\n");
        printf("valid modes: BW, BWR, BWYR, 4GRAY (case insensitive)\n");
        printf("G5 input and output can be binary or .H header files\n");
        return -1;
    }
    iMode = 0;
    while (szModes[iMode] && strcasecmp(szModes[iMode], argv[3]) != 0) {
        iMode++;
    }
    if (szModes[iMode] == NULL) {
        printf("Invalid mode\n");
        return -1;
    }
    pOut = (uint8_t *)argv[2] + strlen(argv[2]) - 1;
    bHFile = (pOut[0] == 'H' || pOut[0] == 'h'); // output an H file?
    pOut = (uint8_t *)argv[2] + strlen(argv[2]) - 3;
    bPNGFile = (pOut[0] == 'P' || pOut[0] == 'p');
    pData = ReadTheFile(argv[1], &iDataSize);
    if (pData == NULL)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        return -1; // bad filename passed
    }
    // Check if we're parsing an H file created by this tool
    // If so, first convert it to binary data
    if (pData[0] == '/' && pData[1] == '/' && pData[2] == 0x0a) {
        iDataSize = ParseHeader(pData, iDataSize);
        if (!iDataSize) { // invalid input
            printf("Invalid input file!\n");
            return -1;
        }
    }
    if (*(uint16_t *)pData == BB_BITMAP_MARKER) { // we're converting a G5 to a BMP
        pBBBM = (BB_BITMAP *)pData;
        w = pBBBM->width;
        h = pBBBM->height;
        printf("Converting 1-bit %dx%d G5 image to %s\n", w, h, (bPNGFile) ? "PNG" : "BMP");
        iPitch = (w + 7)/8;
        pOut = (uint8_t *)malloc(iPitch * (h+1)); // output BMP
        rc = g5_decode_init(&g5dec, w, h, (uint8_t *)&pBBBM[1], pBBBM->size);
        if (rc != G5_SUCCESS) {
            printf("Error decoding Group5 image: %s\n", szG5Errors[rc]);
            return -1;
        }
        for (y=0; y<h; y++) {
            g5_decode_line(&g5dec, &pOut[iPitch*y]);
        } // for y
        if (bPNGFile) {
            SavePNG(argv[2], pOut, NULL, w, h, 1);
        } else { // assume it's a BMP file
            SaveBMP(argv[2], pOut, NULL, w, h, 1);
        }
    } else if (*(uint16_t *)pData == BB_BITMAP2_MARKER) { // 2-bit (separate planes
        pBBBM = (BB_BITMAP *)pData;
        w = pBBBM->width;
        h = pBBBM->height;
        printf("Converting 2-bpp %dx%d G5 image to 4-bpp %s\n", w, h, (bPNGFile) ? "PNG" : "BMP");
        iPitch = (w + 7)/8;
        pOut = (uint8_t *)malloc(iPitch * (h*8+1)); // output BMP
        rc = g5_decode_init(&g5dec, w, h*2, (uint8_t *)&pBBBM[1], pBBBM->size);
        if (rc != G5_SUCCESS) {
            printf("Error decoding Group5 image: %s\n", szG5Errors[rc]);
            return -1;
        }
        for (y=0; y<h*2; y++) {
            g5_decode_line(&g5dec, &pOut[iPitch*y]);
        } // for y
        // Combine the two 1-bit planes into 4-bpp output for compatibility with Windows BMP files
        iPlaneSize = iPitch * h; // offset to second plane
        for (y=0; y<h; y++) {
            uint8_t uc0=0, uc1=0;
            int x;
            s = &pOut[y * iPitch];
            d = &pOut[(iPitch * h * 2) + (y * ((w+1)>>1))]; // start after the bit planes
            for (x=0; x<w; x++) {
                if ((x & 7) == 0) {
                    uc0 = s[0]; uc1 = s[iPlaneSize]; // pixels from each bit plane
                    s++;
                }
                if (x & 1) {
                    d[0] |= ((uc0 >> 7) & 1) | ((uc1 >> 6) & 2);
                    d++;
                } else {
                    d[0] = ((uc0 >> 3) & 0x10) | ((uc1 >> 2) & 0x20); // left pixel
                }
                uc0 <<= 1; uc1 <<= 1;
            } // for x
        } // for y
        if (bPNGFile) {
            SavePNG(argv[2], &pOut[(iPitch * h * 2)], (iMode == MODE_4GRAY) ? NULL : ucBWYRPalette, w, h, 4);
        } else { // assume it's a BMP file
            SaveBMP(argv[2], &pOut[(iPitch * h * 2)], (iMode == MODE_4GRAY) ? NULL : ucBWYRPalette, w, h, 4);
        }
    }
    if (pData[0] == 'B' && pData[1] == 'M') { // input file is a BMP
        pPalette = (uint8_t *)malloc(1024);
        pImage = ReadBMP(pData, iDataSize, &w, &h, &bpp, pPalette);
    } else if (pData[1] == 'P' && pData[2] == 'N') { // PNG file?
        rc = png.openRAM(pData, iDataSize, NULL);
        if (rc == PNG_SUCCESS) {
            w = png.getWidth();
            h = png.getHeight();
            printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", w, h, png.getBpp(), png.getPixelType());
            pImage = (uint8_t *)malloc(png.getBufferSize());
            png.setBuffer(pImage);
            rc = png.decode(NULL, 0); //PNG_CHECK_CRC);
            png.close();
            if (rc != PNG_SUCCESS) {
                printf("Error decoding PNG file = %d\n", rc);
                free(pImage);
                return -1;
            }
            pPalette = NULL;
            switch (png.getPixelType()) {
                case PNG_PIXEL_INDEXED:
                case PNG_PIXEL_GRAYSCALE:
                    if (png.getPixelType() == PNG_PIXEL_INDEXED) {
                        pPalette = png.getPalette();
                    }
                    bpp = png.getBpp();
                    break;
                case PNG_PIXEL_TRUECOLOR:
                    bpp = 24;
                    break;
                case PNG_PIXEL_TRUECOLOR_ALPHA:
                    bpp = 32;
                    break;
            } // switch
        } else { // error opening the PNG
            printf("Error opening %s: \"%s\"\n", argv[1], szPNGErrors[rc]);
            return -1;
        }
        if (iMode != MODE_BW || (iMode == MODE_BW && bpp != 1)) { // need to convert it to 1 or 2-bpp
            printf("Converting pixels to %s\n", szModes[iMode]);
            ConvertBpp(pImage, iMode, w, h, bpp, pPalette);
        }
        iPitch = (w+7) >> 3;
        pOut = (uint8_t *)malloc(iPitch * h * 2);
        if (iMode == MODE_BW) {
            s = png.getBuffer();
            rc = g5_encode_init(&g5enc, w, h, pOut, iPitch * h);
            for (y=0; y<h && rc == G5_SUCCESS; y++) {
                rc = g5_encode_encodeLine(&g5enc, s);
                s += iPitch;
            }
        } else { // split the bit planes
            uint8_t *d, src, ucMask, uc = 0;
            s = pImage;
            rc = g5_encode_init(&g5enc, w, h*2, pOut, iPitch * h * 2);
            // encode plane 0 first
            for (y=0; y<h*2 && rc == G5_SUCCESS; y++) {
                if (y == h) {
                    s = pImage; // restart from the top
                }
                src = *s++;
                d = u8Temp;
                ucMask = (y < h) ? 0x40 : 0x80; // lower or upper source bit
                for (int x=0; x<w; x++) {
                    uc <<= 1;
                    if (src & ucMask) {
                        uc |= 1;
                    }
                    src <<= 2;
                    if ((x & 3) == 3 && x != w-1) { // new input byte
                        src = *s++;
                    }
                    if ((x & 7) == 7) { // new output byte
                        *d++ = uc;
                    }
                } // for x
                *d = uc; // store last partial byte
                rc = g5_encode_encodeLine(&g5enc, u8Temp);
            }
        }
    if (rc == G5_ENCODE_COMPLETE) {
        FILE *f;
        iOutSize = g5_encode_getOutSize(&g5enc);
        if (iMode != MODE_BW) iPitch *= 2;
        printf("Input data size:  %d bytes, compressed size: %d bytes\n", iPitch*h, iOutSize);
        printf("Compression ratio: %2.1f:1\n", (float)(iPitch*h) / (float)iOutSize);
        bbbm.u16Marker = (iMode == MODE_BW) ? BB_BITMAP_MARKER : BB_BITMAP2_MARKER;
        bbbm.width = w;
        bbbm.height = h;
        bbbm.size = iOutSize;
        f = fopen(argv[2], "w+b");
        if (!f) {
            printf("Error opening: %s\n", argv[2]);
        } else {
            if (bHFile) { // generate HEX file to include in a project
                StartHexFile(f, iOutSize+sizeof(BB_BITMAP), w, h, argv[2], iMode);
                AddHexBytes(f, &bbbm, sizeof(BB_BITMAP), 0);
                AddHexBytes(f, pOut, iOutSize, 1);
                printf(".H file created successfully!\n");
            } else { // generate a binary file
                fwrite(&bbbm, 1, sizeof(BB_BITMAP), f);
                fwrite(pOut, 1, iOutSize, f);
                printf("Binary file created successfully!\n");
            }
            fflush(f);
            fclose(f);
        }
    } else {
        printf("Error encoding image: %s\n", szG5Errors[rc]);
    }
    free(pImage);
    }
    return 0;
}
