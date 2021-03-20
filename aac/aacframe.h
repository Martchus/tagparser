#ifndef TAG_PARSER_AACFRAME_H
#define TAG_PARSER_AACFRAME_H

// NOTE: The AAC parser is still WIP. It does not work yet and its API/ABI may change even in patch releases.

#include "../global.h"

#include <c++utilities/io/bitreader.h>

#include <cstdint>
#include <memory>

namespace TagParser {

/// \cond

class AdtsFrame;

constexpr auto aacMaxChannels = 64;
constexpr auto aacMaxSyntaxElements = 48;
constexpr auto aacMaxWindowGroups = 8;
constexpr auto aacMaxSfb = 51;
constexpr auto aacMaxLtpSfb = 40;
constexpr auto aacMaxltpSfbS = 8;
constexpr auto aacInvalidSbrElement = 0xFF;
constexpr auto aacNoTimeSlots960 = 15;
constexpr auto aacNoTimeSlots = 16;
constexpr auto aacSbrRate = 2;
constexpr auto aacSbrM = 49;
constexpr auto aacSbrMaxLe = 5;
constexpr auto aacSbrMaxNtsrhfg = 40;

using SbrHuffTab = const std::int8_t (*)[2];

namespace AacSyntaxElementTypes {
enum KnownTypes : std::uint8_t {
    SingleChannelElement, /**< codes a single audio channel */
    ChannelPairElement, /**< codes steroe signal */
    ChannelCouplingElement, /**< something to do with channel coupling (not implemented in libfaad2) */
    LowFrequencyElement, /**< low-frequency effects? referenced as "special effects" in RTP doc */
    DataStreamElement, /**< user data */
    ProgramConfigElement, /**< describes bitstream */
    FillElement, /**< pad space/extension data */
    EndOfFrame /**< marks the end of the frame */
};
}

namespace AacIcsSequenceTypes {
enum KnownTypes : std::uint8_t { OnlyLongSequence, LongStartSequence, EightShortSequence, LongStopSequence };
}

namespace AacScaleFactorTypes {
enum KnownTypes : std::uint8_t {
    ZeroHcb = 0,
    FirstPairHcb = 5,
    EscHcb = 11,
    QuadLen = 4,
    PairLen = 2,
    NoiseHcb = 13,
    IntensityHcb2 = 14,
    IntensityHcb = 15
};
}

namespace AacExtensionTypes {
enum KnownTypes : std::uint8_t { Fill = 0, FillData = 1, DataElement = 2, DynamicRange = 11, SacData = 12, SbrData = 13, SbrDataCrc = 14 };
}

namespace BsFrameClasses {
enum BsFrameClass : std::uint8_t { FixFix, FixVar, VarFix, VarVar };
}

namespace AacSbrExtensionIds {
enum KnownIds : std::uint8_t { DrmParametricStereo = 0, Ps = 2 };
}

struct TAG_PARSER_EXPORT AacLtpInfo {
    AacLtpInfo();
    std::uint8_t lastBand;
    std::uint8_t dataPresent;
    std::uint16_t lag;
    std::uint8_t lagUpdate;
    std::uint8_t coef;
    std::uint8_t longUsed[aacMaxLtpSfb];
    std::uint8_t shortUsed[8];
    std::uint8_t shortLagPresent[8];
    std::uint8_t shortLag[8];
};

struct TAG_PARSER_EXPORT AacPredictorInfo {
    AacPredictorInfo();
    std::uint8_t maxSfb;
    std::uint8_t reset;
    std::uint8_t resetGroupNumber;
    std::uint8_t predictionUsed[aacMaxSfb];
};

struct TAG_PARSER_EXPORT AacPulseInfo {
    AacPulseInfo();
    std::uint8_t count;
    std::uint8_t startSfb;
    std::uint8_t offset[4];
    std::uint8_t amp[4];
};

struct TAG_PARSER_EXPORT AacTnsInfo {
    AacTnsInfo();
    std::uint8_t filt[8];
    std::uint8_t coefRes[8];
    std::uint8_t length[8][4];
    std::uint8_t order[8][4];
    std::uint8_t direction[8][4];
    std::uint8_t coefCompress[8][4];
    std::uint8_t coef[8][4][32];
};

struct TAG_PARSER_EXPORT AacSsrInfo {
    AacSsrInfo();
    std::uint8_t maxBand;
    std::uint8_t adjustNum[4][8];
    std::uint8_t alevcode[4][8][8];
    std::uint8_t aloccode[4][8][8];
};

struct TAG_PARSER_EXPORT AacDrcInfo {
    AacDrcInfo();
    std::uint8_t present;
    std::uint8_t bandCount;
    std::uint8_t pceInstanceTag;
    std::uint8_t excludedChannelsPresent;
    std::uint8_t bandTop[17];
    std::uint8_t progRefLevel;
    std::uint8_t dynamicRangeSign[17];
    std::uint8_t dynamicRangeControl[17];
    std::uint8_t excludeMask[aacMaxChannels];
    std::uint8_t additionalExcludedChannels[aacMaxChannels];
};

struct TAG_PARSER_EXPORT AacPsInfo {
    AacPsInfo();
    std::uint8_t headerRead;
    std::uint8_t use34HybridBands;
    std::uint8_t enableIID; // Inter-channel Intensity Difference
    std::uint8_t iidMode;
    std::uint8_t iidParCount;
    std::uint8_t iidopdParCount;
    // TODO
};

struct TAG_PARSER_EXPORT AacDrmPsInfo {
    AacDrmPsInfo();
    std::uint8_t headerRead;
    std::uint8_t use34HybridBands;
    std::uint8_t enableIID; // Inter-channel Intensity Difference
    std::uint8_t iidMode;
    std::uint8_t iidParCount;
    std::uint8_t iidopdParCount;
    // TODO
};

struct TAG_PARSER_EXPORT AacSbrInfo {
    AacSbrInfo(std::uint8_t sbrElementType, std::uint16_t samplingFrequency, std::uint16_t frameLength, bool isDrm);

