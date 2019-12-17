
#include "textar.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Args
{
	char operation;
	bool help;
	char* directory;
	char* file;
	int mode;
	int ownid;
	int ownname;
};

struct Args handleArgs(int argc, char** argv)
{
	int debug = 0;

	struct Args args;
	args.operation = 0;
	args.help = false;
	args.directory = "";
	args.file = "";
	args.mode = TEXTAROPTIONS_MODE;
	args.ownid = TEXTAROPTIONS_OWNID;
	args.ownname = TEXTAROPTIONS_OWNNAME;

	const struct option options[] =
	{
		{ "create", no_argument, NULL, 'c' },
		{ "debug", no_argument, &debug, 'd' },
		{ "directory", required_argument, NULL, 'C' },
		{ "extract", no_argument, NULL, 'x' },
		{ "file", required_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "mode", no_argument, &args.mode, TEXTAROPTIONS_MODE },
		{ "uid", no_argument, &args.ownid, TEXTAROPTIONS_OWNID },
		{ "owner", no_argument, &args.ownname, TEXTAROPTIONS_OWNNAME },
		{ "nomode", no_argument, &args.mode, 0 },
		{ "nouid", no_argument, &args.ownid, 0 },
		{ "noowner", no_argument, &args.ownname, 0 },
		{ NULL, 0, NULL, 0 }
	};

	for (;;)
	{
		int option = getopt_long(argc, argv, "C:cf:hx", options, NULL);

		switch (option)
		{
		case -1:
			goto for_break;

		case 0:
			break;

		case 'c':
		case 'x':
			if (args.operation == 0)
			{
				args.operation = option;
			}
			else if (args.operation != option)
			{
				fprintf(stderr, "%s: can't specify both -%c and -%c\n",
				        argv[0], (char)option, args.operation);
				exit(EXIT_FAILURE);
			}
			break;

		case 'C':
			args.directory = optarg;
			break;

		case 'f':
			args.file = optarg;
			break;

		case 'h':
			args.help = true;
			break;
		
		default:
			exit(EXIT_FAILURE);
		}
	}
for_break:

	if (debug)
	{
		printf("operation=%c\nhelp=%i\ndirectory=%s\nfile=%s\n",
		       args.operation, args.help ? 1 : 0, args.directory, args.file);
	}

	return args;
}

void showHelp(const char* name)
{
	printf(
		"Usage: %s [OPTION...] [FILE...]\n"
		"\n"
		"  -c, --create         create a new textar archive\n"
		"  -C, --directory=DIR  change to DIR before performing operation\n"
		"  -f, --file=FILE      use FILE as the textar archive\n"
		"  -h, --help           show this message\n"
		"  -x, --extract        extract files from a textar archive\n"
		"      --uid            store user ID and group ID (default)\n"
		"      --owner          store user name and group name (default)\n"
		"      --mode           store file mode (default)\n"
		"      --no[xxx]        omit specific data\n"
		"\n", name
	);
}

void _puts(const char* x)
{
	puts(x);
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	struct Args args = handleArgs(argc, argv);

	if (args.help || args.operation == 0)
	{
		showHelp(argv[0]);
		return EXIT_SUCCESS;
	}

	TextArOptions options = args.mode | args.ownid | args.ownname;

	char filePath[PATH_MAX];
	realpath(args.file, filePath);

	if (*args.directory)
	{
		if (chdir(args.directory))
		{
			fprintf(stderr, "%s: failed to change dir: %s\n",  argv[0], strerror(errno));
			return 1;
		}
	}

	bool success;

	switch (args.operation)
	{
	case 'c':
		success = textArCreateArchiveFile(filePath, (const char**)argv + optind, options, _puts);
		break;

	case 'x':
		success = textArExtractArchiveFile(filePath, options, _puts);
		break;
	}

	if (!success)
	{
		const char* errDesc = textArErrorDesc();

		fprintf(stderr, "%s: ", argv[0]);

		const char* errFile = textArErrorFile();
		if (errFile)
		{
			fprintf(stderr, "%s: ", errFile);
		}

		fprintf(stderr, "%s", errDesc);

		if (errno)
		{
			fprintf(stderr, ": %s\n", strerror(errno));
		}
		else
		{
			fputc('\n', stderr);
		}
		
		
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
