#include "wl_def.h"

#include "wltext.h"

#include "wlcache.h"
#include "wlinput.h"
#include "wlaudio.h"
#include "wlrender.h"
#include "wlpaint.h"

#include <QDebug>

extern WLCache*  wlcache;
extern WLInput*  wlinput;
extern WLAudio*  wlaudio;
extern WLRender* wlrender;
extern WLPaint*  wlpaint;

extern void gsleep(int ms);

char endFilename[13]  = "ENDART1.";


WLText::WLText():
    m_pageNum(0), m_numPages(0), m_text(0)
{
}

WLText::~WLText()
{
}

void WLText::ripToEOL()
{
    while (*m_text++ != '\n');
}

int  WLText::parseNumber()
{
    char  ch;
    char  num[80];
    char *numPtr;

    ch = *m_text;
    while (ch < '0' || ch >'9')
        ch = *++m_text;

    numPtr = num;
    do {
        *numPtr++ = ch;
        ch = *++m_text;
    } while (ch >= '0' && ch <= '9');
    *numPtr = 0;

    return atoi(num);
}

void WLText::parsePicCommand()
{
    m_picY   = parseNumber();
    m_picX   = parseNumber();
    m_picNum = parseNumber();
    ripToEOL();
}

void WLText::parseTimedCommand()
{
    m_picY     = parseNumber();
    m_picX     = parseNumber();
    m_picNum   = parseNumber();
    m_picDelay = parseNumber();
    ripToEOL();
}

void WLText::timedPicCommand()
{
    parseTimedCommand();
    wlpaint->updateScreen();
    TicDelay(m_picDelay);
    wlpaint->drawPic(m_picX&~7, m_picY, m_picNum);
}

void WLText::handleCommand()
{
    int     i, margin, top, bottom;
    int     picWidth, picHeight, picMid;

    switch (toupper(*++m_text)) {
    case 'B':
        m_picY = parseNumber();
        m_picX = parseNumber();
        picWidth = parseNumber();
        picHeight = parseNumber();
        wlpaint->bar(m_picX, m_picY, picWidth, picHeight, BACKCOLOR);
        ripToEOL();
        break;
    case ';':
        ripToEOL();
        break;
    case 'P':
    case 'E':
        m_layoutDone = true;
        m_text--;
        break;

    case 'C':
        i = toupper(*++m_text);
        if (i >= '0' && i <= '9')
            fontcolor = i - '0';
        else if (i >= 'A' && i <= 'F')
            fontcolor = i - 'A' + 10;

        fontcolor *= 16;
        i = toupper(*++m_text);
        if (i >= '0' && i <= '9')
            fontcolor += i - '0';
        else if (i >= 'A' && i <= 'F')
            fontcolor += i - 'A' + 10;
        m_text++;
        break;

    case '>':
        px = 160;
        m_text++;
        break;

    case 'L':
        py = parseNumber();
        m_rowOn = (py - TOPMARGIN) / FONTHEIGHT;
        py = TOPMARGIN + m_rowOn * FONTHEIGHT;
        px = parseNumber();
        while (*m_text++ != '\n')
            ;
        break;

    case 'T':
        timedPicCommand ();
        break;

    case 'G':
        parsePicCommand ();
        wlpaint->drawPic(m_picX&~7, m_picY, m_picNum);
        picWidth = pictable[m_picNum - STARTPICS].width;
        picHeight = pictable[m_picNum - STARTPICS].height;

        picMid = m_picX + picWidth/2;
        if (picMid > SCREENMID)
            margin = m_picX - PICMARGIN;
        else
            margin = m_picX + picWidth + PICMARGIN;

        top = (m_picY - TOPMARGIN) / FONTHEIGHT;
        if (top < 0)
            top = 0;
        bottom = (m_picY + picHeight - TOPMARGIN) / FONTHEIGHT;
        if (bottom >= TEXTROWS)
            bottom = TEXTROWS-1;

        for (i = top; i <= bottom; i++)
            if (picMid > SCREENMID)
                m_rightMargin[i] = margin;
            else
                m_leftMargin[i] = margin;

        if (px < (qint32)m_leftMargin[m_rowOn])
            px = m_leftMargin[m_rowOn];
        break;
    }
}

