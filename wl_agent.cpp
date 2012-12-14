#include <stdint.h>

#include "wl_def.h"

#include "wlcache.h"
#include "wlaudio.h"
#include "wlstate.h"
#include "wlitem.h"
#include "wlplay.h"
#include "wlpaint.h"
#include "wluser.h"
#include "main.h"

extern WLCache* wlcache;
extern WLAudio* wlaudio;
extern WLState* wlstate;
extern WLItem*  wlitem;
extern WLPlay*  wlplay;
extern WLPaint* wlpaint;
extern WLUser*  wluser;

extern QImage* g_Image;

#define MAXMOUSETURN    10

#define MOVESCALE       150l
#define BACKMOVESCALE   100l
#define ANGLESCALE      20

extern bool     ingame;

qint32         thrustspeed;
quint16            plux,pluy;          // player coordinates scaled to unsigned
short           anglefrac;
objtype        *LastAttacker;

void    T_Player (objtype *ob);
void    T_Attack (objtype *ob);

statetype   s_player = {false,0,0,(statefunc) T_Player,NULL,NULL};
statetype   s_attack = {false,0,0,(statefunc) T_Attack,NULL,NULL};

struct atkinf {
    int8_t    tics,attack,frame;              // attack is 1 for gun, 2 for knife
} attackinfo[4][14] =
{
{ {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,3,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,4,3},{6,-1,4} },
                          };

void Attack (void);
void Use (void);
void Search (objtype *ob);
void SelectWeapon (void);
void SelectItem (void);

qint8 TryMove (objtype *ob);
void T_Player (objtype *ob);

void ClipMove (objtype *ob, qint32 xmove, qint32 ymove);

void CheckWeaponChange (void)
{
    int newWeapon = -1;

    if (!gamestate.ammo)            // must use knife with no ammo
        return;

    if(buttonstate[bt_nextweapon] && !buttonheld[bt_nextweapon]) {
        newWeapon = gamestate.weapon + 1;
        if(newWeapon > gamestate.bestweapon) newWeapon = 0;
    } else if(buttonstate[bt_prevweapon] && !buttonheld[bt_prevweapon]) {
        newWeapon = gamestate.weapon - 1;
        if(newWeapon < 0) newWeapon = gamestate.bestweapon;
    } else {
        for(int i = wp_knife; i <= gamestate.bestweapon; i++) {
            if (buttonstate[bt_readyknife + i - wp_knife]) {
                newWeapon = i;
                break;
            }
        }
    }

    if(newWeapon != -1) {
        gamestate.weapon = gamestate.chosenweapon = (weapontype) newWeapon;
        DrawWeapon();
    }
}

void ControlMovement (objtype *ob)
{
    qint32 oldx,oldy;
    int     angle;
    int     angleunits;

    thrustspeed = 0;

    oldx = player->x;
    oldy = player->y;

    if(buttonstate[bt_strafeleft]) {
        angle = ob->angle + ANGLES/4;
        if(angle >= ANGLES)
            angle -= ANGLES;
        if(buttonstate[bt_run])
            Thrust(angle, RUNMOVE * MOVESCALE * tics);
        else
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
    }

    if(buttonstate[bt_straferight]) {
        angle = ob->angle - ANGLES/4;
        if(angle < 0)
            angle += ANGLES;
        if(buttonstate[bt_run])
            Thrust(angle, RUNMOVE * MOVESCALE * tics );
        else
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
    }
    if (buttonstate[bt_strafe]) {
        if (controlx > 0) {
            angle = ob->angle - ANGLES/4;
            if (angle < 0)
                angle += ANGLES;
            Thrust (angle,controlx*MOVESCALE);      // move to left
        } else if (controlx < 0) {
            angle = ob->angle + ANGLES/4;
            if (angle >= ANGLES)
                angle -= ANGLES;
            Thrust (angle,-controlx*MOVESCALE);     // move to right
        }
    } else {
        anglefrac += controlx;
        angleunits = anglefrac/ANGLESCALE;
        anglefrac -= angleunits*ANGLESCALE;
        ob->angle -= angleunits;

        if (ob->angle >= ANGLES)
            ob->angle -= ANGLES;
        if (ob->angle < 0)
            ob->angle += ANGLES;

    }
    if (controly < 0) {
        Thrust (ob->angle,-controly*MOVESCALE); // move forwards
    } else if (controly > 0) {
        angle = ob->angle + ANGLES/2;
        if (angle >= ANGLES)
            angle -= ANGLES;
        Thrust (angle,controly*BACKMOVESCALE);          // move backwards
    }

    if (gamestate.victoryflag)              // watching the BJ actor
        return;
}

