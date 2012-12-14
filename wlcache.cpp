#include <QDebug>

#include "wl_def.h"

#include "wlcache.h"

#include "wlaudio.h"
#include "wlpaint.h"

extern WLAudio* wlaudio;
extern WLPaint* wlpaint;

extern QImage*  g_Image;
extern QString* g_graphic;
extern QString* g_sound;
extern QString* g_music;

static const char gheadname[] = "vgahead.";
static const char gfilename[] = "vgagraph.";
static const char gdictname[] = "vgadict.";
static const char mheadname[] = "maphead.";
static const char mfilename[] = "maptemp.";
static const char aheadname[] = "audiohed.";
static const char afilename[] = "audiot.";

WLCache::WLCache(qint32 chunks, qint32 sounds)
{
    if (g_Extension.toLower().contains("wl1"))
        m_missingEpisodes = 5;
    else
        m_missingEpisodes = 0;

    m_graphicFile = 0;
    m_mapFile     = 0;
    m_audioFile   = 0;

    m_chunks = chunks;
    m_sounds = sounds;

    oldsoundmode  = (SDMode)0;

    for (int i = 0; !g_sound[i+1].startsWith("LASTSOUND"); i++)
        m_lastSound = i + 1;
    for (int i = 0; !g_music[i+1].startsWith("LASTMUSIC"); i++)
        m_lastMusic = i + 1;
    for (int i = 0; !g_graphic[i+1].startsWith("ENUMEND"); i++)
        m_lastGraphic = i + 1;

    memset(&m_graphicData , 0, sizeof(qint8*)*chunks);
    memset(&m_audioData, 0, sizeof(quint8*)*sounds);
    memset(&m_graphicOffsets, 0, sizeof(qint32)*200);
    memset(&m_graphicData, 0 , sizeof(quint8*)*200);
    memset(&m_mapFog, 0, 64*64);

    setupMapFile();
    setupGraphicFile();
    setupAudioFile();

    m_currentMap = -1;
}

WLCache::~WLCache()
{
    int i,start = 0;

    if(m_mapFile) {
        if (m_mapFile->isOpen()) m_mapFile->close();
        delete m_mapFile;
    }
    if(m_graphicFile) {
        if (m_graphicFile->isOpen()) m_graphicFile->close();
        delete m_graphicFile;
    }
    if(m_audioFile) {
        if (m_audioFile->isOpen()) m_audioFile->close();
        delete m_audioFile;
    }

    //TODO: CRASH! for(i=0; i<NUMCHUNKS; i++) uncacheGraphic(i);
    free(pictable);

    switch(oldsoundmode) {
    case sdm_Off:
        return;
    case sdm_PC:
        start = 0;
        break;
    case sdm_AdLib:
        start = m_lastSound;
        break;
    }
    for(i = 0; i < m_lastSound; i++, start++) uncacheAudio(start);
}

quint32  WLCache::find(const QString& name)
{
    if (name.startsWith("ENUMEND"))
        return m_lastGraphic;

    for (int i = 0; i < m_lastGraphic + 1; i++) {
        if (g_graphic[i].startsWith(name))
            return i;
    }
    return 0;
}

quint32  WLCache::findSound(const QString& name)
{
    if (name.startsWith("LASTSOUND"))
        return m_lastSound;

    for (int i = 0; i < m_lastSound + 1; i++) {
        if (g_sound[i].startsWith(name))
            return i;
    }
    return 0;
}

quint32  WLCache::findMusic(const QString& name)
{
    if (name.startsWith("LASTMUSIC"))
        return m_lastMusic;

    for (int i = 0; i < m_lastMusic + 1; i++) {
        if (g_music[i].startsWith(name))
            return i;
    }
    return 0;
}

void WLCache::cacheGraphic(int idx)
{
    if (m_graphicData[idx])
        return;

    qint32 pos = graphicPos(idx);
    if (pos < 0) return;

    int next = idx + 1;
    while (graphicPos(next) == -1) next++;

    qint32* source = 0;
    qint32 compressed = graphicPos(next)-pos;

    m_graphicFile->seek(pos);

    if (compressed <= bufferSize) {
        m_graphicFile->read((char*)m_workingBuffer,compressed);
        source = m_workingBuffer;
    } else {
        source = (qint32 *) malloc(compressed);
        if (!source) {
            qWarning()<<QString("WLCache::cacheGraphic(%1) : Out of Memory!").arg(idx);
            exit(1);
        }
        m_graphicFile->read((char*)source,compressed);
    }
    expandGraphic(idx,source);

    if (compressed > bufferSize) free(source);
}

