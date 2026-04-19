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
	const struct test_connection *test_conn;
	unsigned int i, count, j, conn_count;

	printf("Parsing %s... ", path);
	fflush(stdout);

	parser = test_parser_init(path);
	tests = test_parser_get_tests(parser);
	testp = array_get(tests, &count);
	for (i = 0; i < count; i++) {
		printf("\n  Test: %s (require_user2: %s)\n", testp[i]->name,
		       testp[i]->require_user2 ? "YES" : "NO");
		test_conn = array_get(&testp[i]->connections, &conn_count);
		for (j = 0; j < conn_count; j++) {
			printf("    Connection %u username: %s\n", j + 1,
			       test_conn[j].username ? test_conn[j].username : "(null)");
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
	conf.username2_template = "user2_template_value";

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
