#define PM_DEBUG 1

#include <QFile>
#include <QDebug>

#include "wlpage.h"


WLPage::WLPage(const QString& filename, quint32 pageSize)
{
    m_pageSize = pageSize;

    m_totalChunks = 0;
    m_spriteStart = 0;
    m_soundStart  = 0;
    m_soundInfoPagePadded = false;

    m_pageFile = new QFile(filename);
    if (!m_pageFile->exists()) {
        qWarning()<<"PageManager::PageManager() : cant open file "<<filename;
        delete m_pageFile;
        m_pageFile = 0;
        return;
    }
    m_pageFile->open(QIODevice::ReadOnly);
    m_totalChunks = readInt16();
    m_spriteStart = readInt16();
    m_soundStart  = readInt16();
    if (m_spriteStart > m_totalChunks || m_soundStart > m_totalChunks
            || m_totalChunks > m_pageSize) {
        qWarning()<<"PageManager::PageManager() : corrupt file "<<filename;
        m_pageFile->close();
        delete m_pageFile;
        m_pageFile = 0;
        return;
    }
#ifdef PM_DEBUG
    qDebug()<<"totalChunks : "<<m_totalChunks;
    qDebug()<<"spriteStart : "<<m_spriteStart;
    qDebug()<<"soundStart  : "<<m_soundStart;
#endif
    m_offsets = (quint32*) malloc((m_totalChunks+1)*sizeof(quint32));
    m_pageFile->read((char*)m_offsets,m_totalChunks*sizeof(quint32));
    m_lengths = (quint16*) malloc(m_totalChunks*sizeof(quint16));
    m_pageFile->read((char*)m_lengths,m_totalChunks*sizeof(quint16));
    quint32 dataStart = m_offsets[0];

    for(int i = 0; i < (int)m_totalChunks; i++) {
        if (!m_offsets[i]) continue;
        if (m_offsets[i] < dataStart || m_offsets[i] >= m_pageFile->size()) {
            qWarning()<<"PageManager::PageManager() : Illegal page offset "
                     <<i<<" of "<<m_offsets[i];
        }
    }
    qint32 padding = 0;
    for(int i = (int)m_spriteStart; i < (int)m_soundStart; i++) {
        if (!m_offsets[i]) continue;
        quint32 offsets = m_offsets[i] - dataStart + padding;
        if (offsets & 1) padding++;
    }
    if((m_offsets[m_totalChunks-1] - dataStart + padding) & 1)
        padding++;

    m_pageDataSize = (m_pageFile->size() - m_offsets[0]) + padding;
    m_pageData = (quint32*) malloc(m_pageDataSize);
    m_pages = (char**)malloc((m_totalChunks+1)*sizeof(char*));
    char *ptr = (char*)m_pageData;
    for(int i = 0; i < (int)m_totalChunks; i++) {
        if ((i >= (int)m_spriteStart && i < (int)m_soundStart) ||
                (i == (int)m_totalChunks-1)) {
            size_t offsets = ptr - (char*)m_pageData;
            if (offsets & 1) {
                *ptr++ = 0;
                if (i == (int)m_totalChunks-1) m_soundInfoPagePadded = true;
            }
        }
        m_pages[i] = ptr;
        if (!m_offsets[i]) continue;
        quint32 length;
        if (!m_offsets[i+1]) length = m_lengths[i];
        else length = m_offsets[i+1] - m_offsets[i];
        m_pageFile->seek(m_offsets[i]);
        if (m_pageFile->pos() + length <= m_pageFile->size()) {
            m_pageFile->read(ptr,length);
            ptr += length;
        } else {
            qWarning()<<"Couldnt read in idx="<<i;
            break;
        }
    }
    m_pages[m_totalChunks] = ptr;
    m_pageFile->close();
}

WLPage::~WLPage()
{
    free(m_pages);
    free(m_pageData);
    free(m_lengths);
    free(m_offsets);
}

char* WLPage::end()
{
    return m_pages[m_totalChunks];
}

char* WLPage::page(qint32 idx)
{
    if (idx < 0 || idx >= (int)m_totalChunks) {
        qWarning("Tried to access illegal page: %d",idx);
        exit(1);
    }
    return m_pages[idx];
}

quint32 WLPage::pageSize(qint32 idx)
{
    if (idx < 0 || idx >= (int)m_totalChunks) {
        qWarning("Tried to access illegal page: %d",idx);
        exit(1);
    }
    return (quint32)(m_pages[idx+1]-m_pages[idx]);
}

quint16* WLPage::sprite(qint32 idx)
{
    return (quint16*)(page(m_spriteStart+idx));
}

char* WLPage::sound(qint32 idx)
{
    return page(m_soundStart+idx);
}

qint16 WLPage::readInt16()
{
    char num[2];

    memset(&num,0,2);
    m_pageFile->read((char*)num,2);
    return (((qint16)(num[0]&0xFF))|(((qint16)(num[1]&0xFF))<<8));
}

quint32 WLPage::readInt32()
{
    char num[4];

    memset(&num,0,4);
    m_pageFile->read((char*)num,4);
    return (((qint32)(num[0]&0xFF))|(((qint32)(num[1]&0xFF))<<8)
            |(((qint32)(num[2]&0xFF))<<16)|(((qint32)num[3]&0xFF)<<24));
}


