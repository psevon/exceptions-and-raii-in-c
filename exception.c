#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exception.h"

__thread struct exception *_exception_ptr = NULL;

__thread jmp_buf *_exc_context = NULL;

void _init_exception(struct exception *e, int type, void (*to_str)(char *, int), void (*del)(struct exception *))
{
	e->type = type; e->to_str = to_str; e->del = del;
}

void _del_exception(struct exception *e)
{
	free(e->file);
	(e->del)(e);
	free(e);
}

void _exc_default_handler(void)
{
	char buf[256];
	if (_exception_ptr->to_str) (_exception_ptr->to_str)(buf, sizeof(buf));
	else snprintf(buf, sizeof(buf), "type %d", _exception_ptr->type);
	fprintf(stderr, "Uncaught exception (%s, line %d): %s\n", _exception_ptr->file, _exception_ptr->line, buf);
	acu_exit(1);
}

void _exc_clear(void)
{
	if (_exception_ptr == NULL) return;
	_del_exception(_exception_ptr);
	_exception_ptr = NULL;
}


