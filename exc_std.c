#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "exception.h"
#include "exc_classes.h"
#include "exc_std.h"

void *malloc_t(size_t s)
{
	void *p = malloc(s);
	if (p == NULL) throw(new_mem_exception("malloc", s));
	return p;
}

void *calloc_t(size_t n, size_t s)
{
	void *p = calloc(n, s);
	if (p == NULL) throw(new_mem_exception("calloc", n * s));
	return p;
}

void *realloc_t(void *ptr, size_t s)
{
	void *p = realloc(ptr, s);
	if (p == NULL) throw(new_mem_exception("realloc", s));
	return p;
}

void free_t(void *p)
{
	if (p == NULL) throw(new_nullptr_exception("free"));
	free(p);
}

char *strdup_t(const char *s)
{
	char *p = strdup(s);
	if (p == NULL) throw(new_mem_exception("strdup", strlen(s) + 1));
}

FILE *fopen_t(const char *n, const char *m)
{
	FILE *f = fopen(n, m);
	if (f == NULL) throw(new_io_exception(errno, n, "fopen"));
	return f;
}

int open_t(const char *n, int m)
{
	int fd = open(n, m);
	if (fd < 0) throw(new_io_exception(errno, n, "open"));
	return fd;
}

char *fgets_t(char *s, int n, FILE *f)
{
	char *p = fgets(s, n, f);
	if (p == NULL) throw(new_io_exception(errno, "", "fgets"));
	if (s[strlen(s)-1] != '\n') throw(new_trunc_exception("fgets", n));
	return p;
}

int snprintf_t(char *s, int n, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int m = vsnprintf(s, n, fmt, ap);
	if (m < 0) throw(new_name_exception("vsnprint error"));
	if (m >= n) throw(new_trunc_exception("vsnprintf", n));
	return m;
}

int fprintf_t(FILE *f, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int m = vfprintf(f, fmt, ap);
	if (m < 0) throw(new_io_exception(errno, "", "vfprintf"));
	return m;
}

char *strncpy_t(char *d, const char *s, int n)
{
	char *r = strncpy(d, s, n);
	if (strlen(s) + 1 > n) throw(new_trunc_exception("strncpy", n));
	return r;
}

int system_t(const char *cmd)
{
	int v = system(cmd);
	if (WIFEXITED(v)) return WEXITSTATUS(v);
	if (WIFSIGNALED(v)) throw(new_sig_exception("system", WTERMSIG(v)));
	throw(new_fail_exception("system", v));
}

void system_t_fail(const char *cmd)
{
	int v = system_t(cmd);
	if (v) throw(new_fail_exception("system", v));
}

