#ifndef TAG_PARSER_EXCEPTIONS_H
#define TAG_PARSER_EXCEPTIONS_H

#include "./global.h"

#include <stdexcept>
#include <string>

namespace TagParser {

class TAG_PARSER_EXPORT Failure : public std::exception {
public:
    Failure() noexcept;
    virtual ~Failure() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT NoDataFoundException : public Failure {
public:
    NoDataFoundException() noexcept;
    virtual ~NoDataFoundException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT InvalidDataException : public Failure {
public:
    InvalidDataException() noexcept;
    virtual ~InvalidDataException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT NoDataProvidedException : public Failure {
public:
    NoDataProvidedException() noexcept;
    virtual ~NoDataProvidedException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT TruncatedDataException : public InvalidDataException {
public:
    TruncatedDataException() noexcept;
    virtual ~TruncatedDataException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT OperationAbortedException : public Failure {
public:
    OperationAbortedException() noexcept;
    virtual ~OperationAbortedException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT VersionNotSupportedException : public Failure {
public:
    VersionNotSupportedException() noexcept;
    virtual ~VersionNotSupportedException() noexcept;
    virtual const char *what() const noexcept;
};

class TAG_PARSER_EXPORT NotImplementedException : public Failure {
public:
    NotImplementedException() noexcept;
    virtual ~NotImplementedException() noexcept;
    virtual const char *what() const noexcept;
};

/*!
 * \brief Throws TruncatedDataException() if the specified \a sizeDenotation exceeds maxSize; otherwise maxSize is reduced by \a sizeDenotation.
 */
#define CHECK_MAX_SIZE(sizeDenotation)                                                                                                               \
    if (maxSize < sizeDenotation) {                                                                                                                  \
        throw TruncatedDataException();                                                                                                              \
    } else {                                                                                                                                         \
        maxSize -= sizeDenotation;                                                                                                                   \
    }

} // namespace TagParser

#endif // TAG_PARSER_EXCEPTIONS_H
