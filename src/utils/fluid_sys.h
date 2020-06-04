/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


/**

   This header contains a bunch of (mostly) system and machine
   dependent functions:

   - timers
   - current time in milliseconds and microseconds
   - debug logging
   - profiling
   - memory locking
   - checking for floating point exceptions

 */

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "fluidsynth_priv.h"

/**
 * Macro used for safely accessing a message from a GError and using a default
 * message if it is NULL.
 * @param err Pointer to a GError to access the message field of.
 * @return Message string
 */
#define fluid_gerror_message(err)  ((err) ? err->message : "No error details")


void fluid_sys_config(void);
void fluid_log_config(void);
void fluid_time_config(void);


/* Misc */

#define fluid_return_val_if_fail(expr, val) if (!(expr)) return (val)
#define fluid_return_if_fail(expr)          if (!(expr)) return
#define FLUID_INLINE                        inline
#define FLUID_POINTER_TO_UINT(x)            ((size_t)(x))
#define FLUID_POINTER_TO_INT(x)             ((intptr_t)(x))
#define FLUID_N_ELEMENTS(struct)            (sizeof (struct) / sizeof (struct[0]))

// TODO: Add proper big endianess check
#define FLUID_IS_BIG_ENDIAN                 false // (G_BYTE_ORDER == G_BIG_ENDIAN)

/*
 * Utility functions
 */
char *fluid_strtok (char **str, char *delim);


/**

  Additional debugging system, separate from the log system. This
  allows to print selected debug messages of a specific subsystem.
 */

extern unsigned int fluid_debug_flags;

#if DEBUG

enum fluid_debug_level {
    FLUID_DBG_DRIVER = 1
};

int fluid_debug(int level, char * fmt, ...);

#else
#define fluid_debug
#endif


#if defined(__OS2__)
#define INCL_DOS
#include <os2.h>

typedef int socklen_t;
#endif

unsigned int fluid_curtime(void);
double fluid_utime(void);
void fluid_msleep(unsigned int msec);


/**
    Timers

 */

/* if the callback function returns 1 the timer will continue; if it
   returns 0 it will stop */
typedef int (*fluid_timer_callback_t)(void* data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

fluid_timer_t* new_fluid_timer(int msec, fluid_timer_callback_t callback,
                               void* data, int new_thread, int auto_destroy,
                               int high_priority);

int delete_fluid_timer(fluid_timer_t* timer);
int fluid_timer_join(fluid_timer_t* timer);
int fluid_timer_stop(fluid_timer_t* timer);

/* Muteces */

#if FLUID_ENABLE_THREADING 

#if HAVE_PTHREAD_H && !defined(PLATFORM_WIN)

static FLUID_INLINE void
fluid_pthread_mutex_init(pthread_mutex_t *m, int kind)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, kind);
    pthread_mutex_init(m, &attr);
}
/* Regular mutex */
typedef pthread_mutex_t fluid_mutex_t;
#define fluid_mutex_init(_m)      pthread_mutex_init (&(_m), NULL)
#define fluid_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))

