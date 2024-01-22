# Tag Parser
C++ library for reading and writing MP4 (iTunes), ID3, Vorbis, Opus, FLAC and Matroska tags.

## Supported formats
The tag library can read and write the following tag formats:

* iTunes-style MP4/M4A tags (MP4-DASH is supported)
* ID3v1 and ID3v2 tags
    * conversion between ID3v1 and different versions of ID3v2 is possible
    * mainly for use in MP3 files but can be added to any kind of file
* Vorbis, Opus and FLAC comments in Ogg streams
    * cover art via "METADATA_BLOCK_PICTURE" is supported
* Vorbis comments and "METADATA_BLOCK_PICTURE" in raw FLAC streams
* Matroska/WebM tags and attachments

Further remarks:

* Unsupported file contents (such as unsupported tag formats) are *generally* preserved as-is.
* Note that APE tags are *not* supported. APE tags in the beginning of a file are strongly
  unrecommended and thus discarded when applying changes. APE tags at the end of the file
  are preserved as-is when applying changes.

## File layout options
### Tag position
The library allows you to choose whether tags should be placed at the beginning or at
the end of an MP4/Matroska file.

### Padding
Padding allows adding additional tag information without rewriting the entire file
or appending the tag. Usage of padding can be configured:
* minimum/maximum padding: The file is rewritten if the padding would fall below/exceed the specified limits.
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

## Usage
* For building/installing the library, checkout the build instructions below.
* To use the library with CMake, use its find module (e.g. `find_package(tagparser 10.1.0 REQUIRED)`) which
  provides the imported target `tagparser` you can link against. Otherwise, use the `pkg-config` module
  `tagparser` to query the required compiler/linker flags.
* For a code example that shows how to read and write tag fields in a format-independent way, have
  a look at [`example.cpp`](doc/example.cpp).
* The most important class is `TagParser::MediaFileInfo` providing access to everything else.
* IO errors are propagated via standard `std::ios_base::failure`.
* Fatal processing errors are propagated by throwing a class derived from `TagParser::Failure`.
* All operations which might generate warnings, non-fatal errors, ... take a `TagParser::Diagnostics` object to store
  those messages.
* All operations which might be aborted or might provide progress feedback take a `TagParser::AbortableProgressFeedback`
  object for callbacks and aborting.
* Field values are stored using `TagParser::TagValue` objects. Those objects erase the actual type similar to `QVariant`
  from the Qt framework. The documentation of `TagParser::TagValue` covers how different types and encodings are
  handled.

### Further documentation
For more examples check out the command line interface of [Tag Editor](https://github.com/Martchus/tageditor).
API documentation can be generated using Doxygen with `cmake --build … --target tagparser_apidoc`.

## Bugs, stability
Bugs can be reported on GitHub.

It is recommend to create backups before editing because I can not test whether the library
works with all kinds of files. (When forcing rewrite a backup is always created.)

## Build instructions
The tagparser library depends on [c++utilities](https://github.com/Martchus/cpp-utilities) and is built
in the same way.
It also depends on zlib, [iso-codes](https://salsa.debian.org/iso-codes-team/iso-codes) and requires at
least CMake 3.19. Tests depend on CppUnit. For checking integrity of testfiles, the OpenSSL crypto library is
required.

The location of the JSON file from iso-codes can be specified via the CMake variable `LANGUAGE_FILE_ISO_639_2`.

For building multiple projects in one go (c++utilities, tagparser and the tag editor), checkout
the ["Building this straight"](https://github.com/Martchus/tageditor#building-this-straight) instructions.

## TODOs
* Support more formats (EXIF, PDF metadata, Theora, ...)
* Support adding cue-sheet to FLAC files

More TODOs are tracked in the [issue section at GitHub](https://github.com/Martchus/tagparser/issues).

## Copyright notice and license
Copyright © 2015-2024 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
