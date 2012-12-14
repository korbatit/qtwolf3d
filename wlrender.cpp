#include <string.h>
#include "wl_def.h"

#include "wlpaint.h"

#include "wlrender.h"

#include <QtCore/qglobal.h>
#include <QPainter>

extern WLPaint*  wlpaint;

extern QImage* g_Image;

extern void gsleep(int ms);

//GLOBALS
quint32 scaleFactor;
qint8	screenfaded;
quint32 bordercolor;
Color_t palette1[256];
Color_t palette2[256];
Color_t curpal[256];



WLRender::WLRender()
{

}

WLRender::~WLRender()
{

}

void WLRender::clearScreen(qint32 color)
{
    g_Image->fill(color);
}

void WLRender::setVideoMode()
{
    QVector<QRgb> table( 256 );
    for( int i = 0; i < 256; ++i ) {
        table[i] = qRgb(gamepal[i].r,gamepal[i].g,gamepal[i].b);
    }
    memcpy(curpal, gamepal, sizeof(Color_t) * 256);
    g_Image = new QImage(640, 400, QImage::Format_Indexed8);
    g_Image->fill(qRgb(0,0,0));

    scaleFactor = g_Image->width()/320;
    if((quint32)g_Image->height()/200 < scaleFactor)
        scaleFactor = g_Image->height()/200;

    pixelangle = (qint16*)malloc(g_Image->width() * sizeof(qint16));
    if (!pixelangle)
        Quit("Out of Memory at %s:%i", __FILE__, __LINE__);

    wallheight = (qint32*) malloc(g_Image->width() * sizeof(qint32));
    if (!wallheight)
        Quit("Out of Memory at %s:%i", __FILE__, __LINE__);

    g_Image->setColorTable(table);
}

void WLRender::fillPalette(qint32 red, qint32 green, qint32 blue)
{
    Color_t pal[256];
    for(int i = 0; i < 256; i++) {
        pal[i].r = red;
        pal[i].g = green;
        pal[i].b = blue;
    }
    setPalette(pal);
}

void WLRender::setPalette(Color_t* palette)
{
    memcpy(curpal, palette, sizeof(Color_t) * 256);

    QVector<QRgb> table( 256 );
    for( int i = 0; i < 256; ++i ) {
        table[i] = qRgb(palette[i].r, palette[i].g, palette[i].b);
    }
    g_Image->setColorTable(table);
    wlpaint->updateScreen();
}

void WLRender::getPalette(Color_t* palette)
{
    memcpy(palette, curpal, sizeof(Color_t) * 256);
}

void WLRender::fadeOut(qint32 start, qint32 end, qint32 red, qint32 green, qint32 blue, qint32 steps)
{
    qint32    orig;
    qint32    delta;
    Color_t*  origPtr;
    Color_t*  newPtr;

    QTime t;
    t.start();

    red = red * 255 / 63;
    green = green * 255 / 63;
    blue = blue * 255 / 63;

    gsleep(8);
    getPalette(palette1);
    memcpy(palette2, palette1, sizeof(Color_t) * 256);

    for (int i = 0; i < steps; i++) {
        origPtr = &palette1[start];
        newPtr = &palette2[start];
        for (int j = start; j <= end; j++) {
            orig = origPtr->r;
            delta = red - orig;
            newPtr->r = orig + delta * i / steps;
            orig = origPtr->g;
            delta = green - orig;
            newPtr->g = orig + delta * i / steps;
            orig = origPtr->b;
            delta = blue - orig;
            newPtr->b = orig + delta * i / steps;
            origPtr++;
            newPtr++;
        }
        gsleep(8);
        setPalette(palette2);
    }
    fillPalette(red, green, blue);
    screenfaded = true;
}

void WLRender::fadeIn(qint32 start, qint32 end, Color_t* palette, qint32 steps)
{
    gsleep(8);
    getPalette(palette1);
    memcpy(palette2, palette1, sizeof(Color_t) * 256);

    for (int i = 0; i < steps; i++) {
        for (int j = start; j <= end; j++) {
            qint32 delta;
            delta = palette[j].r-palette1[j].r;
            palette2[j].r = palette1[j].r + delta * i / steps;
            delta = palette[j].g-palette1[j].g;
            palette2[j].g = palette1[j].g + delta * i / steps;
            delta = palette[j].b-palette1[j].b;
            palette2[j].b = palette1[j].b + delta * i / steps;
        }
        gsleep(8);
        setPalette(palette2);
    }
    setPalette(palette);
    screenfaded = false;
}

void WLRender::plot(qint32 x, qint32 y, qint32 color)
{
    ((quint8*)g_Image->bits())[y + g_Image->bytesPerLine() + x] = color;
}

quint8 WLRender::getPixel(qint32 x, qint32 y)
{
    return ((quint8*)g_Image->bits())[y * g_Image->bytesPerLine() + x];
}

