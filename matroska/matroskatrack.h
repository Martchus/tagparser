#ifndef TAG_PARSER_MATROSKATRACK_H
#define TAG_PARSER_MATROSKATRACK_H

#include "../abstracttrack.h"

namespace TagParser {

class EbmlElement;
class MatroskaContainer;
class MatroskaTrack;
class MatroskaTag;

class TAG_PARSER_EXPORT MatroskaTrackHeaderMaker {
    friend class MatroskaTrack;

public:
    void make(std::ostream &stream) const;
    const MatroskaTrack &track() const;
    std::uint64_t requiredSize() const;

private:
    MatroskaTrackHeaderMaker(const MatroskaTrack &track, Diagnostics &diag);

    const MatroskaTrack &m_track;
    const std::string &m_language;
    const std::string &m_languageIETF;
    std::uint64_t m_dataSize;
    std::uint64_t m_requiredSize;
    std::uint8_t m_sizeDenotationLength;
};

/*!
 * \brief Returns the number of bytes which will be written when making the track.
 */
inline const MatroskaTrack &MatroskaTrackHeaderMaker::track() const
{
    return m_track;
}

/*!
 * \brief Returns the number of bytes which will be written when calling make().
 */
inline std::uint64_t MatroskaTrackHeaderMaker::requiredSize() const
{
    return m_requiredSize;
}

class TAG_PARSER_EXPORT MatroskaTrack : public AbstractTrack {
    friend class MatroskaContainer;
    friend class MatroskaTrackHeaderMaker;

public:
    MatroskaTrack(EbmlElement &trackElement);
    ~MatroskaTrack() override;

    TrackType type() const override;

    static MediaFormat codecIdToMediaFormat(const std::string &codecId);
    void readStatisticsFromTags(const std::vector<std::unique_ptr<MatroskaTag>> &tags, Diagnostics &diag);
    MatroskaTrackHeaderMaker prepareMakingHeader(Diagnostics &diag) const;
    void makeHeader(std::ostream &stream, Diagnostics &diag) const;

protected:
    void internalParseHeader(Diagnostics &diag, AbortableProgressFeedback &progress) override;

private:
    template <typename PropertyType, typename ConversionFunction>
    void assignPropertyFromTagValue(const std::unique_ptr<MatroskaTag> &tag, std::string_view fieldId, PropertyType &integer,
        const ConversionFunction &conversionFunction, Diagnostics &diag);

    EbmlElement *m_trackElement;
};

/*!
 * \brief Prepares making header.
 * \returns Returns a MatroskaTrackHeaderMaker object which can be used to actually make the track
 *          header.
 * \remarks The track must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make() method of the returned object.
 * \throws Throws TagParser::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the track header before making
 * it.
 * \sa make()
 */
inline MatroskaTrackHeaderMaker MatroskaTrack::prepareMakingHeader(Diagnostics &diag) const
{
    return MatroskaTrackHeaderMaker(*this, diag);
}

/*!
 * \brief Writes header information to the specified \a stream (makes a "TrackEntry"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 * \sa prepareMaking()
 */
inline void MatroskaTrack::makeHeader(std::ostream &stream, Diagnostics &diag) const
{
    prepareMakingHeader(diag).make(stream);
}

} // namespace TagParser

#endif // TAG_PARSER_MATROSKATRACK_H
