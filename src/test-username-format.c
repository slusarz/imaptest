/* Copyright (c) ImapTest authors, see the included COPYING file */

#include "lib.h"
#include "test-common.h"
#include "username-format.h"
#include <string.h>

static void test_username_format_valid(void)
{
	const char *error;

	test_begin("username_format_is_valid() - valid");

	/* 0 parameters */
	test_assert(username_format_is_valid("user", &error));
	/* 1 parameter */
	test_assert(username_format_is_valid("user%d", &error));
	test_assert(username_format_is_valid("user%i", &error));
	test_assert(username_format_is_valid("user%04d", &error));
	test_assert(username_format_is_valid("%duser", &error));
	/* 2 parameters */
	test_assert(username_format_is_valid("u%d-d%d", &error));
	test_assert(username_format_is_valid("%i%d", &error));
	test_assert(username_format_is_valid("u%04d-d%04i", &error));
	/* Escaped percent */
	test_assert(username_format_is_valid("user%%d", &error));
	test_assert(username_format_is_valid("user%%d%d", &error));
	test_assert(username_format_is_valid("%%", &error));

	test_end();
}

static void test_username_format_invalid(void)
{
	const char *error;

	test_begin("username_format_is_valid() - invalid");

	/* Too many parameters */
	test_assert(!username_format_is_valid("%d%d%d", &error));
	test_assert(!username_format_is_valid("%i%i%i", &error));
	test_assert(!username_format_is_valid("u%d-d%d-s%d", &error));

	/* Unsupported specifiers */
	test_assert(!username_format_is_valid("user%s", &error));
	test_assert(!username_format_is_valid("user%u", &error));
	test_assert(!username_format_is_valid("user%x", &error));
	test_assert(!username_format_is_valid("user%p", &error));

	/* Mixed valid and invalid */
	test_assert(!username_format_is_valid("%d%s", &error));
	test_assert(!username_format_is_valid("%s%d", &error));

	test_end();
}

static void test_username_format_edge(void)
{
	const char *error;

	test_begin("username_format_is_valid() - edge cases");

	/* Empty string */
	test_assert(username_format_is_valid("", &error));

	/* Single percent */
	test_assert(!username_format_is_valid("%", &error));

	/* Trailing percent */
	test_assert(!username_format_is_valid("user%", &error));

	/* Only numbers after percent */
	test_assert(!username_format_is_valid("%04", &error));

	test_end();
}

int main(void)
{
	static void (*test_functions[])(void) = {
		test_username_format_valid,
		test_username_format_invalid,
		test_username_format_edge,
		NULL
	};

	return test_run(test_functions);
}
