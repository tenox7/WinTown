/* Main entry point for MicropolisNT (Windows NT version)
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulation.h"

#define IDM_FILE_OPEN       1001
#define IDM_FILE_EXIT       1002
#define IDM_TILESET_BASE    2000
#define IDM_TILESET_MAX     2100
#define IDM_SIM_PAUSE       3001
#define IDM_SIM_SLOW        3002
#define IDM_SIM_MEDIUM      3003
#define IDM_SIM_FAST        3004

/* Define needed for older Windows SDK compatibility */
#ifndef LR_CREATEDIBSECTION
#define LR_CREATEDIBSECTION 0x2000
#endif

#ifndef LR_DEFAULTCOLOR
#define LR_DEFAULTCOLOR 0
#endif

#ifndef LR_DEFAULTSIZE
#define LR_DEFAULTSIZE 0x0040
#endif

#ifndef LR_LOADFROMFILE
#define LR_LOADFROMFILE 0x0010
#endif

#ifndef IMAGE_BITMAP
#define IMAGE_BITMAP 0
#endif

#define TILE_SIZE 16

/* Main map and history arrays - now defined in simulation.h/c */
short Map[WORLD_Y][WORLD_X];
short ResHis[HISTLEN/2];
short ComHis[HISTLEN/2];
short IndHis[HISTLEN/2];
short CrimeHis[HISTLEN/2];
short PollutionHis[HISTLEN/2];
short MoneyHis[HISTLEN/2];
short MiscHis[MISCHISTLEN/2];

static HWND hwndMain = NULL;
static HBITMAP hbmBuffer = NULL;
static HDC hdcBuffer = NULL;
static HBITMAP hbmTiles = NULL;
static HDC hdcTiles = NULL;
static HPALETTE hPalette = NULL;

static int cxClient = 0;
static int cyClient = 0;
static int xOffset = 0;
static int yOffset = 0;

static BOOL isMouseDown = FALSE;
static int lastMouseX = 0;
static int lastMouseY = 0;

static char cityFileName[MAX_PATH];
static HMENU hMenu = NULL;
static HMENU hFileMenu = NULL;
static HMENU hTilesetMenu = NULL;
static HMENU hSimMenu = NULL;
static char currentTileset[MAX_PATH] = "classic";

/* Micropolis tile flags - These must match simulation.h */
/* Using LOMASK from simulation.h */
/* Use the constants from simulation.h for consistency */

/* Tile type constants */
#define TILE_DIRT            0
#define TILE_RIVER           2
#define TILE_REDGE           3
#define TILE_CHANNEL         4
#define TILE_FIRSTRIVEDGE    5
#define TILE_LASTRIVEDGE    20
#define TILE_WATER_LOW       TILE_RIVER
#define TILE_WATER_HIGH      TILE_LASTRIVEDGE

#define TILE_TREEBASE       21
#define TILE_WOODS_LOW      TILE_TREEBASE
#define TILE_LASTTREE       36
#define TILE_WOODS          37
#define TILE_UNUSED_TRASH1  38
#define TILE_UNUSED_TRASH2  39
#define TILE_WOODS_HIGH     TILE_UNUSED_TRASH2
#define TILE_WOODS2         40
#define TILE_WOODS3         41
#define TILE_WOODS4         42
#define TILE_WOODS5         43

#define TILE_RUBBLE         44
#define TILE_LASTRUBBLE     47

#define TILE_FLOOD          48
#define TILE_LASTFLOOD      51

#define TILE_RADTILE        52

#define TILE_FIRE           56
#define TILE_FIREBASE       TILE_FIRE
#define TILE_LASTFIRE       63

#define TILE_ROADBASE       64
#define TILE_HBRIDGE        64
#define TILE_VBRIDGE        65
#define TILE_ROADS          66
#define TILE_INTERSECTION   76
#define TILE_HROADPOWER     77
#define TILE_VROADPOWER     78
#define TILE_LASTROAD      206

#define TILE_POWERBASE     208
#define TILE_HPOWER        208
#define TILE_VPOWER        209
#define TILE_LHPOWER       210
#define TILE_LVPOWER       211
#define TILE_LASTPOWER     222

#define TILE_RAILBASE      224
#define TILE_HRAIL         224
#define TILE_VRAIL         225
#define TILE_LASTRAIL      238

#define TILE_RESBASE       240
#define TILE_RESCLR        244
#define TILE_HOUSE         249
#define TILE_LASTRES       420

#define TILE_COMBASE       423
#define TILE_COMCLR        427
#define TILE_LASTCOM       611

/* Tileset constants - commented out because they're defined elsewhere 
   If you need to modify these, update the definitions where they are first defined
#define TILE_TOTAL_COUNT   1024  Maximum number of tiles in the tileset 
#define TILES_IN_ROW       32    Number of tiles per row in the tileset bitmap */
#ifndef TILE_TOTAL_COUNT
#define TILE_TOTAL_COUNT   1024 /* Maximum number of tiles in the tileset */
#endif
#ifndef TILES_IN_ROW
#define TILES_IN_ROW       32   /* Number of tiles per row in the tileset bitmap */
#endif

#define TILE_INDBASE       612
#define TILE_INDCLR        616
#define TILE_LASTIND       692

#define TILE_PORTBASE      693
#define TILE_PORT          698
#define TILE_LASTPORT      708

#define TILE_AIRPORTBASE   709
#define TILE_AIRPORT       716
#define TILE_LASTAIRPORT   744

#define TILE_COALBASE      745
#define TILE_POWERPLANT    750
#define TILE_LASTPOWERPLANT 760

