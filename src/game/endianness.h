#ifndef __ENDIANNESS_H__
#define __ENDIANNESS_H__

#include <boost/version.hpp>
#if BOOST_VERSION >= 106800
#  include <boost/predef/other/endian.h>
#else
#  include <boost/detail/endian.hpp>
#endif

#include "data.h"

uint32_t _Swap32bit(uint32_t value);
uint16_t _Swap16bit(uint16_t value);
float _SwapFloat(float value);
void _SwapPatchHdr(PatchHdr *hdr);
void _SwapPatchHdrSmall(PatchHdrSmall *hdr);

#if BOOST_ENDIAN_BIG_BYTE || defined(BOOST_BIG_ENDIAN)

#define Swap32bit(a)  (a) = _Swap32bit((a))
#define Swap16bit(a)  (a) = _Swap16bit((a))
#define SwapFloat(a)  (a) = _SwapFloat((a))
#define SwapPatchHdr(a) _SwapPatchHdr((a))
#define SwapPatchHdrSmall(a) _SwapPatchHdrSmall((a))

#elif BOOST_ENDIAN_LITTLE_BYTE || defined(BOOST_LITTLE_ENDIAN)

#define Swap32bit(a)
#define Swap16bit(a)
#define SwapFloat(a)
#define SwapGameDat()
#define SwapPatchHdr(a)
#define SwapPatchHdrSmall(a)

#else
#error "Uh oh! I don't know which endianness to pick."
#endif

#endif // __ENDIANNESS_H__
