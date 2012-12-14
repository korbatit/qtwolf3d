
#include "wl_def.h"

#include <QtGlobal>
#include <QDebug>

#include "wlaudio.h"

#include "sound.h"

extern WLAudio* wlaudio;

bool GameSoundServerPrivate::initSound(QAudioFormat fmt)
{
    quit = false;
    mute = false;
#ifdef TOUCHSCREEN
    music = false;
#else
    music = true;
#endif

    for(int i = 0; i < 100; i++) { // STARTMUSIC - STARTDIGISOUNDS
        m_soundChunks[i].buffer = 0;
        m_soundChunks[i].length = 0;
        m_soundChunks[i].pos    = 0;
        m_soundChunks[i].volume = 0;
        m_sessions[i] = 0;
    }

    format = fmt;

#if QT_VERSION >= 0x050000
    audioResampler = new AudioResampler(format.sampleRate(), format.channelCount(), format.sampleRate(), 100);
#else
    audioResampler = new AudioResampler(format.frequency(), format.channels(), format.frequency(), 100);
#endif
    audioOutput = new QAudioOutput(format);
    output = audioOutput->start();
    if (output) return true;

    return false;
}

int GameSoundServerPrivate::resampleAndMix(AudioResampler *audioResampler, QAudioFormat fmt, char *src, int dataAmt, bool first)
{
    int converted = 0;

    if ( fmt.sampleSize() == 16 ) {
        converted = audioResampler->resample( (const qint16*)src, (qint16*)mixbuf, dataAmt/2, !first );
    } else {
        int samplesCount = dataAmt * 8 / fmt.sampleSize();
        qint16 samples[samplesCount];

        switch ( fmt.sampleSize() ) {
        case 8:
            for ( int i=0; i<samplesCount; i++ )
                samples[i] = (int(src[i]) - 128)*256;
            break;
        case 16:
            memcpy( samples, src, dataAmt );
            break;
        case 32:
        {
            int *srcSamples = (int*)src;
            for ( int i=0; i<samplesCount; i++ )
                samples[i] = srcSamples[i]; // is this correct?
        }
            break;
        default:
            memset( samples, 0, sizeof(samples) );
        }
        converted = audioResampler->resample( samples, (qint16*)mixbuf, samplesCount, !first );
    }
    return converted*2;
}

extern void SDL_IMFMusicPlayer(void *udata, quint8 *stream, int len);

void GameSoundServerPrivate::run()
{
    QTime tm;
    char  buffer[audioOutput->periodSize()];
    int   sz = 100; // STARTMUSIC - STARTDIGISOUNDS
    int   bufferTime = audioOutput->bufferSize() * 8000 / (format.frequency() * format.channels() * format.sampleSize()) /3;
    bool first;

    do {
        first = true;
        tm.restart();
        QMutexLocker conditionLock(&mutex);

        while(audioOutput->bytesFree() > audioOutput->periodSize()) {

            memset(buffer, 0, audioOutput->periodSize());

            if (wlaudio && music)
                wlaudio->musicPlayer(NULL,(quint8*)mixbuf,audioOutput->periodSize());

            first = true;
            int  mixLength = 0;

            char* tmp = buffer;

            for(int i = 0; i < sz; i++) {

                if (m_sessions[i] && m_soundChunks[i].length > 0) {
                    memset(tmp,0,audioOutput->periodSize());
                    int read = 0;
                    read = audioOutput->periodSize();
                    mixChunk_t* sample = &m_soundChunks[i];

                    if (sample->pos+audioOutput->periodSize() < sample->length) {
                        // Got a period worth
                        read = audioOutput->periodSize();
                        memcpy(tmp,sample->buffer+sample->pos,audioOutput->periodSize());
                        sample->pos+=audioOutput->periodSize();
                    } else if (sample->pos == sample->length) {
                        // End of sound
                        read = 0;
                    } else {
                        // Got less than period to go
                        read = sample->length-sample->pos;
                        memcpy(tmp,sample->buffer+sample->pos,read);
                        //memset(tmp+read, 0, audioOutput->periodSize()-read);
                        sample->pos = sample->length;
                    }
                    if (read > 0) {
                        mixLength = qMax(resampleAndMix(audioResampler, format, tmp, read, first), mixLength);
                        first = false;

                    } else
                        m_sessions[i] = 0;
                }
            }
            if (wlaudio && (music || mixLength > 0))
                output->write(mixbuf,audioOutput->periodSize());

            if (!music && first) break; // no sounds to play got waiting state
        }
        if (tm.elapsed() < bufferTime) {
            msleep(bufferTime - tm.elapsed());
        }
        if (!music && first)
            condition.wait(&mutex, 1000);
        else
            condition.wait(&mutex, 5);

    } while (!quit);

    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = 0;
    }
    delete audioResampler;
}

GameSoundServer::GameSoundServer(QAudioFormat fmt, QObject *parent)
    : QObject(parent)
{
    server = new GameSoundServerPrivate;
    server->initSound(fmt);
    server->start(QThread::HighPriority);
}

GameSoundServer::~GameSoundServer()
{
    QMutexLocker(&server->mutex);
    server->quit = true;
    server->condition.wakeAll();
    server->wait();
    delete server;
}

QAudioFormat GameSoundServer::format()
{
    return server->format;
}

void GameSoundServer::playSound(int index)
{
    QMutexLocker(&server->mutex);

    server->m_sessions[index] = 1;
    server->m_soundChunks[index].pos = 0;

    server->condition.wakeAll();
}

void GameSoundServer::setSound(int index, mixChunk_t *sample)
{
    QMutexLocker(&server->mutex);

    server->m_soundChunks[index].buffer = sample->buffer;
    server->m_soundChunks[index].length = sample->length;
    server->m_soundChunks[index].volume = sample->volume;
    server->m_soundChunks[index].pos    = sample->pos;
}

void GameSoundServer::sound(int index, mixChunk_t *sample)
{
    QMutexLocker(&server->mutex);

    sample->buffer = server->m_soundChunks[index].buffer;
    sample->length = server->m_soundChunks[index].length;
    sample->volume = server->m_soundChunks[index].volume;
    sample->pos    = server->m_soundChunks[index].pos;
}


