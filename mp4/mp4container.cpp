#include "./mp4container.h"
#include "./mp4ids.h"

#include "../exceptions.h"
#include "../mediafileinfo.h"
#include "../backuphelper.h"

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>
#include <c++utilities/io/copy.h>
#include <c++utilities/misc/memory.h>

#include <tuple>

using namespace std;
using namespace IoUtilities;
using namespace ConversionUtilities;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \class Media::Mp4Container
 * \brief Implementation of GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>.
 */

/*!
 * \brief Constructs a new container for the specified \a fileInfo at the specified \a startOffset.
 */
Mp4Container::Mp4Container(MediaFileInfo &fileInfo, uint64 startOffset) :
    GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>(fileInfo, startOffset),
    m_fragmented(false)
{}

/*!
 * \brief Destroys the container.
 */
Mp4Container::~Mp4Container()
{}

void Mp4Container::reset()
{
    GenericContainer<MediaFileInfo, Mp4Tag, Mp4Track, Mp4Atom>::reset();
    m_fragmented = false;
}

void Mp4Container::internalParseHeader()
{
    //const string context("parsing header of MP4 container"); will be used when generating notifications
    m_firstElement = make_unique<Mp4Atom>(*this, startOffset());
    m_firstElement->parse();
    Mp4Atom *ftypAtom = m_firstElement->siblingById(Mp4AtomIds::FileType, true);
    if(ftypAtom) {
        stream().seekg(ftypAtom->dataOffset());
        m_doctype = reader().readString(4);
        m_version = reader().readUInt32BE();
    } else {
        m_doctype = "mp41";
        m_version = 0;
    }
}

void Mp4Container::internalParseTags()
{
    const string context("parsing tags of MP4 container");
    if(Mp4Atom *udtaAtom = firstElement()->subelementByPath({Mp4AtomIds::Movie, Mp4AtomIds::UserData})) {
        Mp4Atom *metaAtom = udtaAtom->childById(Mp4AtomIds::Meta);
        bool surplusMetaAtoms = false;
        while(metaAtom) {
            metaAtom->parse();
            m_tags.emplace_back(make_unique<Mp4Tag>());
            try {
                m_tags.back()->parse(*metaAtom);
            } catch(NoDataFoundException &) {
                m_tags.pop_back();
            }
            metaAtom = metaAtom->siblingById(Mp4AtomIds::Meta, false);
            if(metaAtom) {
                surplusMetaAtoms = true;
            }
            if(!m_tags.empty()) {
                break;
            }
        }
        if(surplusMetaAtoms) {
            addNotification(NotificationType::Warning, "udta atom contains multiple meta atoms. Surplus meta atoms will be ignored.", context);
        }
    }
}

