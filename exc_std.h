#ifndef EXC_STD_H
#define EXC_STD_H

void *malloc_t(size_t);
void *calloc_t(size_t, size_t);
void *realloc_t(void *, size_t);
void free_t(void *);
char *strdup_t(const char *);

FILE *fopen_t(const char *, const char *);
int open_t(const char *, int);

char *fgets_t(char *, int, FILE *);
int snprintf_t(char *, int, const char *, ...);
int fprintf_t(FILE *, const char *, ...);

char *strncpy_t(char *, const char *, int);

int system_t(const char *);
void system_t_fail(const char *);
#endif
