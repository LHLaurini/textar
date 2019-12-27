/// \file
/// \summary Internals only needed if making a custom implementation

#pragma once

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

/// Set an error or return a previously set error.
/// Once an non-NULL string is passed to this function, it can only be changed
/// after a call to ::textArClearError.
EXTERN const char* textArSetError(const char* err);
/// Clear error previously set by ::textArSetError
EXTERN void textArClearError();
/// Set the file that caused the error
EXTERN const char* textArSetErrorFile(const char* err);
