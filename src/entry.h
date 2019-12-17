#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

size_t findLength(const char* line);
char* findEnd(char* line);
char* findFirstOf(char* line, char c);
char* findLastOf(char* line, char c);
char* findWhitespace(char* line);
char* skipWhitespace(char* line);

const char* modeToString(mode_t mode);
const char* uint32ToString(uint32_t i);
const char* ownerIdToString(uid_t uid, gid_t gid);
bool stringToMode(const char* string, size_t stringLen, mode_t* modeOut);
bool stringToOwnerId(const char* string, size_t stringLen, gid_t* groupOut, uid_t* userOut);
bool stringToOwner(const char* string, size_t stringLen, const char** userOut);
