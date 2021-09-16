/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
# define PURE __attribute__((__pure__))
#else
# define PURE
#endif


struct group {
	char *key;
	struct group *next;
	struct group *prev;
	char **elems;
	size_t nelems;
	size_t elems_size;
	int numerical;
};


static const char *argv0 = "median";

static struct group groups_head;
static struct group groups_tail;


PURE static int
isnumerical(const char *s)
{
	if (*s == '+' || *s == '-')
		s++;
	if (s[0] == '.' && !s[1])
		return 0;
	while (isdigit(*s))
		s++;
	if (*s == '.') {
		s++;
		while (isdigit(*s))
			s++;
	}
	return *s ? 0 : 1;
}


PURE static int
cmp_num(const void *apv, const void *bpv)
{
	const char *a = *(const char *const *)apv;
	const char *b = *(const char *const *)bpv;
	int mul = 1;
	size_t an = 0, bn = 0, i;

	if (*a == '-' || *b == '-') {
		mul = -1;
		a = &a[1];
		b = &b[1];
	} else if (*a == '-') {
		return -1;
	} else if (*b == '-') {
		return +1;
	} else {
		a = &a[*a == '+'];
		b = &b[*b == '+'];
	}

	while (*a == '0') a++;
	while (*b == '0') b++;

	while (a[an] && a[an] != '.') an++;
	while (b[bn] && b[bn] != '.') bn++;

	if (an != bn)
		return an < bn ? -mul : mul;

	for (i = 0; a[i] && a[i] == b[i]; i++);
	a = &a[i];
	b = &b[i];

	if (i > an) {
		if (!*a)
			while (*b == '0') b++;
		if (!*b)
			while (*a == '0') a++;
	}

	if (!*a && !*b) {
		return 0;
	} else if (!*a) {
		return -mul;
	} else if (!*b) {
		return mul;
	} else if (*a < *b) {
		return -mul;
	} else {
		return mul;
	}
}


static int
cmp_str(const void *apv, const void *bpv)
{
	const char *a = *(const char *const *)apv;
	const char *b = *(const char *const *)bpv;
	return strcmp(a, b);
}


static int
avg(char *a, const char *b)
{
	size_t i;
	int carry = 0, val;
	for (i = 0; a[i]; i++) {
		val = (a[i] & 15) + (b[i] & 15);
		carry = val & 1;
		a[i] = (char)((val >> 1) | '0');
	}
	return carry;
}


static int
subavg(char *a, const char *b)
{
	size_t i;
	int carry = 0, val;
	for (i = 0; a[i]; i++) {
		val = (a[i] & 15) - (b[i] & 15);
		carry = val & 1;
		a[i] = (char)((val >> 1) | '0');
	}
	return carry;
}


static void
median2(const char *low, const char *high, const char *key)
{
	int low_plus  = *low  == '+', low_minus  = *low  == '-', low_dot;
	int high_plus = *high == '+', high_minus = *high == '-', high_dot;
	size_t low_int, low_frac = 0, high_int, high_frac = 0;
	size_t max_int, max_frac, i;
	char *low2, *high2, *tmp;
	const char *prefix;
	int carry;

	for (low_int  = 0; low[low_int]   && low[low_int]   != '.'; low_int++);
	for (high_int = 0; high[high_int] && high[high_int] != '.'; high_int++);
	low_dot  = low[low_int]   == '.';
	high_dot = high[high_int] == '.';

	low  = &low[low_plus | low_minus];
	high = &high[high_plus | high_minus];

	if (low_dot)
		low_frac = strlen(&low[low_int + 1]);
	if (high_dot)
		high_frac = strlen(&high[high_int + 1]);

	max_int  = low_int  > high_int  ? low_int  : high_int;
	max_frac = low_frac > high_frac ? low_frac : high_frac;

	low2 = malloc(max_int + max_frac + 1);
	high2 = malloc(max_int + max_frac + 1);
	if (!low2 || !high2) {
		perror(argv0);
		exit(1);
	}
	low2[max_int + max_frac] = '\0';
	high2[max_int + max_frac] = '\0';

	memset(low2, '0', max_int - low_int);
	memcpy(&low2[max_int - low_int], low, low_int);
	memcpy(&low2[max_int], &low[low_int + 1], low_frac);
	memset(&low2[max_int + low_frac], '0', max_frac - low_frac);

	memset(high2, '0', max_int - high_int);
	memcpy(&high2[max_int - high_int], high, high_int);
	memcpy(&high2[max_int], &high[high_int + 1], high_frac);
	memset(&high2[max_int + high_frac], '0', max_frac - high_frac);

	if (low_minus && high_minus) {
		prefix = "-";
		carry = avg(high2, low2);
	} else if (low_minus) {
		for (i = 0; low2[i] && low2[i] == high2[i]; i++);
		if (low2[i] <= high2[i]) {
			prefix = "+";
			carry = subavg(high2, low2);
		} else {
			prefix = "-";
			carry = subavg(low2, high2);
			tmp = low2, low2 = high2, high2 = tmp;
		}
	} else {
		prefix = (low_plus || high_plus) ? "+" : "";
		carry = avg(high2, low2);
	}

	printf("%s%.*s%s""%s%s%s\n",
	       prefix, (int)max_int, high2,
	       (carry || max_frac) ? "." : "", &high2[max_int], carry ? "5" : "", key);

	free(low2);
	free(high2);
}


