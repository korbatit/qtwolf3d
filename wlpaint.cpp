#include "wl_def.h"

#include "wlpaint.h"

#include "wlcache.h"
#include "wlrender.h"
#include "main.h"

extern WLCache*  wlcache;
extern WLRender* wlrender;
extern GameEngine* engine;

extern QTime*  g_getTicks;
extern QImage* g_Image;

extern void gsleep(int ms);

inline void Delay(int wolfticks) {
    if(wolfticks > 0)
        gsleep(wolfticks * 100 / 7);
}

pictabletype*   pictable;
QImage*         latchpics[NUMLATCHPICS];

int	    px,py;
quint8	fontcolor,backcolor;
int	    fontnumber;

// XOR masks for the pseudo-random number sequence starting with n=17 bits
static const quint32 rndmasks[] = {
    // n    XNOR from (starting at 1, not 0 as usual)
    0x00012000,     // 17   17,14
    0x00020400,     // 18   18,11
    0x00040023,     // 19   19,6,2,1
    0x00090000,     // 20   20,17
    0x00140000,     // 21   21,19
    0x00300000,     // 22   22,21
    0x00420000,     // 23   23,18
    0x00e10000,     // 24   24,23,22,17
    0x01200000,     // 25   25,22      (this is enough for 8191x4095)
};

static unsigned int rndbits_y;
static unsigned int rndmask;

extern Color_t curpal[256];

// Returns the number of bits needed to represent the given value
static int log2_ceil(quint32 x)
{
    int n = 0;
    quint32 v = 1;
    while(v < x) {
        n++;
        v <<= 1;
    }
    return n;
}

WLPaint::WLPaint()
{
    int rndbits_x = log2_ceil(g_Image->width());
    rndbits_y = log2_ceil(g_Image->height());

    int rndbits = rndbits_x + rndbits_y;
    if(rndbits < 17)
        rndbits = 17;       // no problem, just a bit slower
    else if(rndbits > 25)
        rndbits = 25;       // fizzle fade will not fill whole screen

    rndmask = rndmasks[rndbits - 17];
}

WLPaint::~WLPaint()
{

}

void WLPaint::drawPropString(const char* string)
{
    fontstruct  *font;
    int		    width, step, height;
    quint8	    *source, *dest;
    quint8	    ch;

    // Draw Text!

    quint8 *vbuf = g_Image->bits();

    font = (fontstruct*) wlcache->graphic(STARTFONT+fontnumber);
    height = font->height;
    dest = vbuf + scaleFactor * (py * g_Image->bytesPerLine() + px);

    while ((ch = (quint8)*string++)!=0) {
        width = step = font->width[ch];
        source = ((quint8*)font)+font->location[ch];
        while (width--) {
            for(int i=0;i<height;i++) {
                if(source[i*step]) {
                    for(unsigned sy=0; sy<scaleFactor; sy++)
                        for(unsigned sx=0; sx<scaleFactor; sx++)
                            dest[(scaleFactor*i+sy)*g_Image->bytesPerLine()+sx]=fontcolor;
                }
            }
            source++;
            px++;
            dest+=scaleFactor;
        }
    }
}

void WLPaint::measureString(const char *string, quint16 *width, quint16 *height, fontstruct *font)
{
    *height = font->height;
    for (*width = 0;*string;string++)
        *width += font->width[*((quint8*)string)];	// proportional width
}

void WLPaint::measurePropString(const char *string, quint16 *width, quint16 *height)
{
    measureString(string, width, height, (fontstruct *)wlcache->graphic(STARTFONT + fontnumber));
}

void WLPaint::updateScreen()
{
    engine->setImage(g_Image);
}


void WLPaint::drawTile8(qint32 x, qint32 y, qint32 tile)
{
    wlrender->latchToScreen(latchpics[0], (tile&7)*8, (tile>>3)*8*64, 8, 8, x, y);
}

void WLPaint::drawTile8M(qint32 x, qint32 y, qint32 tile)
{
    wlrender->memToScreen(((quint8*)wlcache->graphic(wlcache->find("ORDERSCREEN"))) + tile * 64, 8, 8, x, y);
}