#define TILE_FIRESTBASE    761
#define TILE_FIRESTATION   765
#define TILE_LASTFIRESTATION 769

#define TILE_POLICESTBASE   770
#define TILE_POLICESTATION  774
#define TILE_LASTPOLICESTATION 778

#define TILE_STADIUMBASE    779
#define TILE_STADIUM        784
#define TILE_LASTSTADIUM    799

#define TILE_NUCLEARBASE    811
#define TILE_NUCLEAR        816
#define TILE_LASTNUCLEAR    826

#define TILE_TOTAL_COUNT    960

#define TILES_IN_ROW        32
#define TILES_PER_ROW       32
LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
void initializeGraphics(HWND hwnd);
void cleanupGraphics(void);
int loadCity(char *filename);
void drawCity(HDC hdc);
void drawTile(HDC hdc, int x, int y, short tileValue);
int getBaseFromTile(short tile);
void swapShorts(short *buf, int len);
void resizeBuffer(int cx, int cy);
void scrollView(int dx, int dy);
void openCityDialog(HWND hwnd);
HBITMAP loadBitmapFile(const char* filename);
int loadTileset(const char* filename);
HPALETTE createSystemPalette(void);
HMENU createMainMenu(void);
void populateTilesetMenu(HMENU hSubMenu);
int changeTileset(HWND hwnd, const char* tilesetName);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    MSG msg;
    
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
    wc.lpfnWndProc   = wndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "MicropolisNT";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    hMenu = createMainMenu();
    
    hwndMain = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "MicropolisNT",
        "MicropolisNT - Tileset: classic",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        800, 600,
        NULL, hMenu, hInstance, NULL);
    
    if(hwndMain == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    initializeGraphics(hwndMain);
    cityFileName[0] = '\0';
    
    {
        int x, y;
        for (y = 0; y < WORLD_Y; y++)
        {
            for (x = 0; x < WORLD_X; x++)
            {
                Map[y][x] = TILE_DIRT;
            }
        }
    }
    
    /* Initialize simulation */
    DoSimInit();
    
    /* Start simulation at medium speed */
    SetSimulationSpeed(hwndMain, SPEED_MEDIUM);
    
    /* Set some initial demand values so we can see activity */
    RValve = 500;
    CValve = 300; 
    IValve = 100;
    
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    cleanupGraphics();
    
    return msg.wParam;
}


/* These constants should match those in simulation.c */
#define SIM_TIMER_ID 1
#define SIM_TIMER_INTERVAL 50

/* Update menu when simulation speed changes */
void UpdateSimulationMenu(HWND hwnd, int speed)
{
    /* Update menu checkmarks */
    CheckMenuRadioItem(hSimMenu, IDM_SIM_PAUSE, IDM_SIM_FAST, 
                      IDM_SIM_PAUSE + speed, MF_BYCOMMAND);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            CheckMenuRadioItem(hTilesetMenu, 0, GetMenuItemCount(hTilesetMenu)-1, 0, MF_BYPOSITION);
            /* Initialize simulation */
            DoSimInit();
            return 0;
        
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_FILE_OPEN:
                    openCityDialog(hwnd);
                    return 0;
                    
                case IDM_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    return 0;
                    
                case IDM_SIM_PAUSE:
                    SetSimulationSpeed(hwnd, SPEED_PAUSED);
                    return 0;
                    
                case IDM_SIM_SLOW:
                    SetSimulationSpeed(hwnd, SPEED_SLOW);
                    return 0;
                    
                case IDM_SIM_MEDIUM:
                    SetSimulationSpeed(hwnd, SPEED_MEDIUM);
                    return 0;
                    
                case IDM_SIM_FAST:
                    SetSimulationSpeed(hwnd, SPEED_FAST);
                    return 0;
                    
                default:
                    if (LOWORD(wParam) >= IDM_TILESET_BASE && LOWORD(wParam) < IDM_TILESET_MAX)
                    {
                        int index;
                        char tilesetName[MAX_PATH];
                        
                        index = LOWORD(wParam) - IDM_TILESET_BASE;
                        
                        /* Compatible with Windows NT 4.0 */
                        GetMenuString(hTilesetMenu, LOWORD(wParam), tilesetName, MAX_PATH - 1, MF_BYCOMMAND);
                        
                        if (changeTileset(hwnd, tilesetName))
                        {
                            CheckMenuRadioItem(hTilesetMenu, 0, GetMenuItemCount(hTilesetMenu)-1, 
                                            index, MF_BYPOSITION);
                        }
                        return 0;
                    }
            }
            break;
            
        case WM_TIMER:
            if (wParam == SIM_TIMER_ID) {
                BOOL needRedraw;
                
                /* Run the simulation frame */
                SimFrame();
                
                /* Always redraw to handle animations, but skip if paused */
                needRedraw = TRUE;
                
                /* Update the display */
                if (needRedraw) {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;
            }
            break;
            
        case WM_QUERYNEWPALETTE:
        {
            /* Realize the palette when window gets focus */
            if (hPalette != NULL)
            {
                HDC hdc = GetDC(hwnd);
                SelectPalette(hdc, hPalette, FALSE);
                RealizePalette(hdc);
                InvalidateRect(hwnd, NULL, FALSE);
                ReleaseDC(hwnd, hdc);
                return TRUE;
            }
            return FALSE;
        }
        
        case WM_PALETTECHANGED:
        {
            /* Realize palette if it was changed by another window */
            if ((HWND)wParam != hwnd && hPalette != NULL)
            {
                HDC hdc = GetDC(hwnd);
                SelectPalette(hdc, hPalette, FALSE);
                RealizePalette(hdc);
                UpdateColors(hdc);
                ReleaseDC(hwnd, hdc);
            }
            return 0;
        }
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            
            hdc = BeginPaint(hwnd, &ps);
            
            if (hPalette)
            {
                SelectPalette(hdc, hPalette, FALSE);
                RealizePalette(hdc);
            }
            
            if (hbmBuffer)
            {
                drawCity(hdcBuffer);
                BitBlt(hdc, 0, 0, cxClient, cyClient, 
                       hdcBuffer, 0, 0, SRCCOPY);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        {
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            
            isMouseDown = TRUE;
            lastMouseX = xPos;
            lastMouseY = yPos;
            SetCapture(hwnd);
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
            
            return 0;
        }
        
        case WM_MOUSEMOVE:
        {
            if (isMouseDown)
            {
                int xPos = LOWORD(lParam);
                int yPos = HIWORD(lParam);
                int dx = lastMouseX - xPos;
                int dy = lastMouseY - yPos;
                
                lastMouseX = xPos;
                lastMouseY = yPos;
                
                if (dx != 0 || dy != 0)
                {
                    scrollView(dx, dy);
                }
                
                SetCursor(LoadCursor(NULL, IDC_SIZEALL));
            }
            else
            {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            return 0;
        }
        
        case WM_LBUTTONUP:
        {
            isMouseDown = FALSE;
            ReleaseCapture();
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 0;
        }
        
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                if (isMouseDown)
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                else
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            break;
        }
        
        case WM_SIZE:
        {
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            resizeBuffer(cxClient, cyClient);
            return 0;
        }
        
        case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case VK_LEFT:
                    scrollView(-TILE_SIZE, 0);
                    break;
                    
                case VK_RIGHT:
                    scrollView(TILE_SIZE, 0);
                    break;
                    
                case VK_UP:
                    scrollView(0, -TILE_SIZE);
                    break;
                    
                case VK_DOWN:
                    scrollView(0, TILE_SIZE);
                    break;
                    
                case 'O':
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        openCityDialog(hwnd);
                    }
                    break;
                    
                case 'Q':
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    }
                    break;
                    
                case 'I':
                    if (hbmTiles)
                    {
                        BITMAP bm;
                        char debugMsg[256];
                        
                        if (GetObject(hbmTiles, sizeof(BITMAP), &bm))
                        {
                            wsprintf(debugMsg, "Bitmap Info:\nDimensions: %dx%d\nBits/pixel: %d",
                                     bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
                            MessageBox(hwnd, debugMsg, "Bitmap Information", MB_OK);
                        }
                    }
                    break;
            }
            return 0;
        }
        
        case WM_DESTROY:
            CleanupSimTimer(hwnd);
            cleanupGraphics();
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void swapShorts(short *buf, int len)
{
    int i;
    
    for (i = 0; i < len; i++)
    {
        buf[i] = ((buf[i] & 0xFF) << 8) | ((buf[i] & 0xFF00) >> 8);
    }
}

