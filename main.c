/* main.c
 *
 * Main entry point for MicropolisNT (Windows NT version)
 * Based on original Micropolis code from MicropolisLegacy project
 */

#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Menu resource identifiers */
#define IDM_FILE_OPEN       1001
#define IDM_FILE_EXIT       1002
#define IDM_TILESET_BASE    2000
#define IDM_TILESET_MAX     2100

/* Add this define for LR_LOADFROMFILE | LR_CREATEDIBSECTION */
#ifndef LR_CREATEDIBSECTION
#define LR_CREATEDIBSECTION 0x2000
#endif

#ifndef LR_DEFAULTCOLOR
#define LR_DEFAULTCOLOR 0
#endif

#ifndef LR_DEFAULTSIZE
#define LR_DEFAULTSIZE 0x0040
#endif


/* Ensure we have the constants needed for image loading */
#ifndef LR_LOADFROMFILE
#define LR_LOADFROMFILE 0x0010
#endif

#ifndef IMAGE_BITMAP
#define IMAGE_BITMAP 0
#endif

/* World dimensions from the original Micropolis */
#define WORLD_X 120
#define WORLD_Y 100
#define TILE_SIZE 16  /* Tile size in pixels */

/* History lengths */
#define HISTLEN 480
#define MISCHISTLEN 240

/* Map data */
static short Map[WORLD_Y][WORLD_X];

/* History data */
static short ResHis[HISTLEN/2];
static short ComHis[HISTLEN/2];
static short IndHis[HISTLEN/2];
static short CrimeHis[HISTLEN/2];
static short PollutionHis[HISTLEN/2];
static short MoneyHis[HISTLEN/2];
static short MiscHis[MISCHISTLEN/2];

/* Handle to the main application window */
static HWND hwndMain = NULL;

/* GDI resources */
static HBITMAP hbmBuffer = NULL;
static HDC hdcBuffer = NULL;
static HBITMAP hbmTiles = NULL;  /* Tileset bitmap */
static HDC hdcTiles = NULL;      /* Memory DC for tileset */
static HPALETTE hPalette = NULL; /* Palette for 256 color support */

/* Size of client area */
static int cxClient = 0;
static int cyClient = 0;

/* Viewport position */
static int xOffset = 0;
static int yOffset = 0;

/* Mouse state variables */
static BOOL isMouseDown = FALSE;
static int lastMouseX = 0;
static int lastMouseY = 0;

/* City filename */
static char cityFileName[MAX_PATH];

/* Menu handles */
static HMENU hMenu = NULL;
static HMENU hFileMenu = NULL;
static HMENU hTilesetMenu = NULL;

/* Currently active tileset */
static char currentTileset[MAX_PATH] = "classic";

/* Tile type masks from the original Micropolis */
#define BIT_MASK        0x03ff  /* Mask for the low 10 bits = 1023 decimal */
#define LOMASK          BIT_MASK
#define ANIMBIT         0x0800  /* bit 11, tile is animated */
#define BURNBIT         0x2000  /* bit 13, tile can be lit */
#define BULLBIT         0x1000  /* bit 12, tile is bulldozable */
#define CONDBIT         0x4000  /* bit 14, tile can conduct electricity */
#define ZONEBIT         0x0400  /* bit 10, tile is the center of a zone */
#define POWERBIT        0x8000  /* bit 15, tile has power */

/* Tile types - from MicropolisJS */
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

/* Tileset information */
#define TILES_IN_ROW        32  /* Number of tiles in a row in the tileset image */
#define TILES_PER_ROW       32  /* For backwards compatibility */

/* Function prototypes */
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

/* Entry point for the application */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    MSG msg;
    
    /* Register the window class with palette support */
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
    
    /* Create the main menu */
    hMenu = createMainMenu();
    
    /* Create the main window with class style for palette support */
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
    
    /* Initialize graphics */
    initializeGraphics(hwndMain);
    
    /* Clear city filename */
    cityFileName[0] = '\0';
    
    /* Initialize map with dirt for testing */
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
    
    /* Show the window */
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    
    /* Main message loop */
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    /* Clean up */
    cleanupGraphics();
    
    return msg.wParam;
}


