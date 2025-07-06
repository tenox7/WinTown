/* scenarios.c - Scenario implementation for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include "sim.h"
#include "notifications.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "gdifix.h"

/* External log functions */
extern void addGameLog(const char *format, ...);
extern void addDebugLog(const char *format, ...);

/* External cheat flags */
extern int disastersDisabled;

/* Scenario variables */
short ScenarioID = 0;    /* Current scenario ID (0 = none) */
short DisasterEvent = 0; /* Current disaster type */
short DisasterWait = 0;  /* Countdown to next disaster */
short ScoreType = 0;     /* Score type for scenario */
short ScoreWait = 0;     /* Score wait for scenario */

/* External functions needed from other modules */
extern int SimRandom(int range);     /* From simulation.c */
extern int loadFile(char *filename); /* From main.c */

/* Disaster functions from disasters.c */
extern void doEarthquake(void);          /* Earthquake disaster */
extern void makeFlood(void);             /* Flooding disaster */
extern void makeFire(int x, int y);      /* Fire disaster at specific location */
extern void makeMonster(void);           /* Monster attack disaster */
extern void makeTornado(void);           /* Tornado disaster */
extern void makeExplosion(int x, int y); /* Explosion at specific location */
extern void makeMeltdown(void);          /* Nuclear meltdown disaster */

/* External functions from main.c */
extern void ForceFullCensus(void); /* Census calculation function */

/* External variables */
extern HWND hwndMain;
extern char cityFileName[MAX_PATH];

/* Disaster timing arrays - matches original Micropolis */
static short DisTab[9] = {0, 2, 10, 5, 20, 3, 5, 5, 2 * 48};
static short ScoreWaitTab[9] = {0,      30 * 48, 5 * 48, 5 * 48, 10 * 48,
                                5 * 48, 10 * 48, 5 * 48, 10 * 48};

