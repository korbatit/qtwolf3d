#ifndef WL_DEF_H
#define WL_DEF_H

#include <QApplication>
#include <QTime>
#include <QFile>
#include <QImage>

#include "version.h"

#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#if !defined O_BINARY
#	define O_BINARY 0
#endif

#pragma pack(1)

#include "foreign.h"
#include "audio.h"

#define NUMTILE8     72
#define NUMTILE8M    0
#define STRUCTPIC    0
#define STARTFONT    1
#define STARTPICS    3

typedef struct {
    int x,y;
} Point;

typedef struct {
    Point ul,lr;
} Rect;

void Quit(const char *errorStr, ...);

#include "wlrender.h"
#include "wlcache.h"
#include "wl_menu.h"

#define SIGN(x)         ((x)>0?1:-1)
#define ABS(x)          ((int)(x)>0?(x):-(x))
#define LABS(x)         ((qint32)(x)>0?(x):-(x))

#define abs(x) ABS(x)

#define MAXTICS 10
#define DEMOTICS        4

#define MAXACTORS       150
#define MAXSTATS        400
#define MAXDOORS        64
#define MAXWALLTILES    64

#define ICONARROWS      90
#define PUSHABLETILE    98
#define EXITTILE        99
#define AREATILE        107
#define NUMAREAS        37
#define ELEVATORTILE    21
#define AMBUSHTILE      106
#define ALTELEVATORTILE 107

#define NUMBERCHARS     9

#define EXTRAPOINTS     40000

#define PLAYERSPEED     3000
#define RUNSPEED        6000

#define SCREENSEG       0xa000

#define SCREENBWIDE     80

#define HEIGHTRATIO     0.50

#define BORDERCOLOR     3
#define FLASHCOLOR      5
#define FLASHTICS       4

#ifndef SPEAR
#define LRpack      8
#else
#define LRpack      20
#endif

#define PLAYERSIZE      MINDIST
#define MINACTORDIST    0x10000l

#define NUMLATCHPICS    100

#undef M_PI
#define PI              3.141592657
#define M_PI PI

#define GLOBAL1         (1l<<16)
#define TILEGLOBAL      GLOBAL1
#define PIXGLOBAL       (GLOBAL1/64)
#define TILESHIFT       16l
#define UNSIGNEDSHIFT   8

#define ANGLES          360
#define ANGLEQUAD       (ANGLES/4)
#define FINEANGLES      3600
#define ANG90           (FINEANGLES/4)
#define ANG180          (ANG90*2)
#define ANG270          (ANG90*3)
#define ANG360          (ANG90*4)
#define VANG90          (ANGLES/4)
#define VANG180         (VANG90*2)
#define VANG270         (VANG90*3)
#define VANG360         (VANG90*4)

#define MINDIST         (0x5800l)

#define mapshift        6
#define MAPSIZE         (1<<mapshift)
#define maparea         MAPSIZE*MAPSIZE

#define mapheight       MAPSIZE
#define mapwidth        MAPSIZE

#ifdef USE_HIRES

#define TEXTURESHIFT    7
#define TEXTURESIZE     (1<<TEXTURESHIFT)
#define TEXTUREFROMFIXEDSHIFT 2
#define TEXTUREMASK     (TEXTURESIZE*(TEXTURESIZE-1))

#define SPRITESCALEFACTOR 1

#else

#define TEXTURESHIFT    6
#define TEXTURESIZE     (1<<TEXTURESHIFT)
#define TEXTUREFROMFIXEDSHIFT 4
#define TEXTUREMASK     (TEXTURESIZE*(TEXTURESIZE-1))

#define SPRITESCALEFACTOR 2

#endif

#define NORTH   0
#define EAST    1
#define SOUTH   2
#define WEST    3


#define STATUSLINES     40

#define SCREENSIZE      (SCREENBWIDE*208)
#define PAGE1START      0
#define PAGE2START      (SCREENSIZE)
#define PAGE3START      (SCREENSIZE*2u)
#define FREESTART       (SCREENSIZE*3u)


#define PIXRADIUS       512

#define STARTAMMO       8

typedef enum {
    FL_SHOOTABLE        = 0x00000001,
    FL_BONUS            = 0x00000002,
    FL_NEVERMARK        = 0x00000004,
    FL_VISABLE          = 0x00000008,
    FL_ATTACKMODE       = 0x00000010,
    FL_FIRSTATTACK      = 0x00000020,
    FL_AMBUSH           = 0x00000040,
    FL_NONMARK          = 0x00000080,
    FL_FULLBRIGHT       = 0x00000100,
} objflag_t;

