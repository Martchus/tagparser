#include "./aacframe.h"
#include "./aaccodebook.h"

#include "../adts/adtsframe.h"

#include "../mp4/mp4ids.h"

#include "../exceptions.h"

#include <c++utilities/io/bitreader.h>

#include <istream>
#include <limits>

using namespace std;
using namespace CppUtilities;

/*!
 * \file aacframe.cpp
 * \remarks The AAC parser is still WIP. It does not work yet and its API/ABI may change even in patch releases.
 */

namespace TagParser {

/// \cond

const std::uint8_t maxPredictionSfb[16] = { 33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34, 64, 64, 64, 64 };

const uint8_t swb512WindowCount[] = { 0, 0, 0, 36, 36, 37, 31, 31, 0, 0, 0, 0 };

const std::uint8_t swb480WindowCount[] = { 0, 0, 0, 35, 35, 37, 30, 30, 0, 0, 0, 0 };

const std::uint8_t swb960WindowCount[] = { 40, 40, 45, 49, 49, 49, 46, 46, 42, 42, 42, 40 };

const std::uint8_t swb1024WindowCount[] = { 41, 41, 47, 49, 49, 51, 47, 47, 43, 43, 43, 40 };

const std::uint8_t swb128WindowCount[] = { 12, 12, 12, 14, 14, 14, 15, 15, 15, 15, 15, 15 };

const std::uint16_t swbOffset1024_96[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 156, 172,
    188, 212, 240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024 };

const std::uint16_t swbOffset128_96[] = { 0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128 };

const std::uint16_t swbOffset1024_64[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 100, 112, 124, 140, 156, 172, 192,
    216, 240, 268, 304, 344, 384, 424, 464, 504, 544, 584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 984, 1024 };

const std::uint16_t swbOffset128_64[] = { 0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128 };

const std::uint16_t swbOffset1024_48[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160, 176, 196,
    216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 1024 };

const std::uint16_t swbOffset512_48[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 68, 76, 84, 92, 100, 112, 124, 136, 148, 164,
    184, 208, 236, 268, 300, 332, 364, 396, 428, 460, 512 };

const std::uint16_t swbOffset480_48[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 156, 172,
    188, 212, 240, 272, 304, 336, 368, 400, 432, 480 };

const std::uint16_t swbOffset128_48[] = { 0, 4, 8, 12, 16, 20, 28, 36, 44, 56, 68, 80, 96, 112, 128 };

const std::uint16_t swbOffset1024_32[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160, 176, 196,
    216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024 };

const std::uint16_t swbOffset512_32[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160, 176,
    192, 212, 236, 260, 288, 320, 352, 384, 416, 448, 480, 512 };

const std::uint16_t swbOffset480_32[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 72, 80, 88, 96, 104, 112, 124, 136, 148,
    164, 180, 200, 224, 256, 288, 320, 352, 384, 416, 448, 480 };

const std::uint16_t swbOffset1024_24[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124, 136, 148, 160,
    172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396, 432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960, 1024 };

const std::uint16_t swbOffset512_24[]
    = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 80, 92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };

const std::uint16_t swbOffset480_24[]
    = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 80, 92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480 };

const std::uint16_t swbOffset128_24[] = { 0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 64, 76, 92, 108, 128 };

const std::uint16_t swbOffset1024_16[] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 212, 228, 244,
    260, 280, 300, 320, 344, 368, 396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024 };

const std::uint16_t swbOffset128_16[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 60, 72, 88, 108, 128 };

const std::uint16_t swbOffset1024_8[] = { 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 172, 188, 204, 220, 236, 252, 268, 288, 308,
    328, 348, 372, 396, 420, 448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024 };

const std::uint16_t swbOffset128_8[] = { 0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 60, 72, 88, 108, 128 };

static const std::uint16_t *const swbOffset1024Window[] = {
    swbOffset1024_96, /* 96000 */
    swbOffset1024_96, /* 88200 */
    swbOffset1024_64, /* 64000 */
    swbOffset1024_48, /* 48000 */
    swbOffset1024_48, /* 44100 */
    swbOffset1024_32, /* 32000 */
    swbOffset1024_24, /* 24000 */
    swbOffset1024_24, /* 22050 */
    swbOffset1024_16, /* 16000 */
    swbOffset1024_16, /* 12000 */
    swbOffset1024_16, /* 11025 */
    swbOffset1024_8, /* 8000  */
};

static const std::uint16_t *const swbOffset512Window[] = {
    nullptr, /* 96000 */
    nullptr, /* 88200 */
    nullptr, /* 64000 */
    swbOffset512_48, /* 48000 */
    swbOffset512_48, /* 44100 */
    swbOffset512_32, /* 32000 */
    swbOffset512_24, /* 24000 */
    swbOffset512_24, /* 22050 */
    nullptr, /* 16000 */
    nullptr, /* 12000 */
    nullptr, /* 11025 */
    nullptr, /* 8000  */
};

static const std::uint16_t *const swbOffset480Window[] = {
    nullptr, /* 96000 */
    nullptr, /* 88200 */
    nullptr, /* 64000 */
    swbOffset480_48, /* 48000 */
    swbOffset480_48, /* 44100 */
    swbOffset480_32, /* 32000 */
    swbOffset480_24, /* 24000 */
    swbOffset480_24, /* 22050 */
    nullptr, /* 16000 */
    nullptr, /* 12000 */
    nullptr, /* 11025 */
    nullptr, /* 8000  */
};

static const std::uint16_t *const swbOffset128Window[] = {
    swbOffset128_96, /* 96000 */
    swbOffset128_96, /* 88200 */
    swbOffset128_64, /* 64000 */
    swbOffset128_48, /* 48000 */
    swbOffset128_48, /* 44100 */
    swbOffset128_48, /* 32000 */
    swbOffset128_24, /* 24000 */
    swbOffset128_24, /* 22050 */
    swbOffset128_16, /* 16000 */
    swbOffset128_16, /* 12000 */
    swbOffset128_16, /* 11025 */
    swbOffset128_8 /* 8000  */
};

/*!
 * \brief Constructs a new LTP info object.
 */
AacLtpInfo::AacLtpInfo()
    : lastBand(0)
    , dataPresent(0)
    , lag(0)
    , lagUpdate(0)
    , coef(0)
    , longUsed{ 0 }
    , shortUsed{ 0 }
    , shortLagPresent{ 0 }
    , shortLag{ 0 }
{
}

/*!
 * \brief Constructs a new predictor info object.
 */
AacPredictorInfo::AacPredictorInfo()
    : maxSfb(0)
    , reset(0)
    , resetGroupNumber(0)
    , predictionUsed{ 0 }
{
}

/*!
 * \brief Constructs a new pulse info object.
 */
AacPulseInfo::AacPulseInfo()
    : count(0)
    , startSfb(0)
    , offset{ 0 }
    , amp{ 0 }
{
}

/*!
 * \brief Constructs a new TNS info object.
 */
AacTnsInfo::AacTnsInfo()
    : filt{ 0 }
    , coefRes{ 0 }
    , length{ { 0 } }
    , order{ { 0 } }
    , direction{ { 0 } }
    , coefCompress{ { 0 } }
    , coef{ { { 0 } } }
{
}

/*!
 * \brief Constructs a new SSR info object.
 */
AacSsrInfo::AacSsrInfo()
    : maxBand(0)
    , adjustNum{ { 0 } }
    , alevcode{ { { 0 } } }
    , aloccode{ { { 0 } } }
{
}

/*!
 * \brief Constructs a new DRC info object.
 */
AacDrcInfo::AacDrcInfo()
    : present(0)
    , bandCount(0)
    , pceInstanceTag(0)
    , excludedChannelsPresent(0)
    , bandTop{ 0 }
    , progRefLevel(0)
    , dynamicRangeSign{ 0 }
    , dynamicRangeControl{ 0 }
    , excludeMask{ 0 }
    , additionalExcludedChannels{ 0 }
{
}

/*!
 * \brief Constructs a new PS info object.
 */
AacPsInfo::AacPsInfo()
{
}

/*!
 * \brief Constructs a new DRM-PS info object.
 */
AacDrmPsInfo::AacDrmPsInfo()
{
}

/*!
 * \brief Constructs a new SBR info object.
 */
