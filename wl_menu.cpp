#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "wl_def.h"

#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wltext.h"
#include "wlrender.h"
#include "wlplay.h"
#include "wlpaint.h"
#include "wluser.h"
#include "main.h"

#include <QDir>
#include <QFile>

qint32 g_bkgdColor     = 0x2d;
qint32 g_bord2Color    = 0x23;
qint32 g_deActiveColor = 0x2b;
qint32 g_bordColor     = 0x29;
qint32 g_stripColor    = 0x2c;

extern WLCache*  wlcache;
extern WLInput*  wlinput;
extern WLAudio*  wlaudio;
extern WLText*   wltext;
extern WLRender* wlrender;
extern WLPlay*   wlplay;
extern WLPaint*  wlpaint;
extern WLUser*   wluser;
extern GameEngine* engine;

extern QTime*   g_getTicks;
extern QImage*  g_Image;
extern QString  g_Extension;
extern bool     g_UpperCase;
extern QString* g_graphic;
extern QString* g_sound;
extern QString* g_music;

extern void gsleep(int ms);

extern int lastgamemusicoffset;
extern bool ingame;

int CP_ReadThis (int);
const char *IN_GetScanName(ScanCode);


#define STARTITEM       readthis

char endStrings[9][80] = {
    {"Dost thou wish to\nleave with such hasty\nabandon?"},
    {"Chickening out...\nalready?"},
    {"Press N for more carnage.\nPress Y to be a weenie."},
    {"So, you think you can\nquit this easily, huh?"},
    {"Press N to save the world.\nPress Y to abandon it in\nits hour of need."},
    {"Press N if you are brave.\nPress Y to cower in shame."},
    {"Heroes, press N.\nWimps, press Y."},
    {"You are at an intersection.\nA sign says, 'Press Y to quit.'\n>"},
    {"For guns and glory, press N.\nFor work and worry, press Y."}
};

CP_itemtype MainMenu[] = {
    {1, STR_NG, CP_NewGame},
    {1, STR_SD, CP_Sound},
    {1, STR_CL, CP_Control},
    {1, STR_LG, CP_LoadGame},
    {0, STR_SG, CP_SaveGame},
    {1, STR_CV, CP_ChangeView},
    {2, "Read This!", CP_ReadThis},
    {1, STR_VS, CP_ViewScores},
    {1, STR_BD, 0},
    {1, STR_QT, 0}
};

CP_itemtype SndMenu[] = {
    {1, STR_NONE, 0},
    {0, STR_PC, 0},
    {1, STR_ALSB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {0, STR_DISNEY, 0},
    {1, STR_SB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {1, STR_ALSB, 0}
};

enum { CTL_MOUSEENABLE, CTL_MOUSESENS, CTL_JOYENABLE, CTL_CUSTOMIZE };

CP_itemtype CtlMenu[] = {
    {0, STR_MOUSEEN, 0},
    {0, STR_SENS, MouseSensitivity},
    {0, STR_JOYEN, 0},
    {1, STR_CUSTOM, CustomControls}
};

CP_itemtype NewEmenu[] = {
    {1, "Episode 1\n" "Escape from Wolfenstein", 0},
    {0, "", 0},
    {3, "Episode 2\n" "Operation: Eisenfaust", 0},
    {0, "", 0},
    {3, "Episode 3\n" "Die, Fuhrer, Die!", 0},
    {0, "", 0},
    {3, "Episode 4\n" "A Dark Secret", 0},
    {0, "", 0},
    {3, "Episode 5\n" "Trail of the Madman", 0},
    {0, "", 0},
    {3, "Episode 6\n" "Confrontation", 0}
};

CP_itemtype NewMenu[] = {
    {1, STR_DADDY, 0},
    {1, STR_HURTME, 0},
    {1, STR_BRINGEM, 0},
    {1, STR_DEATH, 0}
};

CP_itemtype LSMenu[] = {
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0}
};

CP_itemtype CusMenu[] = {
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0}
};

// CP_iteminfo struct format: short x, y, amount, curpos, indent;
CP_iteminfo MainItems = { MENU_X, MENU_Y, lengthof(MainMenu), STARTITEM, 24 },
SndItems  = { SM_X, SM_Y1, lengthof(SndMenu), 0, 52 },
LSItems   = { LSM_X, LSM_Y, lengthof(LSMenu), 0, 24 },
CtlItems  = { CTL_X, CTL_Y, lengthof(CtlMenu), -1, 56 },
CusItems  = { 8, CST_Y + 13 * 2, lengthof(CusMenu), -1, 0},
NewEitems = { NE_X, NE_Y, lengthof(NewEmenu), 0, 88 },
NewItems  = { NM_X, NM_Y, lengthof(NewMenu), 2, 24 };

int color_hlite[] = {
    g_deActiveColor, //DEACTIVE,
    HIGHLIGHT,
    READHCOLOR,
    0x67
};

int color_norml[] = {
    g_deActiveColor, //DEACTIVE,
    TEXTCOLOR,
    READCOLOR,
    0x6b
};

int EpisodeSelect[6] = { 1 };


static int SaveGamesAvail[10];
static int StartGame;
static int SoundStatus = 1;
static int pickquick;
static char SaveGameNames[10][32];
static char SaveName[13] = "savegam?.";


static const char* const ScanNames[200] =
{
    "?","?","?","?","?","?","?","?",                       // 0x0  - 0x7
    "?","?","?","?","?","?","?","?",                       // 0x8  - 0xF
    "?","?","?","?","?","?","?","?",                       // 0x10 - 0x17
    "?","?","?","?","?","?","?","?",                       // 0x18 - 0x1F
    "Space","?","?","?","?","?","?","?",                   // 0x20 - 0x27
    "?","?","?","?","?","?","?","?",                       // 0x28 - 0x2F
    "0","1","2","3","4","5","6","7",                       // 0x30 - 0x37
    "8","9","?","?","?","?","?","?",                       // 0x38 - 0x3F
    "?","A","B","C","D","E","F","G",                       // 0x40 - 0x47
    "H","I","J","K","L","M","N","O",                       // 0x48 - 0x4F
    "P","Q","R","S","T","U","V","W",                       // 0x50 - 0x57
    "X","Y","Z","?","?","?","?","?",                       // 0x58 - 0x5F
    "Esc","Tab","?","BkSp","Enter","Return","?","Del",     // 0x60 - 0x67
    "Pause","PrtScrn","?","?","?","?","?","?",             // 0x68 - 0x6F
    "Home","End","Left","Up","Right","Down","PgUp","pgDn", // 0x70 - 0x77
    "?","?","?","?","?","?","?","?",                       // 0x78 - 0x7F
    "Shift","Ctrl","?","Alt","?","?","?","?",              // 0x80 - 0x87
    "?","?","?","?","?","?","?","?",                       // 0x88 - 0x8F
    "F1","F2","F3","F4","F5","F6","F7","F8",               // 0x90 - 0x97
    "F9","F10"                                             // 0x98 - 0x9A
};

void US_ControlPanel (ScanCode scancode)
{
    int which;

    if (ingame) {
        if (CP_CheckQuick (scancode))
            return;
        lastgamemusicoffset = StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
    } else
        StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
    SetupControlPanel ();

    //
    // F-KEYS FROM WITHIN GAME
    //
    switch (scancode) {
    case sc_F1:
        wltext->helpScreens();
        goto finishup;

    case sc_F2:
        CP_SaveGame (0);
        goto finishup;

    case sc_F3:
        CP_LoadGame (0);
        goto finishup;

    case sc_F4:
        CP_Sound (0);
        goto finishup;

    case sc_F5:
        CP_ChangeView (0);
        goto finishup;

    case sc_F6:
        CP_Control (0);
        goto finishup;

        finishup:
            CleanupControlPanel ();
        return;
    }
    DrawMainMenu ();
    wlrender->fadeIn(0, 255, gamepal, 10);
    StartGame = 0;

    //
    // MAIN MENU LOOP
    //
    do {
        which = HandleMenu (&MainItems, &MainMenu[0], NULL);
        switch (which) {
        case viewscores:
            if (MainMenu[viewscores].routine == NULL) {
                if (CP_EndGame (0))
                    StartGame = 1;
            } else {
                DrawMainMenu();
                wlrender->fadeIn(0, 255, gamepal, 10);
            }
            break;

        case backtodemo:
            StartGame = 1;
            if (!ingame)
                StartCPMusic(wlcache->findMusic("NAZI_NOR_MUS"));
            wlrender->fadeOut(0, 255, 0, 0, 0, 10);
            break;

        case -1:
        case quit:
            CP_Quit (0);
            break;

        default:
            if (!StartGame) {
                DrawMainMenu ();
                wlrender->fadeIn(0, 255, gamepal, 10);
            }
        }
    }
    while (!StartGame);
    CleanupControlPanel();
    if (startgame || loadedgame)
        EnableEndGameMenuItem();
}

