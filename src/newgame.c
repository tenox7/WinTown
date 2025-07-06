/* newgame.c - New Game Dialog Implementation for MicropolisNT
 * Based on original Micropolis scenarios and new game functionality
 */

#include "newgame.h"
#include "sim.h"
#include "tiles.h"
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions */
extern void addGameLog(const char *format, ...);
extern void addDebugLog(const char *format, ...);
extern int loadFile(char *filename);
extern int loadScenario(int scenarioId);

/* External simulation functions */
extern void DoSimInit(void);
extern void SetValves(int res, int com, int ind);

/* External variables */
extern long TotalFunds;
extern int ResPop;
extern int ComPop;
extern int IndPop;
extern int TotalPop;
extern QUAD CityPop;
extern int CityClass;
extern int LastTotalPop;
extern int GameLevel;

/* Scenario data based on original Micropolis */
static ScenarioInfo scenarios[] = {
    {1, "Dullsville", "Practice your city planning skills on this sleepy little town. Slow and simple.", "dullsville.scn", 1900, 5000},
    {2, "San Francisco", "Can you rebuild after the great earthquake of 1906?", "sanfrancisco.scn", 1906, 20000},
    {3, "Hamburg", "Rebuild this industrial city after the bombing of 1944.", "hamburg.scn", 1944, 20000},
    {4, "Bern", "The capital of Switzerland needs a major update.", "bern.scn", 1965, 20000},
    {5, "Tokyo", "A monster is attacking! Can you stop it and rebuild?", "tokyo.scn", 1957, 20000},
    {6, "Detroit", "Fight crime and rebuild the Motor City.", "detroit.scn", 1972, 20000},
    {7, "Boston", "Nuclear meltdown! Handle the crisis and rebuild.", "boston.scn", 2010, 20000},
    {8, "Rio de Janeiro", "Coastal flooding threatens this Brazilian city.", "rio.scn", 2047, 20000}
};

static int scenarioCount = 8;
static NewGameConfig *currentConfig = NULL;

/* Show the main new game dialog */
int showNewGameDialog(HWND parent, NewGameConfig *config) {
    int result;
    HINSTANCE hInst;
    
    if (!config) {
        addGameLog("ERROR: showNewGameDialog called with NULL config");
        return 0;
    }
    
    /* Initialize default values */
    config->gameType = NEWGAME_NEW_CITY;
    config->difficulty = DIFFICULTY_MEDIUM;
    config->scenarioId = 0;
    strcpy(config->cityName, "New City");
    config->loadFile[0] = '\0';
    
    currentConfig = config;
    
    hInst = GetModuleHandle(NULL);
    addGameLog("About to call DialogBox with resource ID %d, hInst=0x%p", IDD_NEWGAME, hInst);
    
    result = DialogBox(hInst, MAKEINTRESOURCE(IDD_NEWGAME), parent, newGameDialogProc);
    
    addGameLog("DialogBox returned: %d", result);
    if (result == -1) {
        DWORD error = GetLastError();
        addGameLog("ERROR: DialogBox failed with error %lu", error);
        currentConfig = NULL;
        return 0;
    }
    
    currentConfig = NULL;
    return result;
}

/* Show scenario selection dialog */
int showScenarioDialog(HWND parent, int *selectedScenario) {
    int result;
    
    if (!selectedScenario) {
        return 0;
    }
    
    result = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SCENARIO_SELECT), 
                           parent, scenarioDialogProc, (LPARAM)selectedScenario);
    
    return result;
}

