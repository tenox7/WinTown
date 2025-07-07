/* newgame.h - New Game Dialog Interface for MicropolisNT
 * Based on original Micropolis scenarios and new game functionality
 */

#ifndef NEWGAME_H
#define NEWGAME_H

#include <windows.h>

/* Dialog resource IDs */
#define IDD_NEWGAME 100
#define IDD_SCENARIO_SELECT 101

/* Control IDs for New Game dialog */
#define IDC_NEW_CITY 1001
#define IDC_LOAD_CITY 1002
#define IDC_SCENARIO 1003
#define IDC_DIFFICULTY_EASY 1004
#define IDC_DIFFICULTY_MEDIUM 1005
#define IDC_DIFFICULTY_HARD 1006
#define IDC_CITY_NAME 1007
#define IDC_OK 1008
#define IDC_CANCEL 1009

/* Control IDs for Scenario Selection dialog */
#define IDC_SCENARIO_LIST 2001
#define IDC_SCENARIO_PREVIEW 2002
#define IDC_SCENARIO_DESC 2003
#define IDC_SCENARIO_OK 2004
#define IDC_SCENARIO_CANCEL 2005

/* New Game options */
#define NEWGAME_NEW_CITY 0
#define NEWGAME_LOAD_CITY 1
#define NEWGAME_SCENARIO 2

/* Difficulty levels */
#define DIFFICULTY_EASY 0
#define DIFFICULTY_MEDIUM 1
#define DIFFICULTY_HARD 2

/* Scenario definitions based on original Micropolis */
typedef struct {
    int id;
    char name[64];
    char description[512];
    char filename[32];
    int year;
    int funds;
} ScenarioInfo;

/* New game configuration */
typedef struct {
    int gameType;        /* NEWGAME_NEW_CITY, NEWGAME_LOAD_CITY, or NEWGAME_SCENARIO */
    int difficulty;      /* DIFFICULTY_EASY, DIFFICULTY_MEDIUM, or DIFFICULTY_HARD */
    int scenarioId;      /* 1-8 for scenarios, 0 for new city */
    char cityName[64];   /* City name */
    char loadFile[MAX_PATH]; /* File to load (for NEWGAME_LOAD_CITY) */
} NewGameConfig;

/* Function prototypes */
int showNewGameDialog(HWND parent, NewGameConfig *config);
int showScenarioDialog(HWND parent, int *selectedScenario);
BOOL CALLBACK newGameDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK scenarioDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* Initialize new game system */
int initNewGame(NewGameConfig *config);

/* Scenario management */
int getScenarioCount(void);
ScenarioInfo* getScenarioInfo(int index);
int loadScenarioById(int scenarioId);

/* New city generation */
int generateNewCity(char *cityName, int difficulty);

/* City loading */
int loadCityFile(char *filename);

#endif /* NEWGAME_H */
