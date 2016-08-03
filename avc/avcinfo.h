#ifndef AVCINFO_H
#define AVCINFO_H

#include "../margin.h"
#include "../size.h"
#include "../aspectratio.h"

namespace IoUtilities {
class BinaryReader;
class BitReader;
}

namespace Media {

/*!
 * \brief Type used to store unsigned integer values using golomb coding.
 */
typedef uint32 ugolomb;

/*!
 * \brief Type used to store signed integer values using golomb coding.
 */
typedef int32 sgolomb;

struct LIB_EXPORT TimingInfo {
    TimingInfo();
    uint32 unitsInTick;
    uint32 timeScale;
    byte isPresent;
    byte fixedFrameRate;
    int64 defaultDuration() const;
};

inline TimingInfo::TimingInfo() :
    unitsInTick(0),
    timeScale(0),
    isPresent(0),
    fixedFrameRate(0)
{}

inline int64 TimingInfo::defaultDuration() const
{
    return 1000000000ll * unitsInTick / timeScale;
}

struct LIB_EXPORT HrdParameters {
    HrdParameters();
    ugolomb cpbCount;
    byte bitRateScale;
    byte cpbSizeScale;
    byte initialCpbRemovalDelayLength;
    byte cpbRemovalDelayLength;
    byte cpbOutputDelayLength;
    byte timeOffsetLength;

    void parse(IoUtilities::BitReader &reader);
};

inline HrdParameters::HrdParameters() :
    cpbCount(0),
    bitRateScale(0),
    cpbSizeScale(0),
    initialCpbRemovalDelayLength(0),
    cpbRemovalDelayLength(0),
    cpbOutputDelayLength(0),
    timeOffsetLength(0)
{}

struct LIB_EXPORT SpsInfo {
    SpsInfo();
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

inline SpsInfo::SpsInfo() :
    id(0),
    profileIndication(0),
    profileConstraints(0),
    levelIndication(0),
    chromaFormatIndication(0),
    pictureOrderCountType(0),
    log2MaxFrameNum(0),
    log2MaxPictureOrderCountLsb(0),
    offsetForNonRefPic(0),
    offsetForTopToBottomField(0),
    numRefFramesInPicOrderCntCycle(0),
    deltaPicOrderAlwaysZeroFlag(0),
    frameMbsOnly(0),
    vuiPresent(0),
    hrdParametersPresent(0),
    pictureStructPresent(0),
    size(0)
{}

struct LIB_EXPORT PpsInfo {
    PpsInfo();
    ugolomb id;
    ugolomb spsId;
    byte picOrderPresent;
    uint16 size;

    void parse(IoUtilities::BinaryReader &reader, uint32 maxSize);
};

inline PpsInfo::PpsInfo() :
    id(0),
    spsId(0),
    picOrderPresent(false),
    size(0)
{}

struct LIB_EXPORT SliceInfo {
    SliceInfo();
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

inline SliceInfo::SliceInfo() :
    naluType(0),
    naluRefIdc(0),
    type(0),
    ppsId(0),
    frameNum(0),
    fieldPicFlag(false),
    bottomFieldFlag(false),
    idrPicId(0),
    picOrderCntLsb(0),
    deltaPicOrderCntBottom(0),
    deltaPicOrderCnt{0,0},
    firstMbInSlice(0),
    sps(0),
    pps(0)
{}

class LIB_EXPORT AvcFrame {
    AvcFrame();

private:
    uint64 m_start;
    uint64 m_end;
    uint64 m_ref1;
    uint64 m_ref2;
    bool m_keyframe;
    bool m_hasProvidedTimecode;
    SliceInfo m_sliceInfo;
    uint32 m_presentationOrder;
    uint32 m_decodeOrder;
};

inline AvcFrame::AvcFrame() :
    m_start(0),
    m_end(0),
    m_ref1(0),
    m_ref2(0),
    m_keyframe(false),
    m_hasProvidedTimecode(false),
    m_presentationOrder(0),
    m_decodeOrder(0)
{}


}

#endif // AVCINFO_H
