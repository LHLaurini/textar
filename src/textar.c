#include "textar.h"
#include "textar-internals.h"
#include "entry.h"
#include "list.h"
#include "path.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

bool textArCreateArchive(AppendArchiveFn append_archive,
						 EntryIteratorFn entry_iterator,
						 TextArOptions options, VerboseFn verbose,
						 void* userPtr)
{
	textArClearError();

	const TextArEntry* entry = NULL;

	while (entry = entry_iterator(entry, userPtr))
	{
		if (entry == (TextArEntry*)-1)
		{
			textArSetError("failed to iterate through entries");
			return false;
		}

		const char* path = fixPath(entry->path);
		verbose(entry->path);

		append_archive(">>> \"", userPtr);
		append_archive(path, userPtr);

		if (entry->type == TEXTARENTRYTYPE_DIRECTORY && path[strlen(path) - 1] != '/')
		{
			append_archive("/\"", userPtr);
		}
		else
		{
			append_archive("\"", userPtr);
		}

		if (entry->type == TEXTARENTRYTYPE_SYMLINK)
		{
			append_archive(" =>", userPtr);
		}

		if (options & TEXTAROPTIONS_MODE)
		{
			append_archive(" mode=", userPtr);
			append_archive(modeToString(entry->mode), userPtr);
		}

		if (options & TEXTAROPTIONS_OWNID)
		{
			append_archive(" ownerid=", userPtr);
			append_archive(ownerIdToString(entry->ownerId, entry->groupId), userPtr);
		}

		if (options & TEXTAROPTIONS_OWNNAME)
		{
			append_archive(" ownername=", userPtr);
			append_archive(entry->owner, userPtr);
			append_archive(":", userPtr);
			append_archive(entry->group, userPtr);
		}

		if (entry->type == TEXTARENTRYTYPE_FILE ||
		    entry->type == TEXTARENTRYTYPE_SYMLINK)
		{
			append_archive("\n\n", userPtr);
			append_archive(entry->data, userPtr);
		}

		append_archive("\n\n", userPtr);
	}

	return true;
}

static bool closeEntryHelper(TextArEntry* theOpenEntry, IOFn close_entry,
                             size_t numEmptyLines, void* userPtr)
{
	size_t linesToErase = numEmptyLines + 1;
	if (linesToErase > 2)
	{
		linesToErase = 2;
	}

	theOpenEntry->data = (char*)&linesToErase;
	if (!close_entry(theOpenEntry, userPtr))
	{
		textArSetError("failed to close entry");
		return false;
	}
	return true;
}

