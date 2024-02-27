#include "../caseinsensitivecomparer.h"

#include <c++utilities/conversion/stringbuilder.h>
#include <c++utilities/tests/testutils.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <fstream>

using namespace std;
using namespace CppUtilities;
using namespace TagParser;
using namespace CPPUNIT_NS;

/*!
 * \brief The Sha256Checksum struct holds the "hex string representation" of a SHA-256 checksum.
 */
struct Sha256Checksum {
    char checksum[65];
    bool operator==(const Sha256Checksum &other) const;
};

/*!
 * \brief Returns whether the current instance equals \a other ignoring the case.
 */
bool Sha256Checksum::operator==(const Sha256Checksum &other) const
{
    for (const char *i1 = checksum, *i2 = other.checksum, *end = checksum + sizeof(checksum); i1 != end; ++i1, ++i2) {
        if (CaseInsensitiveCharComparer::toLower(static_cast<unsigned char>(*i1))
            != CaseInsensitiveCharComparer::toLower(static_cast<unsigned char>(*i2))) {
            return false;
        }
    }
    return true;
}

ostream &operator<<(ostream &os, const Sha256Checksum &checksum)
{
    os << checksum.checksum;
    return os;
}

/*!
 * \brief The TestFile struct holds the path (relative to testfile dir) and checksum of a test file.
 * \remarks Generated with `scripts/list_required_testfiles.sh`.
 */
struct TestFile {
    const char *path;
    Sha256Checksum expectedSha256sum;
    Sha256Checksum computeSha256Sum() const;
    void verifyChecksum() const;
} const testFiles[] = {
    { "matroska_wave1/logo3_256x256.png", { "810b9172607e281d9a3969018c7d6521de240cc3688fecf598444e666aa6b4dc" } },
    { "matroska_wave1/test1.mkv", { "0996a309ff2095910b9d30d5253b044d637154297ddf7d0bda7f3adedf5addc1" } },
    { "matroska_wave1/test2.mkv", { "5b53d306e56f9bda6e80c3fbd9f3ccd20cc885770449d1fc0b5bec35c71d61e2" } },
    { "matroska_wave1/test3.mkv", { "1722b0d93a6ef1a14dd513bd031cd5901c233b45aa3e3c87be0b0d7348d7d1b5" } },
    { "matroska_wave1/test4.mkv", { "43df750a2a01a37949791b717051b41522081a266b71d113be4b713063843699" } },
    { "matroska_wave1/test5.mkv", { "92acdc33bb0b5d7a4d9b0d6ca792230a78c786a30179dc9999cee41c28642842" } },
    { "matroska_wave1/test6.mkv", { "7cad84b434116e023d340dd584ac833b93f03fb1bd7ea2727fa45de50af0abb9" } },
    { "matroska_wave1/test7.mkv", { "95b21c92ad5a4fe00914ff5009e2a64f12fd4c5fb9cb1c3c888ab50bf0ffe483" } },
    { "matroska_wave1/test8.mkv", { "9dddcd1550b814dae44d62e2b9f27c0eca31d5e190df2220cbf7492e3d6c63da" } },
    { "mtx-test-data/mkv/handbrake-chapters-2.mkv", { "eccc55f3b59a77086b3ffb914525d312c7886eae34e3933352dea2f6f6a1974c" } },
    { "mtx-test-data/mkv/tags.mkv", { "4330019afc3d846600c1ded38158fcac081297f4e56c749251c236f4871e0287" } },
    { "mkv/nested-tags.xml", { "85cfcc94920f114e52fd1aa3df24706cd2710626e065a2c8c55dd209ec8dc8ce" } },
    { "mp4/test1.m4a", { "4f16e0a22525bd13ba859431406d7f5991e0b4f155c51e10e5f32b0c97034b36" } },
    { "mp4/android-8.1-camera-recoding.mp4", { "e7c5624872de1c9c2fb52dd954cef53adc337a6ba88342ff516dde5d4ef374dc" } },
    { "mtx-test-data/aac/he-aacv2-ps.m4a", { "be54be0ae45b0184583ced8a84a881a1652a449feb7f6a917e11f60efabb68ac" } },
    { "mtx-test-data/alac/othertest-itunes.m4a", { "5e9c64cde00902211533fbe38aaa67ef5f79a945e1d717951b78b4bbf9ff84e8" } },
    { "mtx-test-data/mp3/id3-tag-and-xing-header.mp3", { "4a9187b05dc74d32e5a3de53494fde9db8c6c25d46082f86de6f424ad28daacf" } },
    { "mtx-test-data/mp4/10-DanseMacabreOp.40.m4a", { "30c915d5656de049d66fd70b0966a33faf038af42365a2bb973e5c2fc0ba2038" } },
    { "mtx-test-data/mp4/1080p-DTS-HD-7.1.mp4", { "fbf929bf8300fc6e53c5c5b7fde4ed2a427fef2d4fd093511c672083039abbf1" } },
    { "mtx-test-data/mp4/dash/dragon-age-inquisition-H1LkM6IVlm4-video.mp4", { "864891f4510f3fa9c49c19e671171cec08ceb331362cf7161419b957be090d47" } },
    { "mtx-test-data/ogg/qt4dance_medium.ogg", { "0b5429da9713be171c6ae0da69621261e8d5ddc9db3da872e5ade1a1c883decd" } },
    { "mtx-test-data/opus/v-opus.ogg", { "e12adece4dbcccf2471b61c3ebd7c6576dee351d85809ab6f01d6f324d65b417" } },
    { "misc/multiple_id3v2_4_values.mp3", { "da012a41213cdc49b2afe1457625d8baced1a64e2351f17b520bf82c6bfe4e03" } },
    { "ogg/noise-without-cover.opus", { "ff578894c0c47aed4cc41ae94dee2886fe2c556593e44f731135f47bca870464" } },
    { "ogg/noise-broken-segment-termination.opus", { "12835cf12b5b9fa70c239ae05e9d5bb768e715a2d61ef6301ed4af673088de45" } },
    { "ogg/example-cover.png", { "897e1a2d0cfb79c1fe5068108bb34610c3758bd0b9a7e90c1702c4e6972e0801" } },
};

