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
    char *pixels() const;
    void clear(char colour);
    void fillRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void fillRect(const SDL_Rect &area, char color);
    void setPixel(unsigned int x, unsigned int y, char color);
    char getPixel(unsigned int x, unsigned int y);
    void outlineRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char color);
    void copyFrom(Surface *surface, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
    void copyFrom(Surface *surface, unsigned int srcX1, unsigned int srcY1, unsigned int srcX2, unsigned int srcY2, unsigned int dstX, unsigned int dstY);
    void copyTo(Surface *surface, unsigned int x, unsigned int y, Surface::Operation operation = Surface::Set);
    void copyTo(Surface *surface, unsigned int srcX, unsigned int srcY, unsigned int destX1, unsigned int destY1, unsigned int destX2, unsigned int destY2);
    void scaleTo(Surface *surface);
    void maskCopy(Surface *source, char maskValue, Surface::MaskSource maskSource, char offset = 0);
    void filter(char testValue, char offset, Surface::FilterTest filterTest);

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

    uint8_t *_screenBuffer;         // XXX: do not access for any reason unless you're the constructor
    SDL_Surface *_screen;
    bool _dirty;
};

} // namespace display

#endif // DISPLAY_SURFACE_H