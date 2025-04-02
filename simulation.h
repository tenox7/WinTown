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

/*
 * MICROPOLIS TILESET LAYOUT - 16×16 PIXEL TILES IN A GRID
 * -------------------------------------------------------
 * Tile numbers are arranged in a 16×60 grid (16 columns, 60 rows)
 *
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |  Row 0: Dirt, River
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |  Row 1: River edges, Trees
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |  Row 2: Trees, Woods, Rubble
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |  Row 3: Flood, Radiation, Fire
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 64 | 65 | 66 | 67 | 68 | 69 | 70 | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79 |  Row 4: Roads, Bridges, Crossings
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 80 | 81 | 82 | 83 | 84 | 85 | 86 | 87 | 88 | 89 | 90 | 91 | 92 | 93 | 94 | 95 |  Row 5: Light Traffic
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | 96 | .. | .. | .. |... |... |... |... |... |... |... |... |... |... |... |111 |  Row 6-13: Traffic, Bridges
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |208 |209 |210 |211 |212 |213 |214 |215 |216 |217 |218 |219 |220 |221 |222 |223 |  Row 13: Power Lines
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |224 |225 |226 |227 |228 |229 |230 |231 |232 |233 |234 |235 |236 |237 |238 |239 |  Row 14: Rail, Rail crossings
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |240 |241 |242 |243 |244 |245 |246 |247 |248 |249 |250 |251 |252 |253 |254 |255 |  Row 15: Residential zones
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |... |... |... |... |... |... |... |... |... |... |... |... |... |... |... |... |  Rows 16-58: Various zones
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |928 |929 |930 |931 |932 |933 |934 |935 |936 |937 |938 |939 |940 |941 |942 |943 |  Row 58: Stadium games
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |944 |945 |946 |947 |948 |949 |950 |951 |952 |953 |954 |955 |956 |957 |958 |959 |  Row 59: Bridge parts, Nuclear
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * Special crossing tiles:
 * - HROADPOWER (77): Horizontal road with vertical power line (╥)
 * - VROADPOWER (78): Vertical road with horizontal power line (╞)
 * - RAILHPOWERV (221): Horizontal rail with vertical power line
 * - RAILVPOWERH (222): Vertical rail with horizontal power line 
 * - HRAILROAD (237): Horizontal rail with vertical road
 * - VRAILROAD (238): Vertical rail with horizontal road
 */