/* Main new game dialog procedure */
BOOL CALLBACK newGameDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        HWND listBox;
        int i;
        
        addGameLog("New game dialog WM_INITDIALOG - dialog is being created");
        /* Set default selections */
        CheckRadioButton(hwnd, IDC_NEW_CITY, IDC_SCENARIO, IDC_NEW_CITY);
        CheckRadioButton(hwnd, IDC_DIFFICULTY_EASY, IDC_DIFFICULTY_HARD, IDC_DIFFICULTY_MEDIUM);
        SetDlgItemText(hwnd, IDC_CITY_NAME, "New City");
        
        /* Populate scenario list */
        listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
        /* CRITICAL: Clear any existing items first to prevent corruption */
        SendMessage(listBox, LB_RESETCONTENT, 0, 0);
        addGameLog("DEBUG: Cleared and populating NEW GAME scenario listbox with %d scenarios", scenarioCount);
        for (i = 0; i < scenarioCount; i++) {
            int addResult = SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)scenarios[i].name);
            int dataResult = SendMessage(listBox, LB_SETITEMDATA, i, i);
            addGameLog("DEBUG: Adding scenario %d: ID=%d, Name='%s' (add=%d, data=%d)", 
                      i, scenarios[i].id, scenarios[i].name, addResult, dataResult);
        }
        
        /* Select first scenario by default */
        SendMessage(listBox, LB_SETCURSEL, 0, 0);
        SetDlgItemText(hwnd, IDC_SCENARIO_DESC, scenarios[0].description);
        
        /* Hide scenario controls initially since New City is selected */
        ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_LIST), SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_DESC), SW_HIDE);
        
        /* Enable/disable controls based on selection */
        EnableWindow(GetDlgItem(hwnd, IDC_CITY_NAME), TRUE);
        addGameLog("New game dialog initialized with defaults");
        return TRUE;
    }
    
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_NEW_CITY:
            if (HIWORD(wParam) == BN_CLICKED) {
                EnableWindow(GetDlgItem(hwnd, IDC_CITY_NAME), TRUE);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_LIST), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_DESC), SW_HIDE);
                if (currentConfig) {
                    currentConfig->gameType = NEWGAME_NEW_CITY;
                }
            }
            break;
            
        case IDC_LOAD_CITY:
            if (HIWORD(wParam) == BN_CLICKED) {
                EnableWindow(GetDlgItem(hwnd, IDC_CITY_NAME), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_LIST), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_DESC), SW_HIDE);
                if (currentConfig) {
                    currentConfig->gameType = NEWGAME_LOAD_CITY;
                }
            }
            break;
            
        case IDC_SCENARIO:
            if (HIWORD(wParam) == BN_CLICKED) {
                EnableWindow(GetDlgItem(hwnd, IDC_CITY_NAME), FALSE);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_LIST), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd, IDC_SCENARIO_DESC), SW_SHOW);
                if (currentConfig) {
                    currentConfig->gameType = NEWGAME_SCENARIO;
                }
            }
            break;
            
        case IDC_DIFFICULTY_EASY:
            if (HIWORD(wParam) == BN_CLICKED && currentConfig) {
                currentConfig->difficulty = DIFFICULTY_EASY;
            }
            break;
            
        case IDC_DIFFICULTY_MEDIUM:
            if (HIWORD(wParam) == BN_CLICKED && currentConfig) {
                currentConfig->difficulty = DIFFICULTY_MEDIUM;
            }
            break;
            
        case IDC_DIFFICULTY_HARD:
            if (HIWORD(wParam) == BN_CLICKED && currentConfig) {
                currentConfig->difficulty = DIFFICULTY_HARD;
            }
            break;
            
        case IDC_SCENARIO_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                HWND listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
                int sel = SendMessage(listBox, LB_GETCURSEL, 0, 0);
                int itemData = SendMessage(listBox, LB_GETITEMDATA, sel, 0);
                
                addGameLog("DEBUG: Listbox selection changed - sel=%d, itemData=%d", sel, itemData);
                if (sel != LB_ERR && sel >= 0 && sel < scenarioCount) {
                    addGameLog("New game dialog: Selected scenario %d (%s): %s", 
                              sel, scenarios[sel].name, scenarios[sel].description);
                    SetDlgItemText(hwnd, IDC_SCENARIO_DESC, scenarios[sel].description);
                } else {
                    addGameLog("DEBUG: Invalid selection - sel=%d, scenarioCount=%d", sel, scenarioCount);
                }
            }
            break;
            
        case IDC_OK: {
            addGameLog("User clicked OK in new game dialog");
            if (!currentConfig) {
                EndDialog(hwnd, 0);
                return TRUE;
            }
            
            /* Get city name */
            GetDlgItemText(hwnd, IDC_CITY_NAME, currentConfig->cityName, 63);
            addGameLog("City name entered: '%s'", currentConfig->cityName);
            
            /* Handle different game types */
            switch (currentConfig->gameType) {
            case NEWGAME_NEW_CITY:
                if (strlen(currentConfig->cityName) == 0) {
                    MessageBox(hwnd, "Please enter a city name.", "Error", MB_OK | MB_ICONERROR);
                    return TRUE;
                }
                break;
                
            case NEWGAME_LOAD_CITY: {
                OPENFILENAME ofn;
                char fileName[MAX_PATH] = "";
                
                /* Setup file dialog */
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = fileName;
                ofn.nMaxFile = sizeof(fileName);
                ofn.lpstrFilter = "City Files (*.cty)\0*.cty\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrTitle = "Load City";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                
                if (GetOpenFileName(&ofn)) {
                    strcpy(currentConfig->loadFile, fileName);
                } else {
                    return TRUE; /* User cancelled */
                }
                break;
            }
            
            case NEWGAME_SCENARIO: {
                HWND listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
                int sel = SendMessage(listBox, LB_GETCURSEL, 0, 0);
                
                if (sel != LB_ERR && sel >= 0 && sel < scenarioCount) {
                    currentConfig->scenarioId = scenarios[sel].id;
                    addGameLog("New game dialog: Starting scenario %d (%s)", 
                              scenarios[sel].id, scenarios[sel].name);
                } else {
                    MessageBox(hwnd, "Please select a scenario.", "Error", MB_OK | MB_ICONERROR);
                    return TRUE;
                }
                break;
            }
            }
            
            addGameLog("New game dialog completed successfully");
            EndDialog(hwnd, 1);
            return TRUE;
        }
        
        case IDC_CANCEL:
            addGameLog("User clicked Cancel in new game dialog");
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;
    }
    
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    }
    
    return FALSE;
}

