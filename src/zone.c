/* zone.c - Zone processing for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include "sim.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/* Population table values for different zone types */
/* These are local overrides for specific use in this file */
#define LOCAL_RZB 0    /* Residential base level (different from simulation.h) */
#define LOCAL_CZB 0    /* Commercial base level (different from simulation.h) */
#define LOCAL_IZB 0    /* Industrial base level (different from simulation.h) */

/* Some definitions needed by zone.c - matches simulation.h values */
#define HOSPITALBASE 400  /* Hospital base */
#define FOOTBALLBASE 950  /* Football stadium */

/* Note: Other zone base values all come from simulation.h */

/* Zone bit flags */
#define ALLBITS 0xFFFF
#define RESBIT 0x0001
#define COMBIT 0x0002
#define INDBIT 0x0004

/* Internal variables */
static int RZPop; /* Residential zone population */
static int CZPop; /* Commercial zone population */
static int IZPop; /* Industrial zone population */
/* ComRate is declared in simulation.h as quarter size */

/* Forward declarations */
static void DoResidential(int x, int y);
static void DoCommercial(int x, int y);
static void DoIndustrial(int x, int y);
static void DoHospChur(int x, int y);
static void DoSPZ(int x, int y);
/* SetSmoke function is now external from animation.c */
static int EvalLot(int x, int y);
static void BuildHouse(int x, int y, int value);
static void ResPlop(int x, int y, int den, int value);
static void ComPlop(int x, int y, int den);
static void IndPlop(int x, int y, int den);
static int ZonePlop(int x, int y, int base);
static int GetCRVal(int x, int y);
static void DoResIn(int pop, int value);
static void DoComIn(int pop, int value);
static void DoIndIn(int pop, int value);

/* Calculate population in a residential zone - matches original Micropolis */
int calcResPop(int zone) {
    short CzDen;
    
    /* Sanity check the input */
    if (zone < RESBASE || zone > LASTZONE) {
        return 0;  /* Invalid zone tile */
    }
    
    /* Use original RZPop algorithm from s_zone.c */
    CzDen = (((zone - RZB) / 9) % 4);
    return ((CzDen * 8) + 16);
}

/* Calculate population in a commercial zone - matches original Micropolis */
int calcComPop(int zone) {
    short CzDen;
    
    /* Sanity check the input */
    if (zone < COMBASE || zone > LASTZONE) {
        return 0;  /* Invalid zone tile */
    }
    
    /* Use original CZPop algorithm from s_zone.c */
    if (zone == COMCLR) return (0);
    CzDen = (((zone - CZB) / 9) % 5) + 1;
    return (CzDen);
}

/* Calculate population in an industrial zone - matches original Micropolis */
int calcIndPop(int zone) {
    short CzDen;
    
    /* Sanity check the input */
    if (zone < INDBASE || zone > LASTZONE) {
        return 0;  /* Invalid zone tile */
    }
    
    /* Use original IZPop algorithm from s_zone.c */
    if (zone == INDCLR) return (0);
    CzDen = (((zone - IZB) / 9) % 4) + 1;
    return (CzDen);
}
static void IncROG(int amount);
static void DoResOut(int pop, int value, int x, int y);
static void DoComOut(int pop, int x, int y);
static void DoIndOut(int pop, int x, int y);
static int EvalRes(int traf);
static int EvalCom(int traf);
static int EvalInd(int traf);
static int DoFreePop(int x, int y);
static void SetZPower(int x, int y);
/* Using global calcResPop, calcComPop, calcIndPop from simulation.h */

/* Random between 0 and range-1 */
static int ZoneRandom(int range) {
    return rand() % range;
}

/* Main zone processing function - based on original Micropolis code */
void DoZone(int Xloc, int Yloc, int pos) {
    /* First check if this is a zone center */
    if (!(Map[Yloc][Xloc] & ZONEBIT)) {
        return;
    }

    /* Set global position variables for this zone */
    SMapX = Xloc;
    SMapY = Yloc;

    /* Do special processing based on zone type */
    if (pos >= RESBASE) {
        if (pos < COMBASE) {
            /* Residential zone */
            SetZPower(Xloc, Yloc);
            DoResidential(Xloc, Yloc);
            return;
        }

        if (pos < INDBASE) {
            /* Commercial zone */
            SetZPower(Xloc, Yloc);
            DoCommercial(Xloc, Yloc);
            return;
        }

        if (pos < PORTBASE) {
            /* Industrial zone */
            SetZPower(Xloc, Yloc);
            DoIndustrial(Xloc, Yloc);
            return;
        }
    }

    /* Handle special zones */
    if (pos < HOSPITALBASE || pos > FOOTBALLBASE) {
        switch (pos) {
        case POWERPLANT:
        case NUCLEAR:
        case PORT:
        case AIRPORT:
        case POLICESTATION:
        case FIRESTATION:
            SetZPower(Xloc, Yloc);
            break;
        }
    }

    if (pos >= HOSPITALBASE && pos <= FOOTBALLBASE) {
        DoHospChur(Xloc, Yloc);
        return;
    }

    /* Special zones */
    DoSPZ(Xloc, Yloc);
}

