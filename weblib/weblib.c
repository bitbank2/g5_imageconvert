//
// Compress a RGB888 image into a 1-bit Group5 image
//
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../Group5.h"
#include "../g5enc.inl"

int g5_encode_rgba(const uint8_t *rgba, int width, int height, uint8_t *out, int outSize)
{
G5ENCIMAGE enc;
BB_BITMAP *header;
int x, y, g5MaxSize, g5CompressedSize, packedPitch, err;
uint8_t *g5Data;
uint8_t *packedImage;
    
    if (outSize < sizeof(BB_BITMAP)) return -1;

    g5Data = out + sizeof(BB_BITMAP);
    g5MaxSize = outSize - sizeof(BB_BITMAP);

    packedPitch = (width + 7) / 8;
    packedImage = (uint8_t *)malloc(packedPitch);

    if (!packedImage) return -1;

    err = g5_encode_init(&enc, width, height, g5Data, g5MaxSize);
    if (err != G5_SUCCESS) {
      free(packedImage);
      return err;
    }
    for (y = 0; y < height; y++) {
        const uint8_t *s = rgba + (y * width * 4);
        uint8_t *d = packedImage;
        uint8_t u8Mask, u8;
        uint32_t u32Gray;
        u8Mask = 0x80; // 0 = black, 1 = white
        u8 = 0;
        for (x = 0; x < width; x++, s += 4) {
            u32Gray = s[0] + (s[1]*2) + s[2]; // simple RGBA->Gray prioritizes green
            if (u32Gray >= 512) { // threshold to B/W at 50% gray
                u8 |= u8Mask;
            }
            u8Mask >>= 1;
            if (u8Mask == 0) { // completed byte
                *d++ = u8;
                u8Mask = 0x80;
                u8 = 0;
            }
        } // for x
        if (u8Mask != 0x80) {
            *d++ = u8; // write last partial byte
        }
        err = g5_encode_encodeLine(&enc, packedImage);
        if (err < 0) {
          free(packedImage);
          return err;
        }
    } // for y

    g5CompressedSize = g5_encode_getOutSize(&enc);
    header = (BB_BITMAP *)out;
    header->u16Marker = BB_BITMAP_MARKER; // 0xBBBF
    header->width = width;
    header->height = height;
    header->size = g5CompressedSize;
    free(packedImage);
    return sizeof(BB_BITMAP) + g5CompressedSize;
} /* g5_encode_rgba() */

