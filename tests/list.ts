
#include "list.h"

#include <check.h>
#include <stddef.h>

#test empty
	List* list = listCreate();
	ck_assert_ptr_null(listIterate(list, NULL));
	listFree(&list);
	ck_assert_ptr_null(list);

#test append
	List* list = listCreate();

	int* item;
	item = listAppend(list, sizeof(int));
	*item = 123;
	item = listAppend(list, sizeof(int));
	*item = 456;
	item = listAppend(list, sizeof(int));
	*item = 789;

	item = listIterate(list, NULL);
	ck_assert_int_eq(*item, 123);
	item = listIterate(list, item);
	ck_assert_int_eq(*item, 456);
	item = listIterate(list, item);
	ck_assert_int_eq(*item, 789);

	listFree(&list);
	ck_assert_ptr_null(list);

#test insert
	List* list = listCreate();

	int *item, *item2;
	item = listAppend(list, sizeof(int));
	*item = 123;
	item2 = listAppend(list, sizeof(int));
	*item2 = 789;
	item = listInsert(list, item, sizeof(int));
	*item = 456;

	item = listIterate(list, NULL);
	ck_assert_int_eq(*item, 123);
	item = listIterate(list, item);
	ck_assert_int_eq(*item, 456);
	item = listIterate(list, item);
	ck_assert_int_eq(*item, 789);

	listFree(&list);
	ck_assert_ptr_null(list);
