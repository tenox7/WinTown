/* simulation.h - Core simulation structures and functions for MicropolisNT
 * Based on original Micropolis code from MicropolisLegacy project
 */

#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <windows.h>

/* External declaration for UpdateToolbar function */
extern void UpdateToolbar(void);

/* Basic type definitions */
typedef unsigned char Byte;
typedef long QUAD;

/* Constants */
#define WORLD_X         120
#define WORLD_Y         100
#define WORLD_W         WORLD_X
#define WORLD_H         WORLD_Y

#define SmX             (WORLD_X >> 1)
#define SmY             (WORLD_Y >> 1)

/* Game levels */
#define LEVEL_EASY      0
#define LEVEL_MEDIUM    1
#define LEVEL_HARD      2

/* Maximum size for arrays */
#define HISTLEN         480
#define MISCHISTLEN     240

/* Masks for the various tile properties */
#define LOMASK          0x03ff  /* Mask for the low 10 bits = 1023 decimal */
#define ANIMBIT         0x0800  /* bit 11, tile is animated */
#define BURNBIT         0x2000  /* bit 13, tile can be lit */
#define BULLBIT         0x1000  /* bit 12, tile is bulldozable */
#define CONDBIT         0x4000  /* bit 14, tile can conduct electricity */
#define ZONEBIT         0x0400  /* bit 10, tile is the center of a zone */
#define POWERBIT        0x8000  /* bit 15, tile has power */
#define MASKBITS        (~LOMASK)  /* Mask for just the bits */
#define BNCNBIT         0x0400  /* Center bit */
#define ALLBITS         0xFFFF  /* All bits */

/* Zone Types */
#define HOSPITALBASE    400     /* Hospital */
#define HOSPITAL        401     /* Hospital */
#define CHURCH          402     /* Church */
#define FOOTBALLBASE    950     /* Football stadium */

/* Special Zone Types */
#define POWERPLANT      750     /* Coal power plant */
#define NUCLEAR         816     /* Nuclear power plant */
#define STADIUM         785     /* Stadium */
#define ROADS           64      /* Roads */
#define ROADBASE        64      /* First road tile */
#define LASTROAD        206     /* Last road tile */
#define POWERBASE       208     /* First power line tile */
#define RAILBASE        224     /* First rail tile */
#define LASTRAIL        238     /* Last rail tile */
#define BSIZE           8       /* Building size? */
#define LHTHR           249     /* Light house threshold? */
#define HHTHR           260     /* Heavy house threshold? */
#define LASTIND         692     /* Last industrial zone type */
#define PORT            698     /* Seaport */
#define PORTBASE        693     /* Seaport base */
#define LASTPORT        708     /* Last seaport */
#define AIRPORT         716     /* Airport */
#define RADTILE         52      /* Radiation tile */
#define FIREBASE        56      /* Fire base tile */
#define HTRFBASE        80      /* Heavy traffic base (roadbase + 16) */
#define LTRFBASE        64      /* Light traffic base (same as roadbase) */
#define RUBBLE          44      /* Rubble tile */
#define POLICESTATION   774     /* Police station */
#define FIRESTATION     765     /* Fire station */
#define HPOWER          208     /* Horizontal power */
#define LPOWER          211     /* L-shaped power */
#define DIRT            0       /* Dirt */
#define TILE_DIRT       0       /* Dirt tile */
#define PWRBIT          0x8000  /* Power bit (same as POWERBIT) */

/* Zone base and limit values */
#define RESBASE         240     /* Start of residential zones */
#define LASTRES         420     /* End of residential zones */
#define COMBASE         423     /* Start of commercial zones */
#define LASTCOM         611     /* End of commercial zones */
#define INDBASE         612     /* Start of industrial zones */
#define LIGHTNINGBOLT   827     /* No-power indicator */

/* Game simulation rate constants */
#define SPEEDCYCLE      1024  /* The number of cycles before the speed counter loops from 0-1023 */
#define CENSUSRATE      4     /* Census update rate (once per 4 passes) */
#define TAXFREQ         48    /* Tax assessment frequency (once per 48 passes) */
#define VALVEFREQ       16    /* Valve adjustment frequency (once every 16 passes) */

/* Tool states */
#define residentialState    0
#define commercialState     1
#define industrialState     2
#define fireState           3
#define policeState         4
#define wireState           5
#define roadState           6
#define railState           7
#define parkState           8
#define stadiumState        9
#define seaportState       10
#define powerState         11
#define nuclearState       12
#define airportState       13
#define networkState       14
#define bulldozerState     15
#define queryState         16
#define windowState        17
#define noToolState        18

/* Simulation speed settings */
#define SPEED_PAUSED     0
#define SPEED_SLOW       1
#define SPEED_MEDIUM     2
#define SPEED_FAST       3

