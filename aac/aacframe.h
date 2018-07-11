#ifndef TAG_PARSER_AACFRAME_H
#define TAG_PARSER_AACFRAME_H

// NOTE: The AAC parser is still WIP. It does not work yet and its API/ABI may change even in patch releases.

#include <c++utilities/io/bitreader.h>

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

using SbrHuffTab = const sbyte (*)[2];

namespace AacSyntaxElementTypes {
enum KnownTypes : byte {
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
enum KnownTypes : byte { OnlyLongSequence, LongStartSequence, EightShortSequence, LongStopSequence };
}

namespace AacScaleFactorTypes {
enum KnownTypes : byte { ZeroHcb = 0, FirstPairHcb = 5, EscHcb = 11, QuadLen = 4, PairLen = 2, NoiseHcb = 13, IntensityHcb2 = 14, IntensityHcb = 15 };
}

namespace AacExtensionTypes {
enum KnownTypes : byte { Fill = 0, FillData = 1, DataElement = 2, DynamicRange = 11, SacData = 12, SbrData = 13, SbrDataCrc = 14 };
}

namespace BsFrameClasses {
enum BsFrameClass : byte { FixFix, FixVar, VarFix, VarVar };
}

namespace AacSbrExtensionIds {
enum KnownIds : byte { DrmParametricStereo = 0, Ps = 2 };
}

struct LIB_EXPORT AacLtpInfo {
    AacLtpInfo();
    byte lastBand;
    byte dataPresent;
    uint16 lag;
    byte lagUpdate;
    byte coef;
    byte longUsed[aacMaxLtpSfb];
    byte shortUsed[8];
    byte shortLagPresent[8];
    byte shortLag[8];
};

struct LIB_EXPORT AacPredictorInfo {
    AacPredictorInfo();
    byte maxSfb;
    byte reset;
    byte resetGroupNumber;
    byte predictionUsed[aacMaxSfb];
};

struct LIB_EXPORT AacPulseInfo {
    AacPulseInfo();
    byte count;
    byte startSfb;
    byte offset[4];
    byte amp[4];
};

struct LIB_EXPORT AacTnsInfo {
    AacTnsInfo();
    byte filt[8];
    byte coefRes[8];
    byte length[8][4];
    byte order[8][4];
    byte direction[8][4];
    byte coefCompress[8][4];
    byte coef[8][4][32];
};

struct LIB_EXPORT AacSsrInfo {
    AacSsrInfo();
    byte maxBand;
    byte adjustNum[4][8];
    byte alevcode[4][8][8];
    byte aloccode[4][8][8];
};

struct LIB_EXPORT AacDrcInfo {
    AacDrcInfo();
    byte present;
    byte bandCount;
    byte pceInstanceTag;
    byte excludedChannelsPresent;
    byte bandTop[17];
    byte progRefLevel;
    byte dynamicRangeSign[17];
    byte dynamicRangeControl[17];
    byte excludeMask[aacMaxChannels];
    byte additionalExcludedChannels[aacMaxChannels];
};

struct LIB_EXPORT AacPsInfo {
    AacPsInfo();
    byte headerRead;
    byte use34HybridBands;
    byte enableIID; // Inter-channel Intensity Difference
    byte iidMode;
    byte iidParCount;
    byte iidopdParCount;
    // TODO
};

struct LIB_EXPORT AacDrmPsInfo {
    AacDrmPsInfo();
    byte headerRead;
    byte use34HybridBands;
    byte enableIID; // Inter-channel Intensity Difference
    byte iidMode;
    byte iidParCount;
    byte iidopdParCount;
    // TODO
};

struct LIB_EXPORT AacSbrInfo {
    AacSbrInfo(byte sbrElementType, uint16 samplingFrequency, uint16 frameLength, bool isDrm);

    byte aacElementId;
    byte samplingFrequency;

    uint32 maxAacLine;

    byte rate;
    byte justSeeked;
    byte ret;

    byte ampRes[2];