void StatusDrawPic (unsigned x, unsigned y, unsigned picnum)
{
    wlpaint->latchDrawPicScaledCoord((g_Image->width() - scaleFactor * 320) / 16 + scaleFactor * x,
                             g_Image->height() - scaleFactor * (STATUSLINES - y), picnum);
}

void StatusDrawFace(unsigned picnum)
{
    StatusDrawPic(17, 4, picnum);
    StatusDrawLCD(picnum);
}

void DrawFace (void)
{
    if(viewsize == 21 && ingame) return;
    if (wlaudio->soundPlaying() == wlcache->findSound("GETGATLINGSND"))
        StatusDrawFace(wlcache->find("GOTGATLINGPIC"));
    else if (gamestate.health) {
        StatusDrawFace(wlcache->find("FACE1APIC") + 3 * ((100 - gamestate.health) / 16) + gamestate.faceframe);
    } else {
        if (LastAttacker && LastAttacker->obclass == needleobj)
            StatusDrawFace(wlcache->find("MUTANTBJPIC"));
        else
            StatusDrawFace(wlcache->find("FACE8APIC"));
    }
}

int facecount = 0;
int facetimes = 0;

void UpdateFace (void)
{
    // don't make demo depend on sound playback
    if(demoplayback || demorecord) {
        if(facetimes > 0) {
            facetimes--;
            return;
        }
    } else if(wlaudio->soundPlaying() == wlcache->findSound("GETGATLINGSND"))
        return;

    facecount += tics;
    if (facecount > wluser->US_RndT()) {
        gamestate.faceframe = (wluser->US_RndT() >> 6);
        if (gamestate.faceframe == 3)
            gamestate.faceframe = 1;

        facecount = 0;
        DrawFace ();
    }
}

static void LatchNumber (int x, int y, unsigned width, qint32 number)
{
    unsigned length,c;
    char    str[20];

    ltoa (number,str,10);

    length = (unsigned) strlen (str);

    while (length<width) {
        StatusDrawPic (x,y, wlcache->find("N_BLANKPIC"));
        x++;
        width--;
    }

    c = length <= width ? 0 : length-width;

    while (c<length) {
        StatusDrawPic (x,y,str[c]-'0'+ wlcache->find("N_0PIC"));
        x++;
        c++;
    }
}

void DrawHealth (void)
{
    if(viewsize == 21 && ingame) return;
    LatchNumber (21,16,3,gamestate.health);
}

void TakeDamage (int points,objtype *attacker)
{
    LastAttacker = attacker;

    if (gamestate.victoryflag)
        return;
    if (gamestate.difficulty==gd_baby)
        points>>=2;
    if (!godmode)
        gamestate.health -= points;
    if (gamestate.health<=0) {
        gamestate.health = 0;
        playstate = ex_died;
        killerobj = attacker;
    }
    if (godmode != 2)
        wlplay->startDamageFlash(points);
    DrawHealth ();
    DrawFace ();
}

void HealSelf (int points)
{
    gamestate.health += points;
    if (gamestate.health>100)
        gamestate.health = 100;

    DrawHealth ();
    DrawFace ();
}

void DrawLevel (void)
{
    if(viewsize == 21 && ingame) return;
    LatchNumber (2,16,2,gamestate.mapon+1);
}

void DrawLives (void)
{
    if(viewsize == 21 && ingame) return;
    LatchNumber (14,16,1,gamestate.lives);
}

void GiveExtraMan (void)
{
    if (gamestate.lives<9)
        gamestate.lives++;
    DrawLives ();
    wlaudio->playSound("BONUS1UPSND");
}

void DrawScore (void)
{
    if(viewsize == 21 && ingame) return;
    LatchNumber (6,16,6,gamestate.score);
}

