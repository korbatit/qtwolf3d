#ifndef WLPAGE_H
#define WLPAGE_H

typedef struct {
    qint16 start;
    qint16 length;
} digimap_t;

class QFile;

class WLPage {
public:
    WLPage(const QString& filename, quint32 pageSize = 4096);
    ~WLPage();

    quint32  totalChunks() { return m_totalChunks; }
    quint32  spriteStart() { return m_spriteStart; }
    quint32  soundStart()  { return m_soundStart;  }

    char*    end();
    char*    page(qint32 idx);
    quint32  pageSize(qint32 idx);

    char*    texture(qint32 idx) { return page(idx); }
    quint16* sprite(qint32 idx);
    char  *  sound(qint32 idx);

    bool     soundInfoPagePadded() { return m_soundInfoPagePadded; }

    quint32* offset(qint32 idx) { return &m_offsets[idx]; }
    quint16* length(qint32 idx) { return &m_lengths[idx]; }

private:
    qint16  readInt16();
    quint32 readInt32();

    QFile*   m_pageFile;
    char**   m_pages;
    quint32  m_pageSize;
    quint32* m_pageData;
    size_t   m_pageDataSize;
    quint32  m_totalChunks;
    quint32  m_spriteStart;
    quint32  m_soundStart;
    quint32* m_offsets;
    quint16* m_lengths;
    bool     m_soundInfoPagePadded;
};

#endif
