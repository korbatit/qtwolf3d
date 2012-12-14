#include <string.h>

#include "wl_def.h"
#include "main.h"
#include "wlinput.h"

#include <QApplication>
#include <QtCore/qmath.h>
#include <QKeyEvent>
#include <QTimer>

extern QSize   g_screenSize;
extern QTime*  g_getTicks;
extern QImage* g_Image;

extern void gsleep(int ms);

const static quint8 ASCIINames[154] = // Unshifted ASCII for scan codes
{
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x0  - 0x0F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x10 - 0x1F
    ' ',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x20 - 0x2F
    '0','1','2','3','4','5','6','7','8','9',0  ,0  ,0  ,0  ,0  ,0  ,    // 0x30 - 0x3F
    0  ,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',    // 0x40 - 0x4F
    'P','Q','R','S','T','U','V','W','X','Y','Z',0  ,0  ,0  ,0  ,0  ,    // 0x50 - 0x5F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x60 - 0x6F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x70 - 0x7F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,    // 0x80 - 0x8F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0                               // 0x90 - 0x99
};

const static quint8 ShiftNames[16*8] = // Shifted ASCII for scan codes
{
    //	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,8  ,9  ,0  ,0  ,0  ,13 ,0  ,0  ,	// 0
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,27 ,0  ,0  ,0  ,	// 1
    ' ',0  ,0  ,0  ,0  ,0  ,0  ,34 ,0  ,0  ,'*','+','<','_','>','?',	// 2
    ')','!','@','#','$','%','^','&','*','(',0  ,':',0  ,'+',0  ,0  ,	// 3
    '~','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',	// 4
    'P','Q','R','S','T','U','V','W','X','Y','Z','{','|','}',0  ,0  ,	// 5
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0		// 7
};

const static quint8 SpecialNames[] = // ASCII for 0xe0 prefixed codes
{
    //	 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 0
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,13 ,0  ,0  ,0  ,	// 1
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 2
    0  ,0  ,0  ,0  ,0  ,'/',0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 3
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 4
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 5
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,	// 6
    0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0   	// 7
};

typedef struct {
    int x,y;
    ScanCode key;
} virtualKeyLoc_t;

const static virtualKeyLoc_t virtualKeys[39] =
{
    {  20, 110, sc_A}, {  70, 110, sc_B}, { 120, 110, sc_C}, { 520, 110, sc_D}, { 570, 110, sc_E}, { 620, 110, sc_F}, // A B C   D E F
    {  20, 150, sc_G}, {  70, 150, sc_H}, { 120, 150, sc_I}, { 520, 150, sc_J}, { 570, 150, sc_K}, { 620, 150, sc_L}, // G H I   J K L
    {  20, 190, sc_M}, {  70, 190, sc_N}, { 120, 190, sc_O}, { 520, 190, sc_P}, { 570, 190, sc_Q}, { 620, 190, sc_R}, // M N O   P Q R
    {  20, 230, sc_S}, {  70, 230, sc_T}, { 120, 230, sc_U}, { 520, 230, sc_V}, { 570, 230, sc_W}, { 620, 230, sc_X}, // S T U   V W X
    {  20, 270, sc_Y}, {  70, 270, sc_Z}, { 120, 270, sc_BackSpace}, { 520, 270, sc_0}, { 570, 270, sc_1}, { 620, 270, sc_2}, // Y Z Del 0 1 2
    {  20, 310, sc_3}, {  70, 310, sc_4}, { 120, 310, sc_5}, { 520, 310, sc_6}, { 570, 310, sc_7}, { 620, 310, sc_8}, // 3 4 5   6 7 8
    {  20, 350, sc_9}, {  70, 350, sc_RightArrow}, { 120, 350, sc_LeftArrow}                                                                   // 9
};

const static KeyboardDef KbdDefs = {
    sc_Control,             // button0
    sc_Alt,                 // button1
    sc_Home,                // upleft
    sc_UpArrow,             // up
    sc_PgUp,                // upright
    sc_LeftArrow,           // left
    sc_RightArrow,          // right
    sc_End,                 // downleft
    sc_DownArrow,           // down
    sc_PgDn                 // downright
};

const static Direction DirTable[] = // Quick lookup for total direction
{
    dir_NorthWest,
    dir_North,
    dir_NorthEast,
    dir_West,
    dir_None,
    dir_East,
    dir_SouthWest,
    dir_South,
    dir_SouthEast
};

