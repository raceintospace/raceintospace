#ifndef BZ_ANIMATION_H
#define BZ_ANIMATION_H

#include <stdint.h>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "display/legacy_surface.h"
#include "display/palette.h"

#include "data.h"


/**
 *
 *
 *
 */
class BZAnimation {
public:
    typedef boost::shared_ptr<BZAnimation> Ptr;

    static Ptr load(const char *file, const char *id, int x, int y);

    ~BZAnimation();

    void advance();

private:
    int mX, mY;
    int tFrames, cFrame;
    display::LegacySurface *dply;
    struct AnimType AHead;
    std::vector<uint8_t *> mFrameData;

    BZAnimation(struct AnimType header, display::Palette palette,
                std::vector<uint8_t *> frames, int x, int y);

};



#endif // BZ_ANIMATION_H
