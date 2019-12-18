/// \file
/// \brief Main header

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
	TEXTAROPTIONS_MODE = 1 << 0,    ///< Store/extract entry mode
	TEXTAROPTIONS_OWNID = 1 << 1,   ///< Store/extract entry owner id
	TEXTAROPTIONS_OWNNAME = 1 << 2, ///< Store/extract entry owner name
} TextArOptions;

typedef struct TextArEntry
{
	TextArEntryType type;   ///< Type of entry
	const char* path;       ///< Path of the entry
	const char* owner;      ///< Owner name, or \c NULL if not available
	uid_t ownerId;          ///< Owner UID, or (uid_t)-1 if not available
	const char* group;      ///< Owner group name, or \c NULL if not available
	gid_t groupId;          ///< Owner GID, or (gid_t)-1 if not available
	mode_t mode;            ///< File mode, or (mode_t)-1 if not available
	const char* data;       ///< Meaning depends on context (usually a fragment of entry data)
	void* userPtr;	        ///< User data. Different from the argument in the functions
} TextArEntry;

/// Prototype for \c verbose
typedef void (*VerboseFn)(const char* message);
/// Prototype for \c append_archive
typedef bool (*AppendArchiveFn)(const char* data, void* userPtr);
/// Prototype for \c entry_iterator
typedef const TextArEntry* (*EntryIteratorFn)(const TextArEntry* entry, void* userPtr);
/// Prototype for \c open_entry, \c append_entry and \c close_entry
typedef bool (*IOFn)(TextArEntry* entry, void* userPtr);
/// Prototype for \c read_archive_line
typedef char* (*ReadArchiveLineFn)(void* userPtr);

/// Create an archive using a custom implementation. This function allocates no
/// memory directly.
/// \p append_archive must be defined according to the following:
/// - The NULL-terminated string \p data must be appended to the output archive;
/// - \c true must be returned if the operation succeeded or \c false otherwise.
///
/// \p entry_iterator must be defined according to the following:
/// - A pointer to a TextArEntry structure representing the next entry must be
///   returned, or \c NULL, if no more entries are present;
/// - The underlying container is traversed only once;
/// - Each entry must be returned once;
/// - The returned pointer does **not** have to be unique;
/// - If \p entry is \c NULL, the next entry is the first;
/// - \c (TextArEntry*)-1 should be returned in case of error;
/// - ::textArSetError can optionally be called to signal a more meaningful error.
///
/// \param append_archive Called to append data to the archive
/// \param entry_iterator Called to iterate through entries
/// \param options        TextArOptions OR-ed together
/// \param verbose        Function to call to show messages to the user
/// \param userPtr        Data passed to implementation
/// \return \c true if succeeded, \c false otherwise. Use ::textArErrorDesc,
///         ::textArErrorFile and \c errno for details.
bool textArCreateArchive(AppendArchiveFn append_archive,
                         EntryIteratorFn entry_iterator,
                         TextArOptions options, VerboseFn verbose,
                         void* userPtr);
/// Create an archive using the default implementation.
/// \param fileName File name (and path) of the archive
/// \param entries  NULL-terminated array of strings to each entry
/// \param options  TextArOptions OR-ed together
/// \param verbose  Function to call to show messages to the user
/// \return \c true if succeeded, \c false otherwise. Use ::textArErrorDesc,
///         ::textArErrorFile and \c errno for details.
bool textArCreateArchiveFile(const char* fileName, const char* const * entries,
                             TextArOptions options, VerboseFn verbose);

/// Extract an archive using a custom implementation. This function allocates no
/// memory directly.
/// \p open_entry must be defined according to the following:
/// - The entry must be ready for appending after this function is called;
/// - The \c path field is only guaranteed to be valid during this function call;
/// - The \c data field has no meaning;
/// - \c true must be returned if the operation succeeded or \c false otherwise.
///
/// \p append_entry must be defined according to the following:
/// - The string in the field \c data must be appended to the entry;
/// - The string in \c data may be either NULL-terminated or LF-terminated;
/// - The \c path field may not be valid during this function call;
/// - The \c entry pointer may or may not be the same as in \p open_entry;
/// - Calls can be ignored if \c type is TEXTARENTRYTYPE_DIRECTORY;
/// - If \c type is TEXTARENTRYTYPE_SYMLINK, the target is contained in \c data;
/// - Further calls can be ignored if \c type is TEXTARENTRYTYPE_SYMLINK;
/// - \c true must be returned if the operation succeeded or \c false otherwise.
///
/// \p close_entry must be defined according to the following:
/// - No more data will be appended to the entry after this function;
/// - The \c path field may not be valid during this function call;
/// - The \c data field is a pointer to a size_t containing the number of bytes
///   which should be discarded at the end of the file;
/// - \c true must be returned if the operation succeeded or \c false otherwise.
///
/// \p read_archive_line must be defined according to the following:
/// - The function must return a string containing the next line in the archive;
/// - If no more lines are available, \c NULL should be returned;
/// - The returned string may be either NULL-terminated or LF-terminated.
///
/// \param open_entry        Called to open the entry
/// \param append_entry      Called to append data to the entry
/// \param close_entry       Called to close the entry
/// \param read_archive_line Called to read the next line from the archive
/// \param options        TextArOptions OR-ed together
/// \param verbose        Function to call to show messages to the user
/// \param userPtr        Data passed to implementation
/// \return \c true if succeeded, \c false otherwise. Use ::textArErrorDesc,
///         ::textArErrorFile and \c errno for details.
bool textArExtractArchive(IOFn open_entry,
                          IOFn append_entry,
                          IOFn close_entry,
                          ReadArchiveLineFn read_archive_line,
                          TextArOptions options, VerboseFn verbose,
                          void* userPtr);
/// Create an archive using the default implementation.
/// \param fileName File name (and path) of the archive
/// \param entries  NULL-terminated array of strings to each entry
/// \param options  TextArOptions OR-ed together
/// \param verbose  Function to call to show messages to the user
/// \return \c true if succeeded, \c false otherwise. Use ::textArErrorDesc,
///         ::textArErrorFile and \c errno for details.
bool textArExtractArchiveFile(const char* fileName, TextArOptions options, VerboseFn verbose);

/// Return a string describing the last error.
const char* textArErrorDesc();
/// Return a string to the path of the file that caused the error.
const char* textArErrorFile();
