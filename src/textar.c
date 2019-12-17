#include "textar.h"
#include "entry.h"
#include "list.h"
#include "path.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// O_NOFOLLOW is important: unlinking the entry doesn't guarantee a link won't be created between
// unlink and open
#define CREATE_FLAGS O_CREAT | O_WRONLY | O_TRUNC | O_NOFOLLOW

static const char* setError(const char* err) 
{
	static const char* error;
	if (err)
	{
		error = err;
	}
	return error;
}

static const char* setErrorFile(const char* err) 
{
	static char file[PATH_MAX] = "";
	if (err)
	{
		// Just truncate if larger than PATH_MAX
		strncpy(file, err, PATH_MAX - 1);
	}
	return (file[0] != 0) ? file : NULL;
}

bool textArCreateArchive(AppendArchiveFn append_archive,
						 EntryIteratorFn entry_iterator,
						 TextArOptions options, VerboseFn verbose,
						 void* userPtr)
{
	const TextArEntry* entry = NULL;

	while (entry = entry_iterator(entry, userPtr))
	{
		if (entry == (TextArEntry*)-1)
		{
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

struct UserData
{
	List* entryPathList;
	FILE* outputFile;
};

bool textArCreateArchiveFile(const char* fileName, const char* const * entries,
                             TextArOptions options, VerboseFn verbose)
{
	setErrorFile(NULL);
	List* entryPathList = listCreate();

	for (const char* entry; (entry = *entries); entries++)
	{
		size_t length = strlen(entry);
		char* last = (char*)listAppend(entryPathList, length + 1);
		strcpy(last, entry);
	}

	FILE* outputFile = fopen(fileName, "w");

	struct UserData userData = { entryPathList, outputFile };

	bool success = textArCreateArchive(appendArchiveFile, entryIteratorFile,
	                                   options, verbose, &userData);
	int funcErrno = errno;

	if (fclose(outputFile) && success)
	{
		funcErrno = errno;
		setError("failed to close archive");
		success = false;
	}

	listFree(&entryPathList);

	errno = funcErrno;
	return success;
}

bool textArExtractArchive(IOFn open_entry, IOFn append_entry, IOFn close_entry,
                          ReadArchiveLineFn read_archive_line,
                          TextArOptions options, VerboseFn verbose, void* userPtr)
{
	char* line;
	TextArEntry theOpenEntry;
	bool entryIsOpen = false;
	bool isFirstLineOfEntry = false;
	size_t numEmptyLines = 0;

	char originalNameEnd;
	char* nameEnd = NULL;
	
	/// \note: line can be either LF or NULL terminated

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
						setError("failed to extract entry");
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
					theOpenEntry.data = (char*)&numEmptyLines;
					if (!close_entry(&theOpenEntry, userPtr))
					{
						setError("failed to close entry");
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
								setError("conflicting entry type");
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
							setError("invalid entry syntax");
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
									setError("invalid owner id");
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
									setError("invalid owner name");
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
									setError("invalid mode");
									goto error;
								}
							}
						}
						else
						{
							errno = EINVAL;
							setError("unknown attribute");
							goto error;
						}

						current = valueEnd;
					}
				}

				if (!open_entry(&theOpenEntry, userPtr))
				{
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
						setError("failed to extract entry");
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
		setError("failed to read archive");
		goto error;
	}

	if (entryIsOpen)
	{
		entryIsOpen = false;
		theOpenEntry.data = (char*)&numEmptyLines;
		if (!close_entry(&theOpenEntry, userPtr))
		{
			setError("failed to close entry");
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
		theOpenEntry.data = (char*)&numEmptyLines;
		close_entry(&theOpenEntry, userPtr);
		entryIsOpen = false;
	}

	errno = errnoBkp;
	return false;
}

struct UserData2
{
	FILE* inputFile;
	int entryFile;
	char* lineBuffer;
	size_t lineBufferSize;
	char* entryName;
};

bool textArExtractArchiveFile(const char* fileName, TextArOptions options, VerboseFn verbose)
{
	setErrorFile(NULL);

	FILE* inputFile = fopen(fileName, "r");
	if (!inputFile)
	{
		setError("failed to open archive");
		return false;
	}

	struct UserData2 userData = { inputFile, -1, NULL, 0, NULL };

	bool success = textArExtractArchive(openEntry, appendEntry, closeEntry, readArchiveLine,
	                                    options, verbose, &userData);
	int funcErrno = errno;

	if (fclose(inputFile) && success)
	{
		funcErrno = errno;
		setError("failed to close archive");
		success = false;
	}

	if (userData.lineBuffer)
	{
		free(userData.lineBuffer);
		userData.lineBuffer = NULL;
	}

	if (userData.entryName)
	{
		free(userData.entryName);
		userData.entryName = NULL;
	}

	errno = funcErrno;
	return success;
}

