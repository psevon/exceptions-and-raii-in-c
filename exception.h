#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <setjmp.h>
#include <string.h>
#include "autocleanup.h"

struct exception {
	int type;
	void (*to_str)(char *, int);
	void (*del)(struct exception *);
	char *file;
	int line;
};

extern __thread jmp_buf *_exc_context;
extern __thread struct exception *_exception_ptr;

void _init_exception(struct exception *, int type, void (*to_str)(char *buf, int n), void (*del)(struct exception *));

void _exc_default_handler(void);
void _exc_clear(void);

#define TRY BEGIN_SCOPE \
	jmp_buf _new_context, *_prev_context = _exc_context; \
	_exc_context = &_new_context; \
	if (setjmp(_new_context) == 0) {


#define CATCH(e) _exc_context = _prev_context; \
	} else { \
	struct exception *e = _exception_ptr; \
	_exc_context = _prev_context;

#define TRY_END _exc_clear(); } END_SCOPE

#define throw(e) { struct exception *_e = (e); \
	if (_e && _e != _exception_ptr) { \
		_exc_clear(); _exception_ptr = _e; \
		_e->line = __LINE__; _e->file = strdup(__FILE__); \
	} \
	if (_exc_context) longjmp(*_exc_context, 1); \
	_exc_default_handler(); }

#define rethrow throw(NULL)

#endif

