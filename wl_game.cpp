#include <math.h>
#include <stdint.h>

#include <QDebug>

#include "wl_def.h"

#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wlitem.h"
#include "wlrender.h"
#include "wlplay.h"
#include "wlpaint.h"
#include "wluser.h"
#include "main.h"

extern WLCache*  wlcache;
extern WLInput*  wlinput;
extern WLAudio*  wlaudio;
extern WLItem*   wlitem;
extern WLRender* wlrender;
extern WLPlay*   wlplay;
extern WLPaint*  wlpaint;
extern WLUser*   wluser;
extern GameEngine* engine;

extern QImage*  g_Image;
extern QString* g_sound;

qint8    ingame;
qint8    fizzlein;
gametype gamestate;
quint8   bordercol = VIEWCOLOR;        // color of the Change View/Ingame border

int ElevatorBackTo[]={1,1,7,3,5,3};

void SetupGameLevel (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void GameLoop (void);

int leftchannel, rightchannel;
#define ATABLEMAX 15
quint8 righttable[ATABLEMAX][ATABLEMAX * 2] = {
    { 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
quint8 lefttable[ATABLEMAX][ATABLEMAX * 2] = {
    { 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
    { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

void SetSoundLoc(qint32 gx,qint32 gy)
{
    qint32   xt,yt;
    int     x,y;

    gx -= viewx;
    gy -= viewy;
    xt = FixedMul(gx,viewcos);
    yt = FixedMul(gy,viewsin);
    x = (xt - yt) >> TILESHIFT;
    xt = FixedMul(gx,viewsin);
    yt = FixedMul(gy,viewcos);
    y = (yt + xt) >> TILESHIFT;
    if (y >= ATABLEMAX)
        y = ATABLEMAX - 1;
    else if (y <= -ATABLEMAX)
        y = -ATABLEMAX;
    if (x < 0)
        x = -x;
    if (x >= ATABLEMAX)
        x = ATABLEMAX - 1;
    leftchannel  =  lefttable[x][y + ATABLEMAX];
    rightchannel = righttable[x][y + ATABLEMAX];
}

void PlaySoundLocGlobal(quint16 s,qint32 gx,qint32 gy)
{
    SetSoundLoc(gx, gy);
    wlaudio->positionSound(leftchannel, rightchannel);

    int channel = wlaudio->playSound(g_sound[s]);
    if(channel) {
        wlaudio->channelSoundPos(channel - 1)->globalsoundx = gx;
        wlaudio->channelSoundPos(channel - 1)->globalsoundy = gy;
        wlaudio->channelSoundPos(channel - 1)->valid = 1;
    }
}

void UpdateSoundLoc(void)
{
    for(int i = 0; i < 8; i++) {
        if(wlaudio->channelSoundPos(i)->valid) {
            SetSoundLoc(wlaudio->channelSoundPos(i)->globalsoundx,
                        wlaudio->channelSoundPos(i)->globalsoundy);
            wlaudio->setPosition(i, leftchannel, rightchannel);
        }
    }
}

static void ScanInfoPlane(void)
{
    unsigned x,y;
    int      tile;
    quint16     *start;

    start = (quint16*)wlcache->map(1);
    for (y=0;y<mapheight;y++) {
        for (x=0;x<mapwidth;x++) {
            tile = *start++;
            if (!tile)
                continue;

            switch (tile) {
            case 19:
            case 20:
            case 21:
            case 22:
                SpawnPlayer(x,y,NORTH+tile-19);
                break;

            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:

            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:

            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:

            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53:
            case 54:

            case 55:
            case 56:
            case 57:
            case 58:
            case 59:
            case 60:
            case 61:
            case 62:

            case 63:
            case 64:
            case 65:
            case 66:
            case 67:
            case 68:
            case 69:
            case 70:
            case 71:
            case 72:
                wlitem->spawnStatic(x, y, tile - 23);
                break;
            case 98:
                if (!loadedgame)
                    gamestate.secrettotal++;
                break;
            case 180:
            case 181:
            case 182:
            case 183:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 144:
            case 145:
            case 146:
            case 147:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 108:
            case 109:
            case 110:
            case 111:
                SpawnStand(en_guard,x,y,tile-108);
                break;
            case 184:
            case 185:
            case 186:
            case 187:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 148:
            case 149:
            case 150:
            case 151:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 112:
            case 113:
            case 114:
            case 115:
                SpawnPatrol(en_guard,x,y,tile-112);
                break;

            case 124:
                SpawnDeadGuard (x,y);
                break;
            case 188:
            case 189:
            case 190:
            case 191:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 152:
            case 153:
            case 154:
            case 155:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 116:
            case 117:
            case 118:
            case 119:
                SpawnStand(en_officer,x,y,tile-116);
                break;
            case 192:
            case 193:
            case 194:
            case 195:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 156:
            case 157:
            case 158:
            case 159:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 120:
            case 121:
            case 122:
            case 123:
                SpawnPatrol(en_officer,x,y,tile-120);
                break;
            case 198:
            case 199:
            case 200:
            case 201:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 162:
            case 163:
            case 164:
            case 165:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 126:
            case 127:
            case 128:
            case 129:
                SpawnStand(en_ss,x,y,tile-126);
                break;
            case 202:
            case 203:
            case 204:
            case 205:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 166:
            case 167:
            case 168:
            case 169:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 130:
            case 131:
            case 132:
            case 133:
                SpawnPatrol(en_ss,x,y,tile-130);
                break;
            case 206:
            case 207:
            case 208:
            case 209:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 170:
            case 171:
            case 172:
            case 173:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 134:
            case 135:
            case 136:
            case 137:
                SpawnStand(en_dog,x,y,tile-134);
                break;
            case 210:
            case 211:
            case 212:
            case 213:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 36;
            case 174:
            case 175:
            case 176:
            case 177:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 36;
            case 138:
            case 139:
            case 140:
            case 141:
                SpawnPatrol(en_dog,x,y,tile-138);
                break;
            case 214:
                SpawnBoss (x,y);
                break;
            case 197:
                SpawnGretel (x,y);
                break;
            case 215:
                SpawnGift (x,y);
                break;
            case 179:
                SpawnFat (x,y);
                break;
            case 196:
                SpawnSchabbs (x,y);
                break;
            case 160:
                SpawnFakeHitler (x,y);
                break;
            case 178:
                SpawnHitler (x,y);
                break;
            case 252:
            case 253:
            case 254:
            case 255:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 18;
            case 234:
            case 235:
            case 236:
            case 237:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 18;
            case 216:
            case 217:
            case 218:
            case 219:
                SpawnStand(en_mutant,x,y,tile-216);
                break;

            case 256:
            case 257:
            case 258:
            case 259:
                if (gamestate.difficulty<gd_hard)
                    break;
                tile -= 18;
            case 238:
            case 239:
            case 240:
            case 241:
                if (gamestate.difficulty<gd_medium)
                    break;
                tile -= 18;
            case 220:
            case 221:
            case 222:
            case 223:
                SpawnPatrol(en_mutant,x,y,tile-220);
                break;
            case 224:
                SpawnGhosts (en_blinky,x,y);
                break;
            case 225:
                SpawnGhosts (en_clyde,x,y);
                break;
            case 226:
                SpawnGhosts (en_pinky,x,y);
                break;
            case 227:
                SpawnGhosts (en_inky,x,y);
                break;
            }
        }
    }
}

void SetupGameLevel (void)
{
    int  x,y;
    quint16 *map;
    quint16 tile;


    if (!loadedgame) {
        gamestate.TimeCount
                = gamestate.secrettotal
                = gamestate.killtotal
                = gamestate.treasuretotal
                = gamestate.secretcount
                = gamestate.killcount
                = gamestate.treasurecount
                = facetimes = 0;
        wlitem->setpWallPos(0);
        wlitem->setpWallState(0);
        LastAttacker = NULL;
        killerobj = NULL;
    }

    if (demoplayback || demorecord)
        wluser->US_InitRndT(false);
    else
        wluser->US_InitRndT(true);

    wlcache->cacheMap(gamestate.mapon+10*gamestate.episode);
    wlcache->setCurrentMap(wlcache->currentMap()-gamestate.episode*10);
    for (int xx = 0; xx < mapwidth; xx++)
        for (int yy = 0; yy < mapheight; yy++)
            wlcache->setFog(false, xx, yy);
    memset (tilemap,0,sizeof(tilemap));
    memset (actorat,0,sizeof(actorat));
    map = (quint16*)wlcache->map(0);
    for (y=0;y<mapheight;y++) {
        for (x=0;x<mapwidth;x++) {
            tile = *map++;
            if (tile<AREATILE) {
                // solid wall
                tilemap[x][y] = (quint8) tile;
                actorat[x][y] = (objtype *)(uintptr_t) tile;
            } else {
                // area floor
                tilemap[x][y] = 0;
                actorat[x][y] = 0;
            }
        }
    }
    wlplay->initActorList();
    wlitem->initDoorList();
    wlitem->initStaticList();

    map = (quint16*)wlcache->map(0);
    for (y=0;y<mapheight;y++) {
        for (x=0;x<mapwidth;x++) {
            tile = *map++;
            if (tile >= 90 && tile <= 101) {
                // door
                switch (tile) {
                case 90:
                case 92:
                case 94:
                case 96:
                case 98:
                case 100:
                    wlitem->spawnDoor(x, y, 1, (tile - 90) / 2);
                    break;
                case 91:
                case 93:
                case 95:
                case 97:
                case 99:
                case 101:
                    wlitem->spawnDoor(x, y, 0, (tile - 91) / 2);
                    break;
                }
            }
        }
    }
    ScanInfoPlane ();
    map = (quint16*)wlcache->map(0);
    for (y=0;y<mapheight;y++) {
        for (x=0;x<mapwidth;x++) {
            tile = *map++;
            if (tile == AMBUSHTILE) {
                tilemap[x][y] = 0;
                if ( (unsigned)(uintptr_t)actorat[x][y] == AMBUSHTILE)
                    actorat[x][y] = NULL;
                if (*map >= AREATILE)
                    tile = *map;
                if (*(map-1-mapwidth) >= AREATILE)
                    tile = *(map-1-mapwidth);
                if (*(map-1+mapwidth) >= AREATILE)
                    tile = *(map-1+mapwidth);
                if ( *(map-2) >= AREATILE)
                    tile = *(map-2);

                *(map-1) = tile;
            }
        }
    }
    wlcache->loadAllSounds();
}

void DrawPlayBorderSides(void)
{
    if(viewsize == 21) return;

    const int sw = g_Image->width();
    const int sh = g_Image->height();
    const int vw = viewwidth;
    const int vh = viewheight;
    const int px = scaleFactor;

    const int h  = sh - px * STATUSLINES;
    const int xl = sw / 2 - vw / 2;
    const int yl = (h - vh) / 2;

    if(xl != 0) {
        wlrender->barScaledCoord(0, 0, xl - px, h, bordercol);
        wlrender->barScaledCoord(xl + vw + px, 0, xl - px * 2, h, bordercol);
    }

    if(yl != 0) {
        wlrender->barScaledCoord(0, 0, sw, yl - px, bordercol);
        wlrender->barScaledCoord(0, yl + vh + px, sw, yl - px, bordercol);
    }

    if(xl != 0) {
        wlrender->barScaledCoord(xl - px, yl - px, vw + px, px, 0);
        wlrender->barScaledCoord(xl, yl + vh, vw + px, px, bordercol - 2);
        wlrender->barScaledCoord(xl - px, yl - px, px, vh + px, 0);
        wlrender->barScaledCoord(xl + vw, yl - px, px, vh + px * 2, bordercol - 2);
        wlrender->barScaledCoord(xl - px, yl + vh, px, px, bordercol - 3);
    } else
        wlrender->barScaledCoord(0, yl+vh, vw, px, bordercol-2);
}

void DrawStatusBorder (quint8 color)
{
    int statusborderw = (g_Image->width() - scaleFactor * 320) / 2;

    wlrender->barScaledCoord(0, 0, g_Image->width(), g_Image->height() - scaleFactor * (STATUSLINES - 3), color);
    wlrender->barScaledCoord(0, g_Image->height() - scaleFactor * (STATUSLINES - 3),
                        statusborderw + scaleFactor * 8, scaleFactor * (STATUSLINES - 4), color);
    wlrender->barScaledCoord(0, g_Image->height() - scaleFactor * 2, g_Image->width(), scaleFactor * 2, color);
    wlrender->barScaledCoord(g_Image->width() - statusborderw - scaleFactor * 8,
                             g_Image->height() - scaleFactor * (STATUSLINES - 3),
                        statusborderw + scaleFactor * 8, scaleFactor * (STATUSLINES - 4), color);
    wlrender->barScaledCoord(statusborderw + scaleFactor * 9, g_Image->height() - scaleFactor * 3,
                        scaleFactor * 97, scaleFactor * 1, color - 1);
    wlrender->barScaledCoord(statusborderw + scaleFactor * 106, g_Image->height() - scaleFactor * 3,
                        scaleFactor * 161, scaleFactor * 1, color - 2);
    wlrender->barScaledCoord(statusborderw + scaleFactor * 267, g_Image->height() - scaleFactor * 3,
                        scaleFactor * 44, scaleFactor * 1, color - 3);
    wlrender->barScaledCoord(g_Image->width() - statusborderw - scaleFactor * 9,
                             g_Image->height() - scaleFactor * (STATUSLINES - 4),
                        scaleFactor * 1, scaleFactor * 20, color - 2);
    wlrender->barScaledCoord(g_Image->width() - statusborderw - scaleFactor * 9,
                             g_Image->height() - scaleFactor * (STATUSLINES / 2 - 4),
                        scaleFactor * 1, scaleFactor * 14, color - 3);
}

void DrawPlayBorder (void)
{
    const int px = scaleFactor;

    if (bordercol != VIEWCOLOR)
        DrawStatusBorder(bordercol);
    else {
        const int statusborderw = (g_Image->width() - px * 320) / 2;
        wlrender->barScaledCoord(0, g_Image->height() - px * STATUSLINES,
                            statusborderw + px * 8, px * STATUSLINES, bordercol);
        wlrender->barScaledCoord(g_Image->width() - statusborderw - px * 8, g_Image->height() - px * STATUSLINES,
                            statusborderw + px * 8, px * STATUSLINES, bordercol);
    }

    if((unsigned) viewheight == (quint32)g_Image->height()) return;

    wlrender->barScaledCoord(0, 0, g_Image->width(), g_Image->height() - px * STATUSLINES, bordercol);

    const int xl = g_Image->width() / 2 - viewwidth / 2;
    const int yl = (g_Image->height() - px * STATUSLINES - viewheight) / 2;
    wlrender->barScaledCoord(xl, yl, viewwidth, viewheight, 0);

    if(xl != 0) {
        wlrender->barScaledCoord(xl - px, yl - px, viewwidth + px, px, 0);
        wlrender->barScaledCoord(xl, yl + viewheight, viewwidth + px, px, bordercol - 2);
        wlrender->barScaledCoord(xl - px, yl - px, px, viewheight + px, 0);
        wlrender->barScaledCoord(xl + viewwidth, yl - px, px, viewheight + 2 * px, bordercol - 2);
        wlrender->barScaledCoord(xl - px, yl + viewheight, px, px, bordercol - 3);
    } else
        wlrender->barScaledCoord(0, yl + viewheight, viewwidth, px, bordercol - 2);
}

void DrawPlayScreen (void)
{
    wlpaint->drawPicScaledCoord((g_Image->width() - scaleFactor * 320) / 2,
                                g_Image->height() - scaleFactor * STATUSLINES, wlcache->find("STATUSBARPIC"));
    DrawPlayBorder ();

    DrawFace ();
    DrawHealth ();
    DrawLives ();
    DrawLevel ();
    DrawAmmo ();
    DrawKeys ();
    DrawWeapon ();
    DrawScore ();
}

void LatchNumberHERE (int x, int y, unsigned width, qint32 number)
{
    unsigned length,c;
    char str[20];

    ltoa (number,str,10);

    length = (unsigned) strlen (str);

    while (length<width) {
        wlpaint->latchDrawPic(x, y, wlcache->find("N_BLANKPIC"));
        x++;
        width--;
    }

    c = length <= width ? 0 : length-width;

    while (c<length) {
        wlpaint->latchDrawPic(x, y, str[c] - '0' + wlcache->find("N_0PIC"));
        x++;
        c++;
    }
}

void ShowActStatus()
{
    // Draw status bar without borders
    quint8 *source = (quint8*)wlcache->graphic(wlcache->find("STATUSBARPIC"));
    int	picnum = wlcache->find("STATUSBARPIC") - STARTPICS;
    int width = pictable[picnum].width;
    int height = pictable[picnum].height;
    int destx = (g_Image->width()-scaleFactor*320)/2 + 9 * scaleFactor;
    int desty = g_Image->height() - (height - 4) * scaleFactor;
    wlrender->memToScreenScaledCoord(source, width, height, 9, 4, destx, desty, width - 18, height - 7);

    ingame = false;
    DrawFace ();
    DrawHealth ();
    DrawLives ();
    DrawLevel ();
    DrawAmmo ();
    DrawKeys ();
    DrawWeapon ();
    DrawScore ();
    ingame = true;
}

char    demoname[13] = "DEMO?.";

#define MAXDEMOSIZE     8192

void StartDemoRecord (int levelnumber)
{
    demobuffer=malloc(MAXDEMOSIZE);
    CHECKMALLOCRESULT(demobuffer);
    demoptr = (int8_t *) demobuffer;
    lastdemoptr = demoptr+MAXDEMOSIZE;

    *demoptr = levelnumber;
    demoptr += 4;                           // leave space for length
    demorecord = true;
}

void FinishDemoRecord (void)
{
    qint32    length,level;

    demorecord = false;

    length = (qint32) (demoptr - (int8_t *)demobuffer);

    demoptr = ((int8_t *)demobuffer)+1;
    demoptr[0] = (int8_t) length;
    demoptr[1] = (int8_t) (length >> 8);
    demoptr[2] = 0;

    wlrender->fadeIn(0, 255, gamepal, 30);
    wlplay->centerWindow(24,3);
    PrintY+=6;
    fontnumber=0;
    SETFONTCOLOR(0,15);
    wluser->US_Print(" Demo number (0-9): ");
    wlpaint->updateScreen();

    if (wluser->US_LineInput(px, py, str, NULL, true, 1, 0)) {
        level = atoi (str);
        if (level>=0 && level<=9) {
            demoname[4] = (char)('0'+level);
            wlcache->write(demoname,demobuffer,length);
        }
    }
    free(demobuffer);
}

void RecordDemo (void)
{
    int level,esc,maps;

    wlplay->centerWindow(26,3);
    PrintY+=6;
    wlcache->cacheGraphic(STARTFONT);
    fontnumber=0;
    SETFONTCOLOR(0,15);
    wluser->US_Print("  Demo which level(1-10): "); maps = 10;
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 30);
    esc = !wluser->US_LineInput(px, py, str, NULL, true, 2, 0);
    if (esc)
        return;

    level = atoi (str);
    level--;

    if (level >= maps || level < 0)
        return;
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    engine->newGame(gd_hard,level/10);
    gamestate.mapon = level%10;
    StartDemoRecord (level);
    DrawPlayScreen ();
    wlrender->fadeIn(0, 255, gamepal, 30);
    startgame = false;
    demorecord = true;
    SetupGameLevel ();
    wlplay->startMusic();
    fizzlein = true;
    wlplay->playLoop();
    demoplayback = false;
    wlplay->stopMusic();
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    //ClearMemory ();
    wlaudio->stopDigitized();
    FinishDemoRecord ();
}

void PlayDemo (int demonumber)
{
    quint16 length;
    qint32 dems[4]={wlcache->find("T_DEMO0"), wlcache->find("T_DEMO1"), wlcache->find("T_DEMO2"), wlcache->find("T_DEMO3")};
    wlcache->cacheGraphic(dems[demonumber]);
    demoptr = (int8_t*)wlcache->graphic(dems[demonumber]);
    engine->newGame(1,0);
    gamestate.mapon = *demoptr++;
    gamestate.difficulty = gd_hard;
    length = READWORD(*(uint8_t **)&demoptr);
    demoptr++;
    lastdemoptr = demoptr-4+length;
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    SETFONTCOLOR(0,15);
    DrawPlayScreen ();
    startgame = false;
    demoplayback = true;
    SetupGameLevel ();
    wlplay->startMusic();
    wlplay->playLoop();
    wlcache->uncacheGraphic(dems[demonumber]);
    demoplayback = false;
    wlplay->stopMusic();
    wlaudio->stopDigitized();
}

#define DEATHROTATE 2

void Died(void)
{
    float   fangle;
    qint32 dx,dy;
    int     iangle,curangle,clockwise,counter,change;

    if (screenfaded) {
        ThreeDRefresh ();
        wlrender->fadeIn(0, 255, gamepal, 30);
    }

    gamestate.weapon = (weapontype) -1;                     // take away weapon
    wlaudio->playSound("PLAYERDEATHSND");

    if(killerobj) {
        dx = killerobj->x - player->x;
        dy = player->y - killerobj->y;

        fangle = (float) atan2((float) dy, (float) dx);     // returns -pi to pi
        if (fangle<0)
            fangle = (float) (M_PI*2+fangle);

        iangle = (int) (fangle/(M_PI*2)*ANGLES);
    } else {
        iangle = player->angle + ANGLES / 2;
        if(iangle >= ANGLES) iangle -= ANGLES;
    }

    if (player->angle > iangle) {
        counter = player->angle - iangle;
        clockwise = ANGLES-player->angle + iangle;
    } else {
        clockwise = iangle - player->angle;
        counter = player->angle + ANGLES-iangle;
    }
    curangle = player->angle;

    if (clockwise<counter) {
        if (curangle>iangle)
            curangle -= ANGLES;
        do {
            change = tics*DEATHROTATE;
            if (curangle + change > iangle)
                change = iangle-curangle;

            curangle += change;
            player->angle += change;
            if (player->angle >= ANGLES)
                player->angle -= ANGLES;

            ThreeDRefresh ();
            CalcTics ();
        } while (curangle != iangle);
    } else {
        if (curangle<iangle)
            curangle += ANGLES;
        do {
            change = -(int)tics*DEATHROTATE;
            if (curangle + change < iangle)
                change = iangle-curangle;

            curangle += change;
            player->angle += change;
            if (player->angle < 0)
                player->angle += ANGLES;

            ThreeDRefresh ();
            CalcTics ();
        } while (curangle != iangle);
    }
    wlplay->finishPaletteShifts();
    wlrender->barScaledCoord(viewscreenx, viewscreeny, viewwidth, viewheight, 4);
    wlinput->clearKeysDown();
    wlinput->userInput(100);
    wlaudio->waitSoundDone();
    wlaudio->stopDigitized();
    gamestate.lives--;

    if (gamestate.lives > -1) {
        gamestate.health = 100;
        gamestate.weapon = gamestate.bestweapon
                = gamestate.chosenweapon = wp_pistol;
        gamestate.ammo = STARTAMMO;
        gamestate.keys = 0;
        wlitem->setpWallState(0);
        wlitem->setpWallPos(0);
        gamestate.attackframe = gamestate.attackcount =
                gamestate.weaponframe = 0;

        if(viewsize != 21) {
            DrawKeys ();
            DrawWeapon ();
            DrawAmmo ();
            DrawHealth ();
            DrawFace ();
            DrawLives ();
        }
    }
}

void GameLoop (void)
{
    qint8 died;

    restartgame:
    //ClearMemory ();
    wlaudio->stopDigitized();
    SETFONTCOLOR(0,15);
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    DrawPlayScreen ();
    died = false;
    do {
        if (!loadedgame)
            gamestate.score = gamestate.oldscore;
        if(!died || viewsize != 21) DrawScore();

        startgame = false;
        if (!loadedgame)
            SetupGameLevel ();
        ingame = true;
        if(loadedgame) {
            wlplay->continueMusic(lastgamemusicoffset);
            loadedgame = false;
        } else wlplay->startMusic();

        if (!died)
            PreloadGraphics ();             // TODO: Let this do something useful!
        else
            died = false;

        fizzlein = true;
        DrawLevel ();
        wlplay->playLoop();
        wlplay->stopMusic();
        ingame = false;

        if (demorecord && playstate != ex_warped)
            FinishDemoRecord ();

        if (startgame || loadedgame)
            goto restartgame;

        switch (playstate) {
        case ex_completed:
        case ex_secretlevel:
            if(viewsize == 21) DrawPlayScreen();
            gamestate.keys = 0;
            DrawKeys ();
            wlrender->fadeOut(0, 255, 0, 0, 0, 30);

            //ClearMemory ();
            wlaudio->stopDigitized();

            LevelCompleted ();              // do the intermission
            if(viewsize == 21) DrawPlayScreen();
            gamestate.oldscore = gamestate.score;

            if (gamestate.mapon == 9)
                gamestate.mapon = ElevatorBackTo[gamestate.episode];    // back from secret
            else
                if (playstate == ex_secretlevel) gamestate.mapon = 9;
                else
                    gamestate.mapon++;
            break;

        case ex_died:
            Died ();
            died = true;

            if (gamestate.lives > -1)
                break;

            wlrender->fadeOut(0, 255, 0, 0, 0, 30);
            if(g_Image->height() % 200 != 0)
                wlrender->clearScreen(0);

            wlaudio->stopDigitized();
            CheckHighScore (gamestate.score,gamestate.mapon+1);
            strcpy(MainMenu[viewscores].string,STR_VS);
            MainMenu[viewscores].routine = CP_ViewScores;
            return;

        case ex_victorious:
            if(viewsize == 21) DrawPlayScreen();
            wlrender->fadeOut(0, 255, 0, 0, 0, 30);
            wlaudio->stopDigitized();
            Victory ();
            wlaudio->stopDigitized();
            CheckHighScore (gamestate.score,gamestate.mapon+1);
            strcpy(MainMenu[viewscores].string,STR_VS);
            MainMenu[viewscores].routine = CP_ViewScores;
            return;

        default:
            if(viewsize == 21) DrawPlayScreen();
            //ClearMemory ();
            wlaudio->stopDigitized();
            break;
        }
    } while (1);
}
