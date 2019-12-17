#include "entry.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t findLength(const char* line)
{
	const char* orig = line;
	for (; *line && *line != '\n'; line++);
	return line - orig;
}

char* findEnd(char* line)
{
	for (; *line && *line != '\n'; line++);
	return line;
}

char* findFirstOf(char* line, char c)
{
	for (; *line && *line != '\n'; line++)
	{
		if (*line == c)
		{
			return line;
		}
	}
	return NULL;
}

char* findLastOf(char* line, char c)
{
	char* last = NULL;
	for (; *line && *line != '\n'; line++)
	{
		if (*line == c)
		{
			last = line;
		}
	}
	return last;
}

char* findWhitespace(char* line)
{
	for (; *line && *line != '\n'; line++)
	{
		if (isspace(*line))
		{
			return line;
		}
	}
	return NULL;
}

char* skipWhitespace(char* line)
{
	for (; *line && *line != '\n' && isspace(*line); line++);
	return line;
}

const char* modeToString(mode_t mode)
{
	static char string[5] = "xxxx";

	/*for (int i = 3; i >= 0; i--)
	{
		string[i] = (mode & 7) + '0';
		mode >>= 3;
	}

	return string;*/

	snprintf(string, sizeof string, "%04o", mode);
	return string;
}

const char* uint32ToString(uint32_t i)
{
	static char buffer[11] = "xxxxxxxxxx";
	/*char* cur = buffer;
	unsigned int d = 1000000000;
	
	while (d > 0)
	{
		*(cur++) = '0' + i / d;
		i %= d;
		d /= 10;
	}
	
	for (cur = buffer; cur < &buffer[9]; cur++)
	{
		if (*cur != '0')
		{
			return cur;
		}
	}
	
	return &buffer[9];*/

	snprintf(buffer, sizeof buffer, "%u", i);
	return buffer;
}

const char* ownerIdToString(uid_t uid, gid_t gid)
{
	static char string[22];

	{
		const char* str = uint32ToString(uid & 0xFFFFFFFF);
		strcpy(&string[0], &str[0]);
	}

	size_t commaPos = strlen(string);
	string[commaPos] = ':';

	{
		const char* str = uint32ToString(gid & 0xFFFFFFFF);
		strcpy(&string[commaPos + 1], &str[0]);
	}

	return string;
}

bool stringToMode(const char* string, size_t stringLen, mode_t* modeOut)
{
	if (stringLen < 3 || stringLen > 4 || !isdigit(string[0]))
	{
		return false;
	}

	char num[5];
	memcpy(num, string, stringLen);
	num[stringLen] = 0;

	errno = 0;

	char* end;
	mode_t mode = strtoul(num, &end, 8);
	if (end != num + stringLen || errno == ERANGE)
	{
		return false;
	}

	*modeOut = mode;

	return true;
}

bool stringToOwnerId(const char* string, size_t stringLen, gid_t* groupOut, uid_t* userOut)
{
	char* colon = memchr(string, ':', stringLen);
	size_t size1 = colon - string;
	size_t size2 = string + stringLen - colon - 1;

	if (size1 > 10 || size2 > 10 || !isdigit(string[0]) || !isdigit(colon[1]))	// safe to assume at most 32-bit
	{
		errno = ERANGE;
		return false;
	}
	
	char number[11];
	memcpy(number, string, size1);
	number[size1] = 0;

	errno = 0;

	char* end;
	gid_t gid = strtoul(number, &end, 10);
	if (end != &number[size1] || errno == ERANGE)
	{
		return false;
	}

	memcpy(number, colon + 1, size2);
	number[size2] = 0;

	errno = 0;

	uid_t uid = strtoul(number, &end, 10);
	if (end != &number[size2] || errno == ERANGE)
	{
		errno = EINVAL;
		return false;
	}

	*groupOut = gid;
	*userOut = uid;

	return true;
}

bool stringToOwner(const char* string, size_t stringLen, const char** userOut)
{
	char* colon = memchr(string, ':', stringLen);
	size_t size1 = colon - string;
	size_t size2 = string + stringLen - colon - 1;

	if (size1 > 32 || size2 > 32)
	{
		return false;
	}

	*userOut = colon + 1;

	return true;
}
