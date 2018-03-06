#ifndef TAG_PARSER_OGGSTREAM_H
#define TAG_PARSER_OGGSTREAM_H

#include "./oggpage.h"

#include "../abstracttrack.h"

namespace TagParser {

class OggContainer;
class OggIterator;

class TAG_PARSER_EXPORT OggStream : public AbstractTrack
{
    friend class OggContainer;

public:
    OggStream(OggContainer &container, std::vector<OggPage>::size_type startPage);
    ~OggStream();

    TrackType type() const;
    std::size_t startPage() const;

protected:
    void internalParseHeader(Diagnostics &diag);

private:
    void calculateDurationViaSampleCount(uint16 preSkip = 0);

    std::size_t m_startPage;
    OggContainer &m_container;
    uint32 m_currentSequenceNumber;
};

inline std::size_t OggStream::startPage() const
{
    return m_startPage;
}

inline TrackType OggStream::type() const
{
    return TrackType::OggStream;
}

}

#endif // TAG_PARSER_OGGSTREAM_H
