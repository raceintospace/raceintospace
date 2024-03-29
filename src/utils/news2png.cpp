#include <stdint.h>

#include <cerrno>
#include <cstdio>
#include <sstream>

#include <png.h>
#include <zlib.h>


/**
 * This code decodes the images stored in the news.cdr file.
 * I'm moving it here to clear up the news.cpp file, while
 * preserving the understanding of the file.
 *
 * It was recreated from code left in src/game/news.cpp; I did
 * not write the original code myself - I believe @kiyote gets
 * the credit (Patch #127?). I've adopted it to work without
 * relying on the display/ code, and thus without the SDL lib.
 *  -- rnyoakum
 */

#define MAX_X 320
#ifndef MIN
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#endif


/**
 * Palette values copied from src/utils/but2png.c
 * These are transformed using
 *      value << 2 | value >> 6
 */
const uint8_t PortPal0[] = { // USA Port Palette
    0, 0, 0, 60, 60, 60, 44, 44, 44, 34, 35, 35,
    20, 20, 20, 0, 0, 56, 0, 0, 38, 0, 0, 20,
    56, 0, 0, 38, 0, 0, 20, 0, 0, 60, 52, 0,
    56, 32, 0, 56, 16, 0, 56, 31, 0, 32, 44, 0,
    12, 32, 0, 0, 20, 0, 44, 44, 60, 32, 32, 60,
    16, 16, 60, 48, 0, 56, 48, 0, 32, 28, 0, 28,
    24, 8, 0, 40, 20, 8, 56, 48, 40, 0, 19, 0,
    0, 0, 26, 0, 0, 23, 0, 0, 0, 0, 0, 0,
    12, 12, 12, 15, 15, 15, 18, 18, 18, 22, 22, 22,
    25, 25, 25, 29, 29, 29, 32, 32, 32, 35, 35, 35,
    39, 39, 39, 42, 42, 42, 46, 46, 46, 49, 49, 49,
    52, 52, 52, 56, 56, 56, 59, 59, 59, 63, 63, 63,
    27, 39, 27, 15, 63, 63, 0, 0, 0, 0, 0, 0,
    30, 30, 45, 54, 48, 19, 47, 23, 23, 0, 0, 0,
    41, 41, 47, 37, 37, 43, 33, 33, 39, 29, 29, 35,
    25, 25, 31, 22, 22, 27, 18, 18, 23, 15, 15, 20,
    53, 53, 59, 49, 49, 55, 45, 45, 52, 41, 41, 48,
    38, 38, 44, 34, 34, 41, 31, 31, 37, 27, 27, 34,
    24, 24, 30, 21, 21, 27, 18, 18, 23, 15, 15, 20,
    12, 12, 16, 9, 9, 13, 6, 6, 9, 4, 4, 6,
    63, 63, 63, 61, 61, 60, 59, 59, 57, 58, 57, 55,
    56, 55, 53, 54, 54, 50, 51, 51, 47, 48, 48, 44,
    46, 45, 41, 43, 42, 38, 40, 40, 35, 38, 37, 33,
    35, 33, 31, 32, 31, 28, 29, 27, 25, 27, 25, 23,
    36, 46, 23, 32, 43, 20, 28, 40, 17, 25, 37, 14,
    21, 34, 12, 18, 31, 10, 15, 28, 8, 12, 26, 6,
    9, 23, 4, 7, 20, 3, 5, 17, 2, 3, 14, 1,
    1, 11, 0, 0, 8, 0, 0, 5, 0, 0, 3, 0,
    35, 46, 29, 33, 44, 25, 32, 42, 22, 32, 41, 19,
    31, 39, 16, 31, 38, 14, 31, 36, 11, 32, 35, 9,
    32, 33, 7, 31, 29, 5, 29, 26, 4, 27, 22, 3,
    25, 19, 1, 23, 16, 1, 21, 13, 0, 19, 10, 0,
    15, 18, 35, 13, 19, 39, 16, 21, 41, 15, 18, 42,
    15, 18, 46, 16, 20, 47, 17, 22, 45, 19, 23, 46,
    19, 23, 45, 18, 22, 42, 17, 21, 44, 15, 19, 45,
    19, 23, 48, 20, 25, 50, 21, 26, 51, 19, 24, 48,
    45, 0, 0, 40, 0, 0, 35, 0, 0, 31, 0, 0,
    26, 0, 0, 22, 0, 0, 17, 0, 0, 13, 0, 0,
    54, 48, 19, 50, 45, 16, 47, 42, 14, 44, 40, 12,
    40, 37, 10, 37, 34, 8, 34, 31, 6, 31, 29, 5,
    48, 48, 46, 46, 46, 43, 45, 45, 41, 43, 43, 39,
    42, 42, 37, 40, 40, 35, 39, 39, 33, 38, 38, 31,
    36, 36, 26, 35, 34, 22, 33, 32, 18, 32, 30, 15,
    31, 28, 11, 29, 26, 8, 28, 24, 5, 27, 22, 3,
    55, 55, 47, 53, 53, 43, 52, 51, 40, 50, 50, 36,
    49, 48, 33, 47, 47, 30, 46, 45, 27, 44, 43, 24,
    42, 49, 63, 38, 46, 63, 35, 43, 63, 32, 40, 63,
    29, 37, 62, 28, 35, 62, 25, 33, 61, 24, 31, 61,
    21, 29, 61, 20, 26, 60, 18, 24, 60, 16, 22, 60,
    14, 19, 58, 13, 17, 55, 9, 15, 53, 8, 12, 51,
    7, 9, 49, 5, 7, 47, 4, 5, 45, 3, 3, 43,
    1, 1, 40, 0, 0, 37, 0, 0, 35, 0, 0, 32,
    62, 56, 51, 59, 52, 47, 56, 49, 44, 53, 46, 40,
    51, 43, 37, 48, 40, 34, 45, 37, 31, 42, 35, 28,
    40, 32, 26, 37, 29, 23, 34, 27, 20, 32, 24, 18,
    29, 22, 16, 26, 19, 14, 23, 17, 12, 21, 15, 10,
    31, 29, 5, 29, 27, 4, 28, 25, 3, 26, 23, 3,
    25, 21, 2, 23, 19, 2, 22, 18, 1, 20, 16, 1,
    19, 14, 1, 17, 13, 1, 16, 11, 0, 14, 10, 0,
    13, 8, 0, 12, 7, 0, 10, 6, 0, 9, 5, 0,
    7, 4, 0, 6, 2, 0, 4, 2, 0, 3, 1, 0,
    1, 1, 0, 0, 0, 0, 45, 11, 0, 53, 16, 0,
    53, 26, 0, 49, 33, 0, 45, 25, 0, 44, 17, 0,
    57, 0, 0, 34, 0, 0, 0, 0, 0, 63, 63, 63
};
const uint8_t PortPal1[] = { // Soviet Port Palette
    0, 0, 0, 60, 60, 60, 44, 44, 44, 34, 35, 35,
    20, 20, 20, 0, 0, 56, 0, 0, 38, 0, 0, 20,
    56, 0, 0, 38, 0, 0, 20, 0, 0, 60, 52, 0,
    56, 32, 0, 56, 16, 0, 56, 31, 0, 32, 44, 0,
    12, 32, 0, 0, 20, 0, 44, 44, 60, 32, 32, 60,
    16, 16, 60, 48, 0, 56, 48, 0, 32, 28, 0, 28,
    24, 8, 0, 40, 20, 8, 56, 48, 40, 0, 0, 29,
    0, 0, 26, 0, 0, 23, 0, 0, 0, 0, 0, 0,
    12, 12, 12, 15, 15, 15, 18, 18, 18, 22, 22, 22,
    25, 25, 25, 29, 29, 29, 32, 32, 32, 35, 35, 35,
    39, 39, 39, 42, 42, 42, 46, 46, 46, 49, 49, 49,
    52, 52, 52, 56, 56, 56, 59, 59, 59, 63, 63, 63,
    54, 50, 33, 53, 47, 30, 52, 45, 27, 51, 42, 25,
    50, 39, 22, 49, 36, 20, 48, 33, 17, 47, 30, 15,
    41, 41, 47, 37, 37, 43, 33, 33, 39, 29, 29, 35,
    25, 25, 31, 22, 22, 27, 18, 18, 23, 15, 15, 20,
    53, 53, 59, 49, 49, 55, 45, 45, 52, 41, 41, 48,
    38, 38, 44, 34, 34, 41, 31, 31, 37, 27, 27, 34,
    24, 24, 30, 21, 21, 27, 18, 18, 23, 15, 15, 20,
    12, 12, 16, 9, 9, 13, 6, 6, 9, 4, 4, 6,
    63, 63, 63, 61, 61, 60, 59, 59, 57, 58, 57, 55,
    56, 55, 53, 54, 54, 50, 51, 51, 47, 48, 48, 44,
    46, 45, 41, 43, 42, 38, 40, 40, 35, 38, 37, 33,
    35, 33, 31, 32, 31, 28, 29, 27, 25, 27, 25, 23,
    36, 46, 23, 32, 43, 20, 28, 40, 17, 25, 37, 14,
    21, 34, 12, 18, 31, 10, 15, 28, 8, 12, 26, 6,
    9, 23, 4, 7, 20, 3, 5, 17, 2, 3, 14, 1,
    1, 11, 0, 0, 8, 0, 0, 5, 0, 0, 3, 0,
    35, 46, 29, 33, 44, 25, 32, 42, 22, 32, 41, 19,
    31, 39, 16, 31, 38, 14, 31, 36, 11, 32, 35, 9,
    32, 33, 7, 31, 29, 5, 29, 26, 4, 27, 22, 3,
    25, 19, 1, 23, 16, 1, 21, 13, 0, 19, 10, 0,
    15, 18, 35, 13, 19, 39, 16, 21, 41, 15, 18, 42,
    15, 18, 46, 16, 20, 47, 17, 22, 45, 19, 23, 46,
    19, 23, 45, 18, 22, 42, 17, 21, 44, 15, 19, 45,
    19, 23, 48, 20, 25, 50, 21, 26, 51, 19, 24, 48,
    45, 0, 0, 40, 0, 0, 35, 0, 0, 31, 0, 0,
    26, 0, 0, 22, 0, 0, 17, 0, 0, 13, 0, 0,
    54, 48, 19, 50, 45, 16, 47, 42, 14, 44, 40, 12,
    40, 37, 10, 37, 34, 8, 34, 31, 6, 31, 29, 5,
    48, 48, 46, 46, 46, 43, 45, 45, 41, 43, 43, 39,
    42, 42, 37, 40, 40, 35, 39, 39, 33, 38, 38, 31,
    36, 36, 26, 35, 34, 22, 33, 32, 18, 32, 30, 15,
    31, 28, 11, 29, 26, 8, 28, 24, 5, 27, 22, 3,
    55, 55, 47, 53, 53, 43, 52, 51, 40, 50, 50, 36,
    49, 48, 33, 47, 47, 30, 46, 45, 27, 44, 43, 24,
    42, 49, 63, 38, 46, 63, 35, 43, 63, 32, 40, 63,
    29, 37, 62, 28, 35, 62, 25, 33, 61, 24, 31, 61,
    21, 29, 61, 20, 26, 60, 18, 24, 60, 16, 22, 60,
    14, 19, 58, 13, 17, 55, 9, 15, 53, 8, 12, 51,
    7, 9, 49, 5, 7, 47, 4, 5, 45, 3, 3, 43,
    1, 1, 40, 0, 0, 37, 0, 0, 35, 0, 0, 32,
    62, 56, 51, 59, 52, 47, 56, 49, 44, 53, 46, 40,
    51, 43, 37, 48, 40, 34, 45, 37, 31, 42, 35, 28,
    40, 32, 26, 37, 29, 23, 34, 27, 20, 32, 24, 18,
    29, 22, 16, 26, 19, 14, 23, 17, 12, 21, 15, 10,
    31, 29, 5, 29, 27, 4, 28, 25, 3, 26, 23, 3,
    25, 21, 2, 23, 19, 2, 22, 18, 1, 20, 16, 1,
    19, 14, 1, 17, 13, 1, 16, 11, 0, 14, 10, 0,
    13, 8, 0, 12, 7, 0, 10, 6, 0, 45, 8, 0,
    53, 16, 0, 53, 26, 0, 49, 33, 0, 45, 25, 0,
    44, 17, 0, 47, 30, 15, 43, 27, 12, 39, 24, 9,
    35, 21, 6, 32, 18, 4, 28, 16, 2, 24, 13, 1,
    20, 11, 0, 17, 9, 0, 0, 0, 0, 63, 63, 63
};

