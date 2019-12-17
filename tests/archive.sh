#!/bin/sh
set -e
TESTS_DIR="$(dirname "$0")"

if [ ! -e "$TESTS_DIR/testdir" ]; then
	echo "testdir not found. Extracting..."
	tar xf "$TESTS_DIR/testdir.tar.xz" -C "$TESTS_DIR"
fi

../textar -cf archivetest.ttar -C "$TESTS_DIR" testdir
../textar -xf archivetest.ttar

err=0

# Check attributes

# i: itemize
# n: dry run
# r: recursive
# l: preserve symlinks
# p: preserve permissions
# t: preserve times (disabled for now)
# g: preserve group
# o: preserve owner
# D: preserve device and special files

rsync_result="$(rsync -inrlpgoDc --delete "$TESTS_DIR/testdir" .)"

echo "$rsync_result"

if [ ! "$rsync_result" ]; then
	echo "Attributes match."
else
	echo "Attributes don't match."
	err=1
fi

# Check data
if diff -q "$TESTS_DIR/testdir" testdir; then
	echo "Contents match."
else
	echo "Contents don't match."
	err=1
fi

exit $err
