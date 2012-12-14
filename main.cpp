#include <stdint.h>

#include "main.h"

#include "wl_def.h"

#include "wlpage.h"
#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wltext.h"
#include "wlstate.h"
#include "wlitem.h"
#include "wlrender.h"
#include "wlplay.h"
#include "wlpaint.h"
#include "wluser.h"

#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QDebug>

WLPage*    wlpage   = 0;
WLCache*   wlcache  = 0;
WLInput*   wlinput  = 0;
WLAudio*   wlaudio  = 0;
WLText*    wltext   = 0;
WLState*   wlstate  = 0;
WLItem*    wlitem   = 0;
WLRender*  wlrender = 0;
WLPlay*    wlplay   = 0;
WLPaint*   wlpaint  = 0;
WLUser*    wluser   = 0;
GameEngine* engine  = 0;

QImage*    g_Image      = 0;
QTime*     g_getTicks   = 0;
QSize      g_screenSize;
QString    g_Extension  = "wl1";
bool       g_UpperCase  = false;
QString*   g_graphic    = 0;
QString*   g_sound      = 0;
QString*   g_music      = 0;

extern qint32 g_bkgdColor;
extern quint8 signon[];

char    str[80];
qint32  dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
                       5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

qint32   focallength;
quint32  screenofs;
qint32   viewscreenx, viewscreeny;
qint32   viewwidth;
qint32   viewheight;
qint16   centerx;
qint32   shootdelta;
qint32   scale;
qint32   heightnumerator;

void    Quit (const char *error,...);

qint8 startgame;
bool loadedgame;
int     mouseadjustment;

char    configname[13]="config.";

qint8 param_debugmode = false;
qint8 param_nowait = false;
int     param_difficulty = 1;
int     param_tedlevel = -1;
int     param_joystickindex = 0;

int     param_joystickhat = -1;
int     param_samplerate = 44100;
int     param_audiobuffer = 2048 / (44100 / param_samplerate);

int     param_mission = 1;
qint8 param_goodtimes = false;
qint8 param_ignorenumchunks = false;

const float radtoint = (float)(FINEANGLES/2/PI);

extern statetype s_grdstand;
extern statetype s_player;

typedef struct {
    QString name;
    qint32   a;
    qint32   b;
} wolfdigimap_t;

static wolfdigimap_t wl6Wolfdigimap[48] =
{
    { "HALTSND",              0,  -1 },
    { "DOGBARKSND",           1,  -1 },
    { "CLOSEDOORSND",         2,  -1 },
    { "OPENDOORSND",          3,  -1 },
    { "ATKMACHINEGUNSND",     4,   0 },
    { "ATKPISTOLSND",         5,   0 },
    { "ATKGATLINGSND",        6,   0 },
    { "SCHUTZADSND",          7,  -1 },
    { "GUTENTAGSND",          8,  -1 },
    { "MUTTISND",             9,  -1 },
    { "BOSSFIRESND",          10,  1 },
    { "SSFIRESND",            11, -1 },
    { "DEATHSCREAM1SND",      12, -1 },
    { "DEATHSCREAM2SND",      13, -1 },
    { "DEATHSCREAM3SND",      13, -1 },
    { "TAKEDAMAGESND",        14, -1 },
    { "PUSHWALLSND",          15, -1 },
    { "DOGDEATHSND",          16, -1 },
    { "AHHHGSND",             17, -1 },
    { "DIESND",               18, -1 },
    { "EVASND",               19, -1 },
    { "LEBENSND",             20, -1 },
    { "NAZIFIRESND",          21, -1 },
    { "SLURPIESND",           22, -1 },
    { "TOT_HUNDSND",          23, -1 },
    { "MEINGOTTSND",          24, -1 },
    { "SCHABBSHASND",         25, -1 },
    { "HITLERHASND",          26, -1 },
    { "SPIONSND",             27, -1 },
    { "NEINSOVASSND",         28, -1 },
    { "DOGATTACKSND",         29, -1 },
    { "LEVELDONESND",         30, -1 },
    { "MECHSTEPSND",          31, -1 },
    { "YEAHSND",              32, -1 },
    { "SCHEISTSND",           33, -1 },
    { "DEATHSCREAM4SND",      34, -1 },
    { "DEATHSCREAM5SND",      35, -1 },
    { "DONNERSND",            36, -1 },
    { "EINESND",              37, -1 },
    { "ERLAUBENSND",          38, -1 },
    { "DEATHSCREAM6SND",      39, -1 },
    { "DEATHSCREAM7SND",      40, -1 },
    { "DEATHSCREAM8SND",      41, -1 },
    { "DEATHSCREAM9SND",      42, -1 },
    { "KEINSND",              43, -1 },
    { "MEINSND",              44, -1 },
    { "ROSESND",              45, -1 },
    { "LASTSOUND",            46,  0 }
};