/**
 * Palette values extracted from the USA_PORT.DAT & SOV_PORT.DAT files.
 */
// const uint8_t PortPal0[] = { // USA Port Palette
//     0, 0, 0, 240, 240, 240, 176, 176, 176, 136, 140, 140,
//     80, 80, 80, 0, 0, 224, 0, 0, 152, 0, 0, 80,
//     224, 0, 0, 152, 0, 0, 80, 0, 0, 240, 208, 0,
//     224, 128, 0, 224, 64, 0, 224, 124, 0, 128, 176, 0,
//     48, 128, 0, 0, 80, 0, 176, 176, 240, 128, 128, 240,
//     64, 64, 240, 192, 0, 224, 192, 0, 128, 112, 0, 112,
//     96, 32, 0, 160, 80, 32, 224, 192, 160, 0, 76, 0,
//     0, 0, 104, 0, 0, 92, 0, 0, 0, 0, 0, 0,
//     48, 48, 48, 60, 60, 60, 72, 72, 72, 88, 88, 88,
//     100, 100, 100, 116, 116, 116, 128, 128, 128, 140, 140, 140,
//     156, 156, 156, 168, 168, 168, 184, 184, 184, 196, 196, 196,
//     208, 208, 208, 224, 224, 224, 236, 236, 236, 252, 252, 252,
//     108, 156, 108, 60, 252, 252, 0, 0, 0, 0, 0, 0,
//     120, 120, 180, 216, 192, 76, 188, 92, 92, 0, 0, 0,
//     164, 164, 188, 148, 148, 172, 132, 132, 156, 116, 116, 140,
//     100, 100, 124, 88, 88, 108, 72, 72, 92, 60, 60, 80,
//     212, 212, 236, 196, 196, 220, 180, 180, 208, 164, 164, 192,
//     152, 152, 176, 136, 136, 164, 124, 124, 148, 108, 108, 136,
//     96, 96, 120, 84, 84, 108, 72, 72, 92, 60, 60, 80,
//     48, 48, 64, 36, 36, 52, 24, 24, 36, 16, 16, 24,
//     252, 252, 252, 244, 244, 240, 236, 236, 228, 232, 228, 220,
//     224, 220, 212, 216, 216, 200, 204, 204, 188, 192, 192, 176,
//     184, 180, 164, 172, 168, 152, 160, 160, 140, 152, 148, 132,
//     140, 132, 124, 128, 124, 112, 116, 108, 100, 108, 100, 92,
//     144, 184, 92, 128, 172, 80, 112, 160, 68, 100, 148, 56,
//     84, 136, 48, 72, 124, 40, 60, 112, 32, 48, 104, 24,
//     36, 92, 16, 28, 80, 12, 20, 68, 8, 12, 56, 4,
//     4, 44, 0, 0, 32, 0, 0, 20, 0, 0, 12, 0,
//     140, 184, 116, 132, 176, 100, 128, 168, 88, 128, 164, 76,
//     124, 156, 64, 124, 152, 56, 124, 144, 44, 128, 140, 36,
//     128, 132, 28, 124, 116, 20, 116, 104, 16, 108, 88, 12,
//     100, 76, 4, 92, 64, 4, 84, 52, 0, 76, 40, 0,
//     60, 72, 140, 52, 76, 156, 64, 84, 164, 60, 72, 168,
//     60, 72, 184, 64, 80, 188, 68, 88, 180, 76, 92, 184,
//     76, 92, 180, 72, 88, 168, 68, 84, 176, 60, 76, 180,
//     76, 92, 192, 80, 100, 200, 84, 104, 204, 76, 96, 192,
//     180, 0, 0, 160, 0, 0, 140, 0, 0, 124, 0, 0,
//     104, 0, 0, 88, 0, 0, 68, 0, 0, 52, 0, 0,
//     216, 192, 76, 200, 180, 64, 188, 168, 56, 176, 160, 48,
//     160, 148, 40, 148, 136, 32, 136, 124, 24, 124, 116, 20,
//     192, 192, 184, 184, 184, 172, 180, 180, 164, 172, 172, 156,
//     168, 168, 148, 160, 160, 140, 156, 156, 132, 152, 152, 124,
//     144, 144, 104, 140, 136, 88, 132, 128, 72, 128, 120, 60,
//     124, 112, 44, 116, 104, 32, 112, 96, 20, 108, 88, 12,
//     220, 220, 188, 212, 212, 172, 208, 204, 160, 200, 200, 144,
//     196, 192, 132, 188, 188, 120, 184, 180, 108, 176, 172, 96,
//     168, 196, 252, 152, 184, 252, 140, 172, 252, 128, 160, 252,
//     116, 148, 248, 112, 140, 248, 100, 132, 244, 96, 124, 244,
//     84, 116, 244, 80, 104, 240, 72, 96, 240, 64, 88, 240,
//     56, 76, 232, 52, 68, 220, 36, 60, 212, 32, 48, 204,
//     28, 36, 196, 20, 28, 188, 16, 20, 180, 12, 12, 172,
//     4, 4, 160, 0, 0, 148, 0, 0, 140, 0, 0, 128,
//     248, 224, 204, 236, 208, 188, 224, 196, 176, 212, 184, 160,
//     204, 172, 148, 192, 160, 136, 180, 148, 124, 168, 140, 112,
//     160, 128, 104, 148, 116, 92, 136, 108, 80, 128, 96, 72,
//     116, 88, 64, 104, 76, 56, 92, 68, 48, 84, 60, 40,
//     124, 116, 20, 116, 108, 16, 112, 100, 12, 104, 92, 12,
//     100, 84, 8, 92, 76, 8, 88, 72, 4, 80, 64, 4,
//     76, 56, 4, 68, 52, 4, 64, 44, 0, 56, 40, 0,
//     52, 32, 0, 48, 28, 0, 40, 24, 0, 36, 20, 0,
//     28, 16, 0, 24, 8, 0, 16, 8, 0, 12, 4, 0,
//     4, 4, 0, 0, 0, 0, 180, 44, 0, 212, 64, 0,
//     212, 104, 0, 196, 132, 0, 180, 100, 0, 176, 68, 0,
//     228, 0, 0, 136, 0, 0, 0, 0, 0, 252, 252, 252
// };
// const uint8_t PortPal1[] = { // Soviet Port Palette
//     0, 0, 0, 240, 240, 240, 176, 176, 176, 136, 140, 140,
//     80, 80, 80, 0, 0, 224, 0, 0, 152, 0, 0, 80,
//     224, 0, 0, 152, 0, 0, 80, 0, 0, 240, 208, 0,
//     224, 128, 0, 224, 64, 0, 224, 124, 0, 128, 176, 0,
//     48, 128, 0, 0, 80, 0, 176, 176, 240, 128, 128, 240,
//     64, 64, 240, 192, 0, 224, 192, 0, 128, 112, 0, 112,
//     96, 32, 0, 160, 80, 32, 224, 192, 160, 0, 0, 116,
//     0, 0, 104, 0, 0, 92, 0, 0, 0, 0, 0, 0,
//     48, 48, 48, 60, 60, 60, 72, 72, 72, 88, 88, 88,
//     100, 100, 100, 116, 116, 116, 128, 128, 128, 140, 140, 140,
//     156, 156, 156, 168, 168, 168, 184, 184, 184, 196, 196, 196,
//     208, 208, 208, 224, 224, 224, 236, 236, 236, 252, 252, 252,
//     216, 200, 132, 212, 188, 120, 208, 180, 108, 204, 168, 100,
//     200, 156, 88, 196, 144, 80, 192, 132, 68, 188, 120, 60,
//     164, 164, 188, 148, 148, 172, 132, 132, 156, 116, 116, 140,
//     100, 100, 124, 88, 88, 108, 72, 72, 92, 60, 60, 80,
//     212, 212, 236, 196, 196, 220, 180, 180, 208, 164, 164, 192,
//     152, 152, 176, 136, 136, 164, 124, 124, 148, 108, 108, 136,
//     96, 96, 120, 84, 84, 108, 72, 72, 92, 60, 60, 80,
//     48, 48, 64, 36, 36, 52, 24, 24, 36, 16, 16, 24,
//     252, 252, 252, 244, 244, 240, 236, 236, 228, 232, 228, 220,
//     224, 220, 212, 216, 216, 200, 204, 204, 188, 192, 192, 176,
//     184, 180, 164, 172, 168, 152, 160, 160, 140, 152, 148, 132,
//     140, 132, 124, 128, 124, 112, 116, 108, 100, 108, 100, 92,
//     144, 184, 92, 128, 172, 80, 112, 160, 68, 100, 148, 56,
//     84, 136, 48, 72, 124, 40, 60, 112, 32, 48, 104, 24,
//     36, 92, 16, 28, 80, 12, 20, 68, 8, 12, 56, 4,
//     4, 44, 0, 0, 32, 0, 0, 20, 0, 0, 12, 0,
//     140, 184, 116, 132, 176, 100, 128, 168, 88, 128, 164, 76,
//     124, 156, 64, 124, 152, 56, 124, 144, 44, 128, 140, 36,
//     128, 132, 28, 124, 116, 20, 116, 104, 16, 108, 88, 12,
//     100, 76, 4, 92, 64, 4, 84, 52, 0, 76, 40, 0,
//     60, 72, 140, 52, 76, 156, 64, 84, 164, 60, 72, 168,
//     60, 72, 184, 64, 80, 188, 68, 88, 180, 76, 92, 184,
//     76, 92, 180, 72, 88, 168, 68, 84, 176, 60, 76, 180,
//     76, 92, 192, 80, 100, 200, 84, 104, 204, 76, 96, 192,
//     180, 0, 0, 160, 0, 0, 140, 0, 0, 124, 0, 0,
//     104, 0, 0, 88, 0, 0, 68, 0, 0, 52, 0, 0,
//     216, 192, 76, 200, 180, 64, 188, 168, 56, 176, 160, 48,
//     160, 148, 40, 148, 136, 32, 136, 124, 24, 124, 116, 20,
//     192, 192, 184, 184, 184, 172, 180, 180, 164, 172, 172, 156,
//     168, 168, 148, 160, 160, 140, 156, 156, 132, 152, 152, 124,
//     144, 144, 104, 140, 136, 88, 132, 128, 72, 128, 120, 60,
//     124, 112, 44, 116, 104, 32, 112, 96, 20, 108, 88, 12,
//     220, 220, 188, 212, 212, 172, 208, 204, 160, 200, 200, 144,
//     196, 192, 132, 188, 188, 120, 184, 180, 108, 176, 172, 96,
//     168, 196, 252, 152, 184, 252, 140, 172, 252, 128, 160, 252,
//     116, 148, 248, 112, 140, 248, 100, 132, 244, 96, 124, 244,
//     84, 116, 244, 80, 104, 240, 72, 96, 240, 64, 88, 240,
//     56, 76, 232, 52, 68, 220, 36, 60, 212, 32, 48, 204,
//     28, 36, 196, 20, 28, 188, 16, 20, 180, 12, 12, 172,
//     4, 4, 160, 0, 0, 148, 0, 0, 140, 0, 0, 128,
//     248, 224, 204, 236, 208, 188, 224, 196, 176, 212, 184, 160,
//     204, 172, 148, 192, 160, 136, 180, 148, 124, 168, 140, 112,
//     160, 128, 104, 148, 116, 92, 136, 108, 80, 128, 96, 72,
//     116, 88, 64, 104, 76, 56, 92, 68, 48, 84, 60, 40,
//     124, 116, 20, 116, 108, 16, 112, 100, 12, 104, 92, 12,
//     100, 84, 8, 92, 76, 8, 88, 72, 4, 80, 64, 4,
//     76, 56, 4, 68, 52, 4, 64, 44, 0, 56, 40, 0,
//     52, 32, 0, 48, 28, 0, 40, 24, 0, 180, 32, 0,
//     212, 64, 0, 212, 104, 0, 196, 132, 0, 180, 100, 0,
//     176, 68, 0, 188, 120, 60, 172, 108, 48, 156, 96, 36,
//     140, 84, 24, 128, 72, 16, 112, 64, 8, 96, 52, 4,
//     80, 44, 0, 68, 36, 0, 0, 0, 0, 252, 252, 252
// };


