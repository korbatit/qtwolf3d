#ifndef WLSTATE_H
#define WLSTATE_H

#include "wl_def.h"

static const dirType_t opposite[9] = {
    west,
    southWest,
    south,
    southEast,
    east,
    northEast,
    north,
    northWest,
    noDir
};

static const dirType_t diagonal[9][9] =
{
    /* east */  {noDir, noDir, northEast, noDir, noDir, noDir, southEast, noDir, noDir},
                {noDir, noDir, noDir    , noDir, noDir, noDir, noDir    , noDir, noDir},

    /* north */ {northEast, noDir, noDir, noDir, northWest, noDir, noDir, noDir, noDir},
                {noDir    , noDir, noDir, noDir, noDir    , noDir, noDir, noDir, noDir},

    /* west */  {noDir, noDir, northWest, noDir, noDir, noDir, southWest, noDir, noDir},
                {noDir, noDir, noDir    , noDir, noDir, noDir, noDir    , noDir, noDir},

    /* south */ {southEast, noDir, noDir, noDir, southWest, noDir, noDir, noDir, noDir},
                {noDir    , noDir, noDir, noDir, noDir    , noDir, noDir, noDir, noDir},

    {noDir, noDir, noDir, noDir, noDir, noDir, noDir, noDir, noDir}
};

class WLState {
public:
    WLState();
    ~WLState();

    void  spawnNewObj(quint32 tileX, quint32 tileY, statetype *state); //act2
    void  newState(objtype *ob, statetype *state); //act2
    bool  tryWalk(objtype *ob); //act2
    void  moveObj(objtype *ob, qint32 move); //act2
    void  damageActor(objtype *ob, unsigned damage); //agent
    qint8 checkLine(objtype *ob); //act2,agent
    void  selectDodgeDir(objtype *ob); // act2
    void  selectChaseDir(objtype *ob); //act2
    void  selectRunDir(objtype *ob); //act2
    qint8 sightPlayer(objtype *ob); //act2

private:
    void  killActor(objtype *ob);
    bool  checkSight(objtype *ob);
    void  firstSighting(objtype *ob);
    void  dropItem(wl_stat_t itemtype, qint32 tileX, qint32 tileY);
    bool  checkSide(objtype* ob, qint32 x, qint32 y);
    bool  checkDiag( qint32 x, qint32 y);
};

#endif // WLSTATE_H
