#include "wl_def.h"

#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wlitem.h"
#include "wlrender.h"
#include "wlpaint.h"
#include "wluser.h"
#include "main.h"

#include "wlplay.h"

#include <QApplication>
#include <QStringList>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QDir>

extern WLCache*  wlcache;
extern WLInput*  wlinput;
extern WLAudio*  wlaudio;
extern WLItem*   wlitem;
extern WLRender* wlrender;
extern WLPaint*  wlpaint;
extern WLUser*   wluser;

extern QImage* g_Image;
extern QTime*  g_getTicks;
extern QSize   g_screenSize;

extern void gsleep(int ms);

#define sc_Question     0x35
#define MAXX            320
#define MAXY            160
#define NUMREDSHIFTS    6
#define REDSTEPS        8

#define NUMWHITESHIFTS  3
#define WHITESTEPS      20
#define WHITETICS       6

qint8 madenoise;
exit_t playstate;
//static musicNames lastmusicchunk = (musicNames) 0;
static qint32 lastmusicchunk = 0;
objtype objlist[MAXACTORS];
objtype *newobj, *obj, *player, *lastobj, *objfreelist, *killerobj;
qint8 noclip, ammocheat;
int godmode, singlestep, extravbls = 1;
quint8 tilemap[MAPSIZE][MAPSIZE];
quint8 spotvis[MAPSIZE][MAPSIZE];
objtype *actorat[MAPSIZE][MAPSIZE];
unsigned tics;
qint8 mouseenabled, joystickenabled;
int dirscan[4] = { sc_UpArrow, sc_RightArrow, sc_DownArrow, sc_LeftArrow };
int buttonscan[NUMBUTTONS] = { sc_Control, sc_Alt, sc_LShift, sc_Space, sc_1, sc_2, sc_3, sc_4 };
int buttonmouse[4] = { bt_attack, bt_strafe, bt_use, bt_nobutton };
int buttonjoy[32] = {
    bt_attack, bt_strafe, bt_use, bt_run, bt_strafeleft, bt_straferight, bt_esc, bt_pause,
    bt_prevweapon, bt_nextweapon, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton,
    bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton,
    bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton, bt_nobutton
};
int viewsize;
qint8 buttonheld[NUMBUTTONS];
qint8 demorecord, demoplayback;
int8_t *demoptr, *lastdemoptr;
void* demobuffer;
int controlx, controly;
qint8 buttonstate[NUMBUTTONS];
int lastgamemusicoffset = 0;
objtype dummyobj;
int objcount;
Color_t redshifts[NUMREDSHIFTS][256];
Color_t whiteshifts[NUMWHITESHIFTS][256];
qint32 funnyticount;
int damagecount, bonuscount;
qint8 palshifted;

QString songs[] = {
    // Episode One
    "GETTHEM_MUS", "SEARCHN_MUS", "POW_MUS", "SUSPENSE_MUS", "GETTHEM_MUS", "SEARCHN_MUS", "POW_MUS", "SUSPENSE_MUS",
    "WARMARCH_MUS", "CORNER_MUS",
    // Episode Two
    "NAZI_OMI_MUS", "PREGNANT_MUS", "GOINGAFT_MUS", "HEADACHE_MUS", "NAZI_OMI_MUS", "PREGNANT_MUS", "HEADACHE_MUS", "GOINGAFT_MUS",
    "WARMARCH_MUS", "DUNGEON_MUS",
    // Episode Three
    "INTROCW3_MUS", "NAZI_RAP_MUS", "TWELFTH_MUS", "ZEROHOUR_MUS", "INTROCW3_MUS", "NAZI_RAP_MUS", "TWELFTH_MUS", "ZEROHOUR_MUS",
    "ULTIMATE_MUS", "PACMAN_MUS",
    // Episode Four
    "GETTHEM_MUS", "SEARCHN_MUS", "POW_MUS", "SUSPENSE_MUS", "GETTHEM_MUS", "SEARCHN_MUS", "POW_MUS", "SUSPENSE_MUS",
    "WARMARCH_MUS", "CORNER_MUS",
    // Episode Five
    "NAZI_OMI_MUS", "PREGNANT_MUS", "GOINGAFT_MUS", "HEADACHE_MUS", "NAZI_OMI_MUS", "PREGNANT_MUS", "HEADACHE_MUS", "GOINGAFT_MUS",
    "WARMARCH_MUS", "DUNGEON_MUS",
    // Episode Six
    "INTROCW3_MUS", "NAZI_RAP_MUS", "TWELFTH_MUS", "ZEROHOUR_MUS", "INTROCW3_MUS", "NAZI_RAP_MUS", "TWELFTH_MUS", "ZEROHOUR_MUS",
    "ULTIMATE_MUS", "FUNKYOU_MUS"
};

