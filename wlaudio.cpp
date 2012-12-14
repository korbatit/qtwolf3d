#include "wl_def.h"

#include "wlaudio.h"
#include "wlpage.h"
#include "wlcache.h"
#include "wlinput.h"

#include "sound.h"

#include <QDebug>

extern WLPage*  wlpage;
extern WLCache* wlcache;
extern WLInput* wlinput;

extern void gsleep(int ms);

static quint8**          g_soundTable;
static quint8 * volatile g_alSound;

WLAudio::WLAudio():
    m_adLib(false), m_soundBlaster(false), m_soundPositioned(0),
    m_numReadySamples(0), m_curAlSound(0), m_curAlSoundPtr(0), m_curAlLengthLeft(0),
    m_soundTimeCounter(5), m_originalSampleRate(7042), m_sqMaxTracks(10),
    m_alChar(0x20), m_alScale(0x40), m_alAttack(0x60), m_alSus(0x80), m_alWave(0xe0),
    m_alFreqL(0xa0), m_alFreqH(0xb0), m_alFeedCon(0xc0), m_alEffects(0xbd),
    m_nextSoundPos(false)
{
    QAudioFormat format;

#if QT_VERSION >= 0x050000
    format.setSampleRate(param_samplerate);
    format.setChannelCount(2);
#else
    format.setFrequency(param_samplerate);
    format.setChannels(2);
#endif
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    m_soundServer = new GameSoundServer(format);

    m_samplesPerMusicTick = param_samplerate / 700;

    if(YM3812Init(1,3579545,param_samplerate)) {
        printf("Unable to create virtual OPL!!\n");
    }
    for(int i = 1;i < 0xf6;i++)
        YM3812Write(0,i,0);

    YM3812Write(0, 1, 0x20); // Set WSE=1
    m_adLib = true;
    m_soundBlaster = true;
    m_alTimeCount = 0;
    setSoundMode(sdm_Off);
    setMusicMode(smm_Off);
    setupDigi();
}

WLAudio::~WLAudio()
{
    musicOff();
    stopSound();
    free(m_digiList);

    delete m_soundServer;
}

qint32 WLAudio::getChannelForDigi(qint32 which)
{
    if(m_digiChannel[which] != -1) return m_digiChannel[which];
    return 1;
}

void  WLAudio::positionSound(int leftVol, int rightVol)
{
    m_leftPosition = leftVol;
    m_rightPosition = rightVol;
    m_nextSoundPos = true;
}

bool  WLAudio::playSound(const QString& snd)
{
    qint8         ispos;
    SoundCommon*  s;
    qint32        lp,rp;

    lp = m_leftPosition;
    rp = m_rightPosition;
    m_leftPosition = 0;
    m_rightPosition = 0;

    ispos = m_nextSoundPos;
    m_nextSoundPos = false;

    qint32 sound = wlcache->findSound(snd);
    if (sound < 0 || (m_digiMode == sds_Off && m_soundMode == sdm_Off))
        return 0;

    s = (SoundCommon*) g_soundTable[sound];

    if ((m_soundMode != sdm_Off) && !s)
        Quit("SD_PlaySound() - Uncached sound");

    if ((m_digiMode != sds_Off) && (m_digiMap[sound] != -1)) {
        if ((m_digiMode == sds_PC) && (m_soundMode == sdm_PC)) {
            return 0;
        } else {
            int channel = playDigitized(m_digiMap[sound], lp, rp);
            m_soundPositioned = ispos;
            m_digiNumber = sound;
            m_digiPriority = s->priority;
            return channel + 1;
        }
        return(true);
    }
    if (m_soundMode == sdm_Off)
        return 0;

    if (!s->length)
        Quit("SD_PlaySound() - Zero length sound");
    if (s->priority < m_soundPriority)
        return 0;

    switch (m_soundMode) {
    case sdm_PC:
        break;
    case sdm_AdLib:
        alPlaySound((AdLibSound_t*)s);
        break;
    default:
        break;
    }
    m_soundNumber = sound;
    m_soundPriority = s->priority;
    return 0;
}

