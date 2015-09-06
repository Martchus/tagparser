#include "./exceptions.h"

using namespace std;

namespace Media {

/*!
 * \class Media::Failure
 * \brief The class inherits from std::exception and serves as base class for exceptions
 *        thrown by the elements of the Media namespace.
 */

/*!
 * \brief Constructs a new exception.
 */
Failure::Failure() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
Failure::~Failure() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *Failure::what() const USE_NOTHROW
{
    return "unable to parse given data";
}

/*!
 * \class Media::NoDataFoundException
 * \brief The exception that is thrown when the data to be parsed holds no
 *        parsable information.
 */

/*!
 * \brief Constructs a new exception.
 */
NoDataFoundException::NoDataFoundException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
NoDataFoundException::~NoDataFoundException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *NoDataFoundException::what() const USE_NOTHROW
{
    return "no parsable data has been found";
}

/*!
 * \class Media::InvalidDataException
 * \brief The exception that is thrown when the data to be parsed or to be made seems
 *        invalid and therefore can not be parsed.
 */

/*!
 * \brief Constructs a new exception.
 */
InvalidDataException::InvalidDataException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
InvalidDataException::~InvalidDataException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *InvalidDataException::what() const USE_NOTHROW
{
    return "data to be parsed or to be made seems to be invalid";
}

/*!
 * \class Media::TruncatedDataException
 * \brief The exception that is thrown when the data to be parsed is truncated
 *        and therefore can not be parsed at all.
 */

/*!
 * \brief Constructs a new exception.
 */
TruncatedDataException::TruncatedDataException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
TruncatedDataException::~TruncatedDataException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *TruncatedDataException::what() const USE_NOTHROW
{
    return "data to be parsed seems to be truncated";
}

/*!
 * \class Media::OperationAbortedException
 * \brief The exception that is thrown when an operation has been stopped
 *        and thus not sucessfully completed because it has been aborted.
 */

/*!
 * \brief Constructs a new exception.
 */
OperationAbortedException::OperationAbortedException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
OperationAbortedException::~OperationAbortedException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *OperationAbortedException::what() const USE_NOTHROW
{
    return "operation has been aborted";
}

/*!
 * \class Media::VersionNotSupportedException
 * \brief The exception that is thrown when an operation fails because the
 *        detected or specified version is not supported by the implementation.
 */

/*!
 * \brief Constructs a new exception.
 */
VersionNotSupportedException::VersionNotSupportedException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
VersionNotSupportedException::~VersionNotSupportedException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *VersionNotSupportedException::what() const USE_NOTHROW
{
    return "the version of the data to be parsed is not supported";
}

/*!
 * \class Media::NotImplementedException
 * \brief This exception is thrown when the an operation is invoked that has not
 *        been implemented yet.
 */

/*!
 * \brief Constructs a new exception.
 */
NotImplementedException::NotImplementedException() USE_NOTHROW
{}

/*!
 * \brief Destroys the exception.
 */
NotImplementedException::~NotImplementedException() USE_NOTHROW
{}

/*!
 * \brief Returns a C-style character string describing the cause of the exception.
 */
const char *NotImplementedException::what() const USE_NOTHROW
{
    return "the operation has not been implemented yet";
}

}
