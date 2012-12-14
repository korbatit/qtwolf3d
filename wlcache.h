#ifndef WLCACHE_H
#define WLCACHE_H

#include "wl_def.h"

#include "wlaudio.h"

#include <QFile>
#include <QString>

#define NEARTAG 0xa7
#define FARTAG  0xa8

extern QString g_Extension;

const int bufferSize = 0x1000;

const int mapShift   = 6;
const int numMaps    = 60;
const int mapPlanes  = 2;

typedef struct {
    qint32  start[3];
    qint16  length[3];
    qint16  width;
    qint16  height;
    char    name[16];
} maptype_t;

typedef struct {
    qint16 bit0;
    qint16 bit1;
} huffnode_t;

typedef struct {
    qint16  RLEWtag;
    qint32 headeroffsets[100];
} mapfiletype_t;

static QString wl1Graphic[162] =
{
    "", "", "", "BJPIC", "H_CASTLEPIC",                                                              // 0   - 4
    "H_KEYBOARDPIC", "H_JOYPIC", "H_HEALPIC", "H_TREASUREPIC", "H_GUNPIC",                            // 5   - 9
    "H_KEYPIC", "H_BLAZEPIC", "H_WEAPON1234PIC", "H_WOLFLOGOPIC", "H_VISAPIC",                       // 10  - 14
    "H_MCPIC", "H_IDLOGOPIC", "H_TOPWINDOWPIC", "H_LEFTWINDOWPIC", "H_RIGHTWINDOWPIC",               // 15  - 19
    "H_BOTTOMINFOPIC", "H_SPEARADPIC", "C_OPTIONSPIC", "C_CURSOR1PIC", "C_CURSOR2PIC",               // 20  - 24
    "C_NOTSELECTEDPIC", "C_SELECTEDPIC", "C_FXTITLEPIC", "C_DIGITITLEPIC", "C_MUSICTITLEPIC",        // 25  - 29
    "C_MOUSELBACKPIC", "C_BABYMODEPIC", "C_EASYPIC", "C_NORMALPIC", "C_HARDPIC",                     // 30  - 34
    "C_LOADSAVEDISKPIC", "C_DISKLOADING1PIC", "C_DISKLOADING2PIC", "C_CONTROLPIC", "C_CUSTOMIZEPIC", // 35  - 39
    "C_LOADGAMEPIC", "C_SAVEGAMEPIC", "C_EPISODE1PIC", "C_EPISODE2PIC", "C_EPISODE3PIC",             // 40  - 44
    "C_EPISODE4PIC", "C_EPISODE5PIC", "C_EPISODE6PIC", "C_CODEPIC", "C_TIMECODEPIC",                 // 45  - 49
    "C_LEVELPIC", "C_NAMEPIC", "C_SCOREPIC", "C_JOY1PIC", "C_JOY2PIC",                               // 50  - 54
    "L_GUYPIC", "L_COLONPIC", "L_NUM0PIC", "L_NUM1PIC", "L_NUM2PIC",                                 // 55  - 59
    "L_NUM3PIC", "L_NUM4PIC", "L_NUM5PIC", "L_NUM6PIC", "L_NUM7PIC",                                 // 60  - 64
    "L_NUM8PIC", "L_NUM9PIC", "L_PERCENTPIC", "L_APIC", "L_BPIC",                                    // 65  - 69
    "L_CPIC", "L_DPIC", "L_EPIC", "L_FPIC", "L_GPIC",                                                // 70  - 74
    "L_HPIC", "L_IPIC", "L_JPIC", "L_KPIC", "L_LPIC",                                                // 75  - 79
    "L_MPIC", "L_NPIC", "L_OPIC", "L_PPIC", "L_QPIC",                                                // 80  - 84
    "L_RPIC", "L_SPIC", "L_TPIC", "L_UPIC", "L_VPIC",                                                // 85  - 89
    "L_WPIC", "L_XPIC", "L_YPIC", "L_ZPIC", "L_EXPOINTPIC",                                          // 90  - 94
    "L_APOSTROPHEPIC", "L_GUY2PIC", "L_BJWINSPIC", "STATUSBARPIC", "TITLEPIC",                       // 95  - 99
    "PG13PIC", "CREDITSPIC", "HIGHSCORESPIC", "KNIFEPIC", "GUNPIC",                                  // 100 - 104
    "MACHINEGUNPIC", "GATLINGGUNPIC", "NOKEYPIC", "GOLDKEYPIC", "SILVERKEYPIC",                      // 105 - 109
    "N_BLANKPIC", "N_0PIC", "N_1PIC", "N_2PIC", "N_3PIC",                                            // 110 - 114
    "N_4PIC", "N_5PIC", "N_6PIC", "N_7PIC", "N_8PIC",                                                // 115 - 119
    "N_9PIC", "FACE1APIC", "FACE1BPIC", "FACE1CPIC", "FACE2APIC",                                    // 120 - 124
    "FACE2BPIC", "FACE2CPIC", "FACE3APIC", "FACE3BPIC", "FACE3CPIC",                                 // 125 - 129
    "FACE4APIC", "FACE4BPIC", "FACE4CPIC", "FACE5APIC", "FACE5BPIC",                                 // 130 - 134
    "FACE5CPIC", "FACE6APIC", "FACE6BPIC", "FACE6CPIC", "FACE7APIC",                                 // 135 - 139
    "FACE7BPIC", "FACE7CPIC", "FACE8APIC", "GOTGATLINGPIC", "MUTANTBJPIC",                           // 140 - 144
    "PAUSEDPIC", "GETPSYCHEDPIC", "TILE8", "ORDERSCREEN", "ERRORSCREEN",                             // 145 - 149
    "T_HELPART", "T_DEMO0", "T_DEMO1", "T_DEMO2", "T_DEMO3",                                         // 150 - 154
    "T_ENDART1", "T_ENDART2", "T_ENDART3", "T_ENDART4", "T_ENDART5",                                 // 155 - 159
    "T_ENDART6", "ENUMEND"                                                                           // 160 - 161
};