/* Complete Tile Definitions from Original Micropolis */
#define DIRT            0       /* Dirt */
#define RIVER           2       /* River */
#define REDGE           3       /* River edge */
#define CHANNEL         4       /* Channel */
#define FIRSTRIVEDGE    5       /* First river edge */
#define LASTRIVEDGE     20      /* Last river edge */
#define TREEBASE        21      /* First tree */
#define LASTTREE        36      /* Last tree */
#define WOODS           37      /* Regular forest */
#define UNUSED_TRASH1   38      /* Unused trash */
#define UNUSED_TRASH2   39      /* Unused trash */
#define WOODS2          40      /* Forest type 2 */
#define WOODS3          41      /* Forest type 3 */
#define WOODS4          42      /* Forest type 4 */
#define WOODS5          43      /* Forest type 5 */
#define RUBBLE          44      /* Rubble (destroyed buildings) */
#define LASTRUBBLE      47      /* Last rubble tile */
#define FLOOD           48      /* Flood plain */
#define LASTFLOOD       51      /* Last flood plain */
#define RADTILE         52      /* Radiation */
#define UNUSED_TRASH3   53      /* Unused trash */
#define UNUSED_TRASH4   54      /* Unused trash */
#define UNUSED_TRASH5   55      /* Unused trash */
#define FIRE            56      /* Fire */
#define FIREBASE        56      /* First fire tile */
#define LASTFIRE        63      /* Last fire tile */
#define ROADBASE        64      /* First road tile */
#define HBRIDGE         64      /* Horizontal bridge */
#define VBRIDGE         65      /* Vertical bridge */
#define ROADS           66      /* Normal road */
#define INTERSECTION    76      /* Road intersection */
#define HROADPOWER      77      /* Horizontal road with power */
#define VROADPOWER      78      /* Vertical road with power */
#define BRWH            79      /* Bridge west to horizontal (empty) */
#define LTRFBASE        80      /* Light traffic base (roadbase + 16) */
#define BRWV            95      /* Bridge west to vertical (empty) */
#define BRWXXX1         111     /* Extra bridge piece */
#define BRWXXX2         127     /* Extra bridge piece */
#define BRWXXX3         143     /* Extra bridge piece */
#define HTRFBASE        144     /* Heavy traffic base */
#define BRWXXX4         159     /* Extra bridge piece */
#define BRWXXX5         175     /* Extra bridge piece */
#define BRWXXX6         191     /* Extra bridge piece */
#define LASTROAD        206     /* Last road tile */
#define BRWXXX7         207     /* Extra bridge piece */
#define POWERBASE       208     /* First power line */
#define HPOWER          208     /* Horizontal power */
#define VPOWER          209     /* Vertical power */
#define LHPOWER         210     /* Power line corner left-horizontal */
#define LVPOWER         211     /* Power line corner left-vertical */
#define LPOWER          211     /* L-shaped power (same as LVPOWER) */
#define RAILHPOWERV     221     /* Rail with power */
#define RAILVPOWERH     222     /* Rail with power */
#define LASTPOWER       222     /* Last power line */
#define UNUSED_TRASH6   223     /* Unused trash */
#define RAILBASE        224     /* First rail */
#define HRAIL           224     /* Horizontal rail */
#define VRAIL           225     /* Vertical rail */
#define LHRAIL          226     /* Rail corner left-horizontal */
#define LVRAIL          227     /* Rail corner left-vertical */
#define HRAILROAD       237     /* Horizontal rail with road */
#define VRAILROAD       238     /* Vertical rail with road */
#define LASTRAIL        238     /* Last rail */
#define ROAD_POWER_CROSS 239    /* Road and power line crossing */
#define RAIL_POWER_CROSS 240    /* Rail and power line crossing */
#define RESBASE         240     /* First residential zone */
#define FREEZ           244     /* Free fire zone */
#define HOUSE           249     /* First house */
#define LHTHR           249     /* Low-value housing threshold */
#define HHTHR           260     /* High-value housing threshold */
#define RZB             265     /* Residential base tile */
#define HOSPITAL        409     /* Hospital */
#define CHURCH          418     /* Church */
#define COMBASE         423     /* First commercial tile */
#define COMCLR          427     /* Commercial clear tile */
#define CZB             436     /* Commercial base tile */
#define INDBASE         612     /* First industrial tile */
#define INDCLR          616     /* Industrial clear tile */
#define LASTIND         620     /* Last original industrial tile */
#define IND1            621     /* Industrial type 1 */
#define IZB             625     /* Industrial base tile */
#define IND2            641     /* Industrial type 2 */
#define IND3            644     /* Industrial type 3 */
#define IND4            649     /* Industrial type 4 */
#define IND5            650     /* Industrial type 5 */
#define IND6            676     /* Industrial type 6 */
#define IND7            677     /* Industrial type 7 */
#define IND8            686     /* Industrial type 8 */
#define IND9            689     /* Industrial type 9 */
#define PORTBASE        693     /* First seaport */
#define PORT            698     /* Seaport */
#define LASTPORT        708     /* Last seaport */
#define AIRPORTBASE     709     /* First airport */
#define RADAR           711     /* Airport radar */
#define AIRPORT         716     /* Airport */
#define COALBASE        745     /* First coal power plant */
#define POWERPLANT      750     /* Coal power plant */
#define LASTPOWERPLANT  760     /* Last coal power plant */
#define FIRESTBASE      761     /* First fire station */
#define FIRESTATION     765     /* Fire station */
#define POLICESTBASE    770     /* First police station */
#define POLICESTATION   774     /* Police station */
#define STADIUMBASE     779     /* First stadium */
#define STADIUM         784     /* Stadium */
#define FULLSTADIUM     800     /* Stadium with people */
#define NUCLEARBASE     811     /* First nuclear plant */
#define NUCLEAR         816     /* Nuclear power plant */
#define LASTZONE        826     /* Last zone */
#define LIGHTNINGBOLT   827     /* No-power indicator */
#define HBRDG0          828     /* Horizontal bridge support */
#define HBRDG1          829     /* Horizontal bridge support */
#define HBRDG2          830     /* Horizontal bridge support */
#define HBRDG3          831     /* Horizontal bridge support */
#define RADAR0          832     /* Radar animation frame 0 */
#define RADAR1          833     /* Radar animation frame 1 */
#define RADAR2          834     /* Radar animation frame 2 */
#define RADAR3          835     /* Radar animation frame 3 */
#define RADAR4          836     /* Radar animation frame 4 */
#define RADAR5          837     /* Radar animation frame 5 */
#define RADAR6          838     /* Radar animation frame 6 */
#define RADAR7          839     /* Radar animation frame 7 */
#define FOUNTAIN        840     /* Fountain animation */
#define INDBASE2        844     /* More industrial buildings */
#define TELEBASE        844     /* Telecom buildings */
#define TELELAST        851     /* Last telecom building */
#define SMOKEBASE       852     /* First smoke animation frame */
#define TINYEXP         860     /* Small explosion */
#define SOMETINYEXP     864     /* Medium explosion */
#define LASTTINYEXP     867     /* Last explosion */
#define COALSMOKE1      916     /* Coal power plant smoke */
#define COALSMOKE2      920     /* Coal power plant smoke */
#define COALSMOKE3      924     /* Coal power plant smoke */
#define COALSMOKE4      928     /* Coal power plant smoke */
#define FOOTBALLGAME1   932     /* Football game */
#define FOOTBALLGAME2   940     /* Football game */
#define VBRDG0          948     /* Vertical bridge support */
#define VBRDG1          949     /* Vertical bridge support */
#define VBRDG2          950     /* Vertical bridge support */
#define VBRDG3          951     /* Vertical bridge support */
#define FOOTBALLBASE    950     /* Football stadium base - same value as VBRDG2 */

#define TILE_COUNT      960     /* Total number of tiles */

/* Other constants */
#define BSIZE           8       /* Building size? */
#define TILE_DIRT       0       /* Dirt tile (same as DIRT) */
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
void spreadFire(void);                   /* Check for and spread fires */
void makeMonster(void);                  /* Create a monster */
void makeExplosion(int x, int y);        /* Create an explosion */
void makeMeltdown(void);                 /* Create a nuclear meltdown */

/* File I/O functions (main.c) */
int loadFile(char *filename);    /* Load city file data */

/* Animation functions (animation.c) */
void AnimateTiles(void);            /* Process animations for the entire map */
void SetAnimationEnabled(int enabled);  /* Enable or disable animations */
int GetAnimationEnabled(void);      /* Get animation enabled status */
void SetSmoke(int x, int y);        /* Set smoke animation for coal plants */
void UpdateFire(int x, int y);      /* Update fire animations */
void UpdateNuclearPower(int x, int y);  /* Update nuclear power plant animations */
void UpdateAirportRadar(int x, int y);  /* Update airport radar animation */
void UpdateSpecialAnimations(void); /* Update all special animations */

#endif /* _SIMULATION_H */