void  WLAudio::setPosition(int channel, int leftPos,int rightPos)
{
    Q_UNUSED(channel)

    if((leftPos < 0) || (leftPos > 15) || (rightPos < 0) || (rightPos > 15)
            || ((leftPos == 15) && (rightPos == 15)))
        Quit("SD_SetPosition: Illegal position");

    switch (m_digiMode){
    case sds_SoundBlaster:
        break;
    default:
        break;
    }
}

void  WLAudio::stopSound()
{
    if (m_digiPlaying)
        stopDigitized();

    switch (m_soundMode)
    {
    case sdm_PC:
        break;
    case sdm_AdLib:
        alStopSound();
        break;
    default:
        break;
    }
    m_soundPositioned = false;
    soundFinished();
}

void  WLAudio::waitSoundDone()
{
    while (soundPlaying())
        gsleep(5);
}

void  WLAudio::startMusic(int chunk)
{
    musicOff();

    if (m_musicMode == smm_AdLib) {
        qint32 chunkLen = wlcache->cacheAudio(chunk);
        m_sqHack = (quint16 *)(void *) wlcache->audio(chunk);     // alignment is correct
        if(*m_sqHack == 0) m_sqHackLen = m_sqHackSeqLen = chunkLen;
        else m_sqHackLen = m_sqHackSeqLen = *m_sqHack++;

        m_sqHackPtr = m_sqHack;
        m_sqHackTime = 0;
        m_alTimeCount = 0;
        musicOn();
    }
}

void  WLAudio::continueMusic(int chunk, int startoffs)
{
    musicOff();

    if (m_musicMode == smm_AdLib) {
        qint32 chunkLen = wlcache->cacheAudio(chunk);
        m_sqHack = (quint16 *)(void *) wlcache->audio(chunk);     // alignment is correct
        if(*m_sqHack == 0) m_sqHackLen = m_sqHackSeqLen = chunkLen;
        else m_sqHackLen = m_sqHackSeqLen = *m_sqHack++;

        m_sqHackPtr = m_sqHack;

        if(startoffs >= m_sqHackLen) {
            Quit("SD_StartMusic: Illegal startoffs provided!");
        }
        for(int i = 0; i < startoffs; i += 2) {
            quint8 reg = *(quint8*)m_sqHackPtr;
            quint8 val = *(((quint8*)m_sqHackPtr) + 1);
            if(reg >= 0xb1 && reg <= 0xb8) val &= 0xdf;           // disable play note flag
            else if(reg == 0xbd) val &= 0xe0;                     // disable drum flags

            alOut(reg,val);
            m_sqHackPtr += 2;
            m_sqHackLen -= 4;
        }
        m_sqHackTime = 0;
        m_alTimeCount = 0;
        musicOn();
    }
}

void  WLAudio::musicOn()
{
    m_sqActive = true;
}

void  WLAudio::fadeOutMusic()
{
    switch (m_musicMode) {
    case smm_AdLib:
        musicOff();
        break;
    default:
        break;
    }
}

quint16 WLAudio::musicOff()
{
    quint16    i;

    m_sqActive = false;
    switch (m_musicMode) {
    case smm_AdLib:
        alOut(m_alEffects, 0);
        for (i = 0;i < m_sqMaxTracks;i++)
            alOut(m_alFreqH + i + 1, 0);
        break;
    default:
        break;
    }
    return (int) (m_sqHackPtr-m_sqHack);
}

bool  WLAudio::musicPlaying()
{
    qint8 result;

    switch (m_musicMode) {
    case smm_AdLib:
        result = m_sqActive;
        break;
    default:
        result = false;
        break;
    }

    return(result);
}

