/* sprite.c - Transportation sprite system for MicropolisNT
 * Based on original Micropolis w_sprite.c
 * Implements trains, ships, ferries, airplanes, and helicopters
 */

#include "sprite.h"
#include "sim.h"
#include <math.h>

/* Sprite globals */
static SimSprite GlobalSprites[MAX_SPRITES];
static int SpriteCount = 0;
static short CrashX, CrashY;
static int SpriteCycle = 0;

/* Movement direction vectors for 4-directional movement (trains, buses) */
static short Dx[5] = {0, 4, 0, -4, 0};   /* East, South, West, North, Stop */
static short Dy[5] = {-4, 0, 4, 0, 0};

/* Movement direction vectors for 8-directional movement (ships, helicopters) */  
static short BDx[9] = {0, 2, 4, 2, 0, -2, -4, -2, 0};
static short BDy[9] = {-4, -2, 0, 2, 4, 2, 0, -2, 0};

/* Precise movement for boats */
static short BPx[9] = {0, 7, 4, 7, 0, -7, -4, -7, 0};
static short BPy[9] = {-4, -7, 0, 7, 4, 7, 0, -7, 0};

/* Complex movement for aircraft */
static short CDx[12] = {0, 3, 6, 3, 0, -3, -6, -3, 0, 0, 0, 0};
static short CDy[12] = {-6, -3, 0, 3, 6, 3, 0, -3, 0, 0, 0, 0};

/* Animation frames for trains */
static short TrainPic2[5] = {1, 2, 1, 2, 5};

/* Turn direction table for sprite navigation */
static short Dir2Tab[16] = {0, 1, 2, 3, 4, 7, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0};

/* Water navigation clear table */
static short BtClrTab[8] = {RIVER, CHANNEL, POWERBASE, POWERBASE+1, RAILBASE, RAILBASE+1, BRWH, BRWV};

/* Forward declarations */
static int GetDirection(int orgX, int orgY, int desX, int desY);
static int TurnTo(int orgDir, int desDir);
static short GetChar(int x, int y);
static int CanDriveOn(int tileValue);
static int CheckSpriteCollision(SimSprite *s1, SimSprite *s2);
static int SpriteNotInBounds(SimSprite *sprite);
static void ExplodeSprite(SimSprite *sprite);
static void MakeSound(int soundId, int x, int y);
static short TryOther(int x, int y, int dir, SimSprite *sprite);
static void CheckCollisions(SimSprite *sprite);
static int IsWater(short tile);

/* Initialize sprite system */
void InitSprites(void) {
    int i;
    
    for (i = 0; i < MAX_SPRITES; i++) {
        GlobalSprites[i].type = SPRITE_UNDEFINED;
        GlobalSprites[i].frame = 0;
        GlobalSprites[i].x = 0;
        GlobalSprites[i].y = 0;
        GlobalSprites[i].width = 0;
        GlobalSprites[i].height = 0;
        GlobalSprites[i].x_offset = 0;
        GlobalSprites[i].y_offset = 0;
        GlobalSprites[i].x_hot = 0;
        GlobalSprites[i].y_hot = 0;
        GlobalSprites[i].orig_x = 0;
        GlobalSprites[i].orig_y = 0;
        GlobalSprites[i].dest_x = 0;
        GlobalSprites[i].dest_y = 0;
        GlobalSprites[i].count = 0;
        GlobalSprites[i].sound_count = 0;
        GlobalSprites[i].dir = 0;
        GlobalSprites[i].new_dir = 0;
        GlobalSprites[i].step = 0;
        GlobalSprites[i].flag = 0;
        GlobalSprites[i].control = -1;
        GlobalSprites[i].turn = 0;
        GlobalSprites[i].accel = 0;
        GlobalSprites[i].speed = 100;
    }
    
    SpriteCount = 0;
    SpriteCycle = 0;
}

