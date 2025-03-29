/* zone.c - Zone processing for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "simulation.h"

/* Population table values for different zone types */
#define RZB 0   /* Residential base level */
#define hospital 0
#define church 0
#define CZB 0   /* Commercial base level */
#define IZB 0   /* Industrial base level */

/* Zone base values */
#define RESBASE 240
#define COMBASE 423
#define INDBASE 612
#define PORTBASE 693
#define AIRPORTBASE 709
#define COALBASE 745
#define FIRESTBASE 761
#define POLICESTBASE 770
#define STADIUMBASE 779
#define NUCLEARBASE 811

/* Zone bit flags */
#define ALLBITS 0xFFFF
#define RESBIT 0x0001
#define COMBIT 0x0002
#define INDBIT 0x0004

/* Internal variables */
static int RZPop;  /* Residential zone population */
static int CZPop;  /* Commercial zone population */
static int IZPop;  /* Industrial zone population */
/* ComRate is declared in simulation.h as quarter size */

/* Forward declarations */
static void DoResidential(int x, int y);
static void DoCommercial(int x, int y);
static void DoIndustrial(int x, int y);
static void DoHospChur(int x, int y);
static void DoSPZ(int x, int y);
static void SetSmoke(int x, int y);
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
int calcResPop(int zone)
{
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
    
    if (zone == HOSPITAL) {
        return 30;
    }
    if (zone == CHURCH) {
        return 10;
    }
    
    return 0;
}

/* Calculate population in a commercial zone */
int calcComPop(int zone)
{
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
    
    return 0;
}

