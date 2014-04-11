#include <stdio.h>
#include <errno.h>
#ifdef ACU_THREAD_SAFE
	#include <pthread.h>
	#include "acu_std.h"
#endif
#include "exception.h"
#include "exc_classes.h"
#include "exc_std.h"
#include "autocleanup.h"


/* Forward declarations */
static void _acu_del_forward_link(void *p);

/* Opaque classes */

/* Base class storing reference to the actual object, and its destructor function */
struct _acu_node {
	void *ptr;
	void (*del)(void *);
};

/* Class for unique object references */
struct _acu_stack_node {
	struct _acu_node base;
	acu_unique *next, *prev;
};

/* Class for shared object references */
struct _acu_shared_node {
	struct _acu_node base;
	acu_unique *tail;
	int refcnt;
	#ifdef ACU_THREAD_SAFE
		pthread_mutex_t lock;
	#endif
};

/* Global pointer to top of the main stack, and another pointer that either has the same value as _acu_stack_ptr, or NULL
 * _acu_latest is set to NULL at function entry, whenever it's accessed using acu_latest(), and when acu_attach or acu_destruct
 * is called */
__thread acu_unique *_acu_stack_ptr = NULL, *_acu_latest = NULL;


/* Destruct a unique node without updating stack pointers.
 * The caller must make sure that the stack pointer will be valid after cleanup. */
static void _acu_destruct(acu_unique *u)
{
	if (u->base.del) (u->base.del)(u->base.ptr);
	if (u->prev) u->prev->next = u->next;
	if (u->next) u->next->prev = u->prev;
	free(u);
}

/* Pop and destruct all unique nodes in a stack pointed to by *stack_ref_ptr, until node 'u' (exclusive), or
 * all nodes if u == NULL. */
void _acu_cleanup(acu_unique *u, acu_unique **stack_ptr_ref)
{
	if (u) u = u->prev;
	while (*stack_ptr_ref != u)
	{
		acu_unique *c = *stack_ptr_ref;
		*stack_ptr_ref = c->prev;
		_acu_destruct(c);
	}
}

/* Create a new unique node with reference to 'ptr' with destructor 'del' to stack pointed to by *tailptr.
 * Return pointer to the created node */
static acu_unique *_acu_new_unique(void *ptr, void (*del)(void *), acu_unique **stack_ptr_ref)
{
	acu_unique *u = calloc_t(1, sizeof(acu_unique));

	u->base.ptr = ptr;
	u->base.del = del;
	u->prev = _acu_stack_ptr;
	if (*stack_ptr_ref) (*stack_ptr_ref)->next = u;
	*stack_ptr_ref = u;
	return u;
}

/* Create a new unique node with reference to 'ptr' with destructor 'del' to the main stack, return pointer to the ctreated node */
acu_unique *acu_new_unique(void *ptr, void (*del)(void *)) {
	acu_unique *u = _acu_new_unique(ptr, del, &_acu_stack_ptr);
	_acu_latest = u;
	return u;
}

/* Create an empty unique node and return pointer to it */
acu_unique *acu_reserve(void) { return acu_new_unique(NULL, NULL); }


/* Get pointer to the latest unique node in the main stack. Will throw an exception if
 * 1) no unique nodes have been created within the same function,
 * 2) no unique nodes have been created after last call to acu_latest(), acu_share(), acu_destruct() or acu_sumit_to().
 */
acu_unique *acu_latest(void) {
	acu_unique *u = _acu_latest;
	if (u == NULL) throw(new_name_exception("acu_latest: no object available"));
	_acu_latest = NULL;
	return u;
}

/* Destruct unique node 'u' in the main stack. Adjust stack pointer if 'u' was on top of the stack. */
void acu_destruct(acu_unique *u)
{
	_acu_latest = NULL;
	if (u == _acu_stack_ptr) _acu_stack_ptr = u->prev;
	_acu_destruct(u);
}