struct Color {
    uint8_t r, g, b;
};

size_t fread_int32_t(int32_t *dst, size_t nelem, FILE *file);
void InitPalette(int plr, Color pal[256]);
void PrintUsage();
void ReadEvents(int plr, FILE *ffin);
int WriteImage(const char *filename, int width, int height, Color pal[256],
               const uint8_t *pixels);


/**
 * Start of Endian-friendly reading code.
 * Endian-friendly reading copied from src/game/gamedata.cpp
 */
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


DECL_UNSIGNED_GET(16, 8)
DECL_UNSIGNED_GET(32, 16)

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

DECL_xINT_FREAD(8, u)
DECL_xINT_FREAD(32, u)

/**
 * End of Endian-translation code
 */


void InitPalette(int plr, Color pal[256])
{
    if (plr != 0 && plr != 1) {
        fprintf(stderr, "Invalid player selection!");
        return;
    }

    const uint8_t *basePalette = (plr == 0) ? &PortPal0[0] : &PortPal1[0];

    for (int i = 0; i < 32; i++) {
        pal[i].r = basePalette[3 * i + 0];
        pal[i].g = basePalette[3 * i + 1];
        pal[i].b = basePalette[3 * i + 2];
    }

    for (int i = 32; i < 256; i++) {
        pal[i].r = 0;
        pal[i].g = 0;
        pal[i].b = 0;
    }
}


