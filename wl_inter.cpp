#include "wl_def.h"

#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wltext.h"
#include "wlrender.h"
#include "wlpaint.h"
#include "wluser.h"

#include <QDebug>

extern WLCache*  wlcache;
extern WLInput*  wlinput;
extern WLAudio*  wlaudio;
extern WLText*   wltext;
extern WLRender* wlrender;
extern WLPaint*  wlpaint;
extern WLUser*   wluser;

extern QTime*  g_getTicks;
extern QImage* g_Image;

extern qint32 g_bordColor;

extern void gsleep(int ms);

LRstruct LevelRatios[LRpack];
qint32 lastBreathTime = 0;

void Write (int x, int y, const char *string);

void ClearSplitVWB(void)
{
    WindowX = 0;
    WindowY = 0;
    WindowW = 320;
    WindowH = 160;
}

void Victory (void)
{
    qint32 sec;
    int i, min, kr, sr, tr, x;
    char tempstr[8];

#define RATIOX  6
#define RATIOY  14
#define TIMEX   14
#define TIMEY   8

    StartCPMusic (wlcache->findMusic("URAHERO_MUS"));
    ClearSplitVWB ();
    CacheLump(wlcache->find("L_GUYPIC"), wlcache->find("L_BJWINSPIC"));
    wlcache->cacheGraphic(STARTFONT);
    wlcache->cacheGraphic(wlcache->find("C_TIMECODEPIC"));
    wlpaint->bar(0, 0, 320, g_Image->height() / scaleFactor - STATUSLINES + 1, VIEWCOLOR);
    if (bordercol != VIEWCOLOR)
        DrawStatusBorder (VIEWCOLOR);

    Write (18, 2, STR_YOUWIN);
    Write (TIMEX, TIMEY - 2, STR_TOTALTIME);
    Write (12, RATIOY - 2, "averages");
    Write (RATIOX + 8, RATIOY, STR_RATKILL);
    Write (RATIOX + 4, RATIOY + 2, STR_RATSECRET);
    Write (RATIOX, RATIOY + 4, STR_RATTREASURE);
    wlpaint->drawPic(8, 4, wlcache->find("L_BJWINSPIC"));

    for (kr = sr = tr = sec = i = 0; i < LRpack; i++) {
        sec += LevelRatios[i].time;
        kr += LevelRatios[i].kill;
        sr += LevelRatios[i].secret;
        tr += LevelRatios[i].treasure;
    }
    kr /= LRpack;
    sr /= LRpack;
    tr /= LRpack;
    min = sec / 60;
    sec %= 60;
    if (min > 99)
        min = sec = 99;

    i = TIMEX * 8 + 1;
    wlpaint->drawPic(i, TIMEY * 8, wlcache->find("L_NUM0PIC") + (min / 10));
    i += 2 * 8;
    wlpaint->drawPic(i, TIMEY * 8, wlcache->find("L_NUM0PIC") + (min % 10));
    i += 2 * 8;
    Write (i / 8, TIMEY, ":");
    i += 1 * 8;
    wlpaint->drawPic(i, TIMEY * 8, wlcache->find("L_NUM0PIC") + (sec / 10));
    i += 2 * 8;
    wlpaint->drawPic(i, TIMEY * 8, wlcache->find("L_NUM0PIC") + (sec % 10));
    wlpaint->updateScreen();
    itoa (kr, tempstr, 10);
    x = RATIOX + 24 - (int) strlen(tempstr) * 2;
    Write (x, RATIOY, tempstr);
    itoa (sr, tempstr, 10);
    x = RATIOX + 24 - (int) strlen(tempstr) * 2;
    Write (x, RATIOY + 2, tempstr);
    itoa (tr, tempstr, 10);
    x = RATIOX + 24 - (int) strlen(tempstr) * 2;
    Write (x, RATIOY + 4, tempstr);
    fontnumber = 1;
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 30);
    wlinput->ack();
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    if(g_Image->height() % 200 != 0)
        wlrender->clearScreen(0);
    wlcache->uncacheGraphic(wlcache->find("C_TIMECODEPIC"));
    UnCacheLump(wlcache->find("L_GUYPIC"), wlcache->find("L_BJWINSPIC"));
    wltext->endText();
    for (int xx = 0; xx < mapwidth; xx++)
        for (int yy = 0; yy < mapheight; yy++)
            wlcache->setFog(false, xx, yy);
}