enum
{
    SPR_DEMO,
    SPR_DEATHCAM,
    SPR_STAT_0,SPR_STAT_1,SPR_STAT_2,SPR_STAT_3,
    SPR_STAT_4,SPR_STAT_5,SPR_STAT_6,SPR_STAT_7,

    SPR_STAT_8,SPR_STAT_9,SPR_STAT_10,SPR_STAT_11,
    SPR_STAT_12,SPR_STAT_13,SPR_STAT_14,SPR_STAT_15,

    SPR_STAT_16,SPR_STAT_17,SPR_STAT_18,SPR_STAT_19,
    SPR_STAT_20,SPR_STAT_21,SPR_STAT_22,SPR_STAT_23,

    SPR_STAT_24,SPR_STAT_25,SPR_STAT_26,SPR_STAT_27,
    SPR_STAT_28,SPR_STAT_29,SPR_STAT_30,SPR_STAT_31,

    SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
    SPR_STAT_36,SPR_STAT_37,SPR_STAT_38,SPR_STAT_39,

    SPR_STAT_40,SPR_STAT_41,SPR_STAT_42,SPR_STAT_43,
    SPR_STAT_44,SPR_STAT_45,SPR_STAT_46,SPR_STAT_47,

    SPR_GRD_S_1,SPR_GRD_S_2,SPR_GRD_S_3,SPR_GRD_S_4,
    SPR_GRD_S_5,SPR_GRD_S_6,SPR_GRD_S_7,SPR_GRD_S_8,

    SPR_GRD_W1_1,SPR_GRD_W1_2,SPR_GRD_W1_3,SPR_GRD_W1_4,
    SPR_GRD_W1_5,SPR_GRD_W1_6,SPR_GRD_W1_7,SPR_GRD_W1_8,

    SPR_GRD_W2_1,SPR_GRD_W2_2,SPR_GRD_W2_3,SPR_GRD_W2_4,
    SPR_GRD_W2_5,SPR_GRD_W2_6,SPR_GRD_W2_7,SPR_GRD_W2_8,

    SPR_GRD_W3_1,SPR_GRD_W3_2,SPR_GRD_W3_3,SPR_GRD_W3_4,
    SPR_GRD_W3_5,SPR_GRD_W3_6,SPR_GRD_W3_7,SPR_GRD_W3_8,

    SPR_GRD_W4_1,SPR_GRD_W4_2,SPR_GRD_W4_3,SPR_GRD_W4_4,
    SPR_GRD_W4_5,SPR_GRD_W4_6,SPR_GRD_W4_7,SPR_GRD_W4_8,

    SPR_GRD_PAIN_1,SPR_GRD_DIE_1,SPR_GRD_DIE_2,SPR_GRD_DIE_3,
    SPR_GRD_PAIN_2,SPR_GRD_DEAD,

    SPR_GRD_SHOOT1,SPR_GRD_SHOOT2,SPR_GRD_SHOOT3,

    SPR_DOG_W1_1,SPR_DOG_W1_2,SPR_DOG_W1_3,SPR_DOG_W1_4,
    SPR_DOG_W1_5,SPR_DOG_W1_6,SPR_DOG_W1_7,SPR_DOG_W1_8,

    SPR_DOG_W2_1,SPR_DOG_W2_2,SPR_DOG_W2_3,SPR_DOG_W2_4,
    SPR_DOG_W2_5,SPR_DOG_W2_6,SPR_DOG_W2_7,SPR_DOG_W2_8,

    SPR_DOG_W3_1,SPR_DOG_W3_2,SPR_DOG_W3_3,SPR_DOG_W3_4,
    SPR_DOG_W3_5,SPR_DOG_W3_6,SPR_DOG_W3_7,SPR_DOG_W3_8,

    SPR_DOG_W4_1,SPR_DOG_W4_2,SPR_DOG_W4_3,SPR_DOG_W4_4,
    SPR_DOG_W4_5,SPR_DOG_W4_6,SPR_DOG_W4_7,SPR_DOG_W4_8,

    SPR_DOG_DIE_1,SPR_DOG_DIE_2,SPR_DOG_DIE_3,SPR_DOG_DEAD,
    SPR_DOG_JUMP1,SPR_DOG_JUMP2,SPR_DOG_JUMP3,

    SPR_SS_S_1,SPR_SS_S_2,SPR_SS_S_3,SPR_SS_S_4,
    SPR_SS_S_5,SPR_SS_S_6,SPR_SS_S_7,SPR_SS_S_8,

