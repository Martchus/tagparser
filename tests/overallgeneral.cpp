#include "./overall.h"

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

/*!
 * \brief Creates some test meta data.
 */
void OverallTests::setUp()
{
    m_testTitle.assignText("some title", TagTextEncoding::Utf8);
    m_testComment.assignText("some cómment", TagTextEncoding::Utf8);
    m_testComment.setDescription("some descriptión", TagTextEncoding::Utf8);
    m_testAlbum.assignText("some album", TagTextEncoding::Utf8);
    m_testPartNumber.assignInteger(41);
    m_testTotalParts.assignInteger(61);
    m_testPosition.assignPosition(PositionInSet(41, 61));
}

void OverallTests::tearDown()
{
    for(const string &file : {m_nestedTagsMkvPath, m_rawFlacPath, m_flacInOggPath}) {
        if(!file.empty()) {
            remove(file.data());
        }
    }
}

/*!
 * \brief Parses the specified file and tests the results using the specified check routine.
 */
void OverallTests::parseFile(const string &path, void (OverallTests::* checkRoutine)(void))
{
    // print current file
    cerr << "- testing " << path << endl;
    // ensure file is open and everything is parsed
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything();
    // invoke testroutine to check whether parsing results are correct
    (this->*checkRoutine)();
    m_fileInfo.close();
}

/*!
 * \brief Parses the specified file, modifies it using the specified modify routine, parses the file again and checks the results using the
 *        specified check routine.
 */
void OverallTests::makeFile(const string &path, void (OverallTests::*modifyRoutine)(void), void (OverallTests::*checkRoutine)(void))
{
    // print current file
    cerr << "- testing " << path << endl;
    // ensure file is open and everything is parsed
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything();

    // determine expected tag and index position
    switch(m_fileInfo.containerFormat()) {
    case ContainerFormat::Mp4:
        CPPUNIT_ASSERT(m_fileInfo.container());
        if(m_fileInfo.tagPosition() != ElementPosition::Keep) {
            m_expectedTagPos = m_fileInfo.tagPosition();
        } else {
            m_expectedTagPos = m_fileInfo.container()->determineTagPosition();
            if(m_expectedTagPos == ElementPosition::Keep) {
                // if there is no tag present, the resulting tag position should equal the
                // current index position
                m_expectedTagPos = m_fileInfo.container()->determineIndexPosition();
            }
        }
        break;
    case ContainerFormat::Matroska:
        CPPUNIT_ASSERT(m_fileInfo.container());
        // since a tag is always created, it can always be expected at the specified position
        if(m_fileInfo.tagPosition() != ElementPosition::Keep) {
            m_expectedTagPos = m_fileInfo.tagPosition();
        } else {
            m_expectedTagPos = m_fileInfo.container()->determineTagPosition();
        }
        // an index is only present if the file had one before, hence specifying the index position
        // might not have an effect
        m_expectedIndexPos = m_fileInfo.container()->determineIndexPosition();
        if(m_fileInfo.indexPosition() != ElementPosition::Keep && m_expectedIndexPos != ElementPosition::Keep) {
            m_expectedIndexPos = m_fileInfo.indexPosition();
        }
        break;
    default:
        ;
    }

    // invoke testroutine to do and apply changes
    (this->*modifyRoutine)();
    // apply changes and ensure that the previous parsing results are cleared
    m_fileInfo.applyChanges();
    m_fileInfo.clearParsingResults();
    // reparse the file and invoke testroutine to check whether changings have been applied correctly
    m_fileInfo.parseEverything();
    (this->*checkRoutine)();
    // invoke suitable testroutine to check padding constraints
    switch(m_fileInfo.containerFormat()) {
    case ContainerFormat::Matroska:
        checkMkvConstraints();
        break;
    case ContainerFormat::Mp4:
        checkMp4Constraints();
        break;
    case ContainerFormat::MpegAudioFrames:
    case ContainerFormat::Adts:
        checkMp3PaddingConstraints();
        break;
    default:
        ;
    }

    // close and remove file and backup files
    m_fileInfo.close();
    remove(path.c_str());
    remove((path + ".bak").c_str());
}

/*!
 * \brief Removes all tags.
 */
void OverallTests::removeAllTags()
{
    m_fileInfo.removeAllTags();
}

/*!
 * \brief Does nothing.
 * \remarks Used to just resave the file.
 */
void OverallTests::noop()
{
}