/* Create a new sprite */
SimSprite* NewSprite(int type, int x, int y) {
    SimSprite *sprite;
    int i;
    
    /* Find available sprite slot */
    for (i = 0; i < MAX_SPRITES; i++) {
        if (GlobalSprites[i].type == SPRITE_UNDEFINED) {
            sprite = &GlobalSprites[i];
            break;
        }
    }
    
    if (i >= MAX_SPRITES) {
        return NULL; /* No available slots */
    }
    
    /* Initialize sprite */
    sprite->type = type;
    sprite->x = x;
    sprite->y = y;
    sprite->frame = 0;
    sprite->orig_x = x;
    sprite->orig_y = y;
    sprite->dest_x = x;
    sprite->dest_y = y;
    sprite->count = 0;
    sprite->sound_count = 0;
    sprite->dir = 0;
    sprite->new_dir = 0;
    sprite->step = 0;
    sprite->flag = 0;
    sprite->control = -1;
    sprite->turn = 0;
    sprite->accel = 0;
    sprite->speed = 100;
    
    /* Set sprite-specific properties */
    switch (type) {
        case SPRITE_TRAIN:
            sprite->width = 32;
            sprite->height = 32;
            sprite->x_offset = TRA_GROOVE_X;
            sprite->y_offset = TRA_GROOVE_Y;
            sprite->x_hot = 40;
            sprite->y_hot = -8;
            sprite->frame = 1;
            break;
            
        case SPRITE_SHIP:
            sprite->width = 48;
            sprite->height = 48;
            sprite->x_offset = 0;
            sprite->y_offset = 0;
            sprite->x_hot = 24;
            sprite->y_hot = 0;
            sprite->frame = 0;
            break;
            
        case SPRITE_AIRPLANE:
            sprite->width = 48;
            sprite->height = 48;
            sprite->x_offset = 24;
            sprite->y_offset = 16;
            sprite->x_hot = 24;
            sprite->y_hot = 16;
            sprite->frame = 3;
            break;
            
        case SPRITE_HELICOPTER:
            sprite->width = 32;
            sprite->height = 32;
            sprite->x_offset = 32;
            sprite->y_offset = 16;
            sprite->x_hot = 16;
            sprite->y_hot = 16;
            sprite->frame = 0;
            break;
            
        case SPRITE_BUS:
            sprite->width = 32;
            sprite->height = 32;
            sprite->x_offset = BUS_GROOVE_X;
            sprite->y_offset = BUS_GROOVE_Y;
            sprite->x_hot = 40;
            sprite->y_hot = -8;
            sprite->frame = 1;
            break;
    }
    
    SpriteCount++;
    return sprite;
}

/* Remove sprite from system */
void DestroySprite(SimSprite *sprite) {
    if (sprite && sprite->type != SPRITE_UNDEFINED) {
        sprite->type = SPRITE_UNDEFINED;
        SpriteCount--;
    }
}

/* Update all sprites */
void MoveSprites(void) {
    int i;
    SimSprite *sprite;
    
    SpriteCycle++;
    if (SpriteCycle > 1023) {
        SpriteCycle = 0;
    }
    
    for (i = 0; i < MAX_SPRITES; i++) {
        sprite = &GlobalSprites[i];
        
        if (sprite->type == SPRITE_UNDEFINED) {
            continue;
        }
        
        /* Update sprite based on type */
        switch (sprite->type) {
            case SPRITE_TRAIN:
                DoTrainSprite(sprite);
                break;
                
            case SPRITE_SHIP:
                DoShipSprite(sprite);
                break;
                
            case SPRITE_AIRPLANE:
                DoAirplaneSprite(sprite);
                break;
                
            case SPRITE_HELICOPTER:
                DoCopterSprite(sprite);
                break;
                
            case SPRITE_BUS:
                DoBusSprite(sprite);
                break;
                
            case SPRITE_EXPLOSION:
                DoExplosion(sprite);
                break;
        }
        
        /* Check bounds */
        if (SpriteNotInBounds(sprite)) {
            if (sprite->type == SPRITE_AIRPLANE || sprite->type == SPRITE_HELICOPTER) {
                /* Aircraft can leave bounds temporarily */
                continue;
            }
            DestroySprite(sprite);
        }
    }
}

