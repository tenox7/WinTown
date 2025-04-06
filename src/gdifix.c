/* gdifix.c - trying to deal with old NT
 * 
 */

#include "tools.h"
#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "gdifix.h"


//#if(_MSC_VER > 900)
#if 0

HANDLE LoadImageFromFile (LPCSTR filename, UINT fuLoad)	{
	return ( LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, fuLoad) );
}


#else

HANDLE LoadImageFromFile(LPCSTR filename, UINT fuLoad) {
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bmiHeader;
    HBITMAP hBitmap = NULL;
    RGBQUAD *colorTable = NULL;
    BYTE *bitmapBits = NULL;
    FILE *file;
        BITMAPINFO *bmi;
        size_t bmiSize;
    HDC hdc;

    hdc = GetDC(NULL);

    // Open the file in binary mode
    file = fopen(filename, "rb");
    if (file == NULL) {
//        MessageBox(NULL, "Failed to open the file!", "Error", MB_OK | MB_ICONERROR);
//      addDebugLog("LoadBitmapFromFile failed to load %s\n",filename);
        return NULL;
    }

    // Read the BITMAPFILEHEADER
    fread(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file);
    if (bmfHeader.bfType != 0x4D42) { // Check for 'BM' signature
        MessageBox(NULL, "Not a valid BMP file!", "Error", MB_OK | MB_ICONERROR);
        fclose(file);
        return NULL;
    }

    // Read the BITMAPINFOHEADER
    fread(&bmiHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Handle indexed color bitmaps (palettes)
        bmiSize = sizeof(BITMAPINFOHEADER) + (bmiHeader.biBitCount <= 8 ? (1 << bmiHeader.biBitCount) * sizeof(RGBQUAD) : 0);
        bmi = (BITMAPINFO *)malloc(bmiSize);
        if (bmi == NULL) {
                MessageBox(NULL, "Failed to allocate memory for BITMAPINFO!", "Error", MB_OK | MB_ICONERROR);
                free(colorTable);
                free(bitmapBits);
                fclose(file);
                return NULL;
        }

        // Copy BITMAPINFOHEADER
        memcpy(&bmi->bmiHeader, &bmiHeader, sizeof(BITMAPINFOHEADER));

        // Copy color table if present
    if (bmiHeader.biBitCount <= 8) {
        // Calculate the size of the palette
        size_t colorTableSize = (1 << bmiHeader.biBitCount) * sizeof(RGBQUAD);
        colorTable = (RGBQUAD *)malloc(colorTableSize);
        if (colorTable == NULL) {
            MessageBox(NULL, "Failed to allocate memory for color table!", "Error", MB_OK | MB_ICONERROR);
            fclose(file);
            return NULL;
        }
        fread(colorTable, colorTableSize, 1, file);
    }

    // Allocate memory for the bitmap bits
    bitmapBits = (BYTE *)malloc(bmiHeader.biSizeImage);
    if (bitmapBits == NULL) {
        MessageBox(NULL, "Failed to allocate memory for bitmap bits!", "Error", MB_OK | MB_ICONERROR);
        free(colorTable);
        fclose(file);
        return NULL;
    }

    // Move file pointer to the start of bitmap data and read it
    fseek(file, bmfHeader.bfOffBits, SEEK_SET);
    fread(bitmapBits, 1, bmiHeader.biSizeImage, file);
addDebugLog("LoadBitmapFromFile %s bytes in image %d bmiHeader.biBitCount %d\n",
                                filename,bmiHeader.biSizeImage,bmiHeader.biBitCount);

        // Copy color table if present
        if (bmiHeader.biBitCount <= 8) {
                memcpy(bmi->bmiColors, colorTable, (1 << bmiHeader.biBitCount) * sizeof(RGBQUAD));
        }

    // Create the HBITMAP
    hBitmap = CreateDIBitmap(hdc, &bmiHeader, CBM_INIT, bitmapBits,
                             bmi, DIB_RGB_COLORS);

    // Clean up allocated memory
    free(colorTable);
    free(bitmapBits);

    fclose(file);
    if (hBitmap == NULL) {
        MessageBox(NULL, "Failed to create HBITMAP!", "Error", MB_OK | MB_ICONERROR);
    }
addDebugLog("LoadBitmapFromFile loaded %s\n",filename);
    return hBitmap;
}


#endif