    byte k0;
    byte kx;
    byte m;
    byte nMaster;
    byte nHigh;
    byte nLow;
    byte nq;
    byte nl[4];
    byte n[2];

    byte fMaster[64];
    byte fTableRes[2][64];
    byte fTableNoise[64];
    byte fTableLim[4][64];
    byte fGroup[5][64];
    byte ng[5];

    byte tableMapKToG[64];

    byte absBordLead[2];
    byte absBordTrail[2];
    byte relLeadCount[2];
    byte relTrailCount[2];

    byte le[2];
    byte lePrev[2];
    byte lq[2];

    byte te[2][aacSbrMaxLe + 1];
    byte tq[2][3];
    byte f[2][aacSbrMaxLe + 1];
    byte fPrev[2];

    //real_t *gTempPrev[2][5];
    //real_t *qTempPrev[2][5];
    //sbyte gqRingbufIndex[2];

    int16 e[2][64][aacSbrMaxLe];
    int16 ePrev[2][64];
    //real_t eOrig[2][64][aacSbrMaxLe];
    //real_t eCurr[2][64][aacSbrMaxLe];
    int32 q[2][64][2];
    //real_t qDiv[2][64][2];
    //real_t qDiv2[2][64][2];
    int32 qPrev[2][64];

    sbyte la[2];
    sbyte laPrev[2];

    byte bsInvfMode[2][aacSbrMaxLe];
    byte bsInvfModePrev[2][aacSbrMaxLe];
    //real_t bwArray[2][64];
    //real_t bwArrayPrev[2][64];

    byte noPatches;
    byte patchNoSubbands[64];
    byte patchStartSubband[64];

    byte bsAddHarmonic[2][64];
    byte bsAddHarmonicPrev[2][64];

    uint16 indexNoisePrev[2];
    byte psiIsPrev[2];

    byte bsStartFreqPrev;
    byte bsStopFreqPrev;
    byte bsXoverBandPrev;
    byte bsFreqScalePrev;
    byte bsAlterScalePrev;
    byte bsNoiseBandsPrev;

    sbyte prevEnvIsShort[2];

    sbyte kxPrev;
    byte bsco;
    byte bscoPrev;
    byte mPrev;
    uint16 frameLength;

    byte reset;
    uint32 frame;
    uint32 headerCount;

    byte idAac;
    //qmfa_info *qmfa[2];
    //qmfs_info *qmfs[2];

    //qmf_t Xsbr[2][aacSbrMaxNtsrhfg][64];

    byte isDrmSbr;
    std::shared_ptr<AacDrmPsInfo> drmPs;

    byte timeSlotsRateCount;
    byte timeSlotsCount;
    byte tHfGen;
    byte tHfAdj;

    std::shared_ptr<AacPsInfo> ps;
    byte psUsed;
    byte psResetFlag;