void GivePoints (qint32 points)
{
    gamestate.score += points;
    while (gamestate.score >= gamestate.nextextra) {
        gamestate.nextextra += EXTRAPOINTS;
        GiveExtraMan ();
    }
    DrawScore ();
}

void DrawWeapon (void)
{
    if(viewsize == 21 && ingame) return;
    StatusDrawPic (32, 8, wlcache->find("KNIFEPIC") + gamestate.weapon);
}

void DrawKeys (void)
{
    if(viewsize == 21 && ingame) return;
    if (gamestate.keys & 1)
        StatusDrawPic (30, 4, wlcache->find("GOLDKEYPIC"));
    else
        StatusDrawPic (30, 4, wlcache->find("NOKEYPIC"));

    if (gamestate.keys & 2)
        StatusDrawPic (30, 20, wlcache->find("SILVERKEYPIC"));
    else
        StatusDrawPic (30, 20, wlcache->find("NOKEYPIC"));
}

void GiveWeapon (int weapon)
{
    GiveAmmo (6);

    if (gamestate.bestweapon<weapon)
        gamestate.bestweapon = gamestate.weapon
                = gamestate.chosenweapon = (weapontype) weapon;

    DrawWeapon ();
}

void DrawAmmo (void)
{
    if(viewsize == 21 && ingame) return;
    LatchNumber (27,16,2,gamestate.ammo);
}

void GiveAmmo (int ammo)
{
    if (!gamestate.ammo) {
        if (!gamestate.attackframe) {
            gamestate.weapon = gamestate.chosenweapon;
            DrawWeapon ();
        }
    }
    gamestate.ammo += ammo;
    if (gamestate.ammo > 99)
        gamestate.ammo = 99;
    DrawAmmo ();
}

void GiveKey (int key)
{
    gamestate.keys |= (1<<key);
    DrawKeys ();
}

void GetBonus(statObj_t *check)
{
    switch (check->itemNumber) {
    case    bo_firstaid:
        if (gamestate.health == 100)
            return;
        wlaudio->playSound("HEALTH2SND");
        HealSelf (25);
        break;
    case    bo_key1:
    case    bo_key2:
    case    bo_key3:
    case    bo_key4:
        GiveKey (check->itemNumber - bo_key1);
        wlaudio->playSound("GETKEYSND");
        break;
    case    bo_cross:
        wlaudio->playSound("BONUS1SND");
        GivePoints (100);
        gamestate.treasurecount++;
        break;
    case    bo_chalice:
        wlaudio->playSound("BONUS2SND");
        GivePoints (500);
        gamestate.treasurecount++;
        break;
    case    bo_bible:
        wlaudio->playSound("BONUS3SND");
        GivePoints (1000);
        gamestate.treasurecount++;
        break;
    case    bo_crown:
        wlaudio->playSound("BONUS4SND");
        GivePoints (5000);
        gamestate.treasurecount++;
        break;
    case    bo_clip:
        if (gamestate.ammo == 99)
            return;
        wlaudio->playSound("GETAMMOSND");
        GiveAmmo (8);
        break;
    case    bo_clip2:
        if (gamestate.ammo == 99)
            return;
        wlaudio->playSound("GETAMMOSND");
        GiveAmmo (4);
        break;
    case    bo_machinegun:
        wlaudio->playSound("GETMACHINESND");
        GiveWeapon (wp_machinegun);
        break;
    case    bo_chaingun:
        wlaudio->playSound("GETGATLINGSND");
        facetimes = 38;
        GiveWeapon (wp_chaingun);
        if(viewsize != 21)
            StatusDrawFace (wlcache->find("GOTGATLINGPIC"));
        facecount = 0;
        break;
    case    bo_fullheal:
        wlaudio->playSound("BONUS1UPSND");
        HealSelf (99);
        GiveAmmo (25);
        GiveExtraMan ();
        gamestate.treasurecount++;
        break;
    case    bo_food:
        if (gamestate.health == 100)
            return;
        wlaudio->playSound("HEALTH1SND");
        HealSelf (10);
        break;
    case    bo_alpo:
        if (gamestate.health == 100)
            return;
        wlaudio->playSound("HEALTH1SND");
        HealSelf (4);
        break;
    case    bo_gibs:
        if (gamestate.health >10)
            return;
        wlaudio->playSound("SLURPIESND");
        HealSelf (1);
        break;
    }
    wlplay->startBonusFlash();
    check->shapeNum = -1;  // remove from list
}