/* Recursive lock capable mutex */
typedef pthread_mutex_t fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      fluid_pthread_mutex_init(&(_m), PTHREAD_MUTEX_RECURSIVE)
#define fluid_rec_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_rec_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef pthread_mutex_t fluid_cond_mutex_t;
#define fluid_cond_mutex_init(m)      pthread_mutex_init(m, NULL)
#define fluid_cond_mutex_destroy(m)   pthread_mutex_destroy(m)
#define fluid_cond_mutex_lock(m)      pthread_mutex_lock(m)
#define fluid_cond_mutex_unlock(m)    pthread_mutex_unlock(m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex (void)
{
    fluid_cond_mutex_t *mutex;
    mutex = malloc(sizeof(fluid_cond_mutex_t));
    fluid_cond_mutex_init(mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex (fluid_cond_mutex_t *m)
{
    fluid_cond_mutex_destroy(m);
    free(m);
}

/* Thread condition signaling */
typedef pthread_cond_t fluid_cond_t;
#define fluid_cond_init(cond)           pthread_cond_init(cond, NULL)
#define fluid_cond_destroy(cond)        pthread_cond_destroy(cond)
#define fluid_cond_signal(cond)         pthread_cond_signal(cond)
#define fluid_cond_broadcast(cond)      pthread_cond_broadcast(cond)
#define fluid_cond_wait(cond, mutex)    pthread_cond_wait(cond, mutex)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond (void)
{
    fluid_cond_t *cond;
    cond = malloc(sizeof(fluid_cond_t));
    fluid_cond_init(cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond (fluid_cond_t *cond)
{
    fluid_cond_destroy(cond);
    free(cond);
}

/* Thread private data */

typedef pthread_key_t fluid_private_t;
#define fluid_private_init(_priv)                  pthread_key_create(&_priv, NULL)
#define fluid_private_free(_priv)
#define fluid_private_get(_priv)                   pthread_getspecific((_priv))
#define fluid_private_set(_priv, _data)            pthread_setspecific((_priv), (_data))

/* Threads */

#define FLUID_THREAD_RETURN_TYPE void *
#define FLUID_THREAD_RETURN_VALUE NULL

typedef pthread_t fluid_thread_t;
typedef void *(*fluid_thread_func_t)(void* data);

#define FLUID_THREAD_ID_NULL            NULL                   /* A NULL "ID" value */
#define fluid_thread_id_t               pthread_t              /* Data type for a thread ID */
#define fluid_thread_get_id()           pthread_self()         /* Get unique "ID" for current thread */

fluid_thread_t* new_fluid_thread(const char *name, fluid_thread_func_t func, void *data,
                                 int prio_level, int detach);
void delete_fluid_thread(fluid_thread_t* thread);
void fluid_thread_self_set_prio (int prio_level);
int fluid_thread_join(fluid_thread_t* thread);

#elif defined(PLATFORM_WIN)

static FLUID_INLINE void
fluid_win32_mutex_init(PHANDLE m)
{
    *m = CreateMutex(NULL, TRUE, NULL);
}

/* Regular mutex */
typedef SRWLOCK fluid_mutex_t;
#define fluid_mutex_init(_m)      InitializeSRWLock(&(_m))
#define fluid_mutex_destroy(_m)
#define fluid_mutex_lock(_m)      AcquireSRWLockExclusive(&(_m))
#define fluid_mutex_unlock(_m)    ReleaseSRWLockExclusive(&(_m))

/* Recursive lock capable mutex */
typedef SRWLOCK fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      InitializeSRWLock(&(_m))
#define fluid_rec_mutex_destroy(_m)
#define fluid_rec_mutex_lock(_m)      AcquireSRWLockShared(&(_m))
#define fluid_rec_mutex_unlock(_m)    ReleaseSRWLockShared(&(_m))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef CRITICAL_SECTION fluid_cond_mutex_t;
#define fluid_cond_mutex_init(_m)      InitializeCriticalSection(_m)
#define fluid_cond_mutex_destroy(_m)   DeleteCriticalSection(_m)
#define fluid_cond_mutex_lock(_m)      EnterCriticalSection(_m)
#define fluid_cond_mutex_unlock(_m)    LeaveCriticalSection(_m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex (void)
{
    fluid_cond_mutex_t *mutex;
    mutex = (fluid_cond_mutex_t*)malloc(sizeof(fluid_cond_mutex_t));
    fluid_cond_mutex_init(mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex (fluid_cond_mutex_t *m)
{
    fluid_cond_mutex_destroy(m);
    free(m);
}

/* Thread condition signaling */
typedef CONDITION_VARIABLE fluid_cond_t;
#define fluid_cond_init(cond)           InitializeConditionVariable(cond)
#define fluid_cond_destroy(cond)
#define fluid_cond_signal(cond)         WakeConditionVariable(cond)
#define fluid_cond_broadcast(cond)      WakeAllConditionVariable(cond)
#define fluid_cond_wait(cond, mutex)    SleepConditionVariableCS(cond, mutex, INFINITE)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond (void)
{
    fluid_cond_t *cond;
    cond = (fluid_cond_t*)malloc(sizeof(fluid_cond_t));
    fluid_cond_init(cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond (fluid_cond_t *cond)
{
    fluid_cond_destroy(cond);
    free(cond);
}

/* Thread private data */

typedef DWORD fluid_private_t;
#define fluid_private_init(_priv)                  (_priv = TlsAlloc())
#define fluid_private_free(_priv)                  TlsFree(_priv);
#define fluid_private_get(_priv)                   TlsGetValue(_priv)
#define fluid_private_set(_priv, _data)            TlsSetValue(_priv, _data)

/* Threads */

#define FLUID_THREAD_RETURN_TYPE DWORD WINAPI
#define FLUID_THREAD_RETURN_VALUE 0

typedef HANDLE fluid_thread_t;
typedef DWORD (WINAPI *fluid_thread_func_t)(void* data);

#define FLUID_THREAD_ID_NULL            0                      /* A NULL "ID" value */
#define fluid_thread_id_t               DWORD                  /* Data type for a thread ID */
#define fluid_thread_get_id()           GetCurrentThreadId()   /* Get unique "ID" for current thread */

fluid_thread_t* new_fluid_thread(const char *name, fluid_thread_func_t func, void *data,
                                 int prio_level, int detach);
void delete_fluid_thread(fluid_thread_t* thread);
void fluid_thread_self_set_prio (int prio_level);
int fluid_thread_join(fluid_thread_t* thread);

#endif

#else // FLUID_USE_THREADING

/* Regular mutex */
typedef int fluid_mutex_t;
#define fluid_mutex_init(_m)        ((void)((_m)))
#define fluid_mutex_destroy(_m)     ((void)((_m)))
#define fluid_mutex_lock(_m)        ((void)0)
#define fluid_mutex_unlock(_m)      ((void)0)

/* Recursive lock capable mutex */
typedef int fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)    ((void)((_m)))
#define fluid_rec_mutex_destroy(_m) ((void)0)
#define fluid_rec_mutex_lock(_m)    ((void)0)
#define fluid_rec_mutex_unlock(_m)  ((void)((_m)))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef int fluid_cond_mutex_t;
#define fluid_cond_mutex_init(_m)       ((void)0)
#define fluid_cond_mutex_destroy(_m)    ((void)0)
#define fluid_cond_mutex_lock(_m)       ((void)0)
#define fluid_cond_mutex_unlock(_m)     ((void)0)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex(void)
{
    fluid_cond_mutex_t *mutex;
    mutex = (fluid_cond_mutex_t*)malloc(sizeof(fluid_cond_mutex_t));
    fluid_cond_mutex_init(mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex(fluid_cond_mutex_t *m)
{
    fluid_cond_mutex_destroy(m);
    free(m);
}

/* Thread condition signaling */
typedef int fluid_cond_t;
#define fluid_cond_init(cond)
#define fluid_cond_destroy(cond)
#define fluid_cond_signal(cond)
#define fluid_cond_broadcast(cond)
#define fluid_cond_wait(cond, mutex)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond(void)
{
    fluid_cond_t *cond;
    cond = (fluid_cond_t*)malloc(sizeof(fluid_cond_t));
    fluid_cond_init(cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond(fluid_cond_t *cond)
{
    fluid_cond_destroy(cond);
    free(cond);
}

/* Thread private data */

typedef void** fluid_private_t;
#define fluid_private_init(_priv)                  ((void)((_priv = (fluid_private_t) malloc(sizeof(void*))), 0))
#define fluid_private_free(_priv)                  free((_priv))
#define fluid_private_get(_priv)                   (*(_priv))
#define fluid_private_set(_priv, _data)            ((void)((*(_priv) = (_data)), 0))

/* Threads */
#define FLUID_THREAD_RETURN_VALUE 0
#define FLUID_THREAD_RETURN_TYPE int

typedef void* fluid_thread_t;
typedef void *(*fluid_thread_func_t)(void* data);

#define FLUID_THREAD_ID_NULL            0                      /* A NULL "ID" value */

#endif

static inline int
fluid_private_get_int(fluid_private_t priv)
{
    union {
        void *p;
        int i;
    } un;

    un.p = fluid_private_get(priv);
    return un.i;
}

static inline void
fluid_private_set_int(fluid_private_t priv, int data)
{
    union {
        void *p;
        int i;
    } un;

    un.i = data;
    fluid_private_set(priv, un.p);
}

/* Atomic operations */

#include <stdbool.h>

#ifndef STATIC_ASSERT
#if __STDC_VERSION__ >= 201112L
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT(expr, msg) typedef char __static_assertion_ ## __COUNTER__[(expr) ? 1 : -1]
#endif
#endif //STATIC_ASSERT

#if FLUID_USE_THREADING

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || defined(__riscos__)

typedef struct {
    volatile int value;
} fluid_atomic_int;
typedef struct {
    volatile unsigned value;
} fluid_atomic_uint;
typedef struct {
    volatile float value;
} fluid_atomic_float;

#define fluid_atomic_int_add(atomic, val) __extension__ ({            \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),     \
                          "Atomic must be the size of an int");       \
            __sync_fetch_and_add(&(atomic)->value, (val));})

#define fluid_atomic_int_get(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            (atomic)->value;})

#define fluid_atomic_int_set(atomic, newval) __extension__ ({         \
                STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                              "Atomic must be the size of an int");   \
                (atomic)->value = (newval);                           \
                __sync_synchronize();})

#define fluid_atomic_int_inc(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            __sync_fetch_and_add(&(atomic)->value, 1);})

#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) __extension__ ({ \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),       \
                          "Atomic must be the size of an int");         \
            __sync_bool_compare_and_swap(&(atomic)->value, (oldval), (newval));})

#define fluid_atomic_float_get(atomic) __extension__ ({   \
  STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                "Atomic must be the size of a float");    \
  __sync_synchronize();                                   \
  (atomic)->value;})