    SPR_SS_W1_1,SPR_SS_W1_2,SPR_SS_W1_3,SPR_SS_W1_4,
    SPR_SS_W1_5,SPR_SS_W1_6,SPR_SS_W1_7,SPR_SS_W1_8,

    SPR_SS_W2_1,SPR_SS_W2_2,SPR_SS_W2_3,SPR_SS_W2_4,
    SPR_SS_W2_5,SPR_SS_W2_6,SPR_SS_W2_7,SPR_SS_W2_8,

    SPR_SS_W3_1,SPR_SS_W3_2,SPR_SS_W3_3,SPR_SS_W3_4,
    SPR_SS_W3_5,SPR_SS_W3_6,SPR_SS_W3_7,SPR_SS_W3_8,

    SPR_SS_W4_1,SPR_SS_W4_2,SPR_SS_W4_3,SPR_SS_W4_4,
    SPR_SS_W4_5,SPR_SS_W4_6,SPR_SS_W4_7,SPR_SS_W4_8,

    SPR_SS_PAIN_1,SPR_SS_DIE_1,SPR_SS_DIE_2,SPR_SS_DIE_3,
    SPR_SS_PAIN_2,SPR_SS_DEAD,

    SPR_SS_SHOOT1,SPR_SS_SHOOT2,SPR_SS_SHOOT3,

    SPR_MUT_S_1,SPR_MUT_S_2,SPR_MUT_S_3,SPR_MUT_S_4,
    SPR_MUT_S_5,SPR_MUT_S_6,SPR_MUT_S_7,SPR_MUT_S_8,

    SPR_MUT_W1_1,SPR_MUT_W1_2,SPR_MUT_W1_3,SPR_MUT_W1_4,
    SPR_MUT_W1_5,SPR_MUT_W1_6,SPR_MUT_W1_7,SPR_MUT_W1_8,

    SPR_MUT_W2_1,SPR_MUT_W2_2,SPR_MUT_W2_3,SPR_MUT_W2_4,
    SPR_MUT_W2_5,SPR_MUT_W2_6,SPR_MUT_W2_7,SPR_MUT_W2_8,

    SPR_MUT_W3_1,SPR_MUT_W3_2,SPR_MUT_W3_3,SPR_MUT_W3_4,
    SPR_MUT_W3_5,SPR_MUT_W3_6,SPR_MUT_W3_7,SPR_MUT_W3_8,

    SPR_MUT_W4_1,SPR_MUT_W4_2,SPR_MUT_W4_3,SPR_MUT_W4_4,
    SPR_MUT_W4_5,SPR_MUT_W4_6,SPR_MUT_W4_7,SPR_MUT_W4_8,

    SPR_MUT_PAIN_1,SPR_MUT_DIE_1,SPR_MUT_DIE_2,SPR_MUT_DIE_3,
    SPR_MUT_PAIN_2,SPR_MUT_DIE_4,SPR_MUT_DEAD,

    SPR_MUT_SHOOT1,SPR_MUT_SHOOT2,SPR_MUT_SHOOT3,SPR_MUT_SHOOT4,

    SPR_OFC_S_1,SPR_OFC_S_2,SPR_OFC_S_3,SPR_OFC_S_4,
    SPR_OFC_S_5,SPR_OFC_S_6,SPR_OFC_S_7,SPR_OFC_S_8,

    SPR_OFC_W1_1,SPR_OFC_W1_2,SPR_OFC_W1_3,SPR_OFC_W1_4,
    SPR_OFC_W1_5,SPR_OFC_W1_6,SPR_OFC_W1_7,SPR_OFC_W1_8,

    SPR_OFC_W2_1,SPR_OFC_W2_2,SPR_OFC_W2_3,SPR_OFC_W2_4,
    SPR_OFC_W2_5,SPR_OFC_W2_6,SPR_OFC_W2_7,SPR_OFC_W2_8,

    SPR_OFC_W3_1,SPR_OFC_W3_2,SPR_OFC_W3_3,SPR_OFC_W3_4,
    SPR_OFC_W3_5,SPR_OFC_W3_6,SPR_OFC_W3_7,SPR_OFC_W3_8,

    SPR_OFC_W4_1,SPR_OFC_W4_2,SPR_OFC_W4_3,SPR_OFC_W4_4,
    SPR_OFC_W4_5,SPR_OFC_W4_6,SPR_OFC_W4_7,SPR_OFC_W4_8,