void EnableEndGameMenuItem()
{
    MainMenu[viewscores].routine = NULL;
#ifndef JAPAN
    strcpy (MainMenu[viewscores].string, STR_EG);
#endif
}

void DrawMainMenu(void)
{
    ClearMScreen ();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawStripes (10);
    wlpaint->drawPic(84, 0, wlcache->find("C_OPTIONSPIC"));
    DrawWindow (MENU_X - 8, MENU_Y - 3, MENU_W, MENU_H, g_bkgdColor);

    if (ingame) {
        strcpy (&MainMenu[backtodemo].string[8], STR_GAME);
        MainMenu[backtodemo].active = 2;
    } else {
        strcpy (&MainMenu[backtodemo].string[8], STR_DEMO);
        MainMenu[backtodemo].active = 1;
    }
    DrawMenu (&MainItems, &MainMenu[0]);
    wlpaint->updateScreen();
}

int CP_ReadThis (int)
{
    StartCPMusic(wlcache->findMusic("CORNER_MUS"));
    wltext->helpScreens();
    StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
    return true;
}

int CP_CheckQuick (ScanCode scancode)
{
    switch (scancode) {
    case sc_F7:
        wlcache->cacheGraphic(STARTFONT + 1);
        WindowH = 160;
        if (Confirm (ENDGAMESTR)) {
            playstate = ex_died;
            killerobj = NULL;
            pickquick = gamestate.lives = 0;
        }
        WindowH = 200;
        fontnumber = 0;
        MainMenu[savegame].active = 0;
        return 1;
    case sc_F8:
        if (SaveGamesAvail[LSItems.curpos] && pickquick) {
            wlcache->cacheGraphic(STARTFONT + 1);
            fontnumber = 1;
            Message (STR_SAVING "...");
            CP_SaveGame (1);
            fontnumber = 0;
        } else {
            wlcache->cacheGraphic(STARTFONT + 1);
            wlcache->cacheGraphic(wlcache->find("C_CURSOR1PIC"));
            wlcache->cacheGraphic(wlcache->find("C_CURSOR2PIC"));
            wlcache->cacheGraphic(wlcache->find("C_DISKLOADING1PIC"));
            wlcache->cacheGraphic(wlcache->find("C_DISKLOADING2PIC"));
            wlcache->cacheGraphic(wlcache->find("C_SAVEGAMEPIC"));
            wlcache->cacheGraphic(wlcache->find("C_MOUSELBACKPIC"));
            wlrender->fadeOut(0, 255, 0, 0, 0, 10); //VW_FadeOut ();
            if(g_Image->height() % 200 != 0)
                wlrender->clearScreen(0);
            lastgamemusicoffset = StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
            pickquick = CP_SaveGame (0);
            SETFONTCOLOR (0, 15);
            wlinput->clearKeysDown();
            wlrender->fadeOut(0, 255, 0, 0, 0, 10); //VW_FadeOut();
            if(viewsize != 21)
                DrawPlayScreen ();
            if (!startgame && !loadedgame)
                wlplay->continueMusic(lastgamemusicoffset);
            if (loadedgame)
                playstate = ex_abort;
            lasttimecount = g_getTicks->elapsed()*7/100;

            wlcache->uncacheGraphic(wlcache->find("C_CURSOR1PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_CURSOR2PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_DISKLOADING1PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_DISKLOADING2PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_SAVEGAMEPIC"));
            wlcache->uncacheGraphic(wlcache->find("C_MOUSELBACKPIC"));
        }
        return 1;
    case sc_F9:
        if (SaveGamesAvail[LSItems.curpos] && pickquick) {
            char string[100] = STR_LGC;
            wlcache->cacheGraphic(STARTFONT + 1);
            fontnumber = 1;
            strcat (string, SaveGameNames[LSItems.curpos]);
            strcat (string, "\"?");
            if (Confirm (string))
                CP_LoadGame (1);
            fontnumber = 0;
        } else {
            wlcache->cacheGraphic(STARTFONT + 1);
            wlcache->cacheGraphic(wlcache->find("C_CURSOR1PIC"));
            wlcache->cacheGraphic(wlcache->find("C_CURSOR2PIC"));
            wlcache->cacheGraphic(wlcache->find("C_DISKLOADING1PIC"));
            wlcache->cacheGraphic(wlcache->find("C_DISKLOADING2PIC"));
            wlcache->cacheGraphic(wlcache->find("C_LOADGAMEPIC"));
            wlcache->cacheGraphic(wlcache->find("C_MOUSELBACKPIC"));
            wlrender->fadeOut(0, 255, 0, 0, 0, 10); //VW_FadeOut ();
            if(g_Image->height() % 200 != 0)
                wlrender->clearScreen(0);
            lastgamemusicoffset = StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
            pickquick = CP_LoadGame (0);    // loads lastgamemusicoffs
            SETFONTCOLOR (0, 15);
            wlinput->clearKeysDown();
            wlrender->fadeOut(0, 255, 0, 0, 0, 10); //VW_FadeOut();
            if(viewsize != 21)
                DrawPlayScreen ();
            if (!startgame && !loadedgame)
                wlplay->continueMusic(lastgamemusicoffset);
            if (loadedgame)
                playstate = ex_abort;
            lasttimecount = g_getTicks->elapsed()*7/100;
            wlcache->uncacheGraphic(wlcache->find("C_CURSOR1PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_CURSOR2PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_DISKLOADING1PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_DISKLOADING2PIC"));
            wlcache->uncacheGraphic(wlcache->find("C_LOADGAMEPIC"));
            wlcache->uncacheGraphic(wlcache->find("C_MOUSELBACKPIC"));
        }
        return 1;
    case sc_F10:
        wlcache->cacheGraphic(STARTFONT + 1);
        WindowX = WindowY = 0;
        WindowW = 320;
        WindowH = 160;
        if (Confirm (endStrings[(wluser->US_RndT()&0x7) + (wluser->US_RndT()&1)])) {
            wlpaint->updateScreen();
            wlaudio->musicOff();
            wlaudio->stopSound();
            wlrender->fadeOut(0, 255, 43, 0, 0, 10); //MenuFadeOut ();
            Quit (NULL);
        }
        DrawPlayBorder ();
        WindowH = 200;
        fontnumber = 0;
        return 1;
    }
    return 0;
}

int CP_EndGame (int)
{
    int res;
    res = Confirm (ENDGAMESTR);
    DrawMainMenu();
    if(!res) return 0;
    pickquick = gamestate.lives = 0;
    playstate = ex_died;
    killerobj = NULL;
    MainMenu[savegame].active = 0;
    MainMenu[viewscores].routine = CP_ViewScores;
    strcpy (MainMenu[viewscores].string, STR_VS);
    return 1;
}

int CP_ViewScores (int)
{
    fontnumber = 0;
    StartCPMusic(wlcache->findMusic("ROSTER_MUS"));
    DrawHighScores ();
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
    fontnumber = 1;
    gsleep(500);
    wlinput->clearKeysDown();
    wlinput->userInput(2000);
    StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return 0;
}

int CP_NewGame (int)
{
    int which, episode;

    firstpart:
        DrawNewEpisode ();
    do {
        which = HandleMenu (&NewEitems, &NewEmenu[0], NULL);
        switch (which) {
        case -1:
            wlrender->fadeOut(0, 255, 43, 0, 0, 10);
            return 0;

        default:
            if (!EpisodeSelect[which / 2]) {
                wlaudio->playSound("NOWAYSND");
                Message ("Please select \"Read This!\"\n"
                         "from the Options menu to\n"
                         "find out how to order this\n" "episode from Apogee.");
                wlinput->clearKeysDown();
                wlinput->ack();
                DrawNewEpisode ();
                which = 0;
            } else {
                episode = which / 2;
                which = 1;
            }
            break;
        }
    }
    while (!which);
    ShootSnd ();

    if (ingame)
        if (!Confirm (CURGAME)) {
            wlrender->fadeOut(0, 255, 43, 0, 0, 10);
            return 0;
        }
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    DrawNewGame ();
    which = HandleMenu (&NewItems, &NewMenu[0], DrawNewGameDiff);
    if (which < 0) {
        wlrender->fadeOut(0, 255, 43, 0, 0, 10);
        goto firstpart;
    }
    ShootSnd ();
    engine->newGame(which, episode);
    StartGame = 1;
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    MainMenu[readthis].active = 1;
    pickquick = 0;
    return 0;
}