bool appendArchiveFile(const char* data, void* userPtr)
{
	struct UserData* userData = (struct UserData*)userPtr;
	FILE* outputFile = userData->outputFile;
	return fwrite(data, strlen(data), 1, outputFile);
}

const TextArEntry* entryIteratorFile(const TextArEntry* entry, void* userPtr)
{
	struct UserData* userData = (struct UserData*)userPtr;
	List* entryPathList = userData->entryPathList;

	static TextArEntry entryDesc = { 0 };

	entryDesc.path = (char*)listIterate(entryPathList, entry ? entry->path : NULL);
	if (!entryDesc.path)
	{
		free((void*)entryDesc.data);
		entryDesc.data = NULL;
		return NULL;
	}

	setErrorFile(entryDesc.path);
	
	struct stat entryStat;

	if (lstat(entryDesc.path, &entryStat))
	{
		free((void*)entryDesc.data);
		entryDesc.data = NULL;
		return (TextArEntry*)-1;
	}

	switch (entryStat.st_mode & S_IFMT)
	{
	case S_IFREG:
	{
		entryDesc.type = TEXTARENTRYTYPE_FILE;

		FILE* file = fopen(entryDesc.path, "r");
		if (!file)
		{
			free((void*)entryDesc.data);
			entryDesc.data = NULL;
			return (TextArEntry*)-1;
		}

		char* data = (char*)realloc((void*)entryDesc.data, entryStat.st_size + 1);
		fread(data, entryStat.st_size, 1, file);
		data[entryStat.st_size] = 0;
		entryDesc.data = data;
		fclose(file);

		if (strlen(data) != entryStat.st_size)
		{
			free((void*)entryDesc.data);
			entryDesc.data = NULL;
			errno = EPERM;
			setError("refused to archive binary file");
			return (TextArEntry*)-1;
		}

		break;
	}

	case S_IFLNK:
	{
		entryDesc.type = TEXTARENTRYTYPE_SYMLINK;
		char* data = (char*)realloc((void*)entryDesc.data, entryStat.st_size + 1);
		if (readlink(entryDesc.path, data, entryStat.st_size) == -1)
		{
			free(data);
			entryDesc.data = NULL;
			return (TextArEntry*)-1;
		}
		data[entryStat.st_size] = 0;
		entryDesc.data = data;
		break;
	}

	case S_IFDIR:
	{
		entryDesc.type = TEXTARENTRYTYPE_DIRECTORY;
		DIR* dir = opendir(entryDesc.path);
		if (!dir)
		{
			free((void*)entryDesc.data);
			entryDesc.data = NULL;
			return (TextArEntry*)-1;
		}

		errno = 0;
		struct dirent* dirent;

		char* last = (char*)entryDesc.path;

		while ((dirent = readdir(dir)))
		{
			if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
			{
				int dirLen = strlen(entryDesc.path);
				int fileLen = strlen(dirent->d_name);
				last = (char*)listInsert(entryPathList, last, dirLen + fileLen + 2);
				strcpy(last, entryDesc.path);
				if (last[dirLen - 1] != '/')
				{
					strcat(last, "/");
				}
				strcat(last, dirent->d_name);
			}
		}

		if (errno != 0)
		{
			free((void*)entryDesc.data);
			entryDesc.data = NULL;
			return (TextArEntry*)-1;
		}
		
		closedir(dir);
		break;
	}
	
	default:
		free((void*)entryDesc.data);
		entryDesc.data = NULL;
		errno = ENOTSUP;
		setError("file type not supported");
		return (TextArEntry*)-1;
	}

	errno = 0;

	struct group* group = getgrgid(entryStat.st_gid);
	if (!group)
	{
		free((void*)entryDesc.data);
		entryDesc.data = NULL;
		return (TextArEntry*)-1;
	}

	struct passwd* passwd = getpwuid(entryStat.st_uid);
	if (!passwd)
	{
		free((void*)entryDesc.data);
		entryDesc.data = NULL;
		return (TextArEntry*)-1;
	}

	entryDesc.mode = entryStat.st_mode & 07777;
	entryDesc.groupId = entryStat.st_gid;
	entryDesc.group = group->gr_name;
	entryDesc.ownerId = entryStat.st_uid;
	entryDesc.owner = passwd->pw_name;

	return &entryDesc;
}

bool chownEntry(TextArEntry* entry, const char* entryName)
{
	gid_t gid = -1;

	if (entry->groupId != -1)
	{
		gid = entry->groupId;
	}
	else if (entry->group)
	{
		errno = 0;
		struct group* group = getgrnam(entry->group);
		if (group)
		{
			gid = group->gr_gid;
		}
		else
		{
			return false;
		}
	}

	uid_t uid = -1;

	if (entry->ownerId != -1)
	{
		uid = entry->ownerId;
	}
	else if (entry->owner)
	{
		errno = 0;
		struct passwd* user = getpwnam(entry->owner);
		if (user)
		{
			uid = user->pw_uid;
		}
		else
		{
			return false;
		}
	}

	if (gid != -1 && uid != -1)
	{
		if (lchown(entryName, gid, uid))
		{
			return false;
		}
	}

	return true;
}