HPALETTE createSystemPalette(void)
{
    LOGPALETTE *pLogPal;
    HPALETTE hPal;
    int i;
    PALETTEENTRY colorTable[16] = {
        {0, 0, 0, 0},          /* Black */
        {128, 0, 0, 0},        /* Dark Red */
        {0, 128, 0, 0},        /* Dark Green */
        {128, 128, 0, 0},      /* Dark Yellow */
        {0, 0, 128, 0},        /* Dark Blue */
        {128, 0, 128, 0},      /* Dark Magenta */
        {0, 128, 128, 0},      /* Dark Cyan */
        {192, 192, 192, 0},    /* Light Gray */
        {128, 128, 128, 0},    /* Dark Gray */
        {255, 0, 0, 0},        /* Red */
        {0, 255, 0, 0},        /* Green */
        {255, 255, 0, 0},      /* Yellow */
        {0, 0, 255, 0},        /* Blue */
        {255, 0, 255, 0},      /* Magenta */
        {0, 255, 255, 0},      /* Cyan */
        {255, 255, 255, 0}     /* White */
    };
    
    pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 15 * sizeof(PALETTEENTRY));
    if (!pLogPal)
        return NULL;
    
    pLogPal->palVersion = 0x300; /* Windows 3.0 */
    pLogPal->palNumEntries = 16; /* 16 colors (4-bit) */
    
    for (i = 0; i < 16; i++)
    {
        pLogPal->palPalEntry[i] = colorTable[i];
    }
    
    hPal = CreatePalette(pLogPal);
    free(pLogPal);
    
    return hPal;
}

