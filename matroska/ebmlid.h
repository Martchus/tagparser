#ifndef TAG_PARSER_EBMLID_H
#define TAG_PARSER_EBMLID_H

namespace TagParser {

/*!
 * \brief Encapsulates all EBML ID values.
 */
namespace EbmlIds {

/*!
 * \brief Encapsulates all EBML header ID values.
 */
enum EbmlHeaderIds {
    Header = 0x1A45DFA3,
    Version = 0x4286,
    ReadVersion = 0x42F7,
    MaxIdLength = 0x42F2,
    MaxSizeLength = 0x42F3,
    DocType = 0x4282,
    DocTypeVersion = 0x4287,
    DocTypeReadVersion = 0x4285
};

/*!
 * \brief Encapsulates all global EBML ID values.
 */
enum GlobalIds { Void = 0xEC, Crc32 = 0xBF, SignatureSlot = 0x1b538667 };

/*!
 * \brief Encapsulates IDs in the SignatureSlot master.
 */
enum SignatureSlotIds { SignatureAlgo = 0x7e8a, SignatureHash = 0x7e9a, SignaturePublicKey = 0x7ea5, Signature = 0x7eb5, SignatureElements = 0x7e5b };

/*!
 * \brief Encapsulates IDs in the SignatureElements master.
 */
enum SignatureElementsSlotIds { SignatureElementList = 0x7e7b };

/*!
 * \brief Encapsulates IDs in the SignatureElementList master.
 */
enum SignatureElementListSlotIds { SignedElement = 0x6532 };

} // namespace EbmlIds

} // namespace TagParser

#endif // TAG_PARSER_EBMLID_H