void DrawNewEpisode (void)
{
    int i;

    ClearMScreen ();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawWindow (NE_X - 4, NE_Y - 4, NE_W + 8, NE_H + 8, g_bkgdColor);
    SETFONTCOLOR (READHCOLOR, g_bkgdColor);
    PrintY = 2;
    WindowX = 0;
    wluser->US_CPrint("Which episode to play?");
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    DrawMenu (&NewEitems, &NewEmenu[0]);
    for (i = 0; i < 6; i++)
        wlpaint->drawPic(NE_X + 32, NE_Y + i * 26, wlcache->find("C_EPISODE1PIC") + i);
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
    WaitKeyUp ();
}

void DrawNewGame (void)
{
    ClearMScreen ();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    SETFONTCOLOR (READHCOLOR, g_bkgdColor);
    PrintX = NM_X + 20;
    PrintY = NM_Y - 32;
    wluser->US_Print("How tough are you?");
    DrawWindow (NM_X - 5, NM_Y - 10, NM_W, NM_H, g_bkgdColor);
    DrawMenu (&NewItems, &NewMenu[0]);
    DrawNewGameDiff (NewItems.curpos);
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
    WaitKeyUp ();
}

void DrawNewGameDiff (int w)
{
    wlpaint->drawPic(NM_X + 185, NM_Y + 7, w + wlcache->find("C_BABYMODEPIC"));
}

int CP_Sound (int)
{
    int which;
    DrawSoundMenu ();
    wlrender->fadeIn(0, 255, gamepal, 10);
    WaitKeyUp ();

    do {
        which = HandleMenu (&SndItems, &SndMenu[0], NULL);
        switch (which) {
        case 0:
            if (*wlaudio->soundMode() != sdm_Off) {
                wlaudio->waitSoundDone();
                wlaudio->setSoundMode(sdm_Off);
                DrawSoundMenu ();
            }
            break;
        case 1:
            if (*wlaudio->soundMode() != sdm_PC) {
                wlaudio->waitSoundDone();
                wlaudio->setSoundMode(sdm_PC);
                wlcache->loadAllSounds();
                DrawSoundMenu ();
                ShootSnd ();
            }
            break;
        case 2:
            if (*wlaudio->soundMode() != sdm_AdLib) {
                wlaudio->waitSoundDone();
                wlaudio->setSoundMode(sdm_AdLib);
                wlcache->loadAllSounds();
                DrawSoundMenu ();
                ShootSnd ();
            }
            break;
        case 5:
            if (*wlaudio->digiMode() != sds_Off) {
                wlaudio->setDigiDevice(sds_Off);
                DrawSoundMenu ();
            }
            break;
        case 6:
            break;
        case 7:
            if (*wlaudio->digiMode() != sds_SoundBlaster) {
                wlaudio->setDigiDevice(sds_SoundBlaster);
                DrawSoundMenu ();
                ShootSnd ();
            }
            break;
        case 10:
            if (*wlaudio->musicMode() != smm_Off) {
                wlaudio->setMusicMode(smm_Off);
                DrawSoundMenu ();
                ShootSnd ();
            }
            break;
        case 11:
            if (*wlaudio->musicMode() != smm_AdLib) {
                wlaudio->setMusicMode(smm_AdLib);
                DrawSoundMenu ();
                ShootSnd ();
                StartCPMusic(wlcache->findMusic("WONDERIN_MUS"));
            }
            break;
        }
    }
    while (which >= 0);
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return 0;
}

void DrawSoundMenu (void)
{
    int i, on;

    ClearMScreen ();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawWindow (SM_X - 8, SM_Y1 - 3, SM_W, SM_H1, g_bkgdColor);
    DrawWindow (SM_X - 8, SM_Y2 - 3, SM_W, SM_H2, g_bkgdColor);
    DrawWindow (SM_X - 8, SM_Y3 - 3, SM_W, SM_H3, g_bkgdColor);

    if (!wlaudio->hasAdLib() && !wlaudio->hasSoundBlaster()) {
        SndMenu[2].active = SndMenu[10].active = SndMenu[11].active = 0;
    }
    if (!wlaudio->hasSoundBlaster())
        SndMenu[7].active = 0;
    if (!wlaudio->hasSoundBlaster())
        SndMenu[5].active = 0;
    DrawMenu (&SndItems, &SndMenu[0]);
    wlpaint->drawPic(100, SM_Y1 - 20, wlcache->find("C_FXTITLEPIC"));
    wlpaint->drawPic(100, SM_Y2 - 20, wlcache->find("C_DIGITITLEPIC"));
    wlpaint->drawPic(100, SM_Y3 - 20, wlcache->find("C_MUSICTITLEPIC"));

    for (i = 0; i < SndItems.amount; i++)
        if (SndMenu[i].string[0]) {
            on = 0;
            switch (i) {
            case 0:
                if (*wlaudio->soundMode() == sdm_Off)
                    on = 1;
                break;
            case 1:
                if (*wlaudio->soundMode() == sdm_PC)
                    on = 1;
                break;
            case 2:
                if (*wlaudio->soundMode() == sdm_AdLib)
                    on = 1;
                break;
            case 5:
                if (*wlaudio->digiMode() == sds_Off)
                    on = 1;
                break;
            case 6:
                break;
            case 7:
                if (*wlaudio->digiMode() == sds_SoundBlaster)
                    on = 1;
                break;
            case 10:
                if (*wlaudio->musicMode() == smm_Off)
                    on = 1;
                break;
            case 11:
                if (*wlaudio->musicMode() == smm_AdLib)
                    on = 1;
                break;
            }
            if (on)
                wlpaint->drawPic(SM_X + 24, SM_Y1 + i * 13 + 2, wlcache->find("C_SELECTEDPIC"));
            else
                wlpaint->drawPic(SM_X + 24, SM_Y1 + i * 13 + 2, wlcache->find("C_NOTSELECTEDPIC"));
        }
    DrawMenuGun (&SndItems);
    wlpaint->updateScreen();
}

void DrawLSAction (int which)
{
#define LSA_X   96
#define LSA_Y   80
#define LSA_W   130
#define LSA_H   42

    DrawWindow (LSA_X, LSA_Y, LSA_W, LSA_H, TEXTCOLOR);
    DrawOutline (LSA_X, LSA_Y, LSA_W, LSA_H, 0, HIGHLIGHT);
    wlpaint->drawPic(LSA_X + 8, LSA_Y + 5, wlcache->find("C_DISKLOADING1PIC"));

    fontnumber = 1;
    SETFONTCOLOR (0, TEXTCOLOR);
    PrintX = LSA_X + 46;
    PrintY = LSA_Y + 13;

    if (!which)
        wluser->US_Print(STR_LOADING "...");
    else
        wluser->US_Print(STR_SAVING "...");

    wlpaint->updateScreen();
}

int CP_LoadGame(int quick)
{
    int which;
    int exit = 0;
    QString name;

    //QString fileName = QString("%1%2").arg(SaveName).arg(WLEXT);
    QString fileName = SaveName + g_Extension;

    if (quick) {
        which = LSItems.curpos;
        if (SaveGamesAvail[which]) {
            name = fileName;
            name.replace(QString("?"), QString("%1").arg(which));
            QFile fp(name);
            if (fp.open(QIODevice::ReadOnly)) {
                fp.seek(32);
                loadedgame = true;
                engine->loadTheGame(&fp, 0, 0);
                loadedgame = false;
                fp.close();
                DrawFace();
                DrawHealth();
                DrawLives();
                DrawLevel();
                DrawAmmo();
                DrawKeys();
                DrawWeapon();
                DrawScore();
                wlplay->continueMusic(lastgamemusicoffset);
                return 1;
            }
        }
    }
    DrawLoadSaveScreen(0);
    do {
        which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);
        if (which >= 0 && SaveGamesAvail[which]) {
            ShootSnd();
            name = fileName;
            name.replace(QString("?"), QString("%1").arg(which));
            QFile fp(name);
            if (fp.open(QIODevice::ReadOnly)) {
                fp.seek(32);
                DrawLSAction(0);
                loadedgame = true;
                engine->loadTheGame(&fp, LSA_X + 8, LSA_Y + 5);
                fp.close();
                StartGame = 1;
                ShootSnd();
                MainMenu[readthis].active = 1;
                exit = 1;
                break;
            }
        }
    }
    while (which >= 0);
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return exit;
}

void TrackWhichGame (int w)
{
    static int lastgameon = 0;

    PrintLSEntry (lastgameon, TEXTCOLOR);
    PrintLSEntry (w, HIGHLIGHT);
    lastgameon = w;
}

