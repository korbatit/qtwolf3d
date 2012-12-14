#ifndef WLTEXT_H
#define WLTEXT_H

#define BACKCOLOR       0x11
#define WORDLIMIT       80
#define FONTHEIGHT      10
#define TOPMARGIN       16
#define BOTTOMMARGIN    32
#define LEFTMARGIN      16
#define RIGHTMARGIN     16
#define PICMARGIN       8
#define TEXTROWS        ((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define SPACEWIDTH      7
#define SCREENPIXWIDTH  320
#define SCREENMID       (SCREENPIXWIDTH/2)


class WLText {
public:
    WLText();
    ~WLText();

    void helpScreens();
    void endText();

private:
    void ripToEOL();
    int  parseNumber();
    void parsePicCommand();
    void parseTimedCommand();
    void timedPicCommand();
    void handleCommand();
    void newLine();
    void handleCtrls();
    void handleWord();
    void pageLayout(qint8 shownumber);
    void backPage();
    void cacheLayoutGraphics();
    void showArticle(char *article);

    bool     m_layoutDone;
    qint32   m_pageNum;
    qint32   m_numPages;
    quint32  m_leftMargin[TEXTROWS];
    quint32  m_rightMargin[TEXTROWS];
    quint32  m_rowOn;
    qint32   m_picX;
    qint32   m_picY;
    qint32   m_picNum;
    qint32   m_picDelay;
    char*    m_text;
};

#endif // WLTEXT_H