/* Load scenario based on ID */
int loadScenario(int scenarioId) {
    char *name = NULL;
    char *fname = NULL;
    char path[MAX_PATH];
    int startYear = 1900;
    FILE *f;

    /* Reset city filename */
    cityFileName[0] = '\0';

    GameLevel = 0; /* Set game level to easy */

    /* Validate scenario ID range */
    if ((scenarioId < 1) || (scenarioId > 8)) {
        addGameLog("WARNING: Invalid scenario ID! Using Dullsville (1) instead.");
        scenarioId = 1;
    }

    /* Check if scenario files are available - construct filename based on ID */
    switch (scenarioId) {
    case 1:
        strcpy(path, "cities\\dullsville.scn");
        break;
    case 2:
        strcpy(path, "cities\\sanfrancisco.scn");
        break;
    case 3:
        strcpy(path, "cities\\hamburg.scn");
        break;
    case 4:
        strcpy(path, "cities\\bern.scn");
        break;
    case 5:
        strcpy(path, "cities\\tokyo.scn");
        break;
    case 6:
        strcpy(path, "cities\\detroit.scn");
        break;
    case 7:
        strcpy(path, "cities\\boston.scn");
        break;
    case 8:
        strcpy(path, "cities\\rio.scn");
        break;
    default:
        strcpy(path, "cities\\dullsville.scn");
        break;
    }

    f = fopen(path, "rb");
    if (f == NULL) {
        addGameLog("ERROR: Scenario files not found! Please copy scenario files to the cities directory.");
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

        /* Log the scenario load */
        addGameLog("SCENARIO: %s loaded", name);
        addGameLog("Year: %d, Initial funds: $%d", startYear, (int)TotalFunds);

        /* Add scenario-specific descriptions */
        switch (scenarioId) {
        case 1:
            addGameLog("Dullsville: A sleepy town with room to grow");
            addDebugLog("Scenario goal: Build a bigger city");
            break;
        case 2:
            addGameLog("San Francisco 1906: Earthquake-prone metropolis");
            addGameLog("WARNING: Earthquake disaster expected!");
            addDebugLog("Scenario goal: Recover from earthquake");
            break;
        case 3:
            addGameLog("Hamburg 1944: War-torn city requires rebuilding");
            addGameLog("WARNING: Expect fire bombing attacks!");
            addDebugLog("Scenario goal: Rebuild after fire bombing");
            break;
        case 4:
            addGameLog("Bern 1965: Beautiful Swiss capital with growing traffic problems");
            addGameLog("WARNING: Traffic congestion is becoming severe!");
            addDebugLog("Scenario goal: Keep traffic average below 80");
            break;
        case 5:
            addGameLog("Tokyo 1957: Dense Japanese metropolis");
            addGameLog("WARNING: Monster attack imminent!");
            addDebugLog("Scenario goal: Recover from monster disaster");
            break;
        case 6:
            addGameLog("Detroit 1972: Struggling with economic decline and high crime");
            addGameLog("WARNING: Crime levels are dangerously high!");
            addDebugLog("Scenario goal: Reduce crime average below 60");
            break;
        case 7:
            addGameLog("Boston 2010: Home to high-tech industry");
            addGameLog("WARNING: Nuclear accident risk detected!");
            addDebugLog("Scenario goal: Manage nuclear disaster");
            break;
        case 8:
            addGameLog("Rio 2047: Coastal city threatened by flooding");
            addGameLog("WARNING: Flood risk is high!");
            addDebugLog("Scenario goal: Handle rising water levels");
            break;
        }

        addDebugLog("Scenario ID: %d, Starting population ~%d", scenarioId, (int)CityPop);
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
        addGameLog("ERROR: Failed to load scenario file: %s", path);
        return 0;
    }

    /* CRITICAL: Set the flag to prevent ClearCensus from erasing population */
    SkipCensusReset = 1;

    /* Set simulation to medium speed */
    SimSpeed = SPEED_MEDIUM;
    SimPaused = 0;

    /* IMPORTANT: SKIP DoSimInit() for scenarios, as it would reset population!
       Instead, do a directed initialization that preserves scenario values */

    /* Initialize various simulation subsystems */
    /* These are the essential parts of DoSimInit() */

    /* Set up random number generator */
    RandomlySeedRand();

    /* Initialize simulation counters */
    Scycle = 0;
    Fcycle = 0;
    Spdcycle = 0;

    /* Set higher growth demand to encourage population increase */
    SetValves(900, 800, 700);
    ValveFlag = 1;

    /* Initialize budget system without resetting population */
    InitBudget();

    /* Note: DisasterWait is already set correctly from DisTab[] array above */
    addDebugLog("Scenario %d: Disaster wait period = %d", ScenarioID, DisasterWait);
    addDebugLog("Disaster system: Event=%d, Wait=%d, Disabled=%d", DisasterEvent, DisasterWait, disastersDisabled);

    /* CRITICAL: Add initial population even before census to avoid zero values */
    {
        int x, y;
        short tileValue;
        int resCount = 0, comCount = 0, indCount = 0;

        /* Count residential, commercial, and industrial zones */
        for (y = 0; y < WORLD_Y; y++) {
            for (x = 0; x < WORLD_X; x++) {
                tileValue = Map[y][x] & LOMASK;
                if (Map[y][x] & ZONEBIT) {
                    if (tileValue >= RESBASE && tileValue <= LASTRES) {
                        resCount++;
                    } else if (tileValue >= COMBASE && tileValue <= LASTCOM) {
                        comCount++;
                    } else if (tileValue >= INDBASE && tileValue <= LASTIND) {
                        indCount++;
                    }
                }
            }
        }

        /* Set higher initial population based on zones for better gameplay */
        if (resCount > 0) {
            ResPop = resCount * 16; /* Doubled from 8 */
        }
        if (comCount > 0) {
            ComPop = comCount * 8; /* Doubled from 4 */
        }
        if (indCount > 0) {
            IndPop = indCount * 8; /* Doubled from 4 */
        }

        /* Initialize total population using unified function */
        TotalPop = CalculateTotalPopulation(ResPop, ComPop, IndPop);

        /* Initialize city population - scenarios use higher multiplier for challenge */
        CityPop = CalculateCityPopulation(ResPop, ComPop, IndPop);
        if (CityPop > 0) {
            CityPop = (CityPop * 25) / 20; /* 25% boost for scenarios */
        }

        /* Initialize class based on population */
        CityClass = 0; /* Village */
        if (CityPop > 2000) {
            CityClass++; /* Town */
        }
        if (CityPop > 10000) {
            CityClass++; /* City */
        }
        if (CityPop > 50000) {
            CityClass++; /* Capital */
        }
        if (CityPop > 100000) {
            CityClass++; /* Metropolis */
        }
        if (CityPop > 500000) {
            CityClass++; /* Megalopolis */
        }

        /* Ensure we have some population */
        if (CityPop < 100) {
            ResPop = 10;
            TotalPop = ResPop * 8;
            CityPop = ResPop * 20;
        }

        /* Save this value for debugging */
        PrevCityPop = CityPop;
    }

    /* Force a full census calculation */
    /* This is critical for correctly calculating population */
    ForceFullCensus();

    /* After census, explicitly update history graphs */
    TakeCensus();

    /* Make sure population is preserved */
    if (CityPop == 0 && PrevCityPop > 0) {
        /* Restore from previous value if we lost it */
        CityPop = PrevCityPop;
    }
    
    /* Set scenario-specific initial conditions */
    switch (ScenarioID) {
        case 4: /* Bern - High initial traffic */
            TrafficAverage = 120; /* Start with high traffic */
            addDebugLog("Bern scenario: Initial traffic set to %d", TrafficAverage);
            break;
        case 6: /* Detroit - High initial crime */
            CrimeAverage = 100; /* Start with high crime */
            addDebugLog("Detroit scenario: Initial crime set to %d", CrimeAverage);
            break;
        case 7: /* Boston - Low initial land value (pre-meltdown) */
            LVAverage = 80; /* Start with lower land values */
            addDebugLog("Boston scenario: Initial land value set to %d", LVAverage);
            break;
    }

    /* We no longer need to change SkipCensusReset flag as we've modified ClearCensus
       to always reset population counters which allows them to be properly recounted
       during each map scan. This enables growth while preventing population from disappearing. */
    /* Note: SkipCensusReset is now only used for debugging purposes */

    /* Redraw screen */
    InvalidateRect(hwndMain, NULL, TRUE);

    return 1;
}

