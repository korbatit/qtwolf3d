#include "wl_def.h"

#include "wlstate.h"

#include "wlcache.h"
#include "wlaudio.h"
#include "wlitem.h"
#include "wlplay.h"
#include "wluser.h"

extern WLCache* wlcache;
extern WLAudio* wlaudio;
extern WLItem*  wlitem;
extern WLPlay*  wlplay;
extern WLUser*  wluser;

WLState::WLState()
{
}

WLState::~WLState()
{
}

void WLState::spawnNewObj(quint32 tileX, quint32 tileY, statetype *state)
{
    wlplay->getNewActor();
    newobj->state = state;
    if (state->tictime)
        newobj->ticcount = DEMOCHOOSE_ORIG_SDL(
                    wluser->US_RndT() % state->tictime,
                    wluser->US_RndT() % state->tictime + 1);
    else
        newobj->ticcount = 0;

    newobj->tilex = (qint16)tileX;
    newobj->tiley = (qint16)tileY;
    newobj->x = ((qint32)tileX << TILESHIFT) + TILEGLOBAL / 2;
    newobj->y = ((qint32)tileY << TILESHIFT) + TILEGLOBAL / 2;
    newobj->dir = noDir;

    actorat[tileX][tileY] = newobj;
    newobj->areanumber = *(wlcache->map(0) + (newobj->tiley << mapshift) + newobj->tilex) - AREATILE;
}

void WLState::newState(objtype *ob, statetype *state)
{
    ob->state = state;
    ob->ticcount = state->tictime;
}

bool WLState::checkDiag(qint32 x, qint32 y)
{
    uintptr_t temp = (uintptr_t)actorat[x][y];

    if (temp) {
        if (temp < 256)
           return false;

        if (((objtype *)temp)->flags&FL_SHOOTABLE)
            return false;
    }
    return true;
}

bool WLState::checkSide(objtype* ob, qint32 x, qint32 y)
{
    qint32 doorNum = -1;
    uintptr_t temp = (uintptr_t)actorat[x][y];

    if (temp) {
        if (temp < 128)
            return false;

        if (temp < 256) {
            doorNum = (quint32)temp&63;
            wlitem->openDoor(doorNum);
            ob->distance = -wlitem->doorNum() - 1;
            return true;
        } else if (((objtype *)temp)->flags&FL_SHOOTABLE)
            return false;
    }
    return true;
}

bool WLState::tryWalk(objtype *ob)
{
    if (ob->obclass == inertobj) {
        switch (ob->dir) {
        case north:
            ob->tiley--;
            break;

        case northEast:
            ob->tilex++;
            ob->tiley--;
            break;

        case east:
            ob->tilex++;
            break;

        case southEast:
            ob->tilex++;
            ob->tiley++;
            break;

        case south:
            ob->tiley++;
            break;

        case southWest:
            ob->tilex--;
            ob->tiley++;
            break;

        case west:
            ob->tilex--;
            break;

        case northWest:
            ob->tilex--;
            ob->tiley--;
            break;
        default:
            break;
        }
    } else {
        switch (ob->dir) {
        case north:
            if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj) {
                if (!checkDiag(ob->tilex,ob->tiley-1)) return false;
            } else
                if (!checkSide(ob, ob->tilex,ob->tiley-1)) return false;
            ob->tiley--;
            break;

        case northEast:
            if (!checkDiag(ob->tilex+1,ob->tiley-1)) return false;
            if (!checkDiag(ob->tilex+1,ob->tiley)) return false;
            if (!checkDiag(ob->tilex,ob->tiley-1)) return false;
            ob->tilex++;
            ob->tiley--;
            break;

        case east:
            if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj) {
                if (!checkDiag(ob->tilex+1,ob->tiley)) return false;
            } else {
                if (!checkSide(ob, ob->tilex+1,ob->tiley)) return false;
            }
            ob->tilex++;
            break;

        case southEast:
            if (!checkDiag(ob->tilex+1,ob->tiley+1)) return false;
            if (!checkDiag(ob->tilex+1,ob->tiley)) return false;
            if (!checkDiag(ob->tilex,ob->tiley+1)) return false;
            ob->tilex++;
            ob->tiley++;
            break;

        case south:
            if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj) {
                if(!checkDiag(ob->tilex,ob->tiley+1)) return false;
            } else {
                if (!checkSide(ob, ob->tilex,ob->tiley+1)) return false;
            }
            ob->tiley++;
            break;

        case southWest:
            if (!checkDiag(ob->tilex-1,ob->tiley+1)) return false;
            if (!checkDiag(ob->tilex-1,ob->tiley)) return false;
            if (!checkDiag(ob->tilex,ob->tiley+1)) return false;
            ob->tilex--;
            ob->tiley++;
            break;

        case west:
            if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj) {
                if (!checkDiag(ob->tilex-1,ob->tiley)) return false;
            } else {
                if (!checkSide(ob, ob->tilex-1,ob->tiley)) return false;
            }
            ob->tilex--;
            break;

        case northWest:
            if (!checkDiag(ob->tilex-1,ob->tiley-1)) return false;
            if (!checkDiag(ob->tilex-1,ob->tiley)) return false;
            if (!checkDiag(ob->tilex,ob->tiley-1)) return false;
            ob->tilex--;
            ob->tiley--;
            break;

        case noDir:
            return false;

        default:
            Quit ("Walk: Bad dir");
        }
    }
    ob->areanumber = *(wlcache->map(0) + (ob->tiley<<mapshift)+ob->tilex) - AREATILE;

    ob->distance = TILEGLOBAL;
    return true;
}

