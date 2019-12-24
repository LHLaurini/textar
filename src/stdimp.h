/// \file
/// \brief Standard implementation.
/// \warning These functions may change at any time, without notice. Because of
///          that, it's not recommended to use them when making a custom
///          implementation. Instead, copy them and change what's needed.

#pragma once

#include "textar.h"
#include <stdbool.h>

bool stdImpAppendArchive(const char* data, void* userPtr);
const TextArEntry* stdImpEntryIterator(const TextArEntry* entry, void* userPtr);

bool stdImpOpenEntry(TextArEntry* entry, void* userPtr);
bool stdImpAppendEntry(TextArEntry* entry, void* userPtr);
bool stdImpCloseEntry(TextArEntry* entry, void* userPtr);
char* stdImpReadArchiveLine(void* userPtr);
char* stdImpFFMReadArchiveLine(void* userPtr);
