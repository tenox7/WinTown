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
static void ResPlop(int x, int y, int value);
static void ComPlop(int x, int y, int value);
static void IndPlop(int x, int y, int value);
static int ZonePlop(int x, int y, int base);
static int GetCRVal(int x, int y);
static void DoResIn(int pop, int value, int x, int y);
static void DoComIn(int pop, int x, int y);
static void DoIndIn(int pop, int x, int y);

/* Calculate population in a residential zone */
int calcResPop(int zone) {
    int pop;

    /* Check if in residential range */
    if (zone >= RESBASE && zone < RESBASE + 8) {
        /* Low value */
        pop = zone - RESBASE;
        pop = pop << 3;
        return pop;
    }

    if (zone >= RESBASE + 8 && zone < RESBASE + 16) {
        /* Medium value */
        pop = zone - RESBASE - 8;
        pop = pop << 3;
        pop += 32;
        return pop;
    }

    if (zone >= RESBASE + 16 && zone < RESBASE + 24) {
        /* High value */
        pop = zone - RESBASE - 16;
        pop = pop << 3;
        pop += 64;
        return pop;
    }

    /* Continue checking other residential zone types */
    if (zone >= RESBASE + 24 && zone <= LASTRES) {
        /* Higher density residential */
        return 16 + (zone - RESBASE - 24) / 8 * 16;
    }

    if (zone == HOSPITAL) {
        return 30;
    }
    if (zone == CHURCH) {
        return 10;
    }

    return 0;
}

/* Calculate population in a commercial zone */
int calcComPop(int zone) {
    int pop;

    /* Check if in commercial range */
    if (zone >= COMBASE && zone < COMBASE + 8) {
        /* Low value */
        pop = zone - COMBASE;
        pop = pop << 3;
        return pop;
    }

    if (zone >= COMBASE + 8 && zone < COMBASE + 16) {
        /* Medium value */
        pop = zone - COMBASE - 8;
        pop = pop << 3;
        pop += 32;
        return pop;
    }

    if (zone >= COMBASE + 16 && zone < COMBASE + 24) {
        /* High value */
        pop = zone - COMBASE - 16;
        pop = pop << 3;
        pop += 64;
        return pop;
    }

    /* Continue checking other commercial zone types */
    if (zone >= COMBASE + 24 && zone <= LASTCOM) {
        /* Higher density commercial */
        return 16 + (zone - COMBASE - 24) / 8 * 16;
    }

    return 0;
}

/* Calculate population in an industrial zone */
int calcIndPop(int zone) {
    int pop;

    /* Check if in industrial range */
    if (zone >= INDBASE && zone < INDBASE + 8) {
        /* Low value */
        pop = zone - INDBASE;
        pop = pop << 3;
        return pop;
    }

    if (zone >= INDBASE + 8 && zone < INDBASE + 16) {
        /* Medium value */
        pop = zone - INDBASE - 8;
        pop = pop << 3;
        pop += 32;
        return pop;
    }

    if (zone >= INDBASE + 16 && zone < INDBASE + 24) {
        /* High value */
        pop = zone - INDBASE - 16;
        pop = pop << 3;
        pop += 64;
        return pop;
    }

    /* Continue checking other industrial zone types */
    if (zone >= INDBASE + 24 && zone <= LASTIND) {
        /* Higher density industrial */
        return 16 + (zone - INDBASE - 24) / 8 * 16;
    }

    /* Special case for certain building types */
    if (zone == AIRPORT || zone == PORT) {
        return 40;
    }

    return 0;
}
static void IncROG(int x, int y);
static void DoResOut(int pop, int value, int x, int y);
static void DoComOut(int pop, int x, int y);
static void DoIndOut(int pop, int x, int y);
static int EvalRes(int x, int y);
static int EvalCom(int x, int y);
static int EvalInd(int x, int y);
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
            ResPop += 30;
        } else {
            ResPop += 5; /* Even unpowered hospitals have some population */
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
            ResPop += 10;
        } else {
            ResPop += 2; /* Even unpowered churches have some population */
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
        ComPop += 50; /* Stadiums contribute to commercial population */

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

    /* Get actual zone population */
    pop = calcIndPop(zone);
    
    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered industrial zone */
    }

    /* Add to total industrial population for the census */
    IndPop += pop;

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

        value = EvalInd(x, y);

        if (value > 0) {
            DoIndIn(tpop, x, y);
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

    /* Get actual zone population */
    pop = calcComPop(zone);
    
    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered commercial zone */
    }

    /* Add to total commercial population for the census */
    ComPop += pop;

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

        value = EvalCom(x, y);

        if (value > 0) {
            DoComIn(tpop, x, y);
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

    /* Get actual zone population */
    pop = calcResPop(zone);

    /* Make sure even empty zones contribute some population */
    if (pop == 0 && zonePowered) {
        pop = 1;  /* Minimum population for any powered residential zone */
    }

    /* Add to total residential population for the census */
    ResPop += pop;

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

        value = EvalRes(x, y);

        if (value > 0) {
            DoResIn(tpop, value, x, y);
        } else if (value < 0) {
            DoResOut(tpop, value, x, y);
        }
    }

    /* Reset population counter periodically */
    if ((CityTime & 7) == 0) {
        RZPop = 0;
    }
}

