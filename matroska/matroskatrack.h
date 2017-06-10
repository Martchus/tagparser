#ifndef MEDIA_MATROSKATRACK_H
#define MEDIA_MATROSKATRACK_H

#include "../abstracttrack.h"

namespace Media {

class EbmlElement;
class MatroskaContainer;
class MatroskaTrack;

class TAG_PARSER_EXPORT MatroskaTrackHeaderMaker
{
    friend class MatroskaTrack;

public:
    void make(std::ostream &stream) const;
    const MatroskaTrack &track() const;
    uint64 requiredSize() const;

private:
    MatroskaTrackHeaderMaker(const MatroskaTrack &track);

    const MatroskaTrack &m_track;
};

/*!
 * \brief Returns the number of bytes which will be written when making the track.
 */
inline const MatroskaTrack &MatroskaTrackHeaderMaker::track() const
{
    return m_track;
}

class TAG_PARSER_EXPORT MatroskaTrack : public AbstractTrack
{
    friend class MatroskaContainer;
    friend class MatroskaTrackHeaderMaker;

public:
    MatroskaTrack(EbmlElement &trackElement);
    ~MatroskaTrack();

    TrackType type() const;

    static MediaFormat codecIdToMediaFormat(const std::string &codecId);
    MatroskaTrackHeaderMaker prepareMakingHeader() const;
    void makeHeader(std::ostream &stream) const;

protected:
    void internalParseHeader();

private:
    EbmlElement *m_trackElement;
};

/*!
 * \brief Prepares making header.
 * \returns Returns a MatroskaTrackHeaderMaker object which can be used to actually make the track
 *          header.
 * \remarks The track must NOT be mutated after making is prepared when it is intended to actually
 *          make the tag using the make() method of the returned object.
 * \throws Throws Media::Failure or a derived exception when a making error occurs.
 *
 * This method might be useful when it is necessary to know the size of the track header before making
 * it.
 * \sa make()
 * \todo Make inline in next major release.
 */
inline MatroskaTrackHeaderMaker MatroskaTrack::prepareMakingHeader() const
{
    return MatroskaTrackHeaderMaker(*this);
}

/*!
 * \brief Writes header information to the specified \a stream (makes a "TrackEntry"-element).
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 * \sa prepareMaking()
 * \todo Make inline in next major release.
 */
inline void MatroskaTrack::makeHeader(std::ostream &stream) const
{
    prepareMakingHeader().make(stream);
}

}

#endif // MEDIA_MATROSKATRACK_H
