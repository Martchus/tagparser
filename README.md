# tagparser
C++ library for reading and writing MP4 (iTunes), ID3, Vorbis and Matroska tags.

## Supported formats
The tag library can read and write the following tag formats:
- iTunes-style MP4 tags (MP4-DASH is supported)
- ID3v1 and ID3v2 tags
- Vorbis comments (cover art via "METADATA_BLOCK_PICTURE" is supported)
- Matroska/WebM tags and attachments

The library can also display technical information such as the ID, format, language, bitrate,
duration, size, timestamps, sampling frequency, FPS and other information of the tracks.

It also allows to inspect and validate the element structure of MP4 and Matroska files.

For examples check out the CLI interface of my Tag Editor (which is also on Git).

## Build instructions
The tagparser library depends on c++utilities and is built in the same way.
It also depends on zlib.