AacSbrInfo::AacSbrInfo(std::uint8_t sbrElementType, std::uint16_t samplingFrequency, std::uint16_t frameLength, bool isDrm)
    : aacElementId(sbrElementType)
    , samplingFrequency(samplingFrequency)
    ,

    maxAacLine(0)
    ,

    rate(0)
    , justSeeked(0)
    , ret(0)
    ,

    ampRes{ 0 }
    ,

    k0(0)
    , kx(0)
    , m(0)
    , nMaster(0)
    , nHigh(0)
    , nLow(0)
    , nq(0)
    , nl{ 0 }
    , n{ 0 }
    ,

    fMaster{ 0 }
    , fTableRes{ { 0 } }
    , fTableNoise{ 0 }
    , fTableLim{ { 0 } }
    , fGroup{ { 0 } }
    , ng{ 0 }
    ,

    tableMapKToG{ 0 }
    ,

    absBordLead{ 0 }
    , absBordTrail{ 0 }
    , relLeadCount{ 0 }
    , relTrailCount{ 0 }
    ,

    le{ 0 }
    , lePrev{ 0 }
    , lq{ 0 }
    ,

    te{ { 0 } }
    , tq{ { 0 } }
    , f{ { 0 } }
    , fPrev{ 0 }
    ,

    //*G_temp_prev{{0}},
    //*Q_temp_prev{{0}},
    //GQ_ringbuf_index{0},

    e{ { { 0 } } }
    , ePrev{ { 0 } }
    ,
    //E_orig{{{0}}},
    //E_curr{{{0}}},
    q{ { { 0 } } }
    ,
    //Q_div{{{0}}},
    //Q_div2{{{0}}},
    qPrev{ { 0 } }
    ,

    la{ 0 }
    , laPrev{ 0 }
    ,

    bsInvfMode{ { 0 } }
    , bsInvfModePrev{ { 0 } }
    ,
    //bwArray{{0}},
    //bwArray_prev{{0}},

    noPatches(0)
    , patchNoSubbands{ 0 }
    , patchStartSubband{ 0 }
    ,

    bsAddHarmonic{ { 0 } }
    , bsAddHarmonicPrev{ { 0 } }
    ,

    indexNoisePrev{ 0 }
    , psiIsPrev{ 0 }
    ,

    bsStartFreqPrev(numeric_limits<std::uint8_t>::max())
    , bsStopFreqPrev(0)
    , bsXoverBandPrev(0)
    , bsFreqScalePrev(0)
    , bsAlterScalePrev(0)
    , bsNoiseBandsPrev(0)
    ,

    prevEnvIsShort{ -1, -1 }
    ,

    kxPrev(0)
    , bsco(0)
    , bscoPrev(0)
    , mPrev(0)
    , frameLength(frameLength)
    ,

    reset(1)
    , frame(0)
    , headerCount(0)
    ,

    idAac(0)
    ,
    //qmfa_info *qmfa{0},
    //qmfs_info *qmfs{0},

    //qmf_t Xsbr{{{0}}},

    isDrmSbr(isDrm)
    ,
    //drm_ps_info *drm_ps;

    timeSlotsRateCount(aacSbrRate * (frameLength == 960 ? aacNoTimeSlots960 : aacNoTimeSlots))
    , timeSlotsCount(frameLength == 960 ? aacNoTimeSlots960 : aacNoTimeSlots)
    , tHfGen(8)
    , tHfAdj(2)
    ,

    psUsed(0)
    , psResetFlag(0)
    ,

    bsHeaderFlag(0)
    , bsCrcFlag(0)
    , bsSbrCrcBits(0)
    , bsProtocolVersion(0)
    , bsAmpRes(1)
    , bsStartFreq(5)
    , bsStopFreq(0)
    , bsXoverBand(0)
    , bsFreqScale(2)
    , bsAlterScale(1)
    , bsNoiseBands(2)
    , bsLimiterBands(2)
    , bsLimiterGains(2)
    , bsInterpolFreq(1)
    , bsSmoothingMode(1)
    , bsSamplerateMode(1)
    , bsAddHarmonicFlag{ 0 }
    , bsAddHarmonicFlagPrev{ 0 }
    , bsExtendedData(0)
    , bsExtensionId(0)
    , bsExtensionData(0)
    , bsCoupling(0)
    , bsFrameClass{ 0 }
    , bsRelBord{ { 0 } }
    , bsRelBord0{ { 0 } }
    , bsRelBord1{ { 0 } }
    , bsPointer{ 0 }
    , bsAbsBord0{ 0 }
    , bsAbsBord1{ 0 }
    , bsRelCount0{ 0 }
    , bsRelCount1{ 0 }
    , bsDfEnv{ { 0 } }
    , bsDfNoise{ { 0 } }
{
    throw NotImplementedException(); // TODO
    /*
    switch (sbrElementType) {
        using namespace AacSyntaxElementTypes;
    case ChannelPairElement:

        break;
    default:;
    }
    */
}

/*!
 * \brief Constructs a new program config object.
 */
AacProgramConfig::AacProgramConfig()
    : elementInstanceTag(0)
    , objectType(0)
    , samplingFrequencyIndex(0)
    , frontChannelElementCount(0)
    , sideChannelElementCount(0)
    , backChannelElementCount(0)
    , lfeChannelElementCount(0)
    , assocDataElementCount(0)
    , validCcElementCount(0)
    , monoMixdownPresent(0)
    , monoMixdownElementNumber(0)
    , stereoMixdownPresent(0)
    , stereoMixdownElementNumber(0)
    , matrixMixdownIdxPresent(0)
    , pseudoSurroundEnable(0)
    , matrixMixdownIdx(0)
    , frontElementIsCpe{ 0 }
    , frontElementTagSelect{ 0 }
    , sideElementIsCpe{ 0 }
    , sideElementTagSelect{ 0 }
    , backElementIsCpe{ 0 }
    , backElementTagSelect{ 0 }
    , lfeElementTagSelect{ 0 }
    , assocDataElementTagSelect{ 0 }
    , ccElementIsIndSw{ 0 }
    , validCcElementTagSelect{ 0 }
    , channels(0)
    , commentFieldBytes(0)
    , commentFieldData{ 0 }
    , frontChannelCount(0)
    , sideChannelCount(0)
    , backChannelCount(0)
    , lfeChannelCount(0)
    , sceChannel{ 0 }
    , cpeChannel{ 0 }
{
}

/*!
 * \brief Constructs a new ICS info object.
 */
AacIcsInfo::AacIcsInfo()
    : maxSfb(0)
    , swbCount(0)
    , windowGroupCount(0)
    , windowCount(0)
    , windowSequence(0)
    , windowGroupLengths{ 0 }
    , windowShape(0)
    , scaleFactorGrouping(0)
    , sectionSfbOffset{ { 0 } }
    , swbOffset{ 0 }
    , maxSwbOffset(0)
    , sectionCb{ { 0 } }
    , sectionStart{ { 0 } }
    , sectionEnd{ { 0 } }
    , sfbCb{ { 0 } }
    , sectionsPerGroup{ 0 }
    , globalGain(0)
    , scaleFactors{ { 0 } }
    , midSideCodingMaskPresent(0)
    , midSideCodingUsed{ { 0 } }
    , noiseUsed(0)
    , isUsed(0)
    , pulseDataPresent(0)
    , tnsDataPresent(0)
    , gainControlPresent(0)
    , predictorDataPresent(0)
    , reorderedSpectralDataLength(0)
    , longestCodewordLength(0)
    , sfConcealment(0)
    , revGlobalGain(0)
    , rvlcSfLength(0)
    , dpcmNoiseNrg(0)
    , sfEscapesPresent(0)
    , rvlcEscapesLength(0)
    , dpcmNoiseLastPos(0)
{
}

/*!
 * \class TagParser::AacFrameElementParser
 * \brief The AacFrameElementParser class parses AAC frame elements.
 * \remarks
 *  - Only reads reads the basic syntax yet; does not reconstruct samples.
 *  - This class is not well tested yet.
 */

/*!
 * \brief Parses "Long Term Prediction" info.
 */
void AacFrameElementParser::parseLtpInfo(const AacIcsInfo &ics, AacLtpInfo &ltp)
{
    ltp.lag = 0;
    if (m_mpeg4AudioObjectId == Mpeg4AudioObjectIds::ErAacLd) {
        if ((ltp.lagUpdate = m_reader.readBit())) {
            ltp.lag = m_reader.readBits<std::uint16_t>(10);
        }
    } else {
        ltp.lag = m_reader.readBits<std::uint16_t>(11);
    }
    if (ltp.lag > (m_frameLength << 1)) {
        throw InvalidDataException();
    }
    ltp.coef = m_reader.readBits<std::uint8_t>(3);
    switch (ics.windowSequence) {
    case AacIcsSequenceTypes::EightShortSequence:
        for (std::uint8_t w = 0; w < ics.windowCount; ++w) {
            if ((ltp.shortUsed[w] = m_reader.readBit())) {
                if ((ltp.shortLagPresent[w] = m_reader.readBit())) {
                    ltp.shortLag[w] = m_reader.readBits<std::uint8_t>(4);
                }
            }
        }
        break;
    default:
        ltp.lastBand = std::min<std::uint8_t>(ics.maxSfb, aacMaxLtpSfb);
        for (std::uint8_t sfb = 0; sfb < ltp.lastBand; ++sfb) {
            ltp.longUsed[sfb] = m_reader.readBit();
        }
    }
}

/*!
 * \brief Parses "individual channel stream" info.
 */
void AacFrameElementParser::parseIcsInfo(AacIcsInfo &ics)
{
    using namespace AacIcsSequenceTypes;
    if (m_reader.readBit()) { // isc reserved bit (must be 0)
        throw InvalidDataException();
    }
    ics.windowSequence = m_reader.readBits<std::uint8_t>(2);
    ics.windowShape = m_reader.readBit();
    if (m_mpeg4AudioObjectId == Mpeg4AudioObjectIds::ErAacLd && ics.windowSequence != OnlyLongSequence) {
        throw InvalidDataException(); // no block switching in LD
    }
    if (ics.windowSequence == EightShortSequence) {
        ics.maxSfb = m_reader.readBits<std::uint8_t>(4);
        ics.scaleFactorGrouping = m_reader.readBits<std::uint8_t>(7);
    } else {
        ics.maxSfb = m_reader.readBits<std::uint8_t>(6);
    }
    calculateWindowGroupingInfo(ics);
    if (ics.windowSequence != EightShortSequence) {
        if ((ics.predictorDataPresent = m_reader.readBit())) {
            switch (m_mpeg4AudioObjectId) {
            case Mpeg4AudioObjectIds::AacMain:
                // MPEG-2 style AAC predictor
                if ((ics.predictor.reset = m_reader.readBit())) {
                    ics.predictor.resetGroupNumber = m_reader.readBits<std::uint8_t>(5);
                    ics.predictor.maxSfb = ics.maxSfb;
                }
                if (ics.predictor.maxSfb > maxPredictionSfb[m_mpeg4SamplingFrequencyIndex]) {
                    ics.predictor.maxSfb = maxPredictionSfb[m_mpeg4SamplingFrequencyIndex];
                }
                for (std::uint8_t sfb = 0; sfb < ics.predictor.maxSfb; ++sfb) {
                    ics.predictor.predictionUsed[sfb] = m_reader.readBit();
                }
                break;
            default:
                // "Long Term Prediction"
                if (m_mpeg4AudioObjectId < Mpeg4AudioObjectIds::ErAacLc) {
                    if ((ics.ltp1.dataPresent = m_reader.readBit())) {
                        parseLtpInfo(ics, ics.ltp1);
                    }
                    if (m_commonWindow) {
                        if ((ics.ltp2.dataPresent = m_reader.readBit())) {
                            parseLtpInfo(ics, ics.ltp2);
                        }
                    }
                }
                if (!m_commonWindow && (m_mpeg4AudioObjectId >= Mpeg4AudioObjectIds::ErAacLc)) {
                    if ((ics.ltp1.dataPresent = m_reader.readBit())) {
                        parseLtpInfo(ics, ics.ltp1);
                    }
                }
            }
        }
    }
}

/*!
 * \brief Parses section data.
 */
