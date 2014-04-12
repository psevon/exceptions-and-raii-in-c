In the C software projects I'm involved in I grew tired of the excessive
boiler plate code for error checking and releasing resources in exceptional
cases, and despite all the effort there's always a leak here and there.  

I wanted to have an exception handling mechanism like most modern programming
languages have, allowing for centralized error handling. The critical issue
with exception handling, often overlooked, is releasing the acquired
resources. There are two differing philosophies:
- RAII (e.g., C++), in which destructors are automatically called for any
  objects going out of their scope. This work also in cases where functions
  let exceptions pass through uncatched.
- finally blocks (e.g., java), which contain code that releases the resources.

I strongly prefer the first option, since in my opinion finally blocks make
exception handling mechanisms to be no more than syntactic sugar if non-leaky
code is pursued; it practically means that exceptions must be catched in every
function that does acquire some resources. There is garbage collection of
course, but there's no guarantee when it's invoked and for some resources such
as mutex locks that's not good enough.

There are numerous exception handling libraries available for C, most based on
setjmp/longjmp, some providing finally blocks, and some even have support for
local-scope RAII. But for true RAII we need more, we need smartpointers.

This project intends to provide C++ like exception handling and smartpointer
functionality to C programs; RAII style programming, and no explicit cleanup
procedures in "finally" blocks. This project provides both unique pointers
that always have a single owner, but ownership is transferrable, and shared,
reference counted object with strong and weak references.

The project is divided in the following library modules:

- exception.{c,h} define basic exception handling functionality and macros
  defining TRY-CATCH-TRY_END macro brackets, and macros throw(e) and rethrow
  for throwing exceptions.
- exc_classes.{c,h} provide definitions for some useful exception classes
- exc_std.{c,h} implement wrappers for some standard C library functions that
  throw exceptions in case of failure

- autocleanup.{c,h} provides basic smartpointer functionality by defining
  classes acu_unique and acu_shared (analogous to unique_ptr and shared_ptr
  in C++, or more precisely, acu_shared is analogous to the counter object
  pointed to by shared_ptr), functions for managing them, and macro brackets
  BEGIN..END and BEGIN_SCOPE..END_SCOPE, which should be used instead of
  braces at least in those parts of code that assume automatic cleanup, and
  macros acu_return(v), acu_return_void, acu_exit_scope and acu_exit for
  clean exit from a scope. See file "smartpointers_in_c.txt" for a more
  detailed description of this library.

- acu_std.{c,h} provides wrappers for some standard library constructors
  (such as malloc, fopen, pthread_mutex_lock, ...) that create unique
  pointers to the resources making them subject to automatic cleanup.

See simple program "example.c" for examples on use of exceptions and
smartpointers.

gcc *.c -o example