void WLCache::uncacheGraphic(int idx)
{
    if (m_graphicData[idx]) {
        free(m_graphicData[idx]);
        m_graphicData[idx] = 0;
    }
}

void WLCache::cacheAdlib(int idx)
{
    if (m_audioData[idx]) return;

    qint32 pos  = m_audioOffsets[idx];
    qint32 size = m_audioOffsets[idx+1]-pos;

    m_audioFile->seek(pos);
    m_audioFile->read((char*)m_workingBuffer, (6 + 16 + 2)-1);

    AdLibSound_t *sound = (AdLibSound_t*)malloc(size+sizeof(AdLibSound_t)-(6 + 16 + 2));
    if (!sound) {
        qWarning()<<QString("WLCache::cacheAdlib(%1) : Out of Memory!").arg(idx);
        exit(1);
    }

    quint8 *ptr = (quint8*) m_workingBuffer;
    sound->common.length = READLONGWORD(ptr);
    sound->common.priority = READWORD(ptr);
    sound->inst.mChar = *ptr++;
    sound->inst.cChar = *ptr++;
    sound->inst.mScale = *ptr++;
    sound->inst.cScale = *ptr++;
    sound->inst.mAttack = *ptr++;
    sound->inst.cAttack = *ptr++;
    sound->inst.mSus = *ptr++;
    sound->inst.cSus = *ptr++;
    sound->inst.mWave = *ptr++;
    sound->inst.cWave = *ptr++;
    sound->inst.nConn = *ptr++;
    sound->inst.voice = *ptr++;
    sound->inst.mode = *ptr++;
    sound->inst.unused[0] = *ptr++;
    sound->inst.unused[1] = *ptr++;
    sound->inst.unused[2] = *ptr++;
    sound->block = *ptr++;

    m_audioFile->read((char*)sound->data,size-(6 + 16 + 2)+1);

    m_audioData[idx]=(quint8*)sound;
}

qint32 WLCache::cacheAudio(int idx)
{
    qint32 pos = m_audioOffsets[idx];
    qint32 size = m_audioOffsets[idx+1]-pos;

    if (m_audioData[idx]) return size;

    m_audioData[idx] = (quint8*)malloc(size);
    if (!m_audioData[idx]) {
        qWarning()<<QString("WLCache::cacheAudio(%1) : Out of Memory!").arg(idx);
        exit(1);
    }

    m_audioFile->seek(pos);
    m_audioFile->read((char*)m_audioData[idx], size);

    return size;
}

void WLCache::uncacheAudio(int idx)
{
    if (m_audioData[idx]) {
        free(m_audioData[idx]);
        m_audioData[idx] = 0;
    }
}

void WLCache::loadAllSounds()
{
    bool    skip = false;
    quint32 start = 0;

    switch (oldsoundmode) {
    case sdm_Off:
        skip = true;
    case sdm_PC:
        start = 0;
        break;
    case sdm_AdLib:
        start = m_lastSound;
        break;
    }
    if (!skip)
        for (int i=0; i < m_lastSound; i++,start++) uncacheAudio(start);

    oldsoundmode = *wlaudio->soundMode();

    switch (*wlaudio->soundMode()) {
    case sdm_Off:
        start = m_lastSound;
        break;
    case sdm_PC:
        start = 0;
        break;
    case sdm_AdLib:
        start = m_lastSound;
        break;
    }

    if((qint32)start == m_lastSound)
        for (int i = 0; i < m_lastSound; i++, start++) cacheAdlib(start);
    else
        for (int i = 0; i < m_lastSound; i++, start++) cacheAudio(start);
}