bool  WLAudio::setSoundMode(SDMode mode)
{
    qint8 result = false;
    quint16    tableoffset;

    stopSound();

    if ((mode == sdm_AdLib) && !m_adLib)
        mode = sdm_PC;

    switch (mode) {
    case sdm_Off:
        tableoffset = wlcache->findSound("LASTSOUND"); //STARTADLIBSOUNDS;
        result = true;
        break;
    case sdm_PC:
        tableoffset = 0; //STARTPCSOUNDS;
        result = true;
        break;
    case sdm_AdLib:
        tableoffset = wlcache->findSound("LASTSOUND"); //STARTADLIBSOUNDS;
        if (m_adLib)
            result = true;
        break;
    default:
        Quit("SD_SetSoundMode: Invalid sound mode %i", mode);
        return false;
    }
    g_soundTable = (quint8**)wlcache->audioData(tableoffset);

    if (result && (mode != m_soundMode)) {
        shutDevice();
        m_soundMode = mode;
        startDevice();
    }
    return(result);
}

bool  WLAudio::setMusicMode(SMMode mode)
{
    qint8 result = false;

    fadeOutMusic();
    while (musicPlaying())
        gsleep(5);

    switch (mode) {
    case smm_Off:
        result = true;
        break;
    case smm_AdLib:
        if (m_adLib)
            result = true;
        break;
    }
    if (result)
        m_musicMode = mode;
    return(result);
}

quint16 WLAudio::soundPlaying()
{
    qint8 result = false;

    switch (m_soundMode) {
    case sdm_PC:
        result = m_pcSound? true : false;
        break;
    case sdm_AdLib:
        result = g_alSound? true : false;
        break;
    default:
        break;
    }
    if (result)
        return(m_soundNumber);
    else
        return(false);
}

void  WLAudio::setDigiDevice(SDSMode mode)
{
    bool deviceNotPresent = false;

    if (mode == m_digiMode)
        return;

    stopDigitized();

    switch (mode) {
    case sds_SoundBlaster:
        if (!m_soundBlaster)
            deviceNotPresent = true;
        break;
    default:
        break;
    }

    if (!deviceNotPresent) {
        m_digiMode = mode;
    }
}

void  WLAudio::prepareSound(int which)
{
    if(m_digiList == NULL)
        Quit("SD_PrepareSound(%i): DigiList not initialized!\n", which);

    int page = m_digiList[which].startpage;
    int size = m_digiList[which].length;

    quint8 *origsamples = (quint8*)wlpage->sound(page);
    if (origsamples + size >= (quint8*)wlpage->end())
        Quit("SD_PrepareSound(%i): Sound reaches out of page file!\n", which);

    int destsamples = (int) ((float) size * (float) param_samplerate
                             / (float) m_originalSampleRate);

    quint8 *wavebuffer = (quint8*)malloc(sizeof(headChunk_t) + sizeof(waveChunk_t)
                                         + destsamples * 2);     // dest are 16-bit samples
    if(wavebuffer == NULL)
        Quit("Unable to allocate wave buffer for sound %i!\n", which);

    headChunk_t head = {{'R','I','F','F'}, 0, {'W','A','V','E'},
                      {'f','m','t',' '}, 0x10, 0x0001, 1, param_samplerate, param_samplerate*2, 2, 16};
    waveChunk_t dhead = {{'d', 'a', 't', 'a'}, destsamples*2};
    head.filelenminus8 = sizeof(head) + destsamples*2;  // (sizeof(dhead)-8 = 0)
    memcpy(wavebuffer, &head, sizeof(head));
    memcpy(wavebuffer+sizeof(head), &dhead, sizeof(dhead));

    qint16 *newsamples = (qint16 *)(void *) (wavebuffer + sizeof(headChunk_t)
                                             + sizeof(waveChunk_t));
    float cursample = 0.F;
    float samplestep = (float) m_originalSampleRate / (float) param_samplerate;
    for(int i=0; i<destsamples; i++, cursample+=samplestep) {
        newsamples[i] = getSample((float)size * (float)i / (float)destsamples,
                                  origsamples, size);
    }
    mixChunk_t sample;
    sample.buffer = (char*)wavebuffer;
    sample.length = sizeof(headChunk_t)+sizeof(waveChunk_t)+destsamples*2;
    sample.volume = 100;
    sample.pos    = 0;
    m_soundServer->setSound(which,&sample);
}

