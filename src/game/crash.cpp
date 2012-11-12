#include <string>
#include <vector>

#include "display/png_image.h"

#include "Buzz_inc.h"
#include "game_main.h"
#include "gr.h"
#include "sdlhelper.h"
#include "pace.h"

enum vertical_alignment {
    ALIGN_TOP,
    ALIGN_BOTTOM
};

void display_text_in_box(int x, int y, int width, int height, const std::string &text, vertical_alignment valign = ALIGN_TOP)
{
    int characters_per_line = width / 7;
    std::string remaining_text(text);

    // split text into lines
    std::vector<std::string> lines;

    while (remaining_text.size() > 0) {
        if (characters_per_line >= remaining_text.size()) {
            // this is the last line
            // add it in its entirety
            lines.push_back(remaining_text);
            remaining_text.clear();

        } else {
            int split_point = -1;

            // walk forwards to find a newline
            for (int i = 0; i < remaining_text.size(); i++) {
                if (remaining_text[i] == '\n') {
                    // split here
                    split_point = i;
                    break;
                }
            }

            // walk backwards to find a split point
            for (int i = characters_per_line - 1; i >= 0; i++) {
                if (split_point == -1 && remaining_text[i] == ' ') {
                    split_point = i;
                    break;
                }
            }

            if (split_point < characters_per_line / 2) {
                // there's no good location to split this string
                // use the whole line
                split_point = characters_per_line - 1;
            }

            lines.push_back(remaining_text.substr(0, split_point));
            remaining_text = remaining_text.substr(split_point + 1);
        }
    }

    if (valign == ALIGN_TOP) {
        // display all the lines that fit in the box
        for (int i = 0; i * 9 < height && i < lines.size(); i++) {
            PrintAt(x, y + i * 9, lines[i].c_str());
        }

    } else if (valign == ALIGN_BOTTOM) {
        for (int i = lines.size() - 1; i * 9 < height && i >= 0; i--) {
            PrintAt(x, y + height - i * 9 - 9, lines[i].c_str());
        }
    }
}

void pretty_crash(const std::string &title, const std::string &message)
{
    FILE *fp = sOpen("error.png", "rb", FT_IMAGE);
    display::PNGImage image(fp);
    fclose(fp);

    image.export_to_legacy_palette();
    image.draw();

    grSetColor(8);
    display_text_in_box(190, 20, 120, 20, "MASTER ALARM!");

    grSetColor(1);
    display_text_in_box(190, 30, 120, 20, title, ALIGN_BOTTOM);

    grSetColor(3);
    display_text_in_box(190, 50, 120, 108, message);

    av_need_update_xy(0, 0, MAX_X, MAX_Y);

    // wait for a key
    int key = -1;

    while (!(key == K_ENTER || key == K_ESCAPE || key == K_SPACE)) {
        av_block();
        gr_maybe_sync();
        key = bioskey(0);
    }

    // exit with an error code
    exit(1);
}

#ifdef PLATFORM_PROVIDES_UGLY_CRASH
extern "C"
void ugly_crash(const char *title, const char *message);
#else
void ugly_crash(const std::string &title, const std::string &message)
{
    fprintf(stderr, "\nCRASH: %s\n\n%s\n", title.c_str(), message.c_str());
    fflush(stderr);
    abort();
}
#endif

// Display a fatal error message and terminate when it's dismissed
void crash(const std::string &title, const std::string &message)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) && display::PNGImage::libpng_versions_match()) {
        pretty_crash(title, message);
    } else {
        ugly_crash(title.c_str(), message.c_str());
    }
}