/* Process hospital/church zone */
static void DoHospChur(int x, int y) {
    short z;
    int zonePowered;

    if (!(Map[y][x] & ZONEBIT)) {
        return;
    }

    SetZPower(x, y);
    
    /* Check if zone has power */
    zonePowered = (Map[y][x] & POWERBIT) != 0;

    if (CityTime & 3) {
        return;
    }

    z = Map[y][x] & LOMASK;

    if (z == HOSPITAL) {
        /* Add hospital population to census directly */
        if (zonePowered) {
            TempResPop += 30;
        } else {
            TempResPop += 5; /* Even unpowered hospitals have some population */
        }

        /* Also increment trade zone count on some cycles */
        if (ZoneRandom(20) < 10) {
            IZPop++;
        }
        return;
    }

    if (z == CHURCH) {
        /* Add church population to census directly */
        if (zonePowered) {
            TempResPop += 10;
        } else {
            TempResPop += 2; /* Even unpowered churches have some population */
        }

        /* Also increment residential zone count on some cycles */
        if (ZoneRandom(20) < 10) {
            RZPop++;
        }
    }
}

/* Process special zone (stadiums, coal plants, etc) */
static void DoSPZ(int x, int y) {
    short z;

    if (!(Map[y][x] & ZONEBIT)) {
        return;
    }

    SetZPower(x, y);

    /* Only process every 16th time */
    if (CityTime & 15) {
        return;
    }

    z = Map[y][x] & LOMASK;

    /* Handle special case of coal power plant */
    if (z == POWERPLANT) {
        SetSmoke(x, y);
        return;
    }

    /* Handle special case of stadium */
    if (z == STADIUM) {
        int xpos;
        int ypos;

        xpos = (x - 1) + ZoneRandom(3);
        ypos = (y - 1) + ZoneRandom(3);

        /* Add stadium population to census directly */
        TempComPop += 50; /* Stadiums contribute to commercial population */

        /* Additional random chance to increase commercial zone */
        if (ZoneRandom(5) == 1) {
            CZPop += 1;
        }
        return;
    }

    /* Handle special case of nuclear power */
    if (z == NUCLEAR) {
        /* Handle nuclear power plant (no meltdowns for now) */
        if (ZoneRandom(10000) == 0) {
            /* Create meltdown (would be implemented later) */
        }
        return;
    }
}

/* Industrial pollution handler */
/* Forward declaration of the SetSmoke function from animation.c */
extern void SetSmoke(int x, int y);

/* Process industrial zone */
static void DoIndustrial(int x, int y) {
    short zone;
    short tpop;
    int pop;
    int zonePowered;

    zone = Map[y][x];
    if (!(zone & ZONEBIT)) {
        return;
    }

    SetZPower(x, y);
    
    /* Check if zone has power */
    zonePowered = (Map[y][x] & POWERBIT) != 0;

    tpop = IZPop;

    /* Get actual zone population - pass only the tile ID without flags */
    pop = calcIndPop(zone & LOMASK);
    
    /* Debug check for negative or extreme values */
    if (pop < 0 || pop > 1000) {
        char debugMsg[256];
        wsprintf(debugMsg, "WARNING: Industrial zone at (%d,%d) has unusual pop=%d from zone=%d (tile=%d)\n", 
                 x, y, pop, zone, zone & LOMASK);
        OutputDebugString(debugMsg);
    }
    
    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered industrial zone */
    }

    /* Add to total industrial population for the census */
    TempIndPop += pop;

    /* Generate traffic from industrial zones at a certain rate */
    if (pop > 0 && !(CityTime & 15)) {
        /* Industrial zones (2) try to generate traffic to residential zones */
        SMapX = x;
        SMapY = y;

        /* If traffic generation successful, update population count */
        if (MakeTraffic(2) > 0) {
            IZPop += pop;
        }
    }

    /* Process industrial zone less often (every 8th cycle) */
    if ((CityTime & 7) == 0) {
        int value;

        value = GetCRVal(x, y);

        if (value < 0) {
            DoIndOut(tpop, x, y);
            return;
        }

        value = EvalInd(1);  /* Pass 1 for traffic good */

        if (value > 0) {
            DoIndIn(tpop, value);
        } else if (value < 0) {
            DoIndOut(tpop, x, y);
        }
    }

    if ((CityTime & 7) == 0) {
        IZPop = 0;
    }
}