void Mp4Container::internalParseTracks()
{
    invalidateStatus();
    static const string context("parsing tracks of MP4 container");
    try {
        // get moov atom which holds track information
        if(Mp4Atom *moovAtom = firstElement()->siblingById(Mp4AtomIds::Movie, true)) {
            // get mvhd atom which holds overall track information
            if(Mp4Atom *mvhdAtom = moovAtom->childById(Mp4AtomIds::MovieHeader)) {
                if(mvhdAtom->dataSize() > 0) {
                    stream().seekg(mvhdAtom->dataOffset());
                    byte version = reader().readByte();
                    if((version == 1 && mvhdAtom->dataSize() >= 32) || (mvhdAtom->dataSize() >= 20)) {
                        stream().seekg(3, ios_base::cur); // skip flags
                        switch(version) {
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
                        default:
                            ;
                        }
                    } else {
                        addNotification(NotificationType::Critical, "mvhd atom is truncated.", context);
                    }
                } else {
                    addNotification(NotificationType::Critical, "mvhd atom is empty.", context);
                }
            } else {
                addNotification(NotificationType::Critical, "mvhd atom is does not exist.", context);
            }
            // get mvex atom which holds default values for fragmented files
            if(Mp4Atom *mehdAtom = moovAtom->subelementByPath({Mp4AtomIds::MovieExtends, Mp4AtomIds::MovieExtendsHeader})) {
                m_fragmented = true;
                if(mehdAtom->dataSize() > 0) {
                    stream().seekg(mehdAtom->dataOffset());
                    unsigned int durationSize = reader().readByte() == 1u ? 8u : 4u; // duration size depends on atom version
                    if(mehdAtom->dataSize() >= 4 + durationSize) {
                        stream().seekg(3, ios_base::cur); // skip flags
                        switch(durationSize) {
                        case 4u:
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt32BE()) / static_cast<double>(m_timeScale));
                            break;
                        case 8u:
                            m_duration = TimeSpan::fromSeconds(static_cast<double>(reader().readUInt64BE()) / static_cast<double>(m_timeScale));
                            break;
                        default:
                            ;
                        }
                    } else {
                        addNotification(NotificationType::Warning, "mehd atom is truncated.", context);
                    }
                }
            }
            // get first trak atoms which hold information for each track
            Mp4Atom *trakAtom = moovAtom->childById(Mp4AtomIds::Track);
            int trackNum = 1;
            while(trakAtom) {
                try {
                    trakAtom->parse();
                } catch(Failure &) {
                    addNotification(NotificationType::Warning, "Unable to parse child atom of moov.", context);
                }
                // parse the trak atom using the Mp4Track class
                m_tracks.emplace_back(make_unique<Mp4Track>(*trakAtom));
                try { // try to parse header
                    m_tracks.back()->parseHeader();
                } catch(Failure &) {
                    addNotification(NotificationType::Critical, "Unable to parse track " + ConversionUtilities::numberToString(trackNum) + ".", context);
                }
                trakAtom = trakAtom->siblingById(Mp4AtomIds::Track, false); // get next trak atom
                ++trackNum;
            }
            // get overall duration, creation time and modification time if not determined yet
            if(m_duration.isNull() || m_modificationTime.isNull() || m_creationTime.isNull()) {
                for(const auto &track : tracks()) {
                    if(track->duration() > m_duration) {
                        m_duration = track->duration();
                    }
                    if(track->modificationTime() > m_modificationTime) {
                        m_modificationTime = track->modificationTime();
                    }
                    if(track->creationTime() < m_creationTime) {
                        m_creationTime = track->creationTime();
                    }
                }
            }
        }
    } catch(Failure &) {
        addNotification(NotificationType::Warning, "Unable to parse moov atom.", context);
    }
}

