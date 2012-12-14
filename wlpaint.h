#ifndef WLPAINT_H
#define WLPAINT_H

#define WHITE			15
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			15
#define F_BLACK			0
#define F_FIRSTCOLOR	1
#define F_SECONDCOLOR	12

#define MAXSHIFTS	1

#include <QImage>

#include "wl_def.h"

typedef struct {
    int16_t width,height;
} pictabletype;


typedef struct {
    int16_t height;
    int16_t location[256];
    int8_t width[256];
} fontstruct;

extern	pictabletype	*pictable;
extern	pictabletype	*picmtable;

extern  quint8            fontcolor,backcolor;
extern	int             fontnumber;
extern	int             px,py;

extern	QImage* latchpics[NUMLATCHPICS];

#define SETFONTCOLOR(f,b) fontcolor=f;backcolor=b;
#define NUMLATCHPICS	100

class WLPaint {
public:
    WLPaint();
    ~WLPaint();

    void measureString(const char *string, quint16 *width, quint16 *height, fontstruct *font);
    void drawPropString(const char *string);
    void drawTile8(qint32 x, qint32 y, qint32 tile);
    void drawTile8M(qint32 x, qint32 y, qint32 tile);
    void drawTile16(qint32 x, qint32 y, qint32 tile);
    void drawTile16M(qint32 x, qint32 y, qint32 tile);
    void drawPic(qint32 x, qint32 y, qint32 chunknum);
    void drawPicScaledCoord (qint32 x, qint32 y, qint32 chunknum);
    void drawMPic(qint32 x, qint32 y, qint32 chunknum);
    void bar(qint32 x, qint32 y, qint32 width, qint32 height, qint32 color);
    void plot(qint32 x, qint32 y, qint32 color);
    void hLine(qint32 x1, qint32 x2, qint32 y, qint32 color); //menu
    void vLine(qint32 y1, qint32 y2, qint32 x, qint32 color); //menu
    void updateScreen(); //draw,play
    void measurePropString(const char *string, quint16 *width, quint16 *height); //user,inter,text
    void latchDrawPic(quint32 x, quint32 y, quint32 picnum); //game
    void latchDrawPicScaledCoord(quint32 scx, quint32 scy, quint32 picnum); //agent, inter
    void loadLatchMem(); // main,game

private:
};

#endif
