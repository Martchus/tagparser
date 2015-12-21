#ifndef ID3V1TAG_H
#define ID3V1TAG_H

#include "../tag.h"

namespace Media
{

class LIB_EXPORT Id3v1Tag : public Tag
{
public:
    Id3v1Tag();

    virtual TagType type() const;
    virtual const char *typeName() const;
    virtual bool canEncodingBeUsed(TagTextEncoding encoding) const;
    virtual const TagValue &value(KnownField value) const;
    virtual bool setValue(KnownField field, const TagValue &value);
    virtual bool setValueConsideringTypeInfo(KnownField field, const TagValue &value, const std::string &typeInfo);
    virtual bool hasField(KnownField field) const;
    virtual void removeAllFields();
    virtual unsigned int fieldCount() const;
    virtual bool supportsField(KnownField field) const;

    void parse(std::istream &sourceStream, bool autoSeek);
    void parse(std::iostream &sourceStream);
    void make(std::ostream &targetStream);

private:
    void readValue(TagValue &value, size_t length, char *buffer, int offset);
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
