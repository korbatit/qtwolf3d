#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtTest/QtTest>

#include "../../../wlpage.h"
#include "../../../wlrender.h"

typedef struct {
    qint16 leftpix;
    qint16 rightpix;
    qint16 dataofs[1];
} shape_t;

typedef struct {
    qint32 id;
    qint32 size;
    char   data[1];
} riffChunk_t;

typedef struct {
    qint16 fmt;
    qint16 ch;
    qint32 rate;
    qint32 avgBps;
    qint16 block;
    qint16 bps;
} waveFmt_t;

typedef struct
{
    qint32  rate;
    qint32  ch;
    qint32  len;
    qint32  loop;
    char*   data;
} sound_t;

typedef struct
{
    quint32 startpage;
    quint32 length;
} digiInfo_t;

extern Color_t gamepal[256];

class tst_page : public QObject
{
    Q_OBJECT
    
public:
    tst_page();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void checkDemoValues();
    void checkDemoTextures();
    void checkDemoSprites();
    void checkDemoSounds();

    void checkFullValues();
    void checkFullTextures();
    void checkFullSprites();
    void checkFullSounds();

    void checkSpearValues();
    void checkSpearTextures();
    void checkSpearSprites();
    void checkSpearSounds();

private:
    QImage* texture(qint32 idx);
    QImage* sprite(qint32 idx);
    void sound(qint32 idx);
    void expandPalette(char *dst, const char *src, qint32 w, qint32 h);
    bool writeWAVFile(QFile *wavFile, sound_t *snd);
    void loadDemo();
    void loadFull();
    void loadSpear();

    quint16     numDigi;
    digiInfo_t* digiInfo;
    WLPage*     page;
};

tst_page::tst_page()
{
}

void tst_page::expandPalette(char *dst, const char *src, qint32 w, qint32 h)
{
    qint32      pixel;
    const char* p = src;

    p = src;
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            pixel = *src++;
            pixel = pixel & 0xFF;
            dst[y * w * 3 + x * 3] = gamepal[pixel].r;
            dst[y * w * 3 + x * 3 + 1] = gamepal[pixel].g;
            dst[y * w * 3 + x * 3 + 2] = gamepal[pixel].b;
        }
    }
}

bool tst_page::writeWAVFile(QFile *wavFile, sound_t *snd)
{
    if(!snd || !snd->data || !snd->len)
        return false;

    if (wavFile->open(QIODevice::WriteOnly|QIODevice::Unbuffered)) {

        waveFmt_t fmt;

        int size = snd->ch * snd->len;

        qint32 head[5];

        memcpy((char*)&head[0], "RIFF", 4);
        head[1] = size + 36;
        memcpy((char*)&head[2], "WAVE", 4);
        memcpy((char*)&head[3], "fmt ", 4);
        head[4] = sizeof(fmt);

        fmt.fmt = 1;
        fmt.ch = snd->ch;
        fmt.rate = snd->rate;
        fmt.avgBps = snd->rate * snd->ch;
        fmt.block = snd->ch;
        fmt.bps = 8;

        wavFile->write((char*)&head, sizeof(qint32)*5);
        wavFile->write((char*)&fmt, sizeof(fmt));

        memcpy((char*)&head[0], "data", 4);
        head[1] = size;
        wavFile->write((char*)&head, sizeof(qint32)*2);

        wavFile->write((char*)snd->data, size);
        wavFile->close();

        return true;

    } else
        qWarning()<<"writeWAVFile() : cannot write wav file : "<<wavFile->fileName();

    return false;
}

void tst_page::sound(qint32 idx)
{
    char* samples = page->sound(digiInfo[idx].startpage);
    quint16 len = digiInfo[idx].length;

    if (len > 128) {
        QFile wavFile(QString("sound%1.wav").arg(idx));
        sound_t snd;
        snd.data = samples;
        snd.ch = 1;
        snd.len = len;
        snd.rate = 7042;
        snd.loop = 0;
        writeWAVFile(&wavFile, &snd);
    }
}

QImage* tst_page::texture(qint32 idx)
{
    // Get raw data from file
    char* rawData = page->texture(idx);

    // invert and mirror
    char  dst[64*64*3];
    char  tmp[64*64];
    char* p = tmp;

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 64; x++)
            *p++ = rawData[x * 64 + y];
    }

    // Apply palette
    expandPalette(dst, tmp, 64, 64);
    QImage* image = new QImage((uchar*)&dst,64,64,QImage::Format_RGB888);
    image->save(QString("texture%1.png").arg(idx), "PNG");
    return image;
}

