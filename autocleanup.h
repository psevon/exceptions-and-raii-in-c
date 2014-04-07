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

void _acu_cleanup(acu_unique *a, acu_unique **tailptr);
acu_unique *acu_new_unique(void *ptr, void (*del)(void *));
acu_unique *acu_latest(void);
void acu_destruct(acu_unique *a);
void acu_update(acu_unique *a, void *p);
void acu_submit_to(acu_unique *a, acu_shared *s);
acu_unique *acu_reserve(void);
acu_shared *acu_share(acu_unique *a);
acu_unique *acu_new_reference(acu_shared *s);
void acu_transfer(acu_unique *from, acu_unique *to);
void *acu_dereference(acu_unique *a);
void *acu_dereference_shared(acu_shared *s);

void _acu_atexit_cleanup(void);
#define acu_init atexit(_acu_atexit_cleanup);
#ifdef ACU_THREAD_SAFE
	void _acu_thread_cleanup(void *);
	#define acu_init_thread phthread_cleanup_push(_acu_thread_cleanup, NULL);
#endif

#define BEGIN { acu_unique *_acu_stack_ptr_scope = _acu_stack_ptr, *_acu_stack_ptr_fn = _acu_stack_ptr; _acu_latest = NULL;
#define BEGIN_SCOPE { acu_unique *_acu_stack_ptr_scope = _acu_stack_ptr; jmp_buf _acu_scope_context; if (setjmp(_acu_scope_context) == 0) {

#define END_SCOPE } _acu_cleanup(_acu_stack_ptr_scope, &_acu_stack_ptr); }
#define END _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); }

#define acu_exit_scope longjmp(_acu_scope_context, 1)
#define acu_exit(v) { _acu_cleanup(NULL, &_acu_stack_ptr); exit(v); }
#define acu_return(v) { _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); return (v); }
#define acu_return_void { _acu_cleanup(_acu_stack_ptr_fn, &_acu_stack_ptr); return; }

#endif

