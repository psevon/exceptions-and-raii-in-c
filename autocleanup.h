#ifndef AUTOCLEANUP_H
#define AUTOCLEANUP_H

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>


/* Author Petteri Sevon
*  
* AutoCleanUp (ACU): Library for implementing RAII in C
*
* This library provides a mechanism for RAII style resource management; allocated
* resources are objects that are automatically destructed upon leaving their scope.
* This mechanism can be leveraged for existing constructors such as malloc or fopen
* by using wrapper functions that, after successful construction, push a uniquely owned
* smartpointer (acu_unique), pointer to the resource and its respective destructor
* function, to a thread-specific cleanup stack. See acu_std.c for implementation
* of such wrappers for commonly use standard C resource allocators.
*
* The library supports transferring ownership of an acu_unique object to another
* acu_unique object in order to extend its scope. Also, an acu_unique object can be
* copied into an acu_shared object. This is analogous to shared/weak_ptr in C++, but
* implemented differently: there's exactly one acu_shared object for a resource,
* containing strong and weak reference counters and pointers to the resource and its
* destructor, and this shared pointer is referenced (strongly or weakly) by any number
* of acu_unique objects or other acu_shared objects.
*
* The library is compatible (and intended for use) with another library, exception.h,
* that implements exceptions as a macro extension. Resources allocated above the scope
* catching an exception are automatically released. Using this mechanism eliminates
* the need for having "finally" blocks after try-catch. Using these two libraries will
* result in much less boiler plate code, less resource leaks, and more robust detection
* of errors, as silently ignoring errors indicated by return values is no longer possible.
*
* Because accessing the resources through the unique smartpointers is not very attractive
* option in C (performance hit from extra indirection, explicit casting due to lack of
* templates, and lack of operator overloading), I would recommend using direct pointers
* for accessing the objects and smartpointers for managing their ownership and lifespan.
* Actually, the contructors in acu_std.c only return the direct pointer to the allocated
* resource. This is because for the most common usage where the scope of the resource
* is the current scope, the caller does not need to do anything else. If the ownership
* of the resource has to be passed outside the current scope, or the resource must be
* shared, it's possible to get a pointer to the acu_unique smartpointer by calling
* acu_latest() immediately after the constructor call.
*
* The implementation defines macro brackets BEGIN ... END replacing normal curly brackets
* { ... } around function definition, and optional inner context brackets BEGIN_SCOPE ...
* END_SCOPE. Stack pointers are memorized at entry to any scope opened with a macro bracket,
* and the closing macro bracket automatically releases any resources pushed to the
* stack above the stack pointer memorized at entry to the scope.
*
* Since the closing macro brackets cannot catch early exits from their scope, leaving
* the scope using goto, continue, break or return will not trigger the destructors.
* However, the destruct operations will remain in the stack and will be done when the
* cleaning up is triggered next time. Inner scope can be exited using macro acu_exit_scope(),
* which will trigger cleaning up. Macro acu_return can be used to return with cleanup of
* the entire function scope from anywhere within the function.
*
* Resources allocated using the acu_ wrappers may not be freed using the normal
* function calls such as free. If they need to be explicitly released before
* end of their scope, use acu_destruct(u) to release the resource.
*/


struct _acu_stack_node;
typedef struct _acu_stack_node acu_unique;

struct _acu_shared_node;
typedef struct _acu_shared_node acu_shared;

extern __thread acu_unique *_acu_stack_ptr, *_acu_latest;

/* Private cleanup functions, required in the header because the macros use them */
void _acu_cleanup(acu_unique *u, acu_unique **tailptr);
void _acu_atexit_cleanup(void);
#ifdef ACU_THREAD_SAFE
	void _acu_thread_cleanup(void *);
#endif

/* Create a new unique pointer to object 'ptr' with destructor 'del', return pointer to it */
acu_unique *acu_new_unique(void *ptr, void (*del)(void *));

/* Get pointer to the latest unique node. Will throw an exception if
 * 1) no unique nodes have been created within the same function, or
 * 2) no unique nodes have been created after last call to acu_latest(), acu_share(), acu_destruct() or acu_submit_to(). */
acu_unique *acu_latest(void);

/* Explicit destruction of unique pointer *u. usually not needed, but may be useful for releasing critical resources
 * suc as locks as early as possible. */
void acu_destruct(acu_unique *u);

/* Update the pointer to an object, he only motivation for having this is realloc */
void acu_update(acu_unique *u, void *p);

/* Detach unique pointer 'u' from the cleanup stack and push a copy of it to cleanup stack of shared pointer 's'. */
void acu_submit_to(acu_unique *u, acu_shared *s);

/* Create an empty unique node (for receiving ownership by acu_transfer) and return pointer to it */
acu_unique *acu_reserve(void);

/* Create a shared pointer pointing to the same object unique pointer 'u' points to. Turn 'u' into a forward reference to the
 * shared node. Set reference count of the shared node to 1. Return pointer to the acu_shared object. */
acu_shared *acu_share(acu_unique *u);

/* Create new acu_unique object pointing to shared node 's'. Increase reference count of 's' by one, and increase weak reference count if reference count was zero. */
acu_unique *acu_new_reference(acu_shared *s);

/* Copy unique object reference from 'from' to 'to', and unlink unique pointer 'from'. This can be used to transfer ownership of unique node to an outer scope. */
void acu_transfer(acu_unique *from, acu_unique *to);

/* Swap the contents of two unique pointers */
void acu_swap(acu_unique *a, acu_unique *b);

/* Get pointer to managed object, follow chain of forward links first */
void *acu_get_ptr(acu_unique *u);

/* Create new acu_unique object with a weak reference to shared node 's'. Increase weak reference count of 's' by one. */
acu_unique *acu_new_weak_reference(acu_shared *s);

/* Obtain a strong reference to a shared pointer from a weak reference. If the object is already destructed, return NULL. */
acu_unique *acu_lock_reference(acu_unique *weakptr);


#define acu_init atexit(_acu_atexit_cleanup);
#ifdef ACU_THREAD_SAFE
	#define acu_init_thread phthread_cleanup_push(_acu_thread_cleanup, NULL);
#endif

#define BEGIN { acu_unique *_acu_stack_ptr_scope = acu_reserve(), *_acu_stack_ptr_fn = _acu_stack_ptr_scope; _acu_latest = NULL;
#define BEGIN_SCOPE { acu_unique *_acu_stack_ptr_scope = acu_reserve(); jmp_buf _acu_scope_context; if (setjmp(_acu_scope_context) == 0) {

#define END_SCOPE } _acu_cleanup(_acu_stack_ptr_scope, &_acu_stack_ptr); }
#define END _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); }

#define acu_exit_scope longjmp(_acu_scope_context, 1)
#define acu_exit(v) { _acu_cleanup(NULL, &_acu_stack_ptr); exit(v); }

/* This is designed to be usable in any context plain return can be used. Uses "while" instead of "if",
 * because "if" would break things if there's an "else" right after acu_return. */
#define acu_return while (_acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr), 1) return

#endif