/* Train sprite behavior */
void DoTrainSprite(SimSprite *sprite) {
    short tile, dir;
    
    if (sprite->frame == 3 || sprite->frame == 4) {
        sprite->frame = TrainPic2[sprite->dir];
    }
    
    sprite->x += Dx[sprite->dir];
    sprite->y += Dy[sprite->dir];
    
    if (sprite->count > 0) {
        sprite->count--;
    } else {
        sprite->count = 0;
        
        if (sprite->dir == 4) { /* Stopped */
            if (SimRandom(8) == 0) {
                dir = SimRandom(4);
                sprite->new_dir = dir;
                sprite->frame = TrainPic2[dir];
            }
        } else {
            tile = GetChar(sprite->x + sprite->x_hot, sprite->y + sprite->y_hot);
            
            if (tile == HRAIL || (tile >= LHRAIL && tile <= LVRAIL10)) {
                if (sprite->dir == 1 || sprite->dir == 3) { /* North-South movement */
                    if (SimRandom(4) == 0) {
                        sprite->dir = 4; /* Stop */
                        sprite->frame = 5;
                        sprite->count = 30 + SimRandom(30);
                    }
                } else { /* East-West movement */
                    sprite->new_dir = sprite->dir;
                }
            } else if (tile == VRAIL || (tile >= LHRAIL && tile <= LVRAIL10)) {
                if (sprite->dir == 0 || sprite->dir == 2) { /* East-West movement */
                    if (SimRandom(4) == 0) {
                        sprite->dir = 4; /* Stop */
                        sprite->frame = 5;
                        sprite->count = 30 + SimRandom(30);
                    }
                } else { /* North-South movement */
                    sprite->new_dir = sprite->dir;
                }
            } else {
                /* Not on rail - try to find rail */
                dir = TryOther(sprite->x + sprite->x_hot, sprite->y + sprite->y_hot, sprite->dir, sprite);
                if (dir != sprite->dir) {
                    sprite->new_dir = dir;
                    sprite->frame = TrainPic2[dir];
                } else {
                    /* Derail */
                    ExplodeSprite(sprite);
                    return;
                }
            }
        }
        
        if (sprite->new_dir != sprite->dir) {
            sprite->dir = sprite->new_dir;
            sprite->frame = TrainPic2[sprite->dir];
        }
    }
}

/* Ship sprite behavior */
void DoShipSprite(SimSprite *sprite) {
    short tile, dir;
    int i, dx, dy;
    
    if (sprite->sound_count > 0) {
        sprite->sound_count--;
    }
    
    if (sprite->count > 0) {
        sprite->count--;
    }
    
    if (sprite->count == 0) {
        /* Random movement and horn sounds */
        if (SimRandom(7) == 0) {
            MakeSound(SOUND_HONKHONK_LOW, sprite->x, sprite->y);
            sprite->sound_count = 40;
        }
        
        if (sprite->dir == 8) { /* Stopped */
            if (SimRandom(16) == 0) {
                dir = SimRandom(8);
                sprite->new_dir = dir;
            }
        } else {
            /* Check if can continue in current direction */
            dx = sprite->x + BDx[sprite->dir];
            dy = sprite->y + BDy[sprite->dir];
            
            tile = GetChar(dx + sprite->x_hot, dy + sprite->y_hot);
            
            if (IsWater(tile)) {
                sprite->x = dx;
                sprite->y = dy;
            } else {
                /* Find new direction */
                dir = 8; /* Default to stop */
                for (i = 0; i < 8; i++) {
                    dx = sprite->x + BDx[i];
                    dy = sprite->y + BDy[i];
                    tile = GetChar(dx + sprite->x_hot, dy + sprite->y_hot);
                    
                    if (IsWater(tile)) {
                        dir = i;
                        break;
                    }
                }
                sprite->new_dir = dir;
            }
        }
        
        sprite->count = 9;
        
        if (sprite->new_dir != sprite->dir) {
            sprite->dir = TurnTo(sprite->dir, sprite->new_dir);
            sprite->frame = sprite->dir;
        }
    } else {
        sprite->x += BPx[sprite->dir];
        sprite->y += BPy[sprite->dir];
    }
}

