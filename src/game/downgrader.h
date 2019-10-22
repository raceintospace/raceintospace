#ifndef DOWNGRADER_H
#define DOWNGRADER_H

#include <vector>
#include <istream>

// Hate including this in the header file, but without it the
// MissionType structs may be the wrong size.
// Replace with using pointers? -- rnyoakum
#include "Buzz_inc.h"


/**
 * This class takes a base, or seed, mission instance and generates
 * a cyclical list of "downgraded" mission instances that share the
 * crew and hardware.
 */
class Downgrader
{
public:
    Downgrader(const struct MissionType &mission, char downgrades[62][6]);
    // Downgrader(const struct MissionType &mission, std::istream &input);
    ~Downgrader();

    struct MissionType current() const;
    struct MissionType next();
private:
    const struct MissionType mBasis;
    struct MissionType mCurrent;
    std::vector<int> mCodes;
    std::vector<int>::iterator mIndex;
    int mDuration;
};


#endif // DOWNGRADER_H
