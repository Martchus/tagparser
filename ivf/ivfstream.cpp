#include "./ivfstream.h"

#include "../mp4/mp4ids.h"

#include "../exceptions.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/conversion/stringconversion.h>

#include <string>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::IvfStream
 * \brief Implementation of TagParser::AbstractTrack for IVF streams.
 * \sa https://wiki.multimedia.cx/index.php/IVF
 */

void IvfStream::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing IVF header");
    if (!m_istream) {
        throw NoDataFoundException();
    }

    // check signature and version
    if (m_reader.readUInt32BE() != 0x444B4946u) {
        diag.emplace_back(DiagLevel::Critical, "Signature not \"DKIF\".", context);
        throw InvalidDataException();
    }
    const auto version = m_reader.readUInt16LE();
    m_version = version;
    if (version != 0) {
        diag.emplace_back(DiagLevel::Warning, argsToString("Version ", version, " is not supported."), context);
    }

    // read remaining header
    m_headerLength = m_reader.readUInt16LE();
    const auto formatId = m_reader.readUInt32BE();
    m_formatId = interpretIntegerAsString(formatId);
    m_pixelSize.setWidth(m_reader.readUInt16LE());
    m_pixelSize.setHeight(m_reader.readUInt16LE());
    m_fps = m_reader.readUInt32LE();
    m_timeScale = m_reader.readUInt32LE();
    m_sampleCount = m_reader.readUInt32LE();

    // compute further values
    m_format = FourccIds::fourccToMediaFormat(formatId);
    m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_fps);

    // skip unused bytes
    m_istream->seekg(4, ios_base::cur);
}

void IvfStream::readFrame(Diagnostics &diag)
{
    m_frames.emplace_back();
    m_frames.back().parseHeader(m_reader, diag);
}

} // namespace TagParser
