#include "graphics.h"
#include "surface.h"
#include "legacy_surface.h"

#include <SDL/SDL.h>
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
    _news(NULL)
{
}

Graphics::~Graphics()
{
    destroy();
}

LegacySurface *Graphics::legacyScreen()
{
    return _screen;
}

void Graphics::create(const std::string &title, bool fullscreen)
{

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    //END:TODO




    _display = SDL_SetVideoMode(WIDTH * SCALE, HEIGHT * SCALE, 24, 0);

    if (!_display) {
        throw std::runtime_error(SDL_GetError());
    }

    _screen = new LegacySurface(WIDTH, HEIGHT);

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

    if (_screen) {
        delete _screen;
        _screen = NULL;
    }

    SDL_FreeSurface(_display);

    _video = NULL;
    _news = NULL;
    _scaledScreen = NULL;
    _screen = NULL;
    _display = NULL;
}

void Graphics::setForegroundColor(char color)
{
    _foregroundColor = color;
}

void Graphics::setBackgroundColor(char color)
{
    _backgroundColor = color;
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
