#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"
#include "exc_classes.h"

static void _exc_to_str_nomem(char *buf, int n) { (void)snprintf(buf, n, "Out of heap memory"); }
static void _exc_del_nomem(struct exception *e) { return; }
struct exception _exc_sys_nomem_g = {EXCTYPE_NOMEM, _exc_to_str_nomem, _exc_del_nomem, "", 0};


/* Exception type specific functions: each type t requires four functions defined here
 * - _exc_to_str_<t>(char *buf, int n)		// Create string description of the exception to buffer buf
 * - _exc_del_<t>(struct exception *e)		// Destructor for exception e
 * - new_<t>_exception(...)			// Create exception of type <t>, arguments are type-dependent
 * - <t>_exception(e)				// Return correctly casted pointer to e, or NULL if not correct type
 * and in exception.h, struct <t>_exception */

/* NAME */
static void _exc_to_str_name(char *buf, int n)
{
	snprintf(buf, n, "name_exception: '%s'", name_exception(_exception_ptr)->name);
}

static void _exc_del_name(struct exception *e)
{
	struct name_exception *dd = name_exception(e);
	if (dd == NULL) return;
	if (dd->name) free(dd->name);
}

struct exception *new_name_exception(const char *name)
{
	struct name_exception *e = malloc(sizeof(struct name_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->name = strdup(name);	// use strdup_t instead
	_init_exception(&(e->e), EXCTYPE_NAME, _exc_to_str_name, _exc_del_name);
	return &(e->e);
}

struct name_exception *name_exception(struct exception *e) { return (struct name_exception *)(e->type == EXCTYPE_NAME ? e : NULL); }

/* IO */
static void _exc_to_str_io(char *buf, int n)
{
	snprintf(buf, n, "io_exception: errno=%d, function '%s', filename '%s'",
		io_exception(_exception_ptr)->err,
		io_exception(_exception_ptr)->function,
		io_exception(_exception_ptr)->filename
	);	
}

static void _exc_del_io(struct exception *e)
{
	struct io_exception *dd = io_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
	if (dd->filename) free(dd->filename);
}

struct exception *new_io_exception(int err, const char *filename, const char *function)
{
	struct io_exception *e = malloc(sizeof(struct io_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->err = err;
	e->filename = strdup(filename);	// use strdup_t instead
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_IO, _exc_to_str_io, _exc_del_io);
	return &(e->e);
}

struct io_exception *io_exception(struct exception *e) { return (struct io_exception *)(e->type == EXCTYPE_IO ? e : NULL); }

/* MEM */
static void _exc_to_str_mem(char *buf, int n)
{
	snprintf(buf, n, "mem_exception: function '%s', size %ld", mem_exception(_exception_ptr)->function, mem_exception(_exception_ptr)->size);
}

static void _exc_del_mem(struct exception *e)
{
	struct mem_exception *dd = mem_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
}

struct exception *new_mem_exception(const char *function, long size)
{
	struct mem_exception *e = malloc(sizeof(struct mem_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->size = size;
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_MEM, _exc_to_str_mem, _exc_del_mem);
	return &(e->e);
}

struct mem_exception *mem_exception(struct exception *e) { return (struct mem_exception *)(e->type == EXCTYPE_MEM ? e : NULL); }


/* TRUNC */
static void _exc_to_str_trunc(char *buf, int n)
{
	snprintf(buf, n, "trunc_exception: function '%s', bufsize %ld", trunc_exception(_exception_ptr)->function, trunc_exception(_exception_ptr)->bufsize);
}

static void _exc_del_trunc(struct exception *e)
{
	struct trunc_exception *dd = trunc_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
}

struct exception *new_trunc_exception(const char *function, long bufsize)
{
	struct trunc_exception *e = malloc(sizeof(struct trunc_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->bufsize = bufsize;
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_TRUNC, _exc_to_str_trunc, _exc_del_trunc);
	return &(e->e);
}

struct trunc_exception *trunc_exception(struct exception *e) { return (struct trunc_exception *)(e->type == EXCTYPE_TRUNC ? e : NULL); }

/* NULLPTR */
static void _exc_to_str_nullptr(char *buf, int n)
{
	snprintf(buf, n, "nullptr_exception: function '%s'", nullptr_exception(_exception_ptr)->function);
}

static void _exc_del_nullptr(struct exception *e)
{
	struct nullptr_exception *dd = nullptr_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
}

struct exception *new_nullptr_exception(const char *function)
{
	struct nullptr_exception *e = malloc(sizeof(struct nullptr_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_NULLPTR, _exc_to_str_nullptr, _exc_del_nullptr);
	return &(e->e);
}

struct nullptr_exception *nullptr_exception(struct exception *e) { return (struct nullptr_exception *)(e->type == EXCTYPE_NULLPTR ? e : NULL); }

/* SIG */
static void _exc_to_str_sig(char *buf, int n)
{
	snprintf(buf, n, "sig_exception: function '%s', signal %d", sig_exception(_exception_ptr)->function, sig_exception(_exception_ptr)->signal);
}

static void _exc_del_sig(struct exception *e)
{
	struct sig_exception *dd = sig_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
}

struct exception *new_sig_exception(const char *function, int signal)
{
	struct sig_exception *e = malloc(sizeof(struct sig_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->signal = signal;
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_SIG, _exc_to_str_sig, _exc_del_sig);
	return &(e->e);
}

struct sig_exception *sig_exception(struct exception *e) { return (struct sig_exception *)(e->type == EXCTYPE_SIG ? e : NULL); }

/* FAIL */
static void _exc_to_str_fail(char *buf, int n)
{
	snprintf(buf, n, "fail_exception: function '%s' returned %d", fail_exception(_exception_ptr)->function, fail_exception(_exception_ptr)->retval);
}

static void _exc_del_fail(struct exception *e)
{
	struct fail_exception *dd = fail_exception(e);
	if (dd == NULL) return;
	if (dd->function) free(dd->function);
}

struct exception *new_fail_exception(const char *function, int retval)
{
	struct fail_exception *e = malloc(sizeof(struct fail_exception));
	if (e == NULL) throw(&_exc_sys_nomem_g);	// Global const that can be thrown even if no memory can be allocated from the heap
	e->retval = retval;
	e->function = strdup(function);
	_init_exception(&(e->e), EXCTYPE_FAIL, _exc_to_str_fail, _exc_del_fail);
	return &(e->e);
}

struct fail_exception *fail_exception(struct exception *e) { return (struct fail_exception *)(e->type == EXCTYPE_FAIL ? e : NULL); }