/* Update pointer to the base object, follow chain of forward links first */
void acu_update(acu_unique *u, void *newptr)
{
	struct _acu_node *p = (struct _acu_node *)u;
	while (p->del == _acu_del_forward_link) p = p->ptr;
	p->ptr = newptr;
}

/* Dereference unique object pointer, follow chain of forward links first */
void *acu_dereference(acu_unique *u)
{
	struct _acu_node *p = (struct _acu_node *)u;
	while (p->del == _acu_del_forward_link) p = p->ptr;
	return p->ptr;
}

/* Assign unique object reference from 'from' to 'to'. This can be used to transfer ownership of unique node to an outer scope. */
void acu_transfer(acu_unique *from, acu_unique *to)
{
	to->base = from->base;
	from->base.del = NULL; // prevent the object whose ownership was transferred to 'to' from being destructed
	acu_destruct(from);
}

/* Destructor for a unique node pointing to a shared node */
static void _acu_del_forward_link(void *p)
{
	acu_shared *s = (acu_shared *)p;
	if (
	#ifndef ACU_THREAD_SAFE
		--s->refcnt == 0
	#else
		__sync_sub_and_fetch(&(s->refcnt), 1) == 0
	#endif
	)
	{
		_acu_cleanup(NULL, &(s->tail));
		if (s->base.del) (s->base.del)(s->base.ptr);
		#ifdef ACU_THREAD_SAFE
			(void)pthread_mutex_destroy(&(s->lock));
		#endif
		free(s);
	}	
}


/* Create a shared pointer pointing to the object unique pointer 'u' points to. Turn 'u' into a forward reference to the
 * shared node. Set reference count of the shared node to 1. Return pointer to the shared node. */
acu_shared *acu_share(acu_unique *u)
{
	acu_shared *s = calloc_t(1, sizeof(acu_shared));
	s->base = u->base;
	s->refcnt = 1;
	u->base.ptr = s;
	u->base.del = _acu_del_forward_link;
	#ifdef ACU_THREAD_SAFE
		if (pthread_mutex_init(&(s->lock), NULL)) throw(new_io_exception(errno, "", "acu_share"));
	#endif
	return s;
}

/* Create new unique node to main stack, pointing to shared node 's'. Increase reference count of 's' by one. */
acu_unique *acu_new_reference(acu_shared *s)
{
	acu_unique *u = acu_new_unique(s, _acu_del_forward_link);
	#ifndef ACU_THREAD_SAFE
		s->refcnt++;
	#else 
		(void)__sync_add_and_fetch(&(s->refcnt), 1);
	#endif
	return u;
}

/* Detach unique node 'u' from the main stack and push a copy of it to stack of shared object 's'.
 * Note that the library is designed so that user agent may not get handles to unique objects
 * detached from the main stack. This function is not called by other acu library functions, and
 * therefore it may rely on 'u' always being in the main stack */
void acu_submit_to(acu_unique *u, acu_shared *s)
{
	/* Make a copy of a to ensure that caller will not have a handle to the object after attaching */
	#ifdef ACU_THREAD_SAFE
		acu_unique *lockptr = acu_pthread_mutex_lock(&(s->lock));
	#endif
	acu_unique *b = _acu_new_unique(u->base.ptr, u->base.del, &(s->tail));
	#ifdef ACU_THREAD_SAFE
		acu_destruct(lockptr);
	#endif

	/* Unlink the previous object, do not call the destructor, since we are effectively just moving
         * the node to a new context. */
	if (u->prev) u->prev->next = u->next;
	if (u->next) u->next->prev = u->prev; else _acu_stack_ptr = u->prev;
	_acu_latest = NULL;
	free(u);
}

void _acu_atexit_cleanup(void) { _acu_cleanup(NULL, &_acu_stack_ptr); }
#ifdef ACU_THREAD_SAFE
	void _acu_thread_cleanup(void *dummy) { _acu_cleanup(NULL, &_acu_stack_ptr); }
#endif

