/* simulation.c - Core simulation implementation for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simulation.h"

/* Map data */
Byte PopDensity[WORLD_Y/2][WORLD_X/2];
Byte TrfDensity[WORLD_Y/2][WORLD_X/2];
Byte PollutionMem[WORLD_Y/2][WORLD_X/2];
Byte LandValueMem[WORLD_Y/2][WORLD_X/2];
Byte CrimeMem[WORLD_Y/2][WORLD_X/2];
short PowerMap[WORLD_Y][WORLD_X];

/* Quarter-sized maps for effects */
Byte TerrainMem[WORLD_Y/4][WORLD_X/4];
Byte FireStMap[WORLD_Y/4][WORLD_X/4];
Byte FireRate[WORLD_Y/4][WORLD_X/4];
Byte PoliceMap[WORLD_Y/4][WORLD_X/4];
Byte PoliceMapEffect[WORLD_Y/4][WORLD_X/4];

/* Commercial development score */
short ComRate[WORLD_Y/4][WORLD_X/4];

/* Runtime simulation state */
int SimSpeed = SPEED_MEDIUM;
int SimSpeedMeta = 0;
int SimPaused = 1;
int CityTime = 0;
int CityYear = 1900;
int CityMonth = 0;
QUAD TotalFunds = 5000;
int TaxRate = 7;

/* Counters */
int Scycle = 0;
int Fcycle = 0;
int Spdcycle = 0;

/* Game evaluation */
int CityYes = 0;
int CityNo = 0;
QUAD CityPop = 0;
int CityScore = 500;
int deltaCityScore = 0;
int CityClass = 0;
int CityLevel = 0;
int CityLevelPop = 0;
int GameLevel = 0;
int ResCap = 0;
int ComCap = 0;
int IndCap = 0;

/* City statistics */
int ResPop = 0;
int ComPop = 0;
int IndPop = 0;
int TotalPop = 0;
int LastTotalPop = 0;
float Delta = 1.0f;

/* Infrastructure counts */
int PwrdZCnt = 0;
int UnpwrdZCnt = 0;
int RoadTotal = 0;
int RailTotal = 0;
int FirePop = 0;
int PolicePop = 0;
int StadiumPop = 0;
int PortPop = 0;
int APortPop = 0;
int NuclearPop = 0;

/* External effects */
int RoadEffect = 0;
int PoliceEffect = 0;
int FireEffect = 0;
int TrafficAverage = 0;
int PollutionAverage = 0;
int CrimeAverage = 0;
int LVAverage = 0;

/* Growth rates */
short RValve = 0;
short CValve = 0;
short IValve = 0;
int ValveFlag = 0;

/* Disasters */
int DisasterEvent = 0;
int DisasterWait = 0;
int DisasterLevel = 0;

/* Internal work variables - also used by power.c */
int SMapX, SMapY;  /* Current map position (no longer static, needed by power.c) */
static int TMapX, TMapY;
static short CChr;
static short CChr9;

/* Random number generator - Windows compatible */
static void RandomlySeedRand(void)
{
    srand((unsigned int)GetTickCount());
}

/* Public random number function - available to other modules */
int SimRandom(int range)
{
    return (rand() % range);
}

