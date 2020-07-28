/*
 * Copyright 2011 The Chromium Authors, All Rights Reserved.
 * Copyright 2008 Jon Loeliger, Freescale Semiconductor, Inc.
 *
 * util_is_printable_string contributed by
 *	Pantelis Antoniou <pantelis.antoniou AT gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *                                                                   USA
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "util.h"
#include "version_non_gen.h"

char *xstrdup(const char *s)
{
	int len = strlen(s) + 1;
	char *d = xmalloc(len);

	memcpy(d, s, len);

	return d;
}

/* based in part from (3) vsnprintf */
int xasprintf(char **strp, const char *fmt, ...)
{
	int n, size = 128;	/* start with 128 bytes */
	char *p;
	va_list ap;

	/* initial pointer is NULL making the fist realloc to be malloc */
	p = NULL;
	while (1) {
		p = xrealloc(p, size);

		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);

		/* If that worked, return the string. */
		if (n > -1 && n < size)
			break;
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1; /* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2; /* twice the old size */
	}
	*strp = p;
	return strlen(p);
}

char *join_path(const char *path, const char *name)
{
	int lenp = strlen(path);
	int lenn = strlen(name);
	int len;
	int needslash = 1;
	char *str;

	len = lenp + lenn + 2;
	if ((lenp > 0) && (path[lenp-1] == '/')) {
		needslash = 0;
		len--;
	}

	str = xmalloc(len);
	memcpy(str, path, lenp);
	if (needslash) {
		str[lenp] = '/';
		lenp++;
	}
	memcpy(str+lenp, name, lenn+1);
	return str;
}

bool util_is_printable_string(const void *data, int len)
{
	const char *s = data;
	const char *ss, *se;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero */
	if (s[len - 1] != '\0')
		return 0;

	se = s + len;

	while (s < se) {
		ss = s;
		while (s < se && *s && isprint((unsigned char)*s))
			s++;

		/* not zero, or not done yet */
		if (*s != '\0' || s == ss)
			return 0;

		s++;
	}

	return 1;
}

/*
 * Parse a octal encoded character starting at index i in string s.  The
 * resulting character will be returned and the index i will be updated to
 * point at the character directly after the end of the encoding, this may be
 * the '\0' terminator of the string.
 */
static char get_oct_char(const char *s, int *i)
{
	char x[4];
	char *endx;
	long val;

	x[3] = '\0';
	strncpy(x, s + *i, 3);

	val = strtol(x, &endx, 8);

	assert(endx > x);

	(*i) += endx - x;
	return val;
}

/*
 * Parse a hexadecimal encoded character starting at index i in string s.  The
 * resulting character will be returned and the index i will be updated to
 * point at the character directly after the end of the encoding, this may be
 * the '\0' terminator of the string.
 */
static char get_hex_char(const char *s, int *i)
{
	char x[3];
	char *endx;
	long val;

	x[2] = '\0';
	strncpy(x, s + *i, 2);

	val = strtol(x, &endx, 16);
	if (!(endx  > x))
		die("\\x used with no following hex digits\n");

	(*i) += endx - x;
	return val;
}

char get_escape_char(const char *s, int *i)
{
	char	c = s[*i];
	int	j = *i + 1;
	char	val;

	switch (c) {
	case 'a':
		val = '\a';
		break;
	case 'b':
		val = '\b';
		break;
	case 't':
		val = '\t';
		break;
	case 'n':
		val = '\n';
		break;
	case 'v':
		val = '\v';
		break;
	case 'f':
		val = '\f';
		break;
	case 'r':
		val = '\r';
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		j--; /* need to re-read the first digit as
		      * part of the octal value */
		val = get_oct_char(s, &j);
		break;
	case 'x':
		val = get_hex_char(s, &j);
		break;
	default:
		val = c;
	}

	(*i) = j;
	return val;
}

void NORETURN util_version(void)
{
	printf("Version: %s\n", DTC_VERSION);
	exit(0);
}

void NORETURN util_usage(const char *errmsg, const char *synopsis,
			 const char *short_opts,
			 struct option const long_opts[],
			 const char * const opts_help[])
{
	FILE *fp = errmsg ? stderr : stdout;
	const char a_arg[] = "<arg>";
	size_t a_arg_len = strlen(a_arg) + 1;
	size_t i;
	int optlen;

	fprintf(fp,
		"Usage: %s\n"
		"\n"
		"Options: -[%s]\n", synopsis, short_opts);

	/* prescan the --long opt length to auto-align */
	optlen = 0;
	for (i = 0; long_opts[i].name; ++i) {
		/* +1 is for space between --opt and help text */
		int l = strlen(long_opts[i].name) + 1;
		if (long_opts[i].has_arg == a_argument)
			l += a_arg_len;
		if (optlen < l)
			optlen = l;
	}

	for (i = 0; long_opts[i].name; ++i) {
		/* helps when adding new applets or options */
		assert(opts_help[i] != NULL);

		/* first output the short flag if it has one */
		if (long_opts[i].val > '~')
			fprintf(fp, "      ");
		else
			fprintf(fp, "  -%c, ", long_opts[i].val);

		/* then the long flag */
		if (long_opts[i].has_arg == no_argument)
			fprintf(fp, "--%-*s", optlen, long_opts[i].name);
		else
			fprintf(fp, "--%s %s%*s", long_opts[i].name, a_arg,
				(int)(optlen - strlen(long_opts[i].name) - a_arg_len), "");

		/* finally the help text */
		fprintf(fp, "%s\n", opts_help[i]);
	}

	if (errmsg) {
		fprintf(fp, "\nError: %s\n", errmsg);
		exit(EXIT_FAILURE);
	} else
		exit(EXIT_SUCCESS);
}