/* Process scenario disasters */
void scenarioDisaster(void) {
    static int disasterX, disasterY;
    static int debugCounter = 0;

    /* Add periodic debug logging */
    debugCounter++;
    if (debugCounter % 50 == 0) { /* Log every 50 calls */
        addDebugLog("scenarioDisaster called: Event=%d, Wait=%d, Disabled=%d", 
                   DisasterEvent, DisasterWait, disastersDisabled);
    }

    /* Early return if no disaster or disasters are disabled */
    if (DisasterEvent == 0 || disastersDisabled) {
        return;
    }

    /* Ensure DisasterEvent is in valid range */
    if (DisasterEvent < 1 || DisasterEvent > 8) {
        DisasterEvent = 0;
        return;
    }

    switch (DisasterEvent) {
    case 1: /* Dullsville */
        break;
    case 2: /* San Francisco */
        if (DisasterWait <= 1) {
            addGameLog("SCENARIO EVENT: San Francisco earthquake is happening now!");
            addGameLog("Significant damage reported throughout the city!");
            doEarthquake();
        } else {
            /* Debug logging for earthquake countdown */
            static int lastReported = -1;
            if (DisasterWait != lastReported) {
                addDebugLog("San Francisco earthquake countdown: %d", DisasterWait);
                lastReported = DisasterWait;
            }
        }
        break;
    case 3: /* Hamburg */
        /* Drop fire bombs */
        if (DisasterWait % 10 == 0) {
            disasterX = SimRandom(WORLD_X);
            disasterY = SimRandom(WORLD_Y);
            if (DisasterWait == 20) {
                addGameLog("SCENARIO EVENT: Hamburg firebombing attack has begun!");
                addGameLog("Multiple fires are breaking out across the city!");
            }
            addGameLog("Explosion reported at %d,%d", disasterX, disasterY);
            addDebugLog("Firebomb at coordinates %d,%d, %d bombs remaining", disasterX, disasterY,
                        DisasterWait / 10);
            makeExplosion(disasterX, disasterY);
        }
        break;
    case 4: /* Bern */
        /* No disaster in Bern scenario */
        break;
    case 5: /* Tokyo */
        if (DisasterWait <= 1) {
            addGameLog("SCENARIO EVENT: Tokyo monster attack is underway!");
            addGameLog("Giant creature is destroying buildings in its path!");
            makeMonster();
        }
        break;
    case 6: /* Detroit */
        /* Detroit gets tornado to simulate urban decay */
        if (DisasterWait <= 1) {
            addGameLog("SCENARIO EVENT: Detroit tornado has touched down!");
            addGameLog("Severe weather compounds the city's problems!");
            makeTornado();
        }
        break;
    case 7: /* Boston */
        if (DisasterWait <= 1) {
            addGameLog("SCENARIO EVENT: Boston nuclear meltdown is happening!");
            addGameLog("Nuclear power plant has suffered a catastrophic failure!");
            makeMeltdown();
        }
        break;
    case 8: /* Rio */
        if ((DisasterWait % 24) == 0) {
            if (DisasterWait == 48) {
                addGameLog("SCENARIO EVENT: Rio flood disaster is starting!");
                addGameLog("Ocean levels are rising - coastal areas at risk!");
            } else {
                addGameLog("Flood waters continue to spread!");
            }
            addDebugLog("Flood event triggered - %d hours until flood peak", DisasterWait);
            makeFlood();
        }
        break;
    default:
        /* Invalid disaster event */
        DisasterEvent = 0;
        return;
    }

    if (DisasterWait) {
        DisasterWait--;

        /* Log when disaster ends */
        if (DisasterWait == 0) {
            addGameLog("Scenario disaster has ended");
            addDebugLog("Scenario disaster ID %d complete", DisasterEvent);
            DisasterEvent = 0;
        }
    } else {
        DisasterEvent = 0;
    }
}