/* Process commercial zone */
static void DoCommercial(int x, int y) {
    short zone;
    short tpop;
    int pop;
    int zonePowered;

    zone = Map[y][x];
    if (!(zone & ZONEBIT)) {
        return;
    }

    SetZPower(x, y);
    
    /* Check if zone has power */
    zonePowered = (Map[y][x] & POWERBIT) != 0;

    tpop = CZPop;

    /* Get actual zone population - pass only the tile ID without flags */
    pop = calcComPop(zone & LOMASK);
    
    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered commercial zone */
    }

    /* Add to total commercial population for the census */
    TempComPop += pop;

    /* Generate traffic from commercial zones at a certain rate */
    if (pop > 0 && !(CityTime & 15)) {
        /* Commercial zones (1) try to generate traffic to industrial zones */
        SMapX = x;
        SMapY = y;

        /* If traffic generation successful, update population count */
        if (MakeTraffic(1) > 0) {
            CZPop += pop;
        }
    }

    /* Process commercial zone less often (every 8th cycle) */
    if ((CityTime & 7) == 0) {
        int value;

        value = GetCRVal(x, y);

        if (value < 0) {
            DoComOut(tpop, x, y);
            return;
        }

        value = EvalCom(1);  /* Pass 1 for traffic good */

        if (value > 0) {
            DoComIn(tpop, value);
        } else if (value < 0) {
            DoComOut(tpop, x, y);
        }
    }

    if ((CityTime & 7) == 0) {
        CZPop = 0;
    }
}

/* Process residential zone */
static void DoResidential(int x, int y) {
    short zone;
    short tpop;
    int pop;
    int zonePowered;

    zone = Map[y][x];
    if (!(zone & ZONEBIT)) {
        return;
    }

    /* Check if zone has power */
    zonePowered = (Map[y][x] & POWERBIT) != 0;

    tpop = RZPop;

    /* Get actual zone population - pass only the tile ID without flags */
    pop = calcResPop(zone & LOMASK);

    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered residential zone */
    }

    /* Add to total residential population for the census */
    TempResPop += pop;

    /* Generate traffic from residential zones at a certain rate */
    if (pop > 0 && !(CityTime & 15)) {
        /* Residential zones (0) try to generate traffic to commercial or industrial zones */
        SMapX = x;
        SMapY = y;

        /* If traffic generation successful, update population count */
        if (MakeTraffic(0) > 0) {
            RZPop += pop;
        }
    }

    /* Process growth or decline based on power status */
    if ((CityTime & 7) == 0) {
        int value;

        value = GetCRVal(x, y);

        if (value < 0) {
            DoResOut(tpop, value, x, y);
            return;
        }

        /* No growth in unpowered zones */
        if (!zonePowered) {
            DoResOut(tpop, -500, x, y);
            return;
        }

        value = EvalRes(1);  /* Pass 1 for traffic good */

        if (value > 0) {
            DoResIn(tpop, value);
        } else if (value < 0) {
            DoResOut(tpop, value, x, y);
        }
    }

    /* Reset population counter periodically */
    if ((CityTime & 7) == 0) {
        RZPop = 0;
    }
}

/* Calculate land value for a location - matches original Micropolis */
static int GetCRVal(int x, int y) {
    register short LVal;
    
    LVal = LandValueMem[SMapY >>1][SMapX >>1];
    LVal -= PollutionMem[SMapY >>1][SMapX >>1];
    if (LVal < 30) return (0);
    if (LVal < 80) return (1);
    if (LVal < 150) return (2);
    return (3);
}

/* Handle residential zone growth - matches original Micropolis */
static void DoResIn(int pop, int value) {
    short z;
    
    z = PollutionMem[SMapY >>1][SMapX >>1];
    if (z > 128) return;
    
    if (Map[SMapY][SMapX] == FREEZ) {
        if (pop < 8) {
            BuildHouse(SMapX, SMapY, value);
            IncROG(1);
            return;
        }
        if (PopDensity[SMapY >>1][SMapX >>1] > 64) {
            ResPlop(SMapX, SMapY, 0, value);
            IncROG(8);
            return;
        }
        return;
    }
    if (pop < 40) {
        ResPlop(SMapX, SMapY, (pop / 8) - 1, value);
        IncROG(8);
    }
}