WLPlay::WLPlay()
{

}

WLPlay::~WLPlay()
{

}

void WLPlay::pollKeyboardButtons()
{
    for (int i = 0; i < NUMBUTTONS; i++) {
        if (wlinput->isKeyDown((int)buttonscan[i])) {
            buttonstate[i] = true;
        }
    }
}

void WLPlay::pollKeyboardMove()
{
    qint32 delta = buttonstate[bt_run] ? RUNMOVE * tics : BASEMOVE * tics;

    if (wlinput->isKeyDown(dirscan[di_north]))
        controly -= delta;
    if (wlinput->isKeyDown(dirscan[di_south]))
        controly += delta;
    if (wlinput->isKeyDown(dirscan[di_west]))
        controlx -= delta;
    if (wlinput->isKeyDown(dirscan[di_east]))
        controlx += delta;
}

void WLPlay::pollControls()
{
    qint32 max, min, i;
    quint8 buttonbits;

    if (demoplayback || demorecord) {
        quint32 curtime = g_getTicks->elapsed();
        lasttimecount += DEMOTICS;
        qint32 timediff = (lasttimecount * 100) / 7 - curtime;
        if(timediff > 0)
            gsleep(timediff);

        if(timediff < -2 * DEMOTICS)
            lasttimecount = (curtime * 7) / 100;

        tics = DEMOTICS;
    } else
        CalcTics ();

    controlx = 0;
    controly = 0;
    memcpy (buttonheld, buttonstate, sizeof (buttonstate));
    memset (buttonstate, 0, sizeof (buttonstate));

    if (demoplayback) {
        buttonbits = *demoptr++;
        for (i = 0; i < NUMBUTTONS; i++)
        {
            buttonstate[i] = buttonbits & 1;
            buttonbits >>= 1;
        }

        controlx = *demoptr++;
        controly = *demoptr++;

        if (demoptr == lastdemoptr)
            playstate = ex_completed;

        controlx *= (int) tics;
        controly *= (int) tics;

        return;
    }

    pollKeyboardButtons();
    pollKeyboardMove();

#ifndef TOUCHSCREEN
    if (wlinput->buttons[1]) {
        //right button held (move around)
        if (wlinput->mousePos.x() > wlinput->oldPos.x()+5) {
            //left
            controlx -= BASEMOVE * tics;
            wlinput->oldPos = wlinput->mousePos;
        } else if (wlinput->mousePos.x() < wlinput->oldPos.x()-5) {
            //right
            controlx += BASEMOVE * tics;
            wlinput->oldPos = wlinput->mousePos;
        } else if (wlinput->mousePos.y() > wlinput->oldPos.y()+5) {
            //down
            controly += BASEMOVE * tics;
            wlinput->oldPos = wlinput->mousePos;
        } else if (wlinput->mousePos.y() < wlinput->oldPos.y()-5) {
            //up
            controly -= BASEMOVE * tics;
            wlinput->oldPos = wlinput->mousePos;
        }
    }
    if (wlinput->buttons[0]) {
        // left button, fire
        buttonstate[bt_attack] = true;
    }
    if (wlinput->buttons[3]) {
        // middle button, use
        buttonstate[bt_use] = true;
    }
#else
    if (wlinput->mousePos != QPoint(0,0)) {
        if (wlinput->mousePos.x() < g_screenSize.width()*0.2 && wlinput->mousePos.y() < g_screenSize.height()*0.2)
            buttonstate[bt_esc] = true; // top left is Esc
        else if (wlinput->mousePos.x() < g_screenSize.width()*0.2 && wlinput->mousePos.y() > g_screenSize.height()*0.8)
            buttonstate[bt_attack] =  true; // bottom left is fire
        else if (wlinput->mousePos.x() > g_screenSize.width()*0.8 && wlinput->mousePos.y() > g_screenSize.height()*0.8)
            buttonstate[bt_use] = true; // bottom right is use
        else if (wlinput->mousePos.x() > g_screenSize.width()*0.8 && wlinput->mousePos.y() < g_screenSize.height()*0.2)
            buttonstate[bt_nextweapon] = true; // toggle through weapons
        else {
            int x1 = 0;
            int y1 = 0;
            int x2 = wlinput->mousePos.x() - g_screenSize.width()/2;
            int y2 = wlinput->mousePos.y() - g_screenSize.height()/2;
            int distance = qSqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
            if (x2 < -distance/2)
                controlx -= distance/8 * tics;
            else if (x2 > distance/2)
                controlx += distance/8 * tics;
            else if (y2 < -distance/2)
                controly -= distance/8 * tics;
            else if (y2 > distance/2)
                controly += distance/8 * tics;
        }
    }
    if (wlinput->x <= -5) {
        controlx += BASEMOVE * tics;
        wlinput->x = 0;
    } else if (wlinput->x >= 5) {
        controlx -= BASEMOVE * tics;
        wlinput->x = 0;
    } else if (wlinput->y <= -5) {
        controly -= BASEMOVE * tics;
        wlinput->y = 0;
    } else if (wlinput->y >= 5) {
        controly += BASEMOVE * tics;
        wlinput->y = 0;
    }
#endif

    max = 100 * tics;
    min = -max;
    if (controlx > max)
        controlx = max;
    else if (controlx < min)
        controlx = min;

    if (controly > max)
        controly = max;
    else if (controly < min)
        controly = min;

    if (demorecord) {
        controlx /= (int) tics;
        controly /= (int) tics;

        buttonbits = 0;

        for (i = NUMBUTTONS - 1; i >= 0; i--) {
            buttonbits <<= 1;
            if (buttonstate[i])
                buttonbits |= 1;
        }

        *demoptr++ = buttonbits;
        *demoptr++ = controlx;
        *demoptr++ = controly;

        if (demoptr >= lastdemoptr - 8)
            playstate = ex_completed;
        else {
            controlx *= (int) tics;
            controly *= (int) tics;
        }
    }
}