static QString wl6Graphic[160] =
{
    "", "", "", "BJPIC", "H_CASTLEPIC",                                                              // 0   - 4   *
    "H_KEYBOARDPIC", "H_JOYPIC", "H_HEALPIC", "H_TREASUREPIC", "H_GUNPIC",                           // 5   - 9   *
    "H_KEYPIC", "H_BLAZEPIC", "H_WEAPON1234PIC", "H_WOLFLOGOPIC", "H_VISAPIC",                       // 10  - 14  *
    "H_MCPIC", "H_IDLOGOPIC", "H_TOPWINDOWPIC", "H_LEFTWINDOWPIC", "H_RIGHTWINDOWPIC",               // 15  - 19  *
    "H_BOTTOMINFOPIC", "C_OPTIONSPIC", "C_CURSOR1PIC", "C_CURSOR2PIC", "C_NOTSELECTEDPIC",           // 20  - 24  *
    "C_SELECTEDPIC", "C_FXTITLEPIC", "C_DIGITITLEPIC", "C_MUSICTITLEPIC", "C_MOUSELBACKPIC",         // 25  - 29  *
    "C_BABYMODEPIC", "C_EASYPIC", "C_NORMALPIC", "C_HARDPIC", "C_LOADSAVEDISKPIC",                   // 30  - 34  *
    "C_DISKLOADING1PIC", "C_DISKLOADING2PIC", "C_CONTROLPIC", "C_CUSTOMIZEPIC", "C_LOADGAMEPIC",     // 35  - 39  *
    "C_SAVEGAMEPIC", "C_EPISODE1PIC", "C_EPISODE2PIC", "C_EPISODE3PIC", "C_EPISODE4PIC",             // 40  - 44  *
    "C_EPISODE5PIC", "C_EPISODE6PIC", "C_CODEPIC", "C_TIMECODEPIC", "C_LEVELPIC",                    // 45  - 49  *
    "C_NAMEPIC", "C_SCOREPIC", "L_GUYPIC", "L_COLONPIC", "L_NUM0PIC",                                // 50  - 54  *
    "L_NUM1PIC", "L_NUM2PIC", "L_NUM3PIC", "L_NUM4PIC", "L_NUM5PIC",                                 // 55  - 59  *
    "L_NUM6PIC", "L_NUM7PIC", "L_NUM8PIC", "L_NUM9PIC", "L_PERCENTPIC",                              // 60  - 64  *
    "L_APIC", "L_BPIC", "L_CPIC", "L_DPIC", "L_EPIC",                                                // 65  - 69  *
    "L_FPIC", "L_GPIC", "L_HPIC", "L_IPIC", "L_JPIC",                                                // 70  - 74  *
    "L_KPIC", "L_LPIC", "L_MPIC", "L_NPIC", "L_OPIC",                                                // 75  - 79  *
    "L_PPIC", "L_QPIC", "L_RPIC", "L_SPIC", "L_TPIC",                                                // 80  - 84  *
    "L_UPIC", "L_VPIC", "L_WPIC", "L_XPIC", "L_YPIC",                                                // 85  - 89  *
    "L_ZPIC", "L_EXPOINTPIC", "L_APOSTROPHEPIC", "L_GUY2PIC", "L_BJWINSPIC",                         // 90  - 94  *
    "STATUSBARPIC", "TITLEPIC", "PG13PIC", "CREDITSPIC", "HIGHSCORESPIC",                            // 95  - 99  *
    "KNIFEPIC", "GUNPIC", "MACHINEGUNPIC", "GATLINGGUNPIC", "NOKEYPIC",                              // 100 - 104 *
    "GOLDKEYPIC", "SILVERKEYPIC", "N_BLANKPIC", "N_0PIC", "N_1PIC",                                  // 105 - 109 *
    "N_2PIC", "N_3PIC", "N_4PIC", "N_5PIC", "N_6PIC",                                                // 110 - 114 *
    "N_7PIC", "N_8PIC", "N_9PIC", "FACE1APIC", "FACE1BPIC",                                          // 115 - 119 *
    "FACE1CPIC", "FACE2APIC", "FACE2BPIC", "FACE2CPIC", "FACE3APIC",                                 // 120 - 124 *
    "FACE3BPIC", "FACE3CPIC", "FACE4APIC", "FACE4BPIC", "FACE4CPIC",                                 // 125 - 129 *
    "FACE5APIC", "FACE5BPIC", "FACE5CPIC", "FACE6APIC", "FACE6BPIC",                                 // 130 - 134 *
    "FACE6CPIC", "FACE7APIC", "FACE7BPIC", "FACE7CPIC", "FACE8APIC",                                 // 135 - 139 *
    "GOTGATLINGPIC", "MUTANTBJPIC", "PAUSEDPIC", "GETPSYCHEDPIC", "TILE8",                           // 140 - 144
    "ORDERSCREEN", "ERRORSCREEN", "T_HELPART", "T_DEMO0", "T_DEMO1",                                 // 145 - 149
    "T_DEMO2", "T_DEMO3", "T_ENDART1", "T_ENDART2", "T_ENDART3",                                     // 150 - 154
    "T_ENDART4", "T_ENDART5", "T_ENDART6", "ENUMEND"                                                 // 155 - 159
};

