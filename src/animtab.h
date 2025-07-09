/* animtab.h - Animation tables for WiNTown
 * Based on original WiNTown code from WiNTownLegacy project
 */

#ifndef _ANIMTAB_H
#define _ANIMTAB_H

/* Animation table - defines the next frame for each animated tile */
extern short aniTile[1024];

/* Using sim.h definitions directly - don't redefine */

/* Industrial smoke animations - calculated from industrial base values */
#define INDSMOKE1    TELEBASE
#define INDSMOKE2    (TELEBASE + 4)
#define INDSMOKE3    (TELEBASE + 8)
#define INDSMOKE4    (TELEBASE + 12)
#define INDSMOKE5    (TELEBASE + 16)
#define INDSMOKE6    (TELEBASE + 20)
#define INDSMOKE7    (TELEBASE + 24)
#define INDSMOKE8    (TELEBASE + 28)

/* Special animations - using sim.h values directly */
#define NUCLEAR_SWIRL 952  /* Nuclear power plant swirl animation - no sim.h equivalent */

/* Smoke stack position offsets */
extern short smokeOffsetX[4];
extern short smokeOffsetY[4];

/* Industrial smoke position data */
extern short indOffsetX[8];
extern short indOffsetY[8];

/* Industrial building types that can have smoke */
extern short indSmokeTable[8];

#endif /* _ANIMTAB_H */