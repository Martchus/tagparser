#include "./flacstream.h"
#include "./flacmetadata.h"

#include "../vorbis/vorbiscomment.h"

#include "../exceptions.h"
#include "../mediafileinfo.h"
#include "../mediaformat.h"

#include "resources/config.h"

#include <c++utilities/io/copy.h>

#include <sstream>

using namespace std;
using namespace CppUtilities;

namespace TagParser {

/*!
 * \class TagParser::FlacStream
 * \brief Implementation of TagParser::AbstractTrack for raw FLAC streams.
 */

/*!
 * \brief Constructs a new track for the specified \a mediaFileInfo at the specified \a startOffset.
 *
 * The stream of the \a mediaFileInfo instance is used as input stream.
 */
FlacStream::FlacStream(MediaFileInfo &mediaFileInfo, std::uint64_t startOffset)
    : AbstractTrack(mediaFileInfo.stream(), startOffset)
    , m_mediaFileInfo(mediaFileInfo)
    , m_paddingSize(0)
    , m_streamOffset(0)
{
    m_mediaType = MediaType::Audio;
}

/*!
 * \brief Creates a new Vorbis comment for the stream.
 * \remarks Just returns the current Vorbis comment if already present.
 */
VorbisComment *FlacStream::createVorbisComment()
{
    if (!m_vorbisComment) {
        m_vorbisComment = make_unique<VorbisComment>();
    }
    return m_vorbisComment.get();
}

/*!
 * \brief Removes the assigned Vorbis comment if one is assigned; does nothing otherwise.
 * \returns Returns whether there were a Vorbis comment assigned.
 */
bool FlacStream::removeVorbisComment()
{
    if (!m_vorbisComment) {
        return false;
    }
    m_vorbisComment.reset();
    return true;
}

void FlacStream::internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    CPP_UTILITIES_UNUSED(progress)

    static const string context("parsing raw FLAC header");
    if (!m_istream) {
        throw NoDataFoundException();
    }

    m_istream->seekg(static_cast<streamoff>(m_startOffset), ios_base::beg);
    constexpr auto bufferSize = 0x22;
    char buffer[bufferSize];

    // check signature
    if (m_reader.readUInt32BE() != 0x664C6143) {
        diag.emplace_back(DiagLevel::Critical, "Signature (fLaC) not found.", context);
        throw InvalidDataException();
    }
    m_format = GeneralMediaFormat::Flac;

    // parse meta data blocks
    for (FlacMetaDataBlockHeader header; !header.isLast();) {
        // parse block header
        m_istream->read(buffer, 4);
        header.parseHeader(std::string_view(buffer, 4));

        // remember start offset
        const auto startOffset = m_istream->tellg();

        // parse relevant meta data
        switch (static_cast<FlacMetaDataBlockType>(header.type())) {
        case FlacMetaDataBlockType::StreamInfo:
            if (header.dataSize() >= bufferSize) {
                m_istream->read(buffer, bufferSize);
                FlacMetaDataBlockStreamInfo streamInfo;
                streamInfo.parse(std::string_view(buffer, bufferSize));
                m_channelCount = streamInfo.channelCount();
                m_samplingFrequency = streamInfo.samplingFrequency();
                m_sampleCount = streamInfo.totalSampleCount();
                m_bitsPerSample = streamInfo.bitsPerSample();
                m_duration = TimeSpan::fromSeconds(static_cast<double>(m_sampleCount) / m_samplingFrequency);
            } else {
                diag.emplace_back(DiagLevel::Critical, "\"METADATA_BLOCK_STREAMINFO\" is truncated and will be ignored.", context);
            }
            break;

        case FlacMetaDataBlockType::VorbisComment:
            // parse Vorbis comment
            // if more than one comment exist, simply thread those comments as one
            if (!m_vorbisComment) {
                m_vorbisComment = make_unique<VorbisComment>();
            }
            try {
                m_vorbisComment->parse(*m_istream, header.dataSize(), VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte, diag);
            } catch (const Failure &) {
                // error is logged via notifications, just continue with the next metadata block
            }
            break;

        case FlacMetaDataBlockType::Picture:
            try {
                // parse the cover
                VorbisCommentField coverField;
                coverField.setId(m_vorbisComment->fieldId(KnownField::Cover));
                FlacMetaDataBlockPicture picture(coverField.value());
                picture.parse(*m_istream, header.dataSize());
                coverField.setTypeInfo(picture.pictureType());

                if (coverField.value().isEmpty()) {
                    diag.emplace_back(DiagLevel::Warning, "\"METADATA_BLOCK_PICTURE\" contains no picture.", context);
                } else {
                    // add the cover to the Vorbis comment
                    if (!m_vorbisComment) {
                        // create one if none exists yet
                        m_vorbisComment = make_unique<VorbisComment>();
                        m_vorbisComment->setVendor(TagValue(APP_NAME " v" APP_VERSION, TagTextEncoding::Utf8));
                    }
                    m_vorbisComment->fields().insert(make_pair(coverField.id(), std::move(coverField)));
                }

            } catch (const TruncatedDataException &) {
                diag.emplace_back(DiagLevel::Critical, "\"METADATA_BLOCK_PICTURE\" is truncated and will be ignored.", context);
            }
            break;

        case FlacMetaDataBlockType::Padding:
            m_paddingSize += 4 + header.dataSize();
            break;

        default:;
        }

        // seek to next block
        m_istream->seekg(startOffset + static_cast<decltype(startOffset)>(header.dataSize()));

        // TODO: check first FLAC frame
    }