/* Airplane sprite behavior */
void DoAirplaneSprite(SimSprite *sprite) {
    int dx, dy;
    
    if (sprite->control < 0) {
        /* Taking off */
        if (sprite->frame > 8) {
            sprite->x += CDx[sprite->dir];
            sprite->y += CDy[sprite->dir];
            
            if (sprite->control < -1) {
                sprite->control++;
            } else {
                /* Pick random destination */
                sprite->dest_x = sprite->orig_x - 30 + SimRandom(60);
                sprite->dest_y = sprite->orig_y - 30 + SimRandom(60);
                
                if (sprite->dest_x < 0) sprite->dest_x = 30;
                if (sprite->dest_y < 0) sprite->dest_y = 30;
                if (sprite->dest_x >= WORLD_W) sprite->dest_x = WORLD_W - 30;
                if (sprite->dest_y >= WORLD_H) sprite->dest_y = WORLD_H - 30;
                
                sprite->control = 0;
            }
        } else {
            sprite->frame++;
        }
    } else {
        /* In flight */
        sprite->count--;
        if (sprite->count < 0) {
            sprite->count = 3;
            
            /* Navigate toward destination */
            if (sprite->control == 0) {
                dx = sprite->dest_x - sprite->x;
                dy = sprite->dest_y - sprite->y;
                
                if (abs(dx) < 8 && abs(dy) < 8) {
                    /* Reached destination - return to airport */
                    sprite->dest_x = sprite->orig_x;
                    sprite->dest_y = sprite->orig_y;
                    sprite->control = 1;
                } else {
                    sprite->dir = GetDirection(sprite->x, sprite->y, sprite->dest_x, sprite->dest_y);
                }
            } else {
                /* Returning to airport */
                dx = sprite->dest_x - sprite->x;
                dy = sprite->dest_y - sprite->y;
                
                if (abs(dx) < 8 && abs(dy) < 8) {
                    /* Landing */
                    DestroySprite(sprite);
                    return;
                } else {
                    sprite->dir = GetDirection(sprite->x, sprite->y, sprite->dest_x, sprite->dest_y);
                }
            }
            
            sprite->frame = sprite->dir;
        }
        
        sprite->x += CDx[sprite->dir];
        sprite->y += CDy[sprite->dir];
    }
    
    /* Check for collisions with helicopters */
    CheckCollisions(sprite);
}

/* Helicopter sprite behavior */
void DoCopterSprite(SimSprite *sprite) {
    int dx, dy, z;
    
    if (sprite->sound_count > 0) {
        sprite->sound_count--;
    }
    
    if (sprite->control < 0) {
        /* Autonomous mode - seek monsters or report traffic */
        if (sprite->count > 0) {
            sprite->count--;
        } else {
            sprite->count = 100;
            
            /* Report traffic */
            dx = sprite->x >> 4;
            dy = sprite->y >> 4;
            
            if (dx >= 0 && dx < WORLD_X/2 && dy >= 0 && dy < WORLD_Y/2) {
                z = TrfDensity[dy][dx];
                if (z > 170 && SimRandom(7) == 0) {
                    /* Report heavy traffic */
                    MakeSound(SOUND_HEAVY_TRAFFIC, sprite->x, sprite->y);
                    sprite->sound_count = 200;
                }
            }
            
            /* Look for monsters or disasters */
            /* This would scan for monster sprites and navigate toward them */
            
            /* Random movement if no target */
            if (SimRandom(5) == 0) {
                sprite->dir = SimRandom(8);
            }
        }
    } else {
        /* Player controlled mode */
        /* Navigation controlled by player input */
    }
    
    sprite->x += BDx[sprite->dir];
    sprite->y += BDy[sprite->dir];
    
    /* Update animation frame */
    sprite->frame = sprite->dir;
    
    /* Check for collisions with aircraft */
    CheckCollisions(sprite);
}

