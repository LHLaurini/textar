#include "stdimp.h"

#include "entry.h"
#include "list.h"
#include "textar-internals.h"

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

struct UserData
{
	List* entryPathList;
	FILE* outputFile;
};

bool textArCreateArchiveFile(const char* fileName, const char* const * entries,
                             int options, VerboseFn verbose)
{
	textArSetErrorFile(NULL);
	List* entryPathList = listCreate();

	for (const char* entry; (entry = *entries); entries++)
	{
		size_t length = strlen(entry);
		char* last = (char*)listAppend(entryPathList, length + 1);
		strcpy(last, entry);
	}

	FILE* outputFile = fopen(fileName, "w");

	struct UserData userData = { entryPathList, outputFile };

	bool success = textArCreateArchive(stdImpAppendArchive,
	                                   stdImpEntryIterator,
	                                   options, verbose, &userData);
	int funcErrno = errno;

	if (fclose(outputFile) && success)
	{
		funcErrno = errno;
		textArSetError("failed to close archive");
		success = false;
	}

	listFree(&entryPathList);

	errno = funcErrno;
	return success;
}

struct UserData2
{
	void* inputFile;
	int entryFile;
	char* lineBuffer;
	size_t lineBufferSize;
	char* entryName;
};

bool textArExtractArchiveFile(const char* fileName, int options, VerboseFn verbose)
{
	textArSetErrorFile(NULL);

	FILE* inputFile = fopen(fileName, "r");
	if (!inputFile)
	{
		textArSetError("failed to open archive");
		return false;
	}

	struct UserData2 userData = { inputFile, -1, NULL, 0, NULL };

	bool success = textArExtractArchive(stdImpOpenEntry, stdImpAppendEntry,
	                                    stdImpCloseEntry, stdImpReadArchiveLine,
	                                    options, verbose, &userData);
	int funcErrno = errno;

	if (fclose(inputFile) && success)
	{
		funcErrno = errno;
		textArSetError("failed to close archive");
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

bool textArExtractArchiveFileFromMemory(char* data, int options, VerboseFn verbose)
{
	textArSetErrorFile(NULL);

	struct UserData2 userData = { data, -1, NULL, 0, NULL };

	bool success = textArExtractArchive(stdImpOpenEntry, stdImpAppendEntry,
	                                    stdImpCloseEntry, stdImpFFMReadArchiveLine,
	                                    options, verbose, &userData);
	int funcErrno = errno;

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

bool stdImpAppendArchive(const char* data, void* userPtr)
{
	struct UserData* userData = (struct UserData*)userPtr;
	FILE* outputFile = userData->outputFile;
	return fwrite(data, strlen(data), 1, outputFile);
}

const TextArEntry* stdImpEntryIterator(const TextArEntry* entry, void* userPtr)
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

	textArSetErrorFile(entryDesc.path);
	
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
			textArSetError("refused to archive binary file");
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
		textArSetError("file type not supported");
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

static bool chownEntry(TextArEntry* entry, const char* entryName)
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

bool stdImpOpenEntry(TextArEntry* entry, void* userPtr)
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

bool stdImpAppendEntry(TextArEntry* entry, void* userPtr)
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

bool stdImpCloseEntry(TextArEntry* entry, void* userPtr)
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

char* stdImpReadArchiveLine(void* userPtr)
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

char* stdImpFFMReadArchiveLine(void* userPtr)
{
	struct UserData2* userData = (struct UserData2*)userPtr;
	errno = 0;
	
	char* line = (char*)userData->inputFile;

	if (line)
	{
		char* end = findEnd(line);
		if (*end) // is LF
		{
			userData->inputFile = end + 1;
		}
		else // is NULL
		{
			userData->inputFile = NULL;
		}
		
		return line;
	}
	else
	{
		return NULL;
	}
}
