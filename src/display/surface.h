#ifndef DISPLAY_SURFACE_H
#define DISPLAY_SURFACE_H

#include <SDL.h>
#include <boost/shared_ptr.hpp>

namespace display
{

class Surface
{
public:
    Surface(unsigned int width, unsigned int height);
    virtual ~Surface();

    enum Operation {
        Set,
        Xor
    };

    enum MaskSource {
        SourceEqual,
        DestinationEqual,
        SourceNotEqual,
        DestinationNotEqual
    };

    enum FilterTest {
        Equal,
        NotEqual,
        Any
    };

    SDL_Surface *surface() const;

    inline unsigned int width() const {
        return _screen->w;
    }

    inline unsigned int height() const {
        return _screen->h;
    }

    void draw(Surface *surface, unsigned int x, unsigned int y) {
        draw(surface, 0, 0, surface->width(), surface->height(), x, y);
    }

    void draw(Surface *surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y);


    void draw(boost::shared_ptr<Surface> surface, unsigned int x, unsigned int y) {
        draw(surface.get(), x, y);
    }

    void draw(boost::shared_ptr<Surface> surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y) {
        draw(surface.get(), srcX, srcY, srcW, srcH, x, y);
    }

protected:
    Surface(SDL_Surface *surface);

    SDL_Surface *_screen;
    bool _dirty;
};

} // namespace display

#endif // DISPLAY_SURFACE_H