#define fluid_atomic_float_set(atomic, val) __extension__ ({  \
      STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                    "Atomic must be the size of a float");    \
      (atomic)->value = (val);                                \
      __sync_synchronize();})

#elif defined(PLATFORM_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef volatile LONG fluid_atomic_int;
typedef volatile ULONG fluid_atomic_uint;
typedef volatile LONG fluid_atomic_float;

#define fluid_atomic_int_inc(atomic) InterlockedIncrement((atomic))
#define fluid_atomic_int_add(atomic, val) InterlockedAdd((atomic), (val))
#define fluid_atomic_int_get(atomic) (*(LONG*)(atomic))
#define fluid_atomic_int_set(atomic, val) InterlockedExchange((atomic), (val))
#define fluid_atomic_int_exchange_and_add(atomic, add)  \
    InterlockedExchangeAdd((atomic), (add))

#define fluid_atomic_float_get(atomic) (*(FLOAT*)(atomic))

static inline float
fluid_atomic_float_set(fluid_atomic_float *atomic, float val)
{
    LONG ival = *(LONG*)&val;
    LONG rval = InterlockedExchange(atomic, ival);
    return *(float*)&rval;
}

#endif

#else // FLUID_ENABLE_THREADING

typedef volatile int32_t fluid_atomic_int;
typedef volatile uint32_t fluid_atomic_uint;
typedef volatile float_t fluid_atomic_float;