void DrawLoadSaveScreen (int loadsave)
{
#define DISKX   100
#define DISKY   0

    int i;

    ClearMScreen ();
    fontnumber = 1;
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawWindow (LSM_X - 10, LSM_Y - 5, LSM_W, LSM_H, g_bkgdColor);
    DrawStripes (10);
    if (!loadsave)
        wlpaint->drawPic(60, 0, wlcache->find("C_LOADGAMEPIC"));
    else
        wlpaint->drawPic(60, 0, wlcache->find("C_SAVEGAMEPIC"));
    for (i = 0; i < 10; i++)
        PrintLSEntry (i, TEXTCOLOR);
    DrawMenu (&LSItems, &LSMenu[0]);

#ifdef TOUCHSCREEN
    wlinput->setVirtualKeyboard(true);
    px = 5;   py = 50;  wlpaint->drawPropString("A  B  C");
    px = 255; py = 50;  wlpaint->drawPropString("D  E  F");
    px = 5;   py = 70;  wlpaint->drawPropString("G  H  I");
    px = 255; py = 70;  wlpaint->drawPropString("J  K  L");
    px = 5;   py = 90;  wlpaint->drawPropString("M  N  O");
    px = 255; py = 90;  wlpaint->drawPropString("P  Q  R");
    px = 5;   py = 110; wlpaint->drawPropString("S  T  U");
    px = 255; py = 110; wlpaint->drawPropString("V  W  X");
    px = 5;   py = 130; wlpaint->drawPropString("Y  Z  Del");
    px = 255; py = 130; wlpaint->drawPropString("0  1  2");
    px = 5;   py = 150; wlpaint->drawPropString("3  4  5");
    px = 255; py = 150; wlpaint->drawPropString("6  7  8");
    px = 5;   py = 170; wlpaint->drawPropString("9  >  <");
#endif

    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
    WaitKeyUp ();
}

void PrintLSEntry (int w, int color)
{
    SETFONTCOLOR (color, g_bkgdColor);
    DrawOutline (LSM_X + LSItems.indent, LSM_Y + w * 13, LSM_W - LSItems.indent - 15, 11, color,
                 color);
    PrintX = LSM_X + LSItems.indent + 2;
    PrintY = LSM_Y + w * 13 + 1;
    fontnumber = 0;

    if (SaveGamesAvail[w])
        wluser->US_Print(SaveGameNames[w]);
    else
        wluser->US_Print("      - " STR_EMPTY " -");

    fontnumber = 1;
}

int CP_SaveGame (int quick)
{
    int which;
    int exit = 0;
    QString name;

    //QString fileName = QString("%1%2").arg(SaveName).arg(WLEXT);
    QString fileName = SaveName + g_Extension;

    char input[32];

    if (quick) {
        which = LSItems.curpos;
        if (SaveGamesAvail[which]) {
            name = fileName.replace(QString("?"), QString("%1").arg(which)); //(char)(which+48)));
            QFile fp(fileName);
            fp.remove(fileName);
            if (fp.open(QIODevice::WriteOnly)) {
                strcpy (input, &SaveGameNames[which][0]);
                fp.write((char*)input, 32);
                engine->saveTheGame(&fp, 0, 0);
                fp.close();
                return 1;
            }
        }
    }
    DrawLoadSaveScreen (1);
    do {
        which = HandleMenu (&LSItems, &LSMenu[0], TrackWhichGame);
        if (which >= 0) {
            if (SaveGamesAvail[which]) {
                if (!Confirm (GAMESVD)) {
                    DrawLoadSaveScreen (1);
                    continue;
                } else {
                    DrawLoadSaveScreen (1);
                    PrintLSEntry (which, HIGHLIGHT);
                    wlpaint->updateScreen();
                }
            }
            ShootSnd ();
            strcpy (input, &SaveGameNames[which][0]);
            name = fileName.replace(QString("?"), QString("%1").arg((char)(which+48)));
            QFile fp(fileName);
            fontnumber = 0;
            if (!SaveGamesAvail[which])
                wlpaint->bar(LSM_X + LSItems.indent + 1, LSM_Y + which * 13 + 1,
                         LSM_W - LSItems.indent - 16, 10, g_bkgdColor);
            wlpaint->updateScreen();
            if (wluser->US_LineInput
                    (LSM_X + LSItems.indent + 2, LSM_Y + which * 13 + 1, input, input, true, 31,
                     LSM_W - LSItems.indent - 30)) {
                SaveGamesAvail[which] = 1;
                strcpy(&SaveGameNames[which][0], input);
                if (fp.open(QIODevice::WriteOnly)) {;
                    fp.write((char*)input, 32);
                    DrawLSAction(1);
                    engine->saveTheGame(&fp, LSA_X + 8, LSA_Y + 5);
                    fp.close();
                    ShootSnd();
                    exit = 1;
                }
            } else {
                wlpaint->bar(LSM_X + LSItems.indent + 1, LSM_Y + which * 13 + 1,
                         LSM_W - LSItems.indent - 16, 10, g_bkgdColor);
                PrintLSEntry (which, HIGHLIGHT);
                wlpaint->updateScreen();
                wlaudio->playSound("ESCPRESSEDSND");
                continue;
            }
            fontnumber = 1;
            break;
        }
    }
    while (which >= 0);
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
#ifdef TOUCHSCREEN
    wlinput->setVirtualKeyboard(false);
#endif
    return exit;
}

int CP_Control (int)
{
    int which;

    DrawCtlScreen ();
    wlrender->fadeIn(0, 255, gamepal, 10);
    WaitKeyUp ();

    do {
        which = HandleMenu (&CtlItems, CtlMenu, NULL);
        switch (which) {
        case CTL_MOUSEENABLE:
            DrawCtlScreen ();
            CusItems.curpos = -1;
            ShootSnd ();
            break;
        case CTL_JOYENABLE:
            DrawCtlScreen ();
            CusItems.curpos = -1;
            ShootSnd ();
            break;
        case CTL_MOUSESENS:
        case CTL_CUSTOMIZE:
            DrawCtlScreen ();
            wlrender->fadeIn(0, 255, gamepal, 10);
            WaitKeyUp ();
            break;
        }
    }
    while (which >= 0);
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return 0;
}

void DrawMouseSens (void)
{
    ClearMScreen ();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawWindow (10, 80, 300, 30, g_bkgdColor);
    WindowX = 0;
    WindowW = 320;
    PrintY = 82;
    SETFONTCOLOR (READCOLOR, g_bkgdColor);
    wluser->US_CPrint(STR_MOUSEADJ);
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    PrintX = 14;
    PrintY = 95;
    wluser->US_Print(STR_SLOW);
    PrintX = 269;
    wluser->US_Print(STR_FAST);
    wlpaint->bar(60, 97, 200, 10, TEXTCOLOR);
    DrawOutline (60, 97, 200, 10, 0, HIGHLIGHT);
    DrawOutline (60 + 20 * mouseadjustment, 97, 20, 10, 0, READCOLOR);
    wlpaint->bar(61 + 20 * mouseadjustment, 98, 19, 9, READHCOLOR);
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
}

int MouseSensitivity (int)
{
    ControlInfo_t ci;
    int exit = 0, oldMA;


    oldMA = mouseadjustment;
    DrawMouseSens ();
    do {
        gsleep(5);
        ReadAnyControl (&ci);
        switch (ci.dir) {
        case dir_North:
        case dir_West:
            if (mouseadjustment) {
                mouseadjustment--;
                wlpaint->bar(60, 97, 200, 10, TEXTCOLOR);
                DrawOutline (60, 97, 200, 10, 0, HIGHLIGHT);
                DrawOutline (60 + 20 * mouseadjustment, 97, 20, 10, 0, READCOLOR);
                wlpaint->bar(61 + 20 * mouseadjustment, 98, 19, 9, READHCOLOR);
                wlpaint->updateScreen();
                wlaudio->playSound("MOVEGUN1SND");
                TicDelay(20);
            }
            break;

        case dir_South:
        case dir_East:
            if (mouseadjustment < 9) {
                mouseadjustment++;
                wlpaint->bar(60, 97, 200, 10, TEXTCOLOR);
                DrawOutline (60, 97, 200, 10, 0, HIGHLIGHT);
                DrawOutline (60 + 20 * mouseadjustment, 97, 20, 10, 0, READCOLOR);
                wlpaint->bar(61 + 20 * mouseadjustment, 98, 19, 9, READHCOLOR);
                wlpaint->updateScreen();
                wlaudio->playSound("MOVEGUN1SND");
                TicDelay(20);
            }
            break;
        default:
            break;
        }

        if (ci.button0 || wlinput->isKeyDown(sc_Space) || wlinput->isKeyDown(sc_Enter))
            exit = 1;
        else if (ci.button1 || wlinput->isKeyDown(sc_Escape))
            exit = 2;

    }
    while (!exit);

    if (exit == 2) {
        mouseadjustment = oldMA;
        wlaudio->playSound("ESCPRESSEDSND");
    } else
        wlaudio->playSound("SHOOTSND");

    WaitKeyUp ();
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return 0;
}