void PG13(void)
{
//    for (int i = 0; i < 169; i++) {
//        wlpaint->bar(0, 0, 320, 200, 0x82);     // background
//        wlcache->cacheGraphic(i);
//        wlpaint->drawPic(50, 50, i);
//        //wlcache->cacheScreen(i);
//        wlpaint->updateScreen();
//        wlcache->uncacheGraphic(i);
//        qWarning()<<"i="<<i;
//        wlinput->userInput(g_tickBase * 7);
//    }

    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    wlpaint->bar(0, 0, 320, 200, 0x82);     // background
    wlcache->cacheGraphic(wlcache->find("PG13PIC"));
    wlpaint->drawPic(216, 110, wlcache->find("PG13PIC"));
    wlpaint->updateScreen();
    wlcache->uncacheGraphic(wlcache->find("PG13PIC"));
    wlrender->fadeIn(0, 255, gamepal, 30);
    wlinput->userInput(g_tickBase * 7);
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
}

void Write(int x, int y, const char *string)
{
    static const int alpha[] = { wlcache->find("L_NUM0PIC"), wlcache->find("L_NUM1PIC"), wlcache->find("L_NUM2PIC"),
                                 wlcache->find("L_NUM3PIC"), wlcache->find("L_NUM4PIC"), wlcache->find("L_NUM5PIC"),
                                 wlcache->find("L_NUM6PIC"), wlcache->find("L_NUM7PIC"), wlcache->find("L_NUM8PIC"),
                                 wlcache->find("L_NUM9PIC"), wlcache->find("L_COLONPIC"), 0, 0, 0, 0, 0, 0,
                                 wlcache->find("L_APIC"), wlcache->find("L_BPIC"), wlcache->find("L_CPIC"),
                                 wlcache->find("L_DPIC"), wlcache->find("L_EPIC"), wlcache->find("L_FPIC"),
                                 wlcache->find("L_GPIC"), wlcache->find("L_HPIC"), wlcache->find("L_IPIC"),
                                 wlcache->find("L_JPIC"), wlcache->find("L_KPIC"), wlcache->find("L_LPIC"),
                                 wlcache->find("L_MPIC"), wlcache->find("L_NPIC"), wlcache->find("L_OPIC"),
                                 wlcache->find("L_PPIC"), wlcache->find("L_QPIC"), wlcache->find("L_RPIC"),
                                 wlcache->find("L_SPIC"), wlcache->find("L_TPIC"), wlcache->find("L_UPIC"),
                                 wlcache->find("L_VPIC"), wlcache->find("L_WPIC"), wlcache->find("L_XPIC"),
                                 wlcache->find("L_YPIC"), wlcache->find("L_ZPIC")
                               };

    int i, ox, nx, ny, len = (int) strlen(string);
    char ch;

    ox = nx = x * 8;
    ny = y * 8;
    for (i = 0; i < len; i++) {
        if (string[i] == '\n') {
            nx = ox;
            ny += 16;
        } else {
            ch = string[i];
            if (ch >= 'a')
                ch -= ('a' - 'A');
            ch -= '0';

            switch (string[i]) {
            case '!':
                wlpaint->drawPic(nx, ny, wlcache->find("L_EXPOINTPIC"));
                nx += 8;
                continue;
            case '\'':
                wlpaint->drawPic(nx, ny, wlcache->find("L_APOSTROPHEPIC"));
                nx += 8;
                continue;
            case ' ':
                break;

            case 0x3a:     // ':'
                wlpaint->drawPic(nx, ny, wlcache->find("L_COLONPIC"));
                nx += 8;
                continue;

            case '%':
                wlpaint->drawPic(nx, ny, wlcache->find("L_PERCENTPIC"));
                break;

            default:
                wlpaint->drawPic(nx, ny, alpha[(qint32)ch]);
            }
            nx += 16;
        }
    }
}

