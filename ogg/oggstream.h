#ifndef MEDIA_OGGSTREAM_H
#define MEDIA_OGGSTREAM_H

#include "./oggpage.h"

#include "../abstracttrack.h"

namespace Media {

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
    void internalParseHeader();

private:
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

#endif // MEDIA_OGGSTREAM_H
