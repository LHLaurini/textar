#include "path.h"

const char* removeRoot(const char* path)
{
	while (*(path++) == '/')
	{
	}
	return path - 1;
}

const char* findComponentEnd(const char* path)
{
	for (; *path != '/' && *path != 0; path++)
	{
	}
	return path;
}

const char* findNextComponent(const char* path)
{
	return removeRoot(findComponentEnd(path));
}

const char* fixPath(const char* path)
{
	path = removeRoot(path);

	const char* newPath = path;

	while (*path)
	{
		const char* next = findNextComponent(path);
		const char* cend = findComponentEnd(path);
		if (cend - path == 2 && path[0] == '.' && path[1] == '.')
		{
			newPath = next;
		}
		path = next;
	}
	
	return newPath;
}
