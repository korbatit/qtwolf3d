#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <QByteArray>
#include <QVector>
#include <QCache>

class AudioResampler
{
public:
    AudioResampler( int srcFrequency, int srcChannels, int dstFrequency, int duration );
    int resample( const qint16* srcSamples, qint16* dstBuffer, int srcSamplesCount, bool mixWithExistingData );

private:
    int m_srcFrequency;
    int m_srcChannels;
    int m_dstFrequency;
    int m_duration;
    int m_samples;

    qint16 m_prevSamples[16*2];

    struct ResampleTable {
        QVector<int> samples;
        QVector<int> weight;
    };
    ResampleTable* resampleTable( int frequency );
    static QCache <int,ResampleTable> m_resampleTables;
};

#endif