static QString sodGraphic[170] =
{
    " ", " ", " ", "C_BACKDROPPIC", "C_MOUSELBACKPIC",                                               // 0   - 4
    "C_CURSOR1PIC", "C_CURSOR2PIC", "C_NOTSELECTEDPIC", "C_SELECTEDPIC", "C_CUSTOMIZEPIC",            // 5   - 9
    "C_JOY1PIC", "C_JOY2PIC", "C_MOUSEPIC", "C_JOYSTICKPIC", "C_KEYBOARDPIC",                        // 10  - 14
    "C_CONTROLPIC", "C_OPTIONSIC", "C_FXTITLEPIC", "C_DIGITITLEPIC", "C_MUSICTITLEPIC",              // 15  - 19
    "C_HOWTOUGHPIC", "C_BABYMODEPIC", "C_EASYPIC", "C_NORMALPIC", "C_HARDPIC",                       // 20  - 24
    "C_DISKLOADING1PIC", "C_DISKLOADING2PIC", "C_LOADGAMEPIC", "C_SAVEGAMEPIC", "HIGHSCORESPIC",     // 25  - 29
    "C_WONSPEARPIC", "BJCOLLAPSE1PIC", "BJCOLLAPSE2PIC", "BJCOLLAPSE3PIC", "BJCOLLAPSE4PIC",         // 30  - 34
    "ENDPICPIC", "L_GUYPIC", "L_COLONPIC", "L_NUM0PIC", "L_NUM1PIC",                                 // 35  - 39
    "L_NUM2PIC", "L_NUM3PIC", "L_NUM4PIC", "L_NUM5PIC", "L_NUM6PIC",                                 // 40  - 44
    "L_NUM7PIC", "L_NUM8PIC", "L_NUM9PIC", "L_PERCENTPIC", "L_APIC",                                 // 45  - 49
    "L_BPIC", "L_CPIC", "L_DPIC", "L_EPIC", "L_FPIC",                                                // 50  - 54
    "L_GPIC", "L_HPIC", "L_IPIC", "L_JPIC", "L_KPIC",                                                // 55  - 59
    "L_LPIC", "L_MPIC", "L_NPIC", "L_OPIC", "L_PPIC",                                                // 60  - 64
    "L_QPIC", "L_RPIC", "L_SPIC", "L_TPIC", "L_UPIC",                                                // 65  - 69
    "L_VPIC", "L_WPIC", "L_XPIC", "L_YPIC", "L_ZPIC",                                                // 70  - 74
    "L_EXPOINTPIC", "L_APOSTROPHEPIC", "L_GUY2PIC", "L_BJWINSPIC", "TITLE1PIC",                      // 75  - 79
    "TITLE2PIC", "ENDSCREEN11PIC", "ENDSCREEN12PIC", "ENDSCREEN3PIC", "ENDSCREEN4PIC",               // 80  - 84
    "ENDSCREEN5PIC", "ENDSCREEN6PIC", "ENDSCREEN7PIC", "ENDSCREEN8PIC", "ENDSCREEN9PIC",             // 85  - 89
    "STATUSBARPIC", "PG13PIC", "CREDITSPIC", "IDGUYS1PIC", "IDGUYS2PIC",                             // 90  - 94
    "COPYPROTTOPPIC", "COPYPROTBOXPIC", "BOSSPIC1PIC", "BOSSPIC2PIC", "BOSSPIC3PIC",                 // 95  - 99
    "BOSSPIC4PIC", "KNIFEPIC", "GUNPIC", "MACHINEGUNPIC", "GATLINGGUNPIC",                           // 100 - 104
    "NOKEYPIC", "GOLDKEYPIC", "SILVERKEYPIC", "N_BLANKPIC", "N_0PIC",                                // 105 - 109
    "N_1PIC", "N_2PIC", "N_3PIC", "N_4PIC", "N_5PIC",                                                // 110 - 114
    "N_6PIC", "N_7PIC", "N_8PIC", "N_9PIC", "FACE1APIC",                                             // 115 - 119
    "FACE1BPIC", "FACE1CPIC", "FACE2APIC", "FACE2BPIC", "FACE2CPIC",                                 // 120 - 124
    "FACE3APIC", "FACE3BPIC", "FACE3CPIC", "FACE4APIC", "FACE4BPIC",                                 // 125 - 129
    "FACE4CPIC", "FACE5APIC", "FACE5BPIC", "FACE5CPIC", "FACE6APIC",                                 // 130 - 134
    "FACE6BPIC", "FACE6CPIC", "FACE7APIC", "FACE7BPIC", "FACE7CPIC",                                 // 135 - 139
    "FACE8APIC", "GOTGATLINGPIC", "GODMODEFACE1PIC", "GODMODEFACE2PIC", "GODMODEFACE3PIC",           // 140 - 144
    "BJWAITING1PIC", "BJWAITING2PIC", "BJOUCHPIC", "PAUSEDPIC", "GETPSYCHEDPIC",                     // 145 - 149
    "TILE8", "ORDERSCREEN", "ERRORSCREEN", "TITLEPALETTE", "END1PALETTE",                            // 150 - 154
    "END2PALETTE", "END3PALETTE", "END4PALETTE", "END5PALETTE", "END6PALETTE",                       // 155 - 159
    "END7PALETTE", "END8PALETTE", "END9PALETTE", "IDGUYSPALETTE", "T_DEMO0",                         // 160 - 164
    "T_DEMO1", "T_DEMO2", "T_DEMO3", "T_ENDART1", "ENUMEND"                                          // 165 - 169
};