void WLPlay::centerWindow(quint16 w, quint16 h)
{
    wluser->US_DrawWindow(((MAXX / 8) - w) / 2, ((MAXY / 8) - h) / 2, w, h);
}

void WLPlay::checkKeys()
{
    ScanCode scan;

    if (screenfaded || demoplayback)
        return;

    qApp->processEvents();

    scan = wlinput->lastScan();

    // SECRET CHEAT CODE: 'MLI'
    if (wlinput->isKeyDown(sc_M) && wlinput->isKeyDown(sc_L) && wlinput->isKeyDown(sc_I)) {
        gamestate.health = 100;
        gamestate.ammo = 99;
        gamestate.keys = 3;
        gamestate.score = 0;
        gamestate.TimeCount += 42000L;
        godmode = 2;
        GiveWeapon (wp_chaingun);
        DrawWeapon ();
        DrawHealth ();
        DrawKeys ();
        DrawAmmo ();
        DrawScore ();

        wlaudio->stopDigitized();
        wlcache->cacheGraphic(STARTFONT + 1);
        ClearSplitVWB ();

        Message (STR_CHEATER1 "\n"
                 STR_CHEATER2 "\n\n" STR_CHEATER3 "\n" STR_CHEATER4 "\n" STR_CHEATER5);
        wlpaint->updateScreen();
        wlcache->uncacheGraphic(STARTFONT + 1);
        gsleep(500);
        wlinput->clearKeysDown();
        wlinput->ack();

        if (viewsize < 17)
            DrawPlayBorder ();
    }
    // screenshot
    if (wlinput->isKeyDown(sc_F10)) {
        screenShot();
        gsleep(500);
        wlinput->clearKeysDown();
    }
    if (wlinput->isKeyDown(sc_M) && !wlinput->isKeyDown(sc_L))
        showMap();

    // TRYING THE KEEN CHEAT CODE!
    if (wlinput->isKeyDown(sc_B) && wlinput->isKeyDown(sc_A) && wlinput->isKeyDown(sc_T))
    {
        wlaudio->stopDigitized();
        wlcache->cacheGraphic(STARTFONT + 1);
        ClearSplitVWB ();

        Message ("Commander Keen is also\n"
                 "available from Apogee, but\n"
                 "then, you already know\n" "that - right, Cheatmeister?!");

        wlcache->uncacheGraphic(STARTFONT + 1);
        gsleep(1000);
        wlinput->clearKeysDown();
        wlinput->ack();

        if (viewsize < 18)
            DrawPlayBorder ();
    }

    //
    // pause key weirdness can't be checked as a scan code
    //
    if(buttonstate[bt_pause]) wlinput->setPaused(true);
    if(wlinput->isPaused()) {
        int lastoffs = stopMusic();
        wlpaint->latchDrawPic(20 - 4, 80 - 2 * 8, wlcache->find("PAUSEDPIC"));
        wlpaint->updateScreen();
        wlinput->ack();
        wlinput->setPaused(false);
        continueMusic(lastoffs);
        lasttimecount = g_getTicks->elapsed()*7/100;
        return;
    }

    //
    // F1-F7/ESC to enter control panel
    //
    if (
        #ifndef DEBCHECK
            scan == sc_F10 ||
        #endif
            scan == sc_F9 || scan == sc_F7 || scan == sc_F8)     // pop up quit dialog
    {
        wlaudio->stopDigitized();
        ClearSplitVWB ();
        US_ControlPanel (scan);

        DrawPlayBorderSides ();

        SETFONTCOLOR (0, 15);
        wlinput->clearKeysDown();
        return;
    }

    if ((scan >= sc_F1 && scan <= sc_F2) || scan == sc_Escape || buttonstate[bt_esc])
    {
        int lastoffs = stopMusic();
        wlaudio->stopDigitized();
        wlrender->fadeOut(0, 255, 0, 0, 0, 30);

        US_ControlPanel (buttonstate[bt_esc] ? sc_Escape : scan);

        SETFONTCOLOR (0, 15);
        wlinput->clearKeysDown();
        wlrender->fadeOut(0, 255, 0, 0, 0, 30);
        if(viewsize != 21)
            DrawPlayScreen ();
        if (!startgame && !loadedgame)
            continueMusic(lastoffs);
        if (loadedgame)
            playstate = ex_abort;
        lasttimecount = g_getTicks->elapsed()*7/100;
        return;
    }
}