    SPR_OFC_PAIN_1,SPR_OFC_DIE_1,SPR_OFC_DIE_2,SPR_OFC_DIE_3,
    SPR_OFC_PAIN_2,SPR_OFC_DIE_4,SPR_OFC_DEAD,

    SPR_OFC_SHOOT1,SPR_OFC_SHOOT2,SPR_OFC_SHOOT3,

    SPR_BLINKY_W1,SPR_BLINKY_W2,SPR_PINKY_W1,SPR_PINKY_W2,
    SPR_CLYDE_W1,SPR_CLYDE_W2,SPR_INKY_W1,SPR_INKY_W2,

    SPR_BOSS_W1,SPR_BOSS_W2,SPR_BOSS_W3,SPR_BOSS_W4,
    SPR_BOSS_SHOOT1,SPR_BOSS_SHOOT2,SPR_BOSS_SHOOT3,SPR_BOSS_DEAD,

    SPR_BOSS_DIE1,SPR_BOSS_DIE2,SPR_BOSS_DIE3,

    SPR_SCHABB_W1,SPR_SCHABB_W2,SPR_SCHABB_W3,SPR_SCHABB_W4,
    SPR_SCHABB_SHOOT1,SPR_SCHABB_SHOOT2,

    SPR_SCHABB_DIE1,SPR_SCHABB_DIE2,SPR_SCHABB_DIE3,SPR_SCHABB_DEAD,
    SPR_HYPO1,SPR_HYPO2,SPR_HYPO3,SPR_HYPO4,

    SPR_FAKE_W1,SPR_FAKE_W2,SPR_FAKE_W3,SPR_FAKE_W4,
    SPR_FAKE_SHOOT,SPR_FIRE1,SPR_FIRE2,

    SPR_FAKE_DIE1,SPR_FAKE_DIE2,SPR_FAKE_DIE3,SPR_FAKE_DIE4,
    SPR_FAKE_DIE5,SPR_FAKE_DEAD,

    SPR_MECHA_W1,SPR_MECHA_W2,SPR_MECHA_W3,SPR_MECHA_W4,
    SPR_MECHA_SHOOT1,SPR_MECHA_SHOOT2,SPR_MECHA_SHOOT3,SPR_MECHA_DEAD,

    SPR_MECHA_DIE1,SPR_MECHA_DIE2,SPR_MECHA_DIE3,

    SPR_HITLER_W1,SPR_HITLER_W2,SPR_HITLER_W3,SPR_HITLER_W4,
    SPR_HITLER_SHOOT1,SPR_HITLER_SHOOT2,SPR_HITLER_SHOOT3,SPR_HITLER_DEAD,

    SPR_HITLER_DIE1,SPR_HITLER_DIE2,SPR_HITLER_DIE3,SPR_HITLER_DIE4,
    SPR_HITLER_DIE5,SPR_HITLER_DIE6,SPR_HITLER_DIE7,

    SPR_GIFT_W1,SPR_GIFT_W2,SPR_GIFT_W3,SPR_GIFT_W4,
    SPR_GIFT_SHOOT1,SPR_GIFT_SHOOT2,

    SPR_GIFT_DIE1,SPR_GIFT_DIE2,SPR_GIFT_DIE3,SPR_GIFT_DEAD,

    SPR_ROCKET_1,SPR_ROCKET_2,SPR_ROCKET_3,SPR_ROCKET_4,
    SPR_ROCKET_5,SPR_ROCKET_6,SPR_ROCKET_7,SPR_ROCKET_8,

    SPR_SMOKE_1,SPR_SMOKE_2,SPR_SMOKE_3,SPR_SMOKE_4,
    SPR_BOOM_1,SPR_BOOM_2,SPR_BOOM_3,

    SPR_GRETEL_W1,SPR_GRETEL_W2,SPR_GRETEL_W3,SPR_GRETEL_W4,
    SPR_GRETEL_SHOOT1,SPR_GRETEL_SHOOT2,SPR_GRETEL_SHOOT3,SPR_GRETEL_DEAD,

    SPR_GRETEL_DIE1,SPR_GRETEL_DIE2,SPR_GRETEL_DIE3,

    SPR_FAT_W1,SPR_FAT_W2,SPR_FAT_W3,SPR_FAT_W4,
    SPR_FAT_SHOOT1,SPR_FAT_SHOOT2,SPR_FAT_SHOOT3,SPR_FAT_SHOOT4,

