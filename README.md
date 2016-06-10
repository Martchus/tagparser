# Tag Parser
C++ library for reading and writing MP4 (iTunes), ID3, Vorbis and Matroska tags.

## Supported formats
The tag library can read and write the following tag formats:
- iTunes-style MP4 tags (MP4-DASH is supported)
- ID3v1 and ID3v2 tags
- Vorbis and Opus comments (cover art via "METADATA_BLOCK_PICTURE" is supported) in Ogg streams
- Matroska/WebM tags and attachments

## File layout options
### Tag position
The library allows you to choose whether tags should be placed at the beginning or at
the end of an MP4/Matroska file.

### Padding
Padding allows adding additional tag information without rewriting the entire file
or appending the tag. Usage of padding can be configured:
- minimum/maximum padding: The file is rewritten if the padding would fall below/exceed the specifed limits.
- preferred padding: If the file needs to be rewritten the preferred padding is used.

However, it is also possible to force rewriting the entire file.

Taking advantage of padding is currently not supported when dealing with Ogg streams.

## Additional features
The library can also display technical information such as the ID, format, language, bitrate,
duration, size, timestamps, sampling frequency, FPS and other information of the tracks.

It also allows to inspect and validate the element structure of MP4 and Matroska files.

## Text encoding, Unicode support
The library does not do any conversions for you (eg. converting Latin1 to UTF-8). However the
API provides a way to check which encoding is present (when reading) and which encoding(s)
can be used (when writing).

## Usage
For examples check out the command line interface of my [Tag Editor](https://github.com/Martchus/tageditor).

## Bugs, stability
- Matroska files composed of more than one segment aren't tested yet and might not work.
- To add new features I've had to revise a lot of code since the last release. I always test the library with
  files produced by mkvmerge and ffmpeg and several other file but can't verify that it will work with all
  files. Hence I recommend you to create backups of your files.

## Build instructions
The tagparser library depends on c++utilities and is built in the same way.
It also depends on zlib.

## TODO
- Support more formats (EXIF, PDF metadata, Theora, ...).
- Allow adding tags to specific streams when dealing with OGG.
- Do tests with Matroska files which have multiple segments.
