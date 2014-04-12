#ifndef AUTOCLEANUP_H
#define AUTOCLEANUP_H

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>


/* Author Petteri Sevon
*  
* AutoCleanUp (ACU): Library for implementing RAII in C
* The implementation defines macro brackets BEGIN ... END replacing normal
* curly brackets { ... } around function definition, and optional inner context
* brackets BEGIN_SCOPE ... END_SCOPE. 

* The library provides a mechanism for RAII style resource management; allocated
* resources are objects that are automatically destructed upon leaving their scope.
* This mechanism can be leveraged by wrapping existing constructors inside a
* function that, after successful construction, push the respective destruct operation
* together with a pointer to the object, to a thread-specific stack. This is very
* similar concept to unique_ptr in C++ 11. See acu_std.c for implementation of such
* wrappers for commonly use standard C resource allocators.
*
* Stack pointers are memorized at entry to any scope opened with a macro bracket,
* and the closing macro bracket automatically releases any resources pushed to the
* stack above the stack pointer memorized at entry to the scope.
*
* Since the closing macro brackets cannot catch early exits from their scope,
* leaving the scope using goto, continue, break or return will not trigger the destructors.
* However, the destruct operations will remain in the stack and will be done
* when the cleaning up is triggered next time. Inner scope can be exited using
* macro acu_exit_scope(), which will trigger cleaning up. Macro acu_return(v) can
* be used to do return v with cleanup of the entire function scope from anywhere
* within the function.
*
* Due to limitations of C, resources allocated using the acu_ wrappers may not
* be freed using the normal function calls. if they need to be explicitly released,
* get an ACU handle to the resource using acu_unique a = acu_latest() right after
* constructor call, and use acu_destruct(a) to release the resource.
*
* The library supports transferring ownership of an acu_unique object to another
* acu_unique object in order to extend its scope. Also, a acu_unique object can be
* copied into an acu_shared object (analogous to shared_ptr in C++), that keeps a
* reference counter. Upon creation of shared obect, the unique object is replaced
* with a reference to the shared object.
*
* The library is compatible (and intended for use) with another library, exception.h,
* that implements exceptions as a macro extension. Resources allocated above the scope
* catching an exception are automatically released. Using this mechanism eliminates
* the need for having "finally" blocks after try-catch.
*
* Using these two libraries will result in much less boiler plate code, less resource
* leaks, and more robust detection of errors, as silently ignoring errors indicated by
* return values is no longer possible.
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

/* Create new acu_unique object pointing to shared node 's'. Increase reference count of 's' by one. */
acu_unique *acu_new_reference(acu_shared *s);

/* Copy unique object reference from 'from' to 'to', and unlink unique pointer 'from'. This can be used to transfer ownership of unique node to an outer scope. */
void acu_transfer(acu_unique *from, acu_unique *to);

/* Swap the contents of two unique pointers */
void acu_swap(acu_unique *a, acu_unique *b);

/* Dereference unique pointer, follow chain of forward links first */
void *acu_dereference(acu_unique *u);


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
#define acu_return(v) { _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); return (v); }
#define acu_return_void { _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); return; }

#endif