void WLPlay::initActorList()
{
    for (int i = 0; i < MAXACTORS; i++) {
        objlist[i].prev = &objlist[i + 1];
        objlist[i].next = NULL;
    }

    objlist[MAXACTORS - 1].prev = NULL;

    objfreelist = &objlist[0];
    lastobj = NULL;

    objcount = 0;

    getNewActor();
    player = newobj;
}

void WLPlay::getNewActor()
{
    if (!objfreelist)
        Quit ("GetNewActor: No free spots in objlist!");

    newobj = objfreelist;
    objfreelist = newobj->prev;
    memset (newobj, 0, sizeof (*newobj));

    if (lastobj)
        lastobj->next = newobj;
    newobj->prev = lastobj;     // new->next is allready NULL from memset

    newobj->active = ac_no;
    lastobj = newobj;

    objcount++;
}

void WLPlay::removeObj(objtype * gone)
{
    if (gone == player)
        Quit ("RemoveObj: Tried to remove the player!");

    gone->state = NULL;
    if (gone == lastobj)
        lastobj = (objtype *) gone->prev;
    else
        gone->next->prev = gone->prev;
    gone->prev->next = gone->next;
    gone->prev = objfreelist;
    objfreelist = gone;
    objcount--;
}

int WLPlay::stopMusic()
{
    int lastoffs = wlaudio->musicOff();
    wlcache->uncacheAudio(wlcache->findSound("LASTSOUND")*3 + lastmusicchunk); // STARTMUSIC
    return lastoffs;
}

void WLPlay::startMusic()
{
    wlaudio->musicOff();
    lastmusicchunk = wlcache->findMusic(songs[gamestate.mapon + gamestate.episode * 10]);
    wlaudio->startMusic(wlcache->findSound("LASTSOUND")*3 + lastmusicchunk); //STARTMUSIC
}

void WLPlay::continueMusic(qint32 offs)
{
    wlaudio->musicOff();
    lastmusicchunk = wlcache->findMusic(songs[gamestate.mapon + gamestate.episode * 10]);
    wlaudio->continueMusic(wlcache->findSound("LASTSOUND")*3 + lastmusicchunk, offs); //STARTMUSIC
}