qint8 TryMove (objtype *ob)
{
    int         xl,yl,xh,yh,x,y;
    objtype    *check;
    qint32     deltax,deltay;

    xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
    yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

    xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
    yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

#define PUSHWALLMINDIST PLAYERSIZE

    //
    // check for solid walls
    //
    for (y=yl;y<=yh;y++) {
        for (x=xl;x<=xh;x++) {
            check = actorat[x][y];
            if (check && !ISPOINTER(check)) {
                if((tilemap[x][y] == 64) && (x == *wlitem->pWallX()) && (y == *wlitem->pWallY())){
                    switch(*wlitem->pWallDir()) {
                    case di_north:
                        if(ob->y - PUSHWALLMINDIST <= (*wlitem->pWallY() << TILESHIFT) + ((63 - *wlitem->pWallPos()) << 10))
                            return false;
                        break;
                    case di_west:
                        if(ob->x - PUSHWALLMINDIST <= (*wlitem->pWallX() << TILESHIFT) + ((63 - *wlitem->pWallPos()) << 10))
                            return false;
                        break;
                    case di_east:
                        if(ob->x + PUSHWALLMINDIST >= (*wlitem->pWallX() << TILESHIFT) + (*wlitem->pWallPos() << 10))
                            return false;
                        break;
                    case di_south:
                        if(ob->y + PUSHWALLMINDIST >= (*wlitem->pWallY() << TILESHIFT) + (*wlitem->pWallPos() << 10))
                            return false;
                        break;
                    }
                } else return false;
            }
        }
    }

    if (yl > 0)
        yl--;
    if (yh < MAPSIZE-1)
        yh++;
    if (xl > 0)
        xl--;
    if (xh < MAPSIZE-1)
        xh++;

    for (y = yl; y <= yh; y++) {
        for (x = xl; x <= xh; x++) {
            check = actorat[x][y];
            if (ISPOINTER(check) && check != player && (check->flags & FL_SHOOTABLE) ) {
                deltax = ob->x - check->x;
                if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
                    continue;
                deltay = ob->y - check->y;
                if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
                    continue;

                return false;
            }
        }
    }
    return true;
}

void ClipMove (objtype *ob, qint32 xmove, qint32 ymove)
{
    qint32    basex,basey;

    basex = ob->x;
    basey = ob->y;

    ob->x = basex+xmove;
    ob->y = basey+ymove;
    if (TryMove (ob))
        return;

    if (noclip && ob->x > 2*TILEGLOBAL && ob->y > 2*TILEGLOBAL
            && ob->x < (((qint32)(mapwidth-1))<<TILESHIFT)
            && ob->y < (((qint32)(mapheight-1))<<TILESHIFT) )
        return;         // walk through walls

    if (!wlaudio->soundPlaying())
        wlaudio->playSound("HITWALLSND");

    ob->x = basex+xmove;
    ob->y = basey;
    if (TryMove (ob))
        return;

    ob->x = basex;
    ob->y = basey+ymove;
    if (TryMove (ob))
        return;

    ob->x = basex;
    ob->y = basey;
}

void VictoryTile (void)
{
    SpawnBJVictory ();
    gamestate.victoryflag = true;
}

void Thrust(int angle, qint32 speed)
{
    qint32 xmove,ymove;
    unsigned offset;

    thrustspeed += speed;
    if (speed >= MINDIST*2)
        speed = MINDIST*2-1;
    xmove = DEMOCHOOSE_ORIG_SDL(
                FixedByFracOrig(speed, costable[angle]),
                FixedMul(speed,costable[angle]));
    ymove = DEMOCHOOSE_ORIG_SDL(
                -FixedByFracOrig(speed, sintable[angle]),
                -FixedMul(speed,sintable[angle]));
    ClipMove(player,xmove,ymove);
    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);
    offset = (player->tiley<<mapshift)+player->tilex;
    player->areanumber = *((quint16*)wlcache->map(0) + offset) -AREATILE;
    if (*(wlcache->map(1) + offset) == EXITTILE)
        VictoryTile ();
}