void WLText::newLine()
{
    char    ch;

    if (++m_rowOn == TEXTROWS) {
        m_layoutDone = true;
        do {
            if (*m_text == '^') {
                ch = toupper(*(m_text+1));
                if (ch == 'E' || ch == 'P') {
                    m_layoutDone = true;
                    return;
                }
            }
            m_text++;
        } while (1);
    }
    px = m_leftMargin[m_rowOn];
    py+= FONTHEIGHT;
}

void WLText::handleCtrls()
{
    char    ch;

    ch = *m_text++;
    if (ch == '\n') {
        newLine();
        return;
    }
}

void WLText::handleWord()
{
    char     wWord[WORDLIMIT];
    qint32   wordIndex;
    quint16  wWidth,wHeight,newPos;

    wWord[0] = *m_text++;
    wordIndex = 1;
    while (*m_text > 32) {
        wWord[wordIndex] = *m_text++;
        if (++wordIndex == WORDLIMIT)
            Quit("pageLayout: Word limit exceeded");
    }
    wWord[wordIndex] = 0;
    wlpaint->measurePropString(wWord, &wWidth, &wHeight);

    while (px + wWidth > (qint32)m_rightMargin[m_rowOn]) {
        newLine();
        if (m_layoutDone) return;
    }
    newPos = px + wWidth;
    wlpaint->drawPropString(wWord);
    px = newPos;

    while (*m_text == ' ') {
        px += SPACEWIDTH;
        m_text++;
    }
}

void WLText::pageLayout(qint8 showNumber)
{
    qint32  oldFontColor;
    char    ch;

    oldFontColor = fontcolor;
    fontcolor = 0;

    wlpaint->bar(0, 0, 320, 200, BACKCOLOR);
    wlpaint->drawPic(0, 0, wlcache->find("H_TOPWINDOWPIC"));
    wlpaint->drawPic(0, 8, wlcache->find("H_LEFTWINDOWPIC"));
    wlpaint->drawPic(312, 8, wlcache->find("H_RIGHTWINDOWPIC"));
    wlpaint->drawPic(8, 176, wlcache->find("H_BOTTOMINFOPIC"));

    for (int i = 0; i < TEXTROWS; i++) {
        m_leftMargin[i] = LEFTMARGIN;
        m_rightMargin[i] = SCREENPIXWIDTH-RIGHTMARGIN;
    }
    px = LEFTMARGIN;
    py = TOPMARGIN;
    m_rowOn = 0;
    m_layoutDone = false;
    while (*m_text <= 32) m_text++;

    if (*m_text != '^' || toupper(*++m_text) != 'P')
        Quit("pageLayout: Text not headed with ^P");

    while (*m_text++ != '\n');
    do {
        ch = *m_text;
        if (ch == '^') handleCommand();
        else if (ch == 9) {
            px = (px+8)&0xf8;
            m_text++;
        } else if (ch <= 32) handleCtrls();
        else handleWord();
    } while (!m_layoutDone);

    m_pageNum++;

    if (showNumber) {
        sprintf(str, "pg %d of %d", m_pageNum, m_numPages);
        px = 213;
        py = 183;
        fontcolor = 0x4f;
        wlpaint->drawPropString(str);
    }
    fontcolor = oldFontColor;
}

void WLText::backPage()
{
    m_pageNum--;
    do {
        m_text--;
        if (*m_text == '^' && toupper(*(m_text+1)) == 'P') return;
    } while (1);
}