    std::uint8_t aacElementId;
    std::uint16_t samplingFrequency;

    std::uint32_t maxAacLine;

    std::uint8_t rate;
    std::uint8_t justSeeked;
    std::uint8_t ret;

    std::uint8_t ampRes[2];

    std::uint8_t k0;
    std::uint8_t kx;
    std::uint8_t m;
    std::uint8_t nMaster;
    std::uint8_t nHigh;
    std::uint8_t nLow;
    std::uint8_t nq;
    std::uint8_t nl[4];
    std::uint8_t n[2];

    std::uint8_t fMaster[64];
    std::uint8_t fTableRes[2][64];
    std::uint8_t fTableNoise[64];
    std::uint8_t fTableLim[4][64];
    std::uint8_t fGroup[5][64];
    std::uint8_t ng[5];

    std::uint8_t tableMapKToG[64];

    std::uint8_t absBordLead[2];
    std::uint8_t absBordTrail[2];
    std::uint8_t relLeadCount[2];
    std::uint8_t relTrailCount[2];

    std::uint8_t le[2];
    std::uint8_t lePrev[2];
    std::uint8_t lq[2];

    std::uint8_t te[2][aacSbrMaxLe + 1];
    std::uint8_t tq[2][3];
    std::uint8_t f[2][aacSbrMaxLe + 1];
    std::uint8_t fPrev[2];

    //real_t *gTempPrev[2][5];
    //real_t *qTempPrev[2][5];
    //sbyte gqRingbufIndex[2];

    std::int16_t e[2][64][aacSbrMaxLe];
    std::int16_t ePrev[2][64];
    //real_t eOrig[2][64][aacSbrMaxLe];
    //real_t eCurr[2][64][aacSbrMaxLe];
    std::int32_t q[2][64][2];
    //real_t qDiv[2][64][2];
    //real_t qDiv2[2][64][2];
    std::int32_t qPrev[2][64];

    std::int8_t la[2];
    std::int8_t laPrev[2];

    std::uint8_t bsInvfMode[2][aacSbrMaxLe];
    std::uint8_t bsInvfModePrev[2][aacSbrMaxLe];
    //real_t bwArray[2][64];
    //real_t bwArrayPrev[2][64];

    std::uint8_t noPatches;
    std::uint8_t patchNoSubbands[64];
    std::uint8_t patchStartSubband[64];

    std::uint8_t bsAddHarmonic[2][64];
    std::uint8_t bsAddHarmonicPrev[2][64];

    std::uint16_t indexNoisePrev[2];
    std::uint8_t psiIsPrev[2];

    std::uint8_t bsStartFreqPrev;
    std::uint8_t bsStopFreqPrev;
    std::uint8_t bsXoverBandPrev;
    std::uint8_t bsFreqScalePrev;
    std::uint8_t bsAlterScalePrev;
    std::uint8_t bsNoiseBandsPrev;

    std::int8_t prevEnvIsShort[2];

    std::int8_t kxPrev;
    std::uint8_t bsco;
    std::uint8_t bscoPrev;
    std::uint8_t mPrev;
    std::uint16_t frameLength;

    std::uint8_t reset;
    std::uint32_t frame;
    std::uint32_t headerCount;