void DoSimInit(void)
{
    int x, y;
    
    /* Clear all the density maps */
    memset(PopDensity, 0, sizeof(PopDensity));
    memset(TrfDensity, 0, sizeof(TrfDensity));
    memset(PollutionMem, 0, sizeof(PollutionMem));
    memset(LandValueMem, 0, sizeof(LandValueMem));
    memset(CrimeMem, 0, sizeof(CrimeMem));
    
    /* Clear all the effect maps */
    memset(TerrainMem, 0, sizeof(TerrainMem));
    memset(FireStMap, 0, sizeof(FireStMap));
    memset(FireRate, 0, sizeof(FireRate));
    memset(PoliceMap, 0, sizeof(PoliceMap));
    memset(PoliceMapEffect, 0, sizeof(PoliceMapEffect));
    memset(ComRate, 0, sizeof(ComRate));
    
    /* Initialize land values to make simulation more visually interesting */
    for (y = 0; y < WORLD_Y/2; y++) {
        for (x = 0; x < WORLD_X/2; x++) {
            /* Create a gradient of land values */
            int centerX = WORLD_X/4;
            int centerY = WORLD_Y/4;
            int distance;
            int value;
            
            distance = (x - centerX) * (x - centerX) + (y - centerY) * (y - centerY);
            value = 250 - (distance / 10);
            if (value < 1) value = 1;
            if (value > 250) value = 250;
            
            LandValueMem[y][x] = (Byte)value;
        }
    }
    
    /* Clear power map and set to unpowered (0) */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            PowerMap[y][x] = 0;  /* Use 0 instead of -1 */
            /* Also clear the power bit in the map */
            Map[y][x] &= ~POWERBIT;
        }
    }
    
    /* Set up random number generator */
    RandomlySeedRand();
    
    /* Initialize simulation counters */
    Scycle = 0;
    Fcycle = 0;
    Spdcycle = 0;
    
    /* Default settings */
    SimSpeed = SPEED_MEDIUM;
    SimPaused = 0;  /* Set to running by default */
    CityTime = 0;
    CityYear = 1900;
    CityMonth = 0;
    
    /* Set initial growth demand */
    SetValves(500, 300, 100);
    ValveFlag = 1;
    
    /* Set initial funds and tax rate */
    TotalFunds = 50000;  /* Give more starting money */
    TaxRate = 7;
    
    /* Initial game state */
    CityScore = 500;
    DisasterEvent = 0;
    DisasterWait = 0;
    
    /* Initialize evaluation system */
    EvalInit();
    
    /* Initialize budget system */
    InitBudget();
    
    /* Generate a random disaster wait period */
    DisasterWait = SimRandom(51) + 49;
}

void SimFrame(void)
{
    /* Main simulation frame entry point */
    
    if (SimPaused)
        return;
        
    /* Update the speed cycle counter */
    Spdcycle = (Spdcycle + 1) & (SPEEDCYCLE - 1);
    
    /* Execute simulation steps based on speed */
    switch (SimSpeed) {
        case SPEED_PAUSED:
            /* Do nothing - simulation is paused */
            break;
            
        case SPEED_SLOW:
            /* Slow speed - process every 5th frame */
            if ((Spdcycle % 5) == 0) {
                Fcycle = (Fcycle + 1) & 1023;
                Simulate(Fcycle & 15);
            }
            break;
            
        case SPEED_MEDIUM:
            /* Medium speed - process every 3rd frame */
            if ((Spdcycle % 3) == 0) {
                Fcycle = (Fcycle + 1) & 1023;
                Simulate(Fcycle & 15);
            }
            break;
            
        case SPEED_FAST:
            /* Fast speed - process every frame */
            Fcycle = (Fcycle + 1) & 1023;
            Simulate(Fcycle & 15);
            break;
    }
}