QImage* tst_page::sprite(qint32 idx)
{
    char     dst[64*64*4];
    char     tmp[64*64];
    shape_t* shape;
    qint16*  linecmds;
    qint16*  cmdptr;

    // all transparent at the beginning
    memset(tmp, 0xFF, 64*64);

    shape = (shape_t*)page->sprite(idx);
    char* rawData = (char*)shape;
    cmdptr = shape->dataofs;

    for(int x = shape->leftpix; x <= shape->rightpix; x++) {
        linecmds = (qint16*)(rawData + *cmdptr++);
        for(; *linecmds; linecmds += 3) {
            int n = linecmds[2]/2 + linecmds[1];
            for(int y = linecmds[2]/2; y < linecmds[0]/2; y++, n++)
                tmp[y*64+x] = (rawData[n])&0xFF;
        }
    }
    expandPalette(dst, tmp, 64, 64);
    QImage* image = new QImage((uchar*)&dst,64,64,QImage::Format_RGB888);
    image->save(QString("sprite%1.png").arg(idx), "PNG");
    return image;
}

void tst_page::initTestCase()
{
    page = 0;
    digiInfo = 0;
}

void tst_page::loadDemo()
{
    delete page;
    delete digiInfo;

    page = new WLPage(":/content/demo/vswap.wl1");

    quint16 *soundInfoPage = (quint16*)page->page(page->totalChunks()-1);

    numDigi = *page->length(page->totalChunks()-1)/4;
    digiInfo = (digiInfo_t*) malloc(numDigi * sizeof(digiInfo_t));
    memset(digiInfo, 0, numDigi * sizeof(digiInfo_t));

    for(int i = 0; i < numDigi; i++) {
        digiInfo[i].startpage = soundInfoPage[i * 2];
        if((int) digiInfo[i].startpage >= (int)page->totalChunks() - 1) {
            numDigi = i;
            break;
        }
        int lastPage;
        if(i < numDigi - 1) {
            lastPage = soundInfoPage[i * 2 + 2];
            if(lastPage == 0 || lastPage + (int)page->soundStart() > (int)page->totalChunks() - 1) lastPage = page->totalChunks() - 1;
            else lastPage += page->soundStart();
        } else lastPage = page->totalChunks() - 1;

        int size = 0;
        for(int pg = (int)page->soundStart() + digiInfo[i].startpage; pg < lastPage; pg++)
            size += *page->length(pg); //page->pageSize(pg);
        if(lastPage == (int)page->totalChunks() - 1 && (int)page->soundInfoPagePadded()) size--;
        if((size & 0xffff0000) != 0 && (size & 0xffff) < soundInfoPage[i * 2 + 1])
            size -= 0x10000;
        size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];
        digiInfo[i].length = size;
    }
}

void tst_page::loadFull()
{
    delete page;
    delete digiInfo;

    page = new WLPage(":/content/full/vswap.wl6");

    quint16 *soundInfoPage = (quint16*)page->page(page->totalChunks()-1);

    numDigi = *page->length(page->totalChunks()-1)/4;
    digiInfo = (digiInfo_t*) malloc(numDigi * sizeof(digiInfo_t));
    memset(digiInfo, 0, numDigi * sizeof(digiInfo_t));

    for(int i = 0; i < numDigi; i++) {
        digiInfo[i].startpage = soundInfoPage[i * 2];
        if((int) digiInfo[i].startpage >= (int)page->totalChunks() - 1) {
            numDigi = i;
            break;
        }
        int lastPage;
        if(i < numDigi - 1) {
            lastPage = soundInfoPage[i * 2 + 2];
            if(lastPage == 0 || lastPage + (int)page->soundStart() > (int)page->totalChunks() - 1) lastPage = page->totalChunks() - 1;
            else lastPage += page->soundStart();
        } else lastPage = page->totalChunks() - 1;

        int size = 0;
        for(int pg = (int)page->soundStart() + digiInfo[i].startpage; pg < lastPage; pg++)
            size += *page->length(pg); //page->pageSize(pg);
        if(lastPage == (int)page->totalChunks() - 1 && (int)page->soundInfoPagePadded()) size--;
        if((size & 0xffff0000) != 0 && (size & 0xffff) < soundInfoPage[i * 2 + 1])
            size -= 0x10000;
        size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];
        digiInfo[i].length = size;
    }
}

