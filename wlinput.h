#ifndef	WLINPUT_H
#define	WLINPUT_H

#include <QKeyEvent>
#include <QTime>

#define	sc_None			0

#define SPECIALKEY_OFFSET 0x60

// Special Keys with 0x01000000 added
#define sc_Escape       0x0  + SPECIALKEY_OFFSET
#define sc_Tab          0x1  + SPECIALKEY_OFFSET
#define sc_BackSpace    0x3  + SPECIALKEY_OFFSET
#define sc_Enter        0x4  + SPECIALKEY_OFFSET
#define sc_Return       0x5  + SPECIALKEY_OFFSET
#define sc_Delete       0x7  + SPECIALKEY_OFFSET
#define sc_Pause        0x8  + SPECIALKEY_OFFSET
#define sc_PrintScreen  0x9  + SPECIALKEY_OFFSET
#define sc_Home         0x10 + SPECIALKEY_OFFSET
#define sc_End          0x11 + SPECIALKEY_OFFSET
#define sc_LeftArrow    0x12 + SPECIALKEY_OFFSET
#define sc_UpArrow      0x13 + SPECIALKEY_OFFSET
#define sc_RightArrow   0x14 + SPECIALKEY_OFFSET
#define sc_DownArrow    0x15 + SPECIALKEY_OFFSET
#define sc_PgUp         0x16 + SPECIALKEY_OFFSET
#define sc_PgDn         0x17 + SPECIALKEY_OFFSET
#define sc_LShift       0x20 + SPECIALKEY_OFFSET
#define sc_RShift       0x20 + SPECIALKEY_OFFSET
#define sc_Control      0x21 + SPECIALKEY_OFFSET
#define sc_Alt          0x23 + SPECIALKEY_OFFSET
#define sc_F1           0x30 + SPECIALKEY_OFFSET
#define sc_F2           0x31 + SPECIALKEY_OFFSET
#define sc_F3           0x32 + SPECIALKEY_OFFSET
#define sc_F4           0x33 + SPECIALKEY_OFFSET
#define sc_F5           0x34 + SPECIALKEY_OFFSET
#define sc_F6           0x35 + SPECIALKEY_OFFSET
#define sc_F7           0x36 + SPECIALKEY_OFFSET
#define sc_F8           0x37 + SPECIALKEY_OFFSET
#define sc_F9           0x38 + SPECIALKEY_OFFSET
#define sc_F10          0x39 + SPECIALKEY_OFFSET

// Normal Keys, no mod
#define sc_Space        0x20
#define sc_0            0x30
#define sc_1            0x31
#define sc_2            0x32
#define sc_3            0x33
#define sc_4            0x34
#define sc_5            0x35
#define sc_6            0x36
#define sc_7            0x37
#define sc_8            0x38
#define sc_9            0x39
#define sc_A            0x41
#define sc_B            0x42
#define sc_C            0x43
#define sc_D            0x44
#define sc_E            0x45
#define sc_F            0x46
#define sc_G            0x47
#define sc_H            0x48
#define sc_I            0x49
#define sc_J            0x4A
#define sc_K            0x4B
#define sc_L            0x4C
#define sc_M            0x4D
#define sc_N            0x4E
#define sc_O            0x4F
#define sc_P            0x50
#define sc_Q            0x51
#define sc_R            0x52
#define sc_S            0x53
#define sc_T            0x54
#define sc_U            0x55
#define sc_V            0x56
#define sc_W            0x57
#define sc_X            0x58
#define sc_Y            0x59
#define sc_Z            0x5A

#define	key_None		0

typedef	enum {
    motion_Left  = -1,
    motion_Up    = -1,
    motion_None  =  0,
    motion_Right =  1,
    motion_Down  =  1
} Motion;

typedef	enum {
    dir_North,
    dir_NorthEast,
    dir_East,
    dir_SouthEast,
    dir_South,
    dir_SouthWest,
    dir_West,
    dir_NorthWest,
    dir_None
} Direction;

typedef	struct {
    qint8	button0;
    qint8       button1;
    qint8       button2;
    qint8       button3;
    short	x;
    short       y;
    Motion	xaxis;
    Motion      yaxis;
    Direction	dir;
} CursorInfo_t;
typedef	CursorInfo_t ControlInfo_t;

typedef	int ScanCode;

typedef	struct {
    ScanCode button0;
    ScanCode button1;
    ScanCode upleft;
    ScanCode up;
    ScanCode upright;
    ScanCode left;
    ScanCode right;
    ScanCode downleft;
    ScanCode down;
    ScanCode downright;
} KeyboardDef;


class WLInput : public QObject {
    Q_OBJECT
public:
    WLInput(QObject* parent = 0);
    ~WLInput();

    void     readControl(int, ControlInfo_t*);
    void     freeDemoBuffer();
    qint8    userInput(quint32 delay);
    char     waitForASCII();
    ScanCode waitForKey();
    void     ack();

    void    waitAndProcessEvents();
    void    keyDown(QKeyEvent* event);
    void    keyUp(QKeyEvent* event);
    void    mouseDown(QMouseEvent* event);
    void    mouseUp(QMouseEvent* event);
    void    mouseMove(QMouseEvent* event);
    void    processEvents();

    void    startAck();
    qint8   checkAck ();
    bool    isInputGrabbed();

    void     setLastScan(ScanCode key) { m_lastScan = key; }
    ScanCode lastScan() { return m_lastScan; }

    void     setLastAscii(ScanCode key) { m_lastASCII = key; }
    ScanCode lastAscii() { return m_lastASCII; }

    bool isKeyDown(ScanCode key) { return m_keyboard[key]; }
    void clearKey(ScanCode key) { m_keyboard[key] = false; if (key == m_lastScan) m_lastScan = sc_None; }

    bool isPaused() { return m_paused; }
    void setPaused(bool value) { m_paused = value; }

    bool hasVirtualKeyboard() { return m_virtualKeyboard; }
    void setVirtualKeyboard(bool value) { m_virtualKeyboard = value; }

    QTime  pressTime;

    bool   buttons[4];
    QPoint oldPos;
    QPoint mousePos;

#ifdef TOUCHSCREEN
    int x;
    int y;
    int z;
#endif

public slots:
    void     clearKeysDown();

private:
    bool        m_virtualKeyboard;
    bool        m_paused;
    ScanCode	m_lastScan;
    char        m_lastASCII;
    qint8       m_keyboard[200];
    bool        m_grabInput;
};

#endif