void WLState::selectDodgeDir(objtype *ob)
{
    qint32      deltaX;
    qint32      deltaY;
    quint32     absdx, absdy;
    dirType_t   dirTry[5];
    dirType_t   turnAround, tdir;

    if (ob->flags & FL_FIRSTATTACK) {
        turnAround = noDir;
        ob->flags &= ~FL_FIRSTATTACK;
    } else turnAround = opposite[ob->dir];

    deltaX = player->tilex - ob->tilex;
    deltaY = player->tiley - ob->tiley;

    if (deltaX > 0) {
        dirTry[1]= east;
        dirTry[3]= west;
    } else {
        dirTry[1]= west;
        dirTry[3]= east;
    }

    if (deltaY > 0) {
        dirTry[2]= south;
        dirTry[4]= north;
    } else {
        dirTry[2]= north;
        dirTry[4]= south;
    }
    absdx = abs(deltaX);
    absdy = abs(deltaY);

    if (absdx > absdy) {
        tdir = dirTry[1];
        dirTry[1] = dirTry[2];
        dirTry[2] = tdir;
        tdir = dirTry[3];
        dirTry[3] = dirTry[4];
        dirTry[4] = tdir;
    }

    if (wluser->US_RndT() < 128) {
        tdir = dirTry[1];
        dirTry[1] = dirTry[2];
        dirTry[2] = tdir;
        tdir = dirTry[3];
        dirTry[3] = dirTry[4];
        dirTry[4] = tdir;
    }

    dirTry[0] = diagonal [ dirTry[1] ] [ dirTry[2] ];

    for (int i = 0; i < 5; i++) {
        if ( dirTry[i] == noDir || dirTry[i] == turnAround)
            continue;
        ob->dir = dirTry[i];
        if (tryWalk(ob))
            return;
    }

    if (turnAround != noDir) {
        ob->dir = turnAround;
        if (tryWalk(ob))
            return;
    }
    ob->dir = noDir;
}