    std::uint8_t idAac;
    //qmfa_info *qmfa[2];
    //qmfs_info *qmfs[2];

    //qmf_t Xsbr[2][aacSbrMaxNtsrhfg][64];

    std::uint8_t isDrmSbr;
    std::shared_ptr<AacDrmPsInfo> drmPs;

    std::uint8_t timeSlotsRateCount;
    std::uint8_t timeSlotsCount;
    std::uint8_t tHfGen;
    std::uint8_t tHfAdj;

    std::shared_ptr<AacPsInfo> ps;
    std::uint8_t psUsed;
    std::uint8_t psResetFlag;

    std::uint8_t bsHeaderFlag;
    std::uint8_t bsCrcFlag;
    std::uint16_t bsSbrCrcBits;
    std::uint8_t bsProtocolVersion;
    std::uint8_t bsAmpRes;
    std::uint8_t bsStartFreq;
    std::uint8_t bsStopFreq;
    std::uint8_t bsXoverBand;
    std::uint8_t bsFreqScale;
    std::uint8_t bsAlterScale;
    std::uint8_t bsNoiseBands;
    std::uint8_t bsLimiterBands;
    std::uint8_t bsLimiterGains;
    std::uint8_t bsInterpolFreq;
    std::uint8_t bsSmoothingMode;
    std::uint8_t bsSamplerateMode;
    std::uint8_t bsAddHarmonicFlag[2];
    std::uint8_t bsAddHarmonicFlagPrev[2];
    std::uint8_t bsExtendedData;
    std::uint8_t bsExtensionId;
    std::uint8_t bsExtensionData;
    std::uint8_t bsCoupling;
    std::uint8_t bsFrameClass[2];
    std::uint8_t bsRelBord[2][9];
    std::uint8_t bsRelBord0[2][9];
    std::uint8_t bsRelBord1[2][9];
    std::uint8_t bsPointer[2];
    std::uint8_t bsAbsBord0[2];
    std::uint8_t bsAbsBord1[2];
    std::uint8_t bsRelCount0[2];
    std::uint8_t bsRelCount1[2];
    std::uint8_t bsDfEnv[2][9];
    std::uint8_t bsDfNoise[2][3];
};

struct TAG_PARSER_EXPORT AacProgramConfig {
    AacProgramConfig();
    std::uint8_t elementInstanceTag;
    std::uint8_t objectType;
    std::uint8_t samplingFrequencyIndex;
    std::uint8_t frontChannelElementCount;
    std::uint8_t sideChannelElementCount;
    std::uint8_t backChannelElementCount;
    std::uint8_t lfeChannelElementCount;
    std::uint8_t assocDataElementCount;
    std::uint8_t validCcElementCount;
    std::uint8_t monoMixdownPresent;
    std::uint8_t monoMixdownElementNumber;
    std::uint8_t stereoMixdownPresent;
    std::uint8_t stereoMixdownElementNumber;
    std::uint8_t matrixMixdownIdxPresent;
    std::uint8_t pseudoSurroundEnable;
    std::uint8_t matrixMixdownIdx;
    std::uint8_t frontElementIsCpe[16];
    std::uint8_t frontElementTagSelect[16];
    std::uint8_t sideElementIsCpe[16];
    std::uint8_t sideElementTagSelect[16];
    std::uint8_t backElementIsCpe[16];
    std::uint8_t backElementTagSelect[16];
    std::uint8_t lfeElementTagSelect[16];
    std::uint8_t assocDataElementTagSelect[16];
    std::uint8_t ccElementIsIndSw[16];
    std::uint8_t validCcElementTagSelect[16];
    std::uint8_t channels;
    std::uint8_t commentFieldBytes;
    std::uint8_t commentFieldData[257];
    std::uint8_t frontChannelCount;
    std::uint8_t sideChannelCount;
    std::uint8_t backChannelCount;
    std::uint8_t lfeChannelCount;
    std::uint8_t sceChannel[16];
    std::uint8_t cpeChannel[16];
};

struct TAG_PARSER_EXPORT AacIcsInfo {
    AacIcsInfo();

    std::uint8_t maxSfb;

    std::uint8_t swbCount;
    std::uint8_t windowGroupCount;
    std::uint8_t windowCount;
    std::uint8_t windowSequence;
    std::uint8_t windowGroupLengths[8];
    std::uint8_t windowShape;
    std::uint8_t scaleFactorGrouping;
    std::uint16_t sectionSfbOffset[8][15 * 8];
    std::uint16_t swbOffset[52];
    std::uint16_t maxSwbOffset;

