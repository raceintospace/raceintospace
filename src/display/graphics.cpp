#include "graphics.h"

#include <SDL.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <assert.h>
#include <memory.h>

// FIXME: let's try to avoid this
#include "../game/sdlhelper.h"

namespace display
{

Graphics graphics;

Graphics::Graphics():
    _screen(NULL),
    _scaledScreen(NULL),
    _display(NULL),
    _video(NULL),
    _news(NULL),
    _fullscreen(false)
{
    memset(_palette, 0, sizeof(_palette));
}

Graphics::~Graphics()
{
    destroy();
}

void Graphics::create(const std::string &title, bool fullscreen)
{

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        throw std::runtime_error(SDL_GetError());
    }


    //TODO: Move this to a separate audio object
    if (!SDL_getenv("SDL_AUDIODRIVER")) {
//        INFO1("fixing WIN32 audio driver setup");
        SDL_putenv("SDL_AUDIODRIVER=waveout");
    }

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    //END:TODO




    _display = SDL_SetVideoMode(WIDTH * SCALE, HEIGHT * SCALE, 24, 0);

    if (!_display) {
        throw std::runtime_error(SDL_GetError());
    }

    _screen = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 8, 0, 0, 0, 0);

    if (!_screen) {
        throw std::runtime_error(SDL_GetError());
    }

    _scaledScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH * SCALE, HEIGHT * SCALE, 8, 0, 0, 0, 0);

    if (!_scaledScreen) {
        throw std::runtime_error(SDL_GetError());
    }

    _video = SDL_CreateYUVOverlay(VIDEO_WIDTH, VIDEO_HEIGHT, SDL_YV12_OVERLAY, _display);

    if (!_video) {
        throw std::runtime_error(SDL_GetError());
    }

    _news = SDL_CreateYUVOverlay(NEWS_WIDTH, NEWS_HEIGHT, SDL_YV12_OVERLAY, _display);

    if (!_news) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_WM_SetCaption(title.c_str(), NULL);

}

void Graphics::destroy()
{
    SDL_FreeYUVOverlay(_video);
    SDL_FreeYUVOverlay(_news);

    SDL_FreeSurface(_scaledScreen);
    SDL_FreeSurface(_screen);
    SDL_FreeSurface(_display);

    _video = NULL;
    _news = NULL;
    _scaledScreen = NULL;
    _screen = NULL;
    _display = NULL;
}

void Graphics::clearScreen(int colour)
{
    SDL_FillRect(_screen, NULL, colour);
}

void Graphics::fillRect(int x1, int y1, int x2, int y2, char color)
{
    SDL_Rect r;
    int left = std::min(x1, x2);
    int right = std::max(x1, x2);
    int top = std::min(y1, y2);
    int bottom = std::max(y1, y2);

    r.x = left;
    r.y = top;

    // Original used inclusive coordinates.  The +1 was in the original.
    r.w = (right - left) + 1;
    r.h = (bottom - top) + 1;
    fillRect(r, color);
}

void Graphics::fillRect(const SDL_Rect &area, char color)
{
    SDL_FillRect(_screen, const_cast<SDL_Rect *>(&area), color);
    av_need_update(const_cast<SDL_Rect *>(&area));
}

void Graphics::setPixel(int x, int y, char color)
{
    *((char *)(_screen->pixels) + (y * _screen->pitch) + x) = color;
}

void Graphics::setForegroundColor( char color )
{
	_foregroundColor = color;
}

void Graphics::setBackgroundColor( char color )
{
	_backgroundColor = color;
}

char Graphics::getPixel(int x, int y)
{
    assert(x >= 0 && x < WIDTH);
    assert(y >= 0 && y < HEIGHT);

    return display::graphics.screen()[(y * _screen->pitch) + x];
}

void Graphics::outlineRect(int x1, int y1, int x2, int y2, char color)
{
	line( x1, y1, x2, y1, color );
	line( x2, y1, x2, y2, color );
	line( x2, y2, x1, y2, color );
	line( x1, y2, x1, y1, color );
}

#define swap(a,b) (t = a, a = b, b = t)
void Graphics::line(int x1, int y1, int x2, int y2, char color)
{
    int deltax, deltay;
    int error;
    int ystep;
    int x, y;
    int steep;
    int t;

    steep = abs(y2 - y1) > abs(x2 - x1);

    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    deltax = x2 - x1;
    deltay = abs(y2 - y1);
    error = 0;

    y = y1;

    if (y1 < y2) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = x1; x <= x2; x++) {
        if (steep) {
            display::graphics.setPixel(y, x, color);
        } else {
            display::graphics.setPixel(x, y, color);
        }

        error = error + deltay;

        if (2 * error >= deltax) {
            y = y + ystep;
            error = error - deltax;
        }
    }
}

void Graphics::setPalette( uint8_t *palette )
{
    memcpy(_palette, palette, 256 * 3);
}


/*
void Display::present() {
    doScale();
    SDL_BlitSurface( _scaledScreen, NULL, _display, NULL );
    SDL_UpdateRect( _display, 0, 0, 0, 0 );
}

void Display::doScale() {

    for (int x = 0; x < WIDTH; x++ ) {
        for (int y = 0; y < HEIGHT; y++) {

            uint8_t * from = (uint8_t *)_screen->pixels + ((y * WIDTH) + x);
            uint8_t * to = (uint8_t *)_scaledScreen->pixels + ((y * 2) * (WIDTH * 2) + (x * 2));

            *to = *from;
            *(to + 1) = *from;
            *(to + (WIDTH * 2)) = *from;
            *(to + (WIDTH * 2)+1) = *from;
        }
    }
}
*/

};