/* Scenario selection dialog procedure */
BOOL CALLBACK scenarioDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int *selectedScenario = NULL;
    
    switch (msg) {
    case WM_INITDIALOG: {
        HWND listBox;
        int i;
        
        selectedScenario = (int*)lParam;
        
        /* Populate scenario list */
        listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
        for (i = 0; i < scenarioCount; i++) {
            SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)scenarios[i].name);
            SendMessage(listBox, LB_SETITEMDATA, i, i);
        }
        
        /* Select first scenario by default */
        SendMessage(listBox, LB_SETCURSEL, 0, 0);
        
        /* Update description */
        SetDlgItemText(hwnd, IDC_SCENARIO_DESC, scenarios[0].description);
        
        return TRUE;
    }
    
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_SCENARIO_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                HWND listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
                int sel = SendMessage(listBox, LB_GETCURSEL, 0, 0);
                
                if (sel != LB_ERR && sel >= 0 && sel < scenarioCount) {
                    addGameLog("Scenario dialog: Selected scenario %d (%s): %s", 
                              sel, scenarios[sel].name, scenarios[sel].description);
                    SetDlgItemText(hwnd, IDC_SCENARIO_DESC, scenarios[sel].description);
                }
            } else if (HIWORD(wParam) == LBN_DBLCLK) {
                /* Double-click selects and closes dialog */
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_SCENARIO_OK, BN_CLICKED), 0);
            }
            break;
            
        case IDC_SCENARIO_OK: {
            HWND listBox = GetDlgItem(hwnd, IDC_SCENARIO_LIST);
            int sel = SendMessage(listBox, LB_GETCURSEL, 0, 0);
            
            if (sel != LB_ERR && sel >= 0 && sel < scenarioCount && selectedScenario) {
                *selectedScenario = scenarios[sel].id;
                addGameLog("Scenario dialog: Selected scenario %d (%s) for loading", 
                          scenarios[sel].id, scenarios[sel].name);
                EndDialog(hwnd, 1);
            }
            return TRUE;
        }
        
        case IDC_SCENARIO_CANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;
    }
    
    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    }
    
    return FALSE;
}