void WLPlay::initRedShifts()
{
    Color_t *workptr, *baseptr;
    int i, j, delta;

    for (i = 1; i <= NUMREDSHIFTS; i++) {
        workptr = redshifts[i - 1];
        baseptr = gamepal;

        for (j = 0; j <= 255; j++) {
            delta = 256 - baseptr->r;
            workptr->r = baseptr->r + delta * i / REDSTEPS;
            delta = -baseptr->g;
            workptr->g = baseptr->g + delta * i / REDSTEPS;
            delta = -baseptr->b;
            workptr->b = baseptr->b + delta * i / REDSTEPS;
            baseptr++;
            workptr++;
        }
    }

    for (i = 1; i <= NUMWHITESHIFTS; i++) {
        workptr = whiteshifts[i - 1];
        baseptr = gamepal;

        for (j = 0; j <= 255; j++) {
            delta = 256 - baseptr->r;
            workptr->r = baseptr->r + delta * i / WHITESTEPS;
            delta = 248 - baseptr->g;
            workptr->g = baseptr->g + delta * i / WHITESTEPS;
            delta = 0-baseptr->b;
            workptr->b = baseptr->b + delta * i / WHITESTEPS;
            baseptr++;
            workptr++;
        }
    }
}

void WLPlay::clearPaletteShifts()
{
    bonuscount = damagecount = 0;
    palshifted = false;
}

void WLPlay::startBonusFlash()
{
    bonuscount = NUMWHITESHIFTS * WHITETICS;
}

void WLPlay::startDamageFlash(qint32 damage)
{
    damagecount += damage;
}

void WLPlay::updatePaletteShifts()
{
    qint32 red, white;

    if (bonuscount) {
        white = bonuscount / WHITETICS + 1;
        if (white > NUMWHITESHIFTS)
            white = NUMWHITESHIFTS;
        bonuscount -= tics;
        if (bonuscount < 0)
            bonuscount = 0;
    } else
        white = 0;


    if (damagecount) {
        red = damagecount / 10 + 1;
        if (red > NUMREDSHIFTS)
            red = NUMREDSHIFTS;

        damagecount -= tics;
        if (damagecount < 0)
            damagecount = 0;
    } else
        red = 0;

    if (red) {
        wlrender->setPalette(redshifts[red - 1]);
        palshifted = true;
    } else if (white) {
        wlrender->setPalette(whiteshifts[white - 1]);
        palshifted = true;
    } else if (palshifted) {
        wlrender->setPalette(gamepal);
        palshifted = false;
    }
}

void WLPlay::finishPaletteShifts()
{
    if (palshifted) {
        palshifted = 0;
        wlrender->setPalette(gamepal);
    }
}

void WLPlay::doActor(objtype * ob)
{
    void (*think) (objtype *);

    if (!ob->active && !*wlitem->areaByPlayer(ob->areanumber))
        return;

    if (!(ob->flags & (FL_NONMARK | FL_NEVERMARK)))
        actorat[ob->tilex][ob->tiley] = NULL;

    if (!ob->ticcount) {
        think = (void (*)(objtype *)) ob->state->think;
        if (think) {
            think (ob);
            if (!ob->state) {
                removeObj(ob);
                return;
            }
        }

        if (ob->flags & FL_NEVERMARK)
            return;

        if ((ob->flags & FL_NONMARK) && actorat[ob->tilex][ob->tiley])
            return;

        actorat[ob->tilex][ob->tiley] = ob;
        return;
    }
    ob->ticcount -= (short) tics;
    while (ob->ticcount <= 0) {
        think = (void (*)(objtype *)) ob->state->action;
        if (think) {
            think (ob);
            if (!ob->state) {
                removeObj(ob);
                return;
            }
        }
        ob->state = ob->state->next;
        if (!ob->state) {
            removeObj(ob);
            return;
        }
        if (!ob->state->tictime) {
            ob->ticcount = 0;
            goto think;
        }
        ob->ticcount += ob->state->tictime;
    }

    think:
        think = (void (*)(objtype *)) ob->state->think;
    if (think) {
        think (ob);
        if (!ob->state) {
            removeObj(ob);
            return;
        }
    }

    if (ob->flags & FL_NEVERMARK)
        return;

    if ((ob->flags & FL_NONMARK) && actorat[ob->tilex][ob->tiley])
        return;

    actorat[ob->tilex][ob->tiley] = ob;
}