void WLState::selectChaseDir(objtype *ob)
{
    qint32    deltaX;
    qint32    deltaY;
    dirType_t d[3];
    dirType_t tdir, oldDir, turnAround;


    oldDir = ob->dir;
    turnAround = opposite[oldDir];

    deltaX = player->tilex - ob->tilex;
    deltaY = player->tiley - ob->tiley;

    d[1] = noDir;
    d[2] = noDir;

    if (deltaX > 0)
        d[1] = east;
    else if (deltaX < 0)
        d[1] = west;
    if (deltaY > 0)
        d[2] = south;
    else if (deltaY < 0)
        d[2] = north;

    if (abs(deltaY) > abs(deltaX)) {
        tdir = d[1];
        d[1] = d[2];
        d[2] = tdir;
    }
    if (d[1] == turnAround)
        d[1] = noDir;
    if (d[2] == turnAround)
        d[2] = noDir;

    if (d[1] != noDir) {
        ob->dir = d[1];
        if (tryWalk(ob))
            return;
    }
    if (d[2] != noDir) {
        ob->dir = d[2];
        if (tryWalk(ob))
            return;
    }
    if (oldDir != noDir) {
        ob->dir = oldDir;
        if (tryWalk(ob))
            return;
    }
    if (wluser->US_RndT() > 128) {
        for (tdir = north; tdir <= west; tdir = (dirType_t)(tdir+1)) {
            if (tdir != turnAround) {
                ob->dir = tdir;
                if (tryWalk(ob))
                    return;
            }
        }
    } else {
        for (tdir = west; tdir >= north; tdir = (dirType_t)(tdir-1)) {
            if (tdir != turnAround) {
                ob->dir = tdir;
                if (tryWalk(ob))
                    return;
            }
        }
    }
    if (turnAround !=  noDir) {
        ob->dir = turnAround;
        if (ob->dir != noDir) {
            if (tryWalk(ob))
                return;
        }
    }
    ob->dir = noDir;
}

void WLState::selectRunDir(objtype *ob)
{
    qint32 deltaX;
    qint32 deltaY;
    dirType_t d[3];
    dirType_t tdir;


    deltaX = player->tilex - ob->tilex;
    deltaY = player->tiley - ob->tiley;

    if (deltaX < 0)
        d[1] = east;
    else
        d[1] = west;
    if (deltaY < 0)
        d[2] = south;
    else
        d[2] = north;

    if (abs(deltaY) > abs(deltaX)) {
        tdir = d[1];
        d[1] = d[2];
        d[2] = tdir;
    }

    ob->dir = d[1];
    if (tryWalk(ob))
        return;

    ob->dir = d[2];
    if (tryWalk(ob))
        return;

    if (wluser->US_RndT() > 128) {
        for (tdir = north; tdir <= west; tdir = (dirType_t)(tdir+1)) {
            ob->dir = tdir;
            if (tryWalk(ob))
                return;
        }
    } else {
        for (tdir = west; tdir >= north; tdir = (dirType_t)(tdir-1)) {
            ob->dir = tdir;
            if (tryWalk(ob))
                return;
        }
    }
    ob->dir = noDir;
}

void WLState::moveObj(objtype *ob, qint32 move)
{
    qint32 deltaX;
    qint32 deltaY;

    switch (ob->dir) {
    case north:
        ob->y -= move;
        break;
    case northEast:
        ob->x += move;
        ob->y -= move;
        break;
    case east:
        ob->x += move;
        break;
    case southEast:
        ob->x += move;
        ob->y += move;
        break;
    case south:
        ob->y += move;
        break;
    case southWest:
        ob->x -= move;
        ob->y += move;
        break;
    case west:
        ob->x -= move;
        break;
    case northWest:
        ob->x -= move;
        ob->y -= move;
        break;

    case noDir:
        return;

    default:
        Quit("moveObj: bad dir!");
    }

    if (*wlitem->areaByPlayer(ob->areanumber)) {
        deltaX = ob->x - player->x;
        if (deltaX < -MINACTORDIST || deltaX > MINACTORDIST)
            goto moveok;
        deltaY = ob->y - player->y;
        if (deltaY < -MINACTORDIST || deltaY > MINACTORDIST)
            goto moveok;

        if (ob->hidden)
            goto moveok;

        if (ob->obclass == ghostobj || ob->obclass == spectreobj)
            TakeDamage(tics*2,ob);

        switch (ob->dir) {
        case north:
            ob->y += move;
            break;
        case northEast:
            ob->x -= move;
            ob->y += move;
            break;
        case east:
            ob->x -= move;
            break;
        case southEast:
            ob->x -= move;
            ob->y -= move;
            break;
        case south:
            ob->y -= move;
            break;
        case southWest:
            ob->x += move;
            ob->y -= move;
            break;
        case west:
            ob->x += move;
            break;
        case northWest:
            ob->x += move;
            ob->y += move;
            break;

        case noDir:
            return;
        }
        return;
    }
    moveok:
        ob->distance -=move;
}

