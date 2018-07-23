#include "./mp4container.h"
#include "./mp4ids.h"

#include "../backuphelper.h"
#include "../exceptions.h"
#include "../mediafileinfo.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/io/copy.h>

#include <unistd.h>

#include <memory>
#include <numeric>
#include <tuple>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace TagParser {

/*!
 * \class TagParser::Mp4Container
 * \brief Implementation of GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>.
 */

/*!
 * \brief Constructs a new container for the specified \a fileInfo at the specified \a startOffset.
 */
Mp4Container::Mp4Container(MediaFileInfo &fileInfo, uint64 startOffset)
    : GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>(fileInfo, startOffset)
    , m_fragmented(false)
{
}

Mp4Container::~Mp4Container()
{
}

void Mp4Container::reset()
{
    GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>::reset();
    m_fragmented = false;
}

ElementPosition Mp4Container::determineTagPosition(Diagnostics &diag) const
{
    if (m_firstElement) {
        const Mp4Atom *mediaDataAtom = m_firstElement->siblingById(Mp4AtomIds::MediaData, diag);
        const Mp4Atom *userDataAtom = m_firstElement->subelementByPath(diag, Mp4AtomIds::Movie, Mp4AtomIds::UserData);
        if (mediaDataAtom && userDataAtom) {
            return userDataAtom->startOffset() < mediaDataAtom->startOffset() ? ElementPosition::BeforeData : ElementPosition::AfterData;
        }
    }
    return ElementPosition::Keep;
}

ElementPosition Mp4Container::determineIndexPosition(Diagnostics &diag) const
{
    if (m_firstElement) {
        const Mp4Atom *mediaDataAtom = m_firstElement->siblingById(Mp4AtomIds::MediaData, diag);
        const Mp4Atom *movieAtom = m_firstElement->siblingById(Mp4AtomIds::Movie, diag);
        if (mediaDataAtom && movieAtom) {
            return movieAtom->startOffset() < mediaDataAtom->startOffset() ? ElementPosition::BeforeData : ElementPosition::AfterData;
        }
    }
    return ElementPosition::Keep;
}

void Mp4Container::internalParseHeader(Diagnostics &diag)
{
    //const string context("parsing header of MP4 container"); will be used when generating notifications
    m_firstElement = make_unique<Mp4Atom>(*this, startOffset());
    m_firstElement->parse(diag);
    auto *const ftypAtom = m_firstElement->siblingByIdIncludingThis(Mp4AtomIds::FileType, diag);
    if (!ftypAtom) {
        m_doctype.clear();
        m_version = 0;
        return;
    }
    stream().seekg(static_cast<iostream::off_type>(ftypAtom->dataOffset()));
    m_doctype = reader().readString(4);
    m_version = reader().readUInt32BE();
}

void Mp4Container::internalParseTags(Diagnostics &diag)
{
    const string context("parsing tags of MP4 container");
    auto *const udtaAtom = firstElement()->subelementByPath(diag, Mp4AtomIds::Movie, Mp4AtomIds::UserData);
    if (!udtaAtom) {
        return;
    }
    auto *metaAtom = udtaAtom->childById(Mp4AtomIds::Meta, diag);
    bool surplusMetaAtoms = false;
    while (metaAtom) {
        metaAtom->parse(diag);
        m_tags.emplace_back(make_unique<Mp4Tag>());
        try {
            m_tags.back()->parse(*metaAtom, diag);
        } catch (const NoDataFoundException &) {
            m_tags.pop_back();
        }
        if ((metaAtom = metaAtom->siblingById(Mp4AtomIds::Meta, diag))) {
            surplusMetaAtoms = true;
        }
        if (!m_tags.empty()) {
            break;
        }
    }
    if (surplusMetaAtoms) {
        diag.emplace_back(DiagLevel::Warning, "udta atom contains multiple meta atoms. Surplus meta atoms will be ignored.", context);
    }
}