class WLCache {
public:
    WLCache(qint32 chunks, qint32 sounds);
    ~WLCache();

    int     currentMap() { return m_currentMap; }
    void    setCurrentMap(int mapNum) { m_currentMap = mapNum; }
    void    cacheGraphic(int idx);
    void    uncacheGraphic(int idx);
    void    cacheAdlib(int idx);
    qint32  cacheAudio(int idx);
    void    uncacheAudio(int idx);
    void    loadAllSounds();
    void    cacheMap(int idx);
    void    cacheScreen(int idx);
    QString fileExtension() { return g_Extension; }
    void    setFileExtension(QString str) { g_Extension = str; }
    int     missingEpisodes() { return m_missingEpisodes; }
    void    setMissingEpisodes(int num) { m_missingEpisodes = num; }
    bool    load(const QString& filename, void **ptr);
    bool    write(const QString& filename, void *ptr, qint32 length);
    char*   graphic(int idx) { return (char*)m_graphicData[idx]; }
    char*   audio(int idx) { return (char*)m_audioData[idx]; }
    char**  audioData(int idx) { return (char**)&m_audioData[idx]; }
    qint16* map(int idx) { return (qint16*)m_mapData[idx]; }
    qint16  mapSpot(int x, int y, int plane) { return m_mapData[plane][((y)<<mapShift)+(x)]; }
    bool    fog(qint32 x, qint32 y) { return m_mapFog[x][y]; }
    void    setFog(bool value, qint32 x, qint32 y) { m_mapFog[x][y] = value; }