static wolfdigimap_t sodWolfdigimap[42] =
{
    { "HALTSND",              0,  -1 },
    { "DOGBARKSND",           1,  -1 },
    { "CLOSEDOORSND",         2,  -1 },
    { "OPENDOORSND",          3,  -1 },
    { "ATKMACHINEGUNSND",     4,   0 },
    { "ATKPISTOLSND",         5,   0 },
    { "ATKGATLINGSND",        6,   0 },
    { "SCHUTZADSND",          7,  -1 },
    { "BOSFIRESND",           8,  -1 },
    { "MUTTISND",             9,  -1 },
    { "DEATHSCREAM1SND",      10,  1 },
    { "DEATHSCREAM2SND",      11, -1 },
    { "TAKEDAMAGESND",        12, -1 },
    { "PUSHWALLSND",          13, -1 },
    { "DOGDEATHSND",          14, -1 },
    { "AHHHGSND",             15, -1 },
    { "LEBENSND",             16, -1 },
    { "NAZIFIRESND",          17, -1 },
    { "SLURPIESND",           18, -1 },
    { "SPIONSND",             19, -1 },
    { "NEINSOVASND",          20, -1 },
    { "DOGATTACKSND",         21, -1 },
    { "LEVELDONESND",         22, -1 },
    { "DEATHSCREAM4SND",      23, -1 },
    { "DEATHSCREAM3SND",      23, -1 },
    { "DEATHSCREAM5SND",      24, -1 },
    { "DEATHSCREAM6SND",      25, -1 },
    { "DEATHSCREAM7SND",      26, -1 },
    { "DEATHSCREAM8SND",      27, -1 },
    { "DEATHSCREAM9SND",      28, -1 },
    { "TRANSSIGHTSND",        29, -1 },
    { "TRANSDEATHSND",        30, -1 },
    { "WILHELMSIGHTSND",      31, -1 },
    { "WILHELMDEATHSND",      32, -1 },
    { "UBERDEATHSND",         33, -1 },
    { "KNIGHTSIGHTSND",       34, -1 },
    { "KNIGHTDEATHSND",       35, -1 },
    { "ANGELSIGHTSND",        36, -1 },
    { "ANGELDEATHSND",        37, -1 },
    { "GETGATLINGSND",        38, -1 },
    { "GETSPEARSND",          39, -1 },
    { "LASTSOUND",            40,  0 }
};

CP_iteminfo MusicItems = {CTL_X, CTL_Y, 6, 0, 32};
CP_itemtype MusicMenu[]=
{
    {1,"Get Them!",0},
    {1,"Searching",0},
    {1,"P.O.W.",0},
    {1,"Suspense",0},
    {1,"War March",0},
    {1,"Around The Corner!",0},

    {1,"Nazi Anthem",0},
    {1,"Lurking...",0},
    {1,"Going After Hitler",0},
    {1,"Pounding Headache",0},
    {1,"Into the Dungeons",0},
    {1,"Ultimate Conquest",0},

    {1,"Kill the S.O.B.",0},
    {1,"The Nazi Rap",0},
    {1,"Twelfth Hour",0},
    {1,"Zero Hour",0},
    {1,"Ultimate Conquest",0},
    {1,"Wolfpack",0}
};

void gsleep(int ms) {
    SleeperThread::msleep(ms);
}

void Quit (const char *errorStr, ...)
{
    char error[256];
    if(errorStr != NULL) {
        va_list vlist;
        va_start(vlist, errorStr);
        vsprintf(error, errorStr, vlist);
        va_end(vlist);
    } else error[0] = 0;

    if (!pictable) {
        delete engine;
        if(*error) {
            puts(error);
            gsleep(800);
        }
        exit(1);
    }
    if(!*error) engine->writeConfig();
    delete engine;
    if(*error) {
        puts(error);
        gsleep(1600);
        exit(1);
    }
    exit(0);
}

GameEngine::GameEngine(QWidget *parent) :
    QMainWindow(parent)
{
    wlinput = new WLInput(this);
    m_display = new WLDisplay(this, wlinput);
    setCentralWidget(m_display);

    setWindowTitle(tr("Wolfenstien 3D"));
#ifdef TOUCHSCREEN
    showMaximized();
    m_accelerometer = new QAccelerometer(parent);
    connect(m_accelerometer,SIGNAL(readingChanged()),this,SLOT(readingChanged()));
#else
    resize(640, 400);
    show();
#endif
    QTimer::singleShot(500, this, SLOT(start()));
}

GameEngine::~GameEngine()
{
    delete wlaudio;
    delete wlpage;
    delete wlinput;
    delete wlcache;
    delete m_display;

#ifdef TOUCHSCREEN
    m_accelerometer->stop();
    m_accelerometer->disconnect(this);
    delete m_accelerometer;
#endif
}

#ifdef TOUCHSCREEN
void GameEngine::readingChanged()
{
    wlinput->x += m_accelerometer->reading()->x();
    wlinput->y += m_accelerometer->reading()->y();
    wlinput->z += m_accelerometer->reading()->z();
    if (wlinput->x < -5) wlinput->x = -5;
    if (wlinput->x > +5) wlinput->x = 5;
    if (wlinput->y < -5) wlinput->y = -5;
    if (wlinput->y > +5) wlinput->y = 5;
    if (wlinput->z < -5) wlinput->z = -5;
    if (wlinput->z > +5) wlinput->z = 5;
}
#endif

