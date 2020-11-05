#include "./exceptions.h"

using namespace std;

namespace TagParser {

/*!
 * \class TagParser::Failure
 * \brief The class inherits from std::exception and serves as base class for exceptions
 *        thrown by the elements of the Media namespace.
 */

/*!
 * \brief Constructs a new exception.
 */
Failure::Failure() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
Failure::~Failure() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *Failure::what() const noexcept
{
    return "unable to parse given data";
}

/*!
 * \class TagParser::NoDataFoundException
 * \brief The exception that is thrown when the data to be parsed holds no
 *        parsable information (e.g. relevant section in the file does not exist or
 *        has size of zero).
 */

/*!
 * \brief Constructs a new exception.
 */
NoDataFoundException::NoDataFoundException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
NoDataFoundException::~NoDataFoundException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *NoDataFoundException::what() const noexcept
{
    return "no parsable data has been found";
}

/*!
 * \class TagParser::InvalidDataException
 * \brief The exception that is thrown when the data to be parsed or to be made seems
 *        invalid and therefore can not be parsed.
 */

/*!
 * \brief Constructs a new exception.
 */
InvalidDataException::InvalidDataException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
InvalidDataException::~InvalidDataException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *InvalidDataException::what() const noexcept
{
    return "data to be parsed or to be made seems to be invalid";
}

/*!
 * \class TagParser::NoDataProvidedException
 * \brief The exception that is thrown when the value to be written is empty but that
 *        is not allowed in that context (e.g. an empty ID3v2 frame is not allowed).
 */

/*!
 * \brief Constructs a new exception.
 */
NoDataProvidedException::NoDataProvidedException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
NoDataProvidedException::~NoDataProvidedException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *NoDataProvidedException::what() const noexcept
{
    return "can not write empty value";
}

/*!
 * \class TagParser::TruncatedDataException
 * \brief The exception that is thrown when the data to be parsed is truncated
 *        and therefore can not be parsed at all.
 */

/*!
 * \brief Constructs a new exception.
 */
TruncatedDataException::TruncatedDataException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
TruncatedDataException::~TruncatedDataException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *TruncatedDataException::what() const noexcept
{
    return "data to be parsed seems to be truncated";
}

/*!
 * \class TagParser::OperationAbortedException
 * \brief The exception that is thrown when an operation has been stopped
 *        and thus not successfully completed because it has been aborted.
 */

/*!
 * \brief Constructs a new exception.
 */
OperationAbortedException::OperationAbortedException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
OperationAbortedException::~OperationAbortedException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *OperationAbortedException::what() const noexcept
{
    return "operation has been aborted";
}

/*!
 * \class TagParser::VersionNotSupportedException
 * \brief The exception that is thrown when an operation fails because the
 *        detected or specified version is not supported by the implementation.
 */

/*!
 * \brief Constructs a new exception.
 */
VersionNotSupportedException::VersionNotSupportedException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
VersionNotSupportedException::~VersionNotSupportedException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *VersionNotSupportedException::what() const noexcept
{
    return "the version of the data to be parsed is not supported";
}

/*!
 * \class TagParser::NotImplementedException
 * \brief This exception is thrown when the an operation is invoked that has not
 *        been implemented yet.
 */

/*!
 * \brief Constructs a new exception.
 */
NotImplementedException::NotImplementedException() noexcept
{
}

/*!
 * \brief Destroys the exception.
 */
NotImplementedException::~NotImplementedException() noexcept
{
}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *NotImplementedException::what() const noexcept
{
    return "the operation has not been implemented yet";
}

} // namespace TagParser
