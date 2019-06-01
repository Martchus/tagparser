# Tag Parser
C++ library for reading and writing MP4 (iTunes), ID3, Vorbis, Opus, FLAC and Matroska tags.

## Supported formats
The tag library can read and write the following tag formats:

* iTunes-style MP4/M4A tags (MP4-DASH is supported)
* ID3v1 and ID3v2 tags
    * conversion between ID3v1 and different versions of ID3v2 is possible
* Vorbis, Opus and FLAC comments in Ogg streams
    * cover art via "METADATA_BLOCK_PICTURE" is supported
* Vorbis comments and "METADATA_BLOCK_PICTURE" in raw FLAC streams
* Matroska/WebM tags and attachments

## File layout options
### Tag position
The library allows you to choose whether tags should be placed at the beginning or at
the end of an MP4/Matroska file.

### Padding
Padding allows adding additional tag information without rewriting the entire file
or appending the tag. Usage of padding can be configured:
* minimum/maximum padding: The file is rewritten if the padding would fall below/exceed the specifed limits.
* preferred padding: If the file needs to be rewritten the preferred padding is used.

Default value for minimum and maximum padding is zero. Hence the library will almost always have to rewrite
the entire file to apply changes. To prevent this, set at least the maximum padding to a higher value.

It is also possible to force rewriting the entire file always.

Taking advantage of padding is currently not supported when dealing with Ogg streams (it is supported when dealing with raw FLAC streams).

## Additional features
The library can also display technical information such as the ID, format, language, bitrate,
duration, size, timestamps, sampling frequency, FPS and other information of the tracks.

It also allows to inspect and validate the element structure of MP4 and Matroska files.

## Text encoding, Unicode support
The library is aware of different text encodings and can convert between different encodings using iconv.

### Windows specific
A workaround to support filenames containing non-ASCII characters (despite the lack of an UTF-8
supporting `std::fstream` under Windows) can be enabled by adding `-DUSE_NATIVE_FILE_BUFFER=ON`
to the CMake arguments **when building `c++utilities`**. It is *not* sufficient to specify this
option only when building `tagparser`.

## Usage
This example shows how to read and write tag fields in a format-independent way:

```
#include <tagparser/mediafileinfo.h>
#include <tagparser/diagnostics.h>

// create a MediaFileInfo for high-level access to overall functionality of the library
TagParser::MediaFileInfo fileInfo;
// create container for errors, warnings, etc.
Diagnostics diag;

// open file (might throw ios_base::failure)
fileInfo.setPath("/path/to/some/file");
fileInfo.open();
// parse tags
// (might throw exception derived from TagParser::Failure for fatal parsing error or ios_base::failure for IO errors)
fileInfo.parseTags(diag);

// get first tag as an object derived from the Tag class
auto tag = fileInfo.tags().at(0);
// extract title and convert it to UTF-8 std::string
// (toString() might throw ConversionException)
auto title = tag->value(TagParser::KnownField::Title).toString(TagParser::TagTextEncoding::Utf8);

// change album using an encoding suitable for the tag format
tag->setValue(TagParser::KnownField::Album, TagParser::TagValue("some UTF-8 string", TagParser::TagTextEncoding::Utf8, tag->proposedTextEncoding()));

// create progress
TagParser::AbortableProgressFeedback progress([callback for status update], [callback for percentage-only updates]);

// apply changes to the file on disk
// (might throw exception derived from TagParser::Failure for fatal processing error or ios_base::failure for IO errors)
fileInfo.applyChanges(diag, progress);
```

### Summary
* The most important class is `TagParser::MediaFileInfo` providing access to everything else.
* IO errors are propagated via standard `std::ios_base::failure`.
* Fatal processing errors are propagated by throwing a class derived from `TagParser::Failure`.
* All operations which might generate warnings, non-fatal errors, ... take a `TagParser::Diagnostics` object to store
  those messages.
* All operations which can be aborted or provide progress feedback take a `TagParser::AbortableProgressFeedback` object
  for callbacks and aborting.
* Field values are stored using `TagParser::TagValue` objects. Those objects erase the actual type similar to `QVariant`
  from the Qt framework. The documentation of `TagParser::TagValue` covers how different types and encodings are
  handled.

### Further documentation
For more examples check out the command line interface of [Tag Editor](https://github.com/Martchus/tageditor).
API documentation can be generated using Doxygen with `make tagparser_apidoc`.

## Bugs, stability
Bugs can be reported on GitHub.

It is recommend to create backups before editing because I can not test whether the library
works with all kinds of files. (When forcing rewrite a backup is always created.)

## Build instructions
The tagparser library depends on [c++utilities](https://github.com/Martchus/cpp-utilities) and is built
in the same way.
It also depends on zlib. For checking integrity of testfiles, the OpenSSL crypto
library is required.

## TODOs
* Support more formats (EXIF, PDF metadata, Theora, ...)
* Support adding cue-sheet to FLAC files

More TODOs are tracked in the [issue section at GitHub](https://github.com/Martchus/tagparser/issues).