void Mp4Container::internalParseTracks(Diagnostics &diag)
{
    static const string context("parsing tracks of MP4 container");
    try {
        // get moov atom which holds track information
        if (Mp4Atom *moovAtom = firstElement()->siblingByIdIncludingThis(Mp4AtomIds::Movie, diag)) {
            // get mvhd atom which holds overall track information
            if (Mp4Atom *mvhdAtom = moovAtom->childById(Mp4AtomIds::MovieHeader, diag)) {
                if (mvhdAtom->dataSize() > 0) {
                    stream().seekg(static_cast<iostream::off_type>(mvhdAtom->dataOffset()));
                    byte version = reader().readByte();
                    if ((version == 1 && mvhdAtom->dataSize() >= 32) || (mvhdAtom->dataSize() >= 20)) {
                        stream().seekg(3, ios_base::cur); // skip flags
                        switch (version) {
                        case 0:
                            m_creationTime = DateTime::fromDate(1904, 1, 1) + TimeSpan::fromSeconds(reader().readUInt32BE());
                            m_modificationTime = DateTime::fromDate(1904, 1, 1) + TimeSpan::fromSeconds(reader().readUInt32BE());
                            m_timeScale = reader().readUInt32BE();
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt32BE()) / static_cast<double>(m_timeScale));
                            break;
                        case 1:
                            m_creationTime = DateTime::fromDate(1904, 1, 1) + TimeSpan::fromSeconds(reader().readUInt64BE());
                            m_modificationTime = DateTime::fromDate(1904, 1, 1) + TimeSpan::fromSeconds(reader().readUInt64BE());
                            m_timeScale = reader().readUInt32BE();
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt64BE()) / static_cast<double>(m_timeScale));
                            break;
                        default:;
                        }
                    } else {
                        diag.emplace_back(DiagLevel::Critical, "mvhd atom is truncated.", context);
                    }
                } else {
                    diag.emplace_back(DiagLevel::Critical, "mvhd atom is empty.", context);
                }
            } else {
                diag.emplace_back(DiagLevel::Critical, "mvhd atom is does not exist.", context);
            }
            // get mvex atom which holds default values for fragmented files
            if (Mp4Atom *mehdAtom = moovAtom->subelementByPath(diag, Mp4AtomIds::MovieExtends, Mp4AtomIds::MovieExtendsHeader)) {
                m_fragmented = true;
                if (mehdAtom->dataSize() > 0) {
                    stream().seekg(static_cast<iostream::off_type>(mehdAtom->dataOffset()));
                    unsigned int durationSize = reader().readByte() == 1u ? 8u : 4u; // duration size depends on atom version
                    if (mehdAtom->dataSize() >= 4 + durationSize) {
                        stream().seekg(3, ios_base::cur); // skip flags
                        switch (durationSize) {
                        case 4u:
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt32BE()) / static_cast<double>(m_timeScale));
                            break;
                        case 8u:
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt64BE()) / static_cast<double>(m_timeScale));
                            break;
                        default:;
                        }
                    } else {
                        diag.emplace_back(DiagLevel::Warning, "mehd atom is truncated.", context);
                    }
                }
            }
            // get first trak atoms which hold information for each track
            Mp4Atom *trakAtom = moovAtom->childById(Mp4AtomIds::Track, diag);
            int trackNum = 1;
            while (trakAtom) {
                try {
                    trakAtom->parse(diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Warning, "Unable to parse child atom of moov.", context);
                }
                // parse the trak atom using the Mp4Track class
                m_tracks.emplace_back(make_unique<Mp4Track>(*trakAtom));
                try { // try to parse header
                    m_tracks.back()->parseHeader(diag);
                } catch (const Failure &) {
                    diag.emplace_back(DiagLevel::Critical, argsToString("Unable to parse track ", trackNum, '.'), context);
                }
                trakAtom = trakAtom->siblingById(Mp4AtomIds::Track, diag); // get next trak atom
                ++trackNum;
            }
            // get overall duration, creation time and modification time if not determined yet
            if (m_duration.isNull() || m_modificationTime.isNull() || m_creationTime.isNull()) {
                for (const auto &track : tracks()) {
                    if (track->duration() > m_duration) {
                        m_duration = track->duration();
                    }
                    if (track->modificationTime() > m_modificationTime) {
                        m_modificationTime = track->modificationTime();
                    }
                    if (track->creationTime() < m_creationTime) {
                        m_creationTime = track->creationTime();
                    }
                }
            }
        }
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Warning, "Unable to parse moov atom.", context);
    }
}