qint32   WLAudio::playDigitized(quint16 which, qint32 leftPos, qint32 rightPos)
{
    if (!m_digiMode)
        return 0;

    if (which >= m_numDigi)
        Quit("SD_PlayDigitized: bad sound number %i", which);

    int channel = getChannelForDigi(which);
    setPosition(channel, leftPos,rightPos);
    m_digiPlaying = true;
    mixChunk_t sample;
    m_soundServer->sound(which,&sample);
    if (sample.length == 0) {
        qWarning("SoundChunks[%d] is NULL!",which);
        return 0;
    }
    m_soundServer->playSound(which);
    return channel;
}

void  WLAudio::stopDigitized()
{
    m_digiPlaying = false;
    m_digiNumber = 0;
    m_digiPriority = 0;
    m_soundPositioned = false;
    if ((m_digiMode == sds_PC) && (m_soundMode == sdm_PC))
        soundFinished();
    switch (m_digiMode) {
    case sds_PC:
        break;
    case sds_SoundBlaster:
        break;
    default:
        break;
    }
}

void WLAudio::soundFinished()
{
    m_soundNumber   = 0;
    m_soundPriority = 0;
}

qint16 WLAudio::getSample(float csample, quint8 *samples, int size)
{
    float s0=0, s1=0, s2=0;
    int cursample = (int) csample;
    float sf = csample - (float) cursample;

    if(cursample-1 >= 0) s0 = (float) (samples[cursample-1] - 128);
    s1 = (float) (samples[cursample] - 128);
    if(cursample+1 < size) s2 = (float) (samples[cursample+1] - 128);

    float val = s0*sf*(sf-1)/2 - s1*(sf*sf-1) + s2*(sf+1)*sf/2;
    qint32 intval = (qint32) (val * 256);
    if(intval < -32768) intval = -32768;
    else if(intval > 32767) intval = 32767;
    return (qint16) intval;
}

void WLAudio::channelFinished(qint32 channel)
{
    m_channelSoundPos[channel].valid = 0;
}

void WLAudio::setupDigi()
{
    quint16 *soundInfoPage=(quint16*)wlpage->page(wlpage->totalChunks()-1);
    m_numDigi = (quint16)wlpage->pageSize(wlpage->totalChunks()-1)/4;
    m_digiList = (digiInfo_t*) malloc(m_numDigi * sizeof(digiInfo_t));
    int i;
    for(i = 0; i < m_numDigi; i++) {

        m_digiList[i].startpage = soundInfoPage[i * 2];
        if((int) m_digiList[i].startpage >= (int)wlpage->totalChunks() - 1) {
            m_numDigi = i;
            break;
        }
        int lastPage;
        if(i < m_numDigi - 1) {
            lastPage = soundInfoPage[i * 2 + 2];
            if(lastPage == 0 || lastPage + (int)wlpage->soundStart() > (int)wlpage->totalChunks() - 1) lastPage = wlpage->totalChunks() - 1;
            else lastPage += wlpage->soundStart();
        } else lastPage = wlpage->totalChunks() - 1;

        int size = 0;
        for(int page = (int)wlpage->soundStart() + m_digiList[i].startpage; page < lastPage; page++)
        size += wlpage->pageSize(page);
        if(lastPage == (int)wlpage->totalChunks() - 1 && (int)wlpage->soundInfoPagePadded()) size--;
        if((size & 0xffff0000) != 0 && (size & 0xffff) < soundInfoPage[i * 2 + 1])
            size -= 0x10000;
        size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];
        m_digiList[i].length = size;
    }
    for(i = 0; i < (qint32)wlcache->findSound("LASTSOUND"); i++) {
        m_digiMap[i] = -1;
        m_digiChannel[i] = -1;
    }
}

void WLAudio::alStopSound()
{
    g_alSound = 0;
    alOut(m_alFreqH + 0, 0);
}