/* Calculate population in an industrial zone */
int calcIndPop(int zone)
{
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
static int ZoneRandom(int range)
{
    return rand() % range;
}

/* Main zone processing function */
void DoZone(int Xloc, int Yloc, int pos)
{
    /* Do special processing based on zone type */
    if (pos >= RESBASE) {
        if (pos < COMBASE) {
            DoResidential(Xloc, Yloc);
            return;
        }
    
        if (pos < INDBASE) {
            DoCommercial(Xloc, Yloc);
            return;
        }
    
        if (pos < PORTBASE) {
            DoIndustrial(Xloc, Yloc);
            return;
        }
    }
    
    if (pos < PORTBASE)
        return;
        
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
static void DoHospChur(int x, int y)
{
    short z;
    
    if (!(Map[y][x] & ZONEBIT))
        return;
    
    SetZPower(x, y);
    
    if (CityTime & 3)
        return;
    
    z = Map[y][x] & LOMASK;
    
    if (z == HOSPITAL) {
        if (ZoneRandom(20) < 10)
            IZPop++;
        return;
    }
    
    if (z == CHURCH) {
        if (ZoneRandom(20) < 10)
            RZPop++;
    }
}

/* Process special zone (stadiums, coal plants, etc) */
static void DoSPZ(int x, int y)
{
    short z;
    
    if (!(Map[y][x] & ZONEBIT))
        return;
    
    SetZPower(x, y);
    
    /* Only process every 16th time */
    if (CityTime & 15)
        return;
    
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
        
        /* Random chance to increase commercial zone */
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
static void SetSmoke(int x, int y)
{
    int dx;
    int dy;
    int xx;
    int yy;
    int zz;
    
    /* Skip most of the time for performance */
    if ((CityTime & 15) || ZoneRandom(5))
        return;
    
    xx = x - 1;
    yy = y - 1;
    
    if (TestBounds(xx, yy)) {
        zz = Map[yy][xx];
        if ((zz & LOMASK) == HPOWER || (zz & LOMASK) == LPOWER) {
            dx = 0;
            dy = 0;
            
            /* Choose random smoke direction from the coal plant */
            switch (ZoneRandom(8)) {
                case 0: dx = 0; dy = -1; break;
                case 1: dx = 1; dy = -1; break;
                case 2: dx = 1; dy = 0; break;
                case 3: dx = 1; dy = 1; break;
                case 4: dx = 0; dy = 1; break;
                case 5: dx = -1; dy = 1; break;
                case 6: dx = -1; dy = 0; break;
                case 7: dx = -1; dy = -1; break;
            }
            
            /* Would add smoke animation here */
        }
    }
}

/* Process industrial zone */
static void DoIndustrial(int x, int y)
{
    short zone;
    short tpop;
    int pop;
    
    zone = Map[y][x];
    if (!(zone & ZONEBIT))
        return;
        
    SetZPower(x, y);
    
    tpop = IZPop;
    
    /* Get actual zone population */
    pop = calcIndPop(zone);
    
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
static void DoCommercial(int x, int y)
{
    short zone;
    short tpop;
    int pop;
    
    zone = Map[y][x];
    if (!(zone & ZONEBIT))
        return;
        
    SetZPower(x, y);
    
    tpop = CZPop;
    
    /* Get actual zone population */
    pop = calcComPop(zone);
    
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
static void DoResidential(int x, int y)
{
    short zone;
    short tpop;
    int pop;
    
    zone = Map[y][x];
    if (!(zone & ZONEBIT))
        return;
        
    SetZPower(x, y);
    
    tpop = RZPop;
    
    /* Get actual zone population */
    pop = calcResPop(zone);
    
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
    
    /* Process residential zone less often (every 8th cycle) */
    if ((CityTime & 7) == 0) {
        int value;
        
        value = GetCRVal(x, y);
        
        if (value < 0) {
            DoResOut(tpop, value, x, y);
            return;
        }
        
        value = EvalRes(x, y);
        
        if (value > 0) {
            DoResIn(tpop, value, x, y);
        } else if (value < 0) {
            DoResOut(tpop, value, x, y);
        }
    }
    
    if ((CityTime & 7) == 0) {
        RZPop = 0;
    }
}

/* Calculate land value for a location */
static int GetCRVal(int x, int y)
{
    int landVal;
    
    landVal = LandValueMem[y>>1][x>>1];
    
    /* Check if it's connected to power */
    if (!(Map[y][x] & POWERBIT)) {
        landVal -= 20;
        
        /* No power = bad location */
        if (landVal < 0)
            landVal = 0;
            
        if (landVal < 30)
            return -1;
    }
    
    return landVal;
}

/* Handle residential zone growth */
static void DoResIn(int pop, int value, int x, int y)
{
    short base;
    short z;
    short deltaValue;
    
    if (pop > 0) {
        if (pop < 8 + value/8) {
            IncROG(x, y);
            return;
        }
    }
    
    /* Zone is ready for growth! */
    base = (Map[y][x] & LOMASK) - RESBASE;
    
    z = RZPop;
    if (z < 0)
        z = 0;
        
    if (base < 3) {
        /* Low-value residential building: 0, 1, 2 */
        if (ZoneRandom(4)) {
            Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base + 1);
            IncROG(x, y);
        }
    } else if (base < 6) {
        /* Medium-value residential building: 3, 4, 5 */
        deltaValue = 0;
        
        if (value >= 80)
            deltaValue = 1;
            
        if (value >= 150)
            deltaValue = 2;
            
        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base + deltaValue);
        
        if (ZoneRandom(16) < 2) {
            ResPlop(x, y, ZoneRandom(2) + base - 3);
            IncROG(x, y);
        }
    } else {
        /* Big residential building: 6, 7, 8 */
        Map[y][x] = (Map[y][x] & ALLBITS) | (RESBASE + base);
        
        if (value > 150 && (ZoneRandom(4) == 0)) {
            if (ZoneRandom(2)) {
                BuildHouse(x, y, value);
            } else {
                ResPlop(x, y, ZoneRandom(3) + 3);
            }
        }
    }
}

/* Handle commercial zone growth */
static void DoComIn(int pop, int x, int y)
{
    short base;
    
    if (pop < 5) {
        IncROG(x, y);
        return;
    }
    
    /* Zone is ready for growth! */
    base = (Map[y][x] & LOMASK) - COMBASE;
    
    if (ZoneRandom(4) == 0) {
        if (ZoneRandom(2) == 0) {
            ComPlop(x, y, ZoneRandom(5));
        } else {
            ComPlop(x, y, ZoneRandom(4) + 5);
        }
        IncROG(x, y);
    }
}

/* Handle industrial zone growth */
static void DoIndIn(int pop, int x, int y)
{
    if (pop < 4) {
        IncROG(x, y);
        return;
    }
    
    /* Zone is ready for growth! */
    if (ZoneRandom(4) == 0) {
        if (ZoneRandom(2) == 0) {
            IndPlop(x, y, ZoneRandom(4));
        } else {
            IndPlop(x, y, ZoneRandom(4) + 4);
        }
        IncROG(x, y);
    }
}

/* Increment Rate of Growth map */
static void IncROG(int x, int y)
{
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
static void DoResOut(int pop, int value, int x, int y)
{
    short base;
    
    base = (Map[y][x] & LOMASK) - RESBASE;
    
    if (base == 0)
        return;
    
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
            if ((z2 < COMBASE) || (z2 > LASTIND))
                Map[y][x] = z1;
        }
    }
}

/* Handle commercial zone decline */
static void DoComOut(int pop, int x, int y)
{
    short base;
    
    base = (Map[y][x] & LOMASK) - COMBASE;
    
    if (base == 0)
        return;
    
    if ((base > 0) && (ZoneRandom(8) == 0)) {
        /* Gradually decay */
        Map[y][x] = (Map[y][x] & ALLBITS) | (COMBASE + base - 1);
    }
}

/* Handle industrial zone decline */
static void DoIndOut(int pop, int x, int y)
{
    short base;
    
    base = (Map[y][x] & LOMASK) - INDBASE;
    
    if (base == 0)
        return;
    
    if ((base > 0) && (ZoneRandom(8) == 0)) {
        /* Gradually decay */
        Map[y][x] = (Map[y][x] & ALLBITS) | (INDBASE + base - 1);
    }
}

/* Using the global calcResPop, calcComPop, calcIndPop functions defined earlier */

/* Build a house at a location */
static void BuildHouse(int x, int y, int value)
{
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
                if (score > z)
                    z = score;
            }
        }
    }
    
    /* If we found any valid location, build house there */
    if (z > 0) {
        /* Would build the house at the specified location */
    }
}