    SPR_FAT_DIE1,SPR_FAT_DIE2,SPR_FAT_DIE3,SPR_FAT_DEAD,

    SPR_BJ_W1,
    SPR_BJ_W2,SPR_BJ_W3,SPR_BJ_W4,
    SPR_BJ_JUMP1,SPR_BJ_JUMP2,SPR_BJ_JUMP3,SPR_BJ_JUMP4,

    SPR_KNIFEREADY,SPR_KNIFEATK1,SPR_KNIFEATK2,SPR_KNIFEATK3,
    SPR_KNIFEATK4,

    SPR_PISTOLREADY,SPR_PISTOLATK1,SPR_PISTOLATK2,SPR_PISTOLATK3,
    SPR_PISTOLATK4,

    SPR_MACHINEGUNREADY,SPR_MACHINEGUNATK1,SPR_MACHINEGUNATK2,MACHINEGUNATK3,
    SPR_MACHINEGUNATK4,

    SPR_CHAINREADY,SPR_CHAINATK1,SPR_CHAINATK2,SPR_CHAINATK3,
    SPR_CHAINATK4,
};

typedef enum {
    di_north,
    di_east,
    di_south,
    di_west
} controldir_t;

typedef enum {
    dr_normal,
    dr_lock1,
    dr_lock2,
    dr_lock3,
    dr_lock4,
    dr_elevator
} door_t;

typedef enum {
    ac_badobject = -1,
    ac_no,
    ac_yes,
    ac_allways
} activetype;

typedef enum {
    nothing,
    playerobj,
    inertobj,
    guardobj,
    officerobj,
    ssobj,
    dogobj,
    bossobj,
    schabbobj,
    fakeobj,
    mechahitlerobj,
    mutantobj,
    needleobj,
    fireobj,
    bjobj,
    ghostobj,
    realhitlerobj,
    gretelobj,
    giftobj,
    fatobj,
    rocketobj,

    spectreobj,
    angelobj,
    transobj,
    uberobj,
    willobj,
    deathobj,
    hrocketobj,
    sparkobj
} classtype;

typedef enum {
    none,
    block,
    bo_gibs,
    bo_alpo,
    bo_firstaid,
    bo_key1,
    bo_key2,
    bo_key3,
    bo_key4,
    bo_cross,
    bo_chalice,
    bo_bible,
    bo_crown,
    bo_clip,
    bo_clip2,
    bo_machinegun,
    bo_chaingun,
    bo_food,
    bo_fullheal,
    bo_25clip,
    bo_spear
} wl_stat_t;

typedef enum {
    east,
    northEast,
    north,
    northWest,
    west,
    southWest,
    south,
    southEast,
    noDir
} dirType_t;


#define NUMENEMIES  22
typedef enum {
    en_guard,
    en_officer,
    en_ss,
    en_dog,
    en_boss,
    en_schabbs,
    en_fake,
    en_hitler,
    en_mutant,
    en_blinky,
    en_clyde,
    en_pinky,
    en_inky,
    en_gretel,
    en_gift,
    en_fat,
    en_spectre,
    en_angel,
    en_trans,
    en_uber,
    en_will,
    en_death
} enemy_t;

typedef void (* statefunc) (void *);

typedef struct statestruct
{
    qint8 rotate;
    short   shapenum;
    short   tictime;
    void    (*think) (void *),(*action) (void *);
    struct  statestruct *next;
} statetype;

typedef struct statstruct
{
    quint8  tileX;
    quint8  tileY;
    qint16  shapeNum;
    quint8* visSpot;
    quint32 flags;
    quint8  itemNumber;
} statObj_t;

typedef enum
{
    dr_open,dr_closed,dr_opening,dr_closing
} doortype;

typedef struct doorstruct
{
    quint8     tilex,tiley;
    qint8  vertical;
    quint8     lock;
    doortype action;
    short    ticcount;
} doorObj_t;

typedef struct objstruct
{
    activetype  active;
    short       ticcount;
    classtype   obclass;
    statetype   *state;

    quint32    flags;

    qint32     distance;
    dirType_t     dir;

    qint32       x,y;
    quint16     tilex,tiley;
    quint8        areanumber;

    short       viewx;
    quint16        viewheight;
    qint32       transx,transy;

    short       angle;
    short       hitpoints;
    qint32     speed;

    short       temp1,temp2,hidden;
    struct objstruct *next,*prev;
} objtype;

