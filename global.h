// Created via CMake from template global.h.in
// WARNING! Any changes to this file will be overwritten by the next CMake run!

#ifndef TAG_PARSER_GLOBAL
#define TAG_PARSER_GLOBAL

#include <c++utilities/application/global.h>

#ifdef TAG_PARSER_STATIC
# define TAG_PARSER_EXPORT
# define TAG_PARSER_IMPORT
#else
# define TAG_PARSER_EXPORT LIB_EXPORT
# define TAG_PARSER_IMPORT LIB_IMPORT
#endif

#endif // TAG_PARSER_GLOBAL
