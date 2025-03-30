/* animation.c - Tile animation implementation for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulation.h"
#include "animtab.h"

/* Animation state flags */
static int AnimationEnabled = 1; /* Animation enabled by default */

/* Forward declarations */
static void DoCoalSmoke(int x, int y);

/* Process animations for the entire map */
void AnimateTiles(void)
{
    unsigned short tilevalue, tileflags;
    int x, y;

    /* Skip animation if disabled */
    if (!AnimationEnabled) {
        return;
    }

    /* Scan the entire map for tiles with the ANIMBIT set */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            tilevalue = Map[y][x];

            /* Only process tiles with the animation bit set */
            if (tilevalue & ANIMBIT) {
                tileflags = tilevalue & MASKBITS;  /* Save the flags */
                tilevalue &= LOMASK;               /* Extract base tile value */

                /* Look up the next animation frame */
                tilevalue = aniTile[tilevalue];

                /* Reapply the flags */
                tilevalue |= tileflags;

                /* Update the map with the new tile */
                Map[y][x] = tilevalue;
            }
        }
    }
}

/* Enable or disable animations */
void SetAnimationEnabled(int enabled)
{
    AnimationEnabled = enabled;
}

/* Get animation enabled status */
int GetAnimationEnabled(void)
{
    return AnimationEnabled;
}

/* Add smoke animation to coal power plants */
void SetSmoke(int x, int y)
{
    int i;
    int xx, yy;

    /* Skip if the position is invalid */
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y) {
        return;
    }

    /* Check if this is a power plant */
    if ((Map[y][x] & LOMASK) != POWERPLANT) {
        return;
    }

    /* Set smoke on the appropriate tiles */
    for (i = 0; i < 4; i++) {
        xx = x + smokeOffsetX[i];
        yy = y + smokeOffsetY[i];

        /* Make sure the smoke position is valid */
        if (xx >= 0 && xx < WORLD_X && yy >= 0 && yy < WORLD_Y) {
            /* Only set the tile if it doesn't already have the animation bit set
               or if it's not already a smoke tile. This avoids resetting the
               animation sequence and makes it flow better. */
            if (!(Map[yy][xx] & ANIMBIT) ||
                (Map[yy][xx] & LOMASK) < COALSMOKE1 ||
                (Map[yy][xx] & LOMASK) > COALSMOKE4+4) {

                /* Set the appropriate smoke tile with animation */
                switch (i) {
                    case 0:
                        Map[yy][xx] = COALSMOKE1 | ANIMBIT | CONDBIT | POWERBIT | BURNBIT;
                        break;
                    case 1:
                        Map[yy][xx] = COALSMOKE2 | ANIMBIT | CONDBIT | POWERBIT | BURNBIT;
                        break;
                    case 2:
                        Map[yy][xx] = COALSMOKE3 | ANIMBIT | CONDBIT | POWERBIT | BURNBIT;
                        break;
                    case 3:
                        Map[yy][xx] = COALSMOKE4 | ANIMBIT | CONDBIT | POWERBIT | BURNBIT;
                        break;
                }
            }
        }
    }
}

/* Update fire animations */
void UpdateFire(int x, int y)
{
    /* Check bounds */
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y) {
        return;
    }

    /* Set fire tiles to animate */
    if ((Map[y][x] & LOMASK) >= FIREBASE &&
        (Map[y][x] & LOMASK) <= (FIREBASE + 7)) {
        Map[y][x] |= ANIMBIT;
    }
}

/* Update nuclear power plant animations */
void UpdateNuclearPower(int x, int y)
{
    /* Check bounds */
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y) {
        return;
    }

    /* Set the nuclear core to animate */
    if ((Map[y][x] & LOMASK) == NUCLEAR) {
        int xx, yy;

        /* Set animation bits on the nuclear plant's core */
        xx = x + 2;
        yy = y - 1;

        if (xx >= 0 && xx < WORLD_X && yy >= 0 && yy < WORLD_Y) {
            Map[yy][xx] |= ANIMBIT;
        }
    }
}

/* Update airport radar animation */
void UpdateAirportRadar(int x, int y)
{
    /* Check bounds */
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y) {
        return;
    }

    /* Check if this is an airport */
    if ((Map[y][x] & LOMASK) == AIRPORT) {
        int xx, yy;

        /* Set animation bit on the radar tower tile */
        xx = x + 1;
        yy = y - 1;

        if (xx >= 0 && xx < WORLD_X && yy >= 0 && yy < WORLD_Y) {
            /* Set the radar animation bit */
            Map[yy][xx] = 832 | ANIMBIT;
        }
    }
}

/* Update all special animations - called periodically from simulation */
void UpdateSpecialAnimations(void)
{
    int x, y;
    short tileValue;

    /* Skip if animation is disabled */
    if (!AnimationEnabled) {
        return;
    }

    /* Scan entire map for special animations */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            tileValue = Map[y][x] & LOMASK;

            /* Add smoke to power plants */
            if (tileValue == POWERPLANT && (Map[y][x] & ZONEBIT)) {
                SetSmoke(x, y);
            }

            /* Animate nuclear plants */
            if (tileValue == NUCLEAR && (Map[y][x] & ZONEBIT)) {
                UpdateNuclearPower(x, y);
            }

            /* Animate airport radar */
            if (tileValue == AIRPORT && (Map[y][x] & ZONEBIT)) {
                UpdateAirportRadar(x, y);
            }
        }
    }
}