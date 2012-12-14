#include "wlitem.h"

#include "wlcache.h"
#include "wlaudio.h"

extern WLCache* wlcache;
extern WLAudio* wlaudio;

statObj_t   statobjlist[MAXSTATS];
statObj_t   *laststatobj;

static const qint32 g_Dirs[4][2]={{0,-1},{1,0},{0,1},{-1,0}};

static const statInfo_t statinfo[] =
{
    {SPR_STAT_0,(wl_stat_t)0,0},            // puddle          spr1v
    {SPR_STAT_1,block,0},                   // Green Barrel    "
    {SPR_STAT_2,block,0},                   // Table/chairs    "
    {SPR_STAT_3,block,FL_FULLBRIGHT},       // Floor lamp      "
    {SPR_STAT_4,none,FL_FULLBRIGHT},        // Chandelier      "
    {SPR_STAT_5,block,0},                   // Hanged man      "
    {SPR_STAT_6,bo_alpo,0},                 // Bad food        "
    {SPR_STAT_7,block,0},                   // Red pillar      "
    {SPR_STAT_8,block,0},                   // Tree            spr2v
    {SPR_STAT_9,(wl_stat_t)0,0},            // Skeleton flat   "
    {SPR_STAT_10,block,0},                  // Sink            " (SOD:gibs)
    {SPR_STAT_11,block,0},                  // Potted plant    "
    {SPR_STAT_12,block,0},                  // Urn             "
    {SPR_STAT_13,block,0},                  // Bare table      "
    {SPR_STAT_14,none,FL_FULLBRIGHT},       // Ceiling light   "
    {SPR_STAT_15,(wl_stat_t)0,0},           // Kitchen stuff   "
    {SPR_STAT_16,block,0},                  // suit of armor   spr3v
    {SPR_STAT_17,block,0},                  // Hanging cage    "
    {SPR_STAT_18,block,0},                  // SkeletoninCage  "
    {SPR_STAT_19,(wl_stat_t)0,0},           // Skeleton relax  "
    {SPR_STAT_20,bo_key1,0},                // Key 1           "
    {SPR_STAT_21,bo_key2,0},                // Key 2           "
    {SPR_STAT_22,block,0},                  // stuff             (SOD:gibs)
    {SPR_STAT_23,(wl_stat_t)0,0},           // stuff
    {SPR_STAT_24,bo_food,0},                // Good food       spr4v
    {SPR_STAT_25,bo_firstaid,0},            // First aid       "
    {SPR_STAT_26,bo_clip,0},                // Clip            "
    {SPR_STAT_27,bo_machinegun,0},          // Machine gun     "
    {SPR_STAT_28,bo_chaingun,0},            // Gatling gun     "
    {SPR_STAT_29,bo_cross,0},               // Cross           "
    {SPR_STAT_30,bo_chalice,0},             // Chalice         "
    {SPR_STAT_31,bo_bible,0},               // Bible           "
    {SPR_STAT_32,bo_crown,0},               // crown           spr5v
    {SPR_STAT_33,bo_fullheal,FL_FULLBRIGHT},// one up          "
    {SPR_STAT_34,bo_gibs,0},                // gibs            "
    {SPR_STAT_35,block,0},                  // barrel          "
    {SPR_STAT_36,block,0},                  // well            "
    {SPR_STAT_37,block,0},                  // Empty well      "
    {SPR_STAT_38,bo_gibs,0},                // Gibs 2          "
    {SPR_STAT_39,block,0},                  // flag            "
    {SPR_STAT_40,block,0},                  // Call Apogee     spr7v
    {SPR_STAT_41,(wl_stat_t)0,0},           // junk            "
    {SPR_STAT_42,(wl_stat_t)0,0},           // junk            "
    {SPR_STAT_43,(wl_stat_t)0,0},           // junk            "
    {SPR_STAT_44,(wl_stat_t)0,0},           // pots            "
    {SPR_STAT_45,block,0},                  // stove           " (SOD:gibs)
    {SPR_STAT_46,block,0},                  // spears          " (SOD:gibs)
    {SPR_STAT_47,(wl_stat_t)0,0},           // vines           "
    {SPR_STAT_26,bo_clip2,0},               // Clip            "
    {-1,(wl_stat_t)0,0}                     // terminator
};


WLItem::WLItem()
{
}

WLItem::~WLItem()
{
}

void WLItem::initStaticList()
{
    laststatobj = &statobjlist[0];
}

