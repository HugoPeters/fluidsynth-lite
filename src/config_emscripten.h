#define VERSION "1.0-warlock"

#define HAVE_STRING_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDIO_H 1
#define HAVE_MATH_H 1
#define HAVE_STDARG_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_IO_H 1
#define HAVE_WINDOWS_H 0
#define HAVE_STDINT_H 1

#ifndef __STDC_NO_ATOMICS__
#define HAVE_ATOMIC_H 1
#endif

#ifdef __EMSCRIPTEN_PTHREADS__
#define HAVE_PTHREAD_H 1
#endif

#if HAVE_PTHREAD_H
#define FLUID_USE_THREADING 1
#endif


#define DSOUND_SUPPORT 0
#define WINMIDI_SUPPORT 0
#define WITH_FLOAT 1

#define strcasecmp _stricmp

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define WITH_PROFILING 0

#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#pragma warning(disable : 4305)
#pragma warning(disable : 4996)

#ifndef inline
#define inline __inline
#endif

typedef int socklen_t;