enum
{
    bt_nobutton=-1,
    bt_attack=0,
    bt_strafe,
    bt_run,
    bt_use,
    bt_readyknife,
    bt_readypistol,
    bt_readymachinegun,
    bt_readychaingun,
    bt_nextweapon,
    bt_prevweapon,
    bt_esc,
    bt_pause,
    bt_strafeleft,
    bt_straferight,
    bt_moveforward,
    bt_movebackward,
    bt_turnleft,
    bt_turnright,
    NUMBUTTONS
};


#define NUMWEAPONS      4
typedef enum
{
    wp_knife,
    wp_pistol,
    wp_machinegun,
    wp_chaingun
} weapontype;


enum
{
    gd_baby,
    gd_easy,
    gd_medium,
    gd_hard
};

typedef struct
{
    short       difficulty;
    short       mapon;
    qint32     oldscore,score,nextextra;
    short       lives;
    short       health;
    short       ammo;
    short       keys;
    weapontype  bestweapon,weapon,chosenweapon;

    short       faceframe;
    short       attackframe,attackcount,weaponframe;

    short       episode,secretcount,treasurecount,killcount,
    secrettotal,treasuretotal,killtotal;
    qint32     TimeCount;
    qint32     killx,killy;
    qint8     victoryflag;
} gametype;


typedef enum
{
    ex_stillplaying,
    ex_completed,
    ex_died,
    ex_warped,
    ex_resetgame,
    ex_loadedgame,
    ex_victorious,
    ex_abort,
    ex_demodone,
    ex_secretlevel
} exit_t;

extern  bool     loadedgame;
extern  qint32    focallength;
extern  int      viewscreenx, viewscreeny;
extern  int      viewwidth;
extern  int      viewheight;
extern  short    centerx;
extern  qint32  heightnumerator;
extern  qint32    scale;

extern  int      dirangle[9];

extern  int      mouseadjustment;
extern  int      shootdelta;
extern  unsigned screenofs;

extern  qint8  startgame;
extern  char     str[80];
extern  char     configname[13];

extern  qint8  param_debugmode;
extern  qint8  param_nowait;
extern  int      param_difficulty;
extern  int      param_tedlevel;
extern  int      param_joystickindex;
extern  int      param_joystickhat;
extern  int      param_samplerate;
extern  int      param_audiobuffer;
extern  int      param_mission;
extern  qint8  param_goodtimes;
extern  qint8  param_ignorenumchunks;


void            NewGame (int difficulty,int episode);
void            CalcProjection (qint32 focal);
void            NewViewSize (int width);
qint8         SetViewSize (unsigned width, unsigned height);
qint8         LoadTheGame(QFile* fp,int x,int y);
qint8         SaveTheGame(QFile* fp,int x,int y);
void            ShowViewSize (int width);
void            ShutdownId (void);

extern void ScreenShot();

extern  gametype        gamestate;
extern  quint8          bordercol;
extern  QImage*         latchpics[NUMLATCHPICS];
extern  char            demoname[13];

void    SetupGameLevel (void);
void    GameLoop (void);
void    DrawPlayBorder (void);
void    DrawStatusBorder (quint8 color);
void    DrawPlayScreen (void);
void    DrawPlayBorderSides (void);
void    ShowActStatus();

void    PlayDemo (int demonumber);
void    RecordDemo (void);

#define PlaySoundLocTile(s,tx,ty)       PlaySoundLocGlobal(s,(((qint32)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))),(((qint32)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))
#define PlaySoundLocActor(s,ob)         PlaySoundLocGlobal(s,(ob)->x,(ob)->y)
void    PlaySoundLocGlobal(quint16 s,qint32 gx,qint32 gy);
void UpdateSoundLoc(void);

#define BASEMOVE                35
#define RUNMOVE                 70
#define BASETURN                35
#define RUNTURN                 70

#define JOYSCALE                2

extern  quint8            tilemap[MAPSIZE][MAPSIZE];
extern  quint8            spotvis[MAPSIZE][MAPSIZE];
extern  objtype         *actorat[MAPSIZE][MAPSIZE];

extern  objtype         *player;

extern  unsigned        tics;
extern  int             viewsize;

extern  int             lastgamemusicoffset;

extern  int         controlx,controly;
extern  qint8     buttonstate[NUMBUTTONS];
extern  objtype     objlist[MAXACTORS];
extern  qint8     buttonheld[NUMBUTTONS];
extern  exit_t      playstate;
extern  qint8     madenoise;
extern  statObj_t   statobjlist[MAXSTATS];
extern  statObj_t   *laststatobj;
extern  objtype     *newobj,*killerobj;
extern  doorObj_t   doorobjlist[MAXDOORS];
extern  doorObj_t   *lastdoorobj;
extern  int         godmode;

