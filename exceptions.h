#ifndef MEDIA_EXCEPTIONS_H
#define MEDIA_EXCEPTIONS_H

#include <c++utilities/application/global.h>

#include <stdexcept>
#include <string>

namespace Media {

class LIB_EXPORT Failure : public std::exception
{
public:
    Failure() USE_NOTHROW;
    virtual ~Failure() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT NoDataFoundException : public Failure
{
public:
    NoDataFoundException() USE_NOTHROW;
    virtual ~NoDataFoundException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT InvalidDataException : public Failure
{
public:
    InvalidDataException() USE_NOTHROW;
    virtual ~InvalidDataException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT TruncatedDataException : public InvalidDataException
{
public:
    TruncatedDataException() USE_NOTHROW;
    virtual ~TruncatedDataException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT OperationAbortedException : public Failure
{
public:
    OperationAbortedException() USE_NOTHROW;
    virtual ~OperationAbortedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT VersionNotSupportedException : public Failure
{
public:
    VersionNotSupportedException() USE_NOTHROW;
    virtual ~VersionNotSupportedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

class LIB_EXPORT NotImplementedException : public Failure
{
public:
    NotImplementedException() USE_NOTHROW;
    virtual ~NotImplementedException() USE_NOTHROW;
    virtual const char *what() const USE_NOTHROW;
};

}

#endif // MEDIA_EXCEPTIONS_H