/* Calculate land value for a location */
static int GetCRVal(int x, int y) {
    int landVal;

    landVal = LandValueMem[y >> 1][x >> 1];

    /* Check if it's connected to power */
    if (!(Map[y][x] & POWERBIT)) {
        landVal -= 20;

        /* No power = bad location */
        if (landVal < 0) {
            landVal = 0;
        }

        if (landVal < 30) {
            return -1;
        }
    }

    return landVal;
}

/* Handle residential zone growth */
static void DoResIn(int pop, int value, int x, int y) {
    short base;
    short z;
    short deltaValue;
    short growthRate = 1;

    /* Increase growth likelihood */
    if (RValve > 500) {
        growthRate = 2; /* More aggressive growth with high demand */
    }

    if (pop > 0) {
        /* Less restrictive population requirement for growth */
        if (pop < 6 + value / 12) {
            IncROG(x, y);
            return;
        }
    }

    /* Zone is ready for growth! */
    base = (Map[y][x] & LOMASK) - RESBASE;

    z = RZPop;
    if (z < 0) {
        z = 0;
    }

    if (base < 3) {
        /* Low-value residential building: 0, 1, 2 */
        /* Increased chance of growth */
        if (ZoneRandom(3) || (RValve > 300 && ZoneRandom(2))) {
            Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base + growthRate);
            IncROG(x, y);

            /* Debug the growth */
            {
                char debugMsg[256];
                wsprintf(debugMsg, "ZONE GROWTH: Residential base %d grew to %d at (%d,%d)\n", base,
                         base + growthRate, x, y);
                OutputDebugString(debugMsg);
            }
        }
    } else if (base < 6) {
        /* Medium-value residential building: 3, 4, 5 */
        deltaValue = 0;

        if (value >= 80) {
            deltaValue = 1;
        }

        if (value >= 150) {
            deltaValue = 2;
        }

        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base + deltaValue);

        /* Increased chance of growth */
        if (ZoneRandom(12) < 3 || (RValve > 400 && ZoneRandom(10) < 3)) {
            ResPlop(x, y, ZoneRandom(2) + base - 3);
            IncROG(x, y);

            /* Debug the growth */
            {
                char debugMsg[256];
                wsprintf(debugMsg, "ZONE GROWTH: Medium residential at (%d,%d) upgraded\n", x, y);
                OutputDebugString(debugMsg);
            }
        }
    } else {
        /* Big residential building: 6, 7, 8 */
        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base);

        /* Increased chance of growth */
        if ((value > 120 && ZoneRandom(8) < 2) || (RValve > 500 && ZoneRandom(10) < 3)) {
            if (ZoneRandom(2)) {
                BuildHouse(x, y, value);
            } else {
                ResPlop(x, y, ZoneRandom(3) + 3);
            }

            /* Debug the growth */
            {
                char debugMsg[256];
                wsprintf(debugMsg, "ZONE GROWTH: Large residential at (%d,%d) upgraded\n", x, y);
                OutputDebugString(debugMsg);
            }
        }
    }
}