void WLPlay::showMap()
{
    qint32      tile;
    quint16*    start;

    start = (quint16*)wlcache->map(0);
    if (!start) return;
    wlrender->bar((320 - mapwidth*2)/2 - 3, (180 - mapheight*2)/2 -3, mapwidth*2+6, 3, 254);
    wlrender->bar((320 - mapwidth*2)/2 - 3, (180 - mapheight*2)/2 -3, 3, mapheight*2+6, 254);
    wlrender->bar((320 - mapwidth*2)/2 - 3, (180 - mapheight*2)/2 + mapheight*2, mapwidth*2+3, 3, 254);
    wlrender->bar((320 - mapwidth*2)/2 + mapwidth*2, (180 - mapheight*2)/2, 3, mapheight*2+3, 254);
    for (qint32 y = 0; y < mapheight; y++) {
        for (qint32 x = 0; x < mapwidth; x++) {
            tile = *start++;
            int color = 0;
            if (tile >= 106 && tile <= 143)
                  color = 7; // light grey floor
            bool show = false;
            if (x > 0 && y > 0 && wlcache->fog(x - 1, y - 1)) show = true; // top left
            if (!show && y > 0 && wlcache->fog(x, y - 1)) show = true; // top middle
            if (!show && y > 0 && x < (mapwidth - 1) && wlcache->fog(x + 1, y - 1)) show = true; // top right
            if (!show && x > 0 && wlcache->fog(x - 1, y)) show = true; // left
            if (!show && wlcache->fog(x, y)) show = true; // middle
            if (!show && x < (mapwidth - 1) && wlcache->fog(x + 1, y)) show = true; //right
            if (!show && x > 0 && y < (mapheight - 1) && wlcache->fog(x - 1, y + 1)) show = true; // bottom left
            if (!show && y < (mapheight - 1) && wlcache->fog(x, y + 1)) show = true; // bottom middle
            if (!show && x < (mapwidth - 1) && y < (mapheight - 1) && wlcache->fog(x + 1, y + 1)) show = true; // bottom right
            if (show)
                wlrender->bar((320 - mapwidth*2)/2 + x*2, (180 - mapheight*2)/2 + y*2, 2, 2, color);
            else
                wlrender->bar((320 - mapwidth*2)/2 + x*2, (180 - mapheight*2)/2 + y*2, 2, 2, 139);
        }
    }
    wlrender->bar((320 - mapwidth*2)/2 + (quint16)(player->x >> TILESHIFT) * 2 - 1,
                  (180 - mapheight*2)/2 + (quint16)(player->y >> TILESHIFT) * 2 - 1 , 2, 2, 55);

    wlpaint->updateScreen();
    gsleep(500);
    wlinput->clearKeysDown();
    wlinput->ack();
}

void WLPlay::screenShot()
{
    QDir dir;
    QStringList files = dir.entryList(QDir::Files, QDir::Name);
    int number = 0;
    foreach(QString file, files) {
        if (file.startsWith("snapshot")) {
            int n = file.replace("snapshot","").replace(".png","").toInt();
            if (number < n)
                number = n;
        }
    }
    number++;
    g_Image->save(QString("snapshot%1.png").arg(number), "PNG");
    gsleep(1000);
}

void WLPlay::playLoop()
{
    playstate = ex_stillplaying;
    lasttimecount = g_getTicks->elapsed()*7/100;
    frameon = 0;
    anglefrac = 0;
    facecount = 0;
    funnyticount = 0;
    memset (buttonstate, 0, sizeof (buttonstate));
    clearPaletteShifts();

    QTime tm;

    if (demoplayback)
        wlinput->startAck();
    do {
        tm.restart();
        pollControls();
        madenoise = false;
        wlcache->setFog(true, (quint16)(player->x >> TILESHIFT), (quint16)(player->y >> TILESHIFT));
        wlitem->moveDoors();
        wlitem->movePWalls();
        for (obj = player; obj; obj = obj->next)
            doActor(obj);
        updatePaletteShifts();
        ThreeDRefresh ();
        gamestate.TimeCount += tics;

        UpdateSoundLoc ();
        if (screenfaded)
            wlrender->fadeIn(0, 255, gamepal, 30);
        checkKeys();

        if (singlestep)
        {
            gsleep(singlestep*8);
            lasttimecount = g_getTicks->elapsed()*7/100;
        }
        if (extravbls)
            gsleep(extravbls*8);

        if (demoplayback)
        {
            if (wlinput->checkAck())
            {
                wlinput->clearKeysDown();
                playstate = ex_abort;
            }
        }
        if (tm.elapsed() < 40)
            gsleep(40 - tm.elapsed());
    }
    while (!playstate && !startgame);

    if (playstate != ex_died)
        finishPaletteShifts();
}