void BJ_Breathe (void)
{
    static int which = 0, max = 10;
    int pics[2] = { wlcache->find("L_GUYPIC"), wlcache->find("L_GUY2PIC") };

    gsleep(5);

    if ((qint32) g_getTicks->elapsed()*7/100 - lastBreathTime > max) {
        which ^= 1;
        wlpaint->drawPic(0, 16, pics[which]);
        wlpaint->updateScreen();
        lastBreathTime = g_getTicks->elapsed()*7/100;
        max = 35;
    }
}

void LevelCompleted (void)
{
#define VBLWAIT 30
#define PAR_AMOUNT      500
#define PERCENT100AMT   10000

    typedef struct {
        float time;
        char timestr[6];
    } times;

    int x, i, min, sec, ratio, kr, sr, tr;
    char tempstr[10];
    qint32 bonus, timeleft = 0;
    times parTimes[] = {
        //
        // Episode One Par Times
        //
        {1.5, "01:30"},
        {2, "02:00"},
        {2, "02:00"},
        {3.5, "03:30"},
        {3, "03:00"},
        {3, "03:00"},
        {2.5, "02:30"},
        {2.5, "02:30"},
        {0, "??:??"},           // Boss level
        {0, "??:??"},           // Secret level

        //
        // Episode Two Par Times
        //
        {1.5, "01:30"},
        {3.5, "03:30"},
        {3, "03:00"},
        {2, "02:00"},
        {4, "04:00"},
        {6, "06:00"},
        {1, "01:00"},
        {3, "03:00"},
        {0, "??:??"},
        {0, "??:??"},

        //
        // Episode Three Par Times
        //
        {1.5, "01:30"},
        {1.5, "01:30"},
        {2.5, "02:30"},
        {2.5, "02:30"},
        {3.5, "03:30"},
        {2.5, "02:30"},
        {2, "02:00"},
        {6, "06:00"},
        {0, "??:??"},
        {0, "??:??"},

        //
        // Episode Four Par Times
        //
        {2, "02:00"},
        {2, "02:00"},
        {1.5, "01:30"},
        {1, "01:00"},
        {4.5, "04:30"},
        {3.5, "03:30"},
        {2, "02:00"},
        {4.5, "04:30"},
        {0, "??:??"},
        {0, "??:??"},

        //
        // Episode Five Par Times
        //
        {2.5, "02:30"},
        {1.5, "01:30"},
        {2.5, "02:30"},
        {2.5, "02:30"},
        {4, "04:00"},
        {3, "03:00"},
        {4.5, "04:30"},
        {3.5, "03:30"},
        {0, "??:??"},
        {0, "??:??"},

        //
        // Episode Six Par Times
        //
        {6.5, "06:30"},
        {4, "04:00"},
        {4.5, "04:30"},
        {6, "06:00"},
        {5, "05:00"},
        {5.5, "05:30"},
        {5.5, "05:30"},
        {8.5, "08:30"},
        {0, "??:??"},
        {0, "??:??"}
    };

    CacheLump(wlcache->find("L_GUYPIC"), wlcache->find("L_BJWINSPIC"));
    ClearSplitVWB ();           // set up for double buffering in split screen
    wlpaint->bar(0, 0, 320, g_Image->height() / scaleFactor - STATUSLINES + 1, VIEWCOLOR);

    if (bordercol != VIEWCOLOR)
        DrawStatusBorder (VIEWCOLOR);

    StartCPMusic(wlcache->findMusic("ENDLEVEL_MUS"));
    wlinput->clearKeysDown();
    wlinput->startAck();
    wlpaint->drawPic(0, 16, wlcache->find("L_GUYPIC"));

    if (wlcache->currentMap() < 8) {
        Write (14, 2, "floor\ncompleted");
        Write (14, 7, STR_BONUS "     0");
        Write (16, 10, STR_TIME);
        Write (16, 12, STR_PAR);
        Write (9, 14, STR_RAT2KILL);
        Write (5, 16, STR_RAT2SECRET);
        Write (1, 18, STR_RAT2TREASURE);
        Write (26, 2, itoa (gamestate.mapon + 1, tempstr, 10));
        Write (26, 12, parTimes[gamestate.episode * 10 + wlcache->currentMap()].timestr);
        sec = gamestate.TimeCount / 70;

        if (sec > 99 * 60)      // 99 minutes max
            sec = 99 * 60;

        if (gamestate.TimeCount < parTimes[gamestate.episode * 10 + wlcache->currentMap()].time * 4200)
            timeleft = (qint32) ((parTimes[gamestate.episode * 10 + wlcache->currentMap()].time * 4200) / 70 - sec);

        min = sec / 60;
        sec %= 60;
        i = 26 * 8;
        wlpaint->drawPic(i, 10 * 8, wlcache->find("L_NUM0PIC") + (min / 10));
        i += 2 * 8;
        wlpaint->drawPic(i, 10 * 8, wlcache->find("L_NUM0PIC") + (min % 10));
        i += 2 * 8;
        Write (i / 8, 10, ":");
        i += 1 * 8;
        wlpaint->drawPic(i, 10 * 8, wlcache->find("L_NUM0PIC") + (sec / 10));
        i += 2 * 8;
        wlpaint->drawPic(i, 10 * 8, wlcache->find("L_NUM0PIC") + (sec % 10));
        wlpaint->updateScreen();
        wlrender->fadeIn(0, 255, gamepal, 30);

        kr = sr = tr = 0;
        if (gamestate.killtotal)
            kr = (gamestate.killcount * 100) / gamestate.killtotal;
        if (gamestate.secrettotal)
            sr = (gamestate.secretcount * 100) / gamestate.secrettotal;
        if (gamestate.treasuretotal)
            tr = (gamestate.treasurecount * 100) / gamestate.treasuretotal;

        bonus = timeleft * PAR_AMOUNT;
        if (bonus) {
            for (i = 0; i <= timeleft; i++) {
                ltoa ((qint32) i * PAR_AMOUNT, tempstr, 10);
                x = 36 - (int) strlen(tempstr) * 2;
                Write (x, 7, tempstr);
                if (!(i % (PAR_AMOUNT / 10)))
                    wlaudio->playSound("ENDBONUS1SND");
                wlpaint->updateScreen();
                while (wlaudio->soundPlaying())
                    BJ_Breathe ();
                if (wlinput->checkAck())
                    goto done;
            }
            wlpaint->updateScreen();
            wlaudio->playSound("ENDBONUS2SND");
            while (wlaudio->soundPlaying())
                BJ_Breathe ();
        }

#define RATIOXX                37
        ratio = kr;
        for (i = 0; i <= ratio; i++) {
            itoa (i, tempstr, 10);
            x = RATIOXX - (int) strlen(tempstr) * 2;
            Write (x, 14, tempstr);
            if (!(i % 10))
                wlaudio->playSound("ENDBONUS1SND");
            wlpaint->updateScreen();
            while (wlaudio->soundPlaying())
                BJ_Breathe ();

            if (wlinput->checkAck())
                goto done;
        }
        if (ratio >= 100) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            bonus += PERCENT100AMT;
            ltoa (bonus, tempstr, 10);
            x = (RATIOXX - 1) - (int) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            wlpaint->updateScreen();
            wlaudio->playSound("PERCENT100SND");
        } else if (!ratio) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            wlaudio->playSound("NOBONUSSND");
        } else
            wlaudio->playSound("ENDBONUS2SND");

        wlpaint->updateScreen();
        while (wlaudio->soundPlaying())
            BJ_Breathe ();

        ratio = sr;
        for (i = 0; i <= ratio; i++) {
            itoa (i, tempstr, 10);
            x = RATIOXX - (int) strlen(tempstr) * 2;
            Write (x, 16, tempstr);
            if (!(i % 10))
                wlaudio->playSound("ENDBONUS1SND");
            wlpaint->updateScreen();
            while (wlaudio->soundPlaying())
                BJ_Breathe ();
            BJ_Breathe ();

            if (wlinput->checkAck())
                goto done;
        }
        if (ratio >= 100) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            bonus += PERCENT100AMT;
            ltoa (bonus, tempstr, 10);
            x = (RATIOXX - 1) - (int) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            wlpaint->updateScreen();
            wlaudio->playSound("PERCENT100SND");
        } else if (!ratio) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            wlaudio->playSound("NOBONUSSND");
        } else
            wlaudio->playSound("ENDBONUS2SND");
        wlpaint->updateScreen();
        while (wlaudio->soundPlaying())
            BJ_Breathe ();

        ratio = tr;
        for (i = 0; i <= ratio; i++) {
            itoa (i, tempstr, 10);
            x = RATIOXX - (int) strlen(tempstr) * 2;
            Write (x, 18, tempstr);
            if (!(i % 10))
                wlaudio->playSound("ENDBONUS1SND");
            wlpaint->updateScreen();
            while (wlaudio->soundPlaying())
                BJ_Breathe ();
            if (wlinput->checkAck())
                goto done;
        }
        if (ratio >= 100) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            bonus += PERCENT100AMT;
            ltoa (bonus, tempstr, 10);
            x = (RATIOXX - 1) - (int) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            wlpaint->updateScreen();
            wlaudio->playSound("PERCENT100SND");
        } else if (!ratio) {
            gsleep(VBLWAIT*8);
            wlaudio->stopSound();
            wlaudio->playSound("NOBONUSSND");
        } else
            wlaudio->playSound("ENDBONUS2SND");
        wlpaint->updateScreen();
        while (wlaudio->soundPlaying())
            BJ_Breathe ();

        done:   itoa (kr, tempstr, 10);
        x = RATIOXX - (int) strlen(tempstr) * 2;
        Write (x, 14, tempstr);
        itoa (sr, tempstr, 10);
        x = RATIOXX - (int) strlen(tempstr) * 2;
        Write (x, 16, tempstr);
        itoa (tr, tempstr, 10);
        x = RATIOXX - (int) strlen(tempstr) * 2;
        Write (x, 18, tempstr);
        bonus = (qint32) timeleft *PAR_AMOUNT +
                (PERCENT100AMT * (kr >= 100)) +
                (PERCENT100AMT * (sr >= 100)) + (PERCENT100AMT * (tr >= 100));
        GivePoints (bonus);
        ltoa (bonus, tempstr, 10);
        x = 36 - (int) strlen(tempstr) * 2;
        Write (x, 7, tempstr);
        LevelRatios[wlcache->currentMap()].kill = kr;
        LevelRatios[wlcache->currentMap()].secret = sr;
        LevelRatios[wlcache->currentMap()].treasure = tr;
        LevelRatios[wlcache->currentMap()].time = min * 60 + sec;
    } else {
        Write (14, 4, "secret floor\n completed!");
        Write (10, 16, "15000 bonus!");
        wlpaint->updateScreen();
        wlrender->fadeIn(0, 255, gamepal, 30);
        GivePoints (15000);
    }
    DrawScore ();
    wlpaint->updateScreen();
    lastBreathTime = g_getTicks->elapsed()*7/100;
    wlinput->startAck();
    while (!wlinput->checkAck())
        BJ_Breathe ();
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    DrawPlayBorder();
    UnCacheLump(wlcache->find("L_GUYPIC"), wlcache->find("L_BJWINSPIC"));
}

