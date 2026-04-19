/* Copyright (c) ImapTest authors, see the included COPYING file */

#include "lib.h"
#include "username-format.h"
#include <string.h>

int count_printf_ints(const char *s, const char **error_r)
{
	int ints = 0;
	const char *perc = s;
	while((perc = strchr(perc, '%')) != NULL) {
		char c;
		if(perc[1] == '%') {
			perc += 2;
			continue;
		}
		while((c = *++perc), (c >= '0' && c <= '9'))
			;
		if(c != 'd' && c != 'i') {
			*error_r = "username format can only have %i or %d "
				   "format specifiers";
			return -1;
		}
		ints++;
		perc++;
	}
	i_assert(ints >= 0);
	return ints;
}

bool username_format_is_valid(const char *s, const char **error_r)
{
	/* All this does is ensure that there are at most 2, and only,
	 * "%d"s or "%i"s in the format string. If you mess up the '@',
	 * that's your problem. i.e. it makes our printf safe.
	 */
	int ints=count_printf_ints(s, error_r);
	if (ints < 0) {
		/* count_printf_ints sets error_r in this case. */
		return FALSE;
	} else if (ints > 2) {
		*error_r = "username format can have at most two "
			   "integer parameters";
	} else {
		return TRUE;
	}
	return FALSE;
}