/* Bus sprite behavior */
void DoBusSprite(SimSprite *sprite) {
    short tile, dir;
    int dx, dy, speed;
    
    /* Lane correction - buses drift to proper lane */
    if (sprite->dir == 0 || sprite->dir == 2) { /* East-West */
        if (sprite->y & 1) {
            sprite->y--;
        }
    } else { /* North-South */
        if (sprite->x & 1) {
            sprite->x--;
        }
    }
    
    /* Check traffic density for speed adjustment */
    dx = sprite->x >> 5;
    dy = sprite->y >> 5;
    
    if (dx >= 0 && dx < WORLD_X/2 && dy >= 0 && dy < WORLD_Y/2) {
        speed = TrfDensity[dy][dx] >> 1;
        if (speed < 1) speed = 1;
        if (speed > 8) speed = 8;
        
        if (sprite->count > 0) {
            sprite->count--;
            return;
        }
        sprite->count = speed;
    }
    
    if (sprite->frame == 3 || sprite->frame == 4) {
        sprite->frame = TrainPic2[sprite->dir];
    }
    
    /* Move bus */
    sprite->x += Dx[sprite->dir];
    sprite->y += Dy[sprite->dir];
    
    /* Check road surface */
    tile = GetChar(sprite->x + sprite->x_hot, sprite->y + sprite->y_hot);
    
    if (!CanDriveOn(tile)) {
        /* Try to find road */
        dir = TryOther(sprite->x + sprite->x_hot, sprite->y + sprite->y_hot, sprite->dir, sprite);
        if (dir != sprite->dir) {
            sprite->dir = dir;
            sprite->frame = TrainPic2[dir];
        } else {
            /* Stuck - remove bus */
            DestroySprite(sprite);
        }
    }
}

/* Explosion sprite behavior */
void DoExplosion(SimSprite *sprite) {
    if (sprite->frame == 0) {
        /* Start fire at explosion site */
        makeFire(sprite->x >> 4, sprite->y >> 4);
        MakeSound(SOUND_EXPLOSION_HIGH, sprite->x, sprite->y);
    }
    
    sprite->frame++;
    
    if (sprite->frame > 6) {
        DestroySprite(sprite);
    }
}

/* Helper function to determine if tile is water */
static int IsWater(short tile) {
    int i;
    
    if (tile == RIVER || tile == CHANNEL) {
        return 1;
    }
    
    for (i = 0; i < 8; i++) {
        if (tile == BtClrTab[i]) {
            return 1;
        }
    }
    
    return 0;
}

/* Helper function to check if bus can drive on tile */
static int CanDriveOn(int tileValue) {
    return (tileValue >= ROADBASE && tileValue <= LASTROAD) ||
           (tileValue >= BRWH && tileValue <= BRWV) ||
           (tileValue == HROADPOWER) ||
           (tileValue == VROADPOWER);
}

/* Try to find alternate direction */
static short TryOther(int x, int y, int dir, SimSprite *sprite) {
    short tile;
    int i, start_dir;
    
    start_dir = (dir + 1) & 3;
    
    for (i = start_dir; i != dir; i = (i + 1) & 3) {
        tile = GetChar(x + Dx[i], y + Dy[i]);
        
        if (sprite->type == SPRITE_TRAIN) {
            if (tile >= RAILBASE && tile <= LASTRAIL) {
                return i;
            }
        } else if (sprite->type == SPRITE_BUS) {
            if (CanDriveOn(tile)) {
                return i;
            }
        }
    }
    
    return dir; /* No alternate found */
}

/* Get direction from origin to destination */
static int GetDirection(int orgX, int orgY, int desX, int desY) {
    int dispX, dispY;
    int dir = 0;
    
    dispX = desX - orgX;
    dispY = desY - orgY;
    
    if (dispX < 0) {
        if (dispY < 0) {
            dir = 5; /* Northwest */
        } else if (dispY == 0) {
            dir = 6; /* West */
        } else {
            dir = 7; /* Southwest */
        }
    } else if (dispX == 0) {
        if (dispY < 0) {
            dir = 0; /* North */
        } else if (dispY > 0) {
            dir = 4; /* South */
        } else {
            dir = 8; /* Stationary */
        }
    } else {
        if (dispY < 0) {
            dir = 1; /* Northeast */
        } else if (dispY == 0) {
            dir = 2; /* East */
        } else {  
            dir = 3; /* Southeast */
        }
    }
    
    return dir;
}

