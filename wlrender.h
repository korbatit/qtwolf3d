#ifndef WLRENDER_H
#define WLRENDER_H

#include <QImage>

typedef struct{
  quint8 r;
  quint8 g;
  quint8 b;
} Color_t;

void Quit (const char *error,...);

extern quint32 scaleFactor;
extern qint8   screenfaded;
extern quint32 bordercolor;
extern Color_t gamepal[256];

class WLRender {
public:
    WLRender();
    ~WLRender();

    void setVideoMode();
    void setPalette(Color_t* palette);
    void fadeOut(qint32 start, qint32 end, qint32 red, qint32 green, qint32 blue, qint32 steps);
    void fadeIn(qint32 start, qint32 end, Color_t* palette, qint32 steps);
    quint8 getPixel(qint32 x, qint32 y);
    void plot(qint32 x, qint32 y, qint32 color);
    void hLine(quint32 x, quint32 y, quint32 width, qint32 color);
    void vLine(qint32 x, qint32 y, qint32 height, qint32 color);
    void barScaledCoord(qint32 scx, qint32 scy, qint32 scwidth, qint32 scheight, qint32 color);
    void bar(qint32 x, qint32 y, qint32 width, qint32 height, qint32 color)
    {
        barScaledCoord(scaleFactor * x, scaleFactor * y, scaleFactor * width, scaleFactor * height, color);
    }
    void clearScreen(qint32 color);
    void mungePic(quint8 *source, quint32 width, quint32 height);
    void memToLatch(quint8 *source, qint32 width, qint32 height, QImage *destSurface, qint32 x, qint32 y);
    void memToScreenScaledCoord(quint8 *source, qint32 width, qint32 height, qint32 scx, qint32 scy);
    void memToScreenScaledCoord(quint8 *source, qint32 origwidth, qint32 origheight, qint32 srcx, qint32 srcy,
                                     qint32 destx, qint32 desty, qint32 width, qint32 height);
    void memToScreen(quint8 *source, qint32 width, qint32 height, qint32 x, qint32 y)
    {
        memToScreenScaledCoord(source, width, height, scaleFactor * x, scaleFactor * y);
    }
    void latchToScreenScaledCoord(QImage *source, qint32 xsrc, qint32 ysrc,
                                      qint32 width, qint32 height, qint32 scxdest, qint32 scydest);

    void latchToScreen(QImage *source, qint32 xsrc, qint32 ysrc, qint32 width, qint32 height, qint32 xdest, qint32 ydest)
    {
        latchToScreenScaledCoord(source, xsrc, ysrc, width, height, scaleFactor * xdest, scaleFactor * ydest);
    }
    void latchToScreenScaledCoord(QImage *source, qint32 scx, qint32 scy)
    {
        latchToScreenScaledCoord(source, 0, 0, source->width(), source->height(), scx, scy);
    }
    void latchToScreen(QImage *source, qint32 x, qint32 y)
    {
        latchToScreenScaledCoord(source, 0, 0, source->width(), source->height(), scaleFactor * x, scaleFactor * y);
    }

private:
    void fillPalette(qint32 red, qint32 green, qint32 blue);
    void getPalette(Color_t* palette);
};
#endif
