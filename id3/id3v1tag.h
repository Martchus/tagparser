#ifndef TAG_PARSER_ID3V1TAG_H
#define TAG_PARSER_ID3V1TAG_H

#include "../tag.h"

namespace TagParser {

class Diagnostics;

class TAG_PARSER_EXPORT Id3v1Tag final : public Tag {
public:
    Id3v1Tag();

    static constexpr TagType tagType = TagType::Id3v1Tag;
    static constexpr std::string_view tagName = "ID3v1 tag";
    TagType type() const override;
    std::string_view typeName() const override;
    bool canEncodingBeUsed(TagTextEncoding encoding) const override;
    const TagValue &value(KnownField value) const override;
    bool setValue(KnownField field, const TagValue &value) override;
    bool setValueConsideringTypeInfo(KnownField field, const TagValue &value, const std::string &typeInfo);
    bool hasField(KnownField field) const override;
    void removeAllFields() override;
    std::size_t fieldCount() const override;
    bool supportsField(KnownField field) const override;
    void ensureTextValuesAreProperlyEncoded() override;

    void parse(std::istream &sourceStream, Diagnostics &diag);
    void make(std::ostream &targetStream, Diagnostics &diag);

private:
    void readValue(TagValue &value, std::size_t maxLength, const char *buffer);
    void writeValue(const TagValue &value, std::size_t length, char *buffer, std::ostream &targetStream, Diagnostics &diag);

    TagValue m_title;
    TagValue m_artist;
    TagValue m_album;
    TagValue m_year;
    TagValue m_comment;
    TagValue m_trackPos;
    TagValue m_genre;
};

} // namespace TagParser

#endif // TAG_PARSER_ID3V1TAG_H