void initializeGraphics(HWND hwnd)
{
    HDC hdc;
    RECT rect;
    char tilePath[MAX_PATH];
    int width;
    int height;
    BITMAPINFOHEADER bi;
    LPVOID bits;
    HBITMAP hbmOld;
    
    hdc = GetDC(hwnd);
    
    if (hPalette == NULL)
    {
        hPalette = createSystemPalette();
        
        if (hPalette)
        {
            SelectPalette(hdc, hPalette, FALSE);
            RealizePalette(hdc);
        }
    }
    
    hdcBuffer = CreateCompatibleDC(hdc);
    
    if (hPalette)
    {
        SelectPalette(hdcBuffer, hPalette, FALSE);
        RealizePalette(hdcBuffer);
    }
    
    GetClientRect(hwnd, &rect);
    cxClient = rect.right - rect.left;
    cyClient = rect.bottom - rect.top;
    
    width = cxClient;
    height = cyClient;
    
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; /* Negative for top-down DIB */
    bi.biPlanes = 1;
    bi.biBitCount = 8;     /* 8 bits = 256 colors */
    bi.biCompression = BI_RGB;
    
    hbmBuffer = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    hbmOld = SelectObject(hdcBuffer, hbmBuffer);
    
    FillRect(hdcBuffer, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    /* Always load the classic tileset by default */
    strcpy(currentTileset, "classic");
    wsprintf(tilePath, "tilesets\\%s.bmp", currentTileset);
    loadTileset(tilePath);
    
    ReleaseDC(hwnd, hdc);
}

HBITMAP loadBitmapFile(const char* filename)
{
    HBITMAP hBitmap;
    
    hBitmap = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, 
                        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                        
    return hBitmap;
}

int loadTileset(const char* filename)
{
    HDC hdc;
    
    if (hdcTiles)
    {
        DeleteDC(hdcTiles);
        hdcTiles = NULL;
    }
    
    if (hbmTiles)
    {
        DeleteObject(hbmTiles);
        hbmTiles = NULL;
    }
    
    hbmTiles = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, 
                         LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
    if (hbmTiles == NULL)
    {
        return 0;
    }
    
    hdc = GetDC(hwndMain);
    hdcTiles = CreateCompatibleDC(hdc);
    
    SelectObject(hdcTiles, hbmTiles);
    
    ReleaseDC(hwndMain, hdc);
    return 1;
}

int changeTileset(HWND hwnd, const char* tilesetName)
{
    char tilesetPath[MAX_PATH];
    char windowTitle[MAX_PATH];
    HDC hdc;
    
    wsprintf(tilesetPath, "tilesets\\%s.bmp", tilesetName);
    
    if (hdcTiles)
    {
        DeleteDC(hdcTiles);
        hdcTiles = NULL;
    }
    
    if (hbmTiles)
    {
        DeleteObject(hbmTiles);
        hbmTiles = NULL;
    }
    
    hbmTiles = LoadImage(NULL, tilesetPath, IMAGE_BITMAP, 0, 0, 
                       LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
    if (hbmTiles == NULL)
    {
        return 0;
    }
    
    hdc = GetDC(hwndMain);
    hdcTiles = CreateCompatibleDC(hdc);
    
    SelectObject(hdcTiles, hbmTiles);
    
    strcpy(currentTileset, tilesetName);
    
    wsprintf(windowTitle, "MicropolisNT - Tileset: %s", tilesetName);
    SetWindowText(hwnd, windowTitle);
    
    InvalidateRect(hwnd, NULL, TRUE);
    
    ReleaseDC(hwndMain, hdc);
    return 1;
}

void cleanupGraphics(void)
{
    if (hbmBuffer)
    {
        DeleteObject(hbmBuffer);
        hbmBuffer = NULL;
    }
    
    if (hdcBuffer)
    {
        DeleteDC(hdcBuffer);
        hdcBuffer = NULL;
    }
    
    if (hbmTiles)
    {
        DeleteObject(hbmTiles);
        hbmTiles = NULL;
    }
    
    if (hdcTiles)
    {
        DeleteDC(hdcTiles);
        hdcTiles = NULL;
    }
    
    if (hPalette)
    {
        DeleteObject(hPalette);
        hPalette = NULL;
    }
}

void resizeBuffer(int cx, int cy)
{
    HDC hdc;
    HBITMAP hbmNew;
    RECT rcBuffer;
    BITMAPINFOHEADER bi;
    LPVOID bits;
    
    if (cx <= 0 || cy <= 0)
        return;
    
    hdc = GetDC(hwndMain);
    
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = cx;
    bi.biHeight = -cy; /* Negative for top-down DIB */
    bi.biPlanes = 1;
    bi.biBitCount = 8; /* 8 bits = 256 colors */
    bi.biCompression = BI_RGB;
    
    hbmNew = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    
    if (hbmBuffer)
        DeleteObject(hbmBuffer);
    
    hbmBuffer = hbmNew;
    SelectObject(hdcBuffer, hbmBuffer);
    
    rcBuffer.left = 0;
    rcBuffer.top = 0;
    rcBuffer.right = cx;
    rcBuffer.bottom = cy;
    FillRect(hdcBuffer, &rcBuffer, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    ReleaseDC(hwndMain, hdc);
    
    InvalidateRect(hwndMain, NULL, FALSE);
}

void scrollView(int dx, int dy)
{
    xOffset += dx;
    yOffset += dy;
    
    if (xOffset < 0)
        xOffset = 0;
    if (yOffset < 0)
        yOffset = 0;
    
    if (xOffset > WORLD_X * TILE_SIZE - cxClient)
        xOffset = WORLD_X * TILE_SIZE - cxClient;
    if (yOffset > WORLD_Y * TILE_SIZE - cyClient)
        yOffset = WORLD_Y * TILE_SIZE - cyClient;
    
    InvalidateRect(hwndMain, NULL, FALSE);
}

int loadCity(char *filename)
{
    FILE *f;
    DWORD size;
    size_t readResult;
    
    lstrcpy(cityFileName, filename);
    
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        MessageBox(hwndMain, "Failed to open city file", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    /* The original Micropolis city files are 27120 bytes */
    if (size != 27120)
    {
        fclose(f);
        MessageBox(hwndMain, "Invalid city file format", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    readResult = fread(ResHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(ResHis, HISTLEN/2);
    
    readResult = fread(ComHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(ComHis, HISTLEN/2);
    
    readResult = fread(IndHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(IndHis, HISTLEN/2);
    
    readResult = fread(CrimeHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(CrimeHis, HISTLEN/2);
    
    readResult = fread(PollutionHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(PollutionHis, HISTLEN/2);
    
    readResult = fread(MoneyHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(MoneyHis, HISTLEN/2);
    
    readResult = fread(MiscHis, sizeof(short), MISCHISTLEN/2, f);
    if (readResult != MISCHISTLEN/2) goto read_error;
    swapShorts(MiscHis, MISCHISTLEN/2);
    
    /* Original Micropolis stores map transposed compared to our array convention */
    {
        short tmpMap[WORLD_X][WORLD_Y];
        int x, y;
        
        readResult = fread(&tmpMap[0][0], sizeof(short), WORLD_X * WORLD_Y, f);
        if (readResult != WORLD_X * WORLD_Y) goto read_error;
        
        swapShorts((short*)tmpMap, WORLD_X * WORLD_Y);
        
        for (x = 0; x < WORLD_X; x++) {
            for (y = 0; y < WORLD_Y; y++) {
                Map[y][x] = tmpMap[x][y];
            }
        }
    }
    
    fclose(f);
    
    xOffset = (WORLD_X * TILE_SIZE - cxClient) / 2;
    yOffset = (WORLD_Y * TILE_SIZE - cyClient) / 2;
    if (xOffset < 0) xOffset = 0;
    if (yOffset < 0) yOffset = 0;
    
    /* Initialize the simulation with the new city data */
    DoSimInit();
    
    /* Unpause simulation at medium speed */
    SetSimulationSpeed(hwndMain, SPEED_MEDIUM);
    
    /* Update the window title with city name */
    {
        char windowTitle[MAX_PATH];
        char *baseName;
        char *dot;
        
        baseName = cityFileName;
        
        if (strrchr(baseName, '\\'))
            baseName = strrchr(baseName, '\\') + 1;
        if (strrchr(baseName, '/'))
            baseName = strrchr(baseName, '/') + 1;
            
        lstrcpy(windowTitle, "MicropolisNT - ");
        lstrcat(windowTitle, baseName);
        
        /* Remove the extension if present */
        dot = strrchr(windowTitle, '.');
        if (dot)
            *dot = '\0';
            
        SetWindowText(hwndMain, windowTitle);
    }
    
    InvalidateRect(hwndMain, NULL, FALSE);
    
    return 1;
    
read_error:
    fclose(f);
    MessageBox(hwndMain, "Failed to read city data", "Error", MB_ICONERROR | MB_OK);
    return 0;
}

int getBaseFromTile(short tile)
{
    tile &= LOMASK;
    
    if (tile >= TILE_WATER_LOW && tile <= TILE_WATER_HIGH)
        return TILE_RIVER;
    
    if (tile >= TILE_WOODS_LOW && tile <= TILE_WOODS_HIGH)
        return TILE_TREEBASE;
    
    if (tile >= TILE_ROADBASE && tile <= TILE_LASTROAD)
        return TILE_ROADBASE;
    
    if (tile >= TILE_POWERBASE && tile <= TILE_LASTPOWER)
        return TILE_POWERBASE;
    
    if (tile >= TILE_RAILBASE && tile <= TILE_LASTRAIL)
        return TILE_RAILBASE;
    
    if (tile >= TILE_RESBASE && tile <= TILE_LASTRES)
        return TILE_RESBASE;
    
    if (tile >= TILE_COMBASE && tile <= TILE_LASTCOM)
        return TILE_COMBASE;
    
    if (tile >= TILE_INDBASE && tile <= TILE_LASTIND)
        return TILE_INDBASE;
    
    if (tile >= TILE_FIREBASE && tile <= TILE_LASTFIRE)
        return TILE_FIREBASE;
    
    if (tile >= TILE_FLOOD && tile <= TILE_LASTFLOOD)
        return TILE_FLOOD;
    
    if (tile >= TILE_RUBBLE && tile <= TILE_LASTRUBBLE)
        return TILE_RUBBLE;
    
    return TILE_DIRT;
}

void drawTile(HDC hdc, int x, int y, short tileValue)
{
    RECT rect;
    HBRUSH hBrush;
    HBRUSH hOldBrush;
    COLORREF color;
    int tileIndex;
    int srcX;
    int srcY;
    
    /* Don't treat negative values as special - they are valid tile values with the sign bit set 
       In the original code, negative values in PowerMap indicated unpowered state,
       but in our implementation we need to still render the tile */
    
    tileIndex = tileValue & LOMASK;
    
    if (tileIndex >= TILE_TOTAL_COUNT)
    {
        tileIndex = 0;
    }
    
    /* Handle animated traffic tiles */
    if (tileValue & ANIMBIT) {
        /* Use low 2 bits of Fcycle for animation (0-3) */
        int frame = (Fcycle & 3);
        
        /* Light traffic animation range (80-127) */
        if (tileIndex >= 80 && tileIndex <= 127) {
            /* The way traffic works is:
               - Tiles 80-95 are frame 1 (use when frame=1)
               - Tiles 96-111 are frame 2 (use when frame=2)
               - Tiles 112-127 are frame 3 (use when frame=3)
               - Tiles 128-143 are frame 0 (use when frame=0)
            */
            int baseOffset = tileIndex & 0xF;  /* Base road layout (0-15) */
            
            /* Get correct frame tiles */
            switch (frame) {
                case 0: tileIndex = 128 + baseOffset; break; /* Frame 0 */
                case 1: tileIndex = 80 + baseOffset; break;  /* Frame 1 */
                case 2: tileIndex = 96 + baseOffset; break;  /* Frame 2 */
                case 3: tileIndex = 112 + baseOffset; break; /* Frame 3 */
            }
        }
        
        /* Heavy traffic animation range (144-207) */
        if (tileIndex >= 144 && tileIndex <= 207) {
            /* Heavy traffic works the same way:
               - Tiles 144-159 are frame 1 (use when frame=1)
               - Tiles 160-175 are frame 2 (use when frame=2)
               - Tiles 176-191 are frame 3 (use when frame=3)
               - Tiles 192-207 are frame 0 (use when frame=0)
            */
            int baseOffset = tileIndex & 0xF;  /* Base road layout (0-15) */
            
            /* Get correct frame tiles */
            switch (frame) {
                case 0: tileIndex = 192 + baseOffset; break; /* Frame 0 */
                case 1: tileIndex = 144 + baseOffset; break; /* Frame 1 */
                case 2: tileIndex = 160 + baseOffset; break; /* Frame 2 */
                case 3: tileIndex = 176 + baseOffset; break; /* Frame 3 */
            }
        }
    }
    
    rect.left = x;
    rect.top = y;
    rect.right = x + TILE_SIZE;
    rect.bottom = y + TILE_SIZE;
    
    if (hdcTiles && hbmTiles)
    {
        srcX = (tileIndex % TILES_IN_ROW) * TILE_SIZE; 
        srcY = (tileIndex / TILES_IN_ROW) * TILE_SIZE;
        
        BitBlt(hdc, x, y, TILE_SIZE, TILE_SIZE,
               hdcTiles, srcX, srcY, SRCCOPY);
    }
    else
    {
        switch (getBaseFromTile(tileValue))
        {
            case TILE_RIVER:
                color = RGB(0, 0, 128); /* Dark blue */
                break;
            case TILE_TREEBASE:
                color = RGB(0, 128, 0); /* Dark green */
                break;
            case TILE_ROADBASE:
                color = RGB(128, 128, 128); /* Gray */
                break;
            case TILE_RAILBASE:
                color = RGB(192, 192, 192); /* Light gray */
                break;
            case TILE_POWERBASE:
                color = RGB(255, 255, 0); /* Yellow */
                break;
            case TILE_RESBASE:
                color = RGB(0, 255, 0); /* Green */
                break;
            case TILE_COMBASE:
                color = RGB(0, 0, 255); /* Blue */
                break;
            case TILE_INDBASE:
                color = RGB(255, 255, 0); /* Yellow */
                break;
            case TILE_FIREBASE:
                color = RGB(255, 0, 0); /* Red */
                break;
            case TILE_FLOOD:
                color = RGB(0, 128, 255); /* Light blue */
                break;
            case TILE_RUBBLE:
                color = RGB(128, 128, 0); /* Olive */
                break;
            case TILE_DIRT:
            default:
                color = RGB(204, 102, 0); /* Orange-brown */
                break;
        }
        
        hBrush = CreateSolidBrush(color);
        hOldBrush = SelectObject(hdc, hBrush);
        
        FillRect(hdc, &rect, hBrush);
        
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
    
    if (tileValue & ZONEBIT)
    {
        FrameRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    }
    
    if ((tileValue & ZONEBIT) && !(tileValue & POWERBIT))
    {
        /* Unpowered zones get a yellow frame */
        hBrush = CreateSolidBrush(RGB(255, 255, 0));
        FrameRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
    }
    else if ((tileValue & ZONEBIT) && (tileValue & POWERBIT))
    {
        /* Powered zones get a green frame */
        hBrush = CreateSolidBrush(RGB(0, 255, 0));
        FrameRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
    }
    
    /* Commented out power indicator dots for cleaner display 
    else if ((tileValue & CONDBIT) && (tileValue & POWERBIT))
    {
        // Power lines and other conductors with power get a cyan dot
        int dotSize = 4;
        RECT dotRect;
        
        dotRect.left = rect.left + (TILE_SIZE - dotSize) / 2;
        dotRect.top = rect.top + (TILE_SIZE - dotSize) / 2;
        dotRect.right = dotRect.left + dotSize;
        dotRect.bottom = dotRect.top + dotSize;
        
        hBrush = CreateSolidBrush(RGB(0, 255, 255));
        FillRect(hdc, &dotRect, hBrush);
        DeleteObject(hBrush);
    }
    */
    
    /* Commented out traffic visualization dots for cleaner display 
    {
        int tileBase = tileValue & LOMASK;
        Byte trafficLevel;
        
        if ((tileBase >= ROADBASE && tileBase <= LASTROAD) || 
            (tileBase >= RAILBASE && tileBase <= LASTRAIL))
        {
            // Get the traffic density for this location
            trafficLevel = TrfDensity[y/2][x/2];
            
            // Only display if there's significant traffic
            if (trafficLevel > 40) {
                COLORREF trafficColor;
                RECT trafficRect;
                int trafficSize;
                
                // Scale traffic visualization by density level
                if (trafficLevel < 100) {
                    trafficColor = RGB(255, 255, 0); // Yellow for light traffic
                    trafficSize = 2;
                } else if (trafficLevel < 200) {
                    trafficColor = RGB(255, 128, 0); // Orange for medium traffic
                    trafficSize = 3;
                } else {
                    trafficColor = RGB(255, 0, 0);   // Red for heavy traffic
                    trafficSize = 4;
                }
                
                // Draw traffic indicator
                trafficRect.left = rect.left + (TILE_SIZE - trafficSize) / 2;
                trafficRect.top = rect.top + (TILE_SIZE - trafficSize) / 2;
                trafficRect.right = trafficRect.left + trafficSize;
                trafficRect.bottom = trafficRect.top + trafficSize;
                
                hBrush = CreateSolidBrush(trafficColor);
                FillRect(hdc, &trafficRect, hBrush);
                DeleteObject(hBrush);
            }
        }
    }
    */
}

void drawCity(HDC hdc)
{
    int x;
    int y;
    int screenX;
    int screenY;
    int startX;
    int startY;
    int endX;
    int endY;
    int barWidth;
    int maxHeight;
    int spacing;
    int rciStartX;
    int rciStartY;
    int rHeight;
    int cHeight;
    int iHeight;
    int localR;
    int localC;
    int localI;
    int cityMonth;
    int cityYear;
    int fundValue;
    int popValue;
    RECT rcClient;
    RECT rciRect;
    HBRUSH hResBrush;
    HBRUSH hComBrush;
    HBRUSH hIndBrush;
    HPEN hCenterPen;
    HPEN hOldPen;
    char *baseName;
    char *lastSlash;
    char *lastFwdSlash;
    char nameBuffer[MAX_PATH];
    char buffer[256];
    char *dot;
    
    /* Copy simulation values to local variables for display */
    cityMonth = CityMonth;
    cityYear = CityYear;
    fundValue = (int)TotalFunds;
    popValue = TotalPop;
    
    /* Calculate visible range */
    startX = xOffset / TILE_SIZE;
    startY = yOffset / TILE_SIZE;
    endX = startX + (cxClient / TILE_SIZE) + 1;
    endY = startY + (cyClient / TILE_SIZE) + 1;
    
    /* Bounds check */
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > WORLD_X) endX = WORLD_X;
    if (endY > WORLD_Y) endY = WORLD_Y;
    
    /* Clear the background */
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = cxClient;
    rcClient.bottom = cyClient;
    FillRect(hdc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    /* Draw the map tiles */
    for (y = startY; y < endY; y++)
    {
        for (x = startX; x < endX; x++)
        {
            screenX = x * TILE_SIZE - xOffset;
            screenY = y * TILE_SIZE - yOffset;
            
            drawTile(hdc, screenX, screenY, Map[y][x]);
        }
    }
    
    /* Setup text drawing */
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    
    /* Display city info if a city is loaded */
    if (cityFileName[0] != '\0')
    {
        /* Extract the city name from the path */
        baseName = cityFileName;
        lastSlash = strrchr(cityFileName, '\\');
        lastFwdSlash = strrchr(cityFileName, '/');
        
        if (lastSlash && lastSlash > baseName)
            baseName = lastSlash + 1;
        if (lastFwdSlash && lastFwdSlash > baseName)
            baseName = lastFwdSlash + 1;
        
        lstrcpy(nameBuffer, baseName);
        dot = strrchr(nameBuffer, '.');
        if (dot)
            *dot = '\0';
        
        /* Show city name */
        TextOut(hdc, 10, 10, nameBuffer, lstrlen(nameBuffer));
        
        /* Show simulation stats */
        wsprintf(buffer, "Year: %d  Month: %d  Funds: $%d  Population: %d", 
                 cityYear, cityMonth + 1, fundValue, popValue);
        TextOut(hdc, 10, 30, buffer, lstrlen(buffer));
        
        /* Show RCI demands as text */
        wsprintf(buffer, "R:%d C:%d I:%d  Traffic:%d", RValve, CValve, IValve, TrafficAverage);
        TextOut(hdc, 10, 50, buffer, lstrlen(buffer));
        
        /* Show evaluation results */
        wsprintf(buffer, "Score: %d  Class: %s  Rating: %d%%", 
                 CityScore, GetCityClassName(), (CityYes > 0) ? (CityYes) : 0);
        TextOut(hdc, 10, 70, buffer, lstrlen(buffer));
        
        /* Show top problems if we have any */
        if (CityPop > 0) {
            short problems[4];
            GetTopProblems(problems);
            
            if (problems[0] < 7) {  /* 7 = PROB_NONE */
                wsprintf(buffer, "Top Problem: %s", GetProblemText(problems[0]));
                TextOut(hdc, 10, 90, buffer, lstrlen(buffer));
            }
        }
        
        /* Show budget information */
        {
            int taxPercent = TaxRate;
            QUAD taxIncome = GetTaxIncome();
            
            wsprintf(buffer, "Tax: %d%%  Income: $%ld  Fire: %d%%  Police: %d%%  Roads: %d%%", 
                    taxPercent, taxIncome, 
                    GetFireEffect(), GetPoliceEffect(), GetRoadEffect());
            TextOut(hdc, 10, 110, buffer, lstrlen(buffer));
        }
        
        /* Constants for RCI display */
        barWidth = 20;
        maxHeight = 50;
        spacing = 5;
        rciStartX = 10;
        rciStartY = 80;
        
        /* Copy RCI values to local variables */
        localR = RValve;
        localC = CValve;
        localI = IValve;
        
        /* Limit values to range for drawing */
        if (localR > 2000) localR = 2000;
        if (localR < -2000) localR = -2000;
        if (localC > 2000) localC = 2000;
        if (localC < -2000) localC = -2000;
        if (localI > 2000) localI = 2000;
        if (localI < -2000) localI = -2000;
        
        /* Create brushes using system palette colors */
        hResBrush = CreateSolidBrush(RGB(0, 128, 0));    /* Dark Green */
        hComBrush = CreateSolidBrush(RGB(0, 0, 128));    /* Dark Blue */
        hIndBrush = CreateSolidBrush(RGB(128, 128, 0));  /* Dark Yellow */
        
        /* Error check for failed resource creation */
        if (!hResBrush || !hComBrush || !hIndBrush)
        {
            /* Clean up any resources that were created */
            if (hResBrush) DeleteObject(hResBrush);
            if (hComBrush) DeleteObject(hComBrush);
            if (hIndBrush) DeleteObject(hIndBrush);
            return;
        }
        
        /* Draw the horizontal center line */
        hCenterPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        if (hCenterPen)
        {
            hOldPen = (HPEN)SelectObject(hdc, hCenterPen);
            MoveToEx(hdc, rciStartX - 5, rciStartY, NULL);
            LineTo(hdc, rciStartX + barWidth * 3 + spacing * 2 + 5, rciStartY);
            SelectObject(hdc, hOldPen);
        }
        
        /* Residential demand bar */
        if (localR >= 0) {
            rHeight = localR * maxHeight / 2000;
            rciRect.left = rciStartX;
            rciRect.right = rciStartX + barWidth;
            rciRect.bottom = rciStartY;
            rciRect.top = rciStartY - rHeight;
        } else {
            rHeight = -localR * maxHeight / 2000;
            rciRect.left = rciStartX;
            rciRect.right = rciStartX + barWidth;
            rciRect.top = rciStartY;
            rciRect.bottom = rciStartY + rHeight;
        }
        FillRect(hdc, &rciRect, hResBrush);
        
        /* Commercial demand bar */
        if (localC >= 0) {
            cHeight = localC * maxHeight / 2000;
            rciRect.left = rciStartX + barWidth + spacing;
            rciRect.right = rciStartX + barWidth * 2 + spacing;
            rciRect.bottom = rciStartY;
            rciRect.top = rciStartY - cHeight;
        } else {
            cHeight = -localC * maxHeight / 2000;
            rciRect.left = rciStartX + barWidth + spacing;
            rciRect.right = rciStartX + barWidth * 2 + spacing;
            rciRect.top = rciStartY;
            rciRect.bottom = rciStartY + cHeight;
        }
        FillRect(hdc, &rciRect, hComBrush);
        
        /* Industrial demand bar */
        if (localI >= 0) {
            iHeight = localI * maxHeight / 2000;
            rciRect.left = rciStartX + barWidth * 2 + spacing * 2;
            rciRect.right = rciStartX + barWidth * 3 + spacing * 2;
            rciRect.bottom = rciStartY;
            rciRect.top = rciStartY - iHeight;
        } else {
            iHeight = -localI * maxHeight / 2000;
            rciRect.left = rciStartX + barWidth * 2 + spacing * 2;
            rciRect.right = rciStartX + barWidth * 3 + spacing * 2;
            rciRect.top = rciStartY;
            rciRect.bottom = rciStartY + iHeight;
        }
        FillRect(hdc, &rciRect, hIndBrush);
    
        /* Clean up GDI resources properly */
        if (hResBrush) DeleteObject(hResBrush);
        if (hComBrush) DeleteObject(hComBrush);
        if (hIndBrush) DeleteObject(hIndBrush);
        if (hCenterPen) DeleteObject(hCenterPen);
    }
}

void openCityDialog(HWND hwnd)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH];
    
    szFileName[0] = '\0';
    
    ZeroMemory(&ofn, sizeof(ofn));
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "City Files (*.cty)\0*.cty\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "cty";
    
    if(GetOpenFileName(&ofn))
    {
        loadCity(szFileName);
    }
}

HMENU createMainMenu(void)
{
    HMENU hMainMenu;
    
    hMainMenu = CreateMenu();
    
    hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, "&Open City...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, "E&xit");
    
    hTilesetMenu = CreatePopupMenu();
    populateTilesetMenu(hTilesetMenu);
    
    hSimMenu = CreatePopupMenu();
    AppendMenu(hSimMenu, MF_STRING, IDM_SIM_PAUSE, "&Pause");
    AppendMenu(hSimMenu, MF_STRING, IDM_SIM_SLOW, "&Slow");
    AppendMenu(hSimMenu, MF_STRING, IDM_SIM_MEDIUM, "&Medium");
    AppendMenu(hSimMenu, MF_STRING, IDM_SIM_FAST, "&Fast");
    
    /* Default simulation speed is medium */
    CheckMenuRadioItem(hSimMenu, IDM_SIM_PAUSE, IDM_SIM_FAST, IDM_SIM_MEDIUM, MF_BYCOMMAND);
    
    AppendMenu(hMainMenu, MF_POPUP, (UINT)hFileMenu, "&File");
    AppendMenu(hMainMenu, MF_POPUP, (UINT)hTilesetMenu, "&Tileset");
    AppendMenu(hMainMenu, MF_POPUP, (UINT)hSimMenu, "&Speed");
    
    return hMainMenu;
}

void populateTilesetMenu(HMENU hSubMenu)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    char fileName[MAX_PATH];
    char* dot;
    int menuId = IDM_TILESET_BASE;
    UINT menuFlags;
    
    strcpy(searchPath, "tilesets\\*.bmp");
    
    hFind = FindFirstFile(searchPath, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
                continue;
                
            strcpy(fileName, findData.cFileName);
            dot = strrchr(fileName, '.');
            if (dot != NULL)
                *dot = '\0';
                
            menuFlags = MF_STRING;
            if (strcmp(fileName, currentTileset) == 0)
                menuFlags |= MF_CHECKED;
                
            AppendMenu(hSubMenu, menuFlags, menuId++, fileName);
            
        } while (FindNextFile(hFind, &findData) && menuId < IDM_TILESET_MAX);
        
        FindClose(hFind);
    }
    else
    {
        AppendMenu(hSubMenu, MF_GRAYED, 0, "No tilesets found");
    }
}