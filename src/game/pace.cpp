// This file is something to do with processing audio files

#include "pace.h"

#include <cassert>
#include <cctype>

#include "display/graphics.h"
#include "display/surface.h"

#include "Buzz_inc.h"
#include "game_main.h"
#include "gr.h"
#include "mmfile.h"
#include "sdlhelper.h"
#include "utils.h"


void randomize();
double get_time();


LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)


char DoModem(int sel)
{
    LOG_NOTICE("DoModem not implemented");
    return 0;
}

char MPrefs(char mode)
{
    return 0;
}

int put_serial(unsigned char n)
{
    return 0;
}

void MesCenter() {}

char* letter_data;

void OpenEmUp()
{
    randomize();
    letter_data = load_gamedata("letter.json");
    
    if (letter_data == nullptr || letter_data[0] == '\0') {
        throw std::runtime_error("Error: load letter has failed.");
    }
}

int PCX_D(const char* src_raw, char* dest_raw, unsigned src_size)
{
    const char* src = src_raw;
    char* dest = dest_raw;
    char* orig_dest = dest;

    for(;src_size > 0; --src_size) {
        if ((*src & 0xc0) == 0xc0) {
            int num = *src & 0x3f;
            src++;
            src_size--;

            for (; num > 0; --num) {
                *dest = *src;
                dest++;
            }

            src++;
        } else {
            *dest = *src;
            dest++; src++;
        }
    }

    return dest - orig_dest;
}

/* Run-length Encoding Decompression algorithm.
 *
 * Expands a data sequence compressed using the RLEC() function.
 *
 * \param src_raw   Compressed input buffer.
 * \param dest_raw  Buffer for decompressed ouput.
 * \param src_size  Length of the compressed file, in bytes.
 * \return  Size of the decompressed data in bytes.
 */
int RLED(const char* src_raw, char* dest_raw, unsigned int src_size)
{
    const signed char* src = (const signed char*)src_raw;
    signed char* dest = (signed char*)dest_raw;
    unsigned int used = 0;

    while (used < src_size) {
        int count = src[used];
        used++;

        if (count < 0) {
            int val = src[used];
            used++;

            for (int i = 0; i < (-count+1); i++) {
                *dest = val;
                dest++;
            }
        } else {
            for (int i = 0; i < count+1; i++) {
                *dest = src[used];
                dest++; used++;
            }
        }
    }

    return ((char*)dest - (char*)dest_raw);
}

int RLED_img(const char* src_raw, char* dest_raw, unsigned int src_size,
             int w, int h)
{
    const signed char* src = (const signed char*)src_raw;
    signed char buf[128 * 1024];
    signed char* dest = buf;

    unsigned int used = 0;
    while (used < src_size) {
        int count = src[used];
        used++;

        if (count < 0) {
            int val = src[used];
            used++;
            
            memset(dest, val, (-count+1));
            dest += (-count+1);
        } else {
            memcpy(dest, &src[used], count+1);
            used += count+1;
            dest += count+1;
        }
    }

    int total = dest - buf;

    if (total < w * h + h) {
        memcpy(dest_raw, buf, w * h);
        return w * h;
    }

    dest = (signed char*)dest_raw;

    for (int row = 0; row < h; row++) {
        memcpy(dest, &buf[row * (w + 1)], w);
        dest += w;
    }

    return w * h;
}

/**
 * Original sources say:
 * @param wh = 0 - fade colors from 0 to val,
 *           = 1 - fade colors from val to 3*256,
 *           = 2 - fade all colors
 * @param palx pointer to palette
 * @param steps
 * @param val pivot index in palette array
 * @param mode if mode == 1 then preserve non-faded colors, else make black
 */
void FadeIn(char wh, int steps, int val, char mode)
{
    int from = 0;
    int to = 256;

    if (wh == 0) {
        to = val;
    } else if (wh == 1) {
        from = val;
    } else {
        assert(wh == 2);
    }

    av_set_fading(AV_FADE_IN, from, to, steps, !!mode);
}

void FadeOut(char wh, int steps, int val, char mode)
{
    int from = 0;
    int to = 256;

    if (wh == 0) {
        to = val;
    } else if (wh == 1) {
        from = val;
    } else {
        assert(wh == 2);
    }

    av_set_fading(AV_FADE_OUT, from, to, steps, !!mode);
}

