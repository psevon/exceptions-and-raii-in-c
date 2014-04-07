#ifndef ACU_STD_H
#define ACU_STD_H

#include <stdio.h>
#ifdef ACU_THREAD_SAFE
	#include <pthread.h>
#endif
#include "autocleanup.h"

void *acu_malloc(size_t);
void *acu_malloc_t(size_t);
void *acu_calloc(size_t, size_t);
void *acu_calloc_t(size_t, size_t);
void *acu_realloc(size_t, acu_unique *);
void *acu_realloc_t(size_t, acu_unique *);

char *acu_strdup(const char *);
char *acu_strdup_t(const char *);
char *acu_strdup_demo(const char *);

FILE *acu_fopen(const char *, const char *);
FILE *acu_fopen_t(const char *, const char *);

int acu_open(const char *, int);
int acu_open_t(const char *, int);

#ifdef ACU_THREAD_SAFE
	acu_unique *acu_pthread_mutex_lock(pthread_mutex_t *lock);
#endif

#endif