void AacFrameElementParser::parseSectionData(AacIcsInfo &ics)
{
    const std::uint8_t sectionBits = ics.windowSequence == AacIcsSequenceTypes::EightShortSequence ? 3 : 5;
    const std::uint8_t sectionEscValue = static_cast<std::uint8_t>((1 << sectionBits) - 1);
    for (std::uint8_t groupIndex = 0, sectionIndex = 0; groupIndex < ics.windowGroupCount; ++groupIndex, sectionIndex = 0) {
        for (std::uint8_t i = 0, sectionLength, sectionLengthIncrease; i < ics.maxSfb; i += sectionLength, ++sectionIndex) {
            ics.sectionCb[groupIndex][sectionIndex] = m_reader.readBits<std::uint8_t>(m_aacSectionDataResilienceFlag ? 5 : 4);
            sectionLength = 0;
            sectionLengthIncrease
                = (!m_aacSectionDataResilienceFlag && (ics.sectionCb[groupIndex][sectionIndex] < 16 || ics.sectionCb[groupIndex][sectionIndex] > 32)
                      && ics.sectionCb[groupIndex][sectionIndex] != 11)
                ? m_reader.readBits<std::uint8_t>(sectionBits)
                : 1;
            while (sectionLengthIncrease == sectionEscValue) {
                sectionLength += sectionLengthIncrease;
                sectionLengthIncrease = m_reader.readBits<std::uint8_t>(sectionBits);
            }
            sectionLength += sectionLengthIncrease;
            ics.sectionStart[groupIndex][sectionIndex] = i;
            ics.sectionEnd[groupIndex][sectionIndex] = i + sectionLength;
            if (ics.windowSequence == AacIcsSequenceTypes::EightShortSequence) {
                if (i + sectionLength > 8 * 15) {
                    throw InvalidDataException();
                } else if (sectionIndex >= 8 * 15) {
                    throw InvalidDataException();
                }
            } else {
                if (i + sectionLength > aacMaxSfb) {
                    throw InvalidDataException();
                } else if (sectionIndex >= aacMaxSfb) {
                    throw InvalidDataException();
                }
            }
            for (std::uint8_t sfb = i; sfb < i + sectionLength; ++sfb) {
                ics.sfbCb[groupIndex][sfb] = ics.sectionCb[groupIndex][sectionIndex];
            }
        }
        ics.sectionsPerGroup[groupIndex] = sectionIndex;
    }
}

/*!
 * \brief Decodes scale factor data (called by parseScaleFactorData()).
 */
void AacFrameElementParser::decodeScaleFactorData(AacIcsInfo &ics)
{
    std::int16_t tmp;
    std::uint8_t noisePcmFlag = 1;
    std::int16_t scaleFactor = ics.globalGain;
    std::int16_t isPosition = 0;
    std::int16_t noiseEnergy = ics.globalGain - 90;
    using namespace AacScaleFactorTypes;
    for (std::uint8_t group = 0; group < ics.windowGroupCount; ++group) {
        for (std::uint8_t sfb = 0; sfb < ics.maxSfb; ++sfb) {
            switch (ics.sfbCb[group][sfb]) {
            case ZeroHcb: // zero book
                ics.scaleFactors[group][sfb] = 0;
                break;
            case IntensityHcb: // intensity books
            case IntensityHcb2:
                ics.scaleFactors[group][sfb] = static_cast<std::uint16_t>(isPosition += static_cast<std::int16_t>(parseHuffmanScaleFactor() - 60));
                break;
            case NoiseHcb: // noise books
                if (noisePcmFlag) {
                    noisePcmFlag = 0;
                    tmp = m_reader.readBits<std::int16_t>(9);
                } else {
                    tmp = parseHuffmanScaleFactor() - 60;
                }
                ics.scaleFactors[group][sfb] = static_cast<std::uint16_t>(noiseEnergy += tmp);
                break;
            default: // spectral books
                scaleFactor += static_cast<std::int16_t>(parseHuffmanScaleFactor() - 60);
                if (scaleFactor < 0 || scaleFactor > 255) {
                    throw InvalidDataException();
                } else {
                    ics.scaleFactors[group][sfb] = 0;
                }
            }
        }
    }
}

/*!
 * \brief Decodes RVLC scale factor data.
 */
void AacFrameElementParser::decodeRvlcScaleFactorData(AacIcsInfo &ics)
{
    if (ics.rvlcSfLength) {
        m_reader.skipBits(ics.rvlcSfLength);
    }
    if (ics.sfEscapesPresent) {
        m_reader.skipBits(ics.rvlcEscapesLength);
    }
    // TODO: decode RVLC scale factors and escapes
}

/*!
 * \brief Parses scale factor data.
 */
void AacFrameElementParser::parseScaleFactorData(AacIcsInfo &ics)
{
    if (!m_aacScalefactorDataResilienceFlag) {
        decodeScaleFactorData(ics);
    } else {
        decodeRvlcScaleFactorData(ics);
    }
}

/*!
 * \brief Parses pulse data.
 */
void AacFrameElementParser::parsePulseData(AacIcsInfo &ics)
{
    AacPulseInfo &p = ics.pulse;
    p.count = m_reader.readBits<std::uint8_t>(2);
    p.startSfb = m_reader.readBits<std::uint8_t>(6);
    if (p.startSfb > ics.swbCount) {
        throw InvalidDataException();
    }
    for (std::uint8_t i = 0; i <= p.count; ++i) {
        p.offset[i] = m_reader.readBits<std::uint8_t>(5);
        p.amp[i] = m_reader.readBits<std::uint8_t>(4);
    }
}

/*!
 * \brief Parses TNS data.
 */
void AacFrameElementParser::parseTnsData(AacIcsInfo &ics)
{
    std::uint8_t filtBits, lengthBits, orderBits, startCoefBits, coefBits;
    if (ics.windowSequence == AacIcsSequenceTypes::EightShortSequence) {
        filtBits = 1;
        lengthBits = 4;
        orderBits = 3;
    } else {
        filtBits = 2;
        lengthBits = 6;
        orderBits = 5;
    }
    for (std::uint8_t window = 0; window < ics.windowCount; ++window) {
        if ((ics.tns.filt[window] = m_reader.readBits<std::uint8_t>(filtBits))) {
            startCoefBits = (ics.tns.coefRes[window] = m_reader.readBit()) ? 4 : 3;
        }
        for (std::uint8_t filt = 0; filt < ics.tns.filt[window]; ++filt) {
            ics.tns.length[window][filt] = m_reader.readBits<std::uint8_t>(lengthBits);
            if ((ics.tns.order[window][filt] = m_reader.readBits<std::uint8_t>(orderBits))) {
                ics.tns.direction[window][filt] = m_reader.readBit();
                ics.tns.coefCompress[window][filt] = m_reader.readBit();
                coefBits = startCoefBits - ics.tns.coefCompress[window][filt];
                for (std::uint8_t i = 0; i < ics.tns.order[window][filt]; ++i) {
                    ics.tns.coef[window][filt][i] = m_reader.readBits<std::uint8_t>(coefBits);
                }
            }
        }
    }
}

/*!
 * \brief Parses gain control data.
 */
void AacFrameElementParser::parseGainControlData(AacIcsInfo &ics)
{
    AacSsrInfo &ssr = ics.ssr;
    ssr.maxBand = m_reader.readBits<std::uint8_t>(2);
    switch (ics.windowSequence) {
        using namespace AacIcsSequenceTypes;
    case OnlyLongSequence:
        for (std::uint8_t bd = 1; bd <= ssr.maxBand; ++bd) {
            for (std::uint8_t wd = 0; wd < 1; ++wd) {
                ssr.adjustNum[bd][wd] = m_reader.readBits<std::uint8_t>(3);
                for (std::uint8_t ad = 0; ad < ssr.adjustNum[bd][wd]; ++ad) {
                    ssr.alevcode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(4);
                    ssr.aloccode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(5);
                }
            }
        }
        break;
    case LongStartSequence:
        for (std::uint8_t bd = 1; bd <= ssr.maxBand; ++bd) {
            for (std::uint8_t wd = 0; wd < 2; ++wd) {
                ssr.adjustNum[bd][wd] = m_reader.readBits<std::uint8_t>(3);
                for (std::uint8_t ad = 0; ad < ssr.adjustNum[bd][wd]; ++ad) {
                    ssr.alevcode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(4);
                    ssr.aloccode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(wd ? 2 : 4);
                }
            }
        }
        break;
    case EightShortSequence:
        for (std::uint8_t bd = 1; bd <= ssr.maxBand; ++bd) {
            for (std::uint8_t wd = 0; wd < 8; ++wd) {
                ssr.adjustNum[bd][wd] = m_reader.readBits<std::uint8_t>(3);
                for (std::uint8_t ad = 0; ad < ssr.adjustNum[bd][wd]; ++ad) {
                    ssr.alevcode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(4);
                    ssr.aloccode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(2);
                }
            }
        }
        break;
    case LongStopSequence:
        for (std::uint8_t bd = 1; bd <= ssr.maxBand; ++bd) {
            for (std::uint8_t wd = 0; wd < 2; ++wd) {
                ssr.adjustNum[bd][wd] = m_reader.readBits<std::uint8_t>(3);
                for (std::uint8_t ad = 0; ad < ssr.adjustNum[bd][wd]; ++ad) {
                    ssr.alevcode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(4);
                    ssr.aloccode[bd][wd][ad] = m_reader.readBits<std::uint8_t>(wd ? 5 : 4);
                }
            }
        }
        break;
    }
}

/*!
 * \brief Parses spectral data.
 */
void AacFrameElementParser::parseSpectralData(AacIcsInfo &ics, std::int16_t *specData)
{
    //byte groups = 0;
    //uint16 nshort = m_frameLength / 8;
    for (std::uint8_t group = 0; group < ics.windowGroupCount; ++group) {
        //byte p = groups * nshort;
        for (std::uint8_t section = 0; section < ics.sectionsPerGroup[group]; ++section) {
            using namespace AacScaleFactorTypes;
            std::uint8_t sectionCb = ics.sectionCb[group][section];
            std::uint16_t increment = (sectionCb >= FirstPairHcb) ? 2 : 4;
            switch (sectionCb) {
            case ZeroHcb:
            case NoiseHcb:
            case IntensityHcb:
            case IntensityHcb2:
                //p += (ics.sectionSfbOffset[group][ics.sectionEnd[group][section]] - ics.sectionSfbOffset[group][ics.sectionStart[group][section]]);
                break;
            default:
                for (std::uint16_t k = ics.sectionSfbOffset[group][ics.sectionStart[group][section]];
                     k < ics.sectionSfbOffset[group][ics.sectionEnd[group][section]]; k += increment) {
                    parseHuffmanSpectralData(sectionCb, specData);
                    //p += increment;
                }
            }
        }
        //groups += ics.windowGroupLengths[group];
    }
}

/*!
 * \brief Parses "side info".
 */
