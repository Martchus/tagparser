// Created via CMake from template global.h.in
// WARNING! Any changes to this file will be overwritten by the next CMake run!

#ifndef TAG_PARSER_GLOBAL
#define TAG_PARSER_GLOBAL

#include "tagparser-definitions.h"
#include <c++utilities/application/global.h>

#ifdef TAG_PARSER_STATIC
#define TAG_PARSER_EXPORT
#define TAG_PARSER_IMPORT
#else
#define TAG_PARSER_EXPORT CPP_UTILITIES_GENERIC_LIB_EXPORT
#define TAG_PARSER_IMPORT CPP_UTILITIES_GENERIC_LIB_IMPORT
#endif

/*!
 * \def TAG_PARSER_EXPORT
 * \brief Marks the symbol to be exported by the tagparser library.
 */

/*!
 * \def TAG_PARSER_IMPORT
 * \brief Marks the symbol to be imported from the tagparser library.
 */

#endif // TAG_PARSER_GLOBAL