void delay(int millisecs)
{
    idle_loop_secs(millisecs / 1000.0);
}

void bzdelay(int ticks)
{
    idle_loop_secs(ticks / 100.0);
}

int brandom(int limit)
{
    if (limit == 0) {
        return 0;
    }

    return (int)(limit * (rand() / (RAND_MAX + 1.0)));
}

void StopAudio(char mode)
{
    av_silence(AV_SOUND_CHANNEL);
}

void CloseEmUp(unsigned char error, unsigned int value)
{
    /* DEBUG */ /* fprintf (stderr, "CloseEmUp()\n"); */
    exit(EXIT_SUCCESS);
}

void randomize()
{
    srand(get_time() * 1000);
}

/** do nothing for a few seconds.
 *
 * The function will wait a number of seconds but will call av_block() in the meantime.
 *
 * \param secs Number of seconds to wait.
 */
void idle_loop_secs(double secs)
{
    gr_sync();

    double start = get_time();

    while (1) {
        av_block();

        if (get_time() - start >= secs) {
            break;
        }
    }
}

/** wait a number of ticks
 *
 * \param ticks Number of ticks to wait.
 */
void idle_loop(int ticks)
{
    idle_loop_secs(ticks / 2000.0);
}

char* soundbuf = nullptr;
size_t soundbuf_size = 0;
audio_chunk news_chunk;

/* Loads an audio file.
 *
 * \param name  Filename to be loaded.
 * \param data  Buffer storing the uncompressed audio data.
 * \param size  Size of the data buffer.
 * \param music Flag indicating whether the file is a music file.
 *
 * \return Number of bytes written to the data buffer.
 */
ssize_t load_audio_file(const char* name, char** data, size_t* size, bool music)
{
    const size_t def_size = 1024 * (music ? 192 : 32768); // increase from 16b for hq audio files
    double start = get_time();

    /* make compiler happy */
    start *= 1.0;

    assert(name);
    assert(data);
    assert(size);

    Multimedia mf{sOpen(name, "rb", FT_AUDIO)};
    if (!mf.is_good()) {
        return -1;
    }

    if (!mf.is_audio()) {
        CAT_WARNING(audio, "no audio data in file `%s'", name);
        return -1;
    }

    if (mf.channels() != 2 || mf.rate() != 44100) {
        CAT_ERROR(audio, "file `%s' should be stereo, 44100Hz", name);
        return -1;
    }

    if (*data == nullptr) {
        *data = (char*)xmalloc(*size = def_size);
    }

    size_t offset = 0;
    while (true) {
        ssize_t read = mf.decode_audio(*data + offset, *size - offset);
        if (read <= 0) break;
        
        offset += read;

        /* Do not use a dynamically growing buffer for non-music
           audio. See the discussion at
           <https://github.com/raceintospace/raceintospace/issues/920>. */
        if(!music) {
            assert(def_size > offset);
        }
        else {
            if (*size <= offset) {
                *data = (char*)xrealloc(*data, *size *= 2);
            }
        }
    }

    CAT_DEBUG(audio, "loading file `%s' took %5.4f seconds",
                                  name, get_time() - start);

    return offset;
}

void NGetVoice(char plr, char val)
{
    char fname[100];

    snprintf(fname, sizeof(fname), "%s_%03d.ogg",(plr ? "sov" : "usa"), val);
    ssize_t bytes = load_audio_file(fname, &soundbuf, &soundbuf_size, false);
    if (bytes > 0) PlayVoice(bytes);
}

void PlayVoice(size_t soundbuf_used)
{
    if (soundbuf_used == 0) {
        return;
    }

    news_chunk.data = soundbuf;
    news_chunk.size = soundbuf_used;
    news_chunk.next = nullptr;
    play(&news_chunk, AV_SOUND_CHANNEL);
}

void stop_voice()
{
    av_silence(AV_SOUND_CHANNEL);
}

int getch()
{
    while (1) {
        av_block();

        int c;
        if ((c = bioskey(0)) != 0) {
            return c;
        }
    }
}

void play_audio(std::string str, int mode)
{
    str += ".ogg";

    CAT_INFO(audio, "playing sound file `%s'", str.c_str());
    ssize_t size = load_audio_file(str.c_str(), &soundbuf, &soundbuf_size, false);
    if (size > 0) PlayVoice(size);
}
