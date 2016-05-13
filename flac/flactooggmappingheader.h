#ifndef MEDIA_FLACIDENTIFICATIONHEADER_H
#define MEDIA_FLACIDENTIFICATIONHEADER_H

#include "./flacmetadata.h"

namespace Media {

class OggIterator;

class LIB_EXPORT FlacToOggMappingHeader
{
public:
    FlacToOggMappingHeader();

    void parseHeader(OggIterator &iterator);

    byte majorVersion() const;
    byte minorVersion() const;
    uint16 headerCount() const;
    const FlacMetaDataBlockStreamInfo &streamInfo() const;

private:
    byte m_majorVersion;
    byte m_minorVersion;
    uint16 m_headerCount;
    FlacMetaDataBlockStreamInfo m_streamInfo;
};

/*!
 * \brief Constructs a new FLAC identification header.
 */
inline FlacToOggMappingHeader::FlacToOggMappingHeader() :
    m_majorVersion(0),
    m_minorVersion(0),
    m_headerCount(0)
{}

/*!
 * \brief Returns the major version for the mapping (which should be 1 currently).
 */
inline byte FlacToOggMappingHeader::majorVersion() const
{
    return m_majorVersion;
}

/*!
 * \brief Returns the version for the mapping (which should be 0 currently).
 */
inline byte FlacToOggMappingHeader::minorVersion() const
{
    return m_minorVersion;
}

/*!
 * \brief Returns the number of header (non-audio) packets, not including this one.
 */
inline uint16 FlacToOggMappingHeader::headerCount() const
{
    return m_headerCount;
}

/*!
 * \brief Returns the stream info.
 */
inline const FlacMetaDataBlockStreamInfo &FlacToOggMappingHeader::streamInfo() const
{
    return m_streamInfo;
}

}

#endif // MEDIA_FLACIDENTIFICATIONHEADER_H