/* Evaluate scenario goals and determine win/lose status */
void DoScenarioScore(void) {
    int win = 0;
    int score = 0;
    char message[256];
    
    /* Only evaluate if we have an active scenario and score wait has expired */
    if (ScenarioID == 0 || ScoreWait > 0) {
        return;
    }
    
    /* Check victory conditions based on scenario */
    switch (ScenarioID) {
        case 1: /* Dullsville - reach City Class 4 or higher */
            if (CityClass >= 4) {
                win = 1;
                score = 500;
                strcpy(message, "Congratulations! You've transformed Dullsville into a thriving metropolis!");
            } else {
                strcpy(message, "You failed to develop Dullsville into a major city. Try building more zones!");
            }
            break;
            
        case 2: /* San Francisco - reach City Class 4 or higher after earthquake */
            if (CityClass >= 4) {
                win = 1;
                score = 500;
                strcpy(message, "Amazing! You've rebuilt San Francisco after the devastating earthquake!");
            } else {
                strcpy(message, "San Francisco remains in ruins. Focus on rebuilding residential and commercial areas!");
            }
            break;
            
        case 3: /* Hamburg - reach City Class 4 or higher after bombing */
            if (CityClass >= 4) {
                win = 1;
                score = 500;
                strcpy(message, "Excellent! Hamburg has risen from the ashes of war!");
            } else {
                strcpy(message, "Hamburg couldn't recover from the bombing. Try faster reconstruction!");
            }
            break;
            
        case 4: /* Bern - keep traffic average below 80 */
            if (TrafficAverage < 80) {
                win = 1;
                score = 500;
                strcpy(message, "Perfect! You've solved Bern's traffic problems with excellent planning!");
            } else {
                strcpy(message, "Traffic congestion remains a problem in Bern. Build more roads and rails!");
            }
            break;
            
        case 5: /* Tokyo - achieve City Score above 500 after monster attack */
            if (CityScore > 500) {
                win = 1;
                score = 500;
                strcpy(message, "Incredible! Tokyo has recovered and thrived after the monster attack!");
            } else {
                strcpy(message, "Tokyo couldn't fully recover from the monster attack. Focus on rebuilding!");
            }
            break;
            
        case 6: /* Detroit - reduce crime average below 60 */
            if (CrimeAverage < 60) {
                win = 1;
                score = 500;
                strcpy(message, "Outstanding! You've cleaned up Detroit and made it safe again!");
            } else {
                strcpy(message, "Crime remains too high in Detroit. Build more police stations!");
            }
            break;
            
        case 7: /* Boston - restore land value average above 120 after meltdown */
            if (LVAverage > 120) {
                win = 1;
                score = 500;
                strcpy(message, "Remarkable! Boston has recovered from the nuclear disaster!");
            } else {
                strcpy(message, "Land values remain low after the meltdown. Clean up radiation and rebuild!");
            }
            break;
            
        case 8: /* Rio - achieve City Score above 500 despite flooding */
            if (CityScore > 500) {
                win = 1;
                score = 500;
                strcpy(message, "Brilliant! Rio thrives despite the coastal flooding challenges!");
            } else {
                strcpy(message, "Rio couldn't overcome the flooding problems. Try building away from water!");
            }
            break;
            
        default:
            /* Unknown scenario */
            return;
    }
    
    /* Log the result */
    if (win) {
        addGameLog("SCENARIO SUCCESS: %s", message);
        addGameLog("Final Score: %d", score);
        CityScore = score;
    } else {
        addGameLog("SCENARIO FAILED: %s", message);
        addGameLog("Final Score: -200");
        CityScore = -200;
    }
    
    /* Show notification dialog with result */
    {
        Notification notif;
        notif.id = win ? 7001 : 7002; /* Custom scenario result IDs */
        notif.type = win ? NOTIF_MILESTONE : NOTIF_WARNING;
        strcpy(notif.title, win ? "Scenario Complete - YOU WIN!" : "Scenario Failed");
        strcpy(notif.message, message);
        strcpy(notif.explanation, win ? "Congratulations! You have successfully completed this scenario challenge." : "The scenario objectives were not met. Review your strategy and try again.");
        strcpy(notif.advice, win ? "You can try other scenarios or continue playing in sandbox mode." : "Focus on the specific requirements mentioned in the scenario description.");
        notif.hasLocation = 0;
        notif.priority = win ? 1 : 2;
        notif.timestamp = GetTickCount();
        CreateNotificationDialog(&notif);
    }
    
    /* Reset scenario */
    ScenarioID = 0;
    ScoreWait = 0;
    DisasterEvent = 0;
    DisasterWait = 0;
}
