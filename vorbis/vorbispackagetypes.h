#ifndef VORBISPACKAGETYPES_H
#define VORBISPACKAGETYPES_H

namespace Media {

/*!
 * \brief Encapsulates known Vorbis package type IDs.
 */
namespace VorbisPackageTypes {
enum KnownType : byte {
    Identification = 0x1,
    Comments = 0x3,
    Setup = 0x5
};
}

}

#endif // VORBISPACKAGETYPES_H
