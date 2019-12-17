#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum TextArEntryType
{
	TEXTARENTRYTYPE_FILE,
	TEXTARENTRYTYPE_DIRECTORY,
	TEXTARENTRYTYPE_SYMLINK,
} TextArEntryType;

typedef enum TextArOptions
{
	TEXTAROPTIONS_MODE = 1 << 0,
	TEXTAROPTIONS_OWNID = 1 << 1,
	TEXTAROPTIONS_OWNNAME = 1 << 2,
} TextArOptions;

typedef struct TextArEntry
{
	TextArEntryType type;
	const char* path;
	const char* owner;
	uid_t ownerId;
	const char* group;
	gid_t groupId;
	mode_t mode;
	const char* data;
	void* userPtr;
} TextArEntry;

typedef void (*VerboseFn)(const char*);
typedef bool (*AppendArchiveFn)(const char*, void*);
typedef const TextArEntry* (*EntryIteratorFn)(const TextArEntry*, void*);
typedef bool (*IOFn)(TextArEntry*, void*);
typedef char* (*ReadArchiveLineFn)(void*);

bool textArCreateArchive(AppendArchiveFn append_archive,
                         EntryIteratorFn entry_iterator,
                         TextArOptions options, VerboseFn verbose,
                         void* userPtr);
bool textArCreateArchiveFile(const char* fileName, const char* const * entries,
                             TextArOptions options, VerboseFn verbose);

bool textArExtractArchive(IOFn open_entry,
                          IOFn append_entry,
                          IOFn close_entry,
                          ReadArchiveLineFn read_archive_line,
                          TextArOptions options, VerboseFn verbose,
                          void* userPtr);
bool textArExtractArchiveFile(const char* fileName, TextArOptions options, VerboseFn verbose);

bool appendArchiveFile(const char* data, void* userPtr);
const TextArEntry* entryIteratorFile(const TextArEntry* entry, void* userPtr);

bool openEntry(TextArEntry* entry, void* userPtr);
bool appendEntry(TextArEntry* entry, void* userPtr);
bool closeEntry(TextArEntry* entry, void* userPtr);
char* readArchiveLine(void* userPtr);

const char* textArErrorDesc();
const char* textArErrorFile();
