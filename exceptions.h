#ifndef TAG_PARSER_EXCEPTIONS_H
#define TAG_PARSER_EXCEPTIONS_H

#include "./global.h"

#include <stdexcept>
#include <string>

namespace TagParser {

class TAG_PARSER_EXPORT Failure : public std::exception
{
public:
    Failure() USE_NOTHROW;
    virtual ~Failure() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT NoDataFoundException : public Failure
{
public:
    NoDataFoundException() USE_NOTHROW;
    virtual ~NoDataFoundException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT InvalidDataException : public Failure
{
public:
    InvalidDataException() USE_NOTHROW;
    virtual ~InvalidDataException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT TruncatedDataException : public InvalidDataException
{
public:
    TruncatedDataException() USE_NOTHROW;
    virtual ~TruncatedDataException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT OperationAbortedException : public Failure
{
public:
    OperationAbortedException() USE_NOTHROW;
    virtual ~OperationAbortedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT VersionNotSupportedException : public Failure
{
public:
    VersionNotSupportedException() USE_NOTHROW;
    virtual ~VersionNotSupportedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class TAG_PARSER_EXPORT NotImplementedException : public Failure
{
public:
    NotImplementedException() USE_NOTHROW;
    virtual ~NotImplementedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

/*!
 * \brief Throws TruncatedDataException() if the specified \a sizeDenotation exceeds maxSize; otherwise maxSize is reduced by \a sizeDenotation.
 */
#define CHECK_MAX_SIZE(sizeDenotation) \
    if(maxSize < sizeDenotation) { \
        throw TruncatedDataException(); \
    } else { \
        maxSize -= sizeDenotation; \
    }

}

#endif // TAG_PARSER_EXCEPTIONS_H