int
main(int argc, char *argv[])
{
	char *line = NULL, *key;
	struct group *group = NULL;
	size_t size = 0;
	ssize_t len;

	groups_head.next = &groups_tail;
	groups_tail.prev = &groups_head;

	if (argc) {
		argv0 = *argv++, argc--;
		if (argc && argv[0][0] == '-' && argv[0][1] == '-' && !argv[0][2])
			argv++, argc--;
		if (argc) {
			fprintf(stderr, "usage: %s\n", argv0);
			return 1;
		}
	}

	while ((len = getdelim(&line, &size, '\n', stdin)) > 0) {
		if (len && line[--len] == '\n')
			line[len] = '\0';

		for (key = line; *key && !isspace(*key); key++);
		if (group && !strcmp(group->key, key))
			goto found_group;
		for (group = groups_head.next; group->key; group = group->next)
			if (!strcmp(group->key, key))
				goto found_group;
		group = calloc(1, sizeof(*group));
		if (!group) {
			perror(argv0);
			return 1;
		}
		group->key = strdup(key);
		if (!group->key) {
			perror(argv0);
			return 1;
		}
		group->numerical = 1;
		group->prev = groups_tail.prev;
		group->next = &groups_tail;
		groups_tail.prev->next = group;
		groups_tail.prev = group;

	found_group:
		if (group->nelems == group->elems_size) {
			if (group->elems_size > SIZE_MAX / 2 / sizeof(*group->elems)) {
				errno = ENOMEM;
				perror(argv0);
				return 1;
			}
			group->elems_size = group->elems_size ? group->elems_size * 2 : 16;
			group->elems = realloc(group->elems, group->elems_size * sizeof(*group->elems));
			if (!group->elems) {
				perror(argv0);
				return 1;
			}
		}
		*key = '\0';
		group->elems[group->nelems] = strdup(line);
		if (!group->elems[group->nelems]) {
			perror(argv0);
			return 1;
		}
		if (group->numerical)
			if (!isnumerical(line))
				group->numerical = 0;
		group->nelems++;
	}

	if (ferror(stdin)) {
		perror(argv0);
		return 1;
	}

	free(line);
	while ((group = groups_head.next)->key) {
		qsort(group->elems, group->nelems, sizeof(*group->elems), group->numerical ? cmp_num : cmp_str);
		if (group->nelems % 2 || !group->numerical)
			printf("%s%s\n", group->elems[(group->nelems - 1) / 2], group->key);
		else if (group->nelems)
			median2(group->elems[group->nelems / 2 - 1], group->elems[group->nelems / 2 - 0], group->key);
		groups_head.next = group->next;
		while (group->nelems--)
			free(group->elems[group->nelems]);
		free(group->elems);
		free(group->key);
		free(group);
	}

	if (fflush(stdout) || ferror(stdout) || fclose(stdout)) {
		perror(argv0);
		return 1;
	}
	return 0;
}
