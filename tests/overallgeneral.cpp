#include "./overall.h"

CPPUNIT_TEST_SUITE_REGISTRATION(OverallTests);

OverallTests::OverallTests()
    : m_progress(std::function<void(AbortableProgressFeedback &)>(), std::function<void(AbortableProgressFeedback &)>())
{
}

/*!
 * \brief Creates some test meta data.
 */
void OverallTests::setUp()
{
    m_testTitle.assignText("some title"sv, TagTextEncoding::Utf8);
    m_testComment.assignText("some cómment"sv, TagTextEncoding::Utf8);
    m_testComment.setDescription("some descriptión"sv, TagTextEncoding::Utf8);
    m_testCommentWithoutDescription.assignText("some cómment"sv, TagTextEncoding::Utf8);
    m_testAlbum.assignText("some album"sv, TagTextEncoding::Utf8);
    m_testPartNumber.assignInteger(41);
    m_testTotalParts.assignInteger(61);
    m_testPosition.assignPosition(PositionInSet(41, 61));
}

void OverallTests::tearDown()
{
}

/*!
 * \brief Parses the specified file and tests the results using the specified check routine.
 */
void OverallTests::parseFile(const string &path, void (OverallTests::*checkRoutine)(void))
{
    // print current file
    cerr << "- testing " << path << endl;
    // ensure file is open and everything is parsed
    m_diag.clear();
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything(m_diag, m_progress);
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
    m_diag.clear();
    m_fileInfo.setPath(path);
    m_fileInfo.reopen(true);
    m_fileInfo.parseEverything(m_diag, m_progress);

    // determine expected tag and index position
    switch (m_fileInfo.containerFormat()) {
    case ContainerFormat::Mp4:
        CPPUNIT_ASSERT(m_fileInfo.container());
        if (m_fileInfo.tagPosition() != ElementPosition::Keep) {
            m_expectedTagPos = m_fileInfo.tagPosition();
        } else {
            m_expectedTagPos = m_fileInfo.container()->determineTagPosition(m_diag);
            if (m_expectedTagPos == ElementPosition::Keep) {
                // if there is no tag present, the resulting tag position should equal the
                // current index position
                m_expectedTagPos = m_fileInfo.container()->determineIndexPosition(m_diag);
            }
        }
        break;
    case ContainerFormat::Matroska:
        CPPUNIT_ASSERT(m_fileInfo.container());
        // since a tag is always created, it can always be expected at the specified position
        if (m_fileInfo.tagPosition() != ElementPosition::Keep) {
            m_expectedTagPos = m_fileInfo.tagPosition();
        } else {
            m_expectedTagPos = m_fileInfo.container()->determineTagPosition(m_diag);
        }
        // an index is only present if the file had one before, hence specifying the index position
        // might not have an effect
        m_expectedIndexPos = m_fileInfo.container()->determineIndexPosition(m_diag);
        if (m_fileInfo.indexPosition() != ElementPosition::Keep && m_expectedIndexPos != ElementPosition::Keep) {
            m_expectedIndexPos = m_fileInfo.indexPosition();
        }
        break;
    default:;
    }

    // invoke testroutine to do and apply changes
    (this->*modifyRoutine)();
    // apply changes and ensure that the previous parsing results are cleared
    m_fileInfo.applyChanges(m_diag, m_progress);
    m_fileInfo.clearParsingResults();
    // reparse the file and invoke testroutine to check whether changings have been applied correctly
    m_fileInfo.parseEverything(m_diag, m_progress);
    (this->*checkRoutine)();
    // invoke suitable testroutine to check padding constraints
    switch (m_fileInfo.containerFormat()) {
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
    default:;
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
 * \remarks Used to just resave the file without modifications.
 */
void OverallTests::noop()
{
}

/*!
 * \brief Removes the second track of the file to be tested.
 */
void OverallTests::removeSecondTrack()
{
    CPPUNIT_ASSERT(m_fileInfo.container());
    CPPUNIT_ASSERT(m_fileInfo.container()->trackCount() >= 2);
    m_fileInfo.container()->removeTrack(m_fileInfo.container()->track(1));
}
