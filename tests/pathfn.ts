
#include "path.h"

#include <check.h>
#include <stdlib.h>

#test remove_root
	ck_assert_str_eq(removeRoot(""), "");
	ck_assert_str_eq(removeRoot("/"), "");
	ck_assert_str_eq(removeRoot("/a"), "a");
	ck_assert_str_eq(removeRoot("/dir"), "dir");
	ck_assert_str_eq(removeRoot("a"), "a");
	ck_assert_str_eq(removeRoot("dir"), "dir");
	ck_assert_str_eq(removeRoot("//"), "");
	ck_assert_str_eq(removeRoot("//a"), "a");
	ck_assert_str_eq(removeRoot("//dir"), "dir");
	ck_assert_str_eq(removeRoot("//////////////////////"), "");
	ck_assert_str_eq(removeRoot("//////////////////////a"), "a");
	ck_assert_str_eq(removeRoot("//////////////////////dir"), "dir");
	ck_assert_str_eq(removeRoot("/dir1/dir2"), "dir1/dir2");
	ck_assert_str_eq(removeRoot("/dir1/dir2/"), "dir1/dir2/");
	ck_assert_str_eq(removeRoot("//////////dir1///////////dir2"), "dir1///////////dir2");
	ck_assert_str_eq(removeRoot("//////////dir1////////dir2/////////////////////"), "dir1////////dir2/////////////////////");

#test find_component_end
	ck_assert_str_eq(findComponentEnd(""), "");
	ck_assert_str_eq(findComponentEnd("/"), "/");
	ck_assert_str_eq(findComponentEnd("/a"), "/a");
	ck_assert_str_eq(findComponentEnd("/dir"), "/dir");
	ck_assert_str_eq(findComponentEnd("////dir"), "////dir");
	ck_assert_str_eq(findComponentEnd("a"), "");
	ck_assert_str_eq(findComponentEnd("dir"), "");
	ck_assert_str_eq(findComponentEnd("a/"), "/");
	ck_assert_str_eq(findComponentEnd("dir/"), "/");
	ck_assert_str_eq(findComponentEnd("dir///"), "///");

#test find_next_component
	ck_assert_str_eq(findNextComponent(""), "");
	ck_assert_str_eq(findNextComponent("/"), "");
	ck_assert_str_eq(findNextComponent("/a"), "a");
	ck_assert_str_eq(findNextComponent("/dir"), "dir");
	ck_assert_str_eq(findNextComponent("a"), "");
	ck_assert_str_eq(findNextComponent("dir"), "");
	ck_assert_str_eq(findNextComponent("a/"), "");
	ck_assert_str_eq(findNextComponent("dir/"), "");
	ck_assert_str_eq(findNextComponent("dir///"), "");
	ck_assert_str_eq(findNextComponent("//"), "");
	ck_assert_str_eq(findNextComponent("//a"), "a");
	ck_assert_str_eq(findNextComponent("//dir"), "dir");
	ck_assert_str_eq(findNextComponent("////dir"), "dir");
	ck_assert_str_eq(findNextComponent("//////////////////////"), "");
	ck_assert_str_eq(findNextComponent("//////////////////////a"), "a");
	ck_assert_str_eq(findNextComponent("//////////////////////dir"), "dir");
	ck_assert_str_eq(findNextComponent("/dir1/dir2"), "dir1/dir2");
	ck_assert_str_eq(findNextComponent("/dir1/dir2/"), "dir1/dir2/");
	ck_assert_str_eq(findNextComponent("//////////dir1///////////dir2"), "dir1///////////dir2");
	ck_assert_str_eq(findNextComponent("//////////dir1////////dir2/////////////////////"), "dir1////////dir2/////////////////////");
	ck_assert_str_eq(findNextComponent("dir1/dir2"), "dir2");
	ck_assert_str_eq(findNextComponent("dir1/dir2/"), "dir2/");
	ck_assert_str_eq(findNextComponent("dir1///////////dir2"), "dir2");
	ck_assert_str_eq(findNextComponent("dir1////////dir2/////////////////////"), "dir2/////////////////////");

#test fix_path
	ck_assert_str_eq(fixPath(""), "");
	ck_assert_str_eq(fixPath("/"), "");
	ck_assert_str_eq(fixPath("/a"), "a");
	ck_assert_str_eq(fixPath("/dir"), "dir");
	ck_assert_str_eq(fixPath("."), ".");
	ck_assert_str_eq(fixPath(".."), "");
	ck_assert_str_eq(fixPath("/../"), "");
	ck_assert_str_eq(fixPath("/."), ".");
	ck_assert_str_eq(fixPath("/home/user"), "home/user");
	ck_assert_str_eq(fixPath("home/user/../user2/."), "user2/.");
	ck_assert_str_eq(fixPath("home/user/../user2/./"), "user2/./");
	ck_assert_str_eq(fixPath("home/user/../user2/..."), "user2/...");
	ck_assert_str_eq(fixPath("home/user/../user2/.../"), "user2/.../");
	ck_assert_str_eq(fixPath("/////home///////user//////"), "home///////user//////");
	ck_assert_str_eq(fixPath("home///////user//////../////user2///."), "user2///.");
	ck_assert_str_eq(fixPath("home///////user//////../////user2///.////"), "user2///.////");
