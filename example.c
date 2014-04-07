#include <stdio.h>
#include <string.h>
#include "autocleanup.h"
#include "exception.h"
#include "exc_classes.h"
#include "acu_std.h"

/* Example program demonstrating use of RAII in C
 * ----------------------------------------------
 * Simple program that calls main->h->g->f, constructs unique objects in differents scopes, and demonstrates
 * ownership transfer and sharing. */
 

/* unique node references can be allocated from the heap, but this doesn't mean that the scope of the node is global. Scope of
 * an unique node is always the scope in which it was allocated. Global variables provide one means for passing reference to the
 * nodes as in this example. */
acu_unique *q;	


/* Demo destructor that prints to stdout */
static void free_demo(char *p)
{
	fprintf(stderr, "Destructing string '%s'\n", p);
	free(p);
}

/* Demo constructor that allocates a duplicate of a string, and prints a message to stdout */
char *acu_strdup_demo(const char *s)
{
	char *p = strdup(s);
	fprintf(stderr, "Allocated string '%s'\n", p);
	if (p) (void)acu_new_unique(p, (void (*)(void *))free_demo);
	return p;
}



char *f(int x, acu_unique *r)
BEGIN
	printf("...enter f\n");
	/* Local scope objects: allocated object is to be destructed when going out of the
	 * current scope. The function does not need to obtain a pointer to the actual acu_unique
	 * node at all. */
	char *s1 = acu_strdup_demo("String allocated in f");

	/* Transfer of scope: allocate an object in this scope, and then pass ownership to another acu_unique node
         * received as an argument. */
	char *s2 = acu_strdup_demo("String allocated in f but to be destructed at the end of h");
	acu_transfer(acu_latest(), r);

	/* Object sharing: allocate an object in this scope, create an acu_shared node out of it, replacing the
	 * original acu_unique node with a forward link to the shared node. */
	char *s3 = acu_strdup_demo("Shared string allocated in f");
	acu_shared *s = acu_share(acu_latest());

	/* If thrown, the only reference to the shared object will go out of scope here,
	 * and both the reference and the shared object will be destructed asap,
	 * in practice at the end of the CATCH block catching the exception. */
	if (x == 2) throw(new_name_exception("Got two as argument"));

	/* Create new reference to the shared object and pass its ownersip to object pointed by global 'q', whose scope is
	 * the inner scope in main. */
	acu_transfer(acu_new_reference(s), q);

	if (x == 3) throw(new_fail_exception("fake-fail-exception", -1));

	printf("...exit f\n");
	acu_return(s2);
END

char *g(int x, acu_unique *r)
{
	printf("..enter g\n");

	/* Scope of this object is actually function h, since definition of g is not enclosed in BEGIN..END brackets. */
	char *s = acu_strdup_demo("String allocated in g");

	char *p = f(x, r);
	printf("..exit g\n");

	/* Even if BEGIN..END brackets were used, exit by return instead of acu_return()
	 * macro would escape the automatic end-of-scope cleanup, but only until the next
	 * proper exit from a scope, or ultimately end of program. */
	return p;
}

char *h(int x)
BEGIN
	/* Create an empty unique node to this scope for receiving ownership of an object created by f. */
	acu_unique *r = acu_reserve();
	char *s;
	printf(".enter h\n");
	TRY
		s = g(x, r);
		printf("Function g returned %s\n", s);
		printf("Got handle to shared string %s\n", (char *)acu_dereference(q));
	CATCH(e)
		printf("Enter catch block of h\n");
		struct name_exception *n = name_exception(e);
		if (n) printf("Caught name exception: %s\n", n->name);
		else rethrow;
	TRY_END
	printf(".exit h\n");
	acu_return(s);
END

int main(int argc, char *argv[])
BEGIN
	acu_init;
	
	int x = (argc > 1 ? atoi(argv[1]) : -1);
	printf("enter main\n");
	char *s = acu_strdup_demo("String allocated in main");
	BEGIN_SCOPE
		printf("enter main/inner scope\n");
		q = acu_reserve();	
		char *s = h(x);
		printf("exit main/inner scope\n");
		/* unique node pointed to by 'q' goes out of scope, and will be destructed,
		 * reference count to shared object goes to zero, and the shared object
		 * will also be destructed */
	END_SCOPE
	printf("exit main\n");
	acu_return(0);
END

