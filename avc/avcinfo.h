#ifndef AVCINFO_H
#define AVCINFO_H

#include "tagparser/margin.h"
#include "tagparser/size.h"

namespace Media {

struct LIB_EXPORT TimingInfo {
    TimingInfo();
    uint32 unitsInTick;
    uint32 timeScale;
    bool isPresent;
    bool fixedFrameRate;
    int64 defaultDuration() const;
};

inline TimingInfo::TimingInfo() :
    unitsInTick(0),
    timeScale(0),
    isPresent(false),
    fixedFrameRate(false)
{}

inline int64 TimingInfo::defaultDuration() const
{
    return 1000000000ll * unitsInTick / timeScale;
}

struct LIB_EXPORT SpsInfo {
    SpsInfo();
    uint32 id;
    uint32 profileIndication;
    uint32 profileCompat;
    uint32 levelIdc;
    uint32 chromatFormatIndication;
    uint32 log2MaxFrameNum;
    uint32 offsetForNonRefPic;
    uint32 offsetForTopToBottomField;
    uint32 numRefFramesInPicOrderCntCycle;
    bool deltaPicOrderAlwaysZeroFlag;
    bool frameMbsOnly;
    bool vuiPresent;
    bool arFound;
    uint32 parNum;
    uint32 parDen;
    TimingInfo timingInfo;
    Margin cropping;
    Size size;
    uint32 checksum;

    void parse(std::istream &stream);
};

inline SpsInfo::SpsInfo() :
    id(0),
    profileIndication(0),
    profileCompat(0),
    levelIdc(0),
    chromatFormatIndication(0),
    log2MaxFrameNum(0),
    offsetForNonRefPic(0),
    offsetForTopToBottomField(0),
    numRefFramesInPicOrderCntCycle(0),
    deltaPicOrderAlwaysZeroFlag(false),
    frameMbsOnly(false),
    vuiPresent(false),
    arFound(false),
    parNum(0),
    parDen(0),
    checksum(0)
{}

struct PpsInfo {
    PpsInfo();
    uint32 id;
    uint32 spsId;
    bool picOrderPresent;
};

inline PpsInfo::PpsInfo() :
    id(0),
    spsId(0),
    picOrderPresent(false)
{}

struct SliceInfo {
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

class AvcFrame {
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
