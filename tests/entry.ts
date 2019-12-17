
#include "entry.h"

#include <check.h>

#test find_length
	ck_assert_uint_eq(findLength(""), 0);
	ck_assert_uint_eq(findLength("\n"), 0);
	ck_assert_uint_eq(findLength("a"), 1);
	ck_assert_uint_eq(findLength("this is a sentence"), 18);
	ck_assert_uint_eq(findLength("this is a sentence\n"), 18);
	ck_assert_uint_eq(findLength("this is a sentence\nthis is a sentence"), 18);

#test find_end
	ck_assert_str_eq(findEnd(""), "");
	ck_assert_str_eq(findEnd("\n"), "\n");
	ck_assert_str_eq(findEnd("a"), "");
	ck_assert_str_eq(findEnd("this is a sentence"), "");
	ck_assert_str_eq(findEnd("this is a sentence\n"), "\n");
	ck_assert_str_eq(findEnd("this is a sentence\nthis is a sentence"), "\nthis is a sentence");

#test find_first_of
	ck_assert_str_eq(findFirstOf("a", 'a'), "a");
	ck_assert_str_eq(findFirstOf("a\na", 'a'), "a\na");
	ck_assert_str_eq(findFirstOf("a word", 'a'), "a word");
	ck_assert_str_eq(findFirstOf("a word", 'w'), "word");
	ck_assert_str_eq(findFirstOf("this is a sentence", 'e'), "entence");
	ck_assert_str_eq(findFirstOf("this is a sentence", 't'), "this is a sentence");
	ck_assert_str_eq(findFirstOf("this is a sentence", 'i'), "is is a sentence");
	ck_assert_str_eq(findFirstOf("this is a sentence", ' '), " is a sentence");

	ck_assert_pstr_eq(findFirstOf("a", 'j'), NULL);
	ck_assert_pstr_eq(findFirstOf("a\nj", 'j'), NULL);
	ck_assert_pstr_eq(findFirstOf("a", '\0'), NULL);
	ck_assert_pstr_eq(findFirstOf("a\n", '\n'), NULL);
	ck_assert_pstr_eq(findFirstOf("a word", 'k'), NULL);
	ck_assert_pstr_eq(findFirstOf("a word", 'e'), NULL);
	ck_assert_pstr_eq(findFirstOf("a word", '\0'), NULL);
	ck_assert_pstr_eq(findFirstOf("a word", '\n'), NULL);
	ck_assert_pstr_eq(findFirstOf("this is a sentence", '1'), NULL);
	ck_assert_pstr_eq(findFirstOf("this is a sentence", 'k'), NULL);
	ck_assert_pstr_eq(findFirstOf("this is a sentence", 'u'), NULL);
	ck_assert_pstr_eq(findFirstOf("this is a sentence", '\0'), NULL);
	ck_assert_pstr_eq(findFirstOf("this is a sentence", '\n'), NULL);

#test find_last_of
	ck_assert_str_eq(findLastOf("a", 'a'), "a");
	ck_assert_str_eq(findLastOf("a\na", 'a'), "a\na");
	ck_assert_str_eq(findLastOf("a word", 'a'), "a word");
	ck_assert_str_eq(findLastOf("a word", 'w'), "word");
	ck_assert_str_eq(findLastOf("this is a sentence", 'e'), "e");
	ck_assert_str_eq(findLastOf("this is a sentence", 't'), "tence");
	ck_assert_str_eq(findLastOf("this is a sentence", 'i'), "is a sentence");
	ck_assert_str_eq(findLastOf("this is a sentence", ' '), " sentence");

	ck_assert_pstr_eq(findLastOf("a", 'j'), NULL);
	ck_assert_pstr_eq(findLastOf("a\nj", 'j'), NULL);
	ck_assert_pstr_eq(findLastOf("a", '\0'), NULL);
	ck_assert_pstr_eq(findLastOf("a\n", '\n'), NULL);
	ck_assert_pstr_eq(findLastOf("a word", 'k'), NULL);
	ck_assert_pstr_eq(findLastOf("a word", 'e'), NULL);
	ck_assert_pstr_eq(findLastOf("a word", '\0'), NULL);
	ck_assert_pstr_eq(findLastOf("a word", '\n'), NULL);
	ck_assert_pstr_eq(findLastOf("this is a sentence", '1'), NULL);
	ck_assert_pstr_eq(findLastOf("this is a sentence", 'k'), NULL);
	ck_assert_pstr_eq(findLastOf("this is a sentence", 'u'), NULL);
	ck_assert_pstr_eq(findLastOf("this is a sentence", '\0'), NULL);
	ck_assert_pstr_eq(findLastOf("this is a sentence", '\n'), NULL);