void Simulate(int mod16)
{
    /* Main simulation logic */
    
    Scycle = (Scycle + 1) & 1023;
    
    /* Perform different actions based on the cycle position (mod 16) */
    switch (mod16) {
        case 0:
            /* Increment time, check for disasters, process valve changes */
            DoTimeStuff();
            
            /* Adjust valves when needed */
            if (ValveFlag) {
                SetValves(RValve, CValve, IValve);
                ValveFlag = 0;
            }
            break;
            
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            /* Scan map in 8 different segments (1/8th each time) */
            {
                int xs = (mod16 - 1) * (WORLD_X / 8);
                int xe = xs + (WORLD_X / 8);
                MapScan(xs, xe, 0, WORLD_Y);
            }
            break;
            
        case 9:
            /* Process taxes, maintenance, city evaluation if needed */
            if ((Scycle & 1) == 0) {
                /* Even cycles: do census for graphs */
                ClearCensus();
            }
            
            /* Every 4 cycles, take census for graphs */
            if ((Scycle % CENSUSRATE) == 0) {
                TakeCensus();
            }
            
            /* Every 48 cycles, do tax collection and evaluation */
            if ((Scycle % TAXFREQ) == 0) {
                CollectTax();          /* Collect taxes based on population */
                CountSpecialTiles();   /* Count special buildings */
                CityEvaluation();      /* Evaluate city conditions */
            }
            break;
            
        case 10:
            /* Process traffic decrease & other tile updates */
            DecTrafficMap();
            
            /* Calculate traffic average periodically */
            if ((Scycle % 4) == 0) {
                CalcTrafficAverage();
            }
            break;
            
        case 11:
            /* Process power grid updates */
            DoPowerScan();
            
            /* Check if population has gone to zero (but not initially) */
            if (TotalPop > 0 || LastTotalPop == 0) {
                LastTotalPop = TotalPop;
            } else if (TotalPop == 0 && LastTotalPop != 0) {
                /* ToDo: DoShowPicture(POPULATIONLOST_BIT); */
                LastTotalPop = 0;
            }
            break;
            
        case 12:
            /* Process pollution spread (at a reduced rate) */
            if ((Scycle % 16) == 0) {
                PTLScan(); /* Do pollution, terrain, and land value */
            }
            break;
            
        case 13:
            /* Process crime spread (at a reduced rate) */
            if ((Scycle % 4) == 0) {
                CrimeScan(); /* Do crime map analysis */
            }
            break;
            
        case 14:
            /* Process population density (at a reduced rate) */
            if ((Scycle % 16) == 0) {
                PopDenScan(); /* Do population density scan */
                FireAnalysis(); /* Update fire protection effect */
            }
            break;
            
        case 15:
            /* Process fire analysis and disasters (at a reduced rate) */
            if ((Scycle % 4) == 0) {
                /* ToDo: DoFireAnalysis(); */
            }
            
            /* Process disasters */
            if (DisasterEvent) {
                /* ToDo: ProcessDisaster(); */
            }
            break;
    }
}

void DoTimeStuff(void)
{
    /* Process time advancement */
    CityTime++;
    
    CityMonth++;
    if (CityMonth > 11) {
        CityMonth = 0;
        CityYear++;
        
        /* Make the simulation more dynamic - add some random valve changes */
        if (CityYear % 2 == 0) {
            int rDelta, cDelta, iDelta;
            
            rDelta = SimRandom(500) - 250;
            cDelta = SimRandom(500) - 250;
            iDelta = SimRandom(500) - 250;
            
            RValve += rDelta;
            CValve += cDelta;
            IValve += iDelta;
            
            /* Ensure valves stay in reasonable ranges */
            if (RValve < -1500) RValve = -1500;
            if (RValve > 1500) RValve = 1500;
            if (CValve < -1500) CValve = -1500;
            if (CValve > 1500) CValve = 1500;
            if (IValve < -1500) IValve = -1500;
            if (IValve > 1500) IValve = 1500;
        }
        
        /* Add some funds periodically to keep things interesting */
        TotalFunds += 1000;
    }
    
    /* Manage disasters */
    if (DisasterEvent) {
        DisasterWait = 0;
    } else {
        if (DisasterWait > 0) {
            DisasterWait--;
        } else {
            /* Check for random disasters when counter reaches zero */
            if (GameLevel > 0) {
                if (SimRandom(9 - GameLevel) == 0) {
                    /* Choose a disaster type */
                    switch (SimRandom(8)) {
                        case 0:
                        case 1:
                            /* ToDo: MakeFlood(); */
                            break;
                        case 2:
                        case 3:
                            /* ToDo: MakeFire(); */
                            break;
                        case 4:
                        case 5:
                            /* ToDo: MakeAirCrash(); */
                            break;
                        case 6:
                            /* ToDo: MakeTornado(); */
                            break;
                        case 7:
                            /* ToDo: MakeEarthquake(); */
                            break;
                    }
                }
            }
            /* Reset disaster wait period */
            DisasterWait = SimRandom(51) + 49;
        }
    }
}

void SetValves(int res, int com, int ind)
{
    /* Set the development rate valves */
    
    if (res < -2000) res = -2000;
    if (com < -2000) com = -2000;
    if (ind < -2000) ind = -2000;
    
    if (res > 2000) res = 2000;
    if (com > 2000) com = 2000;
    if (ind > 2000) ind = 2000;
    
    RValve = res;
    CValve = com;
    IValve = ind;
}

