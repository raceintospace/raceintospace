#ifndef FIREWORKS_H
#define FIREWORKS_H

#include <stdint.h>


namespace display {
    class LegacySurface;
}

/**
 * A simple fireworks animation class.
 *
 * Draws a fireworks display for the endgame celebrations, consisting
 * of a single firework exploding at random locations on the screen.
 * Fireworks are a "retro" design of six expanding rays.
 *
 * Possible improvements include:
 * - Create a stop/pause option for the display.
 * - Increase the time fireworks persist.
 * - Set a limited number of fireworks for the display.
 * - A staggered fireworks display w/ multiple bombs at a time.
 * - Replace "retro" bomb animations (6 rays) with configurable ray
 *   count.
 */
class Fireworks
{
public:
    Fireworks(int player);
    ~Fireworks();

    void advance();
    void clear();
    void step();
private:
    struct Burst {
        uint8_t color;
        double velocity[2];
        double position[2];
        int life;
    };

    const unsigned int mParticles;
    const int mPlayer;
    int mBombAge;
    int mMaxBombLife;
    int mMaxInitSpeed;
    int mMinInitSpeed;
    Burst *mBomb;
    display::LegacySurface *mBackground;

    void bombStep();
    void clearDisplay();
    uint8_t cycleColor(uint8_t color);
    void drawFrame();
    bool inView(int x, int y)
    {
        return x >= 0 && x < 320 && y >= 0 && y <= 172;
    };
    void initBomb();
    double randomAngle();
};


#endif // FIREWORKS_H