void Cmd_Fire (void)
{
    buttonheld[bt_attack] = true;
    gamestate.weaponframe = 0;
    player->state = &s_attack;
    gamestate.attackframe = 0;
    gamestate.attackcount =
            attackinfo[gamestate.weapon][gamestate.attackframe].tics;
    gamestate.weaponframe =
            attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

void Cmd_Use (void)
{
    int     checkx,checky,doornum,dir;
    qint8 elevatorok;

    if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8) {
        checkx = player->tilex + 1;
        checky = player->tiley;
        dir = di_east;
        elevatorok = true;
    } else if (player->angle < 3*ANGLES/8) {
        checkx = player->tilex;
        checky = player->tiley-1;
        dir = di_north;
        elevatorok = false;
    } else if (player->angle < 5*ANGLES/8) {
        checkx = player->tilex - 1;
        checky = player->tiley;
        dir = di_west;
        elevatorok = true;
    } else {
        checkx = player->tilex;
        checky = player->tiley + 1;
        dir = di_south;
        elevatorok = false;
    }

    doornum = tilemap[checkx][checky];
    if (*((quint16*)wlcache->map(1)+(checky<<mapshift)+checkx) == PUSHABLETILE) {
        wlitem->pushWall(checkx, checky, dir);
        return;
    }
    if (!buttonheld[bt_use] && doornum == ELEVATORTILE && elevatorok) {
        buttonheld[bt_use] = true;

        tilemap[checkx][checky]++;              // flip switch
        if (*(wlcache->map(0)+(player->tiley<<mapshift)+player->tilex) == ALTELEVATORTILE)
            playstate = ex_secretlevel;
        else
            playstate = ex_completed;
        wlaudio->playSound("LEVELDONESND");
        wlaudio->waitSoundDone();
    } else if (!buttonheld[bt_use] && doornum & 0x80) {
        buttonheld[bt_use] = true;
        wlitem->operateDoor(doornum & ~0x80);
    } else
        wlaudio->playSound("DONOTHINGSND");
}

void SpawnPlayer (int tilex, int tiley, int dir)
{
    player->obclass = playerobj;
    player->active = ac_yes;
    player->tilex = tilex;
    player->tiley = tiley;
    player->areanumber = (quint8) *(wlcache->map(0)+(player->tiley<<mapshift)+player->tilex);
    player->x = ((qint32)tilex<<TILESHIFT)+TILEGLOBAL/2;
    player->y = ((qint32)tiley<<TILESHIFT)+TILEGLOBAL/2;
    player->state = &s_player;
    player->angle = (1-dir)*90;
    if (player->angle<0)
        player->angle += ANGLES;
    player->flags = FL_NEVERMARK;
    Thrust (0,0);                           // set some variables

    wlitem->initAreas();
}