#test find_whitespace
	ck_assert_str_eq(findWhitespace(" "), " ");
	ck_assert_str_eq(findWhitespace("  "), "  ");
	ck_assert_str_eq(findWhitespace("\t"), "\t");
	ck_assert_str_eq(findWhitespace("\t\t"), "\t\t");
	ck_assert_str_eq(findWhitespace(" \n "), " \n ");
	ck_assert_str_eq(findWhitespace("this is a sentence"), " is a sentence");
	ck_assert_str_eq(findWhitespace("this  is a sentence"), "  is a sentence");
	ck_assert_str_eq(findWhitespace("\t this  is a sentence"), "\t this  is a sentence");
	ck_assert_str_eq(findWhitespace("  this  is a sentence"), "  this  is a sentence");

	ck_assert_pstr_eq(findWhitespace(""), NULL);
	ck_assert_pstr_eq(findWhitespace("\n "), NULL);
	ck_assert_pstr_eq(findWhitespace("\n  "), NULL);
	ck_assert_pstr_eq(findWhitespace("\n\t"), NULL);
	ck_assert_pstr_eq(findWhitespace("\n\t\t"), NULL);
	ck_assert_pstr_eq(findWhitespace("\n \n "), NULL);
	ck_assert_pstr_eq(findWhitespace("thisisasentence"), NULL);
	ck_assert_pstr_eq(findWhitespace("thisisasentence\n "), NULL);

#test skip_whitespace
	ck_assert_str_eq(skipWhitespace(""), "");
	ck_assert_str_eq(skipWhitespace(" "), "");
	ck_assert_str_eq(skipWhitespace("   "), "");
	ck_assert_str_eq(skipWhitespace("\t"), "");
	ck_assert_str_eq(skipWhitespace("\t \t"), "");
	ck_assert_str_eq(skipWhitespace("test"), "test");
	ck_assert_str_eq(skipWhitespace("word1 word2"), "word1 word2");
	ck_assert_str_eq(skipWhitespace(" word1 word2"), "word1 word2");
	ck_assert_str_eq(skipWhitespace("\tword1 word2"), "word1 word2");
	ck_assert_str_eq(skipWhitespace(" word1\tword2"), "word1\tword2");
	ck_assert_str_eq(skipWhitespace("\nword1"), "\nword1");
	ck_assert_str_eq(skipWhitespace("   \nword1"), "\nword1");
	ck_assert_str_eq(skipWhitespace("\t\nword1"), "\nword1");

	ck_assert_str_eq(skipWhitespace("\n"), "\n");
	ck_assert_str_eq(skipWhitespace(" \n"), "\n");
	ck_assert_str_eq(skipWhitespace("   \n"), "\n");
	ck_assert_str_eq(skipWhitespace("\t\n"), "\n");
	ck_assert_str_eq(skipWhitespace("\t \t\n"), "\n");
	ck_assert_str_eq(skipWhitespace("test\n"), "test\n");
	ck_assert_str_eq(skipWhitespace("word1 word2\n"), "word1 word2\n");
	ck_assert_str_eq(skipWhitespace(" word1 word2\n"), "word1 word2\n");
	ck_assert_str_eq(skipWhitespace("\tword1 word2\n"), "word1 word2\n");
	ck_assert_str_eq(skipWhitespace(" word1\tword2\n"), "word1\tword2\n");
	ck_assert_str_eq(skipWhitespace("\nword1\n"), "\nword1\n");
	ck_assert_str_eq(skipWhitespace("   \nword1\n"), "\nword1\n");
	ck_assert_str_eq(skipWhitespace("\t\nword1\n"), "\nword1\n");

#test mode_to_string
	ck_assert_str_eq(modeToString(00000), "0000");
	ck_assert_str_eq(modeToString(07777), "7777");
	ck_assert_str_eq(modeToString(01234), "1234");
	ck_assert_str_eq(modeToString(04321), "4321");
	ck_assert_str_eq(modeToString(01000), "1000");
	ck_assert_str_eq(modeToString(00100), "0100");
	ck_assert_str_eq(modeToString(00010), "0010");
	ck_assert_str_eq(modeToString(00001), "0001");