void WLItem::spawnStatic(qint32 tileX, qint32 tileY, qint32 type)
{
    laststatobj->shapeNum = statinfo[type].picNum;
    laststatobj->tileX = tileX;
    laststatobj->tileY = tileY;
    laststatobj->visSpot = &spotvis[tileX][tileY];

    switch (statinfo[type].type) {
    case block:
        actorat[tileX][tileY] = (objtype*)1;
    case none:
        laststatobj->flags = 0;
        break;

    case    bo_cross:
    case    bo_chalice:
    case    bo_bible:
    case    bo_crown:
    case    bo_fullheal:
        if (!loadedgame)
            gamestate.treasuretotal++;

    case    bo_firstaid:
    case    bo_key1:
    case    bo_key2:
    case    bo_key3:
    case    bo_key4:
    case    bo_clip:
    case    bo_25clip:
    case    bo_machinegun:
    case    bo_chaingun:
    case    bo_food:
    case    bo_alpo:
    case    bo_gibs:
    case    bo_spear:
        laststatobj->flags = FL_BONUS;
        laststatobj->itemNumber = statinfo[type].type;
        break;
    default:
        break;
    }
    laststatobj->flags |= statinfo[type].specialFlags;
    laststatobj++;
    if (laststatobj == &statobjlist[MAXSTATS])
        Quit ("Too many static objects!\n");
}

void WLItem::placeItemType(qint32 itemType, qint32 tileX, qint32 tileY)
{
    qint32    type;
    statObj_t *spot;

    for (type = 0; ; type++) {
        if (statinfo[type].picNum == -1)  // end of list
            Quit ("placeItemType: couldn't find type!");
        if (statinfo[type].type == itemType)
            break;
    }

    for (spot = &statobjlist[0]; ; spot++) {
        if (spot == laststatobj) {
            if (spot == &statobjlist[MAXSTATS])
                return;
            laststatobj++;
            break;
        }
        if (spot->shapeNum == -1)
            break;
    }
    spot->shapeNum = statinfo[type].picNum;
    spot->tileX = tileX;
    spot->tileY = tileY;
    spot->visSpot = &spotvis[tileX][tileY];
    spot->flags = FL_BONUS | statinfo[type].specialFlags;
    spot->itemNumber = statinfo[type].type;
}

void WLItem::recursiveConnect(qint32 areaNumber)
{
    for (int i = 0; i < NUMAREAS; i++) {
        if (m_areaConnect[areaNumber][i] && !m_areaByPlayer[i]) {
            m_areaByPlayer[i] = true;
            recursiveConnect(i);
        }
    }
}

void WLItem::connectAreas()
{
    memset(m_areaByPlayer, 0, sizeof(m_areaByPlayer));
    m_areaByPlayer[player->areanumber] = true;
    recursiveConnect(player->areanumber);
}

void WLItem::initAreas()
{
    memset(m_areaByPlayer, 0, sizeof(m_areaByPlayer));
    if (player->areanumber < NUMAREAS)
        m_areaByPlayer[player->areanumber] = true;
}

void WLItem::initDoorList()
{
    memset(m_areaByPlayer, 0, sizeof(m_areaByPlayer));
    memset(m_areaConnect, 0, sizeof(m_areaConnect));
    m_lastDoorObj = &m_doorObjList[0];
    m_doorNum = 0;
}

void WLItem::spawnDoor(qint32 tileX, qint32 tileY, qint8 vertical, qint32 lock)
{
    quint16 *map;

    if (m_doorNum == MAXDOORS)
        Quit ("64+ doors on level!");

    m_doorPosition[m_doorNum] = 0;
    m_lastDoorObj->tilex = tileX;
    m_lastDoorObj->tiley = tileY;
    m_lastDoorObj->vertical = vertical;
    m_lastDoorObj->lock = lock;
    m_lastDoorObj->action = dr_closed;
    actorat[tileX][tileY] = (objtype *)(uintptr_t)(m_doorNum | 0x80);
    tilemap[tileX][tileY] = m_doorNum | 0x80;
    map = (quint16*)wlcache->map(0) + (tileY << mapshift) + tileX;
    if (vertical) {
        *map = *(map - 1);
        tilemap[tileX][tileY - 1] |= 0x40;
        tilemap[tileX][tileY + 1] |= 0x40;
    } else {
        *map = *(map - mapwidth);
        tilemap[tileX - 1][tileY] |= 0x40;
        tilemap[tileX + 1][tileY] |= 0x40;
    }
    m_doorNum++;
    m_lastDoorObj++;
}

void WLItem::openDoor(qint32 door)
{
    if (m_doorObjList[door].action == dr_open)
        m_doorObjList[door].ticcount = 0;
    else
        m_doorObjList[door].action = dr_opening;
}