void GameEngine::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    Quit(0);
}

void GameEngine::start()
{
    CheckForEpisodes();
    if (g_Extension.toLower().contains("wl1"))
        wlcache = new WLCache(161, 234); // wl1 NUMCHUNKS = 161 , NUMSNDCHUNKS = 234
    else if (g_Extension.toLower().contains("wl6"))
        wlcache = new WLCache(149, 288); // wl6 NUMCHUNKS = 149 , NUMSNDCHUNKS = 288
    else if (g_Extension.toLower().contains("sod"))
        wlcache = new WLCache(169, 268);
    wltext  = new WLText();
    wlstate = new WLState();
    wlitem  = new WLItem();

    initGame();
    QTimer::singleShot(2000, this, SLOT(demoLoop()));
}

void GameEngine::readConfig()
{
    SDMode  sd;
    SMMode  sm;
    SDSMode sds;

    bool hasConfig = false;

    QString fileName = configname + g_Extension;

    QFile configFile(fileName);
    if (configFile.open(QIODevice::ReadOnly)) {
        quint16 tmpBuffer;
        configFile.read((char*)&tmpBuffer,sizeof(tmpBuffer));
        if(tmpBuffer != 0xfefa) {
            configFile.close();
            hasConfig = true;
        }
    }
    if (hasConfig) {
        configFile.read((char*)Scores, sizeof(HighScore)*MaxScores);
        configFile.read((char*)&sd, sizeof(sd));
        configFile.read((char*)&sm, sizeof(sm));
        configFile.read((char*)&sds, sizeof(sds));
        configFile.read((char*)dirscan, sizeof(dirscan));
        configFile.read((char*)buttonscan, sizeof(buttonscan));
        configFile.read((char*)buttonmouse, sizeof(buttonmouse));
        configFile.read((char*)buttonjoy, sizeof(buttonjoy));
        configFile.read((char*)&viewsize, sizeof(viewsize));
        configFile.read((char*)&mouseadjustment, sizeof(mouseadjustment));
        configFile.close();

        if ((sd == sdm_AdLib || sm == smm_AdLib) && !wlaudio->hasAdLib() && !wlaudio->hasSoundBlaster()) {
            sd = sdm_PC;
            sm = smm_Off;
        }
        if ((sds == sds_SoundBlaster && !wlaudio->hasSoundBlaster())) sds = sds_Off;
        if(viewsize < 4) viewsize = 4;
        else if(viewsize > 21) viewsize = 21;
        MainMenu[6].active = 1;
        MainItems.curpos = 0;

    } else {
        if (wlaudio->hasSoundBlaster() || wlaudio->hasAdLib()) {
            sd = sdm_AdLib;
            sm = smm_AdLib;
        } else {
            sd = sdm_PC;
            sm = smm_Off;
        }
        if (wlaudio->hasSoundBlaster()) sds = sds_SoundBlaster;
        else sds = sds_Off;
        viewsize = 19;
        mouseadjustment=5;
    }
    wlaudio->setMusicMode(sm);
    wlaudio->setSoundMode(sd);
    wlaudio->setDigiDevice(sds);
}

void GameEngine::writeConfig()
{
    QString fileName = configname + g_Extension;

    QFile configFile(fileName);
    if (configFile.open(QIODevice::WriteOnly)) {
        quint16 tmpBuffer = 0xfefa;
        configFile.write((char*)&tmpBuffer, sizeof(tmpBuffer));
        configFile.write((char*)Scores, sizeof(HighScore)*MaxScores);
        configFile.write((char*)wlaudio->soundMode(), sizeof(wlaudio->soundMode()));
        configFile.write((char*)wlaudio->musicMode(), sizeof(wlaudio->musicMode()));
        configFile.write((char*)wlaudio->digiMode(), sizeof(wlaudio->digiMode()));
        configFile.write((char*)dirscan, sizeof(dirscan));
        configFile.write((char*)buttonscan, sizeof(buttonscan));
        configFile.write((char*)buttonmouse, sizeof(buttonmouse));
        configFile.write((char*)buttonjoy, sizeof(buttonjoy));
        configFile.write((char*)&viewsize, sizeof(viewsize));
        configFile.write((char*)&mouseadjustment, sizeof(mouseadjustment));
        configFile.close();
    }
}

void GameEngine::newGame(qint32 difficulty, qint32 episode)
{
    memset (&gamestate,0,sizeof(gamestate));
    gamestate.difficulty = difficulty;
    gamestate.weapon = gamestate.bestweapon
            = gamestate.chosenweapon = wp_pistol;
    gamestate.health = 100;
    gamestate.ammo = STARTAMMO;
    gamestate.lives = 3;
    gamestate.nextextra = EXTRAPOINTS;
    gamestate.episode=episode;
    startgame = true;
}

