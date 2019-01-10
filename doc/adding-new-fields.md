# Adding a new field

This document describes how to add support for a new field. From these instructions
one can also deduce what places need to be adjusted when amending support for a field.

## Support new field in tagparser library
1. Check whether https://wiki.hydrogenaud.io/index.php?title=Tag_Mapping contains
   a recommendation for the specific field to be added.
2. Add the field to the enum `KnownField`.
3. Adjust `TagParser::lastKnownField` accordingly.
4. Adjust `TagParser::Tag::proposedDataType()` if it is not a text field.
   Consider that this method might be overwritten in format-specific implementations
   which might need adjustment as well.
5. Add the format-specific IDs to the corresponding header files, e.g. `vorbiscommentids.h`
   for Vorbis Comments.
6. Add the field mapping to the `internallyGetFieldId()` and
   `internallyGetKnownField()` methods of all formats which
   should be supported, e.g. `TagParser::Id3v2Tag::internallyGetFieldId()`.
7. For ID3v2 tags add the mapping `convertToShortId()` and `convertToLongId()` if
   possible.

## Support new field in the tag editor application
1. Add the field to the `KnownFieldModel` class.
    1. Add the English denotation of the field to `KnownFieldModel::fieldName`.
    2. Add the field to the constructor `KnownFieldModel::KnownFieldModel` which
       composes the list of fields shown in the GUI and whether they are displayed
       by default or not.
2. Add the field to the `FIELD_NAMES` macro. It is used for the CLI's auto-completion
   and `print-field-names`. Only use small letters (a to z) here. No whitespaces!
3. Add the field to the `FieldMapping::fieldMapping` array used by the CLI. Be consistent
   with 2.!