/* Turn toward desired direction */
static int TurnTo(int orgDir, int desDir) {
    if (orgDir == desDir) {
        return orgDir;
    }
    
    if ((orgDir + 1) & 7 == desDir) {
        return desDir;
    }
    
    if ((orgDir - 1) & 7 == desDir) {
        return desDir;
    }
    
    return orgDir; /* Need more frames to turn */
}

/* Get tile at coordinates */
static short GetChar(int x, int y) {
    int mapX, mapY;
    
    mapX = x >> 4; /* Divide by 16 */
    mapY = y >> 4;
    
    if (mapX < 0 || mapX >= WORLD_X || mapY < 0 || mapY >= WORLD_Y) {
        return DIRT;
    }
    
    return Map[mapY][mapX] & LOMASK;
}

/* Check sprite collision */
static int CheckSpriteCollision(SimSprite *s1, SimSprite *s2) {
    int dx, dy, dist;
    
    dx = s1->x - s2->x;
    dy = s1->y - s2->y;
    dist = dx * dx + dy * dy;
    
    return (dist < 900); /* Collision if within 30 pixels */
}

/* Check if sprite is out of bounds */
static int SpriteNotInBounds(SimSprite *sprite) {
    return (sprite->x < -100 || sprite->x > (WORLD_X << 4) + 100 ||
            sprite->y < -100 || sprite->y > (WORLD_Y << 4) + 100);
}

/* Create explosion at sprite location */
static void ExplodeSprite(SimSprite *sprite) {
    SimSprite *exp;
    
    CrashX = (sprite->x + sprite->x_hot) >> 4;
    CrashY = (sprite->y + sprite->y_hot) >> 4;
    
    exp = NewSprite(SPRITE_EXPLOSION, sprite->x, sprite->y);
    if (exp) {
        exp->width = 48;
        exp->height = 48;
        exp->x_offset = 24;
        exp->y_offset = 16;
        exp->x_hot = 24;
        exp->y_hot = 16;
        exp->frame = 0;
    }
    
    DestroySprite(sprite);
}

/* Check collisions between sprites */
static void CheckCollisions(SimSprite *sprite) {
    int i;
    SimSprite *other;
    
    for (i = 0; i < MAX_SPRITES; i++) {
        other = &GlobalSprites[i];
        
        if (other->type == SPRITE_UNDEFINED || other == sprite) {
            continue;
        }
        
        if ((sprite->type == SPRITE_AIRPLANE && other->type == SPRITE_HELICOPTER) ||
            (sprite->type == SPRITE_HELICOPTER && other->type == SPRITE_AIRPLANE)) {
            
            if (CheckSpriteCollision(sprite, other)) {
                ExplodeSprite(sprite);
                ExplodeSprite(other);
                return;
            }
        }
    }
}

/* Generate trains at rail stations when population threshold is met */
void GenerateTrains(void) {
    int x, y;
    short tile;
    
    if (TotalPop < 20) {
        return; /* Not enough population */
    }
    
    if (SpriteCount >= MAX_SPRITES - 5) {
        return; /* Too many sprites */
    }
    
    if (SimRandom(25) != 0) {
        return; /* Random generation */
    }
    
    /* Find rail stations */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            tile = Map[y][x] & LOMASK;
            
            if (tile >= RAILBASE && tile <= LASTRAIL) {
                if (SimRandom(1000) == 0) {
                    /* Generate train here */
                    NewSprite(SPRITE_TRAIN, x << 4, y << 4);
                    return;
                }
            }
        }
    }
}

/* Generate ships at seaports */
void GenerateShips(void) {
    int x, y;
    short tile;
    
    if (SpriteCount >= MAX_SPRITES - 5) {
        return;
    }
    
    if (SimRandom(100) != 0) {
        return;
    }
    
    /* Find seaports */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            tile = Map[y][x] & LOMASK;
            
            if (tile >= PORTBASE && tile <= LASTPORT) {
                if (SimRandom(500) == 0) {
                    /* Generate ship at port */
                    NewSprite(SPRITE_SHIP, x << 4, y << 4);
                    return;
                }
            }
        }
    }
}

