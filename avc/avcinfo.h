#ifndef TAG_PARSER_AVCINFO_H
#define TAG_PARSER_AVCINFO_H

#include "../aspectratio.h"
#include "../margin.h"
#include "../size.h"

namespace CppUtilities {
class BinaryReader;
class BitReader;
} // namespace CppUtilities

namespace TagParser {

/*!
 * \brief Type used to store unsigned integer values using golomb coding.
 */
using ugolomb = std::uint32_t;

/*!
 * \brief Type used to store signed integer values using golomb coding.
 */
using sgolomb = std::int32_t;

struct TAG_PARSER_EXPORT TimingInfo {
    constexpr TimingInfo();
    std::uint32_t unitsInTick;
    std::uint32_t timeScale;
    std::uint8_t isPresent;
    std::uint8_t fixedFrameRate;
    constexpr std::int64_t defaultDuration() const;
};

constexpr TimingInfo::TimingInfo()
    : unitsInTick(0)
    , timeScale(0)
    , isPresent(0)
    , fixedFrameRate(0)
{
}

constexpr std::int64_t TimingInfo::defaultDuration() const
{
    return 1000000000ll * unitsInTick / timeScale;
}

struct TAG_PARSER_EXPORT HrdParameters {
    constexpr HrdParameters();
    ugolomb cpbCount;
    std::uint8_t bitRateScale;
    std::uint8_t cpbSizeScale;
    std::uint8_t initialCpbRemovalDelayLength;
    std::uint8_t cpbRemovalDelayLength;
    std::uint8_t cpbOutputDelayLength;
    std::uint8_t timeOffsetLength;

    void parse(CppUtilities::BitReader &reader);
};

constexpr HrdParameters::HrdParameters()
    : cpbCount(0)
    , bitRateScale(0)
    , cpbSizeScale(0)
    , initialCpbRemovalDelayLength(0)
    , cpbRemovalDelayLength(0)
    , cpbOutputDelayLength(0)
    , timeOffsetLength(0)
{
}

struct TAG_PARSER_EXPORT SpsInfo {
    constexpr SpsInfo();
    ugolomb id;
    std::uint8_t profileIndication;
    std::uint8_t profileConstraints;
    std::uint8_t levelIndication;
    ugolomb chromaFormatIndication;
    ugolomb pictureOrderCountType;
    ugolomb log2MaxFrameNum;
    ugolomb log2MaxPictureOrderCountLsb;
    sgolomb offsetForNonRefPic;
    sgolomb offsetForTopToBottomField;
    ugolomb numRefFramesInPicOrderCntCycle;
    std::uint8_t deltaPicOrderAlwaysZeroFlag;
    std::uint8_t frameMbsOnly;
    std::uint8_t vuiPresent;
    AspectRatio pixelAspectRatio;
    TimingInfo timingInfo;
    Margin cropping;
    Size pictureSize;
    std::uint8_t hrdParametersPresent;
    HrdParameters nalHrdParameters;
    HrdParameters vclHrdParameters;
    std::uint8_t pictureStructPresent;
    std::uint16_t size;
    static constexpr std::uint16_t minSize = 2;

    void parse(CppUtilities::BinaryReader &reader, std::uint32_t maxSize);
};

constexpr SpsInfo::SpsInfo()
    : id(0)
    , profileIndication(0)
    , profileConstraints(0)
    , levelIndication(0)
    , chromaFormatIndication(0)
    , pictureOrderCountType(0)
    , log2MaxFrameNum(0)
    , log2MaxPictureOrderCountLsb(0)
    , offsetForNonRefPic(0)
    , offsetForTopToBottomField(0)
    , numRefFramesInPicOrderCntCycle(0)
    , deltaPicOrderAlwaysZeroFlag(0)
    , frameMbsOnly(0)
    , vuiPresent(0)
    , hrdParametersPresent(0)
    , pictureStructPresent(0)
    , size(0)
{
}

struct TAG_PARSER_EXPORT PpsInfo {
    constexpr PpsInfo();
    ugolomb id;
    ugolomb spsId;
    std::uint8_t picOrderPresent;
    std::uint16_t size;
    static constexpr std::uint16_t minSize = 2;

    void parse(CppUtilities::BinaryReader &reader, std::uint32_t maxSize);
};

constexpr PpsInfo::PpsInfo()
    : id(0)
    , spsId(0)
    , picOrderPresent(false)
    , size(0)
{
}

struct TAG_PARSER_EXPORT SliceInfo {
    constexpr SliceInfo();
    std::uint8_t naluType;
    std::uint8_t naluRefIdc;
    std::uint8_t type;
    std::uint8_t ppsId;
    std::uint32_t frameNum;
    bool fieldPicFlag;
    bool bottomFieldFlag;
    std::uint32_t idrPicId;
    std::uint32_t picOrderCntLsb;
    std::uint32_t deltaPicOrderCntBottom;
    std::uint32_t deltaPicOrderCnt[2];
    std::uint32_t firstMbInSlice;
    std::uint32_t sps;
    std::uint32_t pps;
};

constexpr SliceInfo::SliceInfo()
    : naluType(0)
    , naluRefIdc(0)
    , type(0)
    , ppsId(0)
    , frameNum(0)
    , fieldPicFlag(false)
    , bottomFieldFlag(false)
    , idrPicId(0)
    , picOrderCntLsb(0)
    , deltaPicOrderCntBottom(0)
    , deltaPicOrderCnt{ 0, 0 }
    , firstMbInSlice(0)
    , sps(0)
    , pps(0)
{
}

struct TAG_PARSER_EXPORT AvcFrame {
    constexpr AvcFrame();
    std::uint64_t start;
    std::uint64_t end;
    std::uint64_t ref1;
    std::uint64_t ref2;
    SliceInfo sliceInfo;
    std::uint32_t presentationOrder;
    std::uint32_t decodeOrder;
    bool keyframe;
    bool hasProvidedTimecode;
};

constexpr AvcFrame::AvcFrame()
    : start(0)
    , end(0)
    , ref1(0)
    , ref2(0)
    , presentationOrder(0)
    , decodeOrder(0)
    , keyframe(false)
    , hasProvidedTimecode(false)
{
}

} // namespace TagParser

#endif // TAG_PARSER_AVCINFO_H