    std::uint8_t sectionCb[8][15 * 8];
    std::uint16_t sectionStart[8][15 * 8];
    std::uint16_t sectionEnd[8][15 * 8];
    std::uint8_t sfbCb[8][15 * 8];
    std::uint8_t sectionsPerGroup[8];

    std::uint8_t globalGain;
    std::uint16_t scaleFactors[8][51];

    std::uint8_t midSideCodingMaskPresent;
    std::uint8_t midSideCodingUsed[aacMaxWindowGroups][aacMaxSfb];

    std::uint8_t noiseUsed;
    std::uint8_t isUsed;

    std::uint8_t pulseDataPresent;
    std::uint8_t tnsDataPresent;
    std::uint8_t gainControlPresent;
    std::uint8_t predictorDataPresent;

    AacPulseInfo pulse;
    AacTnsInfo tns;
    AacPredictorInfo predictor;
    AacLtpInfo ltp1;
    AacLtpInfo ltp2;
    AacSsrInfo ssr;
    std::shared_ptr<AacSbrInfo> sbr;

    // error resilience
    std::uint16_t reorderedSpectralDataLength;
    std::uint8_t longestCodewordLength;
    std::uint8_t sfConcealment;
    std::uint8_t revGlobalGain;
    std::uint16_t rvlcSfLength;
    std::uint16_t dpcmNoiseNrg;
    std::uint8_t sfEscapesPresent;
    std::uint8_t rvlcEscapesLength;
    std::uint16_t dpcmNoiseLastPos;
};

class TAG_PARSER_EXPORT AacFrameElementParser {
public:
    AacFrameElementParser(std::uint8_t audioObjectId, std::uint8_t samplingFrequencyIndex, std::uint8_t extensionSamplingFrequencyIndex,
        std::uint8_t channelConfig, std::uint16_t frameLength = 1024);