qint8 PreloadUpdate(unsigned current, unsigned total)
{
    unsigned w = WindowW - scaleFactor * 10;

    wlrender->barScaledCoord(WindowX + scaleFactor * 5, WindowY + WindowH - scaleFactor * 3,
                        w, scaleFactor * 2, BLACK);
    w = ((qint32) w * current) / total;
    if (w) {
        wlrender->barScaledCoord(WindowX + scaleFactor * 5, WindowY + WindowH - scaleFactor * 3,
                            w, scaleFactor * 2, 0x37);
        wlrender->barScaledCoord(WindowX + scaleFactor * 5, WindowY + WindowH - scaleFactor * 3,
                            w - scaleFactor * 1, scaleFactor * 1, 0x32);

    }
    wlpaint->updateScreen();
    return (false);
}

void PreloadGraphics (void)
{
    DrawLevel ();
    ClearSplitVWB ();

    wlrender->barScaledCoord(0, 0, g_Image->width(), g_Image->height() - scaleFactor * (STATUSLINES - 1), bordercol);
    wlpaint->latchDrawPicScaledCoord((g_Image->width() - scaleFactor * 224) / 16,
                             (g_Image->height() - scaleFactor * (STATUSLINES + 48)) / 2, wlcache->find("GETPSYCHEDPIC"));

    WindowX = (g_Image->width() - scaleFactor*224)/2;
    WindowY = (g_Image->height() - scaleFactor*(STATUSLINES+48))/2;
    WindowW = scaleFactor * 28 * 8;
    WindowH = scaleFactor * 48;
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 30);
    PreloadUpdate (10, 10);
    wlinput->userInput(70);
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    DrawPlayBorder ();
    wlpaint->updateScreen();
}