void Mp4Container::internalMakeFile()
{
    invalidateStatus();
    static const string context("making MP4 container");
    if(!firstElement()) {
        addNotification(NotificationType::Critical, "No MP4 atoms could be found.", context);
        throw InvalidDataException();
    }
    // prepare rewriting the file
    bool writeChunkByChunk = m_tracksAltered;
    updateStatus("Preparing for rewriting MP4 file ...");
    fileInfo().close(); // ensure the file is close before renaming it
    string backupPath;
    fstream &outputStream = fileInfo().stream();
    BinaryWriter outputWriter(&outputStream);
    fstream backupStream; // create a stream to open the backup/original file
    try {
        BackupHelper::createBackupFile(fileInfo().path(), backupPath, backupStream);
        // set backup stream as associated input stream since we still the original data/atoms to write the new file
        setStream(backupStream);
        // recreate original file
        outputStream.open(fileInfo().path(), ios_base::out | ios_base::binary | ios_base::trunc);
        // collect needed atoms from the original file
        Mp4Atom *ftypAtom, *pdinAtom, *moovAtom;
        try {
            ftypAtom = firstElement()->siblingById(Mp4AtomIds::FileType, true); // mandatory
            if(!ftypAtom) { // throw error if missing
                addNotification(NotificationType::Critical, "Mandatory \"ftyp\"-atom not found.", context);
            }
            pdinAtom = firstElement()->siblingById(Mp4AtomIds::ProgressiveDownloadInformation, true); // not mandatory
            moovAtom = firstElement()->siblingById(Mp4AtomIds::Movie, true); // mandatory
            if(!moovAtom) { // throw error if missing
                addNotification(NotificationType::Critical, "Mandatory \"moov\"-atom not found.", context);
                throw InvalidDataException();
            }
        } catch (Failure &) {
            addNotification(NotificationType::Critical, "Unable to parse the atom strucutre of the source file.", context);
            throw InvalidDataException();
        }
        if(m_tags.size() > 1) {
            addNotification(NotificationType::Warning, "There are multiple MP4-tags assigned. Only the first one will be attached to the file.", context);
        }
        // write all top-level atoms
        updateStatus("Writing header ...");
        // write "ftype"-atom
        ftypAtom->copyEntirely(outputStream);
        // write "pdin"-atom ("progressive download information")
        if(pdinAtom) {
            pdinAtom->copyEntirely(outputStream);
        }
        // prepare for writing the actual data
        vector<tuple<istream *, vector<uint64>, vector<uint64> > > trackInfos; // used when writing chunk-by-chunk
        uint64 totalChunkCount = 0; // used when writing chunk-by-chunk
        vector<int64> origMdatOffsets; // used when simply copying mdat
        vector<int64> newMdatOffsets; // used when simply copying mdat
        auto trackCount = tracks().size();
        for(byte pass = 0; pass != 2; ++pass) {
            if(fileInfo().tagPosition() == (pass ? ElementPosition::AfterData : ElementPosition::BeforeData)) {
                // write "moov"-atom (contains track and tag information)
                ostream::pos_type newMoovOffset = outputStream.tellp();
                Mp4Atom *udtaAtom = nullptr;
                uint64 newUdtaOffset = 0u;
                // -> write child atoms manually, because the child "udta" has to be altered/ignored
                moovAtom->copyWithoutChilds(outputStream);
                for(Mp4Atom *moovChildAtom = moovAtom->firstChild(); moovChildAtom; moovChildAtom = moovChildAtom->nextSibling()) {
                    try {
                        moovChildAtom->parse();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse childs of moov atom of original file.", context);
                        throw InvalidDataException();
                    }
                    if(moovChildAtom->id() == Mp4AtomIds::UserData) {
                        // found a "udta" (user data) atom which child "meta" holds tag information
                        if(!udtaAtom) {
                            udtaAtom = moovChildAtom;
                            // check whether the "udta"-atom needs to be written
                            // it has to be written only when an MP4 tag is assigned
                            bool writeUdtaAtom = !m_tags.empty();
                            // or when there is at least one child except the meta atom in the original file
                            if(!writeUdtaAtom) {
                                try {
                                    for(Mp4Atom *udtaChildAtom = udtaAtom->firstChild(); udtaChildAtom; udtaChildAtom = udtaChildAtom->nextSibling()) {
                                        udtaChildAtom->parse();
                                        if(udtaChildAtom->id() != Mp4AtomIds::Meta) {
                                            writeUdtaAtom = true;
                                            break;
                                        }
                                    }
                                } catch(Failure &) {
                                    addNotification(NotificationType::Warning,
                                                    "Unable to parse childs of \"udta\"-atom atom of original file. These invalid/unknown atoms will be ignored.", context);
                                }
                            }
                            if(writeUdtaAtom) {
                                updateStatus("Writing tag information ...");
                                newUdtaOffset = outputStream.tellp(); // save offset
                                udtaAtom->copyHeader(outputStream); // and write header
                                // write meta atom if there's a tag assigned
                                if(!m_tags.empty()) {
                                    try {
                                        m_tags.front()->make(outputStream);
                                    } catch(Failure &) {
                                        addNotification(NotificationType::Warning, "Unable to write meta atom (of assigned mp4 tag).", context);
                                    }
                                    addNotifications(*m_tags.front());
                                }
                                // write rest of the child atoms of "udta"-atom
                                try {
                                    for(Mp4Atom *udtaChildAtom = udtaAtom->firstChild(); udtaChildAtom; udtaChildAtom = udtaChildAtom->nextSibling()) {
                                        udtaChildAtom->parse();
                                        // skip "meta"-atoms here of course
                                        if(udtaChildAtom->id() != Mp4AtomIds::Meta) {
                                            udtaChildAtom->copyEntirely(outputStream);
                                        }
                                    }
                                } catch(Failure &) {
                                    addNotification(NotificationType::Warning,
                                                          "Unable to parse childs of \"udta\"-atom of original file. These will be ignored.", context);
                                }
                                // write correct size of udta atom
                                Mp4Atom::seekBackAndWriteAtomSize(outputStream, newUdtaOffset);
                            }
                        } else {
                            addNotification(NotificationType::Warning, "The source file has multiple \"udta\"-atoms. Surplus atoms will be ignored.", context);
                        }
                    } else if(!writeChunkByChunk || moovChildAtom->id() != Mp4AtomIds::Track) {
                        // copy "trak"-atoms only when not writing the data chunk-by-chunk
                        moovChildAtom->copyEntirely(outputStream);
                    }
                }
                // -> the original file has no udta atom but there is tag information to be written
                if(!udtaAtom && !m_tags.empty()) {
                    updateStatus("Writing tag information ...");
                    newUdtaOffset = outputStream.tellp();
                    // write udta atom
                    outputWriter.writeUInt32BE(0); // the size will be written later
                    outputWriter.writeUInt32BE(Mp4AtomIds::UserData);
                    // write tags
                    try {
                        m_tags.front()->make(outputStream);
                        Mp4Atom::seekBackAndWriteAtomSize(outputStream, newUdtaOffset);
                    } catch(Failure &) {
                        addNotification(NotificationType::Warning, "Unable to write meta atom (of assigned mp4 tag).", context);
                    }
                }
                // -> write trak atoms for each currently assigned track (this is only required when writing data chunk-by-chunk)
                if(writeChunkByChunk) {
                    updateStatus("Writing meta information for the tracks ...");
                    for(auto &track : tracks()) {
                        track->setOutputStream(outputStream);
                        track->makeTrack();
                    }
                }
                Mp4Atom::seekBackAndWriteAtomSize(outputStream, newMoovOffset);
            } else {
                // write other atoms and "mdat"-atom (holds actual data)
                for(Mp4Atom *otherTopLevelAtom = firstElement(); otherTopLevelAtom; otherTopLevelAtom = otherTopLevelAtom->nextSibling()) {
                    if(isAborted()) {
                        throw OperationAbortedException();
                    }
                    try {
                        otherTopLevelAtom->parse();
                    } catch(Failure &) {
                        addNotification(NotificationType::Critical, "Unable to parse all top-level atoms of original file.", context);
                        throw InvalidDataException();
                    }
                    using namespace Mp4AtomIds;
                    switch(otherTopLevelAtom->id()) {
                    case FileType: case ProgressiveDownloadInformation: case Movie: case Free: case Skip:
                        break;
                    case MediaData:
                        if(writeChunkByChunk) {
                            break; // write actual data separately when writing chunk by chunk
                        } else {
                            // store the mdat offsets when not writing chunk by chunk to be able to update the stco tables
                            origMdatOffsets.push_back(otherTopLevelAtom->startOffset());
                            newMdatOffsets.push_back(outputStream.tellp());
                        }
                    default:
                        updateStatus("Writing " + otherTopLevelAtom->idToString() + " atom ...");
                        otherTopLevelAtom->forwardStatusUpdateCalls(this);
                        otherTopLevelAtom->copyEntirely(outputStream);
                    }
                }
                // when writing chunk by chunk the actual data needs to be written separately
                if(writeChunkByChunk) {
                    // get the chunk offset and the chunk size table from the old file to be able to write single chunks later ...
                    updateStatus("Reading chunk offsets and sizes from the original file ...");
                    trackInfos.reserve(trackCount);
                    for(auto &track : tracks()) {
                        if(isAborted()) {
                            throw OperationAbortedException();
                        }
                        // ensure the track reads from the original file
                        if(&track->inputStream() == &outputStream) {
                            track->setInputStream(backupStream);
                        }
                        trackInfos.emplace_back(&track->inputStream(), track->readChunkOffsets(), track->readChunkSizes());
                        const vector<uint64> &chunkOffsetTable = get<1>(trackInfos.back());
                        const vector<uint64> &chunkSizesTable = get<2>(trackInfos.back());
                        totalChunkCount += track->chunkCount();
                        if(track->chunkCount() != chunkOffsetTable.size() || track->chunkCount() != chunkSizesTable.size()) {
                            addNotification(NotificationType::Critical, "Chunks of track " + numberToString<uint64, string>(track->id()) + " could not be parsed correctly.", context);
                        }
                    }
                    // writing single chunks is needed when tracks have been added or removed
                    updateStatus("Writing chunks to mdat atom ...");
                    //outputStream.seekp(0, ios_base::end);
                    ostream::pos_type newMdatOffset = outputStream.tellp();
                    writer().writeUInt32BE(1); // denote 64 bit size
                    outputWriter.writeUInt32BE(Mp4AtomIds::MediaData);
                    outputWriter.writeUInt64BE(0); // write size of mdat atom later
                    CopyHelper<0x2000> copyHelper;
                    uint64 chunkIndex = 0;
                    uint64 totalChunksCopied = 0;
                    bool chunksCopied;
                    do {
                        if(isAborted()) {
                            throw OperationAbortedException();
                        }
                        chunksCopied = false;
                        for(size_t trackIndex = 0; trackIndex < trackCount; ++trackIndex) {
                            //auto &track = tracks()[trackIndex];
                            auto &trackInfo = trackInfos[trackIndex];
                            istream &sourceStream = *get<0>(trackInfo);
                            vector<uint64> &chunkOffsetTable = get<1>(trackInfo);
                            const vector<uint64> &chunkSizesTable = get<2>(trackInfo);
                            if(chunkIndex < chunkOffsetTable.size() && chunkIndex < chunkSizesTable.size()) {
                                sourceStream.seekg(chunkOffsetTable[chunkIndex]);
                                //outputStream.seekp(0, ios_base::end);
                                chunkOffsetTable[chunkIndex] = outputStream.tellp();
                                copyHelper.copy(sourceStream, outputStream, chunkSizesTable[chunkIndex]);
                                //track->updateChunkOffset(chunkIndex, chunkOffset);
                                chunksCopied = true;
                                ++totalChunksCopied;
                            }
                        }
                        ++chunkIndex;
                        updatePercentage(static_cast<double>(totalChunksCopied) / totalChunkCount);
                    } while(chunksCopied);
                    //outputStream.seekp(0, ios_base::end);
                    Mp4Atom::seekBackAndWriteAtomSize64(outputStream, newMdatOffset);
                }
            }
        }
        // reparse new file
        updateStatus("Reparsing output file ...");
        outputStream.close(); // outputStream needs to be reopened to be able to read again
        outputStream.open(fileInfo().path(), ios_base::in | ios_base::out | ios_base::binary);
        setStream(outputStream);
        m_headerParsed = false;
        m_tracks.clear();
        m_tracksParsed = false;
        m_tagsParsed = false;
        try {
            parseTracks();
        } catch(Failure &) {
            addNotification(NotificationType::Critical, "Unable to reparse the header of the new file.", context);
            throw;
        }
        // update chunk offsets in the "stco"-atom of each track
        if(trackCount != tracks().size()) {
            stringstream error;
            error << "Unable to update chunk offsets (\"stco\"-atom): Number of tracks in the output file (" << tracks().size()
                  << ") differs from the number of tracks in the original file (" << trackCount << ").";
            addNotification(NotificationType::Critical, error.str(), context);
            throw Failure();
        }
        if(writeChunkByChunk) {
            updateStatus("Updating chunk offset table for each track ...");
            for(size_t trackIndex = 0; trackIndex < trackCount; ++trackIndex) {
                auto &track = tracks()[trackIndex];
                auto &chunkOffsetTable = get<1>(trackInfos[trackIndex]);
                if(track->chunkCount() == chunkOffsetTable.size()) {
                    track->updateChunkOffsets(chunkOffsetTable);
                } else {
                    addNotification(NotificationType::Critical, "Unable to update chunk offsets of track " + numberToString(trackIndex + 1) + ": Number of chunks in the output file differs from the number of chunks in the orignal file.", context);
                    throw Failure();
                }
            }
        } else {
            updateOffsets(origMdatOffsets, newMdatOffsets);
        }
        updatePercentage(100.0);
        // dispose no longer used resources and flush output file stream
        outputStream.flush();
    } catch(OperationAbortedException &) {
        setStream(outputStream);
        m_tracksParsed = false;
        m_headerParsed = false;
        m_firstElement.reset();
        m_tracks.clear();
        addNotification(NotificationType::Information, "Rewriting the file to apply changed tag information has been aborted.", context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
        throw;
    } catch(Failure &) {
        setStream(outputStream);
        m_tracksParsed = false;
        m_headerParsed = false;
        m_firstElement.reset();
        m_tracks.clear();
        addNotification(NotificationType::Critical, "Rewriting the file to apply changed tag information failed.", context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
        throw;
    } catch(ios_base::failure &) {
        setStream(outputStream);
        m_tracksParsed = false;
        m_headerParsed = false;
        m_firstElement.reset();
        m_tracks.clear();
        addNotification(NotificationType::Critical, "An IO error occured when rewriting the file to apply changed tag information.", context);
        BackupHelper::restoreOriginalFileFromBackupFile(fileInfo().path(), backupPath, outputStream, backupStream);
        throw;
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
 * \throws Throws Media::Failure or a derived exception when a making
 *                error occurs.
 */
void Mp4Container::updateOffsets(const std::vector<int64> &oldMdatOffsets, const std::vector<int64> &newMdatOffsets)
{
    // do NOT invalidate the status here since this method is internally called by internalMakeFile(), just update the status
    updateStatus("Updating chunk offset table for each track ...");
    const string context("updating MP4 container chunk offset table");
    if(!firstElement()) {
        addNotification(NotificationType::Critical, "No MP4 atoms could be found.", context);
        throw InvalidDataException();
    }
    // update "base-data-offset-present" of "tfhd"-atom (NOT tested properly)
    try {
        for(Mp4Atom *moofAtom = firstElement()->siblingById(Mp4AtomIds::MovieFragment, false);
            moofAtom; moofAtom = moofAtom->siblingById(Mp4AtomIds::MovieFragment, false)) {
            moofAtom->parse();
            try {
                for(Mp4Atom *trafAtom = moofAtom->childById(Mp4AtomIds::TrackFragment); trafAtom;
                    trafAtom = trafAtom->siblingById(Mp4AtomIds::TrackFragment, false)) {
                    trafAtom->parse();
                    int tfhdAtomCount = 0;
                    for(Mp4Atom *tfhdAtom = trafAtom->childById(Mp4AtomIds::TrackFragmentHeader); tfhdAtom;
                        tfhdAtom = tfhdAtom->siblingById(Mp4AtomIds::TrackFragmentHeader, false)) {
                        tfhdAtom->parse();
                        ++tfhdAtomCount;
                        if(tfhdAtom->dataSize() >= 8) {
                            stream().seekg(tfhdAtom->dataOffset() + 1);
                            uint32 flags = reader().readUInt24BE();
                            if(flags & 1) {
                                if(tfhdAtom->dataSize() >= 16) {
                                    stream().seekg(4, ios_base::cur); // skip track ID
                                    uint64 off = reader().readUInt64BE();
                                    for(auto iOld = oldMdatOffsets.cbegin(), iNew = newMdatOffsets.cbegin(), end = oldMdatOffsets.cend();
                                        iOld != end; ++iOld, ++iNew) {
                                        if(off >= static_cast<uint64>(*iOld)) {
                                            off += (*iNew - *iOld);
                                            stream().seekp(tfhdAtom->dataOffset() + 8);
                                            writer().writeUInt64BE(off);
                                            break;
                                        }
                                    }
                                } else {
                                    addNotification(NotificationType::Warning, "tfhd atom (denoting base-data-offset-present) is truncated.", context);
                                }
                            }
                        } else {
                            addNotification(NotificationType::Warning, "tfhd atom is truncated.", context);
                        }
                    }
                    switch(tfhdAtomCount) {
                    case 0:
                        addNotification(NotificationType::Warning, "traf atom doesn't contain mandatory tfhd atom.", context);
                        break;
                    case 1:
                        break;
                    default:
                        addNotification(NotificationType::Warning, "traf atom stores multiple tfhd atoms but it should only contain exactly one tfhd atom.", context);
                    }
                }
            } catch(Failure &) {
                addNotification(NotificationType::Critical, "Unable to parse childs of top-level atom moof.", context);
            }
        }
    } catch(Failure &) {
        addNotification(NotificationType::Critical, "Unable to parse top-level atom moof.", context);
    }
    // update each track
    for(auto &track : tracks()) {
        if(isAborted()) {
            throw OperationAbortedException();
        }
        if(!track->isHeaderValid()) {
            try {
                track->parseHeader();
            } catch(Failure &) {
                addNotification(NotificationType::Warning, "The chunk offsets of track " + track->name() + " couldn't be updated because the track seems to be invalid..", context);
                throw;
            }
        }
        if(track->isHeaderValid()) {
            try {
                track->updateChunkOffsets(oldMdatOffsets, newMdatOffsets);
            } catch(Failure &) {
                addNotification(NotificationType::Warning, "The chunk offsets of track " + track->name() + " couldn't be updated.", context);
                throw;
            }
        }
    }
}

}
