#ifndef WLAUDIO_H
#define WLAUDIO_H

#include "fmopl.h"

const static qint32 g_tickBase = 70;

typedef enum {
    sdm_Off,
    sdm_PC,
    sdm_AdLib,
} SDMode;

typedef enum {
    smm_Off,
    smm_AdLib
} SMMode;

typedef enum {
    sds_Off,
    sds_PC,
    sds_SoundBlaster
} SDSMode;

typedef struct {
    quint32 length;
    quint16 priority;
} SoundCommon;

typedef struct {
    SoundCommon common;
    quint8      data[1];
} PCSound;

typedef struct {
    quint8 mChar;
    quint8 cChar;
    quint8 mScale;
    quint8 cScale;
    quint8 mAttack;
    quint8 cAttack;
    quint8 mSus;
    quint8 cSus;
    quint8 mWave;
    quint8 cWave;
    quint8 nConn;
    quint8 voice;
    quint8 mode;
    quint8 unused[3];
} Instrument_t;

typedef struct {
    SoundCommon  common;
    Instrument_t inst;
    quint8       block;
    quint8       data[1];
} AdLibSound_t;

typedef struct {
    quint32 startpage;
    quint32 length;
} digiInfo_t;

typedef struct {
    qint32 valid;
    qint32 globalsoundx;
    qint32 globalsoundy;
} globalSoundPos_t;

class GameSoundServer;

class WLAudio {
public:
    WLAudio();
    ~WLAudio();

private:
    typedef struct {
        char RIFF[4];
        quint32 filelenminus8;
        char WAVE[4];
        char fmt_[4];
        quint32 formatlen;
        quint16 val0x0001;
        quint16 channels;
        quint32 samplerate;
        quint32 bytespersec;
        quint16 bytespersample;
        quint16 bitspersample;
    } headChunk_t;

    typedef struct {
        char    chunkid[4];
        quint32 chunklength;
    } waveChunk_t;

public:
    void     musicOn(); // menu
    quint16  musicOff(); // menu,play
    void     startMusic(int chunk); // menu,play
    void     continueMusic(int chunk, int startoffs); // play
    bool     setMusicMode(SMMode mode); // main,menu
    void     setDigiDevice(SDSMode); // main,menu
    void     stopDigitized(); // game,play,text
    void     musicPlayer(void *udata, quint8 *stream, int len); // sound
    SDSMode* digiMode() { return &m_digiMode; } // act2,main,menu
    SMMode*  musicMode() { return &m_musicMode; } // main,menu
    qint32   digiMap(int idx) { return m_digiMap[idx]; }
    void     setDigiMap(int idx, qint32 value) { m_digiMap[idx] = value; } // main
    qint32   digiChannel(int idx) { return m_digiChannel[idx]; }
    void     setDigiChannel(int idx, qint32 value) { m_digiChannel[idx] = value; } // main

    void     positionSound(int leftVol, int rightVol); // game
    //bool     playSound(soundNames sound); // state,menu,inter,game,agent,act1,act2
    bool     playSound(const QString& sound);
    void     setPosition(int channel, int leftVol,int rightVol); //game
    void     stopSound();  // inter, menu
    void     waitSoundDone(); // agent,game,menu
    bool     setSoundMode(SDMode mode); // main,menu
    quint16  soundPlaying(); // agent,inter
    void     prepareSound(int which); // main
    bool     hasAdLib() { return m_adLib; }  // main,menu
    bool     hasSoundBlaster() { return m_soundBlaster; } // main,menu
    SDMode*  soundMode() { return &m_soundMode; } // main,menu

    globalSoundPos_t* channelSoundPos(int idx) { return &m_channelSoundPos[idx]; } // game

private:
    qint32   getChannelForDigi(qint32 which);
    void     fadeOutMusic();
    bool     musicPlaying();
    qint32   playDigitized(quint16 which, qint32 leftPos, qint32 rightPos);
    void     soundFinished();
    void     setupDigi();
    void     channelFinished(qint32 channel);
    void     alStopSound();
    void     alSetFXInst(Instrument_t *inst);
    void     alPlaySound(AdLibSound_t *sound);
    void     shutAl();
    void     startAl();
    void     shutDevice();
    void     startDevice();
    qint16   getSample(float csample, quint8 *samples, int size);

    inline void alOut(qint32 a, qint32 b) { YM3812Write(0, a, b); }

    bool     m_adLib;
    bool     m_soundBlaster;
    qint8    m_soundPositioned;
    qint32   m_numReadySamples;
    quint8*  m_curAlSound;
    quint8*  m_curAlSoundPtr;
    quint32  m_curAlLengthLeft;
    qint32   m_soundTimeCounter;
    qint32   m_samplesPerMusicTick;
    qint16   m_originalSampleRate;
    qint16   m_sqMaxTracks;

    globalSoundPos_t m_channelSoundPos[8];

    SDMode   m_soundMode;
    SDSMode  m_digiMode;
    SMMode   m_musicMode;
    qint32   m_digiMap[100]; // LASTSOUND
    qint32   m_digiChannel[100]; // STARTMUSIC - STARTDIGISOUNDS

    GameSoundServer* m_soundServer;

    qint8    m_alChar;
    qint8    m_alScale;
    qint8    m_alAttack;
    qint8    m_alSus;
    qint8    m_alWave;
    qint8    m_alFreqL;
    qint8    m_alFreqH;
    qint8    m_alFeedCon;
    qint8    m_alEffects;

    bool     m_nextSoundPos;
    qint32   m_leftPosition;
    qint32   m_rightPosition;
    quint16  m_soundPriority;
    quint16  m_digiPriority;
    quint16  m_numDigi;
    bool     m_digiPlaying;

    quint32  m_sqHackTime;
    qint32   m_sqHackSeqLen;
    qint32   m_sqHackLen;

    bool     m_sqActive;
    bool     m_pcSound;

    quint16* m_sqHack;
    quint16* m_sqHackPtr;

    qint32       m_soundNumber;
    qint32       m_digiNumber;
    digiInfo_t*  m_digiList;
    quint8       m_alBlock;
    quint32      m_alLengthLeft;
    quint32      m_alTimeCount;
    Instrument_t m_alZeroInst;
};

#endif