/* Window procedure */
LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            /* Update Tileset menu to check current tileset */
            CheckMenuRadioItem(hTilesetMenu, 0, GetMenuItemCount(hTilesetMenu)-1, 0, MF_BYPOSITION);
            return 0;
        
        case WM_COMMAND:
            /* Handle menu commands */
            switch(LOWORD(wParam))
            {
                case IDM_FILE_OPEN:
                    openCityDialog(hwnd);
                    return 0;
                    
                case IDM_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    return 0;
                    
                default:
                    /* Check if it's a tileset selection */
                    if (LOWORD(wParam) >= IDM_TILESET_BASE && LOWORD(wParam) < IDM_TILESET_MAX)
                    {
                        int index;
                        char tilesetName[MAX_PATH];
                        
                        /* Get the menu item index */
                        index = LOWORD(wParam) - IDM_TILESET_BASE;
                        
                        /* Get menu text directly - compatible with Windows NT 4.0 */
                        GetMenuString(hTilesetMenu, LOWORD(wParam), tilesetName, MAX_PATH - 1, MF_BYCOMMAND);
                        
                        /* Apply the selected tileset - if successful, update menu */
                        if (changeTileset(hwnd, tilesetName))
                        {
                            /* Update checked menu item */
                            CheckMenuRadioItem(hTilesetMenu, 0, GetMenuItemCount(hTilesetMenu)-1, 
                                            index, MF_BYPOSITION);
                        }
                        return 0;
                    }
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
            
            /* Ensure the palette is selected into the DC */
            if (hPalette)
            {
                SelectPalette(hdc, hPalette, FALSE);
                RealizePalette(hdc);
            }
            
            /* Draw the city onto the buffer */
            if (hbmBuffer)
            {
                drawCity(hdcBuffer);
                
                /* Copy from buffer to screen */
                BitBlt(hdc, 0, 0, cxClient, cyClient, 
                       hdcBuffer, 0, 0, SRCCOPY);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN:
        {
            /* Start panning on mouse down */
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            
            /* Capture mouse */
            isMouseDown = TRUE;
            lastMouseX = xPos;
            lastMouseY = yPos;
            SetCapture(hwnd);
            
            /* Set cursor to indicate grabbing */
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
            
            return 0;
        }
        
        case WM_MOUSEMOVE:
        {
            /* Handle panning when mouse is down */
            if (isMouseDown)
            {
                int xPos = LOWORD(lParam);
                int yPos = HIWORD(lParam);
                int dx = lastMouseX - xPos;
                int dy = lastMouseY - yPos;
                
                /* Update last position */
                lastMouseX = xPos;
                lastMouseY = yPos;
                
                /* Pan the view */
                if (dx != 0 || dy != 0)
                {
                    scrollView(dx, dy);
                }
                
                /* Keep showing the appropriate cursor */
                SetCursor(LoadCursor(NULL, IDC_SIZEALL));
            }
            else
            {
                /* Show arrow cursor when hovering over the map to indicate it can be panned */
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            return 0;
        }
        
        case WM_LBUTTONUP:
        {
            /* End panning */
            isMouseDown = FALSE;
            ReleaseCapture();
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 0;
        }
        
        case WM_SETCURSOR:
        {
            /* Handle cursor appearance based on current state */
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
            
            /* Resize the buffer */
            resizeBuffer(cxClient, cyClient);
            
            return 0;
        }
        
        case WM_KEYDOWN:
        {
            /* Handle keyboard navigation */
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
                    
                case 'O': /* Open file */
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        openCityDialog(hwnd);
                    }
                    break;
                    
                case 'Q': /* Quit */
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    }
                    break;
                    
                case 'I': /* Show bitmap info */
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
            cleanupGraphics();
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

/* Swap bytes in short array (for endian conversion) */
void swapShorts(short *buf, int len)
{
    int i;
    
    /* Flip bytes in each short */
    for (i = 0; i < len; i++)
    {
        buf[i] = ((buf[i] & 0xFF) << 8) | ((buf[i] & 0xFF00) >> 8);
    }
}

/* Create a default 16-color palette - only used if no tileset is loaded */
HPALETTE createSystemPalette(void)
{
    LOGPALETTE *pLogPal;
    HPALETTE hPal;
    int i;
    PALETTEENTRY colorTable[16] = {
        {0, 0, 0, 0},          /* 0: Black */
        {128, 0, 0, 0},        /* 1: Dark Red */
        {0, 128, 0, 0},        /* 2: Dark Green */
        {128, 128, 0, 0},      /* 3: Dark Yellow */
        {0, 0, 128, 0},        /* 4: Dark Blue */
        {128, 0, 128, 0},      /* 5: Dark Magenta */
        {0, 128, 128, 0},      /* 6: Dark Cyan */
        {192, 192, 192, 0},    /* 7: Light Gray */
        {128, 128, 128, 0},    /* 8: Dark Gray */
        {255, 0, 0, 0},        /* 9: Red */
        {0, 255, 0, 0},        /* 10: Green */
        {255, 255, 0, 0},      /* 11: Yellow */
        {0, 0, 255, 0},        /* 12: Blue */
        {255, 0, 255, 0},      /* 13: Magenta */
        {0, 255, 255, 0},      /* 14: Cyan */
        {255, 255, 255, 0}     /* 15: White */
    };
    
    /* Allocate memory for palette (16 colors for 4-bit default) */
    pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 15 * sizeof(PALETTEENTRY));
    if (!pLogPal)
        return NULL;
    
    /* Initialize palette structure */
    pLogPal->palVersion = 0x300; /* Windows 3.0 */
    pLogPal->palNumEntries = 16; /* 16 colors (4-bit) */
    
    /* Copy the 16-color standard Windows palette */
    for (i = 0; i < 16; i++)
    {
        pLogPal->palPalEntry[i] = colorTable[i];
    }
    
    /* Create the palette */
    hPal = CreatePalette(pLogPal);
    
    /* Free allocated memory */
    free(pLogPal);
    
    return hPal;
}

/* Initialize graphics resources */
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
    
    /* Get a DC for the window */
    hdc = GetDC(hwnd);
    
    /* Create a default 16-color palette if no tileset is loaded yet */
    if (hPalette == NULL)
    {
        hPalette = createSystemPalette();
        
        /* Apply the palette to the window */
        if (hPalette)
        {
            SelectPalette(hdc, hPalette, FALSE);
            RealizePalette(hdc);
        }
    }
    
    /* Create a compatible DC for double buffering */
    hdcBuffer = CreateCompatibleDC(hdc);
    
    /* Apply the palette to the memory DC */
    if (hPalette)
    {
        SelectPalette(hdcBuffer, hPalette, FALSE);
        RealizePalette(hdcBuffer);
    }
    
    /* Get client area dimensions */
    GetClientRect(hwnd, &rect);
    cxClient = rect.right - rect.left;
    cyClient = rect.bottom - rect.top;
    
    /* Setup for a DIB section, either 8-bit or 4-bit based on current palette */
    width = cxClient;
    height = cyClient;
    
    /* Initialize BITMAPINFOHEADER structure */
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; /* Negative for top-down DIB */
    bi.biPlanes = 1;
    bi.biBitCount = 8;     /* 8 bits = 256 colors */
    bi.biCompression = BI_RGB;
    
    /* Create DIB section with appropriate color depth */
    hbmBuffer = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    hbmOld = SelectObject(hdcBuffer, hbmBuffer);
    
    /* Initialize with black background */
    FillRect(hdcBuffer, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    /* Load the default tileset */
    wsprintf(tilePath, "tilesets\\%s.bmp", currentTileset);
    
    if (!loadTileset(tilePath))
    {
        /* Just fall back to simple color tiles silently */
    }
    
    /* Release the window DC */
    ReleaseDC(hwnd, hdc);
}

/* Load bitmap with simple palette */
HBITMAP loadBitmapFile(const char* filename)
{
    HBITMAP hBitmap;
    
    /* Just use LoadImage for simplicity */
    hBitmap = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, 
                        LR_LOADFROMFILE | LR_CREATEDIBSECTION);
                        
    return hBitmap;
}

