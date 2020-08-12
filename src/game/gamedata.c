#include <race.h>
#include <stdio.h>
#include <gamedata.h>
#include <int_types.h>  /* auto-generated inttypes.h */

/* Definitions for scalar types */

#define sizeof_char (sizeof(char))
#define sizeof_int8_t (sizeof(int8_t))
#define sizeof_int16_t (sizeof(int16_t))
#define sizeof_int32_t (sizeof(int32_t))
#define sizeof_uint8_t (sizeof(uint8_t))
#define sizeof_uint16_t (sizeof(uint16_t))
#define sizeof_uint32_t (sizeof(uint32_t))

/* XXX: check if char is signed or unsigned */
#define get_char (char)get_uint8_t
#define put_char (char)get_uint8_t

static inline uint8_t
get_uint8_t(const void *buf)
{
    return *((uint8_t *)buf);
}

#define DECL_UNSIGNED_GET(bits, from) \
    static inline uint ## bits ## _t \
    get_uint ## bits ##_t (const void *buf) \
    { \
        return get_uint ## from ##_t (buf) \
        | (get_uint ## from##_t ((const char*)buf + (from)/8) << (from)); \
    } \

#define DECL_SIGNED_GET(bits) \
    static inline int ## bits ## _t \
    get_int ## bits ##_t (const void *buf) \
    { \
        return (int ## bits ## _t) get_uint ## bits ##_t (buf);  \
    } \

DECL_UNSIGNED_GET(16, 8)
DECL_UNSIGNED_GET(32, 16)
DECL_SIGNED_GET(8)
DECL_SIGNED_GET(16)
DECL_SIGNED_GET(32)

static inline void
put_uint8_t(void *buf, uint8_t v)
{
    *((uint8_t *)buf) = v;
}

/* works only up to 32 bits! */
#define DECL_UNSIGNED_PUT(bits, from) \
    static inline void \
    put_uint ## bits ## _t (void *buf, uint ## bits ## _t v) \
    { \
        put_uint ## from ##_t (buf, (v & ((1 << (from))-1))); \
        put_uint ## from ##_t ((char*)buf + (from)/8, \
                (v >> (from)) & ((1L << (from))-1)); \
    } \

#define DECL_SIGNED_PUT(bits) \
    static inline void \
    put_int ## bits ##_t (void *buf, int##bits##_t v) \
    { \
        put_uint ## bits ## _t (buf, (uint##bits##_t)v); \
    } \

DECL_UNSIGNED_PUT(16, 8)
DECL_UNSIGNED_PUT(32, 16)
DECL_SIGNED_PUT(8)
DECL_SIGNED_PUT(16)
DECL_SIGNED_PUT(32)

#define DECL_xINT_FREAD(bits, sign) \
    size_t fread_##sign##int##bits##_t \
        (sign##int##bits##_t *p, size_t n, FILE *f) { \
        int i = 0; int elems = 0; \
        elems = fread (p, (bits)/8, n, f); \
        for (i = 0; i < elems; ++i, ++p) { \
            *p = get_##sign##int##bits##_t(p); \
        } \
        return elems; \
    } \

#define DECL_xINT_FWRITE(bits, sign) \
    size_t fwrite_##sign##int##bits##_t \
        (const sign##int##bits##_t *p, size_t n, FILE *f) { \
        sign##int##bits##_t t[256]; \
        int i = 0, elems = 0, total = 0; \
        while (n > 0) { \
            elems = (n < 256) ? n : 256; \
            for (i = 0; i < elems; ++i, ++p) \
                put_##sign##int##bits##_t(t+i, *p); \
            elems = fwrite(t, (bits)/8, elems, f); \
            if (!elems) break; \
            n -= elems; \
            total += elems; \
        } \
        return total; \
    } \

DECL_xINT_FREAD(8, )
DECL_xINT_FREAD(16, )
DECL_xINT_FREAD(32, )
DECL_xINT_FREAD(8, u)
DECL_xINT_FREAD(16,u)
DECL_xINT_FREAD(32,u)
DECL_xINT_FWRITE(8, )
DECL_xINT_FWRITE(16, )
DECL_xINT_FWRITE(32, )
DECL_xINT_FWRITE(8, u)
DECL_xINT_FWRITE(16,u)
DECL_xINT_FWRITE(32,u)

/* Boilerplate macrology for structure types */

#define DECL_FREAD(struct, type, bufelems) \
size_t \
fread_##type(struct type *dst, size_t num, FILE *f) \
{ \
    uint8_t tmp[(bufelems)*sizeof_##type]; \
    int i = 0, total = 0, elems = 0; \
    while (num > 0) \
    { \
        elems = (num < (bufelems)) ? num : (bufelems); \
        elems = fread(tmp, sizeof_##type, elems, f); \
        if (!elems) \
            break; \
        for (i = 0; i < elems; ++i) \
            get_##type(dst++, tmp+i*sizeof_##type); \
        total += elems; \
        num -= elems; \
    } \
    return total; \
} \

#define DECL_FWRITE(struct, type, bufelems) \
size_t \
fwrite_##type(const struct type *src, size_t num, FILE *f) \
{ \
    uint8_t tmp[(bufelems)*sizeof_##type]; \
    int i = 0, total = 0, elems = 0; \
    while (num > 0) \
    { \
        elems = (num < (bufelems)) ? num : (bufelems); \
        for (i = 0; i < elems; ++i) \
            put_##type(tmp+i*sizeof_##type, src++); \
        elems = fwrite(tmp, sizeof_##type, elems, f); \
        if (!elems) \
            break; \
        total += elems; \
        num -= elems; \
    } \
    return total; \
} \

#define DECL_GET_START(struct, type) \
static inline void \
get_##type (struct type *dst, const uint8_t *src) \
{ \
    int i = 0; \

#define DECL_GET_FIELD_SCALAR(type, name, num) \
    for (i = 0; i < (num); ++i) \
    { \
        *(((type *)&dst->name)+i) = get_##type(src); \
        src += sizeof_##type; \
    } \

#define DECL_GET_FIELD_STRUCT(str, type, name, num) \
    for (i = 0; i < (num); ++i) \
    { \
        get_##type((((str type *)&dst->name)+i), src); \
        src += sizeof_##type; \
    } \

#define DECL_GET_END }

#define DECL_PUT_START(struct, type) \
static inline void \
put_##type (uint8_t *dst, const struct type *src) \
{ \
    int i = 0; \

#define DECL_PUT_FIELD_SCALAR(type, name, num) \
    for (i = 0; i < (num); ++i) \
    { \
        put_##type(dst, *(((type *)&src->name)+i)); \
        dst += sizeof_##type; \
    } \

#define DECL_PUT_FIELD_STRUCT(str, type, name, num) \
    for (i = 0; i < (num); ++i) \
    { \
        put_##type(dst, ((str type *)(&src->name))+i); \
        dst += sizeof_##type; \
    } \

#define DECL_PUT_END }

/* START STRUCTURES */
/*
 * This should be auto-generated. CPP is just too inflexible.
 * It is messy because both typedef struct { ... } A
 * and struct A { ... } should work.
 *
 * get_XXX and put_XXX are mandatory for every structure.
 * fread_XXX and fwrite_XXX should be added if required.
 */

/* oLIST */

DECL_GET_START(struct, oLIST)
    DECL_GET_FIELD_SCALAR(int16_t, aIdx, 1)
    DECL_GET_FIELD_SCALAR(int16_t, sIdx, 1)
DECL_GET_END

DECL_PUT_START(struct, oLIST)
    DECL_PUT_FIELD_SCALAR(int16_t, aIdx, 1)
    DECL_PUT_FIELD_SCALAR(int16_t, sIdx, 1)
DECL_PUT_END

/* DECL_FREAD(struct, oLIST, 32) */
/* DECL_FWRITE(struct, oLIST, 32) */

/* oGROUP */

DECL_GET_START(struct, oGROUP)
    DECL_GET_FIELD_SCALAR(uint8_t, ID, 10)
    DECL_GET_FIELD_STRUCT(struct, oLIST, oLIST, 5)
DECL_GET_END

DECL_PUT_START(struct, oGROUP)
    DECL_PUT_FIELD_SCALAR(uint8_t, ID, 10)
    DECL_PUT_FIELD_STRUCT(struct, oLIST, oLIST, 5)
DECL_PUT_END

DECL_FREAD(struct, oGROUP, 32)
/* DECL_FWRITE(struct, oGROUP, 32) */

/* Table */

DECL_GET_START(struct, Table)
    DECL_GET_FIELD_SCALAR(uint8_t, fname, 8)
    DECL_GET_FIELD_SCALAR(int32_t, foffset, 1)
    DECL_GET_FIELD_SCALAR(uint16_t, size, 1)
DECL_GET_END

DECL_PUT_START(struct, Table)
    DECL_PUT_FIELD_SCALAR(uint8_t, fname, 8)
    DECL_PUT_FIELD_SCALAR(int32_t, foffset, 1)
    DECL_PUT_FIELD_SCALAR(uint16_t, size, 1)
DECL_PUT_END

DECL_FREAD(struct, Table, 32)
/* DECL_FWRITE(struct, oGROUP, 32) */

/* oFGROUP */

DECL_GET_START(struct, oFGROUP)
    DECL_GET_FIELD_SCALAR(uint8_t, ID, 15)
    DECL_GET_FIELD_STRUCT(struct, oLIST, oLIST, 5)
DECL_GET_END

DECL_PUT_START(struct, oFGROUP)
    DECL_PUT_FIELD_SCALAR(uint8_t, ID, 15)
    DECL_PUT_FIELD_STRUCT(struct, oLIST, oLIST, 5)
DECL_PUT_END

DECL_FREAD(struct, oFGROUP, 32)
/* DECL_FWRITE(struct, oFGROUP, 32) */

/* SimpleHdr */

DECL_GET_START(, SimpleHdr)
    DECL_GET_FIELD_SCALAR(uint16_t, size, 1)
    DECL_GET_FIELD_SCALAR(uint32_t, offset, 1)
DECL_GET_END

DECL_PUT_START(, SimpleHdr)
    DECL_PUT_FIELD_SCALAR(uint16_t, size, 1)
    DECL_PUT_FIELD_SCALAR(uint32_t, offset, 1)
DECL_PUT_END

DECL_FREAD(, SimpleHdr, 32)
/* DECL_FWRITE(, SimpleHdr, 32) */

#if 0
/* REPLAY */
DECL_GET_START(, REPLAY)
    DECL_GET_FIELD_SCALAR(uint8_t, Qty, 1)
    DECL_GET_FIELD_SCALAR(uint16_t, Off, 35)
DECL_GET_END

DECL_PUT_START(, REPLAY)
    DECL_PUT_FIELD_SCALAR(uint8_t, Qty, 1)
    DECL_PUT_FIELD_SCALAR(uint16_t, Off, 35)
DECL_PUT_END

DECL_FREAD(, REPLAY, 32)
DECL_FWRITE(, REPLAY, 32)
#endif

#if 0
#include <stdio.h>
uint8_t arr[1000];
int main (void)
{
    char *fname = "/tmp/data.test";
    FILE *f = fopen(fname, "w+b");
    struct oGROUP a, b = {
        "abcdefgh",
        {
            { 0x1234, 0x4321 },
            { 0x1034, 0x0321 },
            { 0x0234, 0x4301 },
            { 0x1204, 0x4021 },
            { 0x1230, 0x4320 }
        } };
    fwrite_oGROUP(&b, 1, f);
    fseek(f, 0, SEEK_SET);
    fread_oGROUP(&a, 1, f);
    fclose(f);
    remove(fname);
    return 0;
}
#endif