WLInput::WLInput(QObject* parent)
    :QObject(parent)
{
    m_grabInput = false;
    clearKeysDown();
    buttons[0] = buttons[1] = buttons[2] = buttons[3] = 0;
    oldPos = QPoint(0,0);
    mousePos = QPoint(0,0);
    m_virtualKeyboard = false;
}

WLInput::~WLInput()
{
}

void WLInput::clearKeysDown()
{
    //flush events
    qApp->processEvents();

    m_lastScan = sc_None;
    m_lastASCII = key_None;
    memset((void*)m_keyboard, 0, sizeof(m_keyboard));

    buttons[0] = buttons[1] = buttons[2] = buttons[3] = 0;
    mousePos = oldPos = QPoint(0,0);
}

void WLInput::readControl(int player, ControlInfo_t *info)
{
    Q_UNUSED(player)

    int			dx,dy;
    Motion		mx,my;

    dx = dy = 0;
    mx = my = motion_None;

    qApp->processEvents();

    if (m_keyboard[KbdDefs.upleft])
        mx = motion_Left,my = motion_Up;
    else if (m_keyboard[KbdDefs.upright])
        mx = motion_Right,my = motion_Up;
    else if (m_keyboard[KbdDefs.downleft])
        mx = motion_Left,my = motion_Down;
    else if (m_keyboard[KbdDefs.downright])
        mx = motion_Right,my = motion_Down;

    if (m_keyboard[KbdDefs.up])
        my = motion_Up;
    else if (m_keyboard[KbdDefs.down])
        my = motion_Down;

    if (m_keyboard[KbdDefs.left])
        mx = motion_Left;
    else if (m_keyboard[KbdDefs.right])
        mx = motion_Right;

#ifndef TOUCHSCREEN
    if (m_keyboard[KbdDefs.button0])
        buttons[0] = true;
    if (m_keyboard[KbdDefs.button1])
        buttons[1] = true;

    info->button0 = buttons[0];
    info->button1 = buttons[1];
    info->button2 = buttons[2];
    info->button3 = buttons[3];
#else
    info->button0 = 0;
    info->button1 = 0;
    info->button2 = 0;
    info->button3 = 0;

    if (mousePos != QPoint(0,0)) {
        int check = qSqrt((oldPos.x()-mousePos.x())*(oldPos.x()-mousePos.x())
                          + (oldPos.y()-mousePos.y())*(oldPos.y()-mousePos.y()));

        if (mousePos.x() < g_screenSize.width()*0.2 && mousePos.y() < g_screenSize.height()*0.2)
            info->button1 = 1; // top left is Esc
        else if (mousePos.x() < g_screenSize.width()*0.2 && mousePos.y() > g_screenSize.height()*0.8)
            info->button0 = 1; // bottom left is fire

        else if (check < g_screenSize.width()*0.1 && mousePos != QPoint(0,0)) {
            if (!buttons[0] || !buttons[1] || !buttons[2] || !buttons[3]) {
                if (pressTime.elapsed() > 500 && !m_virtualKeyboard)
                    info->button0 = 1; // select
                else if (pressTime.elapsed() > 500) {
                    bool done = false;
                    int x = mousePos.x() * g_Image->size().width()  / g_screenSize.width();
                    int y = mousePos.y() * g_Image->size().height() / g_screenSize.height();
                    for (int i = 0; i < 39; i++) {
                        int distance = qSqrt((x-virtualKeys[i].x)*(x-virtualKeys[i].x)
                                             + (y-virtualKeys[i].y)*(y-virtualKeys[i].y));
                        if (distance <= 15) {
                            m_lastScan = virtualKeys[i].key;
                            m_keyboard[m_lastScan] = 1;
                            if(m_lastScan < (int)sizeof(ASCIINames) && ASCIINames[m_lastScan])
                                m_lastASCII = ASCIINames[m_lastScan];
                            done = true;
                            pressTime.restart();
                            break;
                        }
                    }
                    if (!done && pressTime.elapsed() > 500)
                        info->button0 = 1; // select
                }

            } else {
                if (mousePos.x() < g_screenSize.width()*0.2 && mousePos.y() < g_screenSize.height()*0.2)
                    info->button1 = 1; // back
            }
        } else if (mousePos != QPoint(0,0)){
            if (mousePos.x() > oldPos.x() + 2) {
                mx = motion_Left;
                oldPos = mousePos;
            } else if (mousePos.x() < oldPos.x() - 2) {
                mx = motion_Right;
                oldPos = mousePos;
            } else if (mousePos.y() > oldPos.y() + 2) {
                my = motion_Down;
                oldPos = mousePos;
            } else if (mousePos.y() < oldPos.y() - 2) {
                my = motion_Up;
                oldPos = mousePos;
            }
        }
    }
    if (x <= -5) {
        mx = motion_Right;
        x = 0;
    } else if (x >= 1) {
        mx = motion_Left;
        x = 0;
    } else if (y <= -1) {
        my = motion_Up;
        y = 0;
    } else if (y >= 1) {
        my = motion_Down;
        y = 0;
    }
#endif
    dx = mx * 127;
    dy = my * 127;

    info->x = dx;
    info->xaxis = mx;
    info->y = dy;
    info->yaxis = my;

    info->dir = DirTable[((my + 1) * 3) + (mx + 1)];

    buttons[0] = buttons[1] = buttons[2] = buttons[3]  = 0;
}

