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


HANDLE LoadImageFromFile(LPCSTR filename, UINT fuLoad) {
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bmiHeader;
    HBITMAP hBitmap = NULL;
    RGBQUAD *colorTable = NULL;
    BYTE *bitmapBits = NULL;
    FILE *file;
    BITMAPINFO *bmi;
    size_t bmiSize, fsize, colorTableSize;
    HDC hdc;

#if(_MSC_VER > 900)
    /* Use newer LoadImage API if available */
    return LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, fuLoad);
#else
    /* Manual bitmap loading for older compilers */
    
    hdc = GetDC(NULL);
    if (hdc == NULL) {
        return NULL;
    }

    file = fopen(filename, "rb");
    if (file == NULL) {
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    if (fread(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file) != 1) {
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }
    
    if (bmfHeader.bfType != 0x4D42) {
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    if (fread(&bmiHeader, sizeof(BITMAPINFOHEADER), 1, file) != 1) {
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    bmiSize = sizeof(BITMAPINFOHEADER) + (bmiHeader.biBitCount <= 8 ? (1 << bmiHeader.biBitCount) * sizeof(RGBQUAD) : 0);
    bmi = (BITMAPINFO *)malloc(bmiSize);
    if (bmi == NULL) {
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    memcpy(&bmi->bmiHeader, &bmiHeader, sizeof(BITMAPINFOHEADER));

    if (bmiHeader.biBitCount <= 8) {
        colorTableSize = (1 << bmiHeader.biBitCount) * sizeof(RGBQUAD);
        colorTable = (RGBQUAD *)malloc(colorTableSize);
        if (colorTable == NULL) {
            free(bmi);
            fclose(file);
            ReleaseDC(NULL, hdc);
            return NULL;
        }
        if (fread(colorTable, colorTableSize, 1, file) != 1) {
            free(colorTable);
            free(bmi);
            fclose(file);
            ReleaseDC(NULL, hdc);
            return NULL;
        }
        memcpy(bmi->bmiColors, colorTable, colorTableSize);
    }

    if (!bmiHeader.biSizeImage) {
        fseek(file, 0, SEEK_END);
        fsize = ftell(file);
        bmiHeader.biSizeImage = fsize - bmfHeader.bfOffBits;
    }

    bitmapBits = (BYTE *)malloc(bmiHeader.biSizeImage);
    if (bitmapBits == NULL) {
        free(colorTable);
        free(bmi);
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    fseek(file, bmfHeader.bfOffBits, SEEK_SET);
    if (fread(bitmapBits, 1, bmiHeader.biSizeImage, file) != bmiHeader.biSizeImage) {
        free(bitmapBits);
        free(colorTable);
        free(bmi);
        fclose(file);
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    hBitmap = CreateDIBitmap(hdc, &bmiHeader, CBM_INIT, bitmapBits, bmi, DIB_RGB_COLORS);

    free(bitmapBits);
    free(colorTable);
    free(bmi);
    fclose(file);
    ReleaseDC(NULL, hdc);

    addDebugLog("LoadBitmapFromFile loaded %s\n", filename);
    return hBitmap;
#endif
}