void GameEngine::diskFlopAnim(qint32 x, qint32 y)
{
    static int8_t which=0;
    if (!x && !y)
        return;
    wlpaint->drawPic(x, y, wlcache->find("C_DISKLOADING1PIC") + which);
    wlpaint->updateScreen();
    which^=1;
}

qint32 GameEngine::doChecksum(quint8 *source, quint32 size,qint32 checksum)
{
    for (quint32 i = 0; i < size - 1; i++)
        checksum += source[i]^source[i + 1];

    return checksum;
}

qint8 GameEngine::saveTheGame(QFile *fp, qint32 x, qint32 y)
{
    qint32    checksum = 0;
    objtype*  ob;
    objtype   nullobj;
    statObj_t nullstat;

    diskFlopAnim(x,y);
    fp->write((char*)&gamestate, sizeof(gamestate));
    checksum = doChecksum((quint8*)&gamestate, sizeof(gamestate), checksum);

    diskFlopAnim(x,y);
    fp->write((char*)&LevelRatios[0], sizeof(LRstruct)*LRpack);
    checksum = doChecksum((quint8*)&LevelRatios[0], sizeof(LRstruct)*LRpack, checksum);

    diskFlopAnim(x,y);
    fp->write((char*)tilemap, sizeof(tilemap));
    checksum = doChecksum((quint8*)tilemap,sizeof(tilemap),checksum);
    diskFlopAnim(x,y);

    for(qint32 i = 0; i < MAPSIZE; i++) {
        for(qint32 j = 0; j < MAPSIZE; j++)
        {
            quint16 actnum;
            objtype *objptr=actorat[i][j];
            if(ISPOINTER(objptr))
                actnum=0x8000 | (quint16)(objptr-objlist);
            else
                actnum=(quint16)(uintptr_t)objptr;
            fp->write((char*)&actnum, sizeof(actnum));
            checksum = doChecksum((quint8*)&actnum,sizeof(actnum),checksum);
        }
    }
    fp->write((char*)wlitem->areaConnect(), sizeof(wlitem->areaConnect()));
    fp->write((char*)wlitem->areaByPlayer(0), sizeof(wlitem->areaByPlayer(0)));
    ob = player;
    diskFlopAnim(x,y);
    memcpy(&nullobj,ob,sizeof(nullobj));
    nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_player);
    fp->write((char*)&nullobj, sizeof(nullobj));
    ob = ob->next;

    diskFlopAnim(x,y);
    for (; ob ; ob=ob->next) {
        memcpy(&nullobj,ob,sizeof(nullobj));
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_grdstand);
        fp->write((char*)&nullobj, sizeof(nullobj));
    }
    nullobj.active = ac_badobject;          // end of file marker
    diskFlopAnim(x,y);
    fp->write((char*)&nullobj, sizeof(nullobj));
    diskFlopAnim(x,y);
    quint16 laststatobjnum=(quint16) (laststatobj-statobjlist);
    fp->write((char*)&laststatobjnum, sizeof(laststatobjnum));

    checksum = doChecksum((quint8*)&laststatobjnum,sizeof(laststatobjnum),checksum);
    diskFlopAnim(x,y);
    for(qint32 i = 0; i < MAXSTATS; i++) {
        memcpy(&nullstat,statobjlist+i,sizeof(nullstat));
        nullstat.visSpot=(quint8*) ((uintptr_t) nullstat.visSpot-(uintptr_t)spotvis);
        fp->write((char*)&nullstat, sizeof(nullstat));
        checksum = doChecksum((quint8*)&nullstat,sizeof(nullstat),checksum);
    }

    diskFlopAnim(x,y);
    fp->write((char*)wlitem->doorPosition(0), sizeof(wlitem->doorPosition(0)));

    checksum = doChecksum((quint8*)wlitem->doorPosition(0),sizeof(wlitem->doorPosition(0)),checksum);
    diskFlopAnim(x,y);
    fp->write((char*)wlitem->doorObjList(0), sizeof(wlitem->doorObjList(0)));

    checksum = doChecksum((quint8*)wlitem->doorObjList(0),sizeof(wlitem->doorObjList(0)),checksum);
    diskFlopAnim(x,y);
    fp->write((char*)wlitem->pWallState(), sizeof(wlitem->pWallState()));

    checksum = doChecksum((quint8*)wlitem->pWallState(),sizeof(wlitem->pWallState()),checksum);
    fp->write((char*)wlitem->pWallTile(), sizeof(wlitem->pWallTile()));

    checksum = doChecksum((quint8*)wlitem->pWallTile(),sizeof(wlitem->pWallTile()),checksum);
    fp->write((char*)wlitem->pWallX(), sizeof(wlitem->pWallX()));

    checksum = doChecksum((quint8*)wlitem->pWallX(),sizeof(wlitem->pWallX()),checksum);
    fp->write((char*)wlitem->pWallY(), sizeof(wlitem->pWallY()));

    checksum = doChecksum((quint8*)wlitem->pWallY(),sizeof(wlitem->pWallY()),checksum);
    fp->write((char*)wlitem->pWallDir(), sizeof(wlitem->pWallDir()));

    checksum = doChecksum((quint8*)wlitem->pWallDir(),sizeof(wlitem->pWallDir()),checksum);
    fp->write((char*)wlitem->pWallPos(), sizeof(wlitem->pWallPos()));

    checksum = doChecksum((quint8*)wlitem->pWallPos(),sizeof(wlitem->pWallPos()),checksum);
    fp->write((char*)&checksum, sizeof(checksum));
    fp->write((char*)&lastgamemusicoffset, sizeof(lastgamemusicoffset));

    for (int x = 0; x < mapwidth; x++)
        for (int y = 0; y < mapheight; y++) {
            bool on = wlcache->fog(x,y);
            fp->write((char*)&on, 1);
        }
    return(true);
}

