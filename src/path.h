#pragma once

#include <stdint.h>
#include <sys/stat.h>

const char* removeRoot(const char* path);
const char* findComponentEnd(const char* path);
const char* findNextComponent(const char* path);
const char* fixPath(const char* path);