void WLRender::hLine(quint32 x, quint32 y, quint32 width, qint32 color)
{
    quint8* dest = ((quint8*)g_Image->bits()) + y * g_Image->bytesPerLine() + x;
    memset(dest, color, width);
}

void WLRender::vLine(qint32 x, qint32 y, qint32 height, qint32 color)
{
    quint8* dest = ((quint8*)g_Image->bits()) + y * g_Image->bytesPerLine() + x;
    while (height--) {
        *dest = color;
        dest += g_Image->bytesPerLine();
    }
}

void WLRender::barScaledCoord(qint32 scx, qint32 scy, qint32 scwidth, qint32 scheight, qint32 color)
{
    quint8* vbuf = (quint8*)(g_Image->bits()) + scy * g_Image->bytesPerLine() + scx;
    while (scheight--) {
        memset(vbuf, color, scwidth);
        vbuf += g_Image->bytesPerLine();
    }
}

void WLRender::memToLatch(quint8 *source, qint32 width, qint32 height,
                   QImage *destSurface, qint32 x, qint32 y)
{
    int pitch = destSurface->bytesPerLine();
    quint8 *dest = (quint8*)destSurface->bits() + y * pitch + x;
    for(int ysrc = 0; ysrc < height; ysrc++) {
        for(int xsrc = 0; xsrc < width; xsrc++) {
            dest[ysrc * pitch + xsrc] = source[(ysrc * (width >> 2) + (xsrc >> 2))
                    + (xsrc & 3) * (width >> 2) * height];
        }
    }
}

void WLRender::memToScreenScaledCoord(quint8 *source, qint32 width, qint32 height, qint32 destx, qint32 desty)
{
    quint8* vbuf = (quint8*)g_Image->bits();
    for(int j = 0, scj = 0; j < height; j++, scj += scaleFactor) {
        for(int i = 0, sci = 0; i < width; i++, sci += scaleFactor) {
            quint8 col = source[(j * (width >> 2) + (i >> 2)) + (i&3) * (width >> 2) * height];
            for(quint32 m = 0; m < scaleFactor; m++) {
                for(quint32 n = 0; n < scaleFactor; n++) {
                    vbuf[(scj + m + desty) * g_Image->bytesPerLine() + sci + n + destx] = col;
                }
            }
        }
    }
}

void WLRender::memToScreenScaledCoord(quint8 *source, qint32 origwidth, qint32 origheight, qint32 srcx, qint32 srcy,
                                qint32 destx, qint32 desty, qint32 width, qint32 height)
{
    quint8* vbuf = (quint8*)g_Image->bits();
    for(int j = 0, scj = 0; j < height; j++, scj += scaleFactor) {
        for(int i = 0, sci = 0; i < width; i++, sci += scaleFactor) {
            quint8 col = source[((j + srcy) * (origwidth >> 2) + ((i + srcx) >> 2))
                    + ((i + srcx)&3) * (origwidth >> 2) * origheight];
            for(quint32 m = 0; m < scaleFactor; m++) {
                for(quint32 n = 0; n < scaleFactor; n++) {
                    vbuf[(scj + m + desty) * g_Image->bytesPerLine() + sci + n + destx] = col;
                }
            }
        }
    }
}

void WLRender::latchToScreenScaledCoord(QImage *source, qint32 xsrc, qint32 ysrc,
                                 qint32 width, qint32 height, qint32 scxdest, qint32 scydest)
{
    if(scaleFactor == 1) {
        QRect srcrect(xsrc, ysrc, width, height);
        QRect destrect(scxdest, scydest, 0, 0 );
        QPainter painter(g_Image);
        painter.drawImage(destrect, *source, srcrect);

    } else {
        quint8 *src = (quint8*)source->bits();
        unsigned srcPitch = source->bytesPerLine();

        quint8 *vbuf = (quint8*)g_Image->bits();
        for(int j = 0, scj = 0; j < height; j++, scj += scaleFactor) {
            for(int i = 0, sci = 0; i < width; i++, sci += scaleFactor) {
                quint8 col = src[(ysrc + j) * srcPitch + xsrc + i];
                for(quint32 m = 0; m < scaleFactor; m++) {
                    for(quint32 n = 0; n < scaleFactor; n++) {
                        vbuf[(scydest + scj + m) * g_Image->bytesPerLine() + scxdest + sci + n] = col;
                    }
                }
            }
        }
    }
}

void WLRender::mungePic(quint8 *source, quint32 width, quint32 height)
{
    quint32 x,y,plane,size,pwidth;
    quint8 *temp, *dest, *srcline;

    size = width*height;

    if (width&3)
        Quit ("mungePic: Not divisable by 4!");

    temp=(quint8*)malloc(size);
    CHECKMALLOCRESULT(temp);
    memcpy (temp,source,size);
    dest = source;
    pwidth = width/4;

    for (plane=0;plane<4;plane++) {
        srcline = temp;
        for (y=0;y<height;y++) {
            for (x=0;x<pwidth;x++)
                *dest++ = *(srcline+x*4+plane);
            srcline+=width;
        }
    }
    free(temp);
}