void AacFrameElementParser::parseSideInfo(AacIcsInfo &ics, bool scaleFlag)
{
    ics.globalGain = m_reader.readBits<std::uint8_t>(8);
    if (!m_commonWindow && !scaleFlag) {
        parseIcsInfo(ics);
    }
    parseSectionData(ics);
    parseScaleFactorData(ics);
    if (!scaleFlag) {
        if ((ics.pulseDataPresent = m_reader.readBit())) {
            parsePulseData(ics);
        }
        if ((ics.tnsDataPresent = m_reader.readBit())) {
            parseTnsData(ics);
        }
        if ((ics.gainControlPresent = m_reader.readBit())) {
            if (m_mpeg4AudioObjectId != Mpeg4AudioObjectIds::AacSsr) {
                throw InvalidDataException();
            } else {
                parseGainControlData(ics);
            }
        }
    }
    if (m_aacScalefactorDataResilienceFlag) {
        decodeRvlcScaleFactorData(ics);
    }
}

std::uint8_t AacFrameElementParser::parseExcludedChannels()
{
    for (std::uint8_t i = 0; i < 7; ++i) {
        m_drc.excludeMask[i] = m_reader.readBit();
    }
    std::uint8_t size = 0;
    for (; (m_drc.additionalExcludedChannels[size] = m_reader.readBit()); ++size) {
        for (std::uint8_t i = 0; i < 7; ++i) {
            m_drc.excludeMask[i] = m_reader.readBit();
        }
    }
    return size + 1;
}

std::uint8_t AacFrameElementParser::parseDynamicRange()
{
    std::uint8_t size = 1;
    m_drc.bandCount = 1;
    if (m_reader.readBit()) { // excluded channels present
        m_drc.pceInstanceTag = m_reader.readBits<std::uint8_t>(4);
        m_reader.skipBits(4); // skip reserved bits
        ++size;
    }
    if ((m_drc.excludedChannelsPresent = m_reader.readBit())) {
        size += parseExcludedChannels();
    }
    if (m_reader.readBit()) { // has bands data
        m_drc.bandCount += m_reader.readBits<std::uint8_t>(4);
        m_reader.skipBits(4); // skip reserved bits
        ++size;
        for (std::uint8_t i = 0; i < m_drc.bandCount; ++i, ++size) {
            m_drc.bandTop[i] = m_reader.readBits<std::uint8_t>(8);
        }
    }
    if (m_reader.readBit()) { // has prog ref level
        m_drc.progRefLevel = m_reader.readBits<std::uint8_t>(7);
        m_reader.skipBits(1); // skip reserved bit
        ++size;
    }
    for (std::uint8_t i = 0; i < m_drc.bandCount; ++i) {
        m_drc.dynamicRangeSign[i] = m_reader.readBit();
        m_drc.dynamicRangeControl[i] = m_reader.readBits<std::uint8_t>(7);
        ++size;
    }
    return size;
}

std::int16_t AacFrameElementParser::sbrHuffmanDec(SbrHuffTab table)
{
    std::uint8_t bit;
    std::int16_t index = 0;
    while (index >= 0) {
        bit = m_reader.readBit();
        index = table[index][bit];
    }
    return index + 64;
}

void AacFrameElementParser::parseSbrGrid(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    std::uint8_t tmp, bsEnvCount = 0;
    //byte bsRelCount0, bsRelCount1;
    switch ((sbr->bsFrameClass[channel] = m_reader.readBits<std::uint8_t>(2))) {
        using namespace BsFrameClasses;
    case FixFix:
        tmp = m_reader.readBits<std::uint8_t>(2);
        sbr->absBordLead[channel] = 0;
        sbr->absBordTrail[channel] = sbr->timeSlotsCount;
        sbr->relLeadCount[channel] = (bsEnvCount = min<std::uint8_t>(static_cast<std::uint8_t>(1 << tmp), 5)) - 1;
        sbr->relTrailCount[channel] = 0;
        tmp = m_reader.readBit();
        for (std::uint8_t env = 0; env < bsEnvCount; ++env) {
            sbr->f[channel][env] = tmp;
        }
        break;
    case FixVar:
        sbr->absBordLead[channel] = 0;
        sbr->absBordTrail[channel] = m_reader.readBits<std::uint8_t>(2) + sbr->timeSlotsCount;
        sbr->relLeadCount[channel] = 0;
        sbr->relTrailCount[channel] = bsEnvCount = m_reader.readBits<std::uint8_t>(2);
        for (std::uint8_t rel = 0; rel < bsEnvCount; ++rel) {
            sbr->bsRelBord[channel][rel] = static_cast<std::uint8_t>(2 * m_reader.readBits<std::uint8_t>(2) + 2);
        }
        sbr->bsPointer[channel] = m_reader.readBits<std::uint8_t>(static_cast<std::uint8_t>(sbrLog2(static_cast<std::int8_t>(bsEnvCount + 2))));
        for (std::uint8_t env = 0; env <= bsEnvCount; ++env) {
            sbr->f[channel][bsEnvCount - env] = m_reader.readBit();
        }
        break;
    case VarFix:
        sbr->absBordLead[channel] = m_reader.readBits<std::uint8_t>(2);
        sbr->absBordTrail[channel] = sbr->timeSlotsCount;
        sbr->relLeadCount[channel] = bsEnvCount = m_reader.readBits<std::uint8_t>(2);
        sbr->relTrailCount[channel] = 0;
        for (std::uint8_t rel = 0; rel < bsEnvCount; ++rel) {
            sbr->bsRelBord[channel][rel] = static_cast<std::uint8_t>(2 * m_reader.readBits<std::uint8_t>(2) + 2);
        }
        sbr->bsPointer[channel] = m_reader.readBits<std::uint8_t>(static_cast<std::uint8_t>(sbrLog2(static_cast<std::int8_t>(bsEnvCount + 2))));
        for (std::uint8_t env = 0; env < bsEnvCount; ++env) {
            sbr->f[channel][env] = m_reader.readBit();
        }
        break;
    case VarVar:
        sbr->absBordLead[channel] = m_reader.readBits<std::uint8_t>(2);
        sbr->absBordTrail[channel] = m_reader.readBits<std::uint8_t>(2) + sbr->timeSlotsCount;
        //bsRelCount0 = m_reader.readBits<std::uint8_t>(2);
        //bsRelCount1 = m_reader.readBits<std::uint8_t>(2);
        m_reader.skipBits(4);
        bsEnvCount = min<std::uint8_t>(5, static_cast<std::uint8_t>(sbr->bsRelCount0[channel] + sbr->bsRelCount1[channel] + 1));
        for (std::uint8_t rel = 0; rel < sbr->bsRelCount0[channel]; ++rel) {
            sbr->bsRelBord0[channel][rel] = static_cast<std::uint8_t>(2 * m_reader.readBits<std::uint8_t>(2) + 2);
        }
        for (std::uint8_t rel = 0; rel < sbr->bsRelCount1[channel]; ++rel) {
            sbr->bsRelBord1[channel][rel] = static_cast<std::uint8_t>(2 * m_reader.readBits<std::uint8_t>(2) + 2);
        }
        sbr->bsPointer[channel] = m_reader.readBits<std::uint8_t>(
            static_cast<std::uint8_t>(sbrLog2(static_cast<std::int8_t>(sbr->bsRelCount0[channel] + sbr->bsRelCount1[channel] + 2))));
        for (std::uint8_t env = 0; env < bsEnvCount; ++env) {
            sbr->f[channel][env] = m_reader.readBit();
        }
        sbr->relLeadCount[channel] = sbr->bsRelCount0[channel];
        sbr->relTrailCount[channel] = sbr->bsRelCount1[channel];
        break;
    default:;
    }
    if ((sbr->le[channel] = min<std::uint8_t>(bsEnvCount, sbr->bsFrameClass[channel] == BsFrameClasses::VarVar ? 5 : 4)) <= 0) {
        throw InvalidDataException();
    }
    sbr->lq[channel] = sbr->le[channel] > 1 ? 2 : 1;
    // TODO: envelope time border vector, noise floor time border vector
}

void AacFrameElementParser::parseSbrDtdf(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    for (std::uint8_t i = 0; i < sbr->le[channel]; ++i) {
        sbr->bsDfEnv[channel][i] = m_reader.readBit();
    }
    for (std::uint8_t i = 0; i < sbr->lq[channel]; ++i) {
        sbr->bsDfNoise[channel][i] = m_reader.readBit();
    }
}

void AacFrameElementParser::parseInvfMode(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    for (std::uint8_t i = 0; i < sbr->nq; ++i) {
        sbr->bsInvfMode[channel][i] = m_reader.readBits<std::uint8_t>(2);
    }
}

void AacFrameElementParser::parseSbrEnvelope(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    std::int8_t delta;
    //SbrHuffTab tHuff;
    SbrHuffTab fHuff;
    if ((sbr->le[channel] == 1) && (sbr->bsFrameClass[channel] == BsFrameClasses::FixFix)) {
        sbr->ampRes[channel] = 0;
    } else {
        sbr->ampRes[channel] = sbr->bsAmpRes;
    }
    if ((sbr->bsCoupling) && (channel == 1)) {
        delta = 1;
        if (sbr->ampRes[channel]) {
            //tHuff = tHuffmanEnvBal30dB;
            fHuff = fHuffmanEnvBal30dB;
        } else {
            //tHuff = tHuffmanEnvBal15dB;
            fHuff = fHuffmanEnvBal15dB;
        }
    } else {
        delta = 0;
        if (sbr->ampRes[channel]) {
            //tHuff = tHuffmanEnv30dB;
            fHuff = fHuffmanEnv30dB;
        } else {
            //tHuff = tHuffmanEnv15dB;
            fHuff = fHuffmanEnv15dB;
        }
    }
    for (std::uint8_t env = 0; env < sbr->le[channel]; ++env) {
        if (sbr->bsDfEnv[channel][env] == 0) {
            if ((sbr->bsCoupling == 1) && (channel == 1)) {
                if (sbr->ampRes[channel]) {
                    sbr->e[channel][0][env] = static_cast<std::int16_t>(m_reader.readBits<std::uint16_t>(5) << delta);
                } else {
                    sbr->e[channel][0][env] = static_cast<std::int16_t>(m_reader.readBits<std::uint16_t>(6) << delta);
                }
            } else {
                if (sbr->ampRes[channel]) {
                    sbr->e[channel][0][env] = static_cast<std::int16_t>(m_reader.readBits<std::uint16_t>(6) << delta);
                } else {
                    sbr->e[channel][0][env] = static_cast<std::int16_t>(m_reader.readBits<std::uint16_t>(7) << delta);
                }
            }
            for (std::uint8_t band = 1; band < sbr->n[sbr->f[channel][env]]; ++band) {
                sbr->e[channel][band][env] = static_cast<std::int16_t>(sbrHuffmanDec(fHuff) << delta);
            }
        } else {
            for (std::uint8_t band = 0; band < sbr->n[sbr->f[channel][env]]; ++band) {
                sbr->e[channel][band][env] = static_cast<std::int16_t>(sbrHuffmanDec(fHuff) << delta);
            }
        }
    }
    // TODO: extract envelope data
}