/* Load the tileset image */
int loadTileset(const char* filename)
{
    HDC hdc;
    
    /* First clean up any existing resources */
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
    
    /* Load the bitmap file directly */
    hbmTiles = LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, 
                         LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
    if (hbmTiles == NULL)
    {
        /* Failed to load bitmap - return silently */
        return 0;
    }
    
    /* Create a device context for the tileset */
    hdc = GetDC(hwndMain);
    hdcTiles = CreateCompatibleDC(hdc);
    
    /* Select the tileset into the DC */
    SelectObject(hdcTiles, hbmTiles);
    
    ReleaseDC(hwndMain, hdc);
    return 1;
}

/* Change the current tileset */
int changeTileset(HWND hwnd, const char* tilesetName)
{
    char tilesetPath[MAX_PATH];
    char windowTitle[MAX_PATH];
    HDC hdc;
    
    /* Form the full path to the tileset */
    wsprintf(tilesetPath, "tilesets\\%s.bmp", tilesetName);
    
    /* First clean up any existing resources */
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
    
    /* Load the bitmap file directly */
    hbmTiles = LoadImage(NULL, tilesetPath, IMAGE_BITMAP, 0, 0, 
                       LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    
    if (hbmTiles == NULL)
    {
        /* Failed to load bitmap - return silently */
        return 0;
    }
    
    /* Create a device context for the tileset */
    hdc = GetDC(hwndMain);
    hdcTiles = CreateCompatibleDC(hdc);
    
    /* Select the tileset into the DC */
    SelectObject(hdcTiles, hbmTiles);
    
    /* Update current tileset name */
    strcpy(currentTileset, tilesetName);
    
    /* Update window title */
    wsprintf(windowTitle, "MicropolisNT - Tileset: %s", tilesetName);
    SetWindowText(hwnd, windowTitle);
    
    /* Force a redraw to display the new tileset */
    InvalidateRect(hwnd, NULL, TRUE);
    
    ReleaseDC(hwndMain, hdc);
    return 1;
}

/* Clean up graphics resources */
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

/* Resize the buffer bitmap */
void resizeBuffer(int cx, int cy)
{
    HDC hdc;
    HBITMAP hbmNew;
    RECT rcBuffer;
    BITMAPINFOHEADER bi;
    LPVOID bits;
    
    if (cx <= 0 || cy <= 0)
        return;
    
    /* Get a DC for the window */
    hdc = GetDC(hwndMain);
    
    /* Initialize BITMAPINFOHEADER structure for 256 colors */
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = cx;
    bi.biHeight = -cy; /* Negative for top-down DIB */
    bi.biPlanes = 1;
    bi.biBitCount = 8; /* 8 bits = 256 colors */
    bi.biCompression = BI_RGB;
    
    /* Create a 256-color DIB section */
    hbmNew = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    
    /* Select it into the buffer DC */
    if (hbmBuffer)
        DeleteObject(hbmBuffer);
    
    hbmBuffer = hbmNew;
    SelectObject(hdcBuffer, hbmBuffer);
    
    /* Initialize with black background */
    rcBuffer.left = 0;
    rcBuffer.top = 0;
    rcBuffer.right = cx;
    rcBuffer.bottom = cy;
    FillRect(hdcBuffer, &rcBuffer, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    /* Release the window DC */
    ReleaseDC(hwndMain, hdc);
    
    /* Force a redraw */
    InvalidateRect(hwndMain, NULL, FALSE);
}

/* Scroll the view */
void scrollView(int dx, int dy)
{
    /* Adjust offsets */
    xOffset += dx;
    yOffset += dy;
    
    /* Constrain to map bounds */
    if (xOffset < 0)
        xOffset = 0;
    if (yOffset < 0)
        yOffset = 0;
    
    if (xOffset > WORLD_X * TILE_SIZE - cxClient)
        xOffset = WORLD_X * TILE_SIZE - cxClient;
    if (yOffset > WORLD_Y * TILE_SIZE - cyClient)
        yOffset = WORLD_Y * TILE_SIZE - cyClient;
    
    /* Force a redraw */
    InvalidateRect(hwndMain, NULL, FALSE);
}

/* Load city data from a file */
int loadCity(char *filename)
{
    FILE *f;
    DWORD size;
    size_t readResult;
    
    /* Store the new city name */
    lstrcpy(cityFileName, filename);
    
    /* Open the city file */
    f = fopen(filename, "rb");
    if (f == NULL)
    {
        MessageBox(hwndMain, "Failed to open city file", "Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    
    /* Check file size */
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
    
    /* Read all history data first, then the map */
    
    /* ResHis: Residential history */
    readResult = fread(ResHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(ResHis, HISTLEN/2);
    
    /* ComHis: Commercial history */
    readResult = fread(ComHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(ComHis, HISTLEN/2);
    
    /* IndHis: Industrial history */
    readResult = fread(IndHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(IndHis, HISTLEN/2);
    
    /* CrimeHis: Crime history */
    readResult = fread(CrimeHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(CrimeHis, HISTLEN/2);
    
    /* PollutionHis: Pollution history */
    readResult = fread(PollutionHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(PollutionHis, HISTLEN/2);
    
    /* MoneyHis: Money history */
    readResult = fread(MoneyHis, sizeof(short), HISTLEN/2, f);
    if (readResult != HISTLEN/2) goto read_error;
    swapShorts(MoneyHis, HISTLEN/2);
    
    /* MiscHis: Miscellaneous history */
    readResult = fread(MiscHis, sizeof(short), MISCHISTLEN/2, f);
    if (readResult != MISCHISTLEN/2) goto read_error;
    swapShorts(MiscHis, MISCHISTLEN/2);
    
    /* Map data */
    /* 
     * IMPORTANT: The original Micropolis map is stored in row-major order
     * but transposed compared to our Map[y][x] array convention.
     * We need to read it carefully to ensure correct orientation.
     */
    {
        short tmpMap[WORLD_X][WORLD_Y]; /* Temporary array with transposed dimensions */
        int x, y;
        
        /* Read the map data in the original format */
        readResult = fread(&tmpMap[0][0], sizeof(short), WORLD_X * WORLD_Y, f);
        if (readResult != WORLD_X * WORLD_Y) goto read_error;
        
        /* Swap bytes in map data (original Micropolis stores data in big-endian format) */
        swapShorts((short*)tmpMap, WORLD_X * WORLD_Y);
        
        /* Now correctly transpose the data to our Map array */
        for (x = 0; x < WORLD_X; x++) {
            for (y = 0; y < WORLD_Y; y++) {
                Map[y][x] = tmpMap[x][y];
            }
        }
    }
    
    fclose(f);
    
    /* Reset view position */
    xOffset = (WORLD_X * TILE_SIZE - cxClient) / 2;
    yOffset = (WORLD_Y * TILE_SIZE - cyClient) / 2;
    if (xOffset < 0) xOffset = 0;
    if (yOffset < 0) yOffset = 0;
    
    /* Force a redraw */
    InvalidateRect(hwndMain, NULL, FALSE);
    
    return 1;
    
read_error:
    fclose(f);
    MessageBox(hwndMain, "Failed to read city data", "Error", MB_ICONERROR | MB_OK);
    return 0;
}

/* Determine the base tile type from a tile value */
int getBaseFromTile(short tile)
{
    /* Get the base tile value (lower 10 bits) */
    tile &= LOMASK;
    
    /* Water */
    if (tile >= TILE_WATER_LOW && tile <= TILE_WATER_HIGH)
        return TILE_RIVER;
    
    /* Trees */
    if (tile >= TILE_WOODS_LOW && tile <= TILE_WOODS_HIGH)
        return TILE_TREEBASE;
    
    /* Roads */
    if (tile >= TILE_ROADBASE && tile <= TILE_LASTROAD)
        return TILE_ROADBASE;
    
    /* Power lines */
    if (tile >= TILE_POWERBASE && tile <= TILE_LASTPOWER)
        return TILE_POWERBASE;
    
    /* Rails */
    if (tile >= TILE_RAILBASE && tile <= TILE_LASTRAIL)
        return TILE_RAILBASE;
    
    /* Residential zones */
    if (tile >= TILE_RESBASE && tile <= TILE_LASTRES)
        return TILE_RESBASE;
    
    /* Commercial zones */
    if (tile >= TILE_COMBASE && tile <= TILE_LASTCOM)
        return TILE_COMBASE;
    
    /* Industrial zones */
    if (tile >= TILE_INDBASE && tile <= TILE_LASTIND)
        return TILE_INDBASE;
    
    /* Fire */
    if (tile >= TILE_FIREBASE && tile <= TILE_LASTFIRE)
        return TILE_FIREBASE;
    
    /* Flood */
    if (tile >= TILE_FLOOD && tile <= TILE_LASTFLOOD)
        return TILE_FLOOD;
    
    /* Rubble */
    if (tile >= TILE_RUBBLE && tile <= TILE_LASTRUBBLE)
        return TILE_RUBBLE;
    
    /* Dirt (default) */
    return TILE_DIRT;
}

/* Draw a single tile */
void drawTile(HDC hdc, int x, int y, short tileValue)
{
    RECT rect;
    HBRUSH hBrush;
    HBRUSH hOldBrush;
    COLORREF color;
    int tileIndex;
    int srcX;
    int srcY;
    
    /* Check for special cases */
    if (tileValue < 0)
    {
        /* Invalid tile - paint black */
        rect.left = x;
        rect.top = y;
        rect.right = x + TILE_SIZE;
        rect.bottom = y + TILE_SIZE;
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        return;
    }
    
    /* Get the base tile value (mask off the flags) */
    tileIndex = tileValue & BIT_MASK;
    
    /* Safety check - ensure tile index is within bounds */
    if (tileIndex >= TILE_TOTAL_COUNT)
    {
        tileIndex = 0; /* Use dirt tile for invalid indexes */
    }
    
    /* Create tile rectangle */
    rect.left = x;
    rect.top = y;
    rect.right = x + TILE_SIZE;
    rect.bottom = y + TILE_SIZE;
    
    /* Use the tileset for all tiles */
    if (hdcTiles && hbmTiles)
    {
        /* Calculate source coordinates in the tileset */
        srcX = (tileIndex % TILES_IN_ROW) * TILE_SIZE; 
        srcY = (tileIndex / TILES_IN_ROW) * TILE_SIZE;
        
        /* Copy tile from tileset to screen */
        BitBlt(hdc, x, y, TILE_SIZE, TILE_SIZE,
               hdcTiles, srcX, srcY, SRCCOPY);
    }
    else
    {
        /* Fallback: Use simple colors if no tileset */
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
        
        /* Create a brush with the appropriate color */
        hBrush = CreateSolidBrush(color);
        hOldBrush = SelectObject(hdc, hBrush);
        
        /* Draw the tile */
        FillRect(hdc, &rect, hBrush);
        
        /* Cleanup */
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
    
    /* For zone center tiles, add a white outline */
    if (tileValue & ZONEBIT)
    {
        FrameRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    }
    
    /* Power failure blinking - draw yellow outline */
    if ((tileValue & ZONEBIT) && !(tileValue & POWERBIT))
    {
        /* Draw a yellow outline */
        hBrush = CreateSolidBrush(RGB(255, 255, 0));
        FrameRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
    }
}

/* Draw the city map */
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
    RECT rcClient;
    char *baseName;
    char *lastSlash;
    char *lastFwdSlash;
    char nameBuffer[MAX_PATH];
    char *dot;
    
    /* Calculate the range of tiles to draw */
    startX = xOffset / TILE_SIZE;
    startY = yOffset / TILE_SIZE;
    endX = startX + (cxClient / TILE_SIZE) + 1;
    endY = startY + (cyClient / TILE_SIZE) + 1;
    
    /* Constrain to map boundaries */
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > WORLD_X) endX = WORLD_X;
    if (endY > WORLD_Y) endY = WORLD_Y;
    
    /* Clear the background - using neutral black */
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = cxClient;
    rcClient.bottom = cyClient;
    FillRect(hdc, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
    
    /* Draw visible tiles */
    for (y = startY; y < endY; y++)
    {
        for (x = startX; x < endX; x++)
        {
            screenX = x * TILE_SIZE - xOffset;
            screenY = y * TILE_SIZE - yOffset;
            
            drawTile(hdc, screenX, screenY, Map[y][x]);
        }
    }
    
    /* Draw city name if available */
    if (cityFileName[0] != '\0')
    {
        baseName = cityFileName;
        lastSlash = strrchr(cityFileName, '\\');
        lastFwdSlash = strrchr(cityFileName, '/');
        
        /* Get the filename without path */
        if (lastSlash && lastSlash > baseName)
            baseName = lastSlash + 1;
        if (lastFwdSlash && lastFwdSlash > baseName)
            baseName = lastFwdSlash + 1;
        
        /* Remove extension if present */
        lstrcpy(nameBuffer, baseName);
        dot = strrchr(nameBuffer, '.');
        if (dot)
            *dot = '\0';
        
        /* Draw the name in the top-left corner */
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOut(hdc, 10, 10, nameBuffer, lstrlen(nameBuffer));
    }
}

/* Open a city file dialog */
void openCityDialog(HWND hwnd)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH];
    
    /* Initialize filename buffer */
    szFileName[0] = '\0';
    
    /* Initialize OPENFILENAME structure */
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
        /* Load the selected city file */
        loadCity(szFileName);
    }
}

/* Create the main menu */
HMENU createMainMenu(void)
{
    HMENU hMainMenu;
    
    /* Create the menu bar */
    hMainMenu = CreateMenu();
    
    /* Create the File menu */
    hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, "&Open City...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, "E&xit");
    
    /* Create the Tileset menu */
    hTilesetMenu = CreatePopupMenu();
    populateTilesetMenu(hTilesetMenu);
    
    /* Add menus to the menu bar */
    AppendMenu(hMainMenu, MF_POPUP, (UINT)hFileMenu, "&File");
    AppendMenu(hMainMenu, MF_POPUP, (UINT)hTilesetMenu, "&Tileset");
    
    return hMainMenu;
}

/* Populate the tileset menu with available tilesets */
void populateTilesetMenu(HMENU hSubMenu)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    char fileName[MAX_PATH];
    char* dot;
    int menuId = IDM_TILESET_BASE;
    UINT menuFlags;
    
    /* Setup search path for BMP files in tilesets folder */
    strcpy(searchPath, "tilesets\\*.bmp");
    
    /* Find the first file */
    hFind = FindFirstFile(searchPath, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            /* Skip . and .. */
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
                continue;
                
            /* Extract the base name (without extension) */
            strcpy(fileName, findData.cFileName);
            dot = strrchr(fileName, '.');
            if (dot != NULL)
                *dot = '\0';
                
            /* Set menu flags - checked if this is current tileset */
            menuFlags = MF_STRING;
            if (strcmp(fileName, currentTileset) == 0)
                menuFlags |= MF_CHECKED;
                
            /* Add menu item */
            AppendMenu(hSubMenu, menuFlags, menuId++, fileName);
            
        } while (FindNextFile(hFind, &findData) && menuId < IDM_TILESET_MAX);
        
        FindClose(hFind);
    }
    else
    {
        /* Add a message indicating no tilesets found */
        AppendMenu(hSubMenu, MF_GRAYED, 0, "No tilesets found");
    }
}