    m_streamOffset = static_cast<std::uint32_t>(m_istream->tellg());
}

/*!
 * \brief Writes the FLAC metadata header to the specified \a outputStream.
 *
 * This basically copies all "METADATA_BLOCK_HEADER" of the current stream to the specified \a outputStream, except:
 *
 *  - Vorbis comment is updated.
 *  - "METADATA_BLOCK_PICTURE" are updated.
 *  - Padding is skipped
 *
 * \returns Returns the start offset of the last "METADATA_BLOCK_HEADER" within \a outputStream.
 */
std::streamoff FlacStream::makeHeader(ostream &outputStream, Diagnostics &diag)
{
    istream &originalStream = m_mediaFileInfo.stream();
    originalStream.seekg(static_cast<streamoff>(m_startOffset + 4));
    CopyHelper<512> copy;

    // write signature
    BE::getBytes(static_cast<std::uint32_t>(0x664C6143u), copy.buffer());
    outputStream.write(copy.buffer(), 4);

    std::streamoff lastStartOffset = -1;

    // write meta data blocks which don't need to be adjusted
    FlacMetaDataBlockHeader header;
    FlacMetaDataBlockHeader lastActuallyWrittenHeader;
    do {
        // parse block header
        originalStream.read(copy.buffer(), 4);
        header.parseHeader(std::string_view(copy.buffer(), 4));

        // skip/copy block
        switch (static_cast<FlacMetaDataBlockType>(header.type())) {
        case FlacMetaDataBlockType::VorbisComment:
        case FlacMetaDataBlockType::Picture:
        case FlacMetaDataBlockType::Padding:
            // skip separately written block
            originalStream.seekg(header.dataSize(), ios_base::cur);
            break;
        default:
            // copy block which doesn't need to be adjusted
            originalStream.seekg(-4, ios_base::cur);
            lastStartOffset = outputStream.tellp();
            copy.copy(originalStream, outputStream, 4 + header.dataSize());
            lastActuallyWrittenHeader = header;
        }
    } while (!header.isLast());

    // adjust "isLast" flag if neccassary
    if (lastStartOffset >= 4
        && ((!m_vorbisComment && !lastActuallyWrittenHeader.isLast()) || (m_vorbisComment && lastActuallyWrittenHeader.isLast()))) {
        outputStream.seekp(lastStartOffset);
        lastActuallyWrittenHeader.setLast(!m_vorbisComment);
        lastActuallyWrittenHeader.makeHeader(outputStream);
        originalStream.seekg(lastActuallyWrittenHeader.dataSize(), ios_base::cur);
    }

    // write Vorbis comment
    if (!m_vorbisComment) {
        return lastStartOffset >= 0 ? lastStartOffset : 0;
    }
    // leave 4 bytes space for the "METADATA_BLOCK_HEADER"
    lastStartOffset = outputStream.tellp();
    outputStream.write(copy.buffer(), 4);

    // determine cover ID since covers must be written separately
    const auto coverId = m_vorbisComment->fieldId(KnownField::Cover);

    // write Vorbis comment
    m_vorbisComment->make(outputStream, VorbisCommentFlags::NoSignature | VorbisCommentFlags::NoFramingByte | VorbisCommentFlags::NoCovers, diag);

    // write "METADATA_BLOCK_HEADER"
    const auto endOffset = outputStream.tellp();
    header.setType(FlacMetaDataBlockType::VorbisComment);
    auto dataSize(static_cast<std::uint64_t>(endOffset) - static_cast<std::uint64_t>(lastStartOffset) - 4);
    if (dataSize > 0xFFFFFF) {
        dataSize = 0xFFFFFF;
        diag.emplace_back(DiagLevel::Critical, "Vorbis Comment is too big and will be truncated.", "write Vorbis Comment to FLAC stream");
    }
    header.setDataSize(static_cast<std::uint32_t>(dataSize));
    header.setLast(!m_vorbisComment->hasField(coverId));
    outputStream.seekp(lastStartOffset);
    header.makeHeader(outputStream);
    outputStream.seekp(static_cast<streamoff>(dataSize), ios_base::cur);
    lastActuallyWrittenHeader = header;

    // write cover fields separately as "METADATA_BLOCK_PICTURE"
    if (header.isLast()) {
        return lastStartOffset;
    }
    header.setType(FlacMetaDataBlockType::Picture);
    const auto coverFields = m_vorbisComment->fields().equal_range(coverId);
    for (auto i = coverFields.first; i != coverFields.second;) {
        const auto lastCoverStartOffset = outputStream.tellp();

        try {
            // write the structure
            FlacMetaDataBlockPicture pictureBlock(i->second.value());
            pictureBlock.setPictureType(i->second.typeInfo());
            header.setDataSize(pictureBlock.requiredSize());
            header.setLast(++i == coverFields.second);
            header.makeHeader(outputStream);
            pictureBlock.make(outputStream);

            // update variables to handle the "isLast" flag
            lastStartOffset = lastCoverStartOffset;
            lastActuallyWrittenHeader = header;

        } catch (const Failure &) {
            // we can expect nothing is written in the error case except the FLAC header, so
            // -> just add an error message
            diag.emplace_back(DiagLevel::Critical, "Unable to serialize \"METADATA_BLOCK_PICTURE\" from assigned value.",
                "write \"METADATA_BLOCK_PICTURE\" to FLAC stream");
            // -> and to recover, go back to where we were before
            outputStream.seekp(lastCoverStartOffset);
        }
    }

    // adjust "isLast" flag if neccassary
    if (!lastActuallyWrittenHeader.isLast()) {
        outputStream.seekp(lastStartOffset);
        lastActuallyWrittenHeader.setLast(true);
        lastActuallyWrittenHeader.makeHeader(outputStream);
        outputStream.seekp(lastActuallyWrittenHeader.dataSize());
    }

    return lastStartOffset;
}

/*!
 * \brief Writes padding of the specified \a size to the specified \a stream.
 * \remarks Size must be at least 4 bytes.
 */
void FlacStream::makePadding(ostream &stream, std::uint32_t size, bool isLast, Diagnostics &diag)
{
    CPP_UTILITIES_UNUSED(diag)

    // make header
    FlacMetaDataBlockHeader header;
    header.setType(FlacMetaDataBlockType::Padding);
    header.setLast(isLast);
    header.setDataSize(size -= 4);
    header.makeHeader(stream);

    // write zeroes
    for (; size; --size) {
        stream.put(0);
    }
}

} // namespace TagParser