void DrawCtlScreen (void)
{
    int i, x, y;

    ClearMScreen ();
    DrawStripes (10);
    wlpaint->drawPic(80, 0, wlcache->find("C_CONTROLPIC"));
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawWindow (CTL_X - 8, CTL_Y - 5, CTL_W, CTL_H, g_bkgdColor);
    WindowX = 0;
    WindowW = 320;
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    CtlMenu[CTL_MOUSESENS].active = mouseenabled;
    DrawMenu (&CtlItems, CtlMenu);

    x = CTL_X + CtlItems.indent - 24;
    y = CTL_Y + 3;
    if (mouseenabled)
        wlpaint->drawPic(x, y, wlcache->find("C_SELECTEDPIC"));
    else
        wlpaint->drawPic(x, y, wlcache->find("C_NOTSELECTEDPIC"));
    y = CTL_Y + 29;
    if (joystickenabled)
        wlpaint->drawPic(x, y, wlcache->find("C_SELECTEDPIC"));
    else
        wlpaint->drawPic(x, y, wlcache->find("C_NOTSELECTEDPIC"));

    if (CtlItems.curpos < 0 || !CtlMenu[CtlItems.curpos].active) {
        for (i = 0; i < CtlItems.amount; i++) {
            if (CtlMenu[i].active) {
                CtlItems.curpos = i;
                break;
            }
        }
    }
    DrawMenuGun (&CtlItems);
    wlpaint->updateScreen();
}

enum
{ FIRE, STRAFE, RUN, OPEN };
char mbarray[4][3] = { "b0", "b1", "b2", "b3" };
int8_t order[4] = { RUN, OPEN, FIRE, STRAFE };


int CustomControls (int)
{
    int which;

    DrawCustomScreen ();
    do {
        which = HandleMenu (&CusItems, &CusMenu[0], FixupCustom);
        switch (which) {
        case 0:
            DefineMouseBtns ();
            DrawCustMouse (1);
            break;
        case 3:
            DefineJoyBtns ();
            DrawCustJoy (0);
            break;
        case 6:
            DefineKeyBtns ();
            DrawCustKeybd (0);
            break;
        case 8:
            DefineKeyMove ();
            DrawCustKeys (0);
        }
    }
    while (which >= 0);
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    return 0;
}

void DefineMouseBtns (void)
{
    CustomCtrls mouseallowed = {{ 0, 1, 1, 1 }};
    EnterCtrlData (2, &mouseallowed, DrawCustMouse, PrintCustMouse, MOUSE);
}

void DefineJoyBtns (void)
{
    CustomCtrls joyallowed = {{ 1, 1, 1, 1 }};
    EnterCtrlData (5, &joyallowed, DrawCustJoy, PrintCustJoy, JOYSTICK);
}

void DefineKeyBtns (void)
{
    CustomCtrls keyallowed = {{ 1, 1, 1, 1 }};
    EnterCtrlData (8, &keyallowed, DrawCustKeybd, PrintCustKeybd, KEYBOARDBTNS);
}

void DefineKeyMove (void)
{
    CustomCtrls keyallowed = {{ 1, 1, 1, 1 }};
    EnterCtrlData (10, &keyallowed, DrawCustKeys, PrintCustKeys, KEYBOARDMOVE);
}

enum
{ FWRD, RIGHT, BKWD, LEFT };
int moveorder[4] = { LEFT, RIGHT, FWRD, BKWD };

void EnterCtrlData (int index, CustomCtrls * cust, void (*DrawRtn) (int), void (*PrintRtn) (int),
                    int type)
{
    int j, exit, tick, redraw, which=0, x=0, picked, lastFlashTime;
    ControlInfo_t ci;

    ShootSnd ();
    PrintY = CST_Y + 13 * index;
    wlinput->clearKeysDown();
    exit = 0;
    redraw = 1;

    for (j = 0; j < 4; j++)
        if (cust->allowed[j]) {
            which = j;
            break;
        }

    do {
        if (redraw) {
            x = CST_START + CST_SPC * which;
            DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);

            DrawRtn (1);
            DrawWindow (x - 2, PrintY, CST_SPC, 11, TEXTCOLOR);
            DrawOutline (x - 2, PrintY, CST_SPC, 11, 0, HIGHLIGHT);
            SETFONTCOLOR (0, TEXTCOLOR);
            PrintRtn (which);
            PrintX = x;
            SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
            wlpaint->updateScreen();
            WaitKeyUp ();
            redraw = 0;
        }

        gsleep(5);
        ReadAnyControl (&ci);

        if (type == MOUSE || type == JOYSTICK)
            if (wlinput->isKeyDown(sc_Enter) || wlinput->isKeyDown(sc_Control) || wlinput->isKeyDown(sc_Alt)) {
                wlinput->clearKeysDown();
                ci.button0 = ci.button1 = false;
            }

        if (((type != KEYBOARDBTNS && type != KEYBOARDMOVE) && (ci.button0 | ci.button1 | ci.button2 | ci.button3)) ||
                ((type == KEYBOARDBTNS || type == KEYBOARDMOVE) && wlinput->lastScan() == sc_Enter)) {
            lastFlashTime = g_getTicks->elapsed()*7/100;
            tick = picked = 0;
            SETFONTCOLOR (0, TEXTCOLOR);

            if (type == KEYBOARDBTNS || type == KEYBOARDMOVE)
                wlinput->clearKeysDown();

            while(1) {
                int result = 0;

                if (g_getTicks->elapsed()*7/100 - lastFlashTime > 10) {
                    switch (tick) {
                    case 0:
                        wlpaint->bar(x, PrintY + 1, CST_SPC - 2, 10, TEXTCOLOR);
                        break;
                    case 1:
                        PrintX = x;
                        wluser->US_Print("?");
                        wlaudio->playSound("HITWALLSND");
                    }
                    tick ^= 1;
                    lastFlashTime = g_getTicks->elapsed()*7/100;
                    wlpaint->updateScreen();
                }
                else
                    gsleep(5);

                switch (type) {
                case MOUSE:
                    break;

                case JOYSTICK:
                    if (ci.button0)
                        result = 1;
                    else if (ci.button1)
                        result = 2;
                    else if (ci.button2)
                        result = 3;
                    else if (ci.button3)
                        result = 4;

                    if (result) {
                        for (int z = 0; z < 4; z++) {
                            if (order[which] == buttonjoy[z]) {
                                buttonjoy[z] = bt_nobutton;
                                break;
                            }
                        }

                        buttonjoy[result - 1] = order[which];
                        picked = 1;
                        wlaudio->playSound("SHOOTDOORSND");
                    }
                    break;

                case KEYBOARDBTNS:
                    if (wlinput->lastScan() && wlinput->lastScan() != sc_Escape) {
                        buttonscan[order[which]] = wlinput->lastScan();
                        picked = 1;
                        ShootSnd ();
                        wlinput->clearKeysDown();
                    }
                    break;

                case KEYBOARDMOVE:
                    if (wlinput->lastScan() && wlinput->lastScan() != sc_Escape) {
                        dirscan[moveorder[which]] = wlinput->lastScan();
                        picked = 1;
                        ShootSnd ();
                        wlinput->clearKeysDown();
                    }
                    break;
                }

                if (wlinput->isKeyDown(sc_Escape) || (type != JOYSTICK && ci.button1)) {
                    picked = 1;
                    wlaudio->playSound("ESCPRESSEDSND");
                }
                if(picked) break;
                ReadAnyControl (&ci);
            }
            SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
            redraw = 1;
            WaitKeyUp ();
            continue;
        }
        if (ci.button1 || wlinput->isKeyDown(sc_Escape))
            exit = 1;

        switch (ci.dir) {
        case dir_West:
            do {
                which--;
                if (which < 0)
                    which = 3;
            }
            while (!cust->allowed[which]);
            redraw = 1;
            wlaudio->playSound("MOVEGUN1SND");
            while (ReadAnyControl (&ci), ci.dir != dir_None) gsleep(5);
            wlinput->clearKeysDown();
            break;

        case dir_East:
            do {
                which++;
                if (which > 3)
                    which = 0;
            } while (!cust->allowed[which]);
            redraw = 1;
            wlaudio->playSound("MOVEGUN1SND");
            while (ReadAnyControl (&ci), ci.dir != dir_None) gsleep(5);
            wlinput->clearKeysDown();
            break;
        case dir_North:
        case dir_South:
            exit = 1;
        default:
            break;
        }
    }
    while (!exit);

    wlaudio->playSound("ESCPRESSEDSND");
    WaitKeyUp ();
    DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);
}