void KnifeAttack(objtype *ob)
{
    objtype *check,*closest;
    qint32  dist;

    wlaudio->playSound("ATKKNIFESND");
    // actually fire
    dist = 0x7fffffff;
    closest = NULL;
    for (check=ob->next; check; check=check->next) {
        if ( (check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
                && abs(check->viewx-centerx) < shootdelta) {
            if (check->transx < dist) {
                dist = check->transx;
                closest = check;
            }
        }
    }
    if (!closest || dist > 0x18000l) {
        // missed
        return;
    }
    // hit something
    wlstate->damageActor(closest, wluser->US_RndT() >> 4);
}



void GunAttack(objtype *ob)
{
    objtype *check,*closest,*oldclosest;
    int      damage;
    int      dx,dy,dist;
    qint32  viewdist;

    switch (gamestate.weapon) {
    case wp_pistol:
        wlaudio->playSound("ATKPISTOLSND");
        break;
    case wp_machinegun:
        wlaudio->playSound("ATKMACHINEGUNSND");
        break;
    case wp_chaingun:
        wlaudio->playSound("ATKGATLINGSND");
        break;
    default:
        break;
    }
    madenoise = true;
    viewdist = 0x7fffffffl;
    closest = NULL;
    while (1) {
        oldclosest = closest;
        for (check=ob->next ; check ; check=check->next) {
            if ((check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
                    && abs(check->viewx-centerx) < shootdelta) {
                if (check->transx < viewdist) {
                    viewdist = check->transx;
                    closest = check;
                }
            }
        }
        if (closest == oldclosest)
            return;
        if (wlstate->checkLine(closest))
            break;
    }
    dx = ABS(closest->tilex - player->tilex);
    dy = ABS(closest->tiley - player->tiley);
    dist = dx > dy ? dx : dy;
    if (dist < 2)
        damage = wluser->US_RndT() / 4;
    else if (dist < 4)
        damage = wluser->US_RndT() / 6;
    else {
        if ( (wluser->US_RndT() / 12) < dist)
            return;
        damage = wluser->US_RndT() / 6;
    }
    wlstate->damageActor(closest, damage);
}

void VictorySpin (void)
{
    qint32    desty;

    if (player->angle > 270) {
        player->angle -= (short)(tics * 3);
        if (player->angle < 270)
            player->angle = 270;
    } else if (player->angle < 270) {
        player->angle += (short)(tics * 3);
        if (player->angle > 270)
            player->angle = 270;
    }
    desty = (((qint32)player->tiley-5)<<TILESHIFT)-0x3000;
    if (player->y > desty) {
        player->y -= tics*4096;
        if (player->y < desty)
            player->y = desty;
    }
}

void    T_Attack (objtype *ob)
{
    struct  atkinf  *cur;
    UpdateFace ();
    if (gamestate.victoryflag) {
        VictorySpin ();
        return;
    }
    if ( buttonstate[bt_use] && !buttonheld[bt_use] )
        buttonstate[bt_use] = false;
    if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
        buttonstate[bt_attack] = false;
    ControlMovement (ob);
    if (gamestate.victoryflag)              // watching the BJ actor
        return;
    plux = (quint16) (player->x >> UNSIGNEDSHIFT);                     // scale to fit in unsigned
    pluy = (quint16) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);
    gamestate.attackcount -= (short) tics;
    while (gamestate.attackcount <= 0) {
        cur = &attackinfo[gamestate.weapon][gamestate.attackframe];
        switch (cur->attack) {
        case -1:
            ob->state = &s_player;
            if (!gamestate.ammo) {
                gamestate.weapon = wp_knife;
                DrawWeapon ();
            } else {
                if (gamestate.weapon != gamestate.chosenweapon) {
                    gamestate.weapon = gamestate.chosenweapon;
                    DrawWeapon ();
                }
            }
            gamestate.attackframe = gamestate.weaponframe = 0;
            return;
        case 4:
            if (!gamestate.ammo)
                break;
            if (buttonstate[bt_attack])
                gamestate.attackframe -= 2;
        case 1:
            if (!gamestate.ammo) {       // can only happen with chain gun
                gamestate.attackframe++;
                break;
            }
            GunAttack (ob);
            if (!ammocheat)
                gamestate.ammo--;
            DrawAmmo ();
            break;
        case 2:
            KnifeAttack (ob);
            break;

        case 3:
            if (gamestate.ammo && buttonstate[bt_attack])
                gamestate.attackframe -= 2;
            break;
        }
        gamestate.attackcount += cur->tics;
        gamestate.attackframe++;
        gamestate.weaponframe =
                attackinfo[gamestate.weapon][gamestate.attackframe].frame;
    }
}

void T_Player(objtype *ob)
{
    if (gamestate.victoryflag) {
        VictorySpin ();
        return;
    }
    UpdateFace ();
    CheckWeaponChange ();
    if ( buttonstate[bt_use] )
        Cmd_Use ();
    if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
        Cmd_Fire ();
    ControlMovement (ob);
    if (gamestate.victoryflag)              // watching the BJ actor
        return;
    plux = (quint16) (player->x >> UNSIGNEDSHIFT);                     // scale to fit in unsigned
    pluy = (quint16) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (short)(player->x >> TILESHIFT);                // scale to tile values
    player->tiley = (short)(player->y >> TILESHIFT);
}