void Mp4Container::internalMakeFile(Diagnostics &diag, AbortableProgressFeedback &progress)
{
    static const string context("making MP4 container");
    progress.updateStep("Calculating atom sizes and padding ...");

    // basic validation of original file
    if (!isHeaderParsed()) {
        diag.emplace_back(DiagLevel::Critical, "The header has not been parsed yet.", context);
        throw InvalidDataException();
    }

    // define variables needed to parse atoms of original file
    if (!firstElement()) {
        diag.emplace_back(DiagLevel::Critical, "No MP4 atoms could be found.", context);
        throw InvalidDataException();
    }

    // define variables needed to manage file layout
    // -> whether media data is written chunk by chunk (need to write chunk by chunk if tracks have been altered)
    const bool writeChunkByChunk = m_tracksAltered;
    // -> whether rewrite is required (always required when forced to rewrite or when tracks have been altered)
    bool rewriteRequired = fileInfo().isForcingRewrite() || writeChunkByChunk;
    // -> use the preferred tag position/index position (force one wins, if both are force tag pos wins; might be changed later if none is forced)
    ElementPosition initialNewTagPos
        = fileInfo().forceTagPosition() || !fileInfo().forceIndexPosition() ? fileInfo().tagPosition() : fileInfo().indexPosition();
    ElementPosition newTagPos = initialNewTagPos;
    // -> current tag position (determined later)
    ElementPosition currentTagPos;
    // -> holds new padding (before actual data)
    uint64 newPadding;
    // -> holds new padding (after actual data)
    uint64 newPaddingEnd;
    // -> holds current offset
    uint64 currentOffset;
    // -> holds track information, used when writing chunk-by-chunk
    vector<tuple<istream *, vector<uint64>, vector<uint64>>> trackInfos;
    // -> holds offsets of media data atoms in original file, used when simply copying mdat
    vector<int64> origMediaDataOffsets;
    // -> holds offsets of media data atoms in new file, used when simply copying mdat
    vector<int64> newMediaDataOffsets;
    // -> new size of movie atom and user data atom
    uint64 movieAtomSize, userDataAtomSize;
    // -> track count of original file
    const auto trackCount = this->trackCount();

    // find relevant atoms in original file
    Mp4Atom *fileTypeAtom, *progressiveDownloadInfoAtom, *movieAtom, *firstMediaDataAtom, *firstMovieFragmentAtom /*, *userDataAtom*/;
    Mp4Atom *level0Atom, *level1Atom, *level2Atom, *lastAtomToBeWritten;
    try {
        // file type atom (mandatory)
        if ((fileTypeAtom = firstElement()->siblingByIdIncludingThis(Mp4AtomIds::FileType, diag))) {
            // buffer atom
            fileTypeAtom->makeBuffer();
        } else {
            // throw error if missing
            diag.emplace_back(DiagLevel::Critical, "Mandatory \"ftyp\"-atom not found.", context);
            throw InvalidDataException();
        }

        // progressive download information atom (not mandatory)
        if ((progressiveDownloadInfoAtom = firstElement()->siblingByIdIncludingThis(Mp4AtomIds::ProgressiveDownloadInformation, diag))) {
            // buffer atom
            progressiveDownloadInfoAtom->makeBuffer();
        }

        // movie atom (mandatory)
        if (!(movieAtom = firstElement()->siblingByIdIncludingThis(Mp4AtomIds::Movie, diag))) {
            // throw error if missing
            diag.emplace_back(DiagLevel::Critical, "Mandatory \"moov\"-atom not in the source file found.", context);
            throw InvalidDataException();
        }

        // movie fragment atom (indicates dash file)
        if ((firstMovieFragmentAtom = firstElement()->siblingById(Mp4AtomIds::MovieFragment, diag))) {
            // there is at least one movie fragment atom -> consider file being dash
            // -> can not write chunk-by-chunk (currently)
            if (writeChunkByChunk) {
                diag.emplace_back(DiagLevel::Critical, "Writing chunk-by-chunk is not implemented for DASH files.", context);
                throw NotImplementedException();
            }
            // -> tags must be placed at the beginning
            newTagPos = ElementPosition::BeforeData;
        }

        // media data atom (mandatory?)
        // -> consider not only mdat as media data atom; consider everything not handled otherwise as media data
        for (firstMediaDataAtom = nullptr, level0Atom = firstElement(); level0Atom; level0Atom = level0Atom->nextSibling()) {
            level0Atom->parse(diag);
            switch (level0Atom->id()) {
            case Mp4AtomIds::FileType:
            case Mp4AtomIds::ProgressiveDownloadInformation:
            case Mp4AtomIds::Movie:
            case Mp4AtomIds::Free:
            case Mp4AtomIds::Skip:
                continue;
            default:
                firstMediaDataAtom = level0Atom;
            }
            break;
        }

        // determine current tag position
        // -> since tags are nested in the movie atom its position is relevant here
        if (firstMediaDataAtom) {
            currentTagPos = firstMediaDataAtom->startOffset() < movieAtom->startOffset() ? ElementPosition::AfterData : ElementPosition::BeforeData;
            if (newTagPos == ElementPosition::Keep) {
                newTagPos = currentTagPos;
            }
        } else {
            currentTagPos = ElementPosition::Keep;
        }

        // ensure index and tags are always placed at the beginning when dealing with DASH files
        if (firstMovieFragmentAtom) {
            if (initialNewTagPos == ElementPosition::AfterData) {
                diag.emplace_back(
                    DiagLevel::Warning, "Sorry, but putting index/tags at the end is not possible when dealing with DASH files.", context);
            }
            initialNewTagPos = newTagPos = ElementPosition::BeforeData;
        }

        // user data atom (currently not used)
        //userDataAtom = movieAtom->childById(Mp4AtomIds::UserData);

    } catch (const NotImplementedException &) {
        throw;

    } catch (const Failure &) {
        // can't ignore parsing errors here
        diag.emplace_back(DiagLevel::Critical, "Unable to parse the overall atom structure of the source file.", context);
        throw InvalidDataException();
    }

    progress.stopIfAborted();

    // calculate sizes
    // -> size of tags
    vector<Mp4TagMaker> tagMaker;
    uint64 tagsSize = 0;
    tagMaker.reserve(m_tags.size());
    for (auto &tag : m_tags) {
        try {
            tagMaker.emplace_back(tag->prepareMaking(diag));
            tagsSize += tagMaker.back().requiredSize();
        } catch (const Failure &) {
        }
    }

    // -> size of movie atom (contains track and tag information)
    movieAtomSize = userDataAtomSize = 0;
    try {
        // add size of children
        for (level0Atom = movieAtom; level0Atom; level0Atom = level0Atom->siblingById(Mp4AtomIds::Movie, diag)) {
            for (level1Atom = level0Atom->firstChild(); level1Atom; level1Atom = level1Atom->nextSibling()) {
                level1Atom->parse(diag);
                switch (level1Atom->id()) {
                case Mp4AtomIds::UserData:
                    try {
                        for (level2Atom = level1Atom->firstChild(); level2Atom; level2Atom = level2Atom->nextSibling()) {
                            level2Atom->parse(diag);
                            switch (level2Atom->id()) {
                            case Mp4AtomIds::Meta:
                                // ignore meta data here; it is added separately
                                break;
                            default:
                                // add size of unknown childs of the user data atom
                                userDataAtomSize += level2Atom->totalSize();
                                level2Atom->makeBuffer();
                            }
                        }
                    } catch (const Failure &) {
                        // invalid children might be ignored as not mandatory
                        diag.emplace_back(
                            DiagLevel::Critical, "Unable to parse the children of \"udta\"-atom of the source file; ignoring them.", context);
                    }
                    break;
                case Mp4AtomIds::Track:
                    // ignore track atoms here; they are added separately
                    break;
                default:
                    // add size of unknown childs of the movie atom
                    movieAtomSize += level1Atom->totalSize();
                    level1Atom->makeBuffer();
                }
            }
        }

        // add size of meta data
        if (userDataAtomSize += tagsSize) {
            Mp4Atom::addHeaderSize(userDataAtomSize);
            movieAtomSize += userDataAtomSize;
        }

        // add size of track atoms
        for (const auto &track : tracks()) {
            movieAtomSize += track->requiredSize(diag);
        }

        // add header size
        Mp4Atom::addHeaderSize(movieAtomSize);
    } catch (const Failure &) {
        // can't ignore parsing errors here
        diag.emplace_back(DiagLevel::Critical, "Unable to parse the children of \"moov\"-atom of the source file.", context);
        throw InvalidDataException();
    }

    progress.stopIfAborted();

    // check whether there are atoms to be voided after movie next sibling (only relevant when not rewriting)
    if (!rewriteRequired) {
        newPaddingEnd = 0;
        uint64 currentSum = 0;
        for (Mp4Atom *level0Atom = firstMediaDataAtom; level0Atom; level0Atom = level0Atom->nextSibling()) {
            level0Atom->parse(diag);
            switch (level0Atom->id()) {
            case Mp4AtomIds::FileType:
            case Mp4AtomIds::ProgressiveDownloadInformation:
            case Mp4AtomIds::Movie:
            case Mp4AtomIds::Free:
            case Mp4AtomIds::Skip:
                // must void these if they occur "between" the media data
                currentSum += level0Atom->totalSize();
                break;
            default:
                newPaddingEnd += currentSum;
                currentSum = 0;
                lastAtomToBeWritten = level0Atom;
            }
        }
    }

    // calculate padding if no rewrite is required; otherwise use the preferred padding
calculatePadding:
    if (rewriteRequired) {
        newPadding = (fileInfo().preferredPadding() && fileInfo().preferredPadding() < 8 ? 8 : fileInfo().preferredPadding());
    } else {
        // file type atom
        currentOffset = fileTypeAtom->totalSize();

        // progressive download information atom
        if (progressiveDownloadInfoAtom) {
            currentOffset += progressiveDownloadInfoAtom->totalSize();
        }

        // if writing tags before data: movie atom (contains tag)
        switch (newTagPos) {
        case ElementPosition::BeforeData:
        case ElementPosition::Keep:
            currentOffset += movieAtomSize;
            break;
        default:;
        }

        // check whether there is sufficiant space before the next atom
        if (!(rewriteRequired = firstMediaDataAtom && currentOffset > firstMediaDataAtom->startOffset())) {
            // there is sufficiant space
            // -> check whether the padding matches specifications
            //    min padding: says "at least ... byte should be reserved to prepend further tag info", so the padding at the end
            //                 shouldn't be tanken into account (it can't be used to prepend further tag info)
            //    max padding: says "do not waste more than ... byte", so here all padding should be taken into account
            newPadding = firstMediaDataAtom->startOffset() - currentOffset;
            rewriteRequired = (newPadding > 0 && newPadding < 8) || newPadding < fileInfo().minPadding()
                || (newPadding + newPaddingEnd) > fileInfo().maxPadding();
        }
        if (rewriteRequired) {
            // can't put the tags before media data
            if (!firstMovieFragmentAtom && !fileInfo().forceTagPosition() && !fileInfo().forceIndexPosition()
                && newTagPos != ElementPosition::AfterData) {
                // writing tag before media data is not forced, its not a DASH file and tags aren't already at the end
                // -> try to put the tags at the end
                newTagPos = ElementPosition::AfterData;
                rewriteRequired = false;
            } else {
                // writing tag before media data is forced -> rewrite the file
                // when rewriting anyways, ensure the preferred tag position is used
                newTagPos = initialNewTagPos == ElementPosition::Keep ? currentTagPos : initialNewTagPos;
            }
            // in any case: recalculate padding
            goto calculatePadding;
        } else {
            // tags can be put before the media data
            // -> ensure newTagPos is not ElementPosition::Keep
            if (newTagPos == ElementPosition::Keep) {
                newTagPos = ElementPosition::BeforeData;
            }
        }
    }

    // setup stream(s) for writing
    // -> update status
    progress.nextStepOrStop("Preparing streams ...");

    // -> define variables needed to handle output stream and backup stream (required when rewriting the file)
    string backupPath;
    NativeFileStream &outputStream = fileInfo().stream();
    NativeFileStream backupStream; // create a stream to open the backup/original file for the case rewriting the file is required
    BinaryWriter outputWriter(&outputStream);

    if (rewriteRequired) {
        if (fileInfo().saveFilePath().empty()) {
            // move current file to temp dir and reopen it as backupStream, recreate original file
            try {
                BackupHelper::createBackupFile(fileInfo().backupDirectory(), fileInfo().path(), backupPath, outputStream, backupStream);
                // recreate original file, define buffer variables
                outputStream.open(fileInfo().path(), ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (...) {
                const char *what = catchIoFailure();
                diag.emplace_back(DiagLevel::Critical, "Creation of temporary file (to rewrite the original file) failed.", context);
                throwIoFailure(what);
            }
        } else {
            // open the current file as backupStream and create a new outputStream at the specified "save file path"
            try {
                backupStream.exceptions(ios_base::badbit | ios_base::failbit);
                backupStream.open(fileInfo().path(), ios_base::in | ios_base::binary);
                fileInfo().close();
                outputStream.open(fileInfo().saveFilePath(), ios_base::out | ios_base::binary | ios_base::trunc);
            } catch (...) {
                const char *what = catchIoFailure();
                diag.emplace_back(DiagLevel::Critical, "Opening streams to write output file failed.", context);
                throwIoFailure(what);
            }
        }

        // set backup stream as associated input stream since we need the original elements to write the new file
        setStream(backupStream);

        // TODO: reduce code duplication

    } else { // !rewriteRequired
        // ensure everything to make track atoms is buffered before altering the source file
        for (const auto &track : tracks()) {
            track->bufferTrackAtoms(diag);
        }

        // reopen original file to ensure it is opened for writing
        try {
            fileInfo().close();
            outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
        } catch (...) {
            const char *what = catchIoFailure();
            diag.emplace_back(DiagLevel::Critical, "Opening the file with write permissions failed.", context);
            throwIoFailure(what);
        }
    }

    // start actual writing
    try {
        // write header
        progress.nextStepOrStop("Writing header and tags ...");
        // -> make file type atom
        fileTypeAtom->copyBuffer(outputStream);
        fileTypeAtom->discardBuffer();
        // -> make progressive download info atom
        if (progressiveDownloadInfoAtom) {
            progressiveDownloadInfoAtom->copyBuffer(outputStream);
            progressiveDownloadInfoAtom->discardBuffer();
        }

        // set input/output streams of each track
        for (auto &track : tracks()) {
            // ensure the track reads from the original file
            if (&track->inputStream() == &outputStream) {
                track->setInputStream(backupStream);
            }
            // ensure the track writes to the output file
            track->setOutputStream(outputStream);
        }

        // write movie atom / padding and media data
        for (byte pass = 0; pass != 2; ++pass) {
            if (newTagPos == (pass ? ElementPosition::AfterData : ElementPosition::BeforeData)) {
                // write movie atom
                // -> write movie atom header
                Mp4Atom::makeHeader(movieAtomSize, Mp4AtomIds::Movie, outputWriter);

                // -> write track atoms
                for (auto &track : tracks()) {
                    track->makeTrack(diag);
                }

                // -> write other movie atom children
                for (level0Atom = movieAtom; level0Atom; level0Atom = level0Atom->siblingById(Mp4AtomIds::Movie, diag)) {
                    for (level1Atom = level0Atom->firstChild(); level1Atom; level1Atom = level1Atom->nextSibling()) {
                        switch (level1Atom->id()) {
                        case Mp4AtomIds::UserData:
                        case Mp4AtomIds::Track:
                            // track and user data atoms are written separately
                            break;
                        default:
                            // write buffered data
                            level1Atom->copyBuffer(outputStream);
                            level1Atom->discardBuffer();
                        }
                    }
                }

                // -> write user data atom
                if (userDataAtomSize) {
                    // writer user data atom header
                    Mp4Atom::makeHeader(userDataAtomSize, Mp4AtomIds::UserData, outputWriter);

                    // write other children of user data atom
                    for (level0Atom = movieAtom; level0Atom; level0Atom = level0Atom->siblingById(Mp4AtomIds::Movie, diag)) {
                        for (level1Atom = level0Atom->childById(Mp4AtomIds::UserData, diag); level1Atom;
                             level1Atom = level1Atom->siblingById(Mp4AtomIds::UserData, diag)) {
                            for (level2Atom = level1Atom->firstChild(); level2Atom; level2Atom = level2Atom->nextSibling()) {
                                switch (level2Atom->id()) {
                                case Mp4AtomIds::Meta:
                                    break;
                                default:
                                    // write buffered data
                                    level2Atom->copyBuffer(outputStream);
                                    level2Atom->discardBuffer();
                                }
                            }
                        }
                    }

                    // write meta atom
                    for (auto &maker : tagMaker) {
                        maker.make(outputStream, diag);
                    }
                }

            } else {
                // write padding
                if (newPadding) {
                    // write free atom header
                    if (newPadding < numeric_limits<uint32>::max()) {
                        outputWriter.writeUInt32BE(static_cast<uint32>(newPadding));
                        outputWriter.writeUInt32BE(Mp4AtomIds::Free);
                        newPadding -= 8;
                    } else {
                        outputWriter.writeUInt32BE(1);
                        outputWriter.writeUInt32BE(Mp4AtomIds::Free);
                        outputWriter.writeUInt64BE(newPadding);
                        newPadding -= 16;
                    }

                    // write zeroes
                    for (; newPadding; --newPadding) {
                        outputStream.put(0);
                    }
                }

                // write media data
                if (rewriteRequired) {
                    for (level0Atom = firstMediaDataAtom; level0Atom; level0Atom = level0Atom->nextSibling()) {
                        level0Atom->parse(diag);
                        switch (level0Atom->id()) {
                        case Mp4AtomIds::FileType:
                        case Mp4AtomIds::ProgressiveDownloadInformation:
                        case Mp4AtomIds::Movie:
                        case Mp4AtomIds::Free:
                        case Mp4AtomIds::Skip:
                            break;
                        case Mp4AtomIds::MediaData:
                            if (writeChunkByChunk) {
                                // write actual data separately when writing chunk-by-chunk
                                break;
                            } else {
                                // store media data offsets when not writing chunk-by-chunk to be able to update chunk offset table
                                origMediaDataOffsets.push_back(static_cast<int64>(level0Atom->startOffset()));
                                newMediaDataOffsets.push_back(outputStream.tellp());
                            }
                            FALLTHROUGH;
                        default:
                            // update status
                            progress.updateStep("Writing atom: " + level0Atom->idToString());
                            // copy atom entirely and forward status update calls
                            level0Atom->copyEntirely(outputStream, diag, &progress);
                        }
                    }

                    // when writing chunk-by-chunk write media data now
                    if (writeChunkByChunk) {
                        // read chunk offset and chunk size table from the old file which are required to get chunks
                        progress.updateStep("Reading chunk offsets and sizes from the original file ...");
                        trackInfos.reserve(trackCount);
                        uint64 totalChunkCount = 0;
                        uint64 totalMediaDataSize = 0;
                        for (auto &track : tracks()) {
                            progress.stopIfAborted();

                            // emplace information
                            trackInfos.emplace_back(
                                &track->inputStream(), track->readChunkOffsets(fileInfo().isForcingFullParse(), diag), track->readChunkSizes(diag));

                            // check whether the chunks could be parsed correctly
                            const vector<uint64> &chunkOffsetTable = get<1>(trackInfos.back());
                            const vector<uint64> &chunkSizesTable = get<2>(trackInfos.back());
                            if (track->chunkCount() != chunkOffsetTable.size() || track->chunkCount() != chunkSizesTable.size()) {
                                diag.emplace_back(DiagLevel::Critical,
                                    "Chunks of track " % numberToString<uint64, string>(track->id()) + " could not be parsed correctly.", context);
                            }

                            // increase total chunk count and size
                            totalChunkCount += track->chunkCount();
                            totalMediaDataSize += accumulate(chunkSizesTable.cbegin(), chunkSizesTable.cend(), 0ul);
                        }

                        // write media data chunk-by-chunk
                        // -> write header of media data atom
                        Mp4Atom::addHeaderSize(totalMediaDataSize);
                        Mp4Atom::makeHeader(totalMediaDataSize, Mp4AtomIds::MediaData, outputWriter);

                        // -> copy chunks
                        CopyHelper<0x2000> copyHelper;
                        uint64 chunkIndexWithinTrack = 0, totalChunksCopied = 0;
                        bool anyChunksCopied;
                        do {
                            progress.stopIfAborted();

                            // copy a chunk from each track
                            anyChunksCopied = false;
                            for (size_t trackIndex = 0; trackIndex < trackCount; ++trackIndex) {
                                // get source stream and tables for current track
                                auto &trackInfo = trackInfos[trackIndex];
                                istream &sourceStream = *get<0>(trackInfo);
                                vector<uint64> &chunkOffsetTable = get<1>(trackInfo);
                                const vector<uint64> &chunkSizesTable = get<2>(trackInfo);

                                // still chunks to be copied (of this track)?
                                if (chunkIndexWithinTrack < chunkOffsetTable.size() && chunkIndexWithinTrack < chunkSizesTable.size()) {
                                    // copy chunk, update entry in chunk offset table
                                    sourceStream.seekg(static_cast<streamoff>(chunkOffsetTable[chunkIndexWithinTrack]));
                                    chunkOffsetTable[chunkIndexWithinTrack] = static_cast<uint64>(outputStream.tellp());
                                    copyHelper.copy(sourceStream, outputStream, chunkSizesTable[chunkIndexWithinTrack]);

                                    // update counter / status
                                    anyChunksCopied = true;
                                    ++totalChunksCopied;
                                }
                            }

                            // incrase chunk index within track, update progress percentage
                            if (!(++chunkIndexWithinTrack % 10)) {
                                progress.updateStepPercentage(static_cast<byte>(totalChunksCopied * 100 / totalChunkCount));
                            }

                        } while (anyChunksCopied);
                    }

                } else {
                    // can't just skip next movie sibling
                    for (Mp4Atom *level0Atom = firstMediaDataAtom; level0Atom; level0Atom = level0Atom->nextSibling()) {
                        level0Atom->parse(diag);
                        switch (level0Atom->id()) {
                        case Mp4AtomIds::FileType:
                        case Mp4AtomIds::ProgressiveDownloadInformation:
                        case Mp4AtomIds::Movie:
                            // must void these if they occur "between" the media data
                            outputStream.seekp(4, ios_base::cur);
                            outputWriter.writeUInt32BE(Mp4AtomIds::Free);
                            break;
                        default:
                            outputStream.seekp(static_cast<iostream::off_type>(level0Atom->totalSize()), ios_base::cur);
                        }
                        if (level0Atom == lastAtomToBeWritten) {
                            break;
                        }
                    }
                }
            }
        }

        // reparse what is written so far
        progress.updateStep("Reparsing output file ...");
        if (rewriteRequired) {
            // report new size
            fileInfo().reportSizeChanged(static_cast<uint64>(outputStream.tellp()));
            // "save as path" is now the regular path
            if (!fileInfo().saveFilePath().empty()) {
                fileInfo().reportPathChanged(fileInfo().saveFilePath());
                fileInfo().setSaveFilePath(string());
            }
            // the outputStream needs to be reopened to be able to read again
            outputStream.close();
            outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
            setStream(outputStream);
        } else {
            const auto newSize = static_cast<uint64>(outputStream.tellp());
            if (newSize < fileInfo().size()) {
                // file is smaller after the modification -> truncate
                // -> close stream before truncating
                outputStream.close();
                // -> truncate file
                if (truncate(fileInfo().path().c_str(), static_cast<iostream::off_type>(newSize)) == 0) {
                    fileInfo().reportSizeChanged(newSize);
                } else {
                    diag.emplace_back(DiagLevel::Critical, "Unable to truncate the file.", context);
                }
                // -> reopen the stream again
                outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
            } else {
                // file is longer after the modification -> just report new size
                fileInfo().reportSizeChanged(newSize);
            }
        }

        reset();
        try {
            parseTracks(diag);
        } catch (const Failure &) {
            diag.emplace_back(DiagLevel::Critical, "Unable to reparse the new file.", context);
            throw;
        }

        if (rewriteRequired) {
            // check whether track count of new file equals track count of old file
            if (trackCount != tracks().size()) {
                diag.emplace_back(DiagLevel::Critical,
                    argsToString("Unable to update chunk offsets (\"stco\"-atom): Number of tracks in the output file (", tracks().size(),
                        ") differs from the number of tracks in the original file (", trackCount, ")."),
                    context);
                throw Failure();
            }

            // update chunk offset table
            if (writeChunkByChunk) {
                progress.updateStep("Updating chunk offset table for each track ...");
                for (size_t trackIndex = 0; trackIndex != trackCount; ++trackIndex) {
                    const auto &track = tracks()[trackIndex];
                    const auto &chunkOffsetTable = get<1>(trackInfos[trackIndex]);
                    if (track->chunkCount() == chunkOffsetTable.size()) {
                        track->updateChunkOffsets(chunkOffsetTable);
                    } else {
                        diag.emplace_back(DiagLevel::Critical,
                            argsToString("Unable to update chunk offsets of track ", (trackIndex + 1),
                                ": Number of chunks in the output file differs from the number of chunks in the orignal file."),
                            context);
                        throw Failure();
                    }
                }
            } else {
                progress.updateStep("Updating chunk offset table for each track ...");
                updateOffsets(origMediaDataOffsets, newMediaDataOffsets, diag);
            }
        }

        // flush output stream
        outputStream.flush();

        // handle errors (which might have been occurred after renaming/creating backup file)
    } catch (...) {
        BackupHelper::handleFailureAfterFileModified(fileInfo(), backupPath, outputStream, backupStream, diag, context);
    }
}

/*!
 * \brief Update the chunk offsets for each track of the file.
 * \param oldMdatOffsets Specifies a vector holding the old offsets of the "mdat"-atoms.
 * \param newMdatOffsets Specifies a vector holding the new offsets of the "mdat"-atoms.
 *
 * Uses internally Mp4Track::updateOffsets(). Offsets stored in the "tfhd"-atom are also
 * updated (this is not tested yet since I don't have files using this atom).
 *
 * \throws Throws std::ios_base::failure when an IO error occurs.
 * \throws Throws TagParser::Failure or a derived exception when a making
 *                error occurs.
 */
void Mp4Container::updateOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets, Diagnostics &diag)
{
    // do NOT invalidate the status here since this method is internally called by internalMakeFile(), just update the status
    const string context("updating MP4 container chunk offset table");
    if (!firstElement()) {
        diag.emplace_back(DiagLevel::Critical, "No MP4 atoms could be found.", context);
        throw InvalidDataException();
    }
    // update "base-data-offset-present" of "tfhd"-atom (NOT tested properly)
    try {
        for (Mp4Atom *moofAtom = firstElement()->siblingById(Mp4AtomIds::MovieFragment, diag); moofAtom;
             moofAtom = moofAtom->siblingById(Mp4AtomIds::MovieFragment, diag)) {
            moofAtom->parse(diag);
            try {
                for (Mp4Atom *trafAtom = moofAtom->childById(Mp4AtomIds::TrackFragment, diag); trafAtom;
                     trafAtom = trafAtom->siblingById(Mp4AtomIds::TrackFragment, diag)) {
                    trafAtom->parse(diag);
                    int tfhdAtomCount = 0;
                    for (Mp4Atom *tfhdAtom = trafAtom->childById(Mp4AtomIds::TrackFragmentHeader, diag); tfhdAtom;
                         tfhdAtom = tfhdAtom->siblingById(Mp4AtomIds::TrackFragmentHeader, diag)) {
                        tfhdAtom->parse(diag);
                        ++tfhdAtomCount;
                        if (tfhdAtom->dataSize() < 8) {
                            diag.emplace_back(DiagLevel::Warning, "tfhd atom is truncated.", context);
                            continue;
                        }
                        stream().seekg(static_cast<iostream::off_type>(tfhdAtom->dataOffset()) + 1);
                        uint32 flags = reader().readUInt24BE();
                        if (!(flags & 1)) {
                            continue;
                        }
                        if (tfhdAtom->dataSize() < 16) {
                            diag.emplace_back(DiagLevel::Warning, "tfhd atom (denoting base-data-offset-present) is truncated.", context);
                            continue;
                        }
                        stream().seekg(4, ios_base::cur); // skip track ID
                        uint64 off = reader().readUInt64BE();
                        for (auto iOld = oldMdatOffsets.cbegin(), iNew = newMdatOffsets.cbegin(), end = oldMdatOffsets.cend(); iOld != end;
                             ++iOld, ++iNew) {
                            if (off < static_cast<uint64>(*iOld)) {
                                continue;
                            }
                            off += static_cast<uint64>(*iNew - *iOld);
                            stream().seekp(static_cast<iostream::off_type>(tfhdAtom->dataOffset()) + 8);
                            writer().writeUInt64BE(off);
                            break;
                        }
                    }
                    switch (tfhdAtomCount) {
                    case 0:
                        diag.emplace_back(DiagLevel::Warning, "traf atom doesn't contain mandatory tfhd atom.", context);
                        break;
                    case 1:
                        break;
                    default:
                        diag.emplace_back(
                            DiagLevel::Warning, "traf atom stores multiple tfhd atoms but it should only contain exactly one tfhd atom.", context);
                    }
                }
            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Critical, "Unable to parse childs of top-level atom moof.", context);
            }
        }
    } catch (const Failure &) {
        diag.emplace_back(DiagLevel::Critical, "Unable to parse top-level atom moof.", context);
    }
    // update each track
    for (auto &track : tracks()) {
        if (!track->isHeaderValid()) {
            try {
                track->parseHeader(diag);
            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Warning,
                    "The chunk offsets of track " % track->name() + " couldn't be updated because the track seems to be invalid..", context);
                throw;
            }
        }
        if (track->isHeaderValid()) {
            try {
                track->updateChunkOffsets(oldMdatOffsets, newMdatOffsets);
            } catch (const Failure &) {
                diag.emplace_back(DiagLevel::Warning, "The chunk offsets of track " % track->name() + " couldn't be updated.", context);
                throw;
            }
        }
    }
}

} // namespace TagParser