bool textArExtractArchive(IOFn open_entry, IOFn append_entry, IOFn close_entry,
                          ReadArchiveLineFn read_archive_line,
                          TextArOptions options, VerboseFn verbose, void* userPtr)
{
	textArClearError();

	char* line;
	TextArEntry theOpenEntry;
	bool entryIsOpen = false;
	bool isFirstLineOfEntry = false;
	size_t numEmptyLines = 0;

	char originalNameEnd;
	char* nameEnd = NULL;

	while (line = read_archive_line(userPtr))
	{
		if (line[0] == '>' && line[1] == '>' && line[2] == '>')
		{
			if (line[3] == '>')	// Is an escaped entry (not an actual entry)
			{
				if (entryIsOpen)
				{
					theOpenEntry.data = line + 1;

					if (!append_entry(&theOpenEntry, userPtr))
					{
						textArSetError("failed to extract entry");
						goto error;
					}
				}
				else
				{
					// This is ok. See below.
				}
			}
			else
			{
				if (entryIsOpen)
				{
					entryIsOpen = false;
					if (!closeEntryHelper(&theOpenEntry, close_entry, numEmptyLines, userPtr))
					{
						goto error;
					}
				}

				theOpenEntry.type = TEXTARENTRYTYPE_FILE;
				theOpenEntry.group = NULL;
				theOpenEntry.owner = NULL;
				theOpenEntry.groupId = -1;
				theOpenEntry.ownerId = -1;
				theOpenEntry.mode = -1;
				theOpenEntry.userPtr = NULL;

				char* current = skipWhitespace(&line[3]);

				char* name = current;

				if (name[0] == '"')
				{
					name++;

					if ((nameEnd = findLastOf(name, '"')))
					{
						originalNameEnd = *nameEnd;
						*nameEnd = 0;
						current = nameEnd + 1;
					}
					else
					{
						name--;
						if (!(nameEnd = findWhitespace(current)))
						{
							nameEnd = findEnd(current);
						}
						originalNameEnd = *nameEnd;
						*nameEnd = 0;
						current = nameEnd + 1;
					}
				}
				else
				{
					name = current;
					if (!(nameEnd = findWhitespace(current)))
					{
						nameEnd = findEnd(current);
					}
					originalNameEnd = *nameEnd;
					*nameEnd = 0;
					current = nameEnd + 1;
				}

				if (nameEnd[-1] == '/')
				{
					theOpenEntry.type = TEXTARENTRYTYPE_DIRECTORY;
				}

				theOpenEntry.path = name;

				verbose(theOpenEntry.path);

				if (originalNameEnd != 0 && originalNameEnd != '\n')
				{
					while (true)
					{
						current = skipWhitespace(current);

						if (*current == 0 || *current == '\n')
						{
							break;
						}
						else if (current[0] == '=' && current[1] == '>')
						{
							if (theOpenEntry.type == TEXTARENTRYTYPE_FILE)
							{
								char* next = findWhitespace(current);
								if (next)
								{
									current = next;
								}
								else
								{
									current = findEnd(current);
								}
								theOpenEntry.type = TEXTARENTRYTYPE_SYMLINK;
								continue;
							}
							else
							{
								errno = EINVAL;
								textArSetError("conflicting entry type");
								goto error;
							}
						}

						char* propEnd = findFirstOf(current, '=');
						char* valueEnd = findWhitespace(current);

						if (!valueEnd)
						{
							valueEnd = findEnd(current);
						}

						if (!propEnd || propEnd > valueEnd)
						{
							errno = EINVAL;
							textArSetError("invalid entry syntax");
							goto error;
						}

						size_t propLen = propEnd - current;
						const char* value = propEnd + 1;
						size_t valueLen = valueEnd - propEnd - 1;

						if (propLen == 7 && strncmp(current, "ownerid", propLen) == 0)
						{
							if (options & TEXTAROPTIONS_OWNID)
							{
								if (!stringToOwnerId(value, valueLen, &theOpenEntry.groupId,
								                     &theOpenEntry.ownerId))
								{
									errno = EINVAL;
									textArSetError("invalid owner id");
									goto error;
								}
							}
						}
						else if (propLen == 9 && strncmp(current, "ownername", propLen) == 0)
						{
							if (options & TEXTAROPTIONS_OWNNAME)
							{
								static char groupName[33];
								static char userName[33];

								const char* userStr;

								if (!stringToOwner(value, valueLen, &userStr))
								{
									errno = EINVAL;
									textArSetError("invalid owner name");
									goto error;
								}

								size_t groupLen = userStr - value - 1;
								size_t userLen = valueLen - (userStr - value);

								memcpy(groupName, value, groupLen);
								groupName[groupLen] = 0;
								memcpy(userName, userStr, userLen);
								userName[userLen] = 0;

								theOpenEntry.group = groupName;
								theOpenEntry.owner = userName;
							}
						}
						else if (propLen == 4 && strncmp(current, "mode", propLen) == 0)
						{
							if (options & TEXTAROPTIONS_MODE)
							{
								if (!stringToMode(value, valueLen, &theOpenEntry.mode))
								{
									errno = EINVAL;
									textArSetError("invalid mode");
									goto error;
								}
							}
						}
						else
						{
							errno = EINVAL;
							textArSetError("unknown attribute");
							goto error;
						}

						current = valueEnd;
					}
				}

				if (!open_entry(&theOpenEntry, userPtr))
				{
					textArSetError("failed to open entry");
					goto error;
				}

				entryIsOpen = true;
				isFirstLineOfEntry = true;
				numEmptyLines = 0;
				
				*nameEnd = originalNameEnd;
				nameEnd = NULL;
			}
		}
		else
		{
			if (entryIsOpen)
			{
				if (!(isFirstLineOfEntry && line[0] == '\n'))
				{
					theOpenEntry.data = line;

					if (!append_entry(&theOpenEntry, userPtr))
					{
						textArSetError("failed to extract entry");
						goto error;
					}
					
					if (line[0] == '\n')
					{
						if (numEmptyLines < 2)
						{
							numEmptyLines++;
						}
					}
					else
					{
						numEmptyLines = 0;
					}
				}
				isFirstLineOfEntry = false;
			}
			else
			{
				// Archives are allowed to have text before any entry. Do
				// nothing (or maybe display it?)
			}
		}
	}

	if (errno)
	{
		textArSetError("failed to read archive");
		goto error;
	}

	if (entryIsOpen)
	{
		entryIsOpen = false;
		if (!closeEntryHelper(&theOpenEntry, close_entry, numEmptyLines, userPtr))
		{
			goto error;
		}
	}

	return true;

error:
	if (nameEnd)
	{
		*nameEnd = originalNameEnd;
	}

	int errnoBkp = errno;

	if (entryIsOpen)
	{
		// Nothing we can do if it fails
		closeEntryHelper(&theOpenEntry, close_entry, numEmptyLines, userPtr);
		entryIsOpen = false;
	}

	errno = errnoBkp;
	return false;
}

const char* textArErrorDesc()
{
	return textArSetError(NULL);
}

const char* textArErrorFile()
{
	return textArSetErrorFile(NULL);
}

const char* textArSetError(const char* err)
{
	static const char* error = NULL;
	if (err)
	{
		if (!error)
		{
			error = err;
		}
		else if (!err[0])
		{
			error = NULL;
		}
	}
	return error;
}

void textArClearError()
{
	textArSetError("");
}

const char* textArSetErrorFile(const char* err)
{
	static char file[PATH_MAX] = "";
	if (err)
	{
		// Just truncate if larger than PATH_MAX
		strncpy(file, err, PATH_MAX - 1);
	}
	return (file[0] != 0) ? file : NULL;
}