    byte bsHeaderFlag;
    byte bsCrcFlag;
    uint16 bsSbrCrcBits;
    byte bsProtocolVersion;
    byte bsAmpRes;
    byte bsStartFreq;
    byte bsStopFreq;
    byte bsXoverBand;
    byte bsFreqScale;
    byte bsAlterScale;
    byte bsNoiseBands;
    byte bsLimiterBands;
    byte bsLimiterGains;
    byte bsInterpolFreq;
    byte bsSmoothingMode;
    byte bsSamplerateMode;
    byte bsAddHarmonicFlag[2];
    byte bsAddHarmonicFlagPrev[2];
    byte bsExtendedData;
    byte bsExtensionId;
    byte bsExtensionData;
    byte bsCoupling;
    byte bsFrameClass[2];
    byte bsRelBord[2][9];
    byte bsRelBord0[2][9];
    byte bsRelBord1[2][9];
    byte bsPointer[2];
    byte bsAbsBord0[2];
    byte bsAbsBord1[2];
    byte bsRelCount0[2];
    byte bsRelCount1[2];
    byte bsDfEnv[2][9];
    byte bsDfNoise[2][3];
};

struct LIB_EXPORT AacProgramConfig {
    AacProgramConfig();
    byte elementInstanceTag;
    byte objectType;
    byte samplingFrequencyIndex;
    byte frontChannelElementCount;
    byte sideChannelElementCount;
    byte backChannelElementCount;
    byte lfeChannelElementCount;
    byte assocDataElementCount;
    byte validCcElementCount;
    byte monoMixdownPresent;
    byte monoMixdownElementNumber;
    byte stereoMixdownPresent;
    byte stereoMixdownElementNumber;
    byte matrixMixdownIdxPresent;
    byte pseudoSurroundEnable;
    byte matrixMixdownIdx;
    byte frontElementIsCpe[16];
    byte frontElementTagSelect[16];
    byte sideElementIsCpe[16];
    byte sideElementTagSelect[16];
    byte backElementIsCpe[16];
    byte backElementTagSelect[16];
    byte lfeElementTagSelect[16];
    byte assocDataElementTagSelect[16];
    byte ccElementIsIndSw[16];
    byte validCcElementTagSelect[16];
    byte channels;
    byte commentFieldBytes;
    byte commentFieldData[257];
    byte frontChannelCount;
    byte sideChannelCount;
    byte backChannelCount;
    byte lfeChannelCount;
    byte sceChannel[16];
    byte cpeChannel[16];
};

struct LIB_EXPORT AacIcsInfo {
    AacIcsInfo();

    byte maxSfb;

    byte swbCount;
    byte windowGroupCount;
    byte windowCount;
    byte windowSequence;
    byte windowGroupLengths[8];
    byte windowShape;
    byte scaleFactorGrouping;
    uint16 sectionSfbOffset[8][15 * 8];
    uint16 swbOffset[52];
    uint16 maxSwbOffset;

    byte sectionCb[8][15 * 8];
    uint16 sectionStart[8][15 * 8];
    uint16 sectionEnd[8][15 * 8];
    byte sfbCb[8][15 * 8];
    byte sectionsPerGroup[8];

    byte globalGain;
    uint16 scaleFactors[8][51];

    byte midSideCodingMaskPresent;
    byte midSideCodingUsed[aacMaxWindowGroups][aacMaxSfb];

    byte noiseUsed;
    byte isUsed;

    byte pulseDataPresent;
    byte tnsDataPresent;
    byte gainControlPresent;
    byte predictorDataPresent;

    AacPulseInfo pulse;
    AacTnsInfo tns;
    AacPredictorInfo predictor;
    AacLtpInfo ltp1;
    AacLtpInfo ltp2;
    AacSsrInfo ssr;
    std::shared_ptr<AacSbrInfo> sbr;