void DrawHighScores (void)
{
    char buffer[16];
    char *str;
    quint16 i, w, h;
    HighScore *s;

    wlcache->cacheGraphic(wlcache->find("HIGHSCORESPIC"));
    wlcache->cacheGraphic(STARTFONT);
    wlcache->cacheGraphic(wlcache->find("C_LEVELPIC"));
    wlcache->cacheGraphic(wlcache->find("C_SCOREPIC"));
    wlcache->cacheGraphic(wlcache->find("C_NAMEPIC"));
    ClearMScreen ();
    DrawStripes (10);
    wlpaint->drawPic(48, 0, wlcache->find("HIGHSCORESPIC"));
    wlcache->uncacheGraphic(wlcache->find("HIGHSCORESPIC"));
    wlpaint->drawPic(4 * 8, 68, wlcache->find("C_NAMEPIC"));
    wlpaint->drawPic(20 * 8, 68, wlcache->find("C_LEVELPIC"));
    wlpaint->drawPic(28 * 8, 68, wlcache->find("C_SCOREPIC"));
    fontnumber = 0;
    SETFONTCOLOR (15, 0x29);

    for (i = 0, s = Scores; i < MaxScores; i++, s++) {
        PrintY = 76 + (16 * i);
        PrintX = 4 * 8;
        wluser->US_Print(s->name);
        itoa (s->completed, buffer, 10);
        for (str = buffer; *str; str++)
            *str = *str + (129 - '0');
        wlpaint->measurePropString(buffer, &w, &h);
        PrintX = (22 * 8) - w;
        wluser->US_Print(buffer);
        itoa (s->score, buffer, 10);
        for (str = buffer; *str; str++)
            *str = *str + (129 - '0');
        wlpaint->measurePropString(buffer, &w, &h);
        PrintX = (34 * 8) - 8 - w;
        wluser->US_Print(buffer);
    }
    wlpaint->updateScreen();
}

void CheckHighScore (qint32 score, quint16 other)
{
    quint16 i, j;
    int n;
    HighScore myscore;

    strcpy (myscore.name, "");
    myscore.score = score;
    myscore.episode = gamestate.episode;
    myscore.completed = other;

    for (i = 0, n = -1; i < MaxScores; i++) {
        if ((myscore.score > Scores[i].score)
                || ((myscore.score == Scores[i].score) && (myscore.completed > Scores[i].completed))) {
            for (j = MaxScores; --j > i;)
                Scores[j] = Scores[j - 1];
            Scores[i] = myscore;
            n = i;
            break;
        }
    }
    StartCPMusic(wlcache->findMusic("ROSTER_MUS"));
    DrawHighScores ();
    wlrender->fadeIn(0, 255, gamepal, 30);
    if (n != -1) {
        PrintY = 76 + (16 * n);
        PrintX = 4 * 8;
        backcolor = g_bordColor; //BORDCOLOR;
        fontcolor = 15;
        wluser->US_LineInput(PrintX, PrintY, Scores[n].name, 0, true, MaxHighName, 100);
    } else {
        wlinput->clearKeysDown();
        wlinput->userInput(500);
    }
}