/* Generate aircraft at airports */
void GenerateAircraft(void) {
    int x, y;
    short tile;
    
    if (SpriteCount >= MAX_SPRITES - 5) {
        return;
    }
    
    if (SimRandom(50) != 0) {
        return;
    }
    
    /* Find airports */
    for (y = 0; y < WORLD_Y; y++) {
        for (x = 0; x < WORLD_X; x++) {
            tile = Map[y][x] & LOMASK;
            
            if (tile >= AIRPORTBASE && tile <= AIRPORT) {
                if (SimRandom(300) == 0) {
                    /* Generate aircraft - favor airplanes at airports */
                    if (SimRandom(4) < 3) {
                        NewSprite(SPRITE_AIRPLANE, x << 4, y << 4);
                    } else {
                        NewSprite(SPRITE_HELICOPTER, x << 4, y << 4);
                    }
                    return;
                }
            }
        }
    }
}

/* Generate helicopters for traffic monitoring and disasters */
void GenerateHelicopters(void) {
    int x, y;
    SimSprite *copter;
    
    if (SpriteCount >= MAX_SPRITES - 5) {
        return;
    }
    
    /* Much more frequent helicopter spawning for better gameplay */
    
    /* 1. Traffic monitoring helicopters */
    if (TrafficAverage > 80 && SimRandom(50) == 0) {
        /* Spawn near high traffic areas */
        x = (SimRandom(WORLD_X - 20) + 10) << 4;
        y = (SimRandom(WORLD_Y - 20) + 10) << 4;
        
        copter = NewSprite(SPRITE_HELICOPTER, x, y);
        if (copter) {
            copter->control = -1; /* Autonomous mode */
            return;
        }
    }
    
    /* 2. Crime monitoring helicopters */
    if (CrimeAverage > 50 && SimRandom(80) == 0) {
        /* Spawn near center of city */
        x = (WORLD_X / 2 + SimRandom(20) - 10) << 4;
        y = (WORLD_Y / 2 + SimRandom(20) - 10) << 4;
        
        copter = NewSprite(SPRITE_HELICOPTER, x, y);
        if (copter) {
            copter->control = -1; /* Autonomous mode */
            return;
        }
    }
    
    /* 3. Random patrol helicopters in medium cities */
    if (TotalPop > 10000 && SimRandom(100) == 0) {
        /* Spawn anywhere in the city */
        x = SimRandom(WORLD_X) << 4;
        y = SimRandom(WORLD_Y) << 4;
        
        copter = NewSprite(SPRITE_HELICOPTER, x, y);
        if (copter) {
            copter->control = -1; /* Autonomous mode */
            return;
        }
    }
    
    /* 4. Guaranteed periodic helicopter for any city with roads */
    if (RoadTotal > 10 && SimRandom(25) == 0) {
        /* Spawn a helicopter somewhere over the city */
        x = (SimRandom(WORLD_X - 10) + 5) << 4;
        y = (SimRandom(WORLD_Y - 10) + 5) << 4;
        
        copter = NewSprite(SPRITE_HELICOPTER, x, y);
        if (copter) {
            copter->control = -1; /* Autonomous mode */
            return;
        }
    }
}

/* Placeholder sound function - to be implemented with Windows audio */
static void MakeSound(int soundId, int x, int y) {
    /* TODO: Implement Windows NT sound system */
    /* For now, just log to debug file */
    /* No MessageBox allowed per guidelines */
}

/* Get sprite count for debugging */
int GetSpriteCount(void) {
    return SpriteCount;
}

/* Get sprite by index for rendering */
SimSprite* GetSprite(int index) {
    if (index >= 0 && index < MAX_SPRITES) {
        if (GlobalSprites[index].type != SPRITE_UNDEFINED) {
            return &GlobalSprites[index];
        }
    }
    return NULL;
}