void WLItem::closeDoor(qint32 door)
{
    qint32  tileX;
    qint32  tileY;
    qint32  area;
    objtype *check;

    tileX = m_doorObjList[door].tilex;
    tileY = m_doorObjList[door].tiley;
    if (actorat[tileX][tileY])
        return;
    if (player->tilex == tileX && player->tiley == tileY)
        return;
    if (m_doorObjList[door].vertical) {
        if ( player->tiley == tileY ) {
            if ( ((player->x + MINDIST) >> TILESHIFT) == tileX )
                return;
            if ( ((player->x - MINDIST) >> TILESHIFT) == tileX )
                return;
        }
        check = actorat[tileX - 1][tileY];
        if (ISPOINTER(check) && ((check->x + MINDIST) >> TILESHIFT) == tileX )
            return;
        check = actorat[tileX + 1][tileY];
        if (ISPOINTER(check) && ((check->x - MINDIST) >> TILESHIFT) == tileX )
            return;
    } else if (!m_doorObjList[door].vertical) {
        if (player->tilex == tileX) {
            if ( ((player->y + MINDIST) >> TILESHIFT) == tileY )
                return;
            if ( ((player->y - MINDIST) >> TILESHIFT) == tileY )
                return;
        }
        check = actorat[tileX][tileY - 1];
        if (ISPOINTER(check) && ((check->y + MINDIST) >> TILESHIFT) == tileY )
            return;
        check = actorat[tileX][tileY + 1];
        if (ISPOINTER(check) && ((check->y - MINDIST) >> TILESHIFT) == tileY )
            return;
    }
    area = *(wlcache->map(0) + (m_doorObjList[door].tiley << mapshift)
             + m_doorObjList[door].tilex) - AREATILE;
    if (m_areaByPlayer[area]) {
        PlaySoundLocTile(wlcache->findSound("CLOSEDOORSND"), m_doorObjList[door].tilex, m_doorObjList[door].tiley);
    }
    m_doorObjList[door].action = dr_closing;
    actorat[tileX][tileY] = (objtype *)(uintptr_t)(door | 0x80);
}

void WLItem::operateDoor(qint32 door)
{
    qint32 lock;

    lock = m_doorObjList[door].lock;
    if (lock >= dr_lock1 && lock <= dr_lock4) {
        if ( ! (gamestate.keys & (1 << (lock-dr_lock1) ) ) ) {
            wlaudio->playSound("NOWAYSND"); // locked
            return;
        }
    }
    switch (m_doorObjList[door].action) {
    case dr_closed:
    case dr_closing:
        openDoor(door);
        break;
    case dr_open:
    case dr_opening:
        closeDoor(door);
        break;
    }
}

void WLItem::doorOpen(qint32 door)
{
    if ( (m_doorObjList[door].ticcount += (short) tics) >= 300)
        closeDoor(door);
}