extern  qint8     demorecord,demoplayback;
extern  int8_t      *demoptr, *lastdemoptr;
extern  void*      demobuffer;

extern  qint8     mouseenabled,joystickenabled;
extern  int         dirscan[4];
extern  int         buttonscan[NUMBUTTONS];
extern  int         buttonmouse[4];
extern  int         buttonjoy[32];

void    InitActorList (void);
void    GetNewActor (void);
void    PlayLoop (void);

void    CenterWindow(quint16 w,quint16 h);

void    InitRedShifts (void);
void    FinishPaletteShifts (void);

void    RemoveObj (objtype *gone);
void    PollControls (void);
int     StopMusic(void);
void    StartMusic(void);
void    ContinueMusic(int offs);
void    StartDamageFlash (int damage);
void    StartBonusFlash (void);

extern  objtype     *objfreelist;

extern  qint8     noclip,ammocheat;
extern  int         singlestep, extravbls;

void IntroScreen (void);
void PG13(void);
void DrawHighScores(void);
void CheckHighScore (qint32 score,quint16 other);
void Victory (void);
void LevelCompleted (void);
void ClearSplitVWB (void);

void PreloadGraphics(void);

int DebugKeys (void);

extern  short *pixelangle;
extern  qint32 finetangent[FINEANGLES/4];
extern  qint32 sintable[];
extern  qint32 *costable;
extern  int *wallheight;
extern  quint16 horizwall[],vertwall[];
extern  qint32    lasttimecount;
extern  qint32    frameon;

extern  unsigned screenloc[3];

extern  qint8 fizzlein, fpscounter;

extern  qint32   viewx,viewy;
extern  qint32   viewsin,viewcos;

void    ThreeDRefresh (void);
void    CalcTics (void);

typedef struct {
    quint16 leftpix,rightpix;
    quint16 dataofs[64];
} t_compshape;

#define TURNTICS        10
#define SPDPATROL       512
#define SPDDOG          1500


void    InitHitRect (objtype *ob, unsigned radius);
void    SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state);
void    NewState (objtype *ob, statetype *state);

qint8 TryWalk (objtype *ob);
void    SelectChaseDir (objtype *ob);
void    SelectDodgeDir (objtype *ob);
void    SelectRunDir (objtype *ob);
void    MoveObj (objtype *ob, qint32 move);
qint8 SightPlayer (objtype *ob);

void    KillActor (objtype *ob);
void    DamageActor (objtype *ob, unsigned damage);

qint8 CheckLine (objtype *ob);
qint8 CheckSight (objtype *ob);

extern  short    anglefrac;
extern  int      facecount, facetimes;
extern  quint16     plux,pluy;
extern  qint32  thrustspeed;
extern  objtype  *LastAttacker;

void    Thrust (int angle, qint32 speed);
void    SpawnPlayer (int tilex, int tiley, int dir);
void    TakeDamage (int points,objtype *attacker);
void    GivePoints (qint32 points);
void    GetBonus (statObj_t *check);
void    GiveWeapon (int weapon);
void    GiveAmmo (int ammo);
void    GiveKey (int key);

void    StatusDrawFace(unsigned picnum);
void    DrawFace (void);
void    DrawHealth (void);
void    HealSelf (int points);
void    DrawLevel (void);
void    DrawLives (void);
void    GiveExtraMan (void);
void    DrawScore (void);
void    DrawWeapon (void);
void    DrawKeys (void);
void    DrawAmmo (void);

extern  doorObj_t doorobjlist[MAXDOORS];
extern  doorObj_t *lastdoorobj;
extern  short     doornum;

extern  quint16      doorposition[MAXDOORS];

extern  quint8      areaconnect[NUMAREAS][NUMAREAS];

extern  qint8   areabyplayer[NUMAREAS];

extern quint16     pwallstate;
extern quint16     pwallpos;
extern quint16     pwallx,pwally;
extern quint8     pwalldir,pwalltile;


void InitDoorList (void);
void InitStaticList (void);
void SpawnStatic (int tilex, int tiley, int type);
void SpawnDoor (int tilex, int tiley, qint8 vertical, int lock);
void MoveDoors (void);
void MovePWalls (void);
void OpenDoor (int door);
void PlaceItemType (int itemtype, int tilex, int tiley);
void PushWall (int checkx, int checky, int dir);
void OperateDoor (int door);
void InitAreas (void);

