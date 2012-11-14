#ifndef DISPLAY__GRAPHICS_H
#define DISPLAY__GRAPHICS_H

#include <SDL.h>
#include <string>
#include <vector>

namespace display
{

class Graphics
{
public:
    Graphics();
    ~Graphics();

    static const int WIDTH = 320;
    static const int HEIGHT = 200;
    static const int SCALE = 2;
    static const int VIDEO_WIDTH = 160;
    static const int VIDEO_HEIGHT = 100;
    static const int NEWS_WIDTH = 312;
    static const int NEWS_HEIGHT = 106;

    void create(const std::string &title, bool fullscreen);
    void destroy();

    SDL_Surface *screenSurface() const {
        return _screen;
    }

    uint8_t *palette() const {
        return (uint8_t *)(&_palette);
    };

    SDL_Surface *scaledScreenSurface() const {
        return _scaledScreen;
    }

    SDL_Surface *displaySurface() const {
        return _display;
    }

    SDL_Overlay *videoOverlay() const {
        return _video;
    }

    SDL_Overlay *newsOverlay() const {
        return _news;
    }

    char *screen() const {
        return (char *)(_screen->pixels);
    }

    SDL_Rect &videoRect() {
        return _videoRect;
    }

    SDL_Rect &newsRect() {
        return _newsRect;
    }

	char foregroundColor() const {
		return _foregroundColor;
	}

	char backgroundColor() const {
		return _backgroundColor;
	}

    void clearScreen(int colour);
    void fillRect(int x1, int y1, int x2, int y2, char color);
    void fillRect(const SDL_Rect &area, char color);
    void setPixel(int x, int y, char color);
	void setForegroundColor( char color );
	void setBackgroundColor( char color );
	char getPixel( int x, int y );
	void outlineRect(int x1, int y1, int x2, int y2, char color );
	void line( int x1, int y1, int x2, int y2, char color );
	void setPalette( uint8_t *palette );


private:
    SDL_Surface *_screen;
    SDL_Surface *_scaledScreen;
    SDL_Surface *_display;
    SDL_Overlay *_video;
    SDL_Overlay *_news;
    SDL_Rect _videoRect;
    SDL_Rect _newsRect;
    bool _fullscreen;
    uint8_t _palette[3 * 256];
	char _foregroundColor;
	char _backgroundColor;
};

extern Graphics graphics;

};

#endif // DISPLAY__GRAPHICS_H