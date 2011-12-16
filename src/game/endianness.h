#ifndef __ENDIANNESS_H__
#define __ENDIANNESS_H__

uint32_t _Swap32bit(uint32_t value);
uint16_t _Swap16bit(uint16_t value);
float _SwapFloat(float value);
void _SwapGameDat(void);
void _SwapEquipment(void);
void _SwapPatchHdr(PatchHdr *hdr);
void _SwapPatchHdrSmall(PatchHdrSmall *hdr);

#ifdef __BIG_ENDIAN__

#define Swap32bit(a)  (a) = _Swap32bit((a))
#define Swap16bit(a)  (a) = _Swap16bit((a))
#define SwapFloat(a)  (a) = _SwapFloat((a))
#define SwapGameDat() _SwapGameDat()
#define SwapEquipment() _SwapEquipment()
#define SwapPatchHdr(a) _SwapPatchHdr((a))
#define SwapPatchHdrSmall(a) _SwapPatchHdrSmall((a))

#else

#define Swap32bit(a)
#define Swap16bit(a)
#define SwapFloat(a)
#define SwapGameDat()
#define SwapEquipment()
#define SwapPatchHdr(a)
#define SwapPatchHdrSmall(a)

#endif


#endif // __ENDIANNESS_H__
