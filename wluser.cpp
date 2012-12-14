#include <string.h>

#include "wl_def.h"
#include "wlpaint.h"

#include "wluser.h"

extern WLInput* wlinput;
extern WLPaint* wlpaint;

extern QTime* g_getTicks;

extern void gsleep(int ms);

#if _MSC_VER == 1200            // Visual C++ 6
#define vsnprintf _vsnprintf
#endif

//	Global variables
quint16		PrintX,PrintY;
quint16		WindowX,WindowY,WindowW,WindowH;

//	Internal variables
#define	ConfigVersion	1

SaveGame	Games[MaxSaveGames];
HighScore	Scores[MaxScores] =
{
    {"id software-'92",10000,1,0},
    {"Adrian Carmack",10000,1,0},
    {"John Carmack",10000,1,0},
    {"Kevin Cloud",10000,1,0},
    {"Tom Hall",10000,1,0},
    {"John Romero",10000,1,0},
    {"Jay Wilbur",10000,1,0},
};

int rndindex = 0;

static quint8 rndtable[] = {
    0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
    74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
    95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
    52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
    25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
    94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
    80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
    24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
    28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
    71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
    17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
    120, 163, 236, 249 };

WLUser::WLUser()
{
    US_InitRndT(true);
}

WLUser::~WLUser()
{
}

void WLUser::US_SetPrintRoutines(const char *a, quint16* b,quint16* c)
{
    wlpaint->measurePropString(a, b, c);
    wlpaint->drawPropString(a);
}


void WLUser::US_Print(const char *sorg)
{
    char c;
    char *sstart = strdup(sorg);
    char *s = sstart;
    char *se;
    quint16 w,h;

    while (*s) {
        se = s;
        while ((c = *se)!=0 && (c != '\n'))
            se++;
        *se = '\0';

        wlpaint->measurePropString(s, &w, &h);
        px = PrintX;
        py = PrintY;
        wlpaint->drawPropString(s);

        s = se;
        if (c) {
            *se = c;
            s++;

            PrintX = WindowX;
            PrintY += h;
        } else
            PrintX += w;
    }
    free(sstart);
}

void WLUser::US_PrintUnsigned(quint32 n)
{
    char buffer[32];
    sprintf(buffer, "%lu", (long unsigned int)n);
    US_Print(buffer);
}

void WLUser::US_PrintSigned(qint32 n)
{
    char buffer[32];
    US_Print(ltoa(n,buffer,10));
}

void WLUser::USL_PrintInCenter(const char *s,Rect r)
{
    quint16	w, h, rw, rh;

    wlpaint->measurePropString(s, &w, &h);
    rw = r.lr.x - r.ul.x;
    rh = r.lr.y - r.ul.y;
    px = r.ul.x + ((rw - w) / 2);
    py = r.ul.y + ((rh - h) / 2);
    wlpaint->drawPropString(s);
}

void WLUser::US_PrintCentered(const char *s)
{
    Rect	r;

    r.ul.x = WindowX;
    r.ul.y = WindowY;
    r.lr.x = r.ul.x + WindowW;
    r.lr.y = r.ul.y + WindowH;
    USL_PrintInCenter(s,r);
}

void WLUser::US_CPrintLine(const char *s)
{
    quint16	w,h;

    wlpaint->measurePropString(s, &w, &h);

    if (w > WindowW)
        Quit("US_CPrintLine() - String exceeds width");
    px = WindowX + ((WindowW - w) / 2);
    py = PrintY;
    wlpaint->drawPropString(s);
    PrintY += h;
}

void WLUser::US_CPrint(const char *sorg)
{
    char	c;
    char *sstart = strdup(sorg);
    char *s = sstart;
    char *se;

    while (*s) {
        se = s;
        while ((c = *se)!=0 && (c != '\n'))
            se++;
        *se = '\0';

        US_CPrintLine(s);

        s = se;
        if (c) {
            *se = c;
            s++;
        }
    }
    free(sstart);
}

void WLUser::US_Printf(const char *formatStr, ...)
{
    char strbuf[256];
    va_list vlist;
    va_start(vlist, formatStr);
    int len = vsnprintf(strbuf, sizeof(strbuf), formatStr, vlist);
    va_end(vlist);
    if((quint32)len <= (quint32)-1 || (quint32)len >= sizeof(strbuf))
        strbuf[sizeof(strbuf) - 1] = 0;
    US_Print(strbuf);
}

void WLUser::US_CPrintf(const char *formatStr, ...)
{
    char strbuf[256];
    va_list vlist;
    va_start(vlist, formatStr);
    qint32 len = vsnprintf(strbuf, sizeof(strbuf), formatStr, vlist);
    va_end(vlist);
    if((quint32)len <= (quint32)-1 || (quint32)len >= sizeof(strbuf))
        strbuf[sizeof(strbuf) - 1] = 0;
    US_CPrint(strbuf);
}

void WLUser::US_ClearWindow()
{
    wlpaint->bar(WindowX, WindowY, WindowW, WindowH, WHITE);
    PrintX = WindowX;
    PrintY = WindowY;
}

