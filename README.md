Note: this repository has been marked read-only. Similar functionality can be achieved with `diff` and `patch`.

# textar

*textar* is a tar-like program for creating plain text archives from plain text files. It is written in plain C and can be used as a library. 

It can be useful in several ways, such as:
- Loading multiple text files into RAM for fast access;
- Embedding multiple text files into a program (using the `.incbin` directive);
- Transferring or storing multiple text files into a media or application which only allows text files;
- **Easily editing the archive's contents without extracting.**

While most of the above can also be done with *tar* in some way, the last one only applies to *textar* and is its greatest advantage.

## Archive syntax

The entire file below is a valid *textar* archive, including the header text.

	All text before the first entry is ignored. This way, you can include a 
	description or any other desired text.

	Entries (and the end of the previous one) are marked as '>>>' at the end of the 
	line. Whitespace is not required before the file name.

	>>> file1

	These are the contents of file1. Note the empty lines before and after it. Both 
	the first and the last lines are ignored, if present. As such, entries ending 
	with none or a single empty line won't contain a trailing line break.


	>>> file2 mode=777
	These are the contents of file2. Note that there is only one empty line. Also 
	note a mode has been specified. Such mode will be used at file creation if the 
	respective option is enabled and must contain either a three or four digit 
	octal number.

	>>> file3 owner=root:root ownerid=0:0

	Note both an owner and ownerid have been specified. An entry is not required to 
	have both: if just one is present, it will be used, provided the respective 
	option is enabled; if both are present and both options are enabled, the 
	implementation is free to choose between them (for the default implementation 
	ownerid will be used).

	It is also important to note that, if using the default implementation, these 
	may cause the extraction process to fail, since only the root user is allowed 
	to perform an effective chown syscall. To avoid that, either extract as root, 
	disable the owner preservation flag or modify the owner and ownerid to match 
	the current user.


	>>> dir/ mode=0770

	This is a directory. Note the trailing slash. Since directories don't have 
	content (other than files), this can be used for other purposes, such as 
	comments. Implementations are free to use this data as desired, however the 
	default implementation simply ignores it.


	>>> "dir/this is a file"

	Files contained in directories must have their complete path specified. Entries 
	cannot be declared inside directories which haven't been declared yet, although 
	the default implementation will ignore such error if the missing directory 
	exists in the output directory. Note that, since the file name contains spaces, 
	it must be enclosed in quotation marks.


	>>> dir/file1 =>

	../file1
	
	This is a symbolic link, which is marked by '=>'. Such mark can appear anywhere 
	on the entry (by itself), as long as it's after the name. The location it 
	points to is given as the first line of its content. Further lines can be used 
	for other purposes, however the default implementation simply ignores it.


	>>> dir/subdir/

	Subdirectories are specified the same as directories...


	>>> dir/subdir/file

	...and files contained in them are still the same too.


	>>> "File names can contain most characters, such as ", ü and 💩, as long as the file name is between quotes"

	All characters are allowed in file names except for the null terminator, the 
	line break and the slash. The contents between the first and the last quotes 
	are passed verbatim to the implementation-defined handler. The implementation 
	may choose (or be forced to, in case of an operating system limitation) to also 
	forbid other characters.

	>>>> Finally, this is not an entry. More than three '>' characters are treated 
	as normal text, except for one is removed. This allows including an archive 
	into an archive (not yet tested as of this commit).


## Compression

Compression is not supported since it would defeat *textar*'s greatest advantage, which is the ability to easily edit an archive without extracting. Thus, no compression support will be added.

## Custom functionality

If custom functionality is needed, such as extracting to memory, a custom implementation can be made by supplying functions for reading, writing and other functionality. Reference and instructions can be found in the Doxygen documentation.

## Bugs

Although *textar* does have automatic tests, such tests are currently somewhat limited, which combined with the age of the project, mean there are likely many bugs present. If you happen to find any of them, please open an issue.