qint8 GameEngine::loadTheGame(QFile* fp, qint32 x, qint32 y)
{
    qint32    checksum,oldchecksum;
    objtype   nullobj;
    statObj_t nullstat;

    checksum = 0;

    diskFlopAnim(x,y);
    fp->read((char*)&gamestate, sizeof(gamestate));
    checksum = doChecksum((quint8*)&gamestate,sizeof(gamestate),checksum);
    diskFlopAnim(x,y);
    fp->read((char*)&LevelRatios[0], sizeof(LRstruct)*LRpack);
    checksum = doChecksum((quint8*)&LevelRatios[0],sizeof(LRstruct)*LRpack,checksum);
    diskFlopAnim(x,y);
    SetupGameLevel();
    diskFlopAnim(x,y);
    fp->read((char*)tilemap, sizeof(tilemap));
    checksum = doChecksum((quint8*)tilemap,sizeof(tilemap),checksum);
    diskFlopAnim(x,y);

    quint16 actnum=0, i;
    for(i=0;i<MAPSIZE;i++) {
        for(int j=0;j<MAPSIZE;j++) {
            fp->read((char*)&actnum, sizeof(quint16));
            checksum = doChecksum((quint8*) &actnum,sizeof(quint16),checksum);
            if(actnum&0x8000)
                actorat[i][j]=objlist+(actnum&0x7fff);
            else
                actorat[i][j]=(objtype *)(uintptr_t) actnum;
        }
    }
    fp->read((char*)wlitem->areaConnect(), sizeof(wlitem->areaConnect()));
    fp->read((char*)wlitem->areaByPlayer(0), sizeof(wlitem->areaByPlayer(0)));
    wlplay->initActorList();
    diskFlopAnim(x,y);
    fp->read((char*)player, sizeof(nullobj));
    player->state=(statetype *) ((uintptr_t)player->state+(uintptr_t)&s_player);
    while (1) {
        diskFlopAnim(x,y);
        fp->read((char*)&nullobj, sizeof(nullobj));
        if (nullobj.active == ac_badobject)
            break;
        wlplay->getNewActor();
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state+(uintptr_t)&s_grdstand);
        memcpy (newobj,&nullobj,sizeof(nullobj)-8);
    }
    diskFlopAnim(x,y);
    quint16 laststatobjnum;
    fp->read((char*)&laststatobjnum, sizeof(laststatobjnum));
    laststatobj=statobjlist+laststatobjnum;
    checksum = doChecksum((quint8*)&laststatobjnum,sizeof(laststatobjnum),checksum);
    diskFlopAnim(x,y);
    for(i=0;i<MAXSTATS;i++) {
        fp->read((char*)&nullstat, sizeof(nullstat));
        checksum = doChecksum((quint8*)&nullstat,sizeof(nullstat),checksum);
        nullstat.visSpot=(quint8*) ((uintptr_t)nullstat.visSpot+(uintptr_t)spotvis);
        memcpy(statobjlist+i,&nullstat,sizeof(nullstat));
    }
    diskFlopAnim(x,y);
    fp->read((char*)wlitem->doorPosition(0), sizeof(wlitem->doorPosition(0)));
    checksum = doChecksum((quint8*)wlitem->doorPosition(0),sizeof(wlitem->doorPosition(0)),checksum);
    diskFlopAnim(x,y);
    fp->read((char*)wlitem->doorObjList(0), sizeof(wlitem->doorObjList(0)));
    checksum = doChecksum((quint8*)wlitem->doorObjList(0),sizeof(wlitem->doorObjList(0)),checksum);
    diskFlopAnim(x,y);
    fp->read((char*)wlitem->pWallState(), sizeof(wlitem->pWallState()));
    checksum = doChecksum((quint8*)wlitem->pWallState(),sizeof(wlitem->pWallState()),checksum);
    fp->read((char*)wlitem->pWallTile(), sizeof(wlitem->pWallTile()));
    checksum = doChecksum((quint8*)wlitem->pWallTile(),sizeof(wlitem->pWallTile()),checksum);
    fp->read((char*)wlitem->pWallX(), sizeof(wlitem->pWallX()));
    checksum = doChecksum((quint8*)wlitem->pWallX(),sizeof(wlitem->pWallX()),checksum);
    fp->read((char*)wlitem->pWallY(), sizeof(wlitem->pWallY()));
    checksum = doChecksum((quint8*)wlitem->pWallY(),sizeof(wlitem->pWallY()),checksum);
    fp->read((char*)wlitem->pWallDir(), sizeof(wlitem->pWallDir()));
    checksum = doChecksum((quint8*)wlitem->pWallDir(),sizeof(wlitem->pWallDir()),checksum);
    fp->read((char*)wlitem->pWallPos(), sizeof(wlitem->pWallPos()));
    checksum = doChecksum((quint8*)wlitem->pWallPos(),sizeof(wlitem->pWallPos()),checksum);
    if (gamestate.secretcount) {
        quint16 *map, *obj; quint16 tile, sprite;
        map = (quint16*)wlcache->map(0);
        obj = (quint16*)wlcache->map(1);
        for (y=0;y<mapheight;y++)
            for (x=0;x<mapwidth;x++) {
                tile = *map++; sprite = *obj++;
                if (sprite == PUSHABLETILE && !tilemap[x][y]
                        && (tile < AREATILE || tile >= (AREATILE+numMaps))) {
                    if (*map >= AREATILE)
                        tile = *map;
                    if (*(map-1-mapwidth) >= AREATILE)
                        tile = *(map-1-mapwidth);
                    if (*(map-1+mapwidth) >= AREATILE)
                        tile = *(map-1+mapwidth);
                    if ( *(map-2) >= AREATILE)
                        tile = *(map-2);

                    *(map-1) = tile; *(obj-1) = 0;
                }
            }
    }
    Thrust(0,0);

    fp->read((char*)&oldchecksum, sizeof(oldchecksum));
    fp->read((char*)&lastgamemusicoffset, sizeof(lastgamemusicoffset));
    if(lastgamemusicoffset<0) lastgamemusicoffset=0;

    for (int xx = 0; xx < mapwidth; xx++)
        for (int yy = 0; yy < mapheight; yy++) {
            bool on;
            fp->read((char*)&on, 1);
            wlcache->setFog(on, xx, yy);
        }

    if (oldchecksum != checksum) {
        Message(STR_SAVECHT1"\n"
                STR_SAVECHT2"\n"
                STR_SAVECHT3"\n"
                STR_SAVECHT4);
        wlinput->clearKeysDown();
        wlinput->ack();
        gamestate.oldscore = gamestate.score = 0;
        gamestate.lives = 1;
        gamestate.weapon =
                gamestate.chosenweapon =
                gamestate.bestweapon = wp_pistol;
        gamestate.ammo = 8;
    }
    return true;
}

