#ifndef MEDIA_MATROSKATRACK_H
#define MEDIA_MATROSKATRACK_H

#include "../abstracttrack.h"

namespace Media {

class EbmlElement;
class MatroskaContainer;

class LIB_EXPORT MatroskaTrack : public AbstractTrack
{
    friend class MatroskaContainer;

public:
    MatroskaTrack(EbmlElement &trackElement);
    ~MatroskaTrack();

    TrackType type() const;

    static MediaFormat codecIdToMediaFormat(const std::string &codecId);

protected:
    void internalParseHeader();

private:
    EbmlElement *m_trackElement;
};

}

#endif // MEDIA_MATROSKATRACK_H