    // error resilience
    uint16 reorderedSpectralDataLength;
    byte longestCodewordLength;
    byte sfConcealment;
    byte revGlobalGain;
    uint16 rvlcSfLength;
    uint16 dpcmNoiseNrg;
    byte sfEscapesPresent;
    byte rvlcEscapesLength;
    uint16 dpcmNoiseLastPos;
};

class LIB_EXPORT AacFrameElementParser {
public:
    AacFrameElementParser(
        byte audioObjectId, byte samplingFrequencyIndex, byte extensionSamplingFrequencyIndex, byte channelConfig, uint16 frameLength = 1024);

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
    void parseSpectralData(AacIcsInfo &ics, int16 *specData);
    void parseSideInfo(AacIcsInfo &ics, bool scaleFlag);
    byte parseExcludedChannels();
    byte parseDynamicRange();
    static sbyte sbrLog2(const sbyte val);
    int16 sbrHuffmanDec(SbrHuffTab table);
    void parseSbrGrid(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    void parseSbrDtdf(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    void parseInvfMode(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    void parseSbrEnvelope(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    void parseSbrNoise(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    void parseSbrSinusoidalCoding(std::shared_ptr<AacSbrInfo> &sbr, byte channel);
    uint16 parseSbrExtension(std::shared_ptr<AacSbrInfo> &sbr, byte extensionId, byte bitsLeft);
    uint16 parsePsData(std::shared_ptr<AacPsInfo> &ps, byte &header);
    uint16 parseDrmPsData(std::shared_ptr<AacDrmPsInfo> &drmPs);
    void parseSbrSingleChannelElement(std::shared_ptr<AacSbrInfo> &sbr);
    void parseSbrChannelPairElement(std::shared_ptr<AacSbrInfo> &sbr);
    std::shared_ptr<AacSbrInfo> makeSbrInfo(byte sbrElement, bool isDrm = false);
    void parseSbrExtensionData(byte sbrElement, uint16 count, bool crcFlag);
    byte parseHuffmanScaleFactor();
    void parseHuffmanSpectralData(byte cb, int16 *sp);
    void huffmanSignBits(int16 *sp, byte len);
    void huffman2StepQuad(byte cb, int16 *sp);
    void huffmanBinaryQuadSign(byte cb, int16 *sp);
    void huffmanBinaryPair(byte cb, int16 *sp);
    void huffman2StepPair(byte cb, int16 *sp);
    void huffmanBinaryPairSign(byte cb, int16 *sp);
    void huffman2StepPairSign(byte cb, int16 *sp);
    int16 huffmanGetEscape(int16 sp);
    constexpr static int16 huffmanCodebook(byte i);
    static void vcb11CheckLav(byte cb, int16 *sp);
    void calculateWindowGroupingInfo(AacIcsInfo &ics);
    void parseIndividualChannelStream(AacIcsInfo &ics, int16 *specData, bool scaleFlag = false);
    void parseSingleChannelElement();
    void parseChannelPairElement();
    void parseCouplingChannelElement();
    void parseLowFrequencyElement();
    void parseDataStreamElement();
    void parseProgramConfigElement();
    void parseFillElement(byte sbrElement = aacInvalidSbrElement);
    void parseRawDataBlock();

    // these fields contain setup information
    IoUtilities::BitReader m_reader;
    byte m_mpeg4AudioObjectId;
    byte m_mpeg4SamplingFrequencyIndex;
    byte m_mpeg4ExtensionSamplingFrequencyIndex;
    byte m_mpeg4ChannelConfig;
    uint16 m_frameLength;
    byte m_aacSectionDataResilienceFlag;
    byte m_aacScalefactorDataResilienceFlag;
    byte m_aacSpectralDataResilienceFlag;
    // these fields will be parsed
    byte m_elementId[aacMaxChannels];
    byte m_channelCount;
    byte m_elementCount;
    byte m_elementChannelCount[aacMaxSyntaxElements];
    //byte m_channel;
    //int16 m_pairedChannel;
    byte m_elementInstanceTag[aacMaxSyntaxElements];
    byte m_commonWindow;
    AacIcsInfo m_ics1;
    AacIcsInfo m_ics2;
    AacDrcInfo m_drc;
    AacProgramConfig m_pce;
    byte m_sbrPresentFlag;
    //byte m_forceUpSampling;
    //byte m_downSampledSbr;
    std::shared_ptr<AacSbrInfo> m_sbrElements[aacMaxSyntaxElements];
    byte m_psUsed[aacMaxSyntaxElements];
    byte m_psUsedGlobal;
    byte m_psResetFlag;
};

/*!
 * \brief Constructs a new parser with the specified setup information.
 */
inline AacFrameElementParser::AacFrameElementParser(
    byte audioObjectId, byte samplingFrequencyIndex, byte extensionSamplingFrequencyIndex, byte channelConfig, uint16 frameLength)
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

inline sbyte AacFrameElementParser::sbrLog2(const sbyte val)
{
    static const int log2tab[] = { 0, 0, 1, 2, 2, 3, 3, 3, 3, 4 };
    return (val < 10 && val >= 0) ? log2tab[val] : 0;
}

constexpr int16 AacFrameElementParser::huffmanCodebook(byte i)
{
    return static_cast<int16>(i ? (16428320 & 0xFFFF) : ((16428320 >> 16) & 0xFFFF));
}

/// \endcond

} // namespace TagParser

#endif // TAG_PARSER_AACFRAME_H