void WLCache::cacheMap(int idx)
{
    quint32   size = maparea * 2;

    m_currentMap = idx;

    for (int plane = 0; plane < mapPlanes; plane++) {
        qint32  pos = m_mapInfo[idx]->start[plane];
        qint32  compressed = m_mapInfo[idx]->length[plane];
        qint16* dest = (qint16*)m_mapData[plane];
        qint16* source = 0;
        void* bigBuffer = 0;

        m_mapFile->seek(pos);
        if (compressed <= bufferSize)
            source = (qint16*)m_workingBuffer;
        else {
            bigBuffer = malloc(compressed);
            if (!bigBuffer) {
                qWarning()<<QString("WLCache::cacheMap(%1) : Out of Memory(1)!").arg(idx);
                exit(1);
            }
            source = (qint16*)bigBuffer;
        }
        m_mapFile->read((char*)source,compressed);
        qint32 expanded = *source;
        source++;
        qint16* tmpBuffer = (qint16*)malloc(expanded);
        if (!tmpBuffer) {
            qWarning()<<QString("WLCache::cacheMap(%1) : Out of Memory(2)!").arg(idx);
            exit(1);
        }
        carmackExpand((quint8*)source, (quint16*)tmpBuffer, expanded);
        expand((quint16*)(tmpBuffer+1),(quint16*)dest,size,m_RLEWtag);
        free(tmpBuffer);
        if (compressed > bufferSize) free(bigBuffer);
    }
}

void WLCache::cacheScreen(int idx)
{
    qint32 pos  = graphicPos(idx);
    int    next = idx + 1;

    while (graphicPos(next) == -1) next++;

    qint32 compressed = graphicPos(next)-pos;
    m_graphicFile->seek(pos);

    void* bigBuffer = malloc(compressed);
    if (!bigBuffer) {
        qWarning()<<QString("WLCache::cacheScreen(%1) : Out of Memory!").arg(idx);
        exit(1);
    }
    m_graphicFile->read((char*)bigBuffer,compressed);
    qint32* source = (qint32*) bigBuffer;
    qint32 expanded = *source++;

    char *pic = (char*)malloc(64000);
    if (!pic) {
        qWarning()<<QString("WLCache::cacheScreen(%1) : Out of Memory!").arg(idx);
        exit(1);
    }
    huffExpand((quint8*)source, (quint8*)pic, expanded, m_graphicHuffman);

    quint8 *vbuf = g_Image->bits();
    for(int y = 0, scy = 0; y < 200; y++, scy += scaleFactor) {
        for(int x = 0, scx = 0; x < 320; x++, scx += scaleFactor) {
            quint8 col = pic[(y * 80 + (x >> 2)) + (x & 3) * 80 * 200];
            for(quint32 i = 0; i < scaleFactor; i++)
                for(quint32 j = 0; j < scaleFactor; j++)
                    vbuf[(scy + i) * g_Image->bytesPerLine() + scx + j] = col;
        }
    }
    free(pic);
    free(bigBuffer);
}

bool WLCache::load(const QString& filename, void **ptr)
{   
    QString fileName = QString(filename);
    QFile fp(fileName);

    if (!fp.open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::load() : cannot open file : "<<filename;
        return false;
    }
    *ptr = malloc(fp.size());
    if (!*ptr) {
        qWarning()<<"WLCache::load() : Out of Memory!";
        exit(1);
    }
    if (!fp.read((char*)*ptr, fp.size())) {
        qWarning()<<"WLCache::load() : cannot read from "<<filename;
        fp.close();
        return false;
    }
    fp.close();
    return true;
}

bool WLCache::write(const QString& filename, void *ptr, qint32 length)
{
    QFile fp(filename);

    if (!fp.open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::write() : cannot open file "<<filename;
        return false;
    }

    if (!fp.write((char*)ptr, length)) {
        qWarning()<<"WLCache::write() : cannot write to file "<<filename;
        fp.close();
        return false;
    }
    fp.close();

    return true;
}

void WLCache::graphicLength(int idx)
{;
    m_graphicFile->seek(graphicPos(idx));
    m_graphicFile->read((char*)&m_uncompressedLength, sizeof(m_uncompressedLength));
    m_compressedLength = graphicPos(idx+1)-graphicPos(idx)-4;
}