void GameEngine::buildTables()
{
    int i;
    for(i=0;i<FINEANGLES/8;i++) {
        double tang=tan((i+0.5)/radtoint);
        finetangent[i]=(qint32)(tang*GLOBAL1);
        finetangent[FINEANGLES/4-1-i]=(qint32)((1/tang)*GLOBAL1);
    }
    float angle=0;
    float anglestep=(float)(PI/2/ANGLEQUAD);
    for(i=0; i<ANGLEQUAD; i++) {
        qint32 value=(qint32)(GLOBAL1*sin(angle));
        sintable[i]=sintable[i+ANGLES]=sintable[ANGLES/2-i]=value;
        sintable[ANGLES-i]=sintable[ANGLES/2+i]=-value;
        angle+=anglestep;
    }
    sintable[ANGLEQUAD] = 65536;
    sintable[3*ANGLEQUAD] = -65536;
}

void GameEngine::calcProjection(qint32 focal)
{
    int     i;
    int    intang;
    float   angle;
    double  tang;
    int     halfview;
    double  facedist;

    focallength = focal;
    facedist = focal+MINDIST;
    halfview = viewwidth/2;

    qint32 viewGlobal = 0x10000;

    scale = (qint32) (halfview*facedist/(viewGlobal/2));
    heightnumerator = (TILEGLOBAL*scale)>>6;
    for (i=0;i<halfview;i++) {
        tang = (qint32)i*viewGlobal/viewwidth/facedist;
        angle = (float) atan(tang);
        intang = (int) (angle*radtoint);
        pixelangle[halfview-1-i] = intang;
        pixelangle[halfview+i] = -intang;
    }
}

void GameEngine::setupWalls()
{
    int     i;

    horizwall[0]=0;
    vertwall[0]=0;

    for (i=1;i<MAXWALLTILES;i++) {
        horizwall[i]=(i-1)*2;
        vertwall[i]=(i-1)*2+1;
    }
}

void GameEngine::signonScreen()
{
    wlrender->setVideoMode();
    wlrender->mungePic(signon, 320, 200);
    wlrender->memToScreen(signon, 320, 200, 0, 0);
}

