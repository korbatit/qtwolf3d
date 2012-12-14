#ifndef SOUND_H
#define SOUND_H

#include "wl_def.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QAudioOutput>
#include <QFile>

#include "resample.h"

typedef struct {
    char* buffer;
    int   length;
    int   volume;
    int   pos;
} mixChunk_t;

class GameSoundServerPrivate : public QThread
{
    Q_OBJECT
public:
    bool initSound(QAudioFormat fmt);

    QMutex         mutex;
    QWaitCondition condition;

    int         m_sessions[100];
    mixChunk_t  m_soundChunks[100];

    bool          music;
    bool          mute;
    bool          quit;

    QAudioFormat     format;
    AudioResampler*  audioResampler;
    char             mixbuf[512000];

    QFile*        debugFile;

protected:
    void run();

private:
    int resampleAndMix(AudioResampler* audioResampler, QAudioFormat fmt,
                       char *src, int dataAmt, bool first);

    QAudioOutput* audioOutput;
    QIODevice*    output;
};

class GameSoundServer : public QObject
{
    Q_OBJECT
public:
    GameSoundServer(QAudioFormat fmt, QObject* parent = 0);
    ~GameSoundServer();

    QAudioFormat format();

    void playSound(int index);

    void setSound(int index, mixChunk_t* sample);
    void sound(int index, mixChunk_t* sample);

private:
    GameSoundServerPrivate* server;
};

#endif // SOUND_H
