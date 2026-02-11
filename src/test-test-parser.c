#include "lib.h"
#include "str.h"
#include "test-common.h"
#include "settings.h"
#include "test-parser.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Mock global variables */
struct settings conf;
bool profile_running = FALSE;

/* Include source directly to test static functions */
#include "test-parser.c"

static void test_test_parser_read_test_success(void)
{
	struct test_parser *parser;
	struct test *test;
	const char *test_dir = ".test-parser-dir";
	const char *test_file = "test.test";
	const char *test_path;
	int fd;
	const char *content =
		"connections: 1\n"
		"\n"
		"login \"user\" \"pass\"\n"
		"ok login\n";

	test_begin("test_parser_read_test success");

	/* Create temp dir for init */
	if (mkdir(test_dir, 0700) < 0 && errno != EEXIST)
		i_fatal("mkdir(%s) failed: %m", test_dir);

	test_path = t_strdup_printf("%s/%s", test_dir, test_file);

	/* Create a dummy test file */
	fd = open(test_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1)
		i_fatal("open(%s) failed: %m", test_path);

	if (write(fd, content, strlen(content)) < 0)
		i_fatal("write() failed: %m");
	close(fd);

	/* Initialize parser with the directory containing our test file */
	parser = test_parser_init(test_dir);

	test_assert(array_count(&parser->tests) == 1);
	if (array_count(&parser->tests) > 0) {
		test = *array_idx(&parser->tests, 0);
		test_assert(strcmp(test->path, test_path) == 0);
		test_assert(test->connection_count == 1);

		/* Verify command parsing (expect 2 groups: login and logout) */
		test_assert(array_count(&test->cmd_groups) == 2);
		if (array_count(&test->cmd_groups) > 0) {
			struct test_command_group *group = *array_idx(&test->cmd_groups, 0);
			test_assert(array_count(&group->commands) == 1);
			if (array_count(&group->commands) > 0) {
				const struct test_command *cmd = array_idx(&group->commands, 0);
				test_assert(strcmp(cmd->command, "login \"user\" \"pass\"") == 0);
			}
		}
	}

	/* Clean up */
	test_parser_deinit(&parser);
	unlink(test_path);
	rmdir(test_dir);

	test_end();
}

static void test_test_parser_read_test_direct(void)
{
	struct test_parser *parser;
	struct test *test;
	const char *test_path = "direct.test";
	int ret;
	int fd;
	const char *content =
		"connections: 2\n"
		"\n"
		"1 login \"user\" \"pass\"\n"
		"ok login\n";

	test_begin("test_parser_read_test direct call");

	/* Create dummy file */
	fd = open(test_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1) i_fatal("open failed");
	if (write(fd, content, strlen(content)) < 0) i_fatal("write failed");
	close(fd);

	/* Create parser manually to avoid scanning dir */
	if (mkdir("empty_dir", 0700) < 0 && errno != EEXIST) i_fatal("mkdir failed");
	parser = test_parser_init("empty_dir");
	rmdir("empty_dir");

	ret = test_parser_read_test(parser, test_path);
	test_assert(ret == 1);

	test_assert(array_count(&parser->tests) == 1);
	if (array_count(&parser->tests) > 0) {
		test = *array_idx(&parser->tests, 0);
		test_assert(test->connection_count == 2);
	}

	test_parser_deinit(&parser);
	unlink(test_path);
	test_end();
}

static void test_test_parser_read_test_pipelining(void)
{
	struct test_parser *parser;
	struct test *test;
	const char *test_path = "pipeline.test";
	int ret;
	int fd;
	const char *content =
		"connections: 2\n"
		"\n"
		"1 tag1 login \"user\" \"pass\"\n"
		"1 tag2 logout\n"
		"tag1 ok login\n"
		"tag2 ok logout\n";

	test_begin("test_parser_read_test pipelining");

	fd = open(test_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd == -1) i_fatal("open failed");
	if (write(fd, content, strlen(content)) < 0) i_fatal("write failed");
	close(fd);

	if (mkdir("empty_dir_pipe", 0700) < 0 && errno != EEXIST) i_fatal("mkdir failed");
	parser = test_parser_init("empty_dir_pipe");
	rmdir("empty_dir_pipe");

	ret = test_parser_read_test(parser, test_path);
	test_assert(ret == 1);

	if (array_count(&parser->tests) > 0) {
		test = *array_idx(&parser->tests, 0);
		test_assert(array_count(&test->cmd_groups) == 1); /* They are in same group */

		if (array_count(&test->cmd_groups) > 0) {
			struct test_command_group *group = *array_idx(&test->cmd_groups, 0);
			test_assert(array_count(&group->commands) == 2);
		}
	}

	test_parser_deinit(&parser);
	unlink(test_path);
	test_end();
}

int main(void)
{
	static void (*test_functions[])(void) = {
		test_test_parser_read_test_success,
		test_test_parser_read_test_direct,
		test_test_parser_read_test_pipelining,
		NULL
	};

	memset(&conf, 0, sizeof(conf));
	conf.username_template = "user%d";

	return test_run(test_functions);
}