void GameEngine::finishSignon()
{
    wlrender->bar(0, 189, 300, 11, wlrender->getPixel(0, 0));
    WindowX = 0;
    WindowW = 320;
    PrintY = 190;
    SETFONTCOLOR(14,4);
    wluser->US_CPrint("Press a key");
    wlpaint->updateScreen();
    if (!param_nowait)
        wlinput->ack();
    wlrender->bar(0, 189, 300, 11, wlrender->getPixel(0, 0));
    PrintY = 190;
    SETFONTCOLOR(10,4);
    wluser->US_CPrint("Working...");
    wlpaint->updateScreen();
    SETFONTCOLOR(0,15);
}

void GameEngine::initDigiMap()
{
    //int *map;
    wolfdigimap_t* map;
    if (g_Extension.toLower().startsWith("wl"))
        map = &wl6Wolfdigimap[0];
    else
        map = &sodWolfdigimap[0];

    for (; map->name != "LASTSOUND"; map++) {
        wlaudio->setDigiMap(wlcache->findSound(map->name), map->a);
        wlaudio->setDigiChannel(map->a, map->b);
        wlaudio->prepareSound(map->a);
    }
}

void GameEngine::doJukebox()
{
    int which,lastsong=-1;
    unsigned start;
    QString songs[]=
    {
        "GETTHEM_MUS", "SEARCHN_MUS", "POW_MUS", "SUSPENSE_MUS", "WARMARCH_MUS",
        "CORNER_MUS", "NAZI_OMI_MUS", "PREGNANT_MUS", "GOINGAFT_MUS", "HEADACHE_MUS",
        "DUNGEON_MUS", "ULTIMATE_MUS", "INTROCW3_MUS", "NAZI_RAP_MUS", "TWELFTH_MUS",
        "ZEROHOUR_MUS", "ULTIMATE_MUS", "PACMAN_MUS"
    };
    wlinput->clearKeysDown();
    if (!wlaudio->hasAdLib() && !wlaudio->hasSoundBlaster())
        return;
    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    start = 0;
    wlcache->cacheGraphic(STARTFONT+1);
    //CacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
    CacheLump(wlcache->find("C_OPTIONSPIC"), wlcache->find("L_GUYPIC") - 1);
    wlcache->loadAllSounds();
    fontnumber = 1;
    ClearMScreen();
    wlpaint->drawPic(112, 184, wlcache->find("C_MOUSELBACKPIC"));
    DrawStripes (10);
    SETFONTCOLOR (TEXTCOLOR,g_bkgdColor);
    DrawWindow(CTL_X-2,CTL_Y-6,280,13*7,g_bkgdColor);
    DrawMenu(&MusicItems,&MusicMenu[start]);
    SETFONTCOLOR (READHCOLOR,g_bkgdColor);
    PrintY = 15;
    WindowX = 0;
    WindowY = 320;
    wluser->US_CPrint("Robert's Jukebox");
    SETFONTCOLOR (TEXTCOLOR,g_bkgdColor);
    wlpaint->updateScreen();
    wlrender->fadeIn(0, 255, gamepal, 10);

    do {
        which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
        if (which>=0)
        {
            if (lastsong >= 0)
                MusicMenu[start+lastsong].active = 1;

            StartCPMusic(wlcache->findMusic(songs[start + which]));
            MusicMenu[start+which].active = 2;
            DrawMenu (&MusicItems,&MusicMenu[start]);
            wlpaint->updateScreen();
            lastsong = which;
        }
    } while(which>=0);

    wlrender->fadeOut(0, 255, 43, 0, 0, 10);
    wlinput->clearKeysDown();
    //UnCacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
    UnCacheLump(wlcache->find("C_OPTIONSPIC"), wlcache->find("L_GUYPIC") - 1);
}

void GameEngine::initGame()
{
    qint8 didjukebox = false;

    wlrender = new WLRender();
    signonScreen();
    wlpaint = new WLPaint();
    wlpage = new WLPage(QString("vswap.%1").arg(g_Extension), 4096);
    wlaudio = new WLAudio();
    wlplay = new WLPlay();
    wluser = new WLUser();
    initDigiMap();
    readConfig();
    SetupSaveGames();
    if (wlinput->isKeyDown(sc_M)) {
        doJukebox();
        didjukebox = true;
    } else
        IntroScreen();
    wlcache->cacheGraphic(STARTFONT);
    wlcache->cacheGraphic(wlcache->find("STATUSBARPIC"));
    wlpaint->loadLatchMem();
    buildTables();
    setupWalls();
    newViewSize(viewsize);
    wlplay->initRedShifts();
    if(!didjukebox) finishSignon();
}

qint8 GameEngine::setViewSize(quint32 width, quint32 height)
{
    viewwidth = width&~15;                  // must be divisable by 16
    viewheight = height&~1;                 // must be even
    centerx = viewwidth/2-1;
    shootdelta = viewwidth/10;
    if((quint32)viewheight == (quint32)g_Image->height())
        viewscreenx = viewscreeny = screenofs = 0;
    else {
        viewscreenx = (g_Image->width()-viewwidth) / 2;
        viewscreeny = (g_Image->height()-scaleFactor*STATUSLINES-viewheight)/2;
        screenofs = viewscreeny*g_Image->width()+viewscreenx;
    }
    calcProjection(0x5700l);
    return true;
}