void PrintUsage()
{
    fprintf(stderr, "usage: news2png <news.cdr>\n");
}


/**
 * Decode all news card images for the given side.
 *
 * TODO: Add more error handling...
 */
void ReadEvents(int plr, FILE *ffin)
{
    uint32_t offset;
    uint32_t length;
    Color pal[256];

    InitPalette(plr, pal);

    for (int card = 0; card < 115; card++) {
        fseek(ffin, (plr * 115 + card) * 2 * sizeof(uint32_t), SEEK_SET);
        fread_uint32_t(&offset, 1, ffin);
        fread_uint32_t(&length, 1, ffin);

        if (! offset || ! length) {
            continue;
        }

        fseek(ffin, offset, SEEK_SET);

        // fread(&pal[128], 3 * 128, 1, ffin);
        // Read byte by byte to avoid having to use pragma packing
        for (int i = 128; i < 256; i++) {
            fread(&pal[i].r, sizeof(uint8_t), 1, ffin);
            fread(&pal[i].g, sizeof(uint8_t), 1, ffin);
            fread(&pal[i].b, sizeof(uint8_t), 1, ffin);
        }

        size_t pixelCount = (size_t) MIN(length, MAX_X * 110);
        uint8_t *pixels = new uint8_t[pixelCount];
        fread(pixels, pixelCount, 1, ffin);

        std::stringstream ss;
        ss << "news.cdr." << ((plr == 1) ? "ussr" : "usa")
           << "." << card << ".png";
        WriteImage( ss.str().c_str(), 320, 110, pal, pixels );
        delete[] pixels;
    }
}