/* Place a residential zone */
static void ResPlop(int x, int y, int value)
{
    ZonePlop(x, y, RESBASE + value);
}

/* Place a commercial zone */
static void ComPlop(int x, int y, int value)
{
    ZonePlop(x, y, COMBASE + value);
}

/* Place an industrial zone */
static void IndPlop(int x, int y, int value)
{
    ZonePlop(x, y, INDBASE + value);
}

/* Evaluate a lot for building a house */
static int EvalLot(int x, int y)
{
    int score;
    short z;
    
    score = 1;
    
    if (x < 0 || x >= WORLD_X || y < 0 || y >= WORLD_Y)
        return -1;
    
    z = Map[y][x] & LOMASK;
    
    if ((z >= RESBASE) && (z <= RESBASE + 8))
        score = 0;
    
    if (score && (z != DIRT))
        score = 0;
    
    if (!score)
        return score;
    
    /* Suitable place found! */
    return score;
}

/* Place a 3x3 zone on the map */
static int ZonePlop(int xpos, int ypos, int base)
{
    short dx;
    short dy;
    short x;
    short y;
    short z;
    
    /* Bounds check */
    if (xpos < 0 || xpos >= WORLD_X || ypos < 0 || ypos >= WORLD_Y)
        return 0;
    
    /* Make sure center tile is bulldozable */
    if (!(Map[ypos][xpos] & BULLBIT))
        return 0;
    
    /* Update zone center with the zone bit and power */
    Map[ypos][xpos] = (Map[ypos][xpos] & MASKBITS) | BNCNBIT |
                      base | CONDBIT | BURNBIT | BULLBIT;
    
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
                            Map[y][x] = (Map[y][x] & MASKBITS) | 
                                       (base + BSIZE + ZoneRandom(2)) |
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
static int EvalRes(int x, int y)
{
    int value;
    
    value = LandValueMem[y>>1][x>>1];
    
    if (value < 30)
        return -1;
    
    if (value < 80) {
        if (PollutionMem[y>>1][x>>1] > 80)
            return -1;
    } else {
        if (PollutionMem[y>>1][x>>1] > 40)
            return -1;
    }
    
    if (CrimeMem[y>>1][x>>1] > 190)
        return -1;
    
    /* Calculate growth based on global residential demand (RValve) */
    if ((value > 30) && RValve < 0)
        return 0;
    
    if ((value > 80) && RValve < 20)
        return 0;
    
    if ((value > 150) && RValve < 40)
        return 0;
    
    return value;
}

/* Evaluate commercial zone desirability */
static int EvalCom(int x, int y)
{
    int value;
    
    value = ComRate[y>>1][x>>1];
    
    if (value < 1)
        return -1;
    
    if (value < 20 && CValve < 0)
        return -1;
    
    if (value < 50 && CValve < 10)
        return -1;
    
    if (value < 100 && CValve < 20)
        return 0;
    
    if (value < 150)
        return value;
    
    if (CValve < 40)
        return 0;
    
    return value;
}

/* Evaluate industrial zone desirability */
static int EvalInd(int x, int y)
{
    if (IValve < 0)
        return -1;
    
    if (IValve < 50)
        return 0;
    
    return IValve;
}

/* Count population of free houses */
static int DoFreePop(int x, int y)
{
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
                
                if (z >= LHTHR && z <= HHTHR)
                    count++;
            }
        }
    }
    
    return count;
}

/* Set zone power status */
static void SetZPower(int x, int y)
{
    int powered;
    short z;
    
    powered = 0;
    
    /* Check if this zone center needs power */
    z = Map[y][x] & LOMASK;
    
    if (z < PORTBASE)
        return;
    
    /* Check if there's power nearby */
    if (GetPValue(x, y)) {
        powered = POWERBIT;
    } else {
        powered = 0;
    }
    
    /* Set power bit appropriately */
    Map[y][x] = (Map[y][x] & (~POWERBIT)) | powered;
    
    /* Update power count */
    if (powered) {
        PwrdZCnt++;
    } else {
        UnpwrdZCnt++;
    }
}