void GameEngine::showViewSize(qint32 width)
{
    int oldwidth,oldheight;

    oldwidth = viewwidth;
    oldheight = viewheight;

    if(width == 21) {
        viewwidth = g_Image->width();
        viewheight = g_Image->height();
        wlrender->barScaledCoord(0, 0, g_Image->width(), g_Image->height(), 0);
    } else if(width == 20) {
        viewwidth = g_Image->width();
        viewheight = g_Image->height() - scaleFactor*STATUSLINES;
        DrawPlayBorder ();
    } else {
        viewwidth = width*16*g_Image->width()/320;
        viewheight = (int) (width*16*HEIGHTRATIO*g_Image->height()/200);
        DrawPlayBorder ();
    }
    viewwidth = oldwidth;
    viewheight = oldheight;
}

void GameEngine::newViewSize(qint32 width)
{
    viewsize = width;
    if(viewsize == 21)
        setViewSize(g_screenSize.width(), g_screenSize.height());
    else if(viewsize == 20)
        setViewSize(g_Image->width(), g_Image->height() - scaleFactor * STATUSLINES);
    else
        setViewSize(width*16*g_Image->width()/320, (unsigned) (width*16*HEIGHTRATIO*g_Image->height()/200));
}

void GameEngine::setImage(QImage *screen)
{
    m_screen = screen->convertToFormat(QImage::Format_RGB32);
    m_display->setImage(&m_screen);
}

void GameEngine::demoLoop()
{
#ifdef TOUCHSCREEN
    m_accelerometer->start();
#endif
    int LastDemo = 0;

    if (param_tedlevel != -1) {
        param_nowait = true;
        EnableEndGameMenuItem();
        newGame(param_difficulty,0);
        gamestate.episode = param_tedlevel/10;
        gamestate.mapon = param_tedlevel%10;
        GameLoop();
        Quit (NULL);
    }
    StartCPMusic(wlcache->findMusic("NAZI_NOR_MUS"));

    if (!param_nowait)
        PG13();
    while (1) {
        while (!param_nowait) {
            if (g_Extension.toLower().startsWith("wl")) {
                wlcache->cacheScreen(wlcache->find("TITLEPIC"));
                wlpaint->updateScreen();
                wlrender->fadeIn(0,255,gamepal,30);
            } else {
                wlcache->cacheGraphic(wlcache->find("TITLEPALETTE"));
                wlcache->cacheGraphic(wlcache->find("TITLE1PIC"));
                wlpaint->drawPic(0, 0, wlcache->find("TITLE1PIC"));
                wlcache->uncacheGraphic(wlcache->find("TITLE1PIC"));
                wlcache->cacheGraphic(wlcache->find("TITLE2PIC"));
                wlpaint->drawPic(0, 80, wlcache->find("TITLE2PIC"));
                wlcache->uncacheGraphic(wlcache->find("TITLE2PIC"));
                wlpaint->updateScreen();
                wlrender->fadeIn(0,255,(Color_t*)wlcache->graphic(wlcache->find("TITLEPALETTE")),30);
                wlcache->uncacheGraphic(wlcache->find("TITLEPALETTE"));
            }


            if (wlinput->userInput(g_tickBase*15))
                break;
            wlrender->fadeOut(0, 255, 0, 0, 0, 30);
            wlcache->cacheScreen(wlcache->find("CREDITSPIC"));
            wlpaint->updateScreen();
            wlrender->fadeIn(0, 255, gamepal, 30);
            //if (wlinput->userInput(g_tickBase*10))
            //   break;
            wlrender->fadeOut(0, 255, 0, 0, 0, 30);
            DrawHighScores();
            wlpaint->updateScreen();
            //if (wlinput->userInput(g_tickBase*10))
            //    break;
            wlrender->fadeIn(0, 255, gamepal, 30);
            //if (wlinput->userInput(g_tickBase*10))
            //    break;
            PlayDemo (LastDemo++%4);
            if (playstate == ex_abort)
                break;
            wlrender->fadeOut(0, 255, 0, 0, 0, 30);
            if(g_Image->height() % 200 != 0)
                wlrender->clearScreen(0);
            StartCPMusic(wlcache->findMusic("NAZI_NOR_MUS"));
        }
        wlrender->fadeOut(0, 255, 0, 0, 0, 30);
        US_ControlPanel (0);

        if (startgame || loadedgame) {
            GameLoop ();
            if(!param_nowait) {
                wlrender->fadeOut(0, 255, 0, 0, 0, 30);
                StartCPMusic(wlcache->findMusic("NAZI_NOR_MUS"));
            }
        }
    }
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    g_getTicks = new QTime;
    g_getTicks->start();
    engine = new GameEngine;
    return app.exec();
}

