#ifndef BZ_ANIMATION_H
#define BZ_ANIMATION_H

#include <stdint.h>

#include <boost/shared_ptr.hpp>

#include "display/legacy_surface.h"

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
    int32_t aLoc;
    display::LegacySurface *dply;
    FILE *fin;
    struct AnimType AHead;
    struct BlockHead BHead;

    BZAnimation(int x, int y);

    void openAnim(const char *fname);
    int stepAnim(int x, int y);
};



#endif // BZ_ANIMATION_H
