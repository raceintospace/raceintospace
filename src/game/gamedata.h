#ifndef _GAMEDATA_H
#define _GAMEDATA_H

/* See Buzz_inc.h for comments on this */
#ifdef ALTERED_STRUCTURE_PACKING
#error Make sure that gamedata.h is included before Buzz_inc.h
#undefined /* to make errors appear on all compilers */
#endif

#include <race.h>
#include <stdio.h>
#include <int_types.h>  /* auto-generated inttypes.h */

/** Routines for read/write access to LITTLE ENDIAN data in game files */
extern size_t fread_uint8_t(uint8_t *dst, size_t nelem, FILE *file);
extern size_t fread_uint16_t(uint16_t *dst, size_t nelem, FILE *file);
extern size_t fread_uint32_t(uint32_t *dst, size_t nelem, FILE *file);
extern size_t fread_int8_t(int8_t *dst, size_t nelem, FILE *file);
extern size_t fread_int16_t(int16_t *dst, size_t nelem, FILE *file);
extern size_t fread_int32_t(int32_t *dst, size_t nelem, FILE *file);
extern size_t fwrite_uint8_t(const uint8_t *src, size_t nelem, FILE *file);
extern size_t fwrite_uint16_t(const uint16_t *src, size_t nelem, FILE *file);
extern size_t fwrite_uint32_t(const uint32_t *src, size_t nelem, FILE *file);
extern size_t fwrite_int8_t(const int8_t *src, size_t nelem, FILE *file);
extern size_t fwrite_int16_t(const int16_t *src, size_t nelem, FILE *file);
extern size_t fwrite_int32_t(const int32_t *src, size_t nelem, FILE *file);

/** \defgroup datafiles Datafile descriptions
 * 
 * @{
 * 
 *  Document every game data file: name, structures, layout.
 *  Each structure that can exist in a file should define total
 *  size in bytes. Also declare functions for reading/writing
 *  from/to stdio streams, if necessary.
 *
 * \verbatim
 * File: SEQ.KEY, FSEQ.KEY
 * Desc: Names of movie & audio files, each name 8 bytes long.
 *       Successful mission video sequences are stored in SEQ,
 *       failures in FSEQ.
 * Structure:
 *      u16 - number of subsequent filenames
 *      u8[8]* - filenames, each should be 0 terminated
 * \endverbatim
 *
 * \verbatim 
 * File: SEQ.DAT
 * Desc: Indexes to video and audio files for successful mission stages.
 * Structure:
 *       sequence of oGROUP structures,
 *       each containing ID sequence string and oLIST structures.
 * \endverbatim
 *
 * \verbatim
 * VAB.IMG is
 *  ** Player 0 image
 *   char[768]   -- palette
 *   uint16_t    -- length of following PCX compressed image
 *   image_data[] --
 *  ** Player 1 image
 *   char[768]   -- palette
 *   uint16_t    -- length of following PCX compressed image
 *   image_data[]
 * 
 * \endverbatim
 *
 * \verbatim
 * File: FSEQ.DAT
 * Desc: Indexes to video and audio files for failed mission stages.
 * Structure:
 *       50 Table structures
 *       sequence of oFGROUP structures,
 *       each containing ID sequence string and oLIST structures.
 * \endverbatim
 *
 * \verbatim
 * File: REPLAY.DAT
 * Desc: Contains indices to animation sequences of past player missions.
 * Structure:
 *       sequence of ReplayItem structures,
 *       each containing offsets to animations.
 * \endverbatim
 */

struct oLIST {
    int16_t aIdx;   /**< index of video filename in {SEQ,FSEQ}.KEY */
    int16_t sIdx;   /**< index of audio filename in {SEQ,FSEQ}.KEY */
};
#define sizeof_oLIST 4
extern size_t fread_oLIST(struct oLIST *dst, size_t num, FILE *f);

struct oGROUP {
    char ID[10];            /**< Sequence identifier */
    struct oLIST oLIST[5];  /**< Audio/video file indexes */
};
#define sizeof_oGROUP (10 + 5*sizeof_oLIST)
extern size_t fread_oGROUP(struct oGROUP *dst, size_t num, FILE *f);

/** Lookup table for filename to offsets
 * 
 */
struct Table {
    char fname[8];          /**< File name */
    int32_t foffset;        /**< Offset for starting position in the file */
    uint16_t size;          /**< Data size */
};
#define sizeof_Table (8 + 4 + 2)
extern size_t fread_Table(struct Table *dst, size_t num, FILE *f);

struct oFGROUP {
    char ID[15];            /**< Sequence identifier */
    struct oLIST oLIST[5];  /**< Audio/video file indexes */
};
#define sizeof_oFGROUP (15 + 5*sizeof_oLIST)
extern size_t fread_oFGROUP(struct oFGROUP *dst, size_t num, FILE *f);

/* used in: MOON.BUT, LMER.BUT, APROG.BUT, INTEL.BUT,
 *          RDFULL.BUT, PRFX.BUT, LFACIL.BUT, PRESR.BUT */
typedef struct {
    uint16_t size;
    uint32_t offset;
} SimpleHdr;
#define sizeof_SimpleHdr (2+4)
extern size_t fread_SimpleHdr(SimpleHdr *dst, size_t num, FILE *f);

#if 0
typedef struct ReplayItem {
   uint8_t Qty;             /* Number of Animated Sequences */
   uint16_t Off[35];        /* Offsets to Each animated Part */
} REPLAY;
#define sizeof_REPLAY (1 + 35*2)
extern size_t fread_REPLAY(REPLAY *dst, size_t num, FILE *f);
extern size_t fwrite_REPLAY(const REPLAY *src, size_t num, FILE *f);
#endif

#endif /* _GAMEDATA_H */

/** @} */
