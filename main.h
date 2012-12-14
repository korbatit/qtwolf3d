#ifndef WLMAIN_H
#define WLMAIN_H

#include <QMainWindow>
#include <QThread>
#include <QFile>

#ifdef TOUCHSCREEN
#include <QtSensors/QAccelerometer>
QTM_USE_NAMESPACE
#endif

#include "display.h"

class SleeperThread : public QThread {
public:
    static void msleep(quint32 msecs) { QThread::msleep(msecs); }
};

class GameEngine : QMainWindow {
    Q_OBJECT
public:
    GameEngine(QWidget *parent = 0);
    ~GameEngine();

    void newGame(qint32 difficulty, qint32 episode);
    void writeConfig();
    void showViewSize (qint32 width);
    void newViewSize (qint32 width);
    qint8 saveTheGame(QFile *fp, qint32 x, qint32 y);
    qint8 loadTheGame(QFile* fp, qint32 x, qint32 y);

    void   setImage(QImage* screen);
    QSize* size() { return &m_size; }

protected:
    void closeEvent(QCloseEvent* event);

public slots:
    void demoLoop();
    void start();
#ifdef TOUCHSCREEN
    void readingChanged();
#endif

private:
    void readConfig();
    void diskFlopAnim(qint32 x, qint32 y);
    qint32 doChecksum(quint8 *source, quint32 size, qint32 checksum);
    void buildTables();
    void calcProjection(qint32 focal);
    void setupWalls();
    void signonScreen();
    void finishSignon();
    void initDigiMap ();
    void doJukebox();
    void initGame();
    qint8 setViewSize (quint32 width, quint32 height);

    QSize      m_size;
    QImage     m_screen;
    WLDisplay* m_display;

#ifdef TOUCHSCREEN
    QAccelerometer* m_accelerometer;
#endif
};

#endif // MAIN_H