    void parse(const AdtsFrame &adtsFrame, std::unique_ptr<char[]> &data, std::size_t dataSize);
    void parse(const AdtsFrame &adtsFrame, std::istream &stream, std::size_t dataSize);

private:
    void parseLtpInfo(const AacIcsInfo &ics, AacLtpInfo &ltp);
    void parseIcsInfo(AacIcsInfo &ics);
    void parseSectionData(AacIcsInfo &ics);
    void decodeScaleFactorData(AacIcsInfo &ics);
    void decodeRvlcScaleFactorData(AacIcsInfo &ics);
    void parseScaleFactorData(AacIcsInfo &ics);
    void parsePulseData(AacIcsInfo &ics);
    void parseTnsData(AacIcsInfo &ics);
    void parseGainControlData(AacIcsInfo &ics);
    void parseSpectralData(AacIcsInfo &ics, std::int16_t *specData);
    void parseSideInfo(AacIcsInfo &ics, bool scaleFlag);
    std::uint8_t parseExcludedChannels();
    std::uint8_t parseDynamicRange();
    static std::int8_t sbrLog2(const std::int8_t val);
    std::int16_t sbrHuffmanDec(SbrHuffTab table);
    void parseSbrGrid(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    void parseSbrDtdf(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    void parseInvfMode(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    void parseSbrEnvelope(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    void parseSbrNoise(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    void parseSbrSinusoidalCoding(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t channel);
    std::uint16_t parseSbrExtension(std::shared_ptr<AacSbrInfo> &sbr, std::uint8_t extensionId, std::uint8_t bitsLeft);
    std::uint16_t parsePsData(std::shared_ptr<AacPsInfo> &ps, std::uint8_t &header);
    std::uint16_t parseDrmPsData(std::shared_ptr<AacDrmPsInfo> &drmPs);
    void parseSbrSingleChannelElement(std::shared_ptr<AacSbrInfo> &sbr);
    void parseSbrChannelPairElement(std::shared_ptr<AacSbrInfo> &sbr);
    std::shared_ptr<AacSbrInfo> makeSbrInfo(std::uint8_t sbrElement, bool isDrm = false);
    void parseSbrExtensionData(std::uint8_t sbrElement, std::uint16_t count, bool crcFlag);
    std::uint8_t parseHuffmanScaleFactor();
    void parseHuffmanSpectralData(std::uint8_t cb, std::int16_t *sp);
    void huffmanSignBits(std::int16_t *sp, std::uint8_t len);
    void huffman2StepQuad(std::uint8_t cb, std::int16_t *sp);
    void huffmanBinaryQuadSign(std::uint8_t cb, std::int16_t *sp);
    void huffmanBinaryPair(std::uint8_t cb, std::int16_t *sp);
    void huffman2StepPair(std::uint8_t cb, std::int16_t *sp);
    void huffmanBinaryPairSign(std::uint8_t cb, std::int16_t *sp);
    void huffman2StepPairSign(std::uint8_t cb, std::int16_t *sp);
    std::int16_t huffmanGetEscape(std::int16_t sp);
    constexpr static std::int16_t huffmanCodebook(std::uint8_t i);
    static void vcb11CheckLav(std::uint8_t cb, std::int16_t *sp);
    void calculateWindowGroupingInfo(AacIcsInfo &ics);
    void parseIndividualChannelStream(AacIcsInfo &ics, std::int16_t *specData, bool scaleFlag = false);
    void parseSingleChannelElement();
    void parseChannelPairElement();
    void parseCouplingChannelElement();
    void parseLowFrequencyElement();
    void parseDataStreamElement();
    void parseProgramConfigElement();
    void parseFillElement(std::uint8_t sbrElement = aacInvalidSbrElement);
    void parseRawDataBlock();

    // these fields contain setup information
    CppUtilities::BitReader m_reader;
    std::uint8_t m_mpeg4AudioObjectId;
    std::uint8_t m_mpeg4SamplingFrequencyIndex;
    std::uint8_t m_mpeg4ExtensionSamplingFrequencyIndex;
    std::uint8_t m_mpeg4ChannelConfig;
    std::uint16_t m_frameLength;
    std::uint8_t m_aacSectionDataResilienceFlag;
    std::uint8_t m_aacScalefactorDataResilienceFlag;
    std::uint8_t m_aacSpectralDataResilienceFlag;
    // these fields will be parsed
    std::uint8_t m_elementId[aacMaxChannels];
    std::uint8_t m_channelCount;
    std::uint8_t m_elementCount;
    std::uint8_t m_elementChannelCount[aacMaxSyntaxElements];
    //std::uint8_t m_channel;
    //std::int16_t m_pairedChannel;
    std::uint8_t m_elementInstanceTag[aacMaxSyntaxElements];
    std::uint8_t m_commonWindow;
    AacIcsInfo m_ics1;
    AacIcsInfo m_ics2;
    AacDrcInfo m_drc;
    AacProgramConfig m_pce;
    std::uint8_t m_sbrPresentFlag;
    //std::uint8_t m_forceUpSampling;
    //std::uint8_t m_downSampledSbr;
    std::shared_ptr<AacSbrInfo> m_sbrElements[aacMaxSyntaxElements];
    std::uint8_t m_psUsed[aacMaxSyntaxElements];
    std::uint8_t m_psUsedGlobal;
    std::uint8_t m_psResetFlag;
};

/*!
 * \brief Constructs a new parser with the specified setup information.
 */
inline AacFrameElementParser::AacFrameElementParser(std::uint8_t audioObjectId, std::uint8_t samplingFrequencyIndex,
    std::uint8_t extensionSamplingFrequencyIndex, std::uint8_t channelConfig, std::uint16_t frameLength)
    : m_reader(nullptr, nullptr)
    , m_mpeg4AudioObjectId(audioObjectId)
    , m_mpeg4SamplingFrequencyIndex(samplingFrequencyIndex)
    , m_mpeg4ExtensionSamplingFrequencyIndex(extensionSamplingFrequencyIndex)
    , m_mpeg4ChannelConfig(channelConfig)
    , m_frameLength(frameLength)
    , m_aacSpectralDataResilienceFlag(0)
    , m_elementId{ 0 }
    , m_channelCount(0)
    , m_elementCount(0)
    , m_elementChannelCount{ 0 }
    , m_elementInstanceTag{ 0 }
    , m_commonWindow(0)
    ,
    //m_channel(0),
    //m_pairedChannel(0),
    m_sbrPresentFlag(0)
    ,
    //m_forceUpSampling(0),
    //m_downSampledSbr(0),
    m_sbrElements{ 0 }
    , m_psUsed{ 0 }
    , m_psUsedGlobal(0)
    , m_psResetFlag(0)
{
}

inline std::int8_t AacFrameElementParser::sbrLog2(const std::int8_t val)
{
    static const std::int8_t log2tab[] = { 0, 0, 1, 2, 2, 3, 3, 3, 3, 4 };
    return (val < 10 && val >= 0) ? log2tab[val] : 0;
}

constexpr std::int16_t AacFrameElementParser::huffmanCodebook(std::uint8_t i)
{
    return static_cast<std::int16_t>(i ? (16428320 & 0xFFFF) : ((16428320 >> 16) & 0xFFFF));
}

/// \endcond

} // namespace TagParser

#endif // TAG_PARSER_AACFRAME_H