    quint32  find(const QString& name);
    quint32  findSound(const QString& name);
    quint32  findMusic(const QString& name);

private:
    void   setupMapFile();
    void   setupGraphicFile();
    void   setupAudioFile();
    void   graphicLength(int idx);
    qint32 graphicPos(const size_t idx) { return m_graphicOffsets[idx]; }
    qint32 compress(quint16 *src, qint32 len, quint16 *dst, quint16 tag);
    void   expand(quint16 *src, quint16 *dst, qint32 len, quint16 tag);
    void   expandGraphic(int idx, qint32 *source);
    void   carmackExpand(quint8 *source, quint16 *dest, int length);
    void   huffExpand(quint8 *source, quint8 *dest, qint32 length, huffnode_t *hufftable);

    int        m_missingEpisodes;

    qint32     m_chunks;
    qint32     m_sounds;

    QFile*     m_mapFile;
    int        m_currentMap;
    maptype_t* m_mapInfo[numMaps];
    quint16*   m_mapData[mapPlanes];
    bool       m_mapFog[64][64];

    QFile*     m_audioFile;
    qint32*    m_audioOffsets;
    quint8*    m_audioData[500];

    qint32     m_lastSound;
    qint32     m_lastMusic;
    qint32     m_lastGraphic;

    SDMode     oldsoundmode;

    QFile*     m_graphicFile;
    qint32     m_graphicOffsets[200];
    quint8*    m_graphicData[200];

    qint32     m_uncompressedLength;
    qint32     m_compressedLength;
    quint16    m_RLEWtag;
    huffnode_t m_graphicHuffman[255];

    qint32     m_workingBuffer[bufferSize/4];
};

#endif
