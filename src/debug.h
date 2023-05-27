////////////////////////////////////////////////////////////////////////////
//
// debug.h
//
// debug macros
//
// Macros for conditionnal compilation :
// INLINE, MY_ERROR, ASSERT, FATAL, CHECKPOINT, TRACE
// (see the following code for details)
//
// Remi Coulom
//
// April, 1996
//
////////////////////////////////////////////////////////////////////////////
#ifndef DEBUG_H
#define DEBUG_H

#ifndef NO_DEBUG_AT_ALL

#include <iostream>
#include <cstdlib>

//
// MY_ERROR
//
#define MY_ERROR(s) do {                                                   \
 std::cout << '\n' << s << '\n';                                           \
 std::cout << "File: " << __FILE__ << '\n';                                \
 std::cout << "Line: " << __LINE__ << '\n';                                \
 exit(1);} while(0)

//
// FATAL(x)
//
#define FATAL(x) do if (x) MY_ERROR("Fatal error: " << #x); while(0)

//
// INLINE, ASSERT(x), CHECKPOINT()
//

#ifdef DEBUG
#define INLINE
#define TRACE(s) do {std::cout << s << '\n';} while(0)
#define ASSERT(x) do if (!(x)) MY_ERROR("Assertion failed: "<<#x); while(0)
#define CHECKPOINT() do {                                                  \
 std::cout << '?';                                                         \
 std::cout.flush();                                                        \
 int x = cin.get();                                                        \
 if (x != '\n')                                                            \
  exit(1);                                                                 \
} while(0);
#endif
#endif

#ifndef INLINE
#define INLINE inline
#define TRACE(s)
#define ASSERT(x)
#define CHECKPOINT()
#endif

#ifndef FATAL
#define FATAL(x)
#endif

#endif
