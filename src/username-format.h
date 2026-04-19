#ifndef USERNAME_FORMAT_H
#define USERNAME_FORMAT_H

#include "lib.h"

int count_printf_ints(const char *s, const char **error_r);
bool username_format_is_valid(const char *s, const char **error_r);

#endif
