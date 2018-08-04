#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CUnit/Basic.h>

#include "musiclist_interface.h"

static __attribute__((unused)) void music_info_test(void)
{
	struct music_info *info;

	info = mozart_musiclist_get_info(0, NULL, NULL, NULL, NULL, NULL, NULL);
	CU_ASSERT_PTR_NOT_NULL_FATAL(info);
	mozart_musiclist_free_music_info(info, NULL);
}

static __attribute__((unused)) void invalid_param_test(void)
{
	struct music_list *list = mozart_musiclist_create(NULL);

	CU_ASSERT_PTR_NOT_NULL_FATAL(list);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(NULL), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_set_play_mode(NULL, play_mode_single), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_play_mode(NULL), play_mode_order);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(NULL), -1);
	CU_ASSERT_PTR_EQUAL_FATAL(mozart_musiclist_get_current(NULL), NULL);
	CU_ASSERT_PTR_EQUAL_FATAL(mozart_musiclist_set_prev(NULL), NULL);
	CU_ASSERT_PTR_EQUAL_FATAL(mozart_musiclist_set_next(NULL, true), NULL);
	CU_ASSERT_PTR_EQUAL_FATAL(mozart_musiclist_set_index(NULL, 5), NULL);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(NULL, NULL), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, NULL), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_delete_index(NULL, 5), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_clean(NULL), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(NULL), -1);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_delete_index(list, 0), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) void order_mode_test(void)
{
	int i;
	char name[32];
	struct music_info *info;
	struct music_list *list = mozart_musiclist_create(NULL);

	CU_ASSERT_PTR_NOT_NULL_FATAL(list);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), -1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 0);

	for (i = 0; i < 5; i++) {
		snprintf(name, 32, "name-%d", i);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", NULL);
		CU_ASSERT_PTR_NOT_NULL_FATAL(info);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), i);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 0);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), i + 1);
	}

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, true));
	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, false));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 2);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-2");
	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_prev(list));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 1);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-1");

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_get_index(list, 4));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 4);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-4");
	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, true));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 0);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-0");
	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_prev(list));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 4);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-4");

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_get_index(list, 2));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 2);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-2");
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_delete_index(list, 3), 0);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 2);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-2");
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_delete_index(list, 1), 0);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 1);
	CU_ASSERT_STRING_EQUAL_FATAL(mozart_musiclist_get_current(list)->music_name, "name-2");
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 3);

	info = mozart_musiclist_get_info(0, "name", NULL, NULL, NULL, NULL, NULL);
	CU_ASSERT_PTR_NOT_NULL_FATAL(info);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), 3);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 1);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 4);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_clean(list), 0);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 0);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), -1);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) void single_mode_test(void)
{
	int i;
	char name[32];
	struct music_info *info;
	struct music_list *list = mozart_musiclist_create(NULL);

	CU_ASSERT_PTR_NOT_NULL_FATAL(list);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_set_play_mode(list, play_mode_single), 0);

	for (i = 0; i < 5; i++) {
		snprintf(name, 32, "name-%d", i);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", NULL);
		mozart_musiclist_insert(list, info);
	}

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 5);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 0);

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, false));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 0);

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, true));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 1);
	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, true));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 2);

	CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_prev(list));
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 1);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) void random_mode_test(void)
{
	int i, index;
	char name[32];
	struct music_info *info;
	struct music_list *list = mozart_musiclist_create(NULL);

	CU_ASSERT_PTR_NOT_NULL_FATAL(list);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_set_play_mode(list, play_mode_random), 0);

	for (i = 0; i < 35; i++) {
		snprintf(name, 32, "name-%d", i);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", NULL);
		mozart_musiclist_insert(list, info);
	}

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 35);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_current_index(list), 0);

	srandom(time(NULL));
	for (i = 0; i < 500; i++) {
		if (random() % 2) {
			CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_next(list, false));
			index = mozart_musiclist_get_current_index(list);
		} else {
			CU_ASSERT_PTR_NOT_NULL_FATAL(mozart_musiclist_set_prev(list));
			index = mozart_musiclist_get_current_index(list);
		}

		CU_ASSERT_TRUE_FATAL(index < 35);
		CU_ASSERT_TRUE_FATAL(index >= 0);
	}

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) void free_usr_data(void *data)
{
}

static __attribute__((unused)) void usr_data_test(void)
{
	int i;
	char name[32];
	struct music_info *info;
	struct music_list *list = mozart_musiclist_create(NULL);

	info = mozart_musiclist_get_info(0, "name", "url", "picture",
					 "albums", "artists", (void *)1);
	CU_ASSERT_PTR_NOT_NULL_FATAL(info);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), -1);
	mozart_musiclist_free_music_info(info, NULL);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);


	list = mozart_musiclist_create(free_usr_data);
	for (i = 0; i < 5; i++) {
		snprintf(name, 32, "name-%d", i);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", (void *)i);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), i);
	}

	info = mozart_musiclist_get_current(list);
	CU_ASSERT_EQUAL_FATAL(info->data, (void *)0);
	for (i = 1; i < 5; i++) {
		info = mozart_musiclist_set_next(list, false);
		CU_ASSERT_EQUAL_FATAL(info->data, (void *)i);
	}

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) void max_index_test(void)
{
	int i;
	char name[32];
	struct music_info *info;
	struct music_list *list = mozart_musiclist_create(NULL);

	list = mozart_musiclist_create(free_usr_data);
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_set_max_index(list, 5), 0);

	for (i = 0; i < 5; i++) {
		snprintf(name, 32, "name-%d", i);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", (void *)i);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), i);
	}
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 5);

	for (i = 0; i < 5; i++) {
		snprintf(name, 32, "name-%d", i + 5);
		info = mozart_musiclist_get_info(0, name, "url", "picture",
						 "albums", "artists", (void *)i);
		CU_ASSERT_EQUAL_FATAL(mozart_musiclist_insert(list, info), i);
	}
	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_get_length(list), 5);

	CU_ASSERT_EQUAL_FATAL(mozart_musiclist_destory(list), 0);
}

static __attribute__((unused)) CU_TestInfo music_list_cases[] = {
	{"music info test", music_info_test},
	{"invalid param test", invalid_param_test},
	{"order mode test", order_mode_test},
	{"single mode test", single_mode_test},
	{"random mode test", random_mode_test},
	{"usr data test", usr_data_test},
	{"max index test", usr_data_test},
	CU_TEST_INFO_NULL
};

CU_SuiteInfo music_list_suites[] = {
	{"music list case:", NULL, NULL, NULL, NULL, music_list_cases},
	CU_SUITE_INFO_NULL
};

int main(void)
{
	if (CU_initialize_registry() != CUE_SUCCESS) {
		fprintf(stderr, "Initialization of Test Registry failed.\n");
		return -1;
	}
	if (CU_register_suites(music_list_suites) != CUE_SUCCESS) {
		fprintf(stderr, "Register cli suites failed - %s ", CU_get_error_msg());
		return -1;
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();

	return 0;
}
