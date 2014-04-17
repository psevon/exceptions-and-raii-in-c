#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef ACU_THREAD_SAFE
	#include <pthread.h>
#endif

#include "exception.h"
#include "exc_std.h"
#include "exc_classes.h"
#include "autocleanup.h"
#include "acu_std.h"

void *acu_malloc(size_t s)
{
	void *p = malloc(s);
	if (p) (void)acu_new_unique(p, free);
	return p;
}

void *acu_malloc_t(size_t s)
{
	void *p = malloc_t(s);
	(void)acu_new_unique(p, free);
	return p;
}

void *acu_calloc(size_t n, size_t s)
{
	void *p = calloc(n, s);
	if (p) acu_new_unique(p, free);
	return p;
}

void *acu_calloc_t(size_t n, size_t s)
{
	void *p = calloc_t(n, s);
	acu_new_unique(p, free);
	return p;
}

void *acu_realloc(size_t s, acu_unique *a)
{
	void *p = realloc(acu_get_ptr(a), s);
	acu_update(a, p);
	if (p == NULL) acu_destruct(a);
	return p;
}

void *acu_realloc_t(size_t s, acu_unique *a)
{
	void *p = realloc(acu_get_ptr(a), s);
	acu_update(a, p);
	if (p == NULL)
	{
		acu_destruct(a);
		throw(new_mem_exception("acu_realloc_t", s));
	}
	return p;
}

char *acu_strdup(const char *s)
{
	char *p = strdup(s);
	if (p) (void)acu_new_unique(p, free);
	return p;
}

char *acu_strdup_t(const char *s)
{
	char *p = strdup_t(s);
	(void)acu_new_unique(p, free);
	return p;
}

FILE *acu_fopen(const char *s, const char *m)
{
	FILE *fi = fopen(s, m);
	if (fi) (void)acu_new_unique(fi, (void (*)(void *))fclose);
	return fi;
}

FILE *acu_fopen_t(const char *s, const char *m)
{
	FILE *fi = fopen_t(s, m);
	(void)acu_new_unique(fi, (void (*)(void *))fclose);
	return fi;
}

static void _acu_std_close(void *fd_voidptr)
{
	close((int)(long)fd_voidptr);
}

int acu_open(const char *s, int m)
{
	int fd = open(s, m);
	if (fd >= 0) (void)acu_new_unique((void *)(long)fd, _acu_std_close);
	return fd;
}

int acu_open_t(const char *s, int m)
{
	int fd = open_t(s, m);
	(void)acu_new_unique((void *)(long)fd, _acu_std_close);
	return fd;
}

#ifdef ACU_THREAD_SAFE
	acu_unique *acu_pthread_mutex_lock(pthread_mutex_t *lock)
	{
		//if (pthread_mutex_lock(lock)) ; //throw(new_io_error(errno, "", "acu_pthread_mutex_lock"));
		return acu_new_unique(lock, (void (*)(void *))pthread_mutex_unlock);	
	}
#endif