void AacFrameElementParser::parseSbrNoise(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    std::int8_t delta;
    //SbrHuffTab tHuff;
    SbrHuffTab fHuff;
    if ((sbr->bsCoupling == 1) && (channel == 1)) {
        delta = 1;
        //tHuff = tHuffmanNoiseBal30dB;
        fHuff = fHuffmanEnvBal30dB;
    } else {
        delta = 1;
        //tHuff = tHuffmanNoise30dB;
        fHuff = fHuffmanEnv30dB;
    }
    for (std::uint8_t noise = 0; noise < sbr->lq[channel]; ++noise) {
        if (sbr->bsDfNoise[channel][noise] == 0) {
            if ((sbr->bsCoupling == 1) && (channel == 1)) {
                sbr->q[channel][0][noise] = m_reader.readBits<std::uint8_t>(5) << delta;
            } else {
                sbr->q[channel][0][noise] = m_reader.readBits<std::uint8_t>(5) << delta;
            }
            for (std::uint8_t band = 1; band < sbr->nq; ++band) {
                sbr->q[channel][band][noise] = sbrHuffmanDec(fHuff) << delta;
            }
        } else {
            for (std::uint8_t band = 0; band < sbr->nq; ++band) {
                sbr->q[channel][band][noise] = sbrHuffmanDec(fHuff) << delta;
            }
        }
    }
    // TODO: extract noise floor data
}

void AacFrameElementParser::parseSbrSinusoidalCoding(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel)
{
    for (std::uint8_t i = 0; i < sbr->nHigh; ++i) {
        sbr->bsAddHarmonic[channel][i] = m_reader.readBit();
    }
}

std::uint16_t AacFrameElementParser::parseSbrExtension(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t extensionId, std::uint8_t)
{
    std::uint8_t header;
    std::uint16_t res;
    switch (extensionId) {
        using namespace AacSbrExtensionIds;
    case Ps:
        if (sbr->psResetFlag) {
            sbr->ps->headerRead = 0;
        }
        res = parsePsData(sbr->ps, header);
        if (sbr->psUsed == 0 && header == 1) {
            sbr->psUsed = 1;
        }
        if (header == 1) {
            sbr->psResetFlag = 0;
        }
        return res;
    case DrmParametricStereo:
        sbr->psUsed = 1;
        return parseDrmPsData(sbr->drmPs);
    default:
        sbr->bsExtendedData = m_reader.readBits<std::uint8_t>(6);
        return 6;
    }
}

std::uint16_t AacFrameElementParser::parsePsData(std::shared_ptr<AacPsInfo> &ps, std::uint8_t &header)
{
    if (m_reader.readBit()) {
        header = 1;
        ps->headerRead = 1;
        ps->use34HybridBands = 0;
        if ((ps->enableIID = m_reader.readBit())) {
            ps->iidMode = m_reader.readBits<std::uint8_t>(3);
        }
    }
    throw NotImplementedException(); // TODO
}

std::uint16_t AacFrameElementParser::parseDrmPsData(std::shared_ptr<AacDrmPsInfo> &drmPs)
{
    CPP_UTILITIES_UNUSED(drmPs)
    throw NotImplementedException(); // TODO
}

void AacFrameElementParser::parseSbrSingleChannelElement(std::shared_ptr<AacSbrInfo> &sbr)
{
    if (m_reader.readBit()) { // bs data extra
        m_reader.skipBits(4); // skip bs reserved
    }
    if (sbr->isDrmSbr) {
        m_reader.skipBits(1); // bs coupling
    }
    parseSbrGrid(sbr, 0);
    parseSbrDtdf(sbr, 0);
    parseInvfMode(sbr, 0);
    parseSbrEnvelope(sbr, 0);
    parseSbrNoise(sbr, 0);
    // TODO: envelope noise dequantisation
    if ((sbr->bsAddHarmonicFlag[0] = m_reader.readBit())) {
        parseSbrSinusoidalCoding(sbr, 0);
    }
    if ((sbr->bsExtendedData = m_reader.readBit())) {
        std::uint16_t cnt = m_reader.readBits<std::uint16_t>(4);
        if (cnt == 0xF) {
            cnt += m_reader.readBits<std::uint16_t>(8);
        }
        std::uint16_t bitsLeft = 8 * cnt;
        while (bitsLeft > 7) {
            sbr->bsExtensionId = m_reader.readBits<std::uint8_t>(2);
            std::uint16_t tmpBitCount = 2 + parseSbrExtension(sbr, sbr->bsExtensionId, static_cast<std::uint8_t>(bitsLeft));
            if (tmpBitCount > bitsLeft) {
                throw InvalidDataException();
            } else {
                bitsLeft -= tmpBitCount;
            }
        }
        if (bitsLeft) { // read remaining bits
            m_reader.skipBits(bitsLeft);
        }
    }
}

void AacFrameElementParser::parseSbrChannelPairElement(std::shared_ptr<AacSbrInfo> &sbr)
{
    if (m_reader.readBit()) { // bs data extra
        m_reader.skipBits(8); // skip bs reserved
    }
    if ((sbr->bsCoupling = m_reader.readBit())) {
        parseSbrGrid(sbr, 0);
        // copy data from left to right
        sbr->bsFrameClass[1] = sbr->bsFrameClass[0];
        sbr->le[1] = sbr->le[0];
        sbr->lq[1] = sbr->lq[0];
        sbr->bsPointer[1] = sbr->bsPointer[0];
        for (std::uint8_t n = 0; n < sbr->le[0]; ++n) {
            sbr->te[1][n] = sbr->te[0][n];
            sbr->f[1][n] = sbr->f[0][n];
        }
        for (std::uint8_t n = 0; n < sbr->lq[0]; ++n) {
            sbr->tq[1][n] = sbr->tq[0][n];
        }
        parseSbrDtdf(sbr, 0);
        parseSbrDtdf(sbr, 1);
        parseInvfMode(sbr, 0);
        for (std::uint8_t n = 0; n < sbr->nq; ++n) {
            sbr->bsInvfMode[1][n] = sbr->bsInvfMode[0][n];
        }
        parseSbrEnvelope(sbr, 0);
        parseSbrNoise(sbr, 0);
        parseSbrEnvelope(sbr, 1);
        parseSbrNoise(sbr, 1);
    } else {
        parseSbrGrid(sbr, 0);
        parseSbrGrid(sbr, 1);
        parseSbrDtdf(sbr, 0);
        parseSbrDtdf(sbr, 1);
        parseInvfMode(sbr, 0);
        parseInvfMode(sbr, 1);
        parseSbrEnvelope(sbr, 0);
        parseSbrEnvelope(sbr, 1);
        parseSbrNoise(sbr, 0);
        parseSbrNoise(sbr, 1);
    }
    if ((sbr->bsAddHarmonicFlag[0] = m_reader.readBit())) {
        parseSbrSinusoidalCoding(sbr, 0);
    }
    if ((sbr->bsAddHarmonicFlag[1] = m_reader.readBit())) {
        parseSbrSinusoidalCoding(sbr, 1);
    }
    // TODO: envelope noise dequantisation (for both channels)
    if (sbr->bsCoupling) {
        // TODO: unmap envelope noise
    }
    if ((sbr->bsExtendedData = m_reader.readBit())) {
        std::uint16_t cnt = m_reader.readBits<std::uint16_t>(4);
        if (cnt == 0xF) {
            cnt += m_reader.readBits<std::uint16_t>(8);
        }
        std::uint16_t bitsLeft = 8 * cnt;
        while (bitsLeft > 7) {
            sbr->bsExtensionId = m_reader.readBits<std::uint8_t>(2);
            std::uint16_t tmpBitCount = 2 + parseSbrExtension(sbr, sbr->bsExtensionId, static_cast<std::uint8_t>(bitsLeft));
            if (tmpBitCount > bitsLeft) {
                throw InvalidDataException();
            } else {
                bitsLeft -= tmpBitCount;
            }
        }
        if (bitsLeft) { // read remaining bits
            m_reader.skipBits(bitsLeft);
        }
    }
}

shared_ptr<AacSbrInfo> AacFrameElementParser::makeSbrInfo(std::uint8_t sbrElement, bool isDrm)
{
    if (m_mpeg4ExtensionSamplingFrequencyIndex >= sizeof(mpeg4SamplingFrequencyTable)
        && m_mpeg4SamplingFrequencyIndex >= sizeof(mpeg4SamplingFrequencyTable)) {
        throw InvalidDataException(); // sampling frequency index is invalid
    }
    return make_shared<AacSbrInfo>(m_elementId[sbrElement],
        m_mpeg4ExtensionSamplingFrequencyIndex < sizeof(mpeg4SamplingFrequencyTable)
            ? mpeg4SamplingFrequencyTable[m_mpeg4ExtensionSamplingFrequencyIndex]
            : mpeg4SamplingFrequencyTable[m_mpeg4SamplingFrequencyIndex] * 2,
        m_frameLength, isDrm);
}

