#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct List List;

List* listCreate();
void* listAppend(List* list, size_t elementSize);
void* listInsert(List* list, const void* afterElement, size_t elementSize);
void* listIterate(List* list, const void* element);
void listFree(List** list);
