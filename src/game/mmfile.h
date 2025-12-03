#ifndef _MM_FILE_H
#define _MM_FILE_H

#include <cstdio>
#include <memory>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <theora/theora.h>
#include <SDL.h>

enum media_type : bool
{
    AUDIO = false,
    VIDEO = true
};

struct Theora_info_raii
{
    std::unique_ptr<theora_info> th_info;

    Theora_info_raii()
    : th_info{new theora_info{}}
    {
        theora_info_init(th_info.get());
    }

    Theora_info_raii(Theora_info_raii&&) = default;
    Theora_info_raii& operator=(Theora_info_raii&&) = default;

    theora_info* get() {return th_info.get();}
    theora_info* release() {return th_info.release();}

    ~Theora_info_raii()
    {
        if (th_info == nullptr) return;        
        theora_info_clear(th_info.get());
    }
};

struct Ogg_stream_raii
{
    std::unique_ptr<ogg_stream_state> stream;

    Ogg_stream_raii() = default;

    Ogg_stream_raii(ogg_page* pg)
    : stream{new ogg_stream_state{}}
    {
        ogg_stream_init(stream.get(), ogg_page_serialno(pg));
    }

    Ogg_stream_raii(Ogg_stream_raii&&) = default;
    Ogg_stream_raii& operator=(Ogg_stream_raii&&) = default;

    ogg_stream_state* get() {return stream.get();}
    ogg_stream_state* release() {return stream.release();}

    ~Ogg_stream_raii()
    {
        if (stream == nullptr) return;
        ogg_stream_clear(stream.get());
    }
};

struct Vorbis_info_raii
{
    std::unique_ptr<vorbis_info> vo_info;

    Vorbis_info_raii()
    : vo_info{new vorbis_info{}}
    {
        vorbis_info_init(vo_info.get());
    }

    Vorbis_info_raii(Vorbis_info_raii&&) = default;
    Vorbis_info_raii& operator=(Vorbis_info_raii&&) = default;

    vorbis_info* get() {return vo_info.get();}
    vorbis_info* release() {return vo_info.release();}

    ~Vorbis_info_raii()
    {
        if (vo_info == nullptr) return;
        vorbis_info_clear(vo_info.get());
    }
};

class Multimedia
{
public:
    bool ignore_audio{};
    bool ignore_video{};

private:
    bool good{true};
    bool stream_has_ended[2]{};
    struct {
        FILE *file;
        ogg_sync_state sync;

        Ogg_stream_raii audio;
        Vorbis_info_raii audio_info;
        std::unique_ptr<vorbis_dsp_state> audio_ctx;
        std::unique_ptr<vorbis_block> audio_blk;

        Ogg_stream_raii video;
        Theora_info_raii video_info;
        std::unique_ptr<theora_state> video_ctx;
    } mmf{};
    ogg_page last_read;

public:
    Multimedia(FILE* fp);
    Multimedia& operator=(Multimedia&&);
    ~Multimedia();

    bool is_good() const;

    bool is_audio() const;
    int channels() const;
    int audio_rate() const;
    int decode_audio(void* buf, int buflen); // returns number of bytes written to buffer

    bool is_video() const;
    float fps() const;
    int w() const;
    int h() const;
    void draw_video_frame(SDL_Overlay& ovl);

private:
    void get_page();
    void init_theora();
    void init_vorbis();
    ogg_packet get_packet(enum media_type is_video);
};

enum stream_type {
    MEDIA_AUDIO = 1,

    MEDIA_VIDEO = 2
};

typedef struct {
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

extern int mm_open(mm_file *mf, const char *fname);
extern int mm_open_fp(mm_file *mf, FILE *file);
extern unsigned mm_ignore(mm_file *mf, unsigned mask);
extern int mm_close(mm_file *mf);
extern int mm_video_info(const mm_file *mf, unsigned *width, unsigned *height, float *fps);
extern int mm_audio_info(const mm_file *mf, unsigned *channels, unsigned *rate);
extern int mm_decode_video(mm_file *mf, SDL_Overlay *ovl);
extern int mm_decode_audio(mm_file *mf, void *buf, int buflen);
#if 0
extern int mm_convert_audio(mm_file *mf, void *buf, int buflen, SDL_AudioSpec *spec);
#endif

#endif /* _MM_FILE_H */
