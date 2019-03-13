#ifndef TAG_PARSER_FLACIDENTIFICATIONHEADER_H
#define TAG_PARSER_FLACIDENTIFICATIONHEADER_H

#include "./flacmetadata.h"

namespace TagParser {

class OggIterator;

class TAG_PARSER_EXPORT FlacToOggMappingHeader {
public:
    constexpr FlacToOggMappingHeader();

    void parseHeader(OggIterator &iterator);

    constexpr std::uint8_t majorVersion() const;
    constexpr std::uint8_t minorVersion() const;
    constexpr std::uint16_t headerCount() const;
    constexpr const FlacMetaDataBlockStreamInfo &streamInfo() const;

private:
    std::uint8_t m_majorVersion;
    std::uint8_t m_minorVersion;
    std::uint16_t m_headerCount;
    FlacMetaDataBlockStreamInfo m_streamInfo;
};

/*!
 * \brief Constructs a new FLAC identification header.
 */
constexpr FlacToOggMappingHeader::FlacToOggMappingHeader()
    : m_majorVersion(0)
    , m_minorVersion(0)
    , m_headerCount(0)
{
}

/*!
 * \brief Returns the major version for the mapping (which should be 1 currently).
 */
constexpr std::uint8_t FlacToOggMappingHeader::majorVersion() const
{
    return m_majorVersion;
}

/*!
 * \brief Returns the version for the mapping (which should be 0 currently).
 */
constexpr std::uint8_t FlacToOggMappingHeader::minorVersion() const
{
    return m_minorVersion;
}

/*!
 * \brief Returns the number of header (non-audio) packets, not including this one.
 */
constexpr std::uint16_t FlacToOggMappingHeader::headerCount() const
{
    return m_headerCount;
}

/*!
 * \brief Returns the stream info.
 */
constexpr const FlacMetaDataBlockStreamInfo &FlacToOggMappingHeader::streamInfo() const
{
    return m_streamInfo;
}

} // namespace TagParser

#endif // TAG_PARSER_FLACIDENTIFICATIONHEADER_H