void WLItem::doorOpening(qint32 door)
{
    quint32 area1,area2;
    quint16 *map;
    qint32  position;

    position = m_doorPosition[door];
    if (!position) {
        map = (quint16*)wlcache->map(0) + (m_doorObjList[door].tiley << mapshift)
                + m_doorObjList[door].tilex;

        if (m_doorObjList[door].vertical) {
            area1 = *(map + 1);
            area2 = *(map - 1);
        } else {
            area1 = *(map - mapwidth);
            area2 = *(map + mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS) {
            m_areaConnect[area1][area2]++;
            m_areaConnect[area2][area1]++;

            if (player->areanumber < NUMAREAS)
                connectAreas();

            if (m_areaByPlayer[area1])
                PlaySoundLocTile(wlcache->findSound("OPENDOORSND"), m_doorObjList[door].tilex, m_doorObjList[door].tiley);
        }
    }
    position += tics << 10;
    if (position >= 0xffff) {
        position = 0xffff;
        m_doorObjList[door].ticcount = 0;
        m_doorObjList[door].action = dr_open;
        actorat[m_doorObjList[door].tilex][m_doorObjList[door].tiley] = 0;
    }
    m_doorPosition[door] = (quint16)position;
}

void WLItem::doorClosing(qint32 door)
{
    quint32 area1,area2;
    quint16 *map;
    qint32  position;
    qint32  tileX;
    qint32  tileY;

    tileX = m_doorObjList[door].tilex;
    tileY = m_doorObjList[door].tiley;

    if ( ((qint32)(uintptr_t)actorat[tileX][tileY] != (door | 0x80))
            || (player->tilex == tileX && player->tiley == tileY) )
    {   // something got inside the door
        openDoor(door);
        return;
    };
    position = m_doorPosition[door];
    position -= tics<<10;
    if (position <= 0) {
        position = 0;
        m_doorObjList[door].action = dr_closed;
        map = (quint16*)wlcache->map(0) + (m_doorObjList[door].tiley << mapshift) + m_doorObjList[door].tilex;
        if (m_doorObjList[door].vertical) {
            area1 = *(map + 1);
            area2 = *(map - 1);
        } else {
            area1 = *(map - mapwidth);
            area2 = *(map + mapwidth);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        if (area1 < NUMAREAS && area2 < NUMAREAS) {
            m_areaConnect[area1][area2]--;
            m_areaConnect[area2][area1]--;
            if (player->areanumber < NUMAREAS)
                connectAreas();
        }
    }
    m_doorPosition[door] = (quint16)position;
}

void WLItem::moveDoors()
{
    if (gamestate.victoryflag)
        return;

    for (qint32 door = 0; door < m_doorNum; door++) {
        switch (m_doorObjList[door].action) {
        case dr_open:
            doorOpen(door);
            break;
        case dr_opening:
            doorOpening(door);
            break;
        case dr_closing:
            doorClosing(door);
            break;
        default:
            break;
        }
    }
}

void WLItem::pushWall(qint32 checkX, qint32 checkY, qint32 dir)
{
    int oldTile, dx, dy;

    if (m_pWallState)
        return;

    oldTile = tilemap[checkX][checkY];
    if (!oldTile)
        return;

    dx = g_Dirs[dir][0];
    dy = g_Dirs[dir][1];

    if (actorat[checkX + dx][checkY + dy]) {
        wlaudio->playSound("NOWAYSND");
        return;
    }
    actorat[checkX + dx][checkY + dy] = (objtype*)(uintptr_t) (tilemap[checkX + dx][checkY + dy] = oldTile);
    gamestate.secretcount++;
    m_pWallX = checkX;
    m_pWallY = checkY;
    m_pWallDir = dir;
    m_pWallState = 1;
    m_pWallPos = 0;
    m_pWallTile = tilemap[m_pWallX][m_pWallY];
    tilemap[m_pWallX][m_pWallY] = 64;
    tilemap[m_pWallX + dx][m_pWallY + dy] = 64;
    *((quint16*)wlcache->map(1)+(m_pWallY << mapshift) + m_pWallX) = 0;
    *((quint16*)wlcache->map(0)+(m_pWallY << mapshift) + m_pWallX) =
            *((quint16*)wlcache->map(0) + (player->tiley << mapshift) + player->tilex);

    wlaudio->playSound("PUSHWALLSND");
}

void WLItem::movePWalls()
{
    qint32 oldBlock;
    qint32 oldTile;

    if (!m_pWallState)
        return;

    oldBlock = m_pWallState / 128;

    m_pWallState += (quint16)tics;

    if (m_pWallState / 128 != oldBlock) {
        // block crossed into a new block
        oldTile = m_pWallTile;
        tilemap[m_pWallX][m_pWallY] = 0;
        actorat[m_pWallX][m_pWallY] = 0;
        *((quint16*)wlcache->map(0)+(m_pWallY << mapshift) + m_pWallX) = player->areanumber + AREATILE;

        qint32 dx = g_Dirs[m_pWallDir][0];
        qint32 dy = g_Dirs[m_pWallDir][1];
        if (m_pWallState >= 256) {
            m_pWallState = 0;
            tilemap[m_pWallX + dx][m_pWallY + dy] = oldTile;
            return;
        } else {
            qint32 xl,yl,xh,yh;
            xl = (player->x - PLAYERSIZE) >> TILESHIFT;
            yl = (player->y - PLAYERSIZE) >> TILESHIFT;
            xh = (player->x + PLAYERSIZE) >> TILESHIFT;
            yh = (player->y + PLAYERSIZE) >> TILESHIFT;

            m_pWallX += dx;
            m_pWallY += dy;

            if (actorat[m_pWallX + dx][m_pWallY + dy]
                    || ((xl <= m_pWallX + dx) && (m_pWallX + dx <= xh)
                    && (yl <= m_pWallY + dy) && (m_pWallY + dy <= yh))) {
                m_pWallState = 0;
                tilemap[m_pWallX][m_pWallY] = oldTile;
                return;
            }
            actorat[m_pWallX + dx][m_pWallY + dy] = (objtype*)(uintptr_t)(tilemap[m_pWallX + dx][m_pWallY + dy] = oldTile);
            tilemap[m_pWallX + dx][m_pWallY + dy] = 64;
        }
    }
    m_pWallPos = (m_pWallState / 2)&63;
}