static FLUID_INLINE int fluid_atomic_int_compare_and_exchange(volatile int *_pi, int _old, int _new)
{
    if (*_pi == _old)
    {
        *_pi = _new;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static FLUID_INLINE int fluid_atomic_int_exchange_and_add(volatile int *_pi, int _add)
{
    int tmp = *_pi;
    *_pi += _add;
    return tmp;
}

#define fluid_atomic_int_inc(atomic) ((void)((*(atomic))++))
#define fluid_atomic_int_add(atomic, val) fluid_atomic_int_exchange_and_add(atomic, val)
#define fluid_atomic_int_get(atomic) (*(atomic))
#define fluid_atomic_int_set(atomic, val) ((void)(*(atomic) = (val)))

static FLUID_INLINE void
fluid_atomic_float_set(volatile float *fptr, float val)
{
    int32_t ival;
    memcpy(&ival, &val, 4);
    fluid_atomic_int_set((volatile int *)fptr, ival);
}

static FLUID_INLINE float
fluid_atomic_float_get(volatile float *fptr)
{
    int32_t ival;
    float fval;
    ival = fluid_atomic_int_get((volatile int *)fptr);
    memcpy(&fval, &ival, 4);
    return fval;
}

#endif


/* Sockets and I/O */

fluid_istream_t fluid_get_stdin (void);
fluid_ostream_t fluid_get_stdout (void);
int fluid_ostream_printf (fluid_ostream_t out, char* format, ...);



/* Profiling */


/**
 * Profile numbers. List all the pieces of code you want to profile
 * here. Be sure to add an entry in the fluid_profile_data table in
 * fluid_sys.c
 */
enum {
    FLUID_PROF_WRITE,
    FLUID_PROF_ONE_BLOCK,
    FLUID_PROF_ONE_BLOCK_CLEAR,
    FLUID_PROF_ONE_BLOCK_VOICE,
    FLUID_PROF_ONE_BLOCK_VOICES,
    FLUID_PROF_ONE_BLOCK_REVERB,
    FLUID_PROF_ONE_BLOCK_CHORUS,
    FLUID_PROF_VOICE_NOTE,
    FLUID_PROF_VOICE_RELEASE,
    FLUID_PROF_LAST
};


#if WITH_PROFILING

void fluid_profiling_print(void);


/** Profiling data. Keep track of min/avg/max values to execute a
    piece of code. */
typedef struct _fluid_profile_data_t {
    int num;
    char* description;
    double min, max, total;
    unsigned int count;
} fluid_profile_data_t;

extern fluid_profile_data_t fluid_profile_data[];

/** Macro to obtain a time refence used for the profiling */
#define fluid_profile_ref() fluid_utime()

/** Macro to create a variable and assign the current reference time for profiling.
 * So we don't get unused variable warnings when profiling is disabled. */
#define fluid_profile_ref_var(name)     double name = fluid_utime()

/** Macro to calculate the min/avg/max. Needs a time refence and a
    profile number. */
#define fluid_profile(_num,_ref) { \
  double _now = fluid_utime(); \
  double _delta = _now - _ref; \
  fluid_profile_data[_num].min = _delta < fluid_profile_data[_num].min ? _delta : fluid_profile_data[_num].min; \
  fluid_profile_data[_num].max = _delta > fluid_profile_data[_num].max ? _delta : fluid_profile_data[_num].max; \
  fluid_profile_data[_num].total += _delta; \
  fluid_profile_data[_num].count++; \
  _ref = _now; \
}


#else

/* No profiling */
#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile_ref_var(name)
#define fluid_profile(_num,_ref)

#endif



/**

    Memory locking

    Memory locking is used to avoid swapping of the large block of
    sample data.
 */

#if defined(HAVE_SYS_MMAN_H) && !defined(__OS2__)
#define fluid_mlock(_p,_n)      mlock(_p, _n)
#define fluid_munlock(_p,_n)    munlock(_p,_n)
#else
#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)
#endif


/**

    Floating point exceptions

    fluid_check_fpe() checks for "unnormalized numbers" and other
    exceptions of the floating point processsor.
*/
#ifdef FPE_CHECK
#define fluid_check_fpe(expl) fluid_check_fpe_i386(expl)
#define fluid_clear_fpe() fluid_clear_fpe_i386()
#else
#define fluid_check_fpe(expl)
#define fluid_clear_fpe()
#endif

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);
void fluid_clear_fpe_i386(void);

#endif /* _FLUID_SYS_H */
