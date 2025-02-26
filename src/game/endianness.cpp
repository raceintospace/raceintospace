#include "endianness.h"

#include <cassert>

#include "Buzz_inc.h"
#include "game_main.h"


// Need these functions to always exist
uint32_t _Swap32bit(uint32_t value)
{
    return (value >> 24)
           | ((value >> 8) & 0x0000FF00)
           | ((value << 8) & 0x00FF0000)
           | (value << 24);
}

uint16_t _Swap16bit(uint16_t value)
{
    return (value << 8) | (value >> 8);
}

float _SwapFloat(float value)
{
    union {
        float f;
        unsigned char b[4];
    } dat1, dat2;

    dat1.f = value;
    dat2.b[0] = dat1.b[3];
    dat2.b[1] = dat1.b[2];
    dat2.b[2] = dat1.b[1];
    dat2.b[3] = dat1.b[0];
    return dat2.f;
}

void _SwapPatchHdr(PatchHdr *hdr)
{
    assert(hdr);

    hdr->w = _Swap16bit(hdr->w);
    hdr->h = _Swap16bit(hdr->h);
    hdr->size = _Swap16bit(hdr->size);
    hdr->offset = _Swap32bit(hdr->offset);
}

void _SwapPatchHdrSmall(PatchHdrSmall *hdr)
{
    assert(hdr);

    hdr->size = _Swap16bit(hdr->size);
    hdr->offset = _Swap32bit(hdr->offset);
}

/* vim: set noet ts=4 sw=4 tw=77: */
