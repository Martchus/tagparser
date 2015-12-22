# Tag Parser
C++ library for reading and writing MP4 (iTunes), ID3, Vorbis and Matroska tags.

## Supported formats
The tag library can read and write the following tag formats:
- iTunes-style MP4 tags (MP4-DASH is supported)
- ID3v1 and ID3v2 tags
- Vorbis comments (cover art via "METADATA_BLOCK_PICTURE" is supported)
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

## Additional features
The library can also display technical information such as the ID, format, language, bitrate,
duration, size, timestamps, sampling frequency, FPS and other information of the tracks.

It also allows to inspect and validate the element structure of MP4 and Matroska files.

## Usage
For examples check out the command line interface of my Tag Editor (which is also on Git).

## Build instructions
The tagparser library depends on c++utilities and is built in the same way.
It also depends on zlib.

## TODO
- Support more tag formats (EXIF, PDF metadata, ...).
- Do tests with Matroska files which have multiple segments.
