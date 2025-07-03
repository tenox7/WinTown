/* tiles.c - Centralized tile management for MicropolisNT
 * All map tile changes go through this module for debugging and validation
 */

#include "sim.h"
#include "tiles.h"
#include <stdio.h>

/* Debug and statistics globals */
int tileDebugEnabled = 0;
long tileChangeCount = 0;
long tileErrorCount = 0;

/* Tile change log file */
static FILE* tileLogFile = (FILE*)0;

/* Initialize tile logging */
static int initTileLogging() {
    if (!tileLogFile && tileDebugEnabled) {
        tileLogFile = fopen("tiles.log", "a");
        if (tileLogFile) {
            fprintf(tileLogFile, "=== Tile logging started ===\n");
            fflush(tileLogFile);
        }
    }
    return (tileLogFile != (FILE*)0);
}

/* Log tile change */
static int logTileChange(int x, int y, int oldTile, int newTile, char* caller) {
    if (!tileDebugEnabled || !initTileLogging()) {
        return 0;
    }
    
    fprintf(tileLogFile, "TILE[%d,%d]: %04X->%04X by %s\n", 
            x, y, oldTile, newTile, caller ? caller : "unknown");
    fflush(tileLogFile);
    return 1;
}

/* Validate coordinates are within map bounds */
int validateTileCoords(int x, int y) {
    return (x >= 0 && x < WORLD_X && y >= 0 && y < WORLD_Y);
}

/* Validate tile value is reasonable */
int validateTileValue(int tile) {
    int baseTile;
    baseTile = tile & LOMASK;
    return (baseTile >= 0 && baseTile < 1024);
}

/* Get tile value at coordinates */
int getMapTile(int x, int y) {
    if (!validateTileCoords(x, y)) {
        return -1;
    }
    return Map[y][x];
}

/* Get only flags from tile at coordinates */
int getMapFlags(int x, int y) {
    if (!validateTileCoords(x, y)) {
        return -1;
    }
    return Map[y][x] & ~LOMASK;
}

/* Main tile setting function - all tile changes go through here */
int setMapTile(int x, int y, int tile, int flags, int operation, char* caller) {
    int oldTile, newTile;
    
    /* Validate coordinates */
    if (!validateTileCoords(x, y)) {
        if (tileDebugEnabled) {
            logTileChange(x, y, -1, -1, caller);
        }
        tileErrorCount++;
        return 0;
    }
    
    /* Get current tile */
    oldTile = Map[y][x];
    
    /* Calculate new tile value based on operation */
    switch (operation) {
        case TILE_SET_REPLACE:
            /* Complete replacement */
            newTile = tile | flags;
            break;
            
        case TILE_SET_PRESERVE:
            /* Set tile preserving existing flags */
            newTile = (oldTile & ~LOMASK) | (tile & LOMASK) | flags;
            break;
            
        case TILE_SET_FLAGS:
            /* Set only flags, preserve tile and existing flags */
            newTile = oldTile | flags;
            break;
            
        case TILE_CLEAR_FLAGS:
            /* Clear specific flags */
            newTile = oldTile & ~flags;
            break;
            
        case TILE_TOGGLE_FLAGS:
            /* Toggle specific flags */
            newTile = oldTile ^ flags;
            break;
            
        default:
            /* Invalid operation */
            tileErrorCount++;
            return 0;
    }
    
    /* Validate new tile value */
    if (!validateTileValue(newTile)) {
        if (tileDebugEnabled) {
            fprintf(tileLogFile, "ERROR: Invalid tile value %04X at [%d,%d] by %s\n",
                    newTile, x, y, caller ? caller : "unknown");
            fflush(tileLogFile);
        }
        tileErrorCount++;
        return 0;
    }
    
    /* Log the change if debugging enabled */
    if (tileDebugEnabled && oldTile != newTile) {
        logTileChange(x, y, oldTile, newTile, caller);
    }
    
    /* Make the change */
    Map[y][x] = newTile;
    tileChangeCount++;
    
    return 1;
}

/* Enable/disable debug logging */
int enableTileDebug(int enable) {
    tileDebugEnabled = enable;
    if (!enable && tileLogFile) {
        fprintf(tileLogFile, "=== Tile logging stopped ===\n");
        fclose(tileLogFile);
        tileLogFile = (FILE*)0;
    }
    return tileDebugEnabled;
}

/* Reset statistics */
int resetTileStats() {
    tileChangeCount = 0;
    tileErrorCount = 0;
    return 1;
}

/* Print statistics */
int printTileStats() {
    printf("Tile Statistics:\n");
    printf("  Changes: %ld\n", tileChangeCount);
    printf("  Errors: %ld\n", tileErrorCount);
    if (tileLogFile) {
        printf("  Debug log: tiles.log\n");
    }
    return 1;
}