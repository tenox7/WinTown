/* scenarios.c - Scenario implementation for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "simulation.h"

/* Scenario variables */
short ScenarioID = 0;        /* Current scenario ID (0 = none) */
short DisasterEvent = 0;     /* Current disaster type */
short DisasterWait = 0;      /* Countdown to next disaster */
short ScoreType = 0;         /* Score type for scenario */
short ScoreWait = 0;         /* Score wait for scenario */

/* External functions needed from other modules */
extern int SimRandom(int range);          /* From simulation.c */
extern int loadFile(char *filename);      /* From main.c */

/* Disaster functions from disasters.c */
extern void doEarthquake(void);           /* Earthquake disaster */
extern void makeFlood(void);              /* Flooding disaster */
extern void makeFire(int x, int y);       /* Fire disaster at specific location */
extern void makeMonster(void);            /* Monster attack disaster */
extern void makeExplosion(int x, int y);  /* Explosion at specific location */
extern void makeMeltdown(void);           /* Nuclear meltdown disaster */

/* External functions from main.c */
extern void ForceFullCensus(void);        /* Census calculation function */

/* External variables */
extern HWND hwndMain;
extern char cityFileName[MAX_PATH];

/* Disaster timing arrays */
static short DisTab[9] = { 0, 2, 10, 5, 20, 3, 5, 5, 2 * 48};
static short ScoreWaitTab[9] = { 0, 30 * 48, 5 * 48, 5 * 48, 10 * 48,
                   5 * 48, 10 * 48, 5 * 48, 10 * 48 };

/* Load scenario based on ID */
int loadScenario(int scenarioId)
{
    char *name = NULL;
    char *fname = NULL;
    char path[MAX_PATH];
    int startYear = 1900;
    FILE *f;
    
    /* Reset city filename */
    cityFileName[0] = '\0';
    
    GameLevel = 0;  /* Set game level to easy */
    
    /* Validate scenario ID range */
    if ((scenarioId < 1) || (scenarioId > 8)) {
        MessageBox(hwndMain, "Invalid scenario ID! Using Dullsville (1) instead.", 
                  "Warning", MB_ICONWARNING | MB_OK);
        scenarioId = 1;
    }
    
    /* Check if scenario files are available - construct filename based on ID */
    switch (scenarioId) {
        case 1: strcpy(path, "cities\\dullsville.scn"); break;
        case 2: strcpy(path, "cities\\sanfrancisco.scn"); break;
        case 3: strcpy(path, "cities\\hamburg.scn"); break;
        case 4: strcpy(path, "cities\\bern.scn"); break;
        case 5: strcpy(path, "cities\\tokyo.scn"); break;
        case 6: strcpy(path, "cities\\detroit.scn"); break;
        case 7: strcpy(path, "cities\\boston.scn"); break;
        case 8: strcpy(path, "cities\\rio.scn"); break;
        default: strcpy(path, "cities\\dullsville.scn"); break;
    }
    
    f = fopen(path, "rb");
    if (f == NULL) {
        MessageBox(hwndMain, "Scenario files not found!\nPlease copy scenario files to the cities directory.", 
                  "Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    fclose(f);
    
    switch (scenarioId) {
    case 1:
        name = "Dullsville";
        fname = "dullsville.scn";
        ScenarioID = 1;
        startYear = 1900;
        TotalFunds = 5000;
        break;
    case 2:
        name = "San Francisco";
        fname = "sanfrancisco.scn";
        ScenarioID = 2;
        startYear = 1906;
        TotalFunds = 20000;
        break;
    case 3:
        name = "Hamburg";
        fname = "hamburg.scn";
        ScenarioID = 3;
        startYear = 1944;
        TotalFunds = 20000;
        break;
    case 4:
        name = "Bern";
        fname = "bern.scn";
        ScenarioID = 4;
        startYear = 1965;
        TotalFunds = 20000;
        break;
    case 5:
        name = "Tokyo";
        fname = "tokyo.scn";
        ScenarioID = 5;
        startYear = 1957;
        TotalFunds = 20000;
        break;
    case 6:
        name = "Detroit";
        fname = "detroit.scn";
        ScenarioID = 6;
        startYear = 1972;
        TotalFunds = 20000;
        break;
    case 7:
        name = "Boston";
        fname = "boston.scn";
        ScenarioID = 7;
        startYear = 2010;
        TotalFunds = 20000;
        break;
    case 8:
        name = "Rio de Janeiro";
        fname = "rio.scn";
        ScenarioID = 8;
        startYear = 2047;
        TotalFunds = 20000;
        break;
    }
    
    /* Set up scenario initial conditions */
    strcpy(cityFileName, name);
    CityYear = startYear;
    CityMonth = 0;
    
    /* Update window title with scenario name */
    {
        char winTitle[256];
        wsprintf(winTitle, "MicropolisNT - Scenario: %s", name);
        SetWindowText(hwndMain, winTitle);
    }
    
    /* Set disaster info */
    DisasterEvent = ScenarioID;
    /* Ensure bounds checking before array access */
    if (DisasterEvent >= 0 && DisasterEvent < 9) {
        DisasterWait = DisTab[DisasterEvent];
        ScoreWait = ScoreWaitTab[DisasterEvent];
    } else {
        DisasterWait = 0;
        ScoreWait = 0;
    }
    ScoreType = ScenarioID;
    
    /* Load scenario file */
    wsprintf(path, "cities\\%s", fname);
    if (!loadFile(path)) {
        MessageBox(hwndMain, "Failed to load scenario file", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    /* Set speed to medium and init sim */
    SimSpeed = SPEED_MEDIUM;
    DoSimInit();
    
    /* Force population census calculation for the loaded scenario */
    ForceFullCensus();
    
    /* Redraw screen */
    InvalidateRect(hwndMain, NULL, TRUE);
    
    return 1;
}

/* Process scenario disasters */
void scenarioDisaster(void)
{
    static int disasterX, disasterY;
    
    /* Early return if no disaster or waiting period is over */
    if (DisasterEvent == 0 || DisasterWait == 0) {
        return;
    }
    
    /* Ensure DisasterEvent is in valid range */
    if (DisasterEvent < 1 || DisasterEvent > 8) {
        DisasterEvent = 0;
        return;
    }
    
    switch (DisasterEvent) {
        case 1:            /* Dullsville */
            break;
        case 2:            /* San Francisco */
            if (DisasterWait == 1) doEarthquake();
            break;
        case 3:            /* Hamburg */
            /* Drop fire bombs */
            if (DisasterWait % 10 == 0) {
                disasterX = SimRandom(WORLD_X);
                disasterY = SimRandom(WORLD_Y);
                makeExplosion(disasterX, disasterY);
            }
            break;
        case 4:            /* Bern */
            break;
        case 5:            /* Tokyo */
            if (DisasterWait == 1) makeMonster();
            break;
        case 6:            /* Detroit */
            break;
        case 7:            /* Boston */
            if (DisasterWait == 1) makeMeltdown();
            break;
        case 8:            /* Rio */
            if ((DisasterWait % 24) == 0) makeFlood();
            break;
        default:
            /* Invalid disaster event */
            DisasterEvent = 0;
            return;
    }
    
    if (DisasterWait) DisasterWait--;
    else DisasterEvent = 0;
}