/* Structures */
extern short Map[WORLD_Y][WORLD_X];      /* The main map */
extern Byte PopDensity[WORLD_Y/2][WORLD_X/2]; /* Population density map (half size) */
extern Byte TrfDensity[WORLD_Y/2][WORLD_X/2]; /* Traffic density map (half size) */
extern Byte PollutionMem[WORLD_Y/2][WORLD_X/2]; /* Pollution density map (half size) */
extern Byte LandValueMem[WORLD_Y/2][WORLD_X/2]; /* Land value map (half size) */
extern Byte CrimeMem[WORLD_Y/2][WORLD_X/2];   /* Crime map (half size) */
extern short PowerMap[WORLD_Y][WORLD_X];  /* Power connectivity map */

/* Quarter-sized maps for effects */
extern Byte TerrainMem[WORLD_Y/4][WORLD_X/4];  /* Terrain memory (quarter size) */
extern Byte FireStMap[WORLD_Y/4][WORLD_X/4];   /* Fire station map (quarter size) */
extern Byte FireRate[WORLD_Y/4][WORLD_X/4];    /* Fire coverage rate (quarter size) */
extern Byte PoliceMap[WORLD_Y/4][WORLD_X/4];   /* Police station map (quarter size) */
extern Byte PoliceMapEffect[WORLD_Y/4][WORLD_X/4]; /* Police station effect (quarter size) */

/* Commercial development score */
extern short ComRate[WORLD_Y/4][WORLD_X/4];    /* Commercial score (quarter size) */

/* Historical data for graphs */
extern short ResHis[HISTLEN/2];      /* Residential history */
extern short ComHis[HISTLEN/2];      /* Commercial history */
extern short IndHis[HISTLEN/2];      /* Industrial history */
extern short CrimeHis[HISTLEN/2];    /* Crime history */
extern short PollutionHis[HISTLEN/2]; /* Pollution history */
extern short MoneyHis[HISTLEN/2];    /* Cash flow history */
extern short MiscHis[MISCHISTLEN/2]; /* Miscellaneous history */

/* Runtime simulation state */
extern int SimSpeed;     /* 0=pause, 1=slow, 2=med, 3=fast */
extern int SimSpeedMeta; /* Counter for adjusting sim speed, 0-3 */
extern int SimPaused;    /* 1 if paused, 0 otherwise */
extern int CityTime;     /* City time from 0 to ~32 depending on scenario */
extern int CityYear;     /* City year from 1900 onwards */
extern int CityMonth;    /* City month from Jan to Dec */
extern QUAD TotalFunds;  /* City operating funds */
extern int TaxRate;      /* City tax rate 0-20 */
extern int SkipCensusReset; /* Flag to skip census reset after loading a scenario */
extern int DebugCensusReset; /* Debug counter for tracking census resets */
extern int PrevResPop;      /* Debug tracker for last residential population value */
extern int PrevCityPop;     /* Debug tracker for last city population value */

/* Counters */
extern int Scycle;       /* Simulation cycle counter (0-1023) */
extern int Fcycle;       /* Frame counter (0-1023) */
extern int Spdcycle;     /* Speed cycle counter (0-1023) */

/* Game evaluation */
extern int CityYes;        /* Positive sentiment votes */
extern int CityNo;         /* Negative sentiment votes */
extern QUAD CityPop;       /* Population assessment */
extern int CityScore;      /* City score */
extern int deltaCityScore; /* Score change */
extern int CityClass;      /* City class (village, town, city, etc.) */
extern int CityLevel;      /* Mayor level (0-5, 0 is worst) */
extern int CityLevelPop;   /* Population threshold for level */
extern int GameLevel;      /* Game level (0=easy, 1=medium, 2=hard) */
extern int ResCap;         /* Residential capacity reached */
extern int ComCap;         /* Commercial capacity reached */
extern int IndCap;         /* Industrial capacity reached */

/* City statistics */
extern int ResPop;       /* Residential population */
extern int ComPop;       /* Commercial population */
extern int IndPop;       /* Industrial population */
extern int TotalPop;     /* Total population */
extern int LastTotalPop; /* Previous total population */
extern float Delta;      /* Population change coefficient */

/* Infrastructure counts */
extern int PwrdZCnt;     /* Number of powered zones */
extern int UnpwrdZCnt;   /* Number of unpowered zones */
extern int RoadTotal;    /* Number of road tiles */
extern int RailTotal;    /* Number of rail tiles */
extern int FirePop;      /* Number of fire station zones */
extern int PolicePop;    /* Number of police station zones */
extern int StadiumPop;   /* Number of stadium tiles */
extern int PortPop;      /* Number of seaport tiles */
extern int APortPop;     /* Number of airport tiles */
extern int NuclearPop;   /* Number of nuclear plant tiles */

/* External effects */
extern int RoadEffect;   /* Road maintenance effectiveness (a function of funding) */
extern int PoliceEffect; /* Police effectiveness */
extern int FireEffect;   /* Fire department effectiveness */
extern int TrafficAverage; /* Average traffic */
extern int PollutionAverage; /* Average pollution */
extern int CrimeAverage; /* Average crime */
extern int LVAverage;    /* Average land value */