void AacFrameElementParser::parseSbrExtensionData(std::uint8_t sbrElement, std::uint16_t count, bool crcFlag)
{
    CPP_UTILITIES_UNUSED(count);
    //uint16 alignBitCount = 0;
    std::shared_ptr<AacSbrInfo> &sbr = m_sbrElements[sbrElement];
    if (m_psResetFlag) {
        sbr->psResetFlag = m_psResetFlag;
    }
    if (!sbr->isDrmSbr) {
        if (crcFlag) {
            sbr->bsSbrCrcBits = m_reader.readBits<std::uint16_t>(10);
        }
    }
    //auto startFrequ = sbr->bsStartFreq;
    //auto samplerateMode = sbr->bsSamplerateMode;
    //auto stopFrequ = sbr->bsStopFreq;
    //auto frequScale = sbr->bsFreqScale;
    //auto alterScale = sbr->bsAlterScale;
    //auto xoverBand = sbr->bsXoverBand;
    if ((sbr->bsHeaderFlag = m_reader.readBit())) {
        sbr->bsStartFreq = m_reader.readBits<std::uint8_t>(4);
        sbr->bsStopFreq = m_reader.readBits<std::uint8_t>(4);
        sbr->bsXoverBand = m_reader.readBits<std::uint8_t>(3);
        m_reader.skipBits(2);
        std::uint8_t bsExtraHeader1 = m_reader.readBit();
        std::uint8_t bsExtraHeader2 = m_reader.readBit();
        if (bsExtraHeader1) {
            sbr->bsFreqScale = m_reader.readBits<std::uint8_t>(2);
            sbr->bsAlterScale = m_reader.readBit();
            sbr->bsNoiseBands = m_reader.readBits<std::uint8_t>(2);
        } else {
            sbr->bsFreqScale = 2;
            sbr->bsAlterScale = 1;
            sbr->bsNoiseBands = 2;
        }
        if (bsExtraHeader2) {
            sbr->bsLimiterBands = m_reader.readBits<std::uint8_t>(2);
            sbr->bsLimiterGains = m_reader.readBits<std::uint8_t>(2);
            sbr->bsInterpolFreq = m_reader.readBit();
            sbr->bsSmoothingMode = m_reader.readBit();
        } else {
            sbr->bsLimiterBands = 2;
            sbr->bsLimiterGains = 2;
            sbr->bsInterpolFreq = 1;
            sbr->bsSmoothingMode = 1;
        }
    }
    if (sbr->headerCount) {
        if (sbr->reset || (sbr->bsHeaderFlag && sbr->justSeeked)) {
            // TODO: calc SBR tables; restore old values on error
        }
        sbr->rate = sbr->bsSamplerateMode ? 2 : 1;
        switch (sbr->aacElementId) {
            using namespace AacSyntaxElementTypes;
        case SingleChannelElement:
            parseSbrSingleChannelElement(sbr);
            break;
        case ChannelPairElement:
            parseSbrChannelPairElement(sbr);
            break;
        }
    }
}

std::uint8_t AacFrameElementParser::parseHuffmanScaleFactor()
{
    std::uint16_t offset = 0;
    while (aacHcbSf[offset][1]) {
        offset += aacHcbSf[offset][m_reader.readBit()];
        if (offset > 240) {
            throw InvalidDataException();
        }
    }
    return aacHcbSf[offset][0];
}

void AacFrameElementParser::parseHuffmanSpectralData(std::uint8_t cb, std::int16_t *sp)
{
    switch (cb) {
    case 1:
    case 2: // 2-step method for data quadruples
        huffman2StepQuad(cb, sp);
        break;
    case 3: // binary search for data quadruples
        huffmanBinaryQuadSign(cb, sp);
        break;
    case 4: // binary search for data paris
        huffmanBinaryPair(cb, sp);
        break;
    case 5: // 2-step method for data pairs
        huffman2StepPair(cb, sp);
        break;
    case 6: // 2-step method for data pairs
        huffman2StepPair(cb, sp);
        break;
    case 7:
    case 9: // binary search for data pairs
        huffmanBinaryPairSign(cb, sp);
        break;
    case 8:
    case 10: // 2-step method for data pairs
        huffman2StepPairSign(cb, sp);
        break;
    case 11:
        try {
            huffman2StepPairSign(11, sp);
        } catch (const InvalidDataException &) {
            sp[0] = huffmanGetEscape(sp[0]);
            sp[1] = huffmanGetEscape(sp[1]);
            throw;
        }
        sp[0] = huffmanGetEscape(sp[0]);
        sp[1] = huffmanGetEscape(sp[1]);
        break;
    case 12:
        try {
            huffman2StepPair(11, sp);
        } catch (const InvalidDataException &) {
            sp[0] = huffmanCodebook(0);
            sp[1] = huffmanCodebook(1);
            throw;
        }
        sp[0] = huffmanCodebook(0);
        sp[1] = huffmanCodebook(1);
        break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
        try {
            huffman2StepPairSign(11, sp);
        } catch (const InvalidDataException &) {
            sp[0] = huffmanGetEscape(sp[0]);
            sp[1] = huffmanGetEscape(sp[1]);
            vcb11CheckLav(cb, sp);
            throw;
        }
        sp[0] = huffmanGetEscape(sp[0]);
        sp[1] = huffmanGetEscape(sp[1]);
        vcb11CheckLav(cb, sp);
        break;
    default:
        throw InvalidDataException(); // non-existent codebook number
    }
}

void AacFrameElementParser::huffmanSignBits(std::int16_t *sp, std::uint8_t len)
{
    for (std::int16_t *end = sp + len; sp < end; ++sp) {
        if (*sp) {
            if (m_reader.readBit()) {
                *sp = -(*sp);
            }
        }
    }
}

void AacFrameElementParser::huffman2StepQuad(std::uint8_t cb, std::int16_t *sp)
{
    std::uint32_t cw = m_reader.showBits<std::uint32_t>(aacHcbN[cb]);
    std::uint16_t offset = aacHcbTable[cb][cw].offset;
    uint8_t extraBits = aacHcbTable[cb][cw].extraBits;
    if (extraBits) {
        m_reader.skipBits(aacHcbN[cb]);
        offset += m_reader.showBits<std::uint16_t>(extraBits);
        m_reader.skipBits(aacHcb2QuadTable[cb][offset].bits - aacHcbN[cb]);
    } else {
        m_reader.skipBits(aacHcb2QuadTable[cb][offset].bits);
    }
    if (offset > aacHcb2QuadTableSize[cb]) {
        throw InvalidDataException();
    }
    sp[0] = aacHcb2QuadTable[cb][offset].x;
    sp[1] = aacHcb2QuadTable[cb][offset].y;
    sp[2] = aacHcb2QuadTable[cb][offset].v;
    sp[3] = aacHcb2QuadTable[cb][offset].w;
}

void AacFrameElementParser::huffmanBinaryQuadSign(std::uint8_t cb, std::int16_t *sp)
{
    try {
        huffman2StepQuad(cb, sp);
    } catch (const InvalidDataException &) {
        huffmanSignBits(sp, 4);
        throw;
    }
    huffmanSignBits(sp, 4);
}

void AacFrameElementParser::huffmanBinaryPair(std::uint8_t cb, std::int16_t *sp)
{
    std::uint16_t offset = 0;
    while (!aacHcbBinTable[cb][offset].isLeaf) {
        offset += static_cast<std::uint16_t>(aacHcbBinTable[cb][offset].data[m_reader.readBit()]);
    }
    if (offset > aacHcbBinTableSize[cb]) {
        throw InvalidDataException();
    }
    sp[0] = aacHcbBinTable[cb][offset].data[0];
    sp[1] = aacHcbBinTable[cb][offset].data[1];
}

void AacFrameElementParser::huffman2StepPair(std::uint8_t cb, std::int16_t *sp)
{
    std::uint32_t cw = m_reader.showBits<std::uint32_t>(aacHcbN[cb]);
    std::uint16_t offset = aacHcbTable[cb][cw].offset;
    uint8_t extraBits = aacHcbTable[cb][cw].extraBits;
    if (extraBits) {
        m_reader.skipBits(aacHcbN[cb]);
        offset += m_reader.showBits<std::uint16_t>(extraBits);
        m_reader.skipBits(aacHcb2PairTable[cb][offset].bits - aacHcbN[cb]);
    } else {
        m_reader.skipBits(aacHcb2PairTable[cb][offset].bits);
    }
    if (offset > aacHcb2PairTableSize[cb]) {
        throw InvalidDataException();
    }
    sp[0] = aacHcb2PairTable[cb][offset].x;
    sp[1] = aacHcb2PairTable[cb][offset].y;
}

void AacFrameElementParser::huffmanBinaryPairSign(std::uint8_t cb, std::int16_t *sp)
{
    try {
        huffmanBinaryPair(cb, sp);
    } catch (const InvalidDataException &) {
        huffmanSignBits(sp, 2);
        throw;
    }
    huffmanSignBits(sp, 2);
}

void AacFrameElementParser::huffman2StepPairSign(std::uint8_t cb, std::int16_t *sp)
{
    try {
        huffman2StepPair(cb, sp);
    } catch (const InvalidDataException &) {
        huffmanSignBits(sp, 2);
        throw;
    }
    huffmanSignBits(sp, 2);
}

std::int16_t AacFrameElementParser::huffmanGetEscape(std::int16_t sp)
{
    std::uint8_t neg;
    if (sp < 0) {
        if (sp != -16)
            return sp;
        neg = 1;
    } else {
        if (sp != 16)
            return sp;
        neg = 0;
    }
    std::uint8_t size;
    for (size = 4; m_reader.readBit(); ++size) {
    }
    const std::int16_t off = m_reader.readBits<std::int16_t>(size);
    return static_cast<std::int16_t>(neg ? -(off | (1 << size)) : (off | (1 << size)));
}

void AacFrameElementParser::vcb11CheckLav(std::uint8_t cb, std::int16_t *sp)
{
    static const uint16_t vcb11LavTab[] = { 16, 31, 47, 63, 95, 127, 159, 191, 223, 255, 319, 383, 511, 767, 1023, 2047 };
    std::uint16_t max = 0;
    if (cb < 16 || cb > 31)
        return;
    max = vcb11LavTab[cb - 16];
    if ((abs(sp[0]) > max) || (abs(sp[1]) > max)) {
        sp[0] = 0;
        sp[1] = 0;
    }
}

/*!
 * \brief Calculates "window grouping info".
 */