#test uint32_to_string
	ck_assert_str_eq(uint32ToString(0), "0");
	ck_assert_str_eq(uint32ToString(5), "5");
	ck_assert_str_eq(uint32ToString(10), "10");
	ck_assert_str_eq(uint32ToString(50), "50");
	ck_assert_str_eq(uint32ToString(100), "100");
	ck_assert_str_eq(uint32ToString(500), "500");
	ck_assert_str_eq(uint32ToString(4294967295), "4294967295");
	ck_assert_str_eq(uint32ToString(1234567890), "1234567890");
	ck_assert_str_eq(uint32ToString(3876549210), "3876549210");
	ck_assert_str_eq(uint32ToString(1000000000), "1000000000");
	ck_assert_str_eq(uint32ToString(100000000), "100000000");
	ck_assert_str_eq(uint32ToString(10000000), "10000000");
	ck_assert_str_eq(uint32ToString(1000000), "1000000");
	ck_assert_str_eq(uint32ToString(100000), "100000");
	ck_assert_str_eq(uint32ToString(10000), "10000");
	ck_assert_str_eq(uint32ToString(1000), "1000");
	ck_assert_str_eq(uint32ToString(100), "100");
	ck_assert_str_eq(uint32ToString(10), "10");
	ck_assert_str_eq(uint32ToString(1), "1");

#test owner_id_to_string
	ck_assert_str_eq(ownerIdToString(0, 0), "0:0");
	ck_assert_str_eq(ownerIdToString(0, 4294967295), "0:4294967295");
	ck_assert_str_eq(ownerIdToString(4294967295, 0), "4294967295:0");
	ck_assert_str_eq(ownerIdToString(4294967295, 4294967295), "4294967295:4294967295");
	ck_assert_str_eq(ownerIdToString(1, 1000000000), "1:1000000000");
	ck_assert_str_eq(ownerIdToString(10, 100000000), "10:100000000");
	ck_assert_str_eq(ownerIdToString(100, 10000000), "100:10000000");
	ck_assert_str_eq(ownerIdToString(1000, 1000000), "1000:1000000");
	ck_assert_str_eq(ownerIdToString(10000, 100000), "10000:100000");
	ck_assert_str_eq(ownerIdToString(100000, 10000), "100000:10000");
	ck_assert_str_eq(ownerIdToString(1000000, 1000), "1000000:1000");
	ck_assert_str_eq(ownerIdToString(10000000, 100), "10000000:100");
	ck_assert_str_eq(ownerIdToString(100000000, 10), "100000000:10");
	ck_assert_str_eq(ownerIdToString(1000000000, 1), "1000000000:1");
	ck_assert_str_eq(ownerIdToString(0, 1000000000), "0:1000000000");
	ck_assert_str_eq(ownerIdToString(0, 100000000), "0:100000000");
	ck_assert_str_eq(ownerIdToString(0, 10000000), "0:10000000");
	ck_assert_str_eq(ownerIdToString(0, 1000000), "0:1000000");
	ck_assert_str_eq(ownerIdToString(0, 100000), "0:100000");
	ck_assert_str_eq(ownerIdToString(0, 10000), "0:10000");
	ck_assert_str_eq(ownerIdToString(0, 1000), "0:1000");
	ck_assert_str_eq(ownerIdToString(0, 100), "0:100");
	ck_assert_str_eq(ownerIdToString(0, 10), "0:10");
	ck_assert_str_eq(ownerIdToString(0, 1), "0:1");
	ck_assert_str_eq(ownerIdToString(1, 0), "1:0");
	ck_assert_str_eq(ownerIdToString(10, 0), "10:0");
	ck_assert_str_eq(ownerIdToString(100, 0), "100:0");
	ck_assert_str_eq(ownerIdToString(1000, 0), "1000:0");
	ck_assert_str_eq(ownerIdToString(10000, 0), "10000:0");
	ck_assert_str_eq(ownerIdToString(100000, 0), "100000:0");
	ck_assert_str_eq(ownerIdToString(1000000, 0), "1000000:0");
	ck_assert_str_eq(ownerIdToString(10000000, 0), "10000000:0");
	ck_assert_str_eq(ownerIdToString(100000000, 0), "100000000:0");
	ck_assert_str_eq(ownerIdToString(1000000000, 0), "1000000000:0");