void ClearCensus(void)
{
    /* Reset census counts */
    ResPop = 0;
    ComPop = 0;
    IndPop = 0;
    RoadTotal = 0;
    RailTotal = 0;
    FirePop = 0;
    PolicePop = 0;
    StadiumPop = 0;
    PortPop = 0;
    APortPop = 0;
    NuclearPop = 0;
    PwrdZCnt = 0;
    UnpwrdZCnt = 0;
}

void TakeCensus(void)
{
    /* Store city statistics in the history arrays */
    int i;
    
    /* Calculate total population */
    TotalPop = (ResPop + ComPop + IndPop) * 8;
    
    if (TotalPop == 0 && CityTime > 10) {
        /* ToDo: DoShowPicture(POPULATIONLOST_BIT); */
    }
    
    /* Update graph history */
    for (i = 0; i < HISTLEN/2 - 1; i++) {
        ResHis[i] = ResHis[i + 1];
        ComHis[i] = ComHis[i + 1];
        IndHis[i] = IndHis[i + 1];
        CrimeHis[i] = CrimeHis[i + 1];
        PollutionHis[i] = PollutionHis[i + 1];
        MoneyHis[i] = MoneyHis[i + 1];
    }
    
    /* Update miscellaneous history */
    for (i = 0; i < MISCHISTLEN/2 - 1; i++) {
        MiscHis[i] = MiscHis[i + 1];
    }
    
    /* Record current values in history */
    ResHis[HISTLEN/2 - 1] = ResPop * 8;
    ComHis[HISTLEN/2 - 1] = ComPop * 8;
    IndHis[HISTLEN/2 - 1] = IndPop * 8;
    CrimeHis[HISTLEN/2 - 1] = CrimeAverage * 8;
    PollutionHis[HISTLEN/2 - 1] = PollutionAverage * 8;
    MoneyHis[HISTLEN/2 - 1] = (short)(TotalFunds / 100);
    
    /* Note: MiscHis will be updated in the specific subsystem implementations */
}

void MapScan(int x1, int x2, int y1, int y2)
{
    /* Scan a section of the map for zone processing */
    int x, y;
    
    if (x1 < 0 || x2 > WORLD_X || y1 < 0 || y2 > WORLD_Y)
        return;
        
    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            SMapX = x;
            SMapY = y;
            CChr = Map[y][x] & LOMASK;
            
            /* Process powered zones */
            if ((Map[y][x] & ZONEBIT) && ((Map[y][x] & POWERBIT) || CChr == 9))
            {
                DoZone(x, y, CChr);
            }
        }
    }
}

int GetPValue(int x, int y)
{
    /* Get power status at a given position */
    if (x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y) {
        return (Map[y][x] & POWERBIT) != 0;
    }
    return 0;
}

/* Check if coordinates are within map bounds */
int TestBounds(int x, int y)
{
    return (x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y);
}

/* Timer ID for simulation */
#define SIM_TIMER_ID 1

/* Timer interval in milliseconds */
#define SIM_TIMER_INTERVAL 50

/* External function declaration for the UI update */
extern void UpdateSimulationMenu(HWND hwnd, int speed);

/* Global timer ID to track between function calls */
static UINT SimTimerID = 0;

void SetSimulationSpeed(HWND hwnd, int speed)
{
    /* Update the simulation speed */
    SimSpeed = speed;
    
    /* Update UI (menu checkmarks) */
    UpdateSimulationMenu(hwnd, speed);
    
    /* If paused, stop the timer */
    if (speed == SPEED_PAUSED) {
        SimPaused = 1;
        if (SimTimerID) {
            KillTimer(hwnd, SIM_TIMER_ID);
            SimTimerID = 0;
        }
    } else {
        /* Otherwise, make sure the timer is running */
        SimPaused = 0;
        if (!SimTimerID) {
            SimTimerID = SetTimer(hwnd, SIM_TIMER_ID, SIM_TIMER_INTERVAL, NULL);
            
            /* Check for timer creation failure */
            if (!SimTimerID) {
                MessageBox(hwnd, "Failed to create simulation timer", "Error", 
                           MB_ICONERROR | MB_OK);
            }
        }
    }
}

/* Cleanup simulation timer when program exits */
void CleanupSimTimer(HWND hwnd)
{
    if (SimTimerID) {
        KillTimer(hwnd, SIM_TIMER_ID);
        SimTimerID = 0;
    }
}