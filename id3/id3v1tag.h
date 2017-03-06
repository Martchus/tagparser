#ifndef ID3V1TAG_H
#define ID3V1TAG_H

#include "../tag.h"

namespace Media
{

class TAG_PARSER_EXPORT Id3v1Tag : public Tag
{
public:
    Id3v1Tag();

    static constexpr TagType tagType = TagType::Id3v1Tag;
    static constexpr const char *tagName = "ID3v1 tag";
    TagType type() const;
    const char *typeName() const;
    bool canEncodingBeUsed(TagTextEncoding encoding) const;
    const TagValue &value(KnownField value) const;
    bool setValue(KnownField field, const TagValue &value);
    bool setValueConsideringTypeInfo(KnownField field, const TagValue &value, const std::string &typeInfo);
    bool hasField(KnownField field) const;
    void removeAllFields();
    unsigned int fieldCount() const;
    bool supportsField(KnownField field) const;
    void ensureTextValuesAreProperlyEncoded();

    void parse(std::istream &sourceStream, bool autoSeek);
    void parse(std::iostream &sourceStream);
    void make(std::ostream &targetStream);

private:
    void readValue(TagValue &value, size_t maxLength, const char *buffer);
    void writeValue(const TagValue &value, size_t length, char *buffer, std::ostream &targetStream);

    TagValue m_title;
    TagValue m_artist;
    TagValue m_album;
    TagValue m_year;
    TagValue m_comment;
    TagValue m_trackPos;
    TagValue m_genre;
};

}

#endif // ID3V1TAG_H
