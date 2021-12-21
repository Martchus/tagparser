#include <tagparser/abstractattachment.h>
#include <tagparser/diagnostics.h>
#include <tagparser/mediafileinfo.h>
#include <tagparser/progressfeedback.h>
#include <tagparser/tag.h>
#include <tagparser/tagvalue.h>

/*!
 * \brief An example for reading and writing tags in a format-independent way.
 * \sa Checkout the README's "Usage" section for further explanations.
 */
void example()
{
    using namespace std::literals;
    using namespace TagParser;

    // create a MediaFileInfo for high-level access to overall functionality of the library
    auto fileInfo = MediaFileInfo();

    // create container for errors, warnings, etc.
    auto diag = Diagnostics();

    // create handle to abort gracefully and get feedback during during long operations
    auto progress = AbortableProgressFeedback(
        [](AbortableProgressFeedback &feedback) {
            // callback for status update
            std::clog << "At step: " << feedback.step() << '\n';
        },
        [](AbortableProgressFeedback &feedback) {
            // callback for percentage-only updates
            std::clog << "Step percentage: " << feedback.stepPercentage() << '\n';
        });

    // open file (might throw ios_base::failure)
    fileInfo.setPath("/path/to/some/file"sv);
    fileInfo.open();

    // parse container format, tags, attachments and/or chapters as needed
    // notes:
    // - These functions might throw exceptions derived from ios_base::failure for IO errors and
    //   populate diag with possibly critical parsing messages you definitely want to check in production
    //   code.
    // - Parsing a file can be expensive if the file is big or the disk IO is slow. You might want to
    //   run it in a separate thread.
    // - At this point the parser does not make much use of the progress object.
    fileInfo.parseContainerFormat(diag, progress);
    fileInfo.parseTags(diag, progress);
    fileInfo.parseAttachments(diag, progress);
    fileInfo.parseChapters(diag, progress);
    fileInfo.parseEverything(diag, progress); // just use that one if you want all over the above

    // get tag as an object derived from the Tag class
    // notes:
    // - In real code you might want to check how many tags are assigned or use
    //   fileInfo.createAppropriateTags(…) to create tags as needed.
    auto tag = fileInfo.tags().at(0);

    // extract a field value and convert it to UTF-8 std::string (toString() might throw ConversionException)
    auto title = tag->value(TagParser::KnownField::Title).toString(TagParser::TagTextEncoding::Utf8);

    // change a field value using an encoding suitable for the tag format
    tag->setValue(KnownField::Album, TagValue("some UTF-8 string", TagTextEncoding::Utf8, tag->proposedTextEncoding()));

    // get/remove/create attachments
    if (auto *const container = fileInfo.container()) {
        for (std::size_t i = 0, count = container->attachmentCount(); i != count; ++i) {
            auto attachment = container->attachment(i);
            if (attachment->mimeType() == "image/jpeg") {
                attachment->setIgnored(true); // remove existing attachment
            }
        }
        // create new attachment
        auto attachment = container->createAttachment();
        attachment->setName("The cover");
        attachment->setFile("cover.jpg", diag, progress);
    }

    // apply changes to the file on disk
    // notes:
    // - Might throw exception derived from TagParser::Failure for fatal processing error or ios_base::failure
    //   for IO errors.
    // - Applying changes can be expensive if the file is big or the disk IO is slow. You might want to
    //   run it in a separate thread.
    // - Use progress.tryToAbort() from another thread or an interrupt handler to abort gracefully without leaving
    //   the file in an inconsistent state.
    // - Be sure everything has been parsed before as the library needs to be aware of the whole file structure.
    fileInfo.parseEverything(diag, progress);
    fileInfo.applyChanges(diag, progress);
}