/// \cond
struct EvpMdCtx {
    EvpMdCtx()
        : handle(EVP_MD_CTX_new())
    {
    }
    ~EvpMdCtx()
    {
        if (handle) {
            EVP_MD_CTX_free(handle);
        }
    }
    EVP_MD_CTX *handle;
};
/// \endcond

/*!
 * \brief Computes the SHA-256 checksums for the file using OpenSSL.
 */
Sha256Checksum TestFile::computeSha256Sum() const
{
    // init sha256 hashing
    const auto mdctx = EvpMdCtx();
    if (!mdctx.handle) {
        throw std::runtime_error("Unable to create EVP context.");
    }
    if (EVP_DigestInit_ex(mdctx.handle, EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Unable to init SHA256-EVP context.");
    }

    // read and hash file
    {
        ifstream file;
        file.exceptions(ios_base::eofbit | ios_base::failbit | ios_base::badbit);
        file.open(testFilePath(path), ios_base::in | ios_base::binary);

        char readBuffer[4096];
        try {
            for (;;) {
                file.read(readBuffer, sizeof(readBuffer));
                if (EVP_DigestUpdate(mdctx.handle, readBuffer, static_cast<std::size_t>(file.gcount())) != 1) {
                    throw std::runtime_error("Unable to update SHA256-EVP.");
                }
            }
        } catch (const std::ios_base::failure &) {
            if (file.eof() && !file.bad()) {
                if (EVP_DigestUpdate(mdctx.handle, readBuffer, static_cast<std::size_t>(file.gcount())) != 1) {
                    throw std::runtime_error("Unable to update SHA256-EVP.");
                }
            } else {
                throw;
            }
        }
    }

    // compute final hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    auto length = static_cast<unsigned int>(SHA256_DIGEST_LENGTH);
    if (EVP_DigestFinal_ex(mdctx.handle, hash, &length) != 1) {
        throw std::runtime_error("Unable to finalize SHA256-EVP.");
    }

    // convert to "hex string"
    Sha256Checksum hexString;
    char *hexStringIterator = hexString.checksum;
    for (unsigned char hashNumber : hash) {
        const string digits = numberToString(hashNumber, static_cast<unsigned char>(16));
        *(hexStringIterator++) = digits.size() < 2 ? '0' : digits.front();
        *(hexStringIterator++) = digits.back();
    }
    *hexStringIterator = '\0';
    return hexString;
}

/*!
 * \brief Checks whether the expected SHA-256 checksum matches the actual checksum.
 */
void TestFile::verifyChecksum() const
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(argsToString("integrity of testfile \"", path, '\"'), expectedSha256sum, computeSha256Sum());
}

/*!
 * \brief The TestFileCheck class verifies integrity of all testfiles used in the testsuite of
 *        tagparser or tageditor.
 */
class TestFileCheck : public TestFixture {
    CPPUNIT_TEST_SUITE(TestFileCheck);
    CPPUNIT_TEST(verifyChecksums);
    CPPUNIT_TEST_SUITE_END();

private:
    void verifyChecksums();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFileCheck);

void TestFileCheck::verifyChecksums()
{
    for (const TestFile &testFile : testFiles) {
        testFile.verifyChecksum();
    }
}