/* Growth rates */
extern short RValve;     /* Residential development rate */
extern short CValve;     /* Commercial development rate */
extern short IValve;     /* Industrial development rate */
extern int ValveFlag;    /* Set to 1 when valves change */

/* Disasters */
extern short DisasterEvent; /* Current disaster type (0=none) - defined in scenarios.c */
extern short DisasterWait;  /* Countdown to next disaster - defined in scenarios.c */
extern int DisasterLevel;   /* Disaster level */

/* Core simulation functions */
void DoSimInit(void);
void SimFrame(void);
void Simulate(int mod16);
void DoTimeStuff(void);
void SetValves(int res, int com, int ind);
void ClearCensus(void);
void TakeCensus(void);
void MapScan(int x1, int x2, int y1, int y2);
int GetPValue(int x, int y);
int TestBounds(int x, int y);
void SetSimulationSpeed(HWND hwnd, int speed);
void CleanupSimTimer(HWND hwnd);

/* Functions implemented in zone.c */
void DoZone(int Xloc, int Yloc, int pos);
int calcResPop(int zone);   /* Calculate residential zone population */
int calcComPop(int zone);   /* Calculate commercial zone population */
int calcIndPop(int zone);   /* Calculate industrial zone population */

/* Power-related variables and functions - power.c */
extern int SMapX;             /* Current map X position for power scan */
extern int SMapY;             /* Current map Y position for power scan */
void CountPowerPlants(void);
void QueuePowerPlant(int x, int y);
void FindPowerPlants(void);
void DoPowerScan(void);

/* Traffic-related functions - traffic.c */
int MakeTraffic(int zoneType);
void DecTrafficMap(void);
void CalcTrafficAverage(void);
void RandomlySeedRand(void); /* Initialize random number generator */
int SimRandom(int range);  /* Random number function used by traffic system */

/* Scanner-related functions - scanner.c */
void FireAnalysis(void);    /* Fire station effect analysis */
void PopDenScan(void);      /* Population density scan */
void PTLScan(void);         /* Pollution/terrain/land value scan */
void CrimeScan(void);       /* Crime level scan */

/* Evaluation-related functions - evaluation.c */
void EvalInit(void);           /* Initialize evaluation system */
void CityEvaluation(void);     /* Perform city evaluation */
void CountSpecialTiles(void);  /* Count special building types */
const char* GetProblemText(int problemIndex);  /* Get problem description */
const char* GetCityClassName(void);            /* Get city class name */
void GetTopProblems(short problems[4]);        /* Get top problems list */
int GetProblemVotes(int problemIndex);         /* Get problem vote count */
QUAD GetCityAssessedValue(void);               /* Get city assessed value */
int IsEvaluationValid(void);                   /* Is evaluation data valid */
int GetAverageCityScore(void);                 /* Get average city score */

/* Budget-related variables and functions - budget.c */
extern float RoadPercent;      /* Road funding percentage (0.0-1.0) */
extern float PolicePercent;    /* Police funding percentage (0.0-1.0) */
extern float FirePercent;      /* Fire funding percentage (0.0-1.0) */
extern QUAD RoadFund;          /* Required road funding amount */
extern QUAD PoliceFund;        /* Required police funding amount */
extern QUAD FireFund;          /* Required fire funding amount */
extern QUAD RoadSpend;         /* Actual road spending */
extern QUAD PoliceSpend;       /* Actual police spending */
extern QUAD FireSpend;         /* Actual fire spending */
extern QUAD TaxFund;           /* Tax income for current year */
extern int AutoBudget;         /* Auto-budget enabled flag */

void InitBudget(void);         /* Initialize budget system */
void CollectTax(void);         /* Calculate and collect taxes */
void Spend(QUAD amount);       /* Spend funds (negative = income) */
void DoBudget(void);           /* Process budget allocation */
QUAD GetTaxIncome(void);       /* Get current tax income */
QUAD GetBudgetBalance(void);   /* Get current budget balance */
int GetRoadEffect(void);       /* Get road effectiveness */
int GetPoliceEffect(void);     /* Get police effectiveness */
int GetFireEffect(void);       /* Get fire department effectiveness */
void SetRoadPercent(float percent);      /* Set road funding percentage */
void SetPolicePercent(float percent);    /* Set police funding percentage */
void SetFirePercent(float percent);      /* Set fire department funding percentage */

/* Scenario functions (scenarios.c) */
int loadScenario(int scenarioId);        /* Load a scenario by ID */
void scenarioDisaster(void);             /* Process scenario disasters */

/* Disaster functions (disasters.c) */
void doEarthquake(void);                 /* Create an earthquake */
void makeFlood(void);                    /* Create a flood */
void makeFire(int x, int y);             /* Start a fire */
void makeMonster(void);                  /* Create a monster */
void makeExplosion(int x, int y);        /* Create an explosion */
void makeMeltdown(void);                 /* Create a nuclear meltdown */

/* File I/O functions (main.c) */
int loadFile(char *filename);    /* Load city file data */

#endif /* _SIMULATION_H */