void WLState::dropItem(wl_stat_t itemtype, qint32 tileX, qint32 tileY)
{
    qint32 x,y,xl,xh,yl,yh;

    if (!actorat[tileX][tileY]) {
        wlitem->placeItemType(itemtype, tileX, tileY);
        return;
    }
    xl = tileX - 1;
    xh = tileX + 1;
    yl = tileY - 1;
    yh = tileY + 1;

    for (x=xl ; x <= xh ; x++) {
        for (y = yl ; y <= yh ; y++) {
            if (!actorat[x][y]) {
                wlitem->placeItemType(itemtype, x, y);
                return;
            }
        }
    }
}

void WLState::killActor(objtype *ob)
{
    qint32 tileX;
    qint32 tileY;

    tileX = ob->tilex = (quint16)(ob->x >> TILESHIFT);
    tileY = ob->tiley = (quint16)(ob->y >> TILESHIFT);

    switch (ob->obclass) {
    case guardobj:
        GivePoints(100);
        newState(ob, &s_grddie1);
        wlitem->placeItemType(bo_clip2, tileX, tileY);
        break;

    case officerobj:
        GivePoints(400);
        newState(ob, &s_ofcdie1);
        wlitem->placeItemType(bo_clip2, tileX, tileY);
        break;

    case mutantobj:
        GivePoints(700);
        newState(ob, &s_mutdie1);
        wlitem->placeItemType(bo_clip2, tileX, tileY);
        break;

    case ssobj:
        GivePoints(500);
        newState(ob, &s_ssdie1);
        if (gamestate.bestweapon < wp_machinegun)
            wlitem->placeItemType(bo_machinegun, tileX, tileY);
        else
            wlitem->placeItemType(bo_clip2, tileX, tileY);
        break;

    case dogobj:
        GivePoints(200);
        newState(ob, &s_dogdie1);
        break;

    case bossobj:
        GivePoints(5000);
        newState(ob, &s_bossdie1);
        wlitem->placeItemType(bo_key1, tileX, tileY);
        break;

    case gretelobj:
        GivePoints(5000);
        newState(ob, &s_greteldie1);
        wlitem->placeItemType(bo_key1, tileX, tileY);
        break;

    case giftobj:
        GivePoints(5000);
        gamestate.killx = player->x;
        gamestate.killy = player->y;
        newState(ob, &s_giftdie1);
        break;

    case fatobj:
        GivePoints(5000);
        gamestate.killx = player->x;
        gamestate.killy = player->y;
        newState(ob, &s_fatdie1);
        break;

    case schabbobj:
        GivePoints(5000);
        gamestate.killx = player->x;
        gamestate.killy = player->y;
        newState(ob, &s_schabbdie1);
        break;
    case fakeobj:
        GivePoints(2000);
        newState(ob, &s_fakedie1);
        break;

    case mechahitlerobj:
        GivePoints(5000);
        newState(ob, &s_mechadie1);
        break;
    case realhitlerobj:
        GivePoints(5000);
        gamestate.killx = player->x;
        gamestate.killy = player->y;
        newState(ob, &s_hitlerdie1);
        break;
    default:
        break;
    }

    gamestate.killcount++;
    ob->flags &= ~FL_SHOOTABLE;
    actorat[ob->tilex][ob->tiley] = NULL;
    ob->flags |= FL_NONMARK;
}

void WLState::damageActor(objtype *ob, quint32 damage)
{
    madenoise = true;

    if ( !(ob->flags & FL_ATTACKMODE) )
        damage <<= 1;

    ob->hitpoints -= (qint16)damage;

    if (ob->hitpoints <= 0)
        killActor(ob);
    else {
        if (! (ob->flags & FL_ATTACKMODE) )
            firstSighting(ob);

        switch (ob->obclass) {
        case guardobj:
            if (ob->hitpoints&1)
                newState(ob, &s_grdpain);
            else
                newState(ob, &s_grdpain1);
            break;

        case officerobj:
            if (ob->hitpoints&1)
                newState(ob, &s_ofcpain);
            else
                newState(ob, &s_ofcpain1);
            break;

        case mutantobj:
            if (ob->hitpoints&1)
                newState(ob, &s_mutpain);
            else
                newState(ob, &s_mutpain1);
            break;

        case ssobj:
            if (ob->hitpoints&1)
                newState(ob, &s_sspain);
            else
                newState(ob, &s_sspain1);

            break;
        default:
            break;
        }
    }
}