/* Handle commercial zone growth */
static void DoComIn(int pop, int x, int y) {
    short base;

    /* Reduced population threshold for growth */
    if (pop < 4) {
        IncROG(x, y);
        return;
    }

    /* Zone is ready for growth! */
    base = (Map[y][x] & LOMASK) - COMBASE;

    /* Increased chance of growth - more likely if CValve (commercial demand) is high */
    if (ZoneRandom(3) == 0 || (CValve > 400 && ZoneRandom(5) < 2)) {
        if (ZoneRandom(2) == 0) {
            ComPlop(x, y, ZoneRandom(5));
        } else {
            ComPlop(x, y, ZoneRandom(4) + 5);
        }
        IncROG(x, y);

        /* Debug the growth */
        {
            char debugMsg[256];
            wsprintf(debugMsg, "ZONE GROWTH: Commercial zone at (%d,%d) upgraded\n", x, y);
            OutputDebugString(debugMsg);
        }
    }
}

/* Handle industrial zone growth */
static void DoIndIn(int pop, int x, int y) {
    /* Reduced population threshold for growth */
    if (pop < 3) {
        IncROG(x, y);
        return;
    }

    /* Zone is ready for growth! */
    /* Increased chance of growth - more likely if IValve (industrial demand) is high */
    if (ZoneRandom(3) == 0 || (IValve > 400 && ZoneRandom(5) < 2)) {
        if (ZoneRandom(2) == 0) {
            IndPlop(x, y, ZoneRandom(4));
        } else {
            IndPlop(x, y, ZoneRandom(4) + 4);
        }
        IncROG(x, y);

        /* Debug the growth */
        {
            char debugMsg[256];
            wsprintf(debugMsg, "ZONE GROWTH: Industrial zone at (%d,%d) upgraded\n", x, y);
            OutputDebugString(debugMsg);
        }
    }
}

/* Increment Rate of Growth map */
static void IncROG(int x, int y) {
    /* Increment the rate of growth map */
    if (x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y) {
        short xx;
        short yy;

        xx = x >> 3;
        yy = y >> 3;
        /* Rate of growth map would be incremented here */
    }
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
static void ResPlop(int x, int y, int value) {
    ZonePlop(x, y, RESBASE + value);
}

/* Place a commercial zone */
static void ComPlop(int x, int y, int value) {
    ZonePlop(x, y, COMBASE + value);
}

/* Place an industrial zone */
static void IndPlop(int x, int y, int value) {
    ZonePlop(x, y, INDBASE + value);
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

/* Evaluate residential zone desirability */
static int EvalRes(int x, int y) {
    int value;

    value = LandValueMem[y >> 1][x >> 1];

    /* Reduced minimum value requirement */
    if (value < 20) {
        return -1;
    }

    /* Increased tolerance for pollution */
    if (value < 80) {
        if (PollutionMem[y >> 1][x >> 1] > 100) { /* Was 80 */
            return -1;
        }
    } else {
        if (PollutionMem[y >> 1][x >> 1] > 60) { /* Was 40 */
            return -1;
        }
    }

    /* Increased tolerance for crime */
    if (CrimeMem[y >> 1][x >> 1] > 220) { /* Was 190 */
        return -1;
    }

    /* Less restrictive RValve requirements */
    if ((value > 30) && RValve < -100) { /* Was 0 */
        return 0;
    }

    if ((value > 80) && RValve < 0) { /* Was 20 */
        return 0;
    }

    if ((value > 150) && RValve < 20) { /* Was 40 */
        return 0;
    }

    /* Bump up values slightly to encourage more growth */
    return value + 10;
}

/* Evaluate commercial zone desirability */
static int EvalCom(int x, int y) {
    int value;

    value = ComRate[y >> 1][x >> 1];

    /* Reduced minimum requirement */
    if (value < 1) {
        return -1;
    }

    /* Less restrictive CValve requirements */
    if (value < 20 && CValve < -50) { /* Was 0 */
        return -1;
    }

    if (value < 50 && CValve < 0) { /* Was 10 */
        return -1;
    }

    if (value < 100 && CValve < 10) { /* Was 20 */
        return 0;
    }

    if (value < 150) {
        return value + 10; /* Boost value to encourage growth */
    }

    if (CValve < 20) { /* Was 40 */
        return 0;
    }

    /* Boost high values */
    return value + 20;
}

/* Evaluate industrial zone desirability */
static int EvalInd(int x, int y) {
    /* Less restrictive IValve requirements */
    if (IValve < -50) { /* Was 0 */
        return -1;
    }

    if (IValve < 20) { /* Was 50 */
        return 0;
    }

    /* Boost value to encourage growth */
    return IValve + 50;
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