/* Initialize new game based on configuration */
int initNewGame(NewGameConfig *config) {
    if (!config) {
        return 0;
    }
    
    switch (config->gameType) {
    case NEWGAME_NEW_CITY:
        return generateNewCity(config->cityName, config->difficulty);
        
    case NEWGAME_LOAD_CITY:
        return loadCityFile(config->loadFile);
        
    case NEWGAME_SCENARIO:
        return loadScenarioById(config->scenarioId);
        
    default:
        addGameLog("ERROR: Unknown game type in new game configuration.");
        return 0;
    }
}

/* Clear map for new city */
static void clearMapForNewCity(void) {
    int x, y;
    
    addGameLog("Clearing map for new city");
    
    /* Clear the main map - set all tiles to empty dirt */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            setMapTile(x, y, DIRT, 0, TILE_SET_REPLACE, "clearMapForNewCity");
        }
    }
    
    /* Reset all population values to 0 for new city */
    ResPop = 0;
    ComPop = 0;
    IndPop = 0;
    TotalPop = 0;
    CityPop = 0;
    CityClass = 0;
    LastTotalPop = 0;
}

/* Generate a new city */
int generateNewCity(char *cityName, int difficulty) {
    if (!cityName) {
        return 0;
    }
    
    addGameLog("Generating new city: %s", cityName);
    
    /* Set difficulty level */
    GameLevel = difficulty;
    
    /* Set starting funds based on difficulty */
    switch (difficulty) {
    case DIFFICULTY_EASY:
        TotalFunds = 20000;
        break;
    case DIFFICULTY_MEDIUM:
        TotalFunds = 10000;
        break;
    case DIFFICULTY_HARD:
        TotalFunds = 5000;
        break;
    default:
        TotalFunds = 10000;
        break;
    }
    
    /* Clear the map completely for a fresh start */
    clearMapForNewCity();
    
    /* Initialize simulation (but preserve the funds we just set) */
    DoSimInit();
    
    /* Restore the funds since DoSimInit overwrites them */
    switch (difficulty) {
    case DIFFICULTY_EASY:
        TotalFunds = 20000;
        break;
    case DIFFICULTY_MEDIUM:
        TotalFunds = 10000;
        break;
    case DIFFICULTY_HARD:
        TotalFunds = 5000;
        break;
    default:
        TotalFunds = 10000;
        break;
    }
    
    /* Set demand valves to initial values */
    SetValves(500, 300, 100);
    
    addGameLog("New city '%s' generated successfully. Difficulty: %s, Funds: $%ld", 
              cityName, 
              (difficulty == DIFFICULTY_EASY) ? "Easy" : 
              (difficulty == DIFFICULTY_MEDIUM) ? "Medium" : "Hard",
              TotalFunds);
    
    return 1;
}

/* Load city from file */
int loadCityFile(char *filename) {
    if (!filename || strlen(filename) == 0) {
        return 0;
    }
    
    addGameLog("Loading city from file: %s", filename);
    
    if (loadFile(filename)) {
        addGameLog("City loaded successfully from %s", filename);
        return 1;
    } else {
        addGameLog("ERROR: Failed to load city from %s", filename);
        return 0;
    }
}

/* Get scenario count */
int getScenarioCount(void) {
    return scenarioCount;
}

/* Get scenario info by index */
ScenarioInfo* getScenarioInfo(int index) {
    if (index < 0 || index >= scenarioCount) {
        return NULL;
    }
    return &scenarios[index];
}

/* Load scenario by ID */
int loadScenarioById(int scenarioId) {
    if (scenarioId < 1 || scenarioId > scenarioCount) {
        addGameLog("ERROR: Invalid scenario ID: %d", scenarioId);
        return 0;
    }
    
    addGameLog("Loading scenario: %s", scenarios[scenarioId - 1].name);
    
    if (loadScenario(scenarioId)) {
        addGameLog("Scenario '%s' loaded successfully", scenarios[scenarioId - 1].name);
        return 1;
    } else {
        addGameLog("ERROR: Failed to load scenario '%s'", scenarios[scenarioId - 1].name);
        return 0;
    }
}