void WLCache::huffExpand(quint8 *source, quint8 *dest, qint32 length, huffnode_t *hufftable)
{
    quint8 *end;
    huffnode_t *headptr, *huffptr;

    if(!length || !dest) {
        Quit("length or dest is null!");
        return;
    }

    headptr = hufftable+254; // head node is always node 254

    int written = 0;

    end = dest+length;

    quint8 val = *source++;
    quint8 mask = 1;
    quint16 nodeval;
    huffptr = headptr;
    while(1) {
        if(!(val & mask))
            nodeval = huffptr->bit0;
        else
            nodeval = huffptr->bit1;
        if(mask == 0x80) {
            val = *source++;
            mask = 1;
        }
        else mask <<= 1;

        if(nodeval<256) {
            *dest++ = (quint8) nodeval;
            written++;
            huffptr = headptr;
            if(dest >= end) break;
        } else {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

void WLCache::setupMapFile()
{
    mapfiletype_t *tinf = 0;

    QString fileName = mheadname + g_Extension;

    m_mapFile = new QFile(fileName);

    if (m_mapFile->open(QIODevice::ReadOnly)) {
        qint32 length = numMaps * 4 + 2;
        tinf = (mapfiletype_t*)malloc(sizeof(mapfiletype_t));
        if (!tinf) {
            qWarning()<<"WLCache::setupMapFile() : Out of Memory!";
            exit(1);
        }
        m_mapFile->read((char*)tinf, length);
        m_mapFile->close();

        m_RLEWtag = tinf->RLEWtag;

    } else {
        qWarning()<<"WLCache::setupMapFile() : cannot open file "<<fileName;
        exit(1);
    }

    fileName = "gamemaps." + g_Extension;

    m_mapFile->setFileName(fileName);

    if (m_mapFile->open(QIODevice::ReadOnly)) {
        for (int i=0; i < numMaps; i++) {
            qint32 pos = tinf->headeroffsets[i];
            if (pos < 0) continue;
            m_mapInfo[i] = (maptype_t*)malloc(sizeof(maptype_t));
            if (!m_mapInfo[i]) {
                qWarning()<<"WLCache::setupMapFile() : Out of Memory!";
                exit(1);
            }
            bool ok = false;
            if (m_mapFile->seek(pos)) {
                if (m_mapFile->read((char*)m_mapInfo[i],sizeof(maptype_t)) == sizeof(maptype_t)) {
                    ok = true;
                    if (!ok) {
                        qWarning()<<"WLCache::setupmapFile() : Failed to load map "<<i;
                        exit(1);
                    }
                }
            }
        }
        free(tinf);
        for (int i=0; i < mapPlanes; i++) {
            m_mapData[i] = (quint16*)malloc(maparea*2);
            if (!m_mapData[i]) {
                qWarning()<<"WLCache::setupMapFile() : Out of Memory!";
                exit(1);
            }
        }
    } else {
        qWarning()<<"WLCache::setupMapFile() : cannot open file : "<<fileName;
        exit(1);
    }
}

void WLCache::setupGraphicFile()
{
    // Get huffman dictionary
    QString fileName = gdictname + g_Extension;

    m_graphicFile = new QFile(fileName);
    if (!m_graphicFile->open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::setupGraphicFile() : cannot open dictionary file : "<<fileName;
        exit(1);
    }
    m_graphicFile->read((char*)m_graphicHuffman, sizeof(m_graphicHuffman));
    m_graphicFile->close();

    // load the data offsets from graphic header file
    fileName = gheadname + g_Extension;

    m_graphicFile->setFileName(fileName);
    if (!m_graphicFile->open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::setupGraphicFile() : cannot open graphic header file : "<<fileName;
        exit(1);
    }
    qint64 headerSize = m_graphicFile->size();
    quint8 data[headerSize];
    m_graphicFile->read((char*)data, sizeof(data));
    m_graphicFile->close();

    const quint8* d = data;
    for (qint32* i = m_graphicOffsets; i != m_graphicOffsets + headerSize/3; ++i) {
        const qint32 val = d[0] | d[1] << 8 | d[2] << 16;
        *i = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }

    // load pic/sprite info
    fileName = gfilename + g_Extension;

    m_graphicFile->setFileName(fileName);

    if (!m_graphicFile->open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::setupGraphicFile() : cannot open graphic file : "<<fileName;
        exit(1);
    }
    pictable = (pictabletype *)malloc(find("GETPSYCHEDPIC") * sizeof(pictabletype));
    if (!pictable) {
        qWarning()<<"WLCache::setupGraphicFile() : Out of Memory!";
        exit(1);
    }
    graphicLength(STRUCTPIC);  // position file pointer
    char tmpBuffer[m_compressedLength];
    m_graphicFile->read((char*)&tmpBuffer, m_compressedLength);
    huffExpand((quint8*)&tmpBuffer, (quint8*)pictable, find("GETPSYCHEDPIC") * sizeof(pictabletype), m_graphicHuffman);
}

void WLCache::setupAudioFile()
{
    QString fileName = aheadname + g_Extension;

    void* ptr;
    if (!load(fileName, &ptr)) {
        qWarning()<<"WLCache::setupAudioFile() : cannot load file: "<<fileName;
        exit(1);
    }
    m_audioOffsets = (qint32*)ptr;

    fileName = afilename + g_Extension;

    m_audioFile = new QFile(fileName);

    if (!m_audioFile->open(QIODevice::ReadOnly)) {
        qWarning()<<"WLCache::setupAudioFile() : cannot open file : "<<fileName;
        exit(1);
    }
}

void WLCache::carmackExpand(quint8 *source, quint16 *dest, int length)
{
    quint16 ch,chhigh,count,offset;
    quint8 *inptr;
    quint16 *copyptr, *outptr;

    length/=2;

    inptr = (quint8*) source;
    outptr = dest;

    while (length > 0) {
        ch = READWORD(inptr);
        chhigh = ch>>8;
        if (chhigh == NEARTAG) {
            count = ch&0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            } else {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;
                if(length < 0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        } else if (chhigh == FARTAG) {
            count = ch&0xff;
            if (!count) { // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr++ = ch;
                length --;
            } else {
                offset = READWORD(inptr);
                copyptr = dest + offset;
                length -= count;
                if(length < 0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        } else {
            *outptr++ = ch;
            length --;
        }
    }
}

void WLCache::expandGraphic(int idx, qint32 *source)
{
    quint32 chunk = idx;

    qint32    expanded;

    if (chunk >= find("TILE8") && chunk < find("ORDERSCREEN")) {

#define BLOCK           64
#define MASKBLOCK       128

        if (chunk < find("ORDERSCREEN"))          // tile 8s are all in one chunk!
            expanded = BLOCK*NUMTILE8;
        else if (chunk < find("ORDERSCREEN"))
            expanded = MASKBLOCK*NUMTILE8M;
        else if (chunk < find("ORDERSCREEN"))    // all other tiles are one/chunk
            expanded = BLOCK*4;
        else if (chunk < find("ORDERSCREEN"))
            expanded = MASKBLOCK*4;
        else if (chunk < find("ORDERSCREEN"))
            expanded = BLOCK*16;
        else
            expanded = MASKBLOCK*16;
    } else {
        expanded = *source++;
    }
    m_graphicData[chunk]=(quint8*)malloc(expanded);
    CHECKMALLOCRESULT(m_graphicData[chunk]);
    huffExpand((quint8*)source, m_graphicData[chunk], expanded, m_graphicHuffman);
}

qint32 WLCache::compress(quint16 *source, qint32 length, quint16 *dest, quint16 rlewtag)
{
    quint16 value,count;
    unsigned i;
    quint16 *start,*end;

    start = dest;

    end = source + (length+1)/2;

    do {
        count = 1;
        value = *source++;
        while (*source == value && source < end) {
            count++;
            source++;
        }
        if (count > 3 || value == rlewtag) {
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        } else {
            for (i=1;i<=count;i++) *dest++ = value;
        }
    } while (source < end);

    return (qint32)(2*(dest-start));
}

void WLCache::expand(quint16 *source, quint16 *dest, qint32 length, quint16 rlewtag)
{
    quint16 value,count,i;
    quint16 *end=dest+length/2;

    do {
        value = *source++;
        if (value != rlewtag)
            *dest++=value;
        else {
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++) *dest++ = value;
        }
    } while (dest < end);
}