void WLUser::US_DrawWindow(quint16 x,quint16 y,quint16 w,quint16 h)
{
    quint16	i, sx, sy, sw, sh;

    WindowX = x * 8;
    WindowY = y * 8;
    WindowW = w * 8;
    WindowH = h * 8;
    PrintX = WindowX;
    PrintY = WindowY;
    sx = (x - 1) * 8;
    sy = (y - 1) * 8;
    sw = (w + 1) * 8;
    sh = (h + 1) * 8;
    US_ClearWindow();
    wlpaint->drawTile8(sx, sy, 0);
    wlpaint->drawTile8(sx, sy + sh, 5);
    for (i = sx + 8;i <= sx + sw - 8;i += 8) {
        wlpaint->drawTile8(i, sy, 1);
        wlpaint->drawTile8(i, sy + sh, 6);
    }
    wlpaint->drawTile8(i, sy, 2);
    wlpaint->drawTile8(i, sy + sh, 7);
    for (i = sy + 8;i <= sy + sh - 8;i += 8) {
        wlpaint->drawTile8(sx, i, 3);
        wlpaint->drawTile8(sx + sw, i, 4);
    }
}

void WLUser::US_CenterWindow(quint16 w,quint16 h)
{
    US_DrawWindow(((MaxX / 8) - w) / 2,((MaxY / 8) - h) / 2,w,h);
}

void WLUser::US_SaveWindow(WindowRec *win)
{
    win->x = WindowX;
    win->y = WindowY;
    win->w = WindowW;
    win->h = WindowH;
    win->px = PrintX;
    win->py = PrintY;
}

void WLUser::US_RestoreWindow(WindowRec *win)
{
    WindowX = win->x;
    WindowY = win->y;
    WindowW = win->w;
    WindowH = win->h;
    PrintX = win->px;
    PrintY = win->py;
}

void WLUser::USL_XORICursor(int x,int y,const char *s,quint16 cursor)
{
    static	qint8	status;
    char	buf[MaxString];
    int		temp;
    quint16	w,h;

    strcpy(buf,s);
    buf[cursor] = '\0';
    wlpaint->measurePropString(buf, &w, &h);
    px = x + w - 1;
    py = y;
    if (status^=1)
        wlpaint->drawPropString("\x80");
    else {
        temp = fontcolor;
        fontcolor = backcolor;
        wlpaint->drawPropString("\x80");
        fontcolor = temp;
    }
}