ScanCode WLInput::waitForKey()
{
    ScanCode	result;

    while ((result = m_lastScan) == 0)
        waitAndProcessEvents();
    m_lastScan = 0;

    return(result);
}

char WLInput::waitForASCII()
{
    char		result;

    while ((result = m_lastASCII) == 0)
        waitAndProcessEvents();
    m_lastASCII = '\0';

    return(result);
}

void WLInput::ack()
{
    startAck();
    do {
        waitAndProcessEvents();
    } while(!checkAck());
}

qint8 WLInput::userInput(quint32 delay)
{
    quint32	lasttime;

    lasttime = g_getTicks->elapsed()*7/100;
    startAck ();
    do {
        if (checkAck())
            return true;
        gsleep(50);
    } while (g_getTicks->elapsed()*7/100 - lasttime < delay);

    return(false);
}

void WLInput::waitAndProcessEvents()
{
    while(!m_lastScan) {
        qApp->processEvents();
        if (buttons[0] || buttons[1] || buttons[2] || buttons[3])
            return;
        gsleep(50);
    }
}

void WLInput::mouseDown(QMouseEvent *event)
{
    buttons[event->button()-1] = true;
    oldPos = event->pos();
    mousePos = event->pos();

    pressTime.restart();
}

void WLInput::mouseUp(QMouseEvent *event)
{
    oldPos = QPoint(0,0);
    mousePos = QPoint(0,0);
    buttons[event->button()-1] = false;

    pressTime.restart();
}

void WLInput::mouseMove(QMouseEvent *event)
{
    mousePos = event->pos();
    pressTime.restart();
}

void WLInput::keyDown(QKeyEvent *event)
{
    bool     specialKey = event->key()&0x01000000;
    quint32  key = event->key()&0xFFFFF;
    quint32  offset = SPECIALKEY_OFFSET;

    if ((key + offset) > sizeof(m_keyboard))
        return;

    if (specialKey)
        m_lastScan = offset + key;
    else
        m_lastScan = key;

    m_keyboard[m_lastScan] = 1;

    if(m_lastScan < (int)sizeof(ASCIINames) && ASCIINames[m_lastScan])
        m_lastASCII = ASCIINames[m_lastScan];

    if(m_lastScan == (int)(sc_Pause + offset))
        m_paused = true;
}

void WLInput::keyUp(QKeyEvent *event)
{
    bool     specialKey = event->key()&0x01000000;
    quint32  key = event->key()&0xFFFFF;
    quint32  offset = SPECIALKEY_OFFSET;

    if ((key + offset) > sizeof(m_keyboard))
        return;

    if (specialKey)
        m_lastScan = offset + key;
    else
        m_lastScan = key;

    m_keyboard[m_lastScan] = 0;

    if(m_lastScan < (int)sizeof(ASCIINames) && ASCIINames[m_lastScan])
        m_lastASCII = 0;

    if (m_lastScan == sc_Return || m_lastScan == sc_Enter)
        m_lastScan = 0;
}

void WLInput::startAck()
{
    clearKeysDown();
}

qint8 WLInput::checkAck()
{
    qApp->processEvents();

    if (buttons[0] || buttons[1] || buttons[2] || buttons[3])
        return true;

    if (m_lastScan)
        return true;

    return false;
}

bool WLInput::isInputGrabbed()
{
    return m_grabInput;
}