qint8 WLState::checkLine(objtype *ob)
{
    qint32    x1,y1,xt1,yt1,x2,y2,xt2,yt2;
    qint32    x,y;
    qint32    xdist,ydist,xstep,ystep;
    qint32    partial,delta;
    qint32    ltemp;
    qint32    xfrac,yfrac,deltafrac;
    quint32   value,intercept;

    x1 = ob->x >> UNSIGNEDSHIFT;
    y1 = ob->y >> UNSIGNEDSHIFT;
    xt1 = x1 >> 8;
    yt1 = y1 >> 8;

    x2 = plux;
    y2 = pluy;
    xt2 = player->tilex;
    yt2 = player->tiley;

    xdist = abs(xt2 - xt1);

    if (xdist > 0) {
        if (xt2 > xt1) {
            partial = 256 - (x1&0xff);
            xstep = 1;
        } else {
            partial = x1&0xff;
            xstep = -1;
        }

        deltafrac = abs(x2-x1);
        delta = y2 - y1;
        ltemp = ((qint32)delta << 8) / deltafrac;
        if (ltemp > 0x7fffl)
            ystep = 0x7fff;
        else if (ltemp < -0x7fffl)
            ystep = -0x7fff;
        else
            ystep = ltemp;
        yfrac = y1 + (((qint32)ystep * partial) >> 8);

        x = xt1 + xstep;
        xt2 += xstep;
        do {
            y = yfrac >> 8;
            yfrac += ystep;

            value = (quint32)tilemap[x][y];
            x += xstep;

            if (!value)
                continue;

            if (value < 128 || value > 256)
                return false;

            value &= ~0x80;
            intercept = yfrac - ystep / 2;

            if (intercept > *wlitem->doorPosition(value))
                return false;

        } while (x != xt2);
    }

    ydist = abs(yt2 - yt1);

    if (ydist > 0) {
        if (yt2 > yt1) {
            partial = 256 - (y1&0xff);
            ystep = 1;
        } else {
            partial = y1&0xff;
            ystep = -1;
        }

        deltafrac = abs(y2-y1);
        delta = x2 - x1;
        ltemp = ((qint32)delta << 8) / deltafrac;
        if (ltemp > 0x7fffl)
            xstep = 0x7fff;
        else if (ltemp < -0x7fffl)
            xstep = -0x7fff;
        else
            xstep = ltemp;
        xfrac = x1 + (((qint32)xstep * partial) >> 8);

        y = yt1 + ystep;
        yt2 += ystep;
        do {
            x = xfrac >> 8;
            xfrac += xstep;

            value = (quint32)tilemap[x][y];
            y += ystep;

            if (!value)
                continue;

            if (value < 128 || value > 256)
                return false;

            value &= ~0x80;
            intercept = xfrac - xstep / 2;

            if (intercept > *wlitem->doorPosition(value))
                return false;
        } while (y != yt2);
    }

    return true;
}

#define MINSIGHT        0x18000l

bool WLState::checkSight(objtype *ob)
{
    qint32 deltaX;
    qint32 deltaY;

    if (!*wlitem->areaByPlayer(ob->areanumber))
        return false;

    deltaX = player->x - ob->x;
    deltaY = player->y - ob->y;

    if (deltaX > -MINSIGHT && deltaX < MINSIGHT
            && deltaY > -MINSIGHT && deltaY < MINSIGHT)
        return true;

    switch (ob->dir) {
    case north:
        if (deltaY > 0)
            return false;
        break;

    case east:
        if (deltaX < 0)
            return false;
        break;

    case south:
        if (deltaY < 0)
            return false;
        break;

    case west:
        if (deltaX > 0)
            return false;
        break;

    case northWest:
        if (DEMOCOND_SDL && deltaY > -deltaX)
            return false;
        break;

    case northEast:
        if (DEMOCOND_SDL && deltaY > deltaX)
            return false;
        break;

    case southWest:
        if (DEMOCOND_SDL && deltaX > deltaY)
            return false;
        break;

    case southEast:
        if (DEMOCOND_SDL && -deltaX > deltaY)
            return false;
        break;
    default:
        break;
    }
    return checkLine(ob);
}