bool openEntry(TextArEntry* entry, void* userPtr)
{
	struct UserData2* userData = (struct UserData2*)userPtr;

	// TODO: Probably best to stat the entry instead of relying on error checking

	if (unlink(entry->path))
	{
		if (errno == ENOENT)
		{
		}
		else if (errno == EISDIR)
		{
			if (entry->type != TEXTARENTRYTYPE_DIRECTORY)
			{
				if (rmdir(entry->path))
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	switch (entry->type)
	{
	case TEXTARENTRYTYPE_DIRECTORY:
		if (entry->mode != -1)
		{
			mode_t oldMask = umask(0);
			userData->entryFile = mkdir(entry->path, entry->mode);
			umask(oldMask);
		}
		else
		{
			userData->entryFile = mkdir(entry->path, 0777);
		}

		if (userData->entryFile == -1)	// not really a fd
		{
			if (errno == EEXIST)
			{
				if (entry->mode != -1)
				{
					if (chmod(entry->path, entry->mode))
					{
						return false;
					}
				}
			}
			else
			{
				return false;
			}
		}

		break;

	case TEXTARENTRYTYPE_FILE:
		if (entry->mode != -1)
		{
			mode_t oldMask = umask(0);
			userData->entryFile = open(entry->path, CREATE_FLAGS, entry->mode);
			umask(oldMask);
		}
		else
		{
			userData->entryFile = open(entry->path, CREATE_FLAGS, 0666);
		}

		if (userData->entryFile == -1)
		{
			return false;
		}

		break;

	case TEXTARENTRYTYPE_SYMLINK:
		entry->userPtr = NULL;
		userData->entryName = realloc(userData->entryName, strlen(entry->path));
		strcpy(userData->entryName, entry->path);
		break;

	default:
		abort();
	}

	return entry->type == TEXTARENTRYTYPE_SYMLINK || chownEntry(entry, entry->path);
}

bool appendEntry(TextArEntry* entry, void* userPtr)
{
	struct UserData2* userData = (struct UserData2*)userPtr;

	switch (entry->type)
	{
	case TEXTARENTRYTYPE_DIRECTORY:
		// Ignore it. Leave an opportunity for comments
		break;

	case TEXTARENTRYTYPE_FILE:
	{
		size_t length = findLength(entry->data);

		if (entry->data[length] == '\n')
		{
			length++;
		}

		if (write(userData->entryFile, entry->data, length) != length)
		{
			return false;
		}
		break;
	}
	
	case TEXTARENTRYTYPE_SYMLINK:
		if (entry->userPtr != (void*)-1)
		{
			size_t length = findLength(entry->data);
			char* dest = malloc(length + 1);
			memcpy(dest, entry->data, length);
			dest[length] = 0;

			userData->entryFile = symlink(dest, userData->entryName);

			free(dest);

			if (userData->entryFile == -1)
			{
				return false;
			}

			entry->userPtr = (void*)-1;
		}
		break;

	default:
		abort();
	}

	return entry->type != TEXTARENTRYTYPE_SYMLINK || chownEntry(entry, userData->entryName);
}

bool closeEntry(TextArEntry* entry, void* userPtr)
{
	struct UserData2* userData = (struct UserData2*)userPtr;

	switch (entry->type)
	{
	case TEXTARENTRYTYPE_DIRECTORY:
		// Do nothing
		break;

	case TEXTARENTRYTYPE_FILE:
	{
		off_t size = lseek(userData->entryFile, 0, SEEK_CUR);
		if (size == (off_t)-1)
		{
			close(userData->entryFile);
			userData->entryFile = -1;
			return false;
		}

		size_t bytesToTruncate = *(size_t*)entry->data;

		if (ftruncate(userData->entryFile, size - bytesToTruncate))
		{
			close(userData->entryFile);
			userData->entryFile = -1;
			return false;
		}

		if (close(userData->entryFile))
		{
			userData->entryFile = -1;
			return false;
		}
		userData->entryFile = -1;
		break;
	}

	case TEXTARENTRYTYPE_SYMLINK:
		// Do nothing
		break;

	default:
		abort();
	}

	return true;
}

char* readArchiveLine(void* userPtr)
{
	struct UserData2* userData = (struct UserData2*)userPtr;

	errno = 0;
	ssize_t length = getline(&userData->lineBuffer, &userData->lineBufferSize, userData->inputFile);

	if (length < 0)
	{
		return NULL;
	}

	return userData->lineBuffer;
}

const char* textArErrorDesc()
{
	return setError(NULL);
}

const char* textArErrorFile()
{
	return setErrorFile(NULL);
}
