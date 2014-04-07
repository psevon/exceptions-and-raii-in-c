#ifndef EXC_CLASSES_H
#define EXC_CLASSES_H

#include "exception.h"

#define EXCTYPE_NOMEM 0
#define EXCTYPE_NAME 1
#define EXCTYPE_IO 2
#define EXCTYPE_MEM 3
#define EXCTYPE_TRUNC 4
#define EXCTYPE_NULLPTR 5
#define EXCTYPE_SIG 6
#define EXCTYPE_FAIL 7

extern struct exception _exc_sys_nomem_g;

/* name_exception */
struct name_exception {
	struct exception e;
	char *name;
};

struct exception *new_name_exception(const char *name);
struct name_exception *name_exception(struct exception *e);

/* io_exception */
struct io_exception {
	struct exception e;
	int err;
	char *filename;
	char *function;
};

struct exception *new_io_exception(int err, const char *filename, const char *function);
struct io_exception *io_exception(struct exception *e);

/* trunc_exception */
struct trunc_exception {
	struct exception e;
	char *function;
	long bufsize;
};

struct exception *new_trunc_exception(const char *function, long bufsize);
struct trunc_exception *trunc_exception(struct exception *e);

/* mem_exception */
struct mem_exception {
	struct exception e;
	char *function;
	long size;
};

struct exception *new_mem_exception(const char *function, long size);
struct mem_exception *mem_exception(struct exception *e);

/* nullptr_exception */
struct nullptr_exception {
	struct exception e;
	char *function;
};

struct exception *new_nullptr_exception(const char *function);
struct nullptr_exception *nullptr_exception(struct exception *e);

/* sig_exception */
struct sig_exception {
	struct exception e;
	char *function;
	int signal;
};

struct exception *new_sig_exception(const char *function, int signal);
struct sig_exception *sig_exception(struct exception *e);

/* fail_exception */
struct fail_exception {
	struct exception e;
	char *function;
	int retval;
};

struct exception *new_fail_exception(const char *function, int retval);
struct fail_exception *fail_exception(struct exception *e);

#endif