int WriteImage(const char *filename, int width, int height, Color pal[256],
               const uint8_t *pixels)
{
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    uint8_t *rows[240];
    uint8_t alpha_values[256];
    int maximum_transparent_color;
    int i;

    fp = fopen(filename, "wb");

    if (!fp) {
        fprintf(stderr, "unable to open output file: %s\n",
                strerror(errno));
        return 4;
    }

    png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        return 5;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr) {
        return 6;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        fprintf(stderr, "PNG write error!\n");
        return 7;
    }

    png_init_io(png_ptr, fp);

    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

    png_set_IHDR(png_ptr, info_ptr,
                 width, height,
                 8, PNG_COLOR_TYPE_PALETTE,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
                );

    png_color png_pal[256];

    for (i = 0; i < 256; i++) {
        png_pal[i].red = (pal[i].r << 2 | pal[i].r >> 6);
        png_pal[i].green = (pal[i].g << 2 | pal[i].g >> 6);
        png_pal[i].blue = (pal[i].b << 2 | pal[i].b >> 6);
    }

    png_set_PLTE(png_ptr, info_ptr, png_pal, 256);

    // set up a buffer of alpha values as needed
    memset(alpha_values, 255, sizeof(alpha_values));
    maximum_transparent_color = -1;

    for (i = 0; i < height; i++) {
        rows[i] = (uint8_t *)(pixels + (i * width));
    }

    png_set_rows(png_ptr, info_ptr, rows);

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    return 0;
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    FILE *ffin = fopen(argv[1], "rb");

    if (! ffin) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1],
                strerror(errno));
        return 2;
    }

    ReadEvents(0, ffin); // USA cards
    ReadEvents(1, ffin); // USSR cards

    fclose(ffin);
    return 0;
}
