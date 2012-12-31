#ifndef DISPLAY_SURFACE_H
#define DISPLAY_SURFACE_H

#include <SDL.h>
#include <boost/shared_ptr.hpp>

namespace display
{

class Image;

class Surface
{
public:
    Surface(unsigned int width, unsigned int height);
    ~Surface();

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
    void draw(Image *image, unsigned int x, unsigned int y);
    void draw(Image *image, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y);
    void maskCopy(Surface *source, char maskValue, Surface::MaskSource maskSource, char offset = 0);
    void filter(char testValue, char offset, Surface::FilterTest filterTest);

    unsigned int width() const {
        return _width;
    }

    unsigned int height() const {
        return _height;
    }

    void draw(boost::shared_ptr<Image> image, unsigned int x, unsigned int y) {
		draw(image.get(), x, y);
	}

    void draw(boost::shared_ptr<Image> image, unsigned int srcX, unsigned int srcY, unsigned int srcW, unsigned int srcH, unsigned int x, unsigned int y) {
		draw(image.get(), srcX, srcY, srcW, srcH, x, y);
	}


private:
    unsigned int _width;
    unsigned int _height;
    SDL_Surface *_screen;
    bool _dirty;
};

} // namespace display

#endif // DISPLAY_SURFACE_H