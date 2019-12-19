/// \file
/// \summary Internals only needed if making a custom implementation

#pragma once

/// Set an error or return a previously set error.
/// Once an non-NULL string is passed to this function, it can only be changed
/// after a call to ::textArClearError.
const char* textArSetError(const char* err);
/// Clear error previously set by ::textArSetError
void textArClearError();
/// Set the file that caused the error
const char* textArSetErrorFile(const char* err);
