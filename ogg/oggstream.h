#ifndef MEDIA_OGGSTREAM_H
#define MEDIA_OGGSTREAM_H

#include "oggpage.h"
#include "../abstracttrack.h"

namespace Media {

class OggContainer;
class OggIterator;

class LIB_EXPORT OggStream : public AbstractTrack
{
    friend class OggContainer;

public:
    OggStream(OggContainer &container, std::vector<OggPage>::size_type startPage);
    ~OggStream();

    TrackType type() const;

protected:
    void internalParseHeader();

private:
    std::vector<OggPage>::size_type m_startPage;
    OggContainer &m_container;
};

inline TrackType OggStream::type() const
{
    return TrackType::OggStream;
}

}

#endif // MEDIA_OGGSTREAM_H