void tst_page::loadSpear()
{
    delete page;
    delete digiInfo;

    page = new WLPage(":/content/spear/vswap.sod");

    quint16 *soundInfoPage = (quint16*)page->page(page->totalChunks()-1);

    numDigi = *page->length(page->totalChunks()-1)/4;
    digiInfo = (digiInfo_t*) malloc(numDigi * sizeof(digiInfo_t));
    memset(digiInfo, 0, numDigi * sizeof(digiInfo_t));

    for(int i = 0; i < numDigi; i++) {
        digiInfo[i].startpage = soundInfoPage[i * 2];
        if((int) digiInfo[i].startpage >= (int)page->totalChunks() - 1) {
            numDigi = i;
            break;
        }
        int lastPage;
        if(i < numDigi - 1) {
            lastPage = soundInfoPage[i * 2 + 2];
            if(lastPage == 0 || lastPage + (int)page->soundStart() > (int)page->totalChunks() - 1) lastPage = page->totalChunks() - 1;
            else lastPage += page->soundStart();
        } else lastPage = page->totalChunks() - 1;

        int size = 0;
        for(int pg = (int)page->soundStart() + digiInfo[i].startpage; pg < lastPage; pg++)
            size += *page->length(pg); //page->pageSize(pg);
        if(lastPage == (int)page->totalChunks() - 1 && (int)page->soundInfoPagePadded()) size--;
        if((size & 0xffff0000) != 0 && (size & 0xffff) < soundInfoPage[i * 2 + 1])
            size -= 0x10000;
        size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];
        digiInfo[i].length = size;
    }
}

void tst_page::cleanupTestCase()
{
    delete page;
    delete digiInfo;
}

void tst_page::checkDemoValues()
{
//    loadDemo();
//    QVERIFY2(page->totalChunks() == 663, qPrintable(QString("should be %1").arg(page->totalChunks())));
//    QVERIFY2(page->spriteStart() == 106, qPrintable(QString("should be %1").arg(page->spriteStart())));
//    QVERIFY2(page->soundStart()  == 542, qPrintable(QString("should be %1").arg(page->soundStart())));
}

void tst_page::checkDemoTextures()
{
//    loadDemo();
//    int textureCount = page->spriteStart();
//    for (int i = 0; i < textureCount; i++) {
//        QImage* img = texture(i);
//        QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
//        delete img;
//    }
}

void tst_page::checkDemoSprites()
{
//    loadDemo();
//    int spriteCount = page->soundStart() - page->spriteStart();
//    for(int i = 0; i < spriteCount; i++) {
//        QImage* img = sprite(i);
//        QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
//        delete img;
//    }
}

void tst_page::checkDemoSounds()
{
//    loadDemo();
//    int soundCount = 46;
//    for (int i = 0; i < soundCount; i++)
//        sound(i);
}

void tst_page::checkFullValues()
{
//    loadFull();
//    QVERIFY2(page->totalChunks() == 663, qPrintable(QString("should be %1").arg(page->totalChunks())));
//    QVERIFY2(page->spriteStart() == 106, qPrintable(QString("should be %1").arg(page->spriteStart())));
//    QVERIFY2(page->soundStart()  == 542, qPrintable(QString("should be %1").arg(page->soundStart())));
}

void tst_page::checkFullTextures()
{
//    loadFull();
//    int textureCount = page->spriteStart();
//    for (int i = 0; i < textureCount; i++) {
//        QImage* img = texture(i);
//        QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
//        delete img;
//    }
}

void tst_page::checkFullSprites()
{
//    loadFull();
//    int spriteCount = page->soundStart() - page->spriteStart();
//    for(int i = 0; i < spriteCount; i++) {
//        QImage* img = sprite(i);
//        QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
//        delete img;
//    }
}

void tst_page::checkFullSounds()
{
//    loadFull();
//    int soundCount = numDigi;
//    for (int i = 0; i < soundCount; i++)
//        sound(i);
}

void tst_page::checkSpearValues()
{
    loadSpear();
    QVERIFY2(page->totalChunks() == 666, qPrintable(QString("should be %1").arg(page->totalChunks())));
    QVERIFY2(page->spriteStart() == 134, qPrintable(QString("should be %1").arg(page->spriteStart())));
    QVERIFY2(page->soundStart()  == 555, qPrintable(QString("should be %1").arg(page->soundStart())));
}

void tst_page::checkSpearTextures()
{
    loadSpear();
    int textureCount = page->spriteStart();
    for (int i = 0; i < textureCount; i++) {
        QImage* img = texture(i);
        //QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
        delete img;
    }
}

void tst_page::checkSpearSprites()
{
    loadSpear();
    int spriteCount = page->soundStart() - page->spriteStart();
    for(int i = 0; i < spriteCount; i++) {
        QImage* img = sprite(i);
        //QVERIFY2(img->size() == QSize(64,64), qPrintable("image size not 64x64!"));
        delete img;
    }
}

void tst_page::checkSpearSounds()
{
    loadSpear();
    int soundCount = numDigi;
    for (int i = 0; i < soundCount; i++)
        sound(i);
}

QTEST_MAIN(tst_page)

#include "tst_page.moc"