/* Handle commercial zone growth - matches original Micropolis */
static void DoComIn(int pop, int value) {
    register short z;
    
    z = LandValueMem[SMapY >>1][SMapX >>1];
    z = z >>5;
    if (pop > z) return;
    
    if (pop < 5) {
        ComPlop(SMapX, SMapY, pop);
        IncROG(8);
    }
}

/* Handle industrial zone growth - matches original Micropolis */
static void DoIndIn(int pop, int value) {
    if (pop < 4) {
        IndPlop(SMapX, SMapY, pop);
        IncROG(8);
    }
}

/* Increment Rate of Growth map - matches original Micropolis */
static void IncROG(int amount) {
    /* RateOGMem[SMapX>>3][SMapY>>3] += amount<<2; */
    /* Rate of Growth tracking not implemented yet */
}

/* Handle residential zone decline */
static void DoResOut(int pop, int value, int x, int y) {
    short base;

    base = (Map[y][x] & LOMASK) - RESBASE;

    if (base == 0) {
        return;
    }

    if (pop > 16) {
        /* Turn to small house */
        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base - 1);
    } else if ((base > 0) && (ZoneRandom(4) == 0)) {
        /* Gradually decay */
        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base - 1);
    }

    /* Check for complete ruin */
    if ((base < 4) && (value < 30) && (!pop) && (ZoneRandom(4) == 0)) {
        if (ZoneRandom(2) == 0) {
            /* Make into rubble */
            short z1;
            short z2;

            z1 = (ZoneRandom(3) + 43) | BULLBIT;

            z2 = Map[y][x] & LOMASK;
            if ((z2 < COMBASE) || (z2 > LASTIND)) {
                Map[y][x] = z1;
            }
        }
    }
}

/* Handle commercial zone decline */
static void DoComOut(int pop, int x, int y) {
    short base;

    base = (Map[y][x] & LOMASK) - COMBASE;

    if (base == 0) {
        return;
    }

    if ((base > 0) && (ZoneRandom(8) == 0)) {
        /* Gradually decay */
        Map[y][x] = (Map[y][x] & ALLBITS) | (COMBASE + base - 1);
    }
}

/* Handle industrial zone decline */
static void DoIndOut(int pop, int x, int y) {
    short base;

    base = (Map[y][x] & LOMASK) - INDBASE;

    if (base == 0) {
        return;
    }

    if ((base > 0) && (ZoneRandom(8) == 0)) {
        /* Gradually decay */
        Map[y][x] = (Map[y][x] & ALLBITS) | (INDBASE + base - 1);
    }
}

/* Using the global calcResPop, calcComPop, calcIndPop functions defined earlier */

/* Build a house at a location */
static void BuildHouse(int x, int y, int value) {
    short z;
    short score;
    short xx;
    short yy;

    z = 0;

    /* Find best place to build a house */
    for (yy = -1; yy <= 1; yy++) {
        for (xx = -1; xx <= 1; xx++) {
            if (xx || yy) {
                score = EvalLot(x + xx, y + yy);
                if (score > z) {
                    z = score;
                }
            }
        }
    }

    /* If we found any valid location, build house there */
    if (z > 0) {
        /* Would build the house at the specified location */
    }
}

/* Place a residential zone */
static void ResPlop(int x, int y, int den, int value) {
    /* den parameter is density offset for residential */
    ZonePlop(x, y, RESBASE + (den * 9) + value);
}

/* Place a commercial zone */
static void ComPlop(int x, int y, int den) {
    /* den parameter is density value for commercial */
    ZonePlop(x, y, COMBASE + (den * 9));
}

/* Place an industrial zone */
static void IndPlop(int x, int y, int den) {
    /* den parameter is density value for industrial */
    ZonePlop(x, y, INDBASE + (den * 9));
}

/* Evaluate a lot for building a house */
static int EvalLot(int x, int y) {
    int score;
    short z;

    score = 1;

    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y) {
        return -1;
    }

    z = Map[y][x] & LOMASK;

    if ((z >= RESBASE) && (z <= RESBASE + 8)) {
        score = 0;
    }

    if (score && (z != DIRT)) {
        score = 0;
    }

    if (!score) {
        return score;
    }

    /* Suitable place found! */
    return score;
}

