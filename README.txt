This project intends to provide C++ like exception handling and 
smartpointer functionality to C programs. There are several existing 
setjmp/longjmp based implementations for exceptions, but exceptions are 
not very useful without automatic cleanup. The goal of this project was 
to produce an exception handling framework that is closer to the 
philosophy in C++: avoiding the need for explicit cleanup procedures in 
"finally" blocks by RAII style programming, where resources are released 
automatically when they go out of scope. This project provides both 
unique pointers that always have a single owner, but ownership is 
transferrable, and shared pointers with reference counters.

The project is divided in the following library modules:

- exception.{c,h} define basic exception handling functionality and macros
  defining TRY-CATCH-TRY_END macro brackets, and macros throw(e) and rethrow
  for throwing exceptions.
- exc_classes.{c,h} provide definitions for some useful exception classes
- exc_std.{c,h} implement wrappers for some standard C library functions that
  throw exceptions in case of failure

- autocleanup.{c,h} provides basic smartpointer functionality by defining
  classes acu_unique and acu_shared (analogous to unique_ptr and shared_ptr
  in C++), functions for managing them, and macro brackets BEGIN..END and
  BEGIN_SCOPE..END_SCOPE, which should be used instead of braces at least in
  those parts of code that assume automatic cleanup, and macros acu_return(v),
  acu_return_void, acu_exit_scope and acu_exit for clean exit from a scope.
  See file "smartpointers_in_c.txt" for a more detailed description of this
  library.

- acu_std.{c,h} provides wrappers for some standard library constructors
  (such as malloc, fopen, pthread_mutex_lock, ...) that create a unique
  pointers to the resources making them subject to automatic cleanup.

See simple program "example.c" for examples on use of exceptions and
smartpointers.

gcc *.c -o example