void FixupCustom (int w)
{
    static int lastwhich = -1;
    int y = CST_Y + 26 + w * 13;


    wlpaint->hLine(7, 32, y - 1, g_deActiveColor);
    wlpaint->hLine(7, 32, y + 12, g_bord2Color);
    wlpaint->hLine(7, 32, y - 2, g_bordColor);
    wlpaint->hLine(7, 32, y + 13, g_bordColor);

    switch (w) {
    case 0:
        DrawCustMouse (1);
        break;
    case 3:
        DrawCustJoy (1);
        break;
    case 6:
        DrawCustKeybd (1);
        break;
    case 8:
        DrawCustKeys (1);
    }

    if (lastwhich >= 0)
    {
        y = CST_Y + 26 + lastwhich * 13;
        wlpaint->hLine(7, 32, y - 1, g_deActiveColor);
        wlpaint->hLine(7, 32, y + 12, g_bord2Color);
        wlpaint->hLine(7, 32, y - 2, g_bordColor);
        wlpaint->hLine(7, 32, y + 13, g_bordColor);

        if (lastwhich != w)
            switch (lastwhich) {
            case 0:
                DrawCustMouse (0);
                break;
            case 3:
                DrawCustJoy (0);
                break;
            case 6:
                DrawCustKeybd (0);
                break;
            case 8:
                DrawCustKeys (0);
            }
    }
    lastwhich = w;
}