void WLState::firstSighting(objtype *ob)
{
    switch (ob->obclass)
    {
    case guardobj:
        PlaySoundLocActor(wlcache->findSound("HALTSND"), ob);
        newState(ob, &s_grdchase1);
        ob->speed *= 3;
        break;

    case officerobj:
        PlaySoundLocActor(wlcache->findSound("SPIONSND"), ob);
        newState(ob, &s_ofcchase1);
        ob->speed *= 5;
        break;

    case mutantobj:
        newState(ob, &s_mutchase1);
        ob->speed *= 3;
        break;

    case ssobj:
        PlaySoundLocActor(wlcache->findSound("SCHUTZADSND"), ob);
        newState(ob, &s_sschase1);
        ob->speed *= 4;
        break;

    case dogobj:
        PlaySoundLocActor(wlcache->findSound("DOGBARKSND"), ob);
        newState(ob, &s_dogchase1);
        ob->speed *= 2;
        break;

    case bossobj:
        wlaudio->playSound("GUTENTAGSND");
        newState(ob, &s_bosschase1);
        ob->speed = SPDPATROL*3;
        break;

    case schabbobj:
        wlaudio->playSound("SCHABBSHASND");
        newState(ob, &s_schabbchase1);
        ob->speed *= 3;
        break;

    case fakeobj:
        wlaudio->playSound("TOT_HUNDSND");
        newState(ob, &s_fakechase1);
        ob->speed *= 3;
        break;

    case mechahitlerobj:
        wlaudio->playSound("DIESND");
        newState(ob, &s_mechachase1);
        ob->speed *= 3;
        break;

    case realhitlerobj:
        wlaudio->playSound("DIESND");
        newState(ob, &s_hitlerchase1);
        ob->speed *= 5;
        break;

    case ghostobj:
        newState(ob, &s_blinkychase1);
        ob->speed *= 2;
        break;
    default:
        break;
    }

    if (ob->distance < 0)
        ob->distance = 0;

    ob->flags |= FL_ATTACKMODE|FL_FIRSTATTACK;
}

qint8 WLState::sightPlayer(objtype *ob)
{
    if (ob->flags & FL_ATTACKMODE)
        Quit ("An actor in ATTACKMODE called sightPlayer!");

    if (ob->temp2) {
        ob->temp2 -= (qint16) tics;
        if (ob->temp2 > 0)
            return false;
        ob->temp2 = 0; // time to react
    } else {
        if (!*wlitem->areaByPlayer(ob->areanumber))
            return false;

        if (ob->flags & FL_AMBUSH) {
            if (!checkSight(ob))
                return false;
            ob->flags &= ~FL_AMBUSH;
        } else {
            if (!madenoise && !checkSight(ob))
                return false;
        }

        switch (ob->obclass) {
        case guardobj:
            ob->temp2 = 1 + wluser->US_RndT() / 4;
            break;
        case officerobj:
            ob->temp2 = 2;
            break;
        case mutantobj:
            ob->temp2 = 1 + wluser->US_RndT() / 6;
            break;
        case ssobj:
            ob->temp2 = 1 + wluser->US_RndT() / 6;
            break;
        case dogobj:
            ob->temp2 = 1 + wluser->US_RndT() / 8;
            break;

        case bossobj:
        case schabbobj:
        case fakeobj:
        case mechahitlerobj:
        case realhitlerobj:
        case gretelobj:
        case giftobj:
        case fatobj:
        case spectreobj:
        case angelobj:
        case transobj:
        case uberobj:
        case willobj:
        case deathobj:
            ob->temp2 = 1;
            break;
        default:
            break;
        }
        return false;
    }

    firstSighting(ob);

    return true;
}
