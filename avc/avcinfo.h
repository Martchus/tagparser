#ifndef TAG_PARSER_AVCINFO_H
#define TAG_PARSER_AVCINFO_H

#include "../aspectratio.h"
#include "../margin.h"
#include "../size.h"

namespace IoUtilities {
class BinaryReader;
class BitReader;
} // namespace IoUtilities

namespace TagParser {

/*!
 * \brief Type used to store unsigned integer values using golomb coding.
 */
using ugolomb = uint32;

/*!
 * \brief Type used to store signed integer values using golomb coding.
 */
using sgolomb = int32;

struct TAG_PARSER_EXPORT TimingInfo {
    constexpr TimingInfo();
    uint32 unitsInTick;
    uint32 timeScale;
    byte isPresent;
    byte fixedFrameRate;
    constexpr int64 defaultDuration() const;
};

constexpr TimingInfo::TimingInfo()
    : unitsInTick(0)
    , timeScale(0)
    , isPresent(0)
    , fixedFrameRate(0)
{
}

constexpr int64 TimingInfo::defaultDuration() const
{
    return 1000000000ll * unitsInTick / timeScale;
}

struct TAG_PARSER_EXPORT HrdParameters {
    constexpr HrdParameters();
    ugolomb cpbCount;
    byte bitRateScale;
    byte cpbSizeScale;
    byte initialCpbRemovalDelayLength;
    byte cpbRemovalDelayLength;
    byte cpbOutputDelayLength;
    byte timeOffsetLength;

    void parse(IoUtilities::BitReader &reader);
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
    byte profileIndication;
    byte profileConstraints;
    byte levelIndication;
    ugolomb chromaFormatIndication;
    ugolomb pictureOrderCountType;
    ugolomb log2MaxFrameNum;
    ugolomb log2MaxPictureOrderCountLsb;
    sgolomb offsetForNonRefPic;
    sgolomb offsetForTopToBottomField;
    ugolomb numRefFramesInPicOrderCntCycle;
    byte deltaPicOrderAlwaysZeroFlag;
    byte frameMbsOnly;
    byte vuiPresent;
    AspectRatio pixelAspectRatio;
    TimingInfo timingInfo;
    Margin cropping;
    Size pictureSize;
    byte hrdParametersPresent;
    HrdParameters nalHrdParameters;
    HrdParameters vclHrdParameters;
    byte pictureStructPresent;
    uint16 size;

    void parse(IoUtilities::BinaryReader &reader, uint32 maxSize);
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
    byte picOrderPresent;
    uint16 size;

    void parse(IoUtilities::BinaryReader &reader, uint32 maxSize);
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
    byte naluType;
    byte naluRefIdc;
    byte type;
    byte ppsId;
    uint32 frameNum;
    bool fieldPicFlag;
    bool bottomFieldFlag;
    uint32 idrPicId;
    uint32 picOrderCntLsb;
    uint32 deltaPicOrderCntBottom;
    uint32 deltaPicOrderCnt[2];
    uint32 firstMbInSlice;
    uint32 sps;
    uint32 pps;
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
    uint64 start;
    uint64 end;
    uint64 ref1;
    uint64 ref2;
    SliceInfo sliceInfo;
    uint32 presentationOrder;
    uint32 decodeOrder;
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