void WLAudio::alSetFXInst(Instrument_t *inst)
{
    quint8 c,m;

    m = 0;      // modulator cell for channel 0
    c = 3;      // carrier cell for channel 0
    alOut(m + m_alChar,inst->mChar);
    alOut(m + m_alScale,inst->mScale);
    alOut(m + m_alAttack,inst->mAttack);
    alOut(m + m_alSus,inst->mSus);
    alOut(m + m_alWave,inst->mWave);
    alOut(c + m_alChar,inst->cChar);
    alOut(c + m_alScale,inst->cScale);
    alOut(c + m_alAttack,inst->cAttack);
    alOut(c + m_alSus,inst->cSus);
    alOut(c + m_alWave,inst->cWave);
    alOut(m_alFeedCon,0);
}

void WLAudio::alPlaySound(AdLibSound_t *sound)
{
    Instrument_t      *inst;
    quint8            *data;

    alStopSound();
    m_alLengthLeft = sound->common.length;
    data = sound->data;
    m_alBlock = ((sound->block & 7) << 2) | 0x20;
    inst = &sound->inst;

    if (!(inst->mSus | inst->cSus)) {
        Quit("alPlaySound() - Bad instrument");
    }
    alSetFXInst(inst);
    g_alSound = (quint8*)data;
}

void WLAudio::shutAl()
{
    g_alSound = 0;
    alOut(m_alEffects,0);
    alOut(m_alFreqH + 0,0);
    alSetFXInst(&m_alZeroInst);
}

void WLAudio::startAl()
{
    alOut(m_alEffects, 0);
    alSetFXInst(&m_alZeroInst);
}

void WLAudio::shutDevice()
{
    switch (m_soundMode)
    {
    case sdm_PC:
        break;
    case sdm_AdLib:
        shutAl();
        break;
    default:
        break;
    }
    m_soundMode = sdm_Off;
}

void WLAudio::startDevice()
{
    switch (m_soundMode) {
    case sdm_AdLib:
        startAl();
        break;
    default:
        break;
    }
    m_soundNumber = 0;
    m_soundPriority = 0;
}

void WLAudio::musicPlayer(void *udata, quint8 *stream, int len)
{
    Q_UNUSED(udata)

    int stereolen = len>>1;
    int sampleslen = stereolen>>1;
    qint16 *stream16 = (qint16*) (void *) stream;    // expect correct alignment

    while(1) {
        if(m_numReadySamples) {
            if(m_numReadySamples < sampleslen) {
                YM3812UpdateOne(0, stream16, m_numReadySamples);
                stream16 += m_numReadySamples * 2;
                sampleslen -= m_numReadySamples;
            } else {
                YM3812UpdateOne(0, stream16, sampleslen);
                m_numReadySamples -= sampleslen;
                return;
            }
        }
        m_soundTimeCounter--;
        if(!m_soundTimeCounter) {
            m_soundTimeCounter = 5;
            if(m_curAlSound != g_alSound) {
                m_curAlSound = m_curAlSoundPtr = g_alSound;
                m_curAlLengthLeft = m_alLengthLeft;
            }
            if(m_curAlSound) {
                if(*m_curAlSoundPtr) {
                    alOut(m_alFreqL, *m_curAlSoundPtr);
                    alOut(m_alFreqH, m_alBlock);
                } else alOut(m_alFreqH, 0);
                m_curAlSoundPtr++;
                m_curAlLengthLeft--;
                if(!m_curAlLengthLeft) {
                    m_curAlSound = g_alSound = 0;
                    m_soundNumber = 0;
                    m_soundPriority = 0;
                    alOut(m_alFreqH, 0);
                }
            }
        }
        if(m_sqActive) {
            do {
                if(m_sqHackTime > m_alTimeCount) break;
                m_sqHackTime = m_alTimeCount + *(m_sqHackPtr+1);
                alOut(*(quint8*)m_sqHackPtr, *(((quint8*)m_sqHackPtr)+1));
                m_sqHackPtr += 2;
                m_sqHackLen -= 4;
            }
            while(m_sqHackLen > 0);
            m_alTimeCount++;
            if(!m_sqHackLen) {
                m_sqHackPtr = m_sqHack;
                m_sqHackLen = m_sqHackSeqLen;
                m_sqHackTime = 0;
                m_alTimeCount = 0;
            }
        }
        m_numReadySamples = m_samplesPerMusicTick;
    }
}
