/**
 * Copyright (C) 2005 Michael K. McCarty & Fritz Bronner,
 * Copyright (C) 2020 Ryan Yoakum
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "fireworks.h"

#include <cmath>

#include "display/graphics.h"
#include "display/legacy_surface.h"

#include "pace.h"


static const double GRAVITY = 0.6;
static const double FRICTION = 0.3;
static const double PI = 3.145926;


Fireworks::Fireworks(int player)
    : mMaxBombLife(20), mMaxInitSpeed(270), mMinInitSpeed(70),
      mParticles(100), mBombAge(mMaxBombLife + 1), mPlayer(player)
{
    unsigned int bgWidth = display::graphics.legacyScreen()->width();
    unsigned int bgHeight = display::graphics.legacyScreen()->height();
    mBackground = new display::LegacySurface(bgWidth, bgHeight);
    mBackground->copyFrom(display::graphics.legacyScreen(),
                          0, 0, bgWidth - 1, bgHeight - 1);
    mBomb = new Burst[mParticles];
}


Fireworks::~Fireworks()
{
    if (mBackground) {
        delete mBackground;
    }

    if (mBomb) {
        delete[] mBomb;
    }
}


/**
 * Advance the animation according to time passed.
 *
 * TODO: Don't bother updating animation unless a minimum time
 *   interval has passed.
 * TODO: So that animations remain accurate, position/velocity must
 *   be updated every so many time intervals - unless an algorithm
 *   can be used instead.
 * TODO: If enough time has passed (1+ bomb life spans) just reinit
 *   the bomb and advance the appropriate amount of time.
 */
void Fireworks::advance()
{
    step();
}


/**
 * Erase the animation.
 *
 * Overwrites the firework animation with the buffered background.
 */
void Fireworks::clear()
{
    clearDisplay();
}

/**
 * Advance the animation by a single frame.
 */
void Fireworks::step()
{
    bool bombIsDead = mBombAge > mMaxBombLife;

    clearDisplay();

    if (bombIsDead) {
        initBomb();
    } else {
        bombStep();
    }

    drawFrame();
}


//----------------------------------------------------------------------
// Private Method Declarations
//----------------------------------------------------------------------


void Fireworks::bombStep()
{
    for (unsigned int i = 0; i < mParticles; i++) {

        // Don't bother updating dead particles.
        if (mBomb[i].life < mBombAge) {
            continue;
        }

        mBomb[i].velocity[1] += GRAVITY;
        mBomb[i].velocity[0] *= FRICTION;
        mBomb[i].velocity[1] *= FRICTION;

        mBomb[i].position[0] += mBomb[i].velocity[0];
        mBomb[i].position[1] += mBomb[i].velocity[1];
    }

    mBombAge++;
}


void Fireworks::clearDisplay()
{
    if (!mBackground) {
        return;
    }

    // Could optimize by skipping over pixels that _were_ dead.
    for (unsigned int i = 0; i < mParticles; i++) {
        int xx = mBomb[i].position[0];
        int yy = mBomb[i].position[1];

        if (inView(xx, yy)) {
            display::graphics.legacyScreen()->setPixel(
                xx, yy, mBackground->getPixel(xx, yy));
        }
    }
}


uint8_t Fireworks::cycleColor(uint8_t color)
{
    // If color isn't part of the cycle, reset.
    uint8_t nextColor = 1;

    // If color isn't part of the cycle, should an error be logged?
    if (mPlayer == 0) {
        if (color == 1) {
            nextColor = 6;
        } else if (color == 6) {
            nextColor = 9;
        } else if (color == 9) {
            nextColor = 1;
        }
    } else {
        if (color == 1) {
            nextColor = 9;
        } else if (color == 9) {
            nextColor = 11;
        } else if (color == 11) {
            nextColor = 9;
        }
    }

    return nextColor;
}


/**
 * Draw a single frame of the fireworks' explosion.
 */
void Fireworks::drawFrame()
{
    for (unsigned int i = 0; i < mParticles; i++) {
        int xx = mBomb[i].position[0];
        int yy = mBomb[i].position[1];

        if (mBomb[i].life >= mBombAge && inView(xx, yy)) {
            display::graphics.legacyScreen()->setPixel(
                xx, yy, char(mBomb[i].color));
        }
    }
}


/**
 * Reinitialize the firework.
 */
void Fireworks::initBomb()
{
    uint8_t color = 1;
    int initSpeed, startx, starty;
    int region = brandom(100);

    if (region < 60) {
        startx = 132 + brandom(187);
        starty = 5 + brandom(39);
    } else {
        startx = 178 + brandom(66);
        starty = 11 + brandom(33);
    }

    do {
        initSpeed = brandom(mMaxInitSpeed);
    } while (initSpeed < mMinInitSpeed);

    for (unsigned int i = 0; i < mParticles; i++) {
        double angle = randomAngle();
        double speed = double(brandom(initSpeed));
        color = cycleColor(color);

        mBomb[i].position[0] = double(startx);
        mBomb[i].position[1] = double(starty);
        mBomb[i].velocity[0] = speed * std::cos(angle);
        mBomb[i].velocity[1] = speed * std::sin(angle);
        mBomb[i].color = color;
        mBomb[i].life = brandom(mMaxBombLife);
    }

    mBombAge = 0;
}


/**
 * Return a random polar coordinate angle.
 *
 * Due to limitations of the angle calculations, all firework
 * particles are forced along one of six paths. This creates
 * a coherent firework effect; a uniform distribution, combined
 * with random velocities for each particle, yields an indistinct
 * scattering of particles within a radius.
 */
double Fireworks::randomAngle()
{
    // If using rand(), #include <cstdlib>
    // return 2.0 * PI * (double(rand()) / double(RAND_MAX + 1.0));
    return double(brandom(2 * PI));
}