void AacFrameElementParser::calculateWindowGroupingInfo(AacIcsInfo &ics)
{
    using namespace AacIcsSequenceTypes;
    switch (ics.windowSequence) {
    case OnlyLongSequence:
    case LongStartSequence:
    case LongStopSequence:
        ics.windowCount = ics.windowGroupCount = ics.windowGroupLengths[0] = 1;
        if (m_mpeg4AudioObjectId == Mpeg4AudioObjectIds::ErAacLd) {
            if (m_frameLength == 512) {
                ics.swbCount = swb512WindowCount[m_mpeg4SamplingFrequencyIndex];
            } else { // if 480
                ics.swbCount = swb480WindowCount[m_mpeg4SamplingFrequencyIndex];
            }
        } else {
            if (m_frameLength == 1024) {
                ics.swbCount = swb1024WindowCount[m_mpeg4SamplingFrequencyIndex];
            } else {
                ics.swbCount = swb960WindowCount[m_mpeg4SamplingFrequencyIndex];
            }
        }
        if (ics.maxSfb > ics.swbCount) {
            throw InvalidDataException();
        }
        if (m_mpeg4AudioObjectId == Mpeg4AudioObjectIds::ErAacLd) {
            if (m_frameLength == 512) {
                for (std::uint8_t i = 0; i <= ics.swbCount; ++i) {
                    ics.sectionSfbOffset[0][i] = swbOffset512Window[m_mpeg4SamplingFrequencyIndex][i];
                    ics.swbOffset[i] = swbOffset512Window[m_mpeg4SamplingFrequencyIndex][i];
                }
            } else {
                for (std::uint8_t i = 0; i <= ics.swbCount; ++i) {
                    ics.sectionSfbOffset[0][i] = swbOffset480Window[m_mpeg4SamplingFrequencyIndex][i];
                    ics.swbOffset[i] = swbOffset480Window[m_mpeg4SamplingFrequencyIndex][i];
                }
            }
        } else {
            for (std::uint8_t i = 0; i <= ics.swbCount; ++i) {
                ics.sectionSfbOffset[0][i] = swbOffset1024Window[m_mpeg4SamplingFrequencyIndex][i];
                ics.swbOffset[i] = swbOffset1024Window[m_mpeg4SamplingFrequencyIndex][i];
            }
        }
        ics.sectionSfbOffset[0][ics.swbCount] = ics.swbOffset[ics.swbCount] = ics.maxSwbOffset = m_frameLength;
        break;
    case EightShortSequence:
        ics.windowCount = 8;
        ics.windowGroupCount = ics.windowGroupLengths[0] = 1;
        ics.swbCount = swb128WindowCount[m_mpeg4SamplingFrequencyIndex];
        if (ics.maxSfb > ics.swbCount) {
            throw InvalidDataException();
        }
        for (std::uint8_t i = 0; i < ics.swbCount; ++i) {
            ics.swbOffset[i] = swbOffset128Window[m_mpeg4SamplingFrequencyIndex][i];
        }
        ics.swbOffset[ics.swbCount] = ics.maxSwbOffset = m_frameLength / 8;
        for (std::uint8_t i = 0; i < ics.windowCount - 1; ++i) {
            if (!(ics.scaleFactorGrouping & (1 << (6 - i)))) {
                ics.windowGroupLengths[ics.windowGroupCount] = 1;
                ++ics.windowGroupCount;
            } else {
                ++(ics.windowGroupLengths[ics.windowGroupCount - 1]);
            }
        }
        for (std::uint8_t g = 0; g < ics.windowGroupCount; ++g) {
            std::uint8_t sectionSfb = 0;
            std::uint16_t offset = 0, width;
            for (std::uint8_t i = 0; i < ics.swbCount; ++i) {
                if (i + 1 == ics.swbCount) {
                    width = (m_frameLength / 8) - swbOffset128Window[m_mpeg4SamplingFrequencyIndex][i];
                } else {
                    width = swbOffset128Window[m_mpeg4SamplingFrequencyIndex][i + 1] - swbOffset128Window[m_mpeg4SamplingFrequencyIndex][i];
                }
                width *= ics.windowGroupLengths[g];
                ics.sectionSfbOffset[g][sectionSfb++] = offset;
                offset += width;
            }
            ics.sectionSfbOffset[g][sectionSfb] = offset;
        }
        break;
    default:
        throw InvalidDataException();
    }
}

/*!
 * \brief Parses "individual channel stream" (basic audio unit).
 */
void AacFrameElementParser::parseIndividualChannelStream(AacIcsInfo &ics, std::int16_t *specData, bool scaleFlag)
{
    parseSideInfo(ics, scaleFlag);
    if (m_mpeg4AudioObjectId >= Mpeg4AudioObjectIds::ErAacLc) {
        if (ics.tnsDataPresent) {
            parseTnsData(ics);
        }
    }
    if (m_mpeg4AudioObjectId == Mpeg4AudioObjectIds::ErParametric) { // DRM stuff?
        // TODO: check CRC
        throw NotImplementedException();
    }
    if (m_aacSpectralDataResilienceFlag) {
        // TODO: parseReorderedSpectralData(ic);
        throw NotImplementedException();
    } else {
        parseSpectralData(ics, specData);
    }
    if (ics.pulseDataPresent) {
        if (ics.windowSequence == AacIcsSequenceTypes::EightShortSequence) {
            throw InvalidDataException(); // pulse coding not allowed for short blocks
        } else {
            // TODO: reconstruct pulse coding
            //decodePulseData(ic);
        }
    }
}

/*!
 * \brief Parses "single channel element".
 */
void AacFrameElementParser::parseSingleChannelElement()
{
    if (m_elementCount + 1 > aacMaxSyntaxElements) {
        throw NotImplementedException(); // can not parse frame with more than aacMaxSyntaxElements syntax elements
    }
    // TODO: check whether limit of channels is exceeded

    std::int16_t specData[1024] = { 0 };
    m_elementId[m_elementCount] = AacSyntaxElementTypes::SingleChannelElement;
    m_elementInstanceTag[m_elementCount] = m_reader.readBits<std::uint8_t>(4);
    //m_channel = channel;
    //m_pairedChannel = -1;
    parseIndividualChannelStream(m_ics1, specData);
    if (m_ics1.isUsed) {
        throw InvalidDataException(); // IS not allowed in single channel
    }
    // check whether next bitstream element is a fill element (for SBR decoding)
    if (m_reader.showBits<std::uint8_t>(3) == AacSyntaxElementTypes::FillElement) {
        parseFillElement(m_elementCount);
    }
    // TODO: reconstruct single channel element
    // TODO: map output channels position to internal data channels
    m_channelCount += m_elementChannelCount[m_elementCount];
    ++m_elementCount;
}

/*!
 * \brief Parses "channel pair element".
 */
void AacFrameElementParser::parseChannelPairElement()
{
    if (m_elementCount + 2 > aacMaxSyntaxElements) {
        throw NotImplementedException(); // can not parse frame with more than aacMaxSyntaxElements syntax elements
    }
    // TODO: check whether limit of channels is exceeded
    m_elementId[m_elementCount] = AacSyntaxElementTypes::ChannelPairElement;
    m_elementChannelCount[m_elementCount] = 2; // number of output channels in CPE is always 2

    std::int16_t specData1[1024] = { 0 };
    std::int16_t specData2[1024] = { 0 };
    //m_channel = channels;
    //m_pairedChannel = channels + 1;
    m_elementInstanceTag[m_elementCount] = m_reader.readBits<std::uint8_t>(4);
    if ((m_commonWindow = m_reader.readBit())) {
        // both channels have common ics data
        parseIcsInfo(m_ics1);
        if ((m_ics1.midSideCodingMaskPresent = m_reader.readBits<std::uint8_t>(2) == 1)) { // ms mask present
            for (std::uint8_t g = 0; g < m_ics1.windowGroupCount; ++g) {
                for (std::uint8_t sfb = 0; sfb < m_ics1.maxSfb; ++sfb) {
                    m_ics1.midSideCodingUsed[g][sfb] = m_reader.readBit();
                }
            }
            //m_reader.skipBits(m_ics1.windowGroupCount * m_ics1.maxSfb);
        }
        if (m_mpeg4AudioObjectId >= Mpeg4AudioObjectIds::ErAacLc && m_ics1.predictorDataPresent) {
            if ((m_ics1.ltp1.dataPresent = m_reader.readBit())) {
                parseLtpInfo(m_ics1, m_ics1.ltp1);
            }
        }
        m_ics2 = m_ics1;
    } else {
        m_ics1.midSideCodingMaskPresent = false;
    }
    parseIndividualChannelStream(m_ics1, specData1);
    if (m_commonWindow && m_mpeg4AudioObjectId >= Mpeg4AudioObjectIds::ErAacLc && m_ics1.predictorDataPresent) {
        if ((m_ics1.ltp2.dataPresent = m_reader.readBit())) {
            parseLtpInfo(m_ics1, m_ics1.ltp2);
        }
    }
    parseIndividualChannelStream(m_ics2, specData2);
    // check if next bitstream element is a fill element (for SBR decoding)
    if (m_reader.showBits<std::uint8_t>(3) == AacSyntaxElementTypes::FillElement) {
        parseFillElement(m_elementCount);
    }
    // TODO: reconstruct channel pair
    // TODO: map output channels position to internal data channels
    m_channelCount += 2;
    ++m_elementCount;
}

/*!
 * \brief Parses/skips "channel coupling element".
 */
void AacFrameElementParser::parseCouplingChannelElement()
{
    m_reader.skipBits(4); // element instance tag
    std::uint8_t swCceFlag = m_reader.readBit();
    std::uint8_t coupledElementCount = m_reader.readBits<std::uint8_t>(3);
    std::uint8_t gainElementLists = 0;
    for (std::uint8_t c = 0; c < coupledElementCount; ++c) {
        ++gainElementLists;
        std::uint8_t ccTargetIsCpe = m_reader.readBit();
        //byte ccTargetTagSelect = m_reader.readBits<byte>(4);
        m_reader.skipBits(4); // cc target tag select
        if (ccTargetIsCpe) {
            // cc left and right
            if (m_reader.readBit() & m_reader.readBit()) {
                ++gainElementLists;
            }
        }
    }
    m_reader.skipBits(4); // 1 bit cc domain, 1 bit gain element sign, 2 bits gain element scale
    AacIcsInfo ics;
    std::int16_t specData[1024];
    parseIndividualChannelStream(ics, specData);
    for (std::uint8_t c = 1; c < gainElementLists; ++c) {
        if (swCceFlag || m_reader.readBit()) {
            parseHuffmanScaleFactor();
        } else {
            for (std::uint8_t group = 0; group < ics.windowCount; ++group) {
                for (std::uint8_t sfb = 0; sfb < ics.maxSfb; ++sfb) {
                    if (ics.sfbCb[group][sfb] != AacScaleFactorTypes::ZeroHcb) {
                        parseHuffmanScaleFactor();
                    }
                }
            }
        }
    }
}

/*!
 * \brief Parses "low frequency element".
 */
void AacFrameElementParser::parseLowFrequencyElement()
{
    parseSingleChannelElement();
}

/*!
 * \brief Parses/skips "data stream element".
 */
void AacFrameElementParser::parseDataStreamElement()
{
    std::uint8_t byteAligned = m_reader.readBit();
    std::uint16_t count = m_reader.readBits<std::uint16_t>(8);
    if (count == 0xFF) {
        count += m_reader.readBits<std::uint16_t>(8);
    }
    if (byteAligned) {
        m_reader.align();
    }
    m_reader.skipBits(count * 8);
}