char WLUser::USL_RotateChar(char ch, int dir)
{
    static const char charSet[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ.,-!?0123456789";
    const int numChars = sizeof(charSet) / sizeof(char) - 1;
    int i;
    for(i = 0; i < numChars; i++){
        if(ch == charSet[i]) break;
    }
    if(i == numChars) i = 0;
    i += dir;
    if(i < 0) i = numChars - 1;
    else if(i >= numChars) i = 0;
    return charSet[i];
}

qint8 WLUser::US_LineInput(qint32 x, qint32 y, char *buf,const char *def,qint8 escok,
                   qint32 maxchars, qint32 maxwidth)
{
    qint8		redraw, cursorvis, cursormoved, done, result=0, checkkey;
    ScanCode	sc;
    char		c;
    char		s[MaxString],olds[MaxString];
    qint32      cursor,len = 0;
    quint16		i, w, h, temp;
    quint32	curtime, lasttime, lastdirtime, lastbuttontime, lastdirmovetime;
    ControlInfo_t ci;
    Direction   lastdir = dir_None;

    if (def)
        strcpy(s,def);
    else
        *s = '\0';

    *olds = '\0';
    cursor = (qint32) strlen(s);
    cursormoved = redraw = true;

    cursorvis = done = false;
    lasttime = lastdirtime = lastdirmovetime = g_getTicks->elapsed()*7/100; //GetTicks();
    lastbuttontime = lasttime + g_tickBase / 4;	// 250 ms => first button press accepted after 500 ms
    wlinput->setLastScan(sc_None);
    wlinput->setLastAscii(key_None);

    while (!done) {
        ReadAnyControl(&ci);
        if (cursorvis)
            USL_XORICursor(x,y,s,cursor);

        sc = wlinput->lastScan();
        wlinput->setLastScan(sc_None);
        c = wlinput->lastAscii();
        wlinput->setLastAscii(key_None);
        checkkey = true;
        curtime = g_getTicks->elapsed()*7/100; //GetTicks();

        // After each direction change accept the next change after 250 ms and then everz 125 ms
        if(ci.dir != lastdir || (curtime - lastdirtime > g_tickBase / 4 && curtime - lastdirmovetime > g_tickBase / 8)) {
            if(ci.dir != lastdir) {
                lastdir = ci.dir;
                lastdirtime = curtime;
            }
            lastdirmovetime = curtime;

            switch(ci.dir) {
            case dir_West:
                if(cursor) {
                    // Remove trailing whitespace if cursor is at end of string
                    if(s[cursor] == ' ' && s[cursor + 1] == 0)
                        s[cursor] = 0;
                    cursor--;
                }
                cursormoved = true;
                checkkey = false;
                break;
            case dir_East:
                if(cursor >= MaxString - 1) break;
                if(!s[cursor]) {
                    wlpaint->measurePropString(s, &w, &h);
                    if(len >= maxchars || (maxwidth && w >= maxwidth)) break;
                    s[cursor] = ' ';
                    s[cursor + 1] = 0;
                }
                cursor++;
                cursormoved = true;
                checkkey = false;
                break;
            case dir_North:
                if(!s[cursor])
                {
                    wlpaint->measurePropString(s, &w, &h);
                    if(len >= maxchars || (maxwidth && w >= maxwidth)) break;
                    s[cursor + 1] = 0;
                }
                s[cursor] = USL_RotateChar(s[cursor], 1);
                redraw = true;
                checkkey = false;
                break;
            case dir_South:
                if(!s[cursor]) {
                    wlpaint->measurePropString(s, &w, &h);
                    if(len >= maxchars || (maxwidth && w >= maxwidth)) break;
                    s[cursor + 1] = 0;
                }
                s[cursor] = USL_RotateChar(s[cursor], -1);
                redraw = true;
                checkkey = false;
                break;
            default:
                break;
            }
        }

        if((int)(curtime - lastbuttontime) > g_tickBase / 4) {
            if(ci.button0) {
                strcpy(buf,s);
                done = true;
                result = true;
                checkkey = false;
            }
            if(ci.button1 && escok) {
                done = true;
                result = false;
                checkkey = false;
            }
            if(ci.button2) {
                lastbuttontime = curtime;
                if(cursor) {
                    strcpy(s + cursor - 1,s + cursor);
                    cursor--;
                    redraw = true;
                }
                cursormoved = true;
                checkkey = false;
            }
        }
        if(checkkey) {
            switch (sc) {
            case sc_LeftArrow:
                if (cursor)
                    cursor--;
                c = key_None;
                cursormoved = true;
                break;
            case sc_RightArrow:
                if (s[cursor])
                    cursor++;
                c = key_None;
                cursormoved = true;
                break;
            case sc_Home:
                cursor = 0;
                c = key_None;
                cursormoved = true;
                break;
            case sc_End:
                cursor = (int) strlen(s);
                c = key_None;
                cursormoved = true;
                break;
            case sc_Enter:
            case sc_Return:
                strcpy(buf,s);
                done = true;
                result = true;
                c = key_None;
                break;
            case sc_Escape:
                if (escok) {
                    done = true;
                    result = false;
                }
                c = key_None;
                break;
            case sc_BackSpace:
                if (cursor) {
                    strcpy(s + cursor - 1,s + cursor);
                    cursor--;
                    redraw = true;
                }
                c = key_None;
                cursormoved = true;
                break;
            case sc_Delete:
                if (s[cursor]) {
                    strcpy(s + cursor,s + cursor + 1);
                    redraw = true;
                }
                c = key_None;
                cursormoved = true;
                break;
            case sc_UpArrow:
            case sc_DownArrow:
            case sc_PgUp:
            case sc_PgDn:
                c = key_None;
                break;
            }

            if (c) {
                len = (int) strlen(s);
                wlpaint->measurePropString(s, &w, &h);

                if(isprint(c) && (len < MaxString - 1) && ((!maxchars) || (len < maxchars))
                        && ((!maxwidth) || (w < maxwidth))) {
                    for (i = len + 1;i > cursor;i--)
                        s[i] = s[i - 1];
                    s[cursor++] = c;
                    redraw = true;
                }
            }
        }
        if (redraw) {
            px = x;
            py = y;
            temp = fontcolor;
            fontcolor = backcolor;
            wlpaint->drawPropString(olds);
            fontcolor = (quint8)temp;
            strcpy(olds,s);
            px = x;
            py = y;
            wlpaint->drawPropString(s);
            redraw = false;
        }
        if (cursormoved) {
            cursorvis = false;
            lasttime = curtime - g_tickBase;

            cursormoved = false;
        }
        if (curtime - lasttime > g_tickBase / 2) {
            lasttime = curtime;
            cursorvis ^= true;
        } else gsleep(20);
        if (cursorvis)
            USL_XORICursor(x,y,s,cursor);
        wlpaint->updateScreen();
    }

    if (cursorvis)
        USL_XORICursor(x,y,s,cursor);
    if (!result) {
        px = x;
        py = y;
        wlpaint->drawPropString(olds);
    }
    wlpaint->updateScreen();
    wlinput->clearKeysDown();
    return(result);
}

void WLUser::US_InitRndT(qint32 randomize)
{
    if(randomize)
        rndindex = (g_getTicks->elapsed() >> 4) & 0xff;
    else
        rndindex = 0;
}

int WLUser::US_RndT()
{
    rndindex = (rndindex+1)&0xff;
    return rndtable[rndindex];
}
