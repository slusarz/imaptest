/* Copyright (c) ImapTest authors, see the included COPYING file */

#include "lib.h"
#include "test-parser.h"
#include "settings.h"
#include "array.h"
#include "str.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

struct settings conf;

static void test_parse_path(const char *path)
{
	struct test_parser *parser;
	const ARRAY_TYPE(test) *tests;
	struct test *const *testp;

	printf("Parsing %s... ", path);
	fflush(stdout);

	parser = test_parser_init(path);
	tests = test_parser_get_tests(parser);

	array_foreach(tests, testp) {
		struct test *test = *testp;
		const struct test_connection *conns;
		unsigned int count;

		printf("\n  Test: %s (require_user2=%s)\n", test->name,
		       test->require_user2 ? "yes" : "no");
		conns = array_get(&test->connections, &count);
		for (unsigned int i = 0; i < count; i++) {
			printf("    Conn %u: user=%s\n", i + 1,
			       conns[i].username ? conns[i].username : "(null)");
		}
	}

	test_parser_deinit(&parser);

	printf("OK\n");
}

int main(int argc, char *argv[])
{
	unsigned int count;
	ARRAY_TYPE(const_string) files;
	const char *const *fpaths;
	const char *path;
	struct stat st;

	lib_init();
	set_conf_default(&conf);
	conf.username_template = "user1-tmpl";
	conf.username2_template = "user2-tmpl";

	if (argc > 1)
		path = argv[1];
	else {
		fprintf(stderr, "Usage: test-test-parser <test file or directory>\n");
		return 1;
	}

	if (stat(path, &st) < 0) {
		fprintf(stderr, "stat(%s) failed: %s\n", path, strerror(errno));
		return 1;
	}

	t_array_init(&files, 64);

	if (S_ISDIR(st.st_mode)) {
		DIR *dir = opendir(path);
		struct dirent *d;

		if (dir == NULL) {
			fprintf(stderr, "opendir(%s) failed: %s\n", path, strerror(errno));
			return 1;
		}

		while ((d = readdir(dir)) != NULL) {
			const char *fname = d->d_name;
			if (fname[0] == '.')
				continue;

			size_t len = strlen(fname);
			if (len >= 5 && strcmp(fname + len - 5, ".mbox") == 0)
				continue;

			const char *fpath = t_strdup_printf("%s/%s", path, fname);
			struct stat fst;
			if (lstat(fpath, &fst) < 0)
				continue;

			if (S_ISDIR(fst.st_mode)) {
				printf("Skipping subdirectory %s\n", fpath);
				continue;
			}
			array_append(&files, &fpath, 1);
		}
		closedir(dir);
	} else {
		array_append(&files, &path, 1);
	}

	array_sort(&files, i_strcmp_p);

	fpaths = array_get(&files, &count);
	for (unsigned int i = 0; i < count; i++)
		test_parse_path(fpaths[i]);

	lib_deinit();
	return 0;
}
