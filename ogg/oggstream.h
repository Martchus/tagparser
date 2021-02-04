#ifndef TAG_PARSER_OGGSTREAM_H
#define TAG_PARSER_OGGSTREAM_H

#include "./oggpage.h"

#include "../abstracttrack.h"

namespace TagParser {

class OggContainer;
class OggIterator;

class TAG_PARSER_EXPORT OggStream final : public AbstractTrack {
    friend class OggContainer;

public:
    OggStream(OggContainer &container, std::vector<OggPage>::size_type startPage);
    ~OggStream() override;

    TrackType type() const override;
    std::size_t startPage() const;

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    void calculateDurationViaSampleCount(std::uint16_t preSkip = 0);

    std::size_t m_startPage;
    OggContainer &m_container;
    std::uint32_t m_currentSequenceNumber;
};

inline std::size_t OggStream::startPage() const
{
    return m_startPage;
}

inline TrackType OggStream::type() const
{
    return TrackType::OggStream;
}

} // namespace TagParser

#endif // TAG_PARSER_OGGSTREAM_H