void DrawCustomScreen (void)
{
    int i;

    ClearMScreen ();
    WindowX = 0;
    WindowW = 320;
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawStripes (10);
    wlpaint->drawPic(80, 0, wlcache->find("C_CUSTOMIZEPIC"));
    SETFONTCOLOR (READCOLOR, g_bkgdColor);
    WindowX = 0;
    WindowW = 320;
    PrintY = CST_Y;
    wluser->US_CPrint("Mouse\n");
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    PrintX = CST_START;
    wluser->US_Print(STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    wluser->US_Print(STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    wluser->US_Print(STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    wluser->US_Print(STR_CSTRAFE "\n");
    DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);
    DrawCustMouse (0);
    wluser->US_Print("\n");
    SETFONTCOLOR (READCOLOR, g_bkgdColor);
    wluser->US_CPrint("Joystick/Gravis GamePad\n");
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    PrintX = CST_START;
    wluser->US_Print(STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    wluser->US_Print(STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    wluser->US_Print(STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    wluser->US_Print(STR_CSTRAFE "\n");
    DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);
    DrawCustJoy (0);
    wluser->US_Print("\n");
    SETFONTCOLOR (READCOLOR, g_bkgdColor);
    wluser->US_CPrint("Keyboard\n");
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    PrintX = CST_START;
    wluser->US_Print(STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    wluser->US_Print(STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    wluser->US_Print(STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    wluser->US_Print(STR_CSTRAFE "\n");
    DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);
    DrawCustKeybd (0);
    wluser->US_Print("\n");
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    PrintX = CST_START;
    wluser->US_Print(STR_LEFT);
    PrintX = CST_START + CST_SPC * 1;
    wluser->US_Print(STR_RIGHT);
    PrintX = CST_START + CST_SPC * 2;
    wluser->US_Print(STR_FRWD);
    PrintX = CST_START + CST_SPC * 3;
    wluser->US_Print(STR_BKWD "\n");
    DrawWindow (5, PrintY - 1, 310, 13, g_bkgdColor);
    DrawCustKeys (0);
    if (CusItems.curpos < 0)
        for (i = 0; i < CusItems.amount; i++)
            if (CusMenu[i].active) {
                CusItems.curpos = i;
                break;
            }
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);
}

void PrintCustMouse (int i)
{
    int j;

    for (j = 0; j < 4; j++) {
        if (order[i] == buttonmouse[j]) {
            PrintX = CST_START + CST_SPC * i;
            wluser->US_Print(mbarray[j]);
            break;
        }
    }
}

void DrawCustMouse (int hilight)
{
    int i, color;

    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, g_bkgdColor);

    if (!mouseenabled) {
        SETFONTCOLOR (g_deActiveColor, g_bkgdColor);
        CusMenu[0].active = 0;
    } else
        CusMenu[0].active = 1;

    PrintY = CST_Y + 13 * 2;
    for (i = 0; i < 4; i++)
        PrintCustMouse (i);
}

void PrintCustJoy (int i)
{
    for (int j = 0; j < 4; j++) {
        if (order[i] == buttonjoy[j]) {
            PrintX = CST_START + CST_SPC * i;
            wluser->US_Print(mbarray[j]);
            break;
        }
    }
}

void DrawCustJoy (int hilight)
{
    int i, color;

    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, g_bkgdColor);

    if (!joystickenabled) {
        SETFONTCOLOR (g_deActiveColor, g_bkgdColor);
        CusMenu[3].active = 0;
    } else
        CusMenu[3].active = 1;

    PrintY = CST_Y + 13 * 5;
    for (i = 0; i < 4; i++)
        PrintCustJoy (i);
}

void PrintCustKeybd (int i)
{
    PrintX = CST_START + CST_SPC * i;
    wluser->US_Print((const char *) IN_GetScanName (buttonscan[order[i]]));
}

void DrawCustKeybd (int hilight)
{
    int i, color;

    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, g_bkgdColor);

    PrintY = CST_Y + 13 * 8;
    for (i = 0; i < 4; i++)
        PrintCustKeybd (i);
}

const char *IN_GetScanName (ScanCode scan)
{
    return (ScanNames[scan]);
}

void PrintCustKeys (int i)
{
    PrintX = CST_START + CST_SPC * i;
    wluser->US_Print((const char *) IN_GetScanName (dirscan[moveorder[i]]));
}

void DrawCustKeys (int hilight)
{
    int i, color;

    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, g_bkgdColor);

    PrintY = CST_Y + 13 * 10;
    for (i = 0; i < 4; i++)
        PrintCustKeys (i);
}

int CP_ChangeView (int)
{
    int exit = 0, oldview, newview;
    ControlInfo_t ci;

    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
    newview = oldview = viewsize;
    DrawChangeView (oldview);
    wlrender->fadeIn(0, 255, gamepal, 10);

    do {
        CheckPause ();
        gsleep(5);
        ReadAnyControl (&ci);
        switch (ci.dir) {
        case dir_South:
        case dir_West:
            newview--;
            if (newview < 4)
                newview = 4;
            if(newview >= 19) DrawChangeView(newview);
            else engine->showViewSize(newview);
            wlpaint->updateScreen();
            wlaudio->playSound("HITWALLSND");
            TicDelay (10);
            break;

        case dir_North:
        case dir_East:
            newview++;
            if (newview >= 21) {
                newview = 21;
                DrawChangeView(newview);
            } else engine->showViewSize(newview);
            wlpaint->updateScreen();
            wlaudio->playSound("HITWALLSND");
            TicDelay (10);
            break;
        default:
            break;
        }
        if (ci.button0 || wlinput->isKeyDown(sc_Enter))
            exit = 1;
        else if (ci.button1 || wlinput->isKeyDown(sc_Escape)) {
            wlaudio->playSound("ESCPRESSEDSND");
            wlrender->fadeOut(0, 255, 43, 0, 0, 10);
            if(g_Image->height() % 200 != 0)
                wlrender->clearScreen(0);
            return 0;
        }
    }
    while (!exit);

    if (oldview != newview) {
        wlaudio->playSound("SHOOTSND");
        Message (STR_THINK "...");
        engine->newViewSize(newview);
    }
    ShootSnd ();
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    if(g_Image->height() % 200 != 0)
        wlrender->clearScreen(0);
    return 0;
}

void DrawChangeView (int view)
{
    int rescaledHeight = g_Image->height() / scaleFactor;
    if(view != 21) wlpaint->bar(0, rescaledHeight - 40, 320, 40, bordercol);
    engine->showViewSize(view);
    PrintY = (g_Image->height() / scaleFactor) - 39;
    WindowX = 0;
    WindowY = 320;                                  // TODO: Check this!
    SETFONTCOLOR (HIGHLIGHT, g_bkgdColor);
    wluser->US_CPrint(STR_SIZE1 "\n");
    wluser->US_CPrint(STR_SIZE2 "\n");
    wluser->US_CPrint(STR_SIZE3);
    wlpaint->updateScreen();
}

int CP_Quit (int)
{
#ifndef TOUCHSCREEN
    if (Confirm (endStrings[(wluser->US_RndT () & 0x7) + (wluser->US_RndT () & 1)])) {
        wlpaint->updateScreen();
        wlaudio->musicOff();
        wlaudio->stopSound();
        wlrender->fadeOut(0, 255, 43, 0, 0, 10);
        Quit (NULL);
    }
    DrawMainMenu ();
#else
    Quit(0);
#endif
    return 0;
}

void IntroScreen (void)
{
#define MAINCOLOR       0x6c
#define EMSCOLOR        0x6c    // 0x4f
#define XMSCOLOR        0x6c    // 0x7f
#define FILLCOLOR       14

    int i;

    for (i = 0; i < 10; i++)
        wlpaint->bar(49, 163 - 8 * i, 6, 5, MAINCOLOR - i);
    for (i = 0; i < 10; i++)
        wlpaint->bar(89, 163 - 8 * i, 6, 5, EMSCOLOR - i);
    for (i = 0; i < 10; i++)
        wlpaint->bar(129, 163 - 8 * i, 6, 5, XMSCOLOR - i);

    // mouse always on
    wlpaint->bar(164, 82, 12, 2, FILLCOLOR);

    if (wlaudio->hasAdLib() && !wlaudio->hasSoundBlaster())
        wlpaint->bar(164, 128, 12, 2, FILLCOLOR);

    if (wlaudio->hasSoundBlaster())
        wlpaint->bar(164, 151, 12, 2, FILLCOLOR);
}

void ClearMScreen (void)
{
    wlpaint->bar(0, 0, 320, 200, g_bordColor);
}

void CacheLump (int lumpstart, int lumpend)
{
    int i;

    for (i = lumpstart; i <= lumpend; i++)
        wlcache->cacheGraphic(i);
}


void UnCacheLump (int lumpstart, int lumpend)
{
    int i;

    for (i = lumpstart; i <= lumpend; i++)
        if (wlcache->graphic(i))
            wlcache->uncacheGraphic(i);
}

void DrawWindow (int x, int y, int w, int h, int wcolor)
{
    wlpaint->bar(x, y, w, h, wcolor);
    DrawOutline (x, y, w, h, g_bord2Color, g_deActiveColor);
}

void DrawOutline (int x, int y, int w, int h, int color1, int color2)
{
    wlpaint->hLine(x, x + w, y, color2);
    wlpaint->vLine(y, y + h, x, color2);
    wlpaint->hLine(x, x + w, y + h, color1);
    wlpaint->vLine(y, y + h, x + w, color1);
}

void SetupControlPanel (void)
{
    wlcache->cacheGraphic(STARTFONT + 1);
    //CacheLump (CONTROLS_LUMP_START, CONTROLS_LUMP_END);
    CacheLump(wlcache->find("C_OPTIONSPIC"), wlcache->find("L_GUYPIC") - 1);
    SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
    fontnumber = 1;
    WindowH = 200;
    if(g_Image->height() % 200 != 0)
        wlrender->clearScreen(0);
    if (!ingame)
        wlcache->loadAllSounds();
    else
        MainMenu[savegame].active = 1;
}

void SetupSaveGames()
{
    //QString fileName = QString("%1%2").arg(SaveName).arg(WLEXT);
    QString fileName = SaveName + g_Extension;

    for(int i = 0; i < 10; i++) {
        QString file = fileName;
        file.replace(QString("?"), QString("%1").arg(i));
        QFile saveFile(QDir::currentPath() + "/" + file);
        if (saveFile.open(QIODevice::ReadOnly)) {
            char tmpBuffer[32];
            SaveGamesAvail[i] = 1;
            saveFile.read((char*)&tmpBuffer, 32);
            saveFile.close();
            strcpy(&SaveGameNames[i][0], tmpBuffer);
        }
    }
}

void CleanupControlPanel (void)
{
    //UnCacheLump (CONTROLS_LUMP_START, CONTROLS_LUMP_END);
    UnCacheLump(wlcache->find("C_OPTIONSPIC"), wlcache->find("L_GUYPIC") - 1);
    fontnumber = 0;
}

int HandleMenu(CP_iteminfo * item_i, CP_itemtype * items, void (*routine) (int w))
{
    char key;
    static int redrawitem = 1, lastitem = -1;
    int i, x, y, basey, exit, which, shape;
    qint32 lastBlinkTime, timer;
    ControlInfo_t ci;

    which = item_i->curpos;
    x = item_i->x & -8;
    basey = item_i->y - 2;
    y = basey + which * 13;

    wlpaint->drawPic(x, y, wlcache->find("C_CURSOR1PIC"));
    SetTextColor (items + which, 1);
    if (redrawitem) {
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        wluser->US_Print((items + which)->string);
    }
    if (routine)
        routine (which);
    wlpaint->updateScreen();

    shape = wlcache->find("C_CURSOR1PIC");
    timer = 8;
    exit = 0;
    lastBlinkTime = g_getTicks->elapsed()*7/100;
    wlinput->clearKeysDown();

    do {
        if ((qint32)g_getTicks->elapsed()*7/100 - lastBlinkTime > timer) {
            lastBlinkTime = g_getTicks->elapsed()*7/100;
            if (shape == (qint32)wlcache->find("C_CURSOR1PIC")) {
                shape = (qint32)wlcache->find("C_CURSOR2PIC");
                timer = 8;
            } else {
                shape = (qint32)wlcache->find("C_CURSOR1PIC");
                timer = 70;
            }
            wlpaint->drawPic(x, y, shape);
            if (routine)
                routine (which);
            wlpaint->updateScreen();
        } else gsleep(5);

        CheckPause ();

        key = wlinput->lastAscii();
        if (key) {
            int ok = 0;

            if (key >= 'a')
                key -= 'a' - 'A';

            for (i = which + 1; i < item_i->amount; i++) {
                if ((items + i)->active && (items + i)->string[0] == key) {
                    EraseGun (item_i, items, x, y, which);
                    which = i;
                    DrawGun (item_i, items, x, &y, which, basey, routine);
                    ok = 1;
                    wlinput->clearKeysDown();
                    break;
                }
            }
            if (!ok) {
                for (i = 0; i < which; i++) {
                    if ((items + i)->active && (items + i)->string[0] == key)
                    {
                        EraseGun (item_i, items, x, y, which);
                        which = i;
                        DrawGun (item_i, items, x, &y, which, basey, routine);
                        wlinput->clearKeysDown();
                        break;
                    }
                }
            }
        }

        ReadAnyControl (&ci);
        switch (ci.dir) {
        case dir_North:
            EraseGun (item_i, items, x, y, which);
            if (which && (items + which - 1)->active) {
                y -= 6;
                DrawHalfStep (x, y);
            }
            do {
                if (!which)
                    which = item_i->amount - 1;
                else
                    which--;
            }
            while (!(items + which)->active);
            DrawGun (item_i, items, x, &y, which, basey, routine);
            TicDelay (20);
            break;

        case dir_South:
            EraseGun (item_i, items, x, y, which);
            if (which != item_i->amount - 1 && (items + which + 1)->active) {
                y += 6;
                DrawHalfStep (x, y);
            }
            do {
                if (which == item_i->amount - 1)
                    which = 0;
                else
                    which++;
            }
            while (!(items + which)->active);
            DrawGun (item_i, items, x, &y, which, basey, routine);
            TicDelay (20);
            break;
        default:
            break;
        }
        if (ci.button0 || wlinput->isKeyDown(sc_Space) || wlinput->isKeyDown(sc_Enter))
            exit = 1;
        if ((ci.button1 && !wlinput->isKeyDown(sc_Alt)) || wlinput->isKeyDown(sc_Escape))
            exit = 2;
    }
    while (!exit);
    wlinput->clearKeysDown();

    if (lastitem != which) {
        wlpaint->bar(x - 1, y, 25, 16, g_bkgdColor);
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        wluser->US_Print((items + which)->string);
        redrawitem = 1;
    } else
        redrawitem = 0;

    if (routine)
        routine (which);
    wlpaint->updateScreen();
    item_i->curpos = which;
    lastitem = which;
    switch (exit) {
    case 1:
        if ((items + which)->routine != NULL) {
            ShootSnd ();
            wlrender->fadeOut(0, 255, 43, 0, 0, 10);
            (items + which)->routine (0);
        }
        return which;
    case 2:
        wlaudio->playSound("ESCPRESSEDSND");
        return -1;
    }
    return 0;
}

void EraseGun(CP_iteminfo * item_i, CP_itemtype * items, int x, int y, int which)
{
    wlpaint->bar(x - 1, y, 25, 16, g_bkgdColor);
    SetTextColor (items + which, 0);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    wluser->US_Print((items + which)->string);
    wlpaint->updateScreen();
}

void DrawHalfStep(int x, int y)
{
    wlpaint->drawPic(x, y, wlcache->find("C_CURSOR1PIC"));
    wlpaint->updateScreen();
    wlaudio->playSound("MOVEGUN1SND");
    gsleep(8 * 100 / 7);
}

void DrawGun(CP_iteminfo * item_i, CP_itemtype * items, int x, int *y, int which, int basey,
             void (*routine) (int w))
{
    wlpaint->bar(x - 1, *y, 25, 16, g_bkgdColor);
    *y = basey + which * 13;
    wlpaint->drawPic(x, *y, wlcache->find("C_CURSOR1PIC"));
    SetTextColor (items + which, 1);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    wluser->US_Print((items + which)->string);

    if (routine)
        routine (which);
    wlpaint->updateScreen();
    wlaudio->playSound("MOVEGUN2SND");
}

void TicDelay (int count)
{
    ControlInfo_t ci;

    qint32 startTime = g_getTicks->elapsed()*7/100;
    do {
        gsleep(5);
        ReadAnyControl (&ci);
    }
    while ((qint32) g_getTicks->elapsed()*7/100 - startTime < count && ci.dir != dir_None);
}

void DrawMenu (CP_iteminfo * item_i, CP_itemtype * items)
{
    int i, which = item_i->curpos;

    WindowX = PrintX = item_i->x + item_i->indent;
    WindowY = PrintY = item_i->y;
    WindowW = 320;
    WindowH = 200;

    for (i = 0; i < item_i->amount; i++) {
        SetTextColor (items + i, which == i);

        PrintY = item_i->y + i * 13;
        if ((items + i)->active)
            wluser->US_Print((items + i)->string);
        else {
            SETFONTCOLOR (g_deActiveColor, g_bkgdColor);
            wluser->US_Print((items + i)->string);
            SETFONTCOLOR (TEXTCOLOR, g_bkgdColor);
        }
        wluser->US_Print("\n");
    }
}

void SetTextColor (CP_itemtype * items, int hlight)
{
    if (hlight) {
        SETFONTCOLOR (color_hlite[items->active], g_bkgdColor);
    } else {
        SETFONTCOLOR (color_norml[items->active], g_bkgdColor);
    }
}

void WaitKeyUp (void)
{
    ControlInfo_t ci;
    while (ReadAnyControl (&ci), ci.button0 |
           ci.button1 |
           ci.button2 | ci.button3 | wlinput->isKeyDown(sc_Space) | wlinput->isKeyDown(sc_Enter) | wlinput->isKeyDown(sc_Escape)) {
        wlinput->waitAndProcessEvents();
    }
}

void ReadAnyControl (ControlInfo_t * ci)
{
    wlinput->readControl(0, ci);
}

int Confirm (const char *string)
{
#ifdef TOUCHSCREEN
    return true;
#endif
    int xit = 0, x, y, tick = 0, lastBlinkTime;
    int whichsnd[2] = { wlcache->findSound("ESCPRESSEDSND"), wlcache->findSound("SHOOTSND") };
    ControlInfo_t ci;

    Message (string);
    wlinput->clearKeysDown();
    WaitKeyUp ();

    x = PrintX;
    y = PrintY;
    lastBlinkTime = g_getTicks->elapsed()*7/100;

    do {
        ReadAnyControl(&ci);

        if (g_getTicks->elapsed()*7/100 - lastBlinkTime >= 10) {
            switch (tick) {
            case 0:
                wlpaint->bar(x, y, 8, 13, TEXTCOLOR);
                break;
            case 1:
                PrintX = x;
                PrintY = y;
                wluser->US_Print("_");
            }
            wlpaint->updateScreen();
            tick ^= 1;
            lastBlinkTime = g_getTicks->elapsed()*7/100;
        } else
            gsleep(5);
    }
    while (!wlinput->isKeyDown(sc_Y) && !wlinput->isKeyDown(sc_N) && !wlinput->isKeyDown(sc_Escape) && !ci.button0 && !ci.button1);

    if (wlinput->isKeyDown(sc_Y) || ci.button0) {
        xit = 1;
        ShootSnd ();
    }
    wlinput->clearKeysDown();
    WaitKeyUp ();
    wlaudio->playSound(g_sound[whichsnd[xit]]);

    return xit;
}

void Message(const char *string)
{
    int h = 0, w = 0, mw = 0, i, len = (int) strlen(string);
    fontstruct *font;

    wlcache->cacheGraphic(STARTFONT + 1);
    fontnumber = 1;
    font = (fontstruct *) wlcache->graphic(STARTFONT + fontnumber);
    h = font->height;
    for (i = 0; i < len; i++) {
        if (string[i] == '\n') {
            if (w > mw)
                mw = w;
            w = 0;
            h += font->height;
        } else
            w += font->width[(int)string[i]];
    }

    if (w + 10 > mw)
        mw = w + 10;

    PrintY = (WindowH / 2) - h / 2;
    PrintX = WindowX = 160 - mw / 2;
    DrawWindow (WindowX - 5, PrintY - 5, mw + 10, h + 10, TEXTCOLOR);
    DrawOutline (WindowX - 5, PrintY - 5, mw + 10, h + 10, 0, HIGHLIGHT);
    SETFONTCOLOR (0, TEXTCOLOR);
    wluser->US_Print(string);
    wlpaint->updateScreen();
}

static int lastmusic;

int StartCPMusic (int song)
{
    int lastoffs;

    lastmusic = song;
    lastoffs = wlaudio->musicOff();
    wlcache->uncacheAudio(wlcache->findSound("LASTSOUND")*3 + lastmusic); // STARTMUSIC

    wlaudio->startMusic(wlcache->findSound("LASTSOUND")*3 + song); //STARTMUSIC
    return lastoffs;
}

void FreeMusic (void)
{
    wlcache->uncacheAudio(wlcache->findSound("LASTSOUND")*3 + lastmusic); //STARTMUSIC
}

void CheckPause (void)
{
    if (wlinput->isPaused()) {
        switch (SoundStatus) {
        case 0:
            wlaudio->musicOn();
            break;
        case 1:
            wlaudio->musicOff();
            break;
        }

        SoundStatus ^= 1;
        gsleep(24);
        wlinput->clearKeysDown();
        wlinput->setPaused(false);
    }
}

void DrawMenuGun (CP_iteminfo * iteminfo)
{
    int x, y;

    x = iteminfo->x;
    y = iteminfo->y + iteminfo->curpos * 13 - 2;
    wlpaint->drawPic(x, y, wlcache->find("C_CURSOR1PIC"));
}

void DrawStripes(int y)
{
    wlpaint->bar(0, y, 320, 24, 0);
    wlpaint->hLine(0, 319, y + 22, g_stripColor);
}

void ShootSnd(void)
{
    wlaudio->playSound("SHOOTSND");
}

void CheckForEpisodes (void)
{
    QFile contentFile("vswap.wl6");
    QFile contentFileUpper("VSWAP.WL6");

    if (contentFile.exists() || contentFileUpper.exists()) {
        // Full version Wolf3d
        g_Extension = "wl6";

        if (contentFileUpper.exists())
            g_UpperCase = true;

        g_graphic = &wl6Graphic[0];
        g_sound = &wl6Sounds[0];
        g_music = &wl6Music[0];

        NewEmenu[2].active = NewEmenu[4].active = NewEmenu[6].active = NewEmenu[8].active = NewEmenu[10].active = 1;
        EpisodeSelect[1] = EpisodeSelect[2] = EpisodeSelect[3] = EpisodeSelect[4] = EpisodeSelect[5] = 1;
        return;
    }

    contentFile.setFileName("vswap.wl1");
    contentFileUpper.setFileName("VSWAP.WL1");
    if (contentFile.exists() || contentFileUpper.exists()) {
        // Demo version Wolf3d
        g_Extension = "wl1";
        if (contentFileUpper.exists())
            g_UpperCase = true;
        g_graphic = &wl1Graphic[0];
        g_sound = &wl6Sounds[0];
        g_music = &wl6Music[0];
        return;
    }

    contentFile.setFileName("vswap.wl3");
    contentFileUpper.setFileName("VSWAP.WL3");
    if (contentFile.exists() || contentFileUpper.exists()) {
        // Full version Wolf3d - not tested?
        g_Extension = "wl3";
        if (contentFileUpper.exists())
            g_UpperCase = true;
        g_graphic = &wl1Graphic[0];
        g_sound = &wl6Sounds[0];
        g_music = &wl6Music[0];
        //wlcache->setMissingEpisodes(3);
        NewEmenu[2].active = NewEmenu[4].active = EpisodeSelect[1] = EpisodeSelect[2] = 1;
        return;
    }

    contentFile.setFileName("vswap.sod");
    contentFileUpper.setFileName("VSWAP.sod");
    if (contentFile.exists() || contentFileUpper.exists()) {
        // Full version Spear of Destiny
        g_Extension = "sod";
        if (contentFileUpper.exists())
            g_UpperCase = true;
        g_graphic = &sodGraphic[0];
        g_sound = &sodSounds[0];
        g_music = &sodMusic[0];

        g_bkgdColor     = 0x9d;
        g_bord2Color    = 0x93;
        g_deActiveColor = 0x9b;
        g_bordColor     = 0x99;
        g_stripColor    = 0x9c;

        return;
    }

    Quit ("NO WOLFENSTEIN 3-D DATA FILES to be found!");
}