void WLPaint::drawPic(qint32 x, qint32 y, qint32 chunknum)
{
    qint32	picnum = chunknum - STARTPICS;
    quint32 width,height;

    x &= ~7;

    width = pictable[picnum].width;
    height = pictable[picnum].height;

    wlrender->memToScreen((quint8*)wlcache->graphic(chunknum), width, height, x, y);
}

void WLPaint::drawPicScaledCoord(qint32 scx, qint32 scy, qint32 chunknum)
{
    qint32	picnum = chunknum - STARTPICS;
    quint32 width,height;

    width = pictable[picnum].width;
    height = pictable[picnum].height;

    wlrender->memToScreenScaledCoord((quint8*)wlcache->graphic(chunknum), width, height, scx, scy);
}


void WLPaint::bar(qint32 x, qint32 y, qint32 width, qint32 height, qint32 color)
{
    wlrender->bar(x, y, width, height, color);
}

void WLPaint::plot(qint32 x, qint32 y, qint32 color)
{
    if(scaleFactor == 1)
        wlrender->plot(x, y, color);
    else
        wlrender->bar(x, y, 1, 1, color);
}

void WLPaint::hLine(qint32 x1, qint32 x2, qint32 y, qint32 color)
{
    if(scaleFactor == 1)
        wlrender->hLine(x1, y, x2 - x1 + 1, color);
    else
        wlrender->bar(x1, y, x2 - x1 + 1, 1, color);
}

void WLPaint::vLine(qint32 y1, qint32 y2, qint32 x, qint32 color)
{
    if(scaleFactor == 1)
        wlrender->vLine(x, y1, y2 - y1 + 1, color);
    else
        wlrender->bar(x, y1, 1, y2 - y1 + 1, color);
}

void WLPaint::latchDrawPic(quint32 x, quint32 y, quint32 picnum)
{
    wlrender->latchToScreen(latchpics[2 + picnum - wlcache->find("KNIFEPIC")], x * 8, y);
}

void WLPaint::latchDrawPicScaledCoord(quint32 scx, quint32 scy, quint32 picnum)
{
    wlrender->latchToScreenScaledCoord(latchpics[2 + picnum - wlcache->find("KNIFEPIC")], scx * 8, scy);
}

void WLPaint::loadLatchMem()
{
    int	i,width,height,start,end;
    quint8 *src;
    QImage *surf;

    surf = new QImage(8*8, ((NUMTILE8 + 7) / 8) * 8, QImage::Format_Indexed8);
    if(surf == NULL){
        Quit("Unable to create surface for tiles!");
    }
    QVector<QRgb> table( 256 );
    for( int i = 0; i < 256; ++i ) {
        table[i] = qRgb(gamepal[i].r,gamepal[i].g,gamepal[i].b);
    }
    surf->setColorTable(table);

    latchpics[0] = surf;
    wlcache->cacheGraphic(wlcache->find("TILE8"));
    src = (quint8*)wlcache->graphic(wlcache->find("TILE8"));

    for (i=0;i<NUMTILE8;i++)
        src += 64;
    //wlcache->uncacheGraphic(STARTTILE8);

    start = wlcache->find("KNIFEPIC");
    end = wlcache->find("GETPSYCHEDPIC");

    for (i=start;i<=end;i++) {
        width = pictable[i-STARTPICS].width;
        height = pictable[i-STARTPICS].height;
        surf = new QImage(width, height, QImage::Format_Indexed8);
        if(surf == NULL) {
            Quit("Unable to create surface for picture!");
        }
        QVector<QRgb> table( 256 );
        for( int j = 0; j < 256; ++j ) {
            table[j] = qRgb(gamepal[j].r,gamepal[j].g,gamepal[j].b);
        }
        surf->setColorTable(table);

        latchpics[2+i-start] = surf;
        wlcache->cacheGraphic(i);
        wlrender->memToLatch((quint8*)wlcache->graphic(i), width, height, surf, 0, 0);
        wlcache->uncacheGraphic(i);
    }
}




