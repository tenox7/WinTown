/* sprite.h - Transportation sprite system header for MicropolisNT
 * Based on original Micropolis sprite system
 */

#ifndef _SPRITE_H
#define _SPRITE_H

#include <windows.h>

/* Maximum number of sprites that can exist simultaneously */
#define MAX_SPRITES     48

/* Sprite types */
#define SPRITE_UNDEFINED    0
#define SPRITE_TRAIN        1
#define SPRITE_HELICOPTER   2
#define SPRITE_AIRPLANE     3
#define SPRITE_SHIP         4
#define SPRITE_BUS          5
#define SPRITE_MONSTER      6
#define SPRITE_TORNADO      7
#define SPRITE_EXPLOSION    8
#define SPRITE_FERRY        9

/* Train and bus groove offsets for lane positioning */
#define TRA_GROOVE_X    -39
#define TRA_GROOVE_Y    6
#define BUS_GROOVE_X    -39
#define BUS_GROOVE_Y    6

/* Sound effect IDs */
#define SOUND_HONKHONK_LOW      1
#define SOUND_HEAVY_TRAFFIC     2
#define SOUND_EXPLOSION_HIGH    3

/* Sprite structure */
typedef struct SimSprite {
    int type;           /* Sprite type (train, ship, etc.) */
    int frame;          /* Current animation frame */
    int x, y;          /* Position in world coordinates */
    int width, height; /* Sprite dimensions */
    int x_offset, y_offset; /* Drawing offset */
    int x_hot, y_hot;  /* Hot spot for collision detection */
    int orig_x, orig_y; /* Original spawn position */
    int dest_x, dest_y; /* Destination coordinates */
    int count;         /* General purpose counter */
    int sound_count;   /* Sound effect timer */
    int dir;           /* Current direction (0-7 or 0-3) */
    int new_dir;       /* Target direction for turning */
    int step;          /* Animation step counter */
    int flag;          /* General purpose flag */
    int control;       /* Control state (-1=auto, 0+=player) */
    int turn;          /* Turn state */
    int accel;         /* Acceleration value */
    int speed;         /* Movement speed */
} SimSprite;

/* Function prototypes */

/* Core sprite system */
void InitSprites(void);
SimSprite* NewSprite(int type, int x, int y);
void DestroySprite(SimSprite *sprite);
void MoveSprites(void);

/* Individual sprite behaviors */
void DoTrainSprite(SimSprite *sprite);
void DoShipSprite(SimSprite *sprite);
void DoAirplaneSprite(SimSprite *sprite);
void DoCopterSprite(SimSprite *sprite);
void DoBusSprite(SimSprite *sprite);
void DoMonsterSprite(SimSprite *sprite);
void DoTornadoSprite(SimSprite *sprite);
void DoExplosion(SimSprite *sprite);

/* Sprite generation functions */
void GenerateTrains(void);
void GenerateShips(void);
void GenerateAircraft(void);
void GenerateHelicopters(void);

/* Utility functions */
int GetSpriteCount(void);
SimSprite* GetSprite(int index);

/* External variables that need to be defined elsewhere */
extern short Map[100][120];          /* Main game map */
extern unsigned char TrfDensity[50][60]; /* Traffic density map */
extern int TotalPop;                 /* Total city population */
extern int TrafficAverage;           /* Average traffic level */
extern int CrimeAverage;             /* Average crime level */
extern int RoadTotal;                /* Total road count */
extern int SimRandom(int range);     /* Random number generator */
extern void makeFire(int x, int y);  /* Fire creation function */

#endif /* _SPRITE_H */