void WLText::cacheLayoutGraphics()
{
    char    *bombPoint, *textStart;
    char    ch;

    textStart  = m_text;
    bombPoint  = m_text + 30000;
    m_numPages = m_pageNum = 0;

    do {
        if (*m_text == '^') {
            ch = toupper(*++m_text);
            if (ch == 'P') m_numPages++;
            if (ch == 'E') {
                wlcache->cacheGraphic(wlcache->find("H_TOPWINDOWPIC"));
                wlcache->cacheGraphic(wlcache->find("H_LEFTWINDOWPIC"));
                wlcache->cacheGraphic(wlcache->find("H_RIGHTWINDOWPIC"));
                wlcache->cacheGraphic(wlcache->find("H_BOTTOMINFOPIC"));
                m_text = textStart;
                return;
            }
            if (ch == 'G') {
                parsePicCommand();
                wlcache->cacheGraphic(m_picNum);
            }
            if (ch == 'T') {
                parseTimedCommand();
                wlcache->cacheGraphic(m_picNum);
            }
        }
        else
            m_text++;

    } while (m_text < bombPoint);

    Quit("cacheLayoutGraphics: No ^E to terminate file!");
}

void WLText::showArticle(char *article)
{
    quint32   oldFontNumber;
    qint8     newPage,firstPage;
    ControlInfo_t ci;

    m_text = article;
    oldFontNumber = fontnumber;
    fontnumber = 0;
    wlcache->cacheGraphic(STARTFONT);
    wlpaint->bar(0, 0, 320, 200, BACKCOLOR);
    cacheLayoutGraphics();
    newPage = true;
    firstPage = true;

    do {
        if (newPage) {
            newPage = false;
            pageLayout(true);
            wlpaint->updateScreen();
            if (firstPage) {
                wlrender->fadeIn(0, 255, gamepal, 10);
                firstPage = false;
            }
        }
        gsleep(5);
        wlinput->setLastScan(0);
        ReadAnyControl(&ci);
        Direction dir = ci.dir;
        switch(dir) {
        case dir_North:
        case dir_South:
            break;

        default:
            if(ci.button0) dir = dir_South;
            switch(wlinput->lastScan()) {
            case sc_UpArrow:
            case sc_PgUp:
            case sc_LeftArrow:
                dir = dir_North;
                break;

            case sc_Enter:
            case sc_DownArrow:
            case sc_PgDn:
            case sc_RightArrow:
                dir = dir_South;
                break;
            }
            break;
        }

        switch(dir) {
        case dir_North:
        case dir_West:
            if (m_pageNum > 1) {
                backPage();
                backPage();
                newPage = true;
            }
            TicDelay(20);
            break;

        case dir_South:
        case dir_East:
            if (m_pageNum < m_numPages) {
                newPage = true;
            }
            TicDelay(20);
            break;
        default:
            break;
        }
    } while (wlinput->lastScan() != sc_Escape && !ci.button1);

    wlinput->clearKeysDown();
    fontnumber = oldFontNumber;
}

void WLText::helpScreens()
{
    char    *text;

    if (g_Extension.toLower().startsWith("wl")) {
        wlcache->cacheGraphic(wlcache->find("T_HELPART"));
        text = wlcache->graphic(wlcache->find("T_HELPART"));
    } else {
        //wlcache->cacheGraphic(wlcache->find("T_ENDART1"));
        //text = wlcache->graphic(wlcache->find("T_ENDART1"));
        return;
    }
    showArticle(text);
    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    FreeMusic();
}

void WLText::endText()
{
    char    *text;

    wlaudio->stopDigitized();

    wlcache->cacheGraphic(wlcache->find("T_ENDART1") + gamestate.episode);
    text = wlcache->graphic(wlcache->find("T_ENDART1") + gamestate.episode);
    showArticle(text);

    wlrender->fadeOut(0, 255, 0, 0, 0, 30);
    SETFONTCOLOR(0,15);
    wlinput->clearKeysDown();
    FreeMusic();
}