#test string_to_mode
	mode_t mode = 01234;

	ck_assert(!stringToMode("xxxxxxxxxxxxxxxxxx", 0, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("0xxxxxxxxxxxxxxxxx", 1, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("00xxxxxxxxxxxxxxxx", 2, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(stringToMode("000xxxxxxxxxxxxxxx", 3, &mode));
	ck_assert_int_eq(mode, 00000);
	ck_assert(stringToMode("0000xxxxxxxxxxxxxx", 4, &mode));
	ck_assert_int_eq(mode, 00000);
	ck_assert(stringToMode("123xxxxxxxxxxxxxxx", 3, &mode));
	ck_assert_int_eq(mode, 0123);
	ck_assert(stringToMode("1230xxxxxxxxxxxxxx", 4, &mode));
	ck_assert_int_eq(mode, 01230);
	ck_assert(stringToMode("1234xxxxxxxxxxxxxx", 4, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("00000xxxxxxxxxxxxx", 5, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("helloworldxxxxxxxx", 10, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("4321axxxxxxxxxxxxx", 5, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode("4321 xxxxxxxxxxxxx", 5, &mode));
	ck_assert_int_eq(mode, 01234);
	ck_assert(!stringToMode(" 4321xxxxxxxxxxxxx", 5, &mode));
	ck_assert_int_eq(mode, 01234);

#test string_to_owner_id
	gid_t gid = 54321;
	uid_t uid = 54321;

	ck_assert(!stringToOwnerId("xxxxxxxxxxxxxxxxxx", 0, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(!stringToOwnerId(":xxxxxxxxxxxxxxxxx", 1, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(!stringToOwnerId("0:xxxxxxxxxxxxxxxx", 2, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(!stringToOwnerId(":0xxxxxxxxxxxxxxxx", 2, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(!stringToOwnerId("0xxxxxxxxxxxxxxxxx", 1, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(!stringToOwnerId("a:axxxxxxxxxxxxxxx", 3, &gid, &uid));
	ck_assert_int_eq(gid, 54321);
	ck_assert_int_eq(uid, 54321);
	ck_assert(stringToOwnerId("0:0xxxxxxxxxxxxxxx", 3, &gid, &uid));
	ck_assert_int_eq(gid, 0);
	ck_assert_int_eq(uid, 0);
	ck_assert(stringToOwnerId("1234:5678xxxxxxxxx", 9, &gid, &uid));
	ck_assert_int_eq(gid, 1234);
	ck_assert_int_eq(uid, 5678);
	ck_assert(!stringToOwnerId("-8765:4321xxxxxxxx", 10, &gid, &uid));
	ck_assert_int_eq(gid, 1234);
	ck_assert_int_eq(uid, 5678);
	ck_assert(!stringToOwnerId("8765:4321axxxxxxxx", 10, &gid, &uid));
	ck_assert_int_eq(gid, 1234);
	ck_assert_int_eq(uid, 5678);
	ck_assert(!stringToOwnerId(" 8765:4321xxxxxxxx", 10, &gid, &uid));
	ck_assert_int_eq(gid, 1234);
	ck_assert_int_eq(uid, 5678);
	ck_assert(!stringToOwnerId("8765 :4321xxxxxxxx", 10, &gid, &uid));
	ck_assert_int_eq(gid, 1234);
	ck_assert_int_eq(uid, 5678);

#test string_to_owner
	const char* initial = "string";
	const char* user = initial;

	ck_assert(!stringToOwner("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, &user));
	ck_assert_ptr_eq(user, initial);
	ck_assert(!stringToOwner("thisnameislongerthanthirtytwocharacters:userxxxxxxxxxxxxxx", 44, &user));
	ck_assert_ptr_eq(user, initial);
	ck_assert(!stringToOwner("group:thisnameislongerthanthirtytwocharactersxxxxxxxxxxxxx", 4, &user));
	ck_assert_ptr_eq(user, initial);
	ck_assert(stringToOwner("group:userxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 10, &user));
	ck_assert_str_eq(user - 6, "group:userxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	ck_assert_str_eq(user, "userxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