/* Place a 3x3 zone on the map */
static int ZonePlop(int xpos, int ypos, int base) {
    short dx;
    short dy;
    short x;
    short y;
    short z;

    /* Bounds check */
    if (xpos < 0 || xpos >= WORLD_X || ypos < 0 || ypos >= WORLD_Y) {
        return 0;
    }

    /* Make sure center tile is bulldozable */
    if (!(Map[ypos][xpos] & BULLBIT)) {
        return 0;
    }

    /* Update zone center with the zone bit and power */
    Map[ypos][xpos] = (Map[ypos][xpos] & MASKBITS) | BNCNBIT | base | CONDBIT | BURNBIT | BULLBIT;

    /* Set the 3x3 zone around center */
    for (dy = -1; dy <= 1; dy++) {
        for (dx = -1; dx <= 1; dx++) {
            x = xpos + dx;
            y = ypos + dy;

            if (x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y) {
                if (!(dx == 0 && dy == 0)) {
                    /* Not the center tile */
                    z = Map[y][x] & LOMASK;

                    if ((z < ROADS) || (z > LASTRAIL)) {
                        if (Map[y][x] & BULLBIT) {
                            Map[y][x] = (Map[y][x] & MASKBITS) | (base + BSIZE + ZoneRandom(2)) |
                                        CONDBIT | BURNBIT | BULLBIT;
                        }
                    }
                }
            }
        }
    }

    return 1;
}

/* Evaluate residential zone desirability - matches original Micropolis */
static int EvalRes(int traf) {
    register short Value;
    
    if (traf < 0) return (-3000);
    
    Value = LandValueMem[SMapY >>1][SMapX >>1];
    Value -= PollutionMem[SMapY >>1][SMapX >>1];
    
    if (Value < 0) Value = 0;        /* Cap at 0 */
    else Value = Value <<5;
    
    if (Value > 6000) Value = 6000;  /* Cap at 6000 */
    
    Value = Value - 3000;
    return (Value);
}

/* Evaluate commercial zone desirability - matches original Micropolis */
static int EvalCom(int traf) {
    short Value;
    
    if (traf < 0) return (-3000);
    Value = ComRate[SMapY >>3][SMapX >>3];
    return (Value);
}

/* Evaluate industrial zone desirability - matches original Micropolis */
static int EvalInd(int traf) {
    if (traf < 0) return (-1000);
    return (0);
}

/* Count population of free houses */
static int DoFreePop(int x, int y) {
    int count;
    int xx;
    int yy;
    int xxx;
    int yyy;
    short z;

    count = 0;

    for (yy = -1; yy <= 1; yy++) {
        for (xx = -1; xx <= 1; xx++) {
            xxx = x + xx;
            yyy = y + yy;

            if (xxx >= 0 && xxx < WORLD_X && yyy >= 0 && yyy < WORLD_Y) {
                z = Map[yyy][xxx] & LOMASK;

                if (z >= LHTHR && z <= HHTHR) {
                    count++;
                }
            }
        }
    }

    return count;
}

/* Set zone power status - simplified for reliability */
static void SetZPower(int x, int y) {
    short z;
    int powered;
    int dx, dy;

    /* First check if this is a power plant - they're always powered */
    z = Map[y][x] & LOMASK;

    if (z == NUCLEAR || z == POWERPLANT) {
        /* Power plants are always powered */
        Map[y][x] |= POWERBIT;
        PwrdZCnt++;
        return;
    }

    /* Check if already powered according to PowerMap */
    if (PowerMap[y][x] == 1) {
        Map[y][x] |= POWERBIT;
        powered = 1;
    }
    /* If not already powered, check surrounding tiles */
    else {
        powered = 0;

        /* Check all 8 surrounding tiles for power */
        for (dy = -1; dy <= 1 && !powered; dy++) {
            for (dx = -1; dx <= 1 && !powered; dx++) {
                int nx = x + dx;
                int ny = y + dy;

                /* Skip the center tile */
                if (dx == 0 && dy == 0) {
                    continue;
                }

                /* Check if neighbor is in bounds */
                if (nx >= 0 && nx < WORLD_X && ny >= 0 && ny < WORLD_Y) {
                    /* If a neighboring tile has power, this zone gets power */
                    if (Map[ny][nx] & POWERBIT) {
                        powered = 1;
                        break;
                    }
                }
            }
        }

        /* Update the power bit based on our check */
        if (powered) {
            Map[y][x] |= POWERBIT;
            PowerMap[y][x] = 1;
        } else {
            Map[y][x] &= ~POWERBIT;
            PowerMap[y][x] = 0;
        }
    }

    /* Update power count - use the external variables from sim.h */
    if (powered) {
        PwrdZCnt++;
    } else {
        UnpwrdZCnt++;
    }
}