/*!
 * \brief Parses "program config element".
 */
void AacFrameElementParser::parseProgramConfigElement()
{
    m_pce.elementInstanceTag = m_reader.readBits<std::uint8_t>(4);
    m_pce.objectType = m_reader.readBits<std::uint8_t>(2);
    m_pce.samplingFrequencyIndex = m_reader.readBits<std::uint8_t>(4);
    m_pce.frontChannelElementCount = m_reader.readBits<std::uint8_t>(4);
    m_pce.sideChannelElementCount = m_reader.readBits<std::uint8_t>(4);
    m_pce.backChannelElementCount = m_reader.readBits<std::uint8_t>(4);
    m_pce.lfeChannelElementCount = m_reader.readBits<std::uint8_t>(2);
    m_pce.assocDataElementCount = m_reader.readBits<std::uint8_t>(3);
    m_pce.validCcElementCount = m_reader.readBits<std::uint8_t>(4);
    if ((m_pce.monoMixdownPresent = m_reader.readBit())) {
        m_pce.monoMixdownElementNumber = m_reader.readBits<std::uint8_t>(4);
    }
    if ((m_pce.stereoMixdownPresent = m_reader.readBit())) {
        m_pce.stereoMixdownElementNumber = m_reader.readBits<std::uint8_t>(4);
    }
    if ((m_pce.matrixMixdownIdxPresent = m_reader.readBit())) {
        m_pce.matrixMixdownIdx = m_reader.readBits<std::uint8_t>(2);
        m_pce.pseudoSurroundEnable = m_reader.readBit();
    }
    std::uint8_t i;
    for (i = 0; i < m_pce.frontChannelElementCount; ++i) {
        m_pce.frontElementIsCpe[i] = m_reader.readBit();
        m_pce.frontElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
        if (m_pce.frontElementIsCpe[i]) { // channel coupling element
            m_pce.cpeChannel[m_pce.frontElementTagSelect[i]] = m_pce.channels;
            m_pce.frontChannelCount += 2;
            m_pce.channels += 2;
        } else { // single channel element
            m_pce.sceChannel[m_pce.frontElementTagSelect[i]] = m_pce.channels;
            ++m_pce.frontChannelCount;
            ++m_pce.channels;
        }
    }
    for (i = 0; i < m_pce.sideChannelElementCount; ++i) {
        m_pce.sideElementIsCpe[i] = m_reader.readBit();
        m_pce.sideElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
        if (m_pce.sideElementIsCpe[i]) { // channel coupling element
            m_pce.cpeChannel[m_pce.sideElementTagSelect[i]] = m_pce.channels;
            m_pce.sideChannelCount += 2;
            m_pce.channels += 2;
        } else { // single channel element
            m_pce.sceChannel[m_pce.sideElementTagSelect[i]] = m_pce.channels;
            ++m_pce.sideChannelCount;
            ++m_pce.channels;
        }
    }
    for (i = 0; i < m_pce.backChannelElementCount; ++i) {
        m_pce.backElementIsCpe[i] = m_reader.readBit();
        m_pce.backElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
        if (m_pce.backElementIsCpe[i]) { // channel coupling element
            m_pce.cpeChannel[m_pce.backElementTagSelect[i]] = m_pce.channels;
            m_pce.backChannelCount += 2;
            m_pce.channels += 2;
        } else { // single channel element
            m_pce.sceChannel[m_pce.backElementTagSelect[i]] = m_pce.channels;
            ++m_pce.backChannelCount;
            ++m_pce.channels;
        }
    }
    for (i = 0; i < m_pce.lfeChannelElementCount; ++i) {
        m_pce.lfeElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
        m_pce.sceChannel[m_pce.lfeElementTagSelect[i]] = m_pce.channels;
        ++m_pce.lfeChannelCount;
        ++m_pce.channels;
    }
    for (i = 0; i < m_pce.assocDataElementCount; ++i) {
        m_pce.assocDataElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
    }
    for (i = 0; i < m_pce.validCcElementCount; ++i) {
        m_pce.ccElementIsIndSw[i] = m_reader.readBit();
        m_pce.validCcElementTagSelect[i] = m_reader.readBits<std::uint8_t>(4);
    }
    m_reader.align();
    m_pce.commentFieldBytes = m_reader.readBits<std::uint8_t>(8);
    for (i = 0; i < m_pce.commentFieldBytes; ++i) {
        m_pce.commentFieldData[i] = m_reader.readBits<std::uint8_t>(8);
    }
    m_pce.commentFieldData[i] = 0;
    if (m_pce.channels > aacMaxChannels) {
        throw NotImplementedException(); // supported channel maximum exceeded
    }
}

/*!
 * \brief Parses "fill element".
 */
void AacFrameElementParser::parseFillElement(std::uint8_t sbrElement)
{
    std::uint16_t count = m_reader.readBits<std::uint8_t>(4);
    bool crcFlag = 0;
    if (count == 0xF) {
        count += m_reader.readBits<std::uint8_t>(8);
    }
    while (count > 0) {
    continueWhile:
        switch (m_reader.readBits<std::uint8_t>(4)) { // extension type
            using namespace AacExtensionTypes;
        case DynamicRange:
            count -= parseDynamicRange();
            break;
        case SbrDataCrc:
            crcFlag = true;
            [[fallthrough]];
        case SbrData:
            if (sbrElement == aacInvalidSbrElement) {
                throw InvalidDataException();
            } else {
                // ensure SBR element exists
                if (!m_sbrElements[sbrElement]) {
                    m_sbrElements[sbrElement] = makeSbrInfo(sbrElement);
                }
                parseSbrExtensionData(sbrElement, count, crcFlag);
                // set global flags
                m_sbrPresentFlag = 1;
                if (m_sbrElements[sbrElement]->ps) {
                    m_psUsed[sbrElement] = 1;
                    m_psUsedGlobal = 1;
                }
            }
            count = 0;
            break;
        case FillData:
            m_reader.skipBits(static_cast<std::size_t>(4 + 8 * (count - 1)));
            count = 0;
            break;
        case DataElement:
            // data element version
            if (m_reader.readBits<std::uint8_t>(4) == 0) {
                // ANC data
                std::uint8_t dataElementLength = 0, loopCounter = 0;
                std::uint16_t dataElementLengthPart;
                do {
                    dataElementLengthPart = m_reader.readBits<std::uint8_t>(8);
                    dataElementLength += static_cast<std::uint8_t>(dataElementLengthPart);
                    ++loopCounter;
                } while (dataElementLengthPart == 0xFF);
                for (std::uint16_t i = 0; i < dataElementLength; ++i) {
                    m_reader.skipBits(8); // data element byte
                    count -= static_cast<std::uint16_t>(dataElementLength + loopCounter + 1);
                    goto continueWhile;
                    // FIXME: loop will run at most once
                }
            }
            m_reader.skipBits(static_cast<std::size_t>(8 * (count - 1)));
            count = 0;
            break;
        case Fill:
        case SacData:
        default:
            m_reader.skipBits(static_cast<std::size_t>(4 + 8 * (count - 1)));
            count = 0;
        }
    }
}

/*!
 * \brief Parses a raw data block.
 *
 * Reads the element type first and then calls the appropriate method for the element type.
 */
void AacFrameElementParser::parseRawDataBlock()
{
    if (m_mpeg4AudioObjectId < Mpeg4AudioObjectIds::ErAacLc) {
        for (;;) {
            switch (m_reader.readBits<std::uint8_t>(3)) { // parse element type
                using namespace AacSyntaxElementTypes;
            case SingleChannelElement:
                parseSingleChannelElement();
                break;
            case ChannelPairElement:
                parseChannelPairElement();
                break;
            case ChannelCouplingElement:
                parseCouplingChannelElement();
                break;
            case LowFrequencyElement:
                parseLowFrequencyElement();
                break;
            case DataStreamElement:
                parseDataStreamElement();
                break;
            case ProgramConfigElement:
                parseProgramConfigElement();
                break;
            case FillElement:
                parseFillElement();
                break;
            case EndOfFrame:
                goto endOfBlock;
            default:;
            }
        }
    } else { // error resilience
        switch (m_mpeg4ChannelConfig) {
            using namespace Mpeg4ChannelConfigs;
            using namespace AacSyntaxElementTypes;
        case FrontCenter:
            parseSingleChannelElement();
            break;
        case FrontLeftFrontRight:
            parseChannelPairElement();
            break;
        case FrontCenterFrontLeftFrontRight:
            parseSingleChannelElement();
            parseChannelPairElement();
            break;
        case FrontCenterFrontLeftFrontRightBackCenter:
            parseSingleChannelElement();
            parseChannelPairElement();
            parseSingleChannelElement();
            break;
        case FrontCenterFrontLeftFrontRightBackLeftBackRight:
            parseSingleChannelElement();
            parseChannelPairElement();
            parseChannelPairElement();
            break;
        case FrontCenterFrontLeftFrontRightBackLeftBackRightLFEChannel:
            parseSingleChannelElement();
            parseChannelPairElement();
            parseChannelPairElement();
            parseSingleChannelElement();
            break;
        case FrontCenterFrontLeftFrontRightSideLeftSideRightBackLeftBackRightLFEChannel:
            parseSingleChannelElement();
            parseChannelPairElement();
            parseChannelPairElement();
            parseChannelPairElement();
            parseSingleChannelElement();
            break;
        }
    }
endOfBlock:;
}

/*!
 * \brief Parses the frame data from the specified \a stream at the current position.
 */
void AacFrameElementParser::parse(const AdtsFrame &adtsFrame, std::istream &stream, std::size_t dataSize)
{
    auto data = make_unique<char[]>(dataSize);
    stream.read(data.get(), static_cast<std::streamsize>(dataSize));
    parse(adtsFrame, data, dataSize);
}

/*!
 * \brief Parses the specified frame \a data.
 */
void AacFrameElementParser::parse(const AdtsFrame &adtsFrame, std::unique_ptr<char[]> &data, std::size_t dataSize)
{
    m_reader.reset(data.get(), dataSize);
    m_mpeg4AudioObjectId = adtsFrame.mpeg4AudioObjectId();
    m_mpeg4SamplingFrequencyIndex = adtsFrame.mpeg4SamplingFrequencyIndex();
    parseRawDataBlock();
}

/// \endcond

} // namespace TagParser