#define s_nakedbody s_static10

extern  statetype s_grddie1;
extern  statetype s_dogdie1;
extern  statetype s_ofcdie1;
extern  statetype s_mutdie1;
extern  statetype s_ssdie1;
extern  statetype s_bossdie1;
extern  statetype s_schabbdie1;
extern  statetype s_fakedie1;
extern  statetype s_mechadie1;
extern  statetype s_hitlerdie1;
extern  statetype s_greteldie1;
extern  statetype s_giftdie1;
extern  statetype s_fatdie1;

extern  statetype s_spectredie1;
extern  statetype s_angeldie1;
extern  statetype s_transdie0;
extern  statetype s_uberdie0;
extern  statetype s_willdie1;
extern  statetype s_deathdie1;


extern  statetype s_grdchase1;
extern  statetype s_dogchase1;
extern  statetype s_ofcchase1;
extern  statetype s_sschase1;
extern  statetype s_mutchase1;
extern  statetype s_bosschase1;
extern  statetype s_schabbchase1;
extern  statetype s_fakechase1;
extern  statetype s_mechachase1;
extern  statetype s_gretelchase1;
extern  statetype s_giftchase1;
extern  statetype s_fatchase1;

extern  statetype s_spectrechase1;
extern  statetype s_angelchase1;
extern  statetype s_transchase1;
extern  statetype s_uberchase1;
extern  statetype s_willchase1;
extern  statetype s_deathchase1;

extern  statetype s_blinkychase1;
extern  statetype s_hitlerchase1;

extern  statetype s_grdpain;
extern  statetype s_grdpain1;
extern  statetype s_ofcpain;
extern  statetype s_ofcpain1;
extern  statetype s_sspain;
extern  statetype s_sspain1;
extern  statetype s_mutpain;
extern  statetype s_mutpain1;

extern  statetype s_deathcam;

extern  statetype s_schabbdeathcam2;
extern  statetype s_hitlerdeathcam2;
extern  statetype s_giftdeathcam2;
extern  statetype s_fatdeathcam2;

void SpawnStand (enemy_t which, int tilex, int tiley, int dir);
void SpawnPatrol (enemy_t which, int tilex, int tiley, int dir);
void KillActor (objtype *ob);

void SpawnDeadGuard (int tilex, int tiley);
void SpawnBoss (int tilex, int tiley);
void SpawnGretel (int tilex, int tiley);
void SpawnTrans (int tilex, int tiley);
void SpawnUber (int tilex, int tiley);
void SpawnWill (int tilex, int tiley);
void SpawnDeath (int tilex, int tiley);
void SpawnAngel (int tilex, int tiley);
void SpawnSpectre (int tilex, int tiley);
void SpawnGhosts (int which, int tilex, int tiley);
void SpawnSchabbs (int tilex, int tiley);
void SpawnGift (int tilex, int tiley);
void SpawnFat (int tilex, int tiley);
void SpawnFakeHitler (int tilex, int tiley);
void SpawnHitler (int tilex, int tiley);

void A_DeathScream (objtype *ob);
void SpawnBJVictory (void);

#define StatusDrawLCD(x)

static inline qint32 FixedMul(qint32 a, qint32 b)
{
    return (qint32)(((int64_t)a * b + 0x8000) >> 16);
}

#define DEMOCHOOSE_ORIG_SDL(orig, sdl) (sdl)
#define DEMOCOND_ORIG                  false
#define DEMOIF_SDL
#define DEMOCOND_SDL                   (!DEMOCOND_ORIG)

#define ISPOINTER(x) ((((uintptr_t)(x)) & ~0xffff) != 0)

#define CHECKMALLOCRESULT(x) if(!(x)) Quit("Out of memory at %s:%i", __FILE__, __LINE__)

#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp
#else
static inline char* itoa(int value, char* string, int radix)
{
    sprintf(string, "%d", value);
    if(radix) return string;
    return string;
}

static inline char* ltoa(long value, char* string, int radix)
{
    sprintf(string, "%ld", value);
    if(radix) return string;
    return string;
}
#endif

#define lengthof(x) (sizeof(x) / sizeof(*(x)))
#define endof(x)    ((x) + lengthof(x))

static inline quint16 READWORD(quint8 *&ptr)
{
    quint16 val = ptr[0] | ptr[1] << 8;
    ptr += 2;
    return val;
}

static inline quint32 READLONGWORD(quint8 *&ptr)
{
    quint32 val = ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;
    ptr += 4;
    return val;
}


#endif
