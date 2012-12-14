#ifndef	WLUSER_H
#define	WLUSER_H

#define	MaxX	      320
#define	MaxY	      200
#define	MaxHelpLines  500
#define	MaxHighName	   57
#define	MaxScores	    7
#define	MaxGameName    32
#define	MaxSaveGames    6
#define	MaxString     128

typedef	struct {
    char	name[MaxHighName + 1];
    qint32	score;
    quint16	completed,episode;
} HighScore;

typedef	struct {
    char	signature[4];
    quint16	*oldtest;
    qint8	present;
    char	name[MaxGameName + 1];
} SaveGame;

typedef	struct {
    int	x,y,
    w,h,
    px,py;
} WindowRec;

extern bool     loadedgame;
extern quint16  PrintX;
extern quint16  PrintY;
extern quint16  WindowX;
extern quint16  WindowY;
extern quint16  WindowW;
extern quint16  WindowH;

extern	qint8		(*USL_SaveGame)(int),(*USL_LoadGame)(int);
extern	void		(*USL_ResetGame)(void);
extern	SaveGame	Games[MaxSaveGames];
extern	HighScore	Scores[];

#define	US_HomeWindow()	{PrintX = WindowX; PrintY = WindowY;}

class WLUser {
public:
    WLUser();
    ~WLUser();

    void US_TextScreen();
    void US_UpdateTextScreen();
    void US_FinishTextScreen();
    void US_DrawWindow(quint16 x,quint16 y,quint16 w,quint16 h);
    void US_CenterWindow(quint16,quint16);
    void US_SaveWindow(WindowRec *win);
    void US_RestoreWindow(WindowRec *win);
    void US_ClearWindow();
    void US_SetPrintRoutines(void (*measure)(const char *,quint16 *,quint16 *), void (*print)(const char *));
    void US_SetPrintRoutines(const char *a, quint16* b,quint16* c);
    void US_PrintCentered(const char *s);
    void US_CPrint(const char *s);
    void US_CPrintLine(const char *s);
    void US_Print(const char *s);
    void US_Printf(const char *formatStr, ...);
    void US_CPrintf(const char *formatStr, ...);
    void US_PrintUnsigned(quint32 n);
    void US_PrintSigned(qint32 n);
    void US_StartCursor();
    void US_ShutCursor();
    char USL_RotateChar(char ch, int dir);
    void US_CheckHighScore(qint32 score,quint16 other);
    void US_DisplayHighScores(qint32 which);
    qint8 US_LineInput(qint32 x, qint32 y, char *buf, const char *def, qint8 escok, qint32 maxchars, qint32 maxwidth);
    void USL_PrintInCenter(const char *s,Rect r);
    char* USL_GiveSaveName(quint16 game);
    void US_InitRndT(qint32 randomize);
    qint32 US_RndT();

    static void USL_XORICursor(qint32 x, qint32 y, const char *s, quint16 cursor);
};

#endif
