#ifndef WLITEM_H
#define WLITEM_H

#include "wl_def.h"

typedef struct {
    qint16     picNum;
    wl_stat_t  type;
    quint32    specialFlags;
} statInfo_t;

class WLItem {
public:
    WLItem();
    ~WLItem();

    void initStaticList(); //game
    void spawnStatic(qint32 tileX, qint32 tileY, qint32 type); //game
    void placeItemType(qint32 itemType, qint32 tileX, qint32 tileY); //state
    void initAreas(); //agent
    void initDoorList(); //game
    void spawnDoor(qint32 tileX, qint32 tileY, qint8 vertical, qint32 lock); //game
    void openDoor(qint32 door); //act2
    void operateDoor(qint32 door); //agent
    void moveDoors(); //play
    void pushWall(qint32 checkX, qint32 checkY, qint32 dir); // agent
    void movePWalls(); //play

    quint8*  pWallDir() { return &m_pWallDir; }
    void    setpWallDir(quint8 value) { m_pWallDir = value; }
    quint8*  pWallTile() { return &m_pWallTile; }
    void    setpWallTile(quint8 value) { m_pWallTile = value; }
    quint16*  pWallX() { return &m_pWallX; }
    void    setpWallX(quint16 x) { m_pWallX = x; }
    quint16*  pWallY() { return &m_pWallY; }
    void    setpWallY(quint16 y) { m_pWallY = y; }
    quint16* pWallState() { return &m_pWallState; }
    void    setpWallState(quint16 state) { m_pWallState = state; }
    quint16* pWallPos() { return &m_pWallPos; }
    void    setpWallPos(quint16 pos) { m_pWallPos = pos; }
    qint16  doorNum() { return m_doorNum; }
    void    setDoorNum(qint16 doorNum) { m_doorNum = doorNum; }
    quint16* doorPosition(qint32 idx) { return &m_doorPosition[idx]; }
    void    setDoorPosition(qint32 idx, quint16 value) { m_doorPosition[idx] = value; }
    quint8* areaConnect() { return &m_areaConnect[0][0]; }
    qint8*   areaByPlayer(qint32 idx) { return &m_areaByPlayer[idx]; }
    void    setAreaByPlayer(qint32 idx, qint8 value) { m_areaByPlayer[idx] = value; }
    doorObj_t* doorObjList(qint32 idx) { return &m_doorObjList[idx]; }

private:
    void doorClosing(qint32 door);
    void doorOpening(qint32 door);
    void doorOpen(qint32 door);
    void closeDoor(qint32 door);
    void connectAreas();
    void recursiveConnect(qint32 areaNumber);

    qint16    m_doorNum;
    quint16   m_doorPosition[MAXDOORS];
    quint8    m_areaConnect[NUMAREAS][NUMAREAS];
    qint8     m_areaByPlayer[NUMAREAS];
    quint16   m_pWallState;
    quint16   m_pWallPos;
    quint16   m_pWallX;
    quint16   m_pWallY;
    quint8    m_pWallDir;
    quint8    m_pWallTile;

    doorObj_t*  m_lastDoorObj;
    doorObj_t   m_doorObjList[MAXDOORS];
};

#endif // WLITEM_H
