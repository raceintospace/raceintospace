#ifndef _MM_FILE_H
#define _MM_FILE_H

#include <stdio.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theora.h>
#include <SDL.h>

#define MM_AUDIO_FORMAT AUDIO_U16SYS

enum stream_type
{
	MEDIA_AUDIO = 1,
	MEDIA_VIDEO = 2
};

typedef struct
{
	FILE *file;
	ogg_sync_state sync;
	ogg_stream_state *audio;
	vorbis_info *audio_info;
	vorbis_dsp_state *audio_ctx;
	vorbis_block *audio_blk;
	ogg_stream_state *video;
	theora_info *video_info;
	theora_state *video_ctx;
    unsigned end_of_stream;
    unsigned drop_packets;
} mm_file;

extern int mm_open(mm_file * mf, const char *fname);
extern int mm_open_fp(mm_file * mf, FILE *file);
extern unsigned mm_ignore(mm_file * mf, unsigned mask);
extern int mm_close(mm_file * mf);
extern int mm_video_info(const mm_file * mf, unsigned *width, unsigned *height, float *fps);
extern int mm_audio_info(const mm_file * mf, unsigned *channels, unsigned *rate);
extern int mm_decode_video(mm_file * mf, SDL_Overlay * ovl);
extern int mm_decode_audio(mm_file * mf, void *buf, int buflen);
#if 0
extern int mm_convert_audio(mm_file * mf, void *buf, int buflen, SDL_AudioSpec *spec);
#endif

#endif /* _MM_FILE_H */
