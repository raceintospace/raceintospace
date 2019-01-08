#ifndef DISPLAY_SURFACE_H
#define DISPLAY_SURFACE_H

#include <SDL/SDL.h>
#include <boost/shared_ptr.hpp>

#include "palette.h"

namespace display
{

class Surface
{
public:
    Surface(unsigned int width, unsigned int height);
    virtual ~Surface();

    SDL_Surface *surface() const;

    inline unsigned int width() const
    {
        return _screen->w;
    }

    inline unsigned int height() const
    {
        return _screen->h;
    }

    // Set the surface to a solid color
    void clear(const Color &color);

    // Set the surface to black
    inline void clear()
    {
        clear(Color(0, 0, 0));
    };

    void draw(const Surface &surface, unsigned int x, unsigned int y)
    {
        draw(surface, 0, 0, surface.width(), surface.height(), x, y);
    }

    void draw(const Surface &surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH)
    {
        draw(surface, srcX, srcY, srcW, srcH, 0, 0);
    }

    void draw(const Surface &surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y);


    void draw(const boost::shared_ptr<Surface> surface, unsigned int x, unsigned int y)
    {
        draw(*surface, x, y);
    }


    void draw(const boost::shared_ptr<Surface> surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH)
    {
        draw(*surface, srcX, srcY, srcW, srcH, 0, 0);
    }

    void draw(const boost::shared_ptr<Surface> surface, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y)
    {
        draw(*surface, srcX, srcY, srcW, srcH, x, y);
    }


protected:
    Surface(SDL_Surface *surface);

    // Return a representation of this Color appropriate for this surface
    uint32_t mapColor(const Color &color);

    SDL_Surface *_screen;
    bool _dirty;
};

} // namespace display

#endif // DISPLAY_SURFACE_H