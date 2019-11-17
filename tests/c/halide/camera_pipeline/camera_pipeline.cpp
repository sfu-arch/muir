#include <iostream>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern "C" {
int64_t halide_current_time_ns(void *ctx);
void halide_profiler_pipeline_end(void *, void *);
}

#ifdef _WIN32
__declspec(dllimport) float __cdecl roundf(float);
__declspec(dllimport) double __cdecl round(double);
#else
inline float asinh_f32(float x) {return asinhf(x);}
inline float acosh_f32(float x) {return acoshf(x);}
inline float atanh_f32(float x) {return atanhf(x);}
inline double asinh_f64(double x) {return asinh(x);}
inline double acosh_f64(double x) {return acosh(x);}
inline double atanh_f64(double x) {return atanh(x);}
#endif
inline float sqrt_f32(float x) {return sqrtf(x);}
inline float sin_f32(float x) {return sinf(x);}
inline float asin_f32(float x) {return asinf(x);}
inline float cos_f32(float x) {return cosf(x);}
inline float acos_f32(float x) {return acosf(x);}
inline float tan_f32(float x) {return tanf(x);}
inline float atan_f32(float x) {return atanf(x);}
inline float sinh_f32(float x) {return sinhf(x);}
inline float cosh_f32(float x) {return coshf(x);}
inline float tanh_f32(float x) {return tanhf(x);}
inline float hypot_f32(float x, float y) {return hypotf(x, y);}
inline float exp_f32(float x) {return expf(x);}
inline float log_f32(float x) {return logf(x);}
inline float pow_f32(float x, float y) {return powf(x, y);}
inline float floor_f32(float x) {return floorf(x);}
inline float ceil_f32(float x) {return ceilf(x);}
inline float round_f32(float x) {return roundf(x);}

inline double sqrt_f64(double x) {return sqrt(x);}
inline double sin_f64(double x) {return sin(x);}
inline double asin_f64(double x) {return asin(x);}
inline double cos_f64(double x) {return cos(x);}
inline double acos_f64(double x) {return acos(x);}
inline double tan_f64(double x) {return tan(x);}
inline double atan_f64(double x) {return atan(x);}
inline double sinh_f64(double x) {return sinh(x);}
inline double cosh_f64(double x) {return cosh(x);}
inline double tanh_f64(double x) {return tanh(x);}
inline double hypot_f64(double x, double y) {return hypot(x, y);}
inline double exp_f64(double x) {return exp(x);}
inline double log_f64(double x) {return log(x);}
inline double pow_f64(double x, double y) {return pow(x, y);}
inline double floor_f64(double x) {return floor(x);}
inline double ceil_f64(double x) {return ceil(x);}
inline double round_f64(double x) {return round(x);}

inline float nan_f32() {return NAN;}
inline float neg_inf_f32() {return -INFINITY;}
inline float inf_f32() {return INFINITY;}
inline bool is_nan_f32(float x) {return x != x;}
inline bool is_nan_f64(double x) {return x != x;}

template<typename A, typename B>
inline A reinterpret(const B &b) {
    #if __cplusplus >= 201103L
    static_assert(sizeof(A) == sizeof(B), "type size mismatch");
    #endif
    A a;
    memcpy(&a, &b, sizeof(a));
    return a;
}
inline float float_from_bits(uint32_t bits) {
    return reinterpret<float, uint32_t>(bits);
}

template<typename T>
inline uint8_t halide_count_leading_zeros(T v) {
    int bits = sizeof(v) * 8;
    while (v) {
        v >>= 1;
        bits--;
    }
    return bits;
}

template<typename T>
inline T halide_cpp_max(const T &a, const T &b) {return (a > b) ? a : b;}

template<typename T>
inline T halide_cpp_min(const T &a, const T &b) {return (a < b) ? a : b;}

template<typename A, typename B>
const B &return_second(const A &a, const B &b) {
    (void) a;
    return b;
}

template<typename A, typename B>
inline auto quiet_div(const A &a, const B &b) -> decltype(a / b) {
    return b == 0 ? static_cast<decltype(a / b)>(0) : (a / b);
}

template<typename A, typename B>
inline auto quiet_mod(const A &a, const B &b) -> decltype(a % b) {
    return b == 0 ? static_cast<decltype(a % b)>(0) : (a % b);
}

namespace {
class HalideFreeHelper {
    typedef void (*FreeFunction)(void *user_context, void *p);
    void * user_context;
    void *p;
    FreeFunction free_function;
public:
    HalideFreeHelper(void *user_context, void *p, FreeFunction free_function)
        : user_context(user_context), p(p), free_function(free_function) {}
    ~HalideFreeHelper() { free(); }
    void free() {
        if (p) {
            // TODO: do all free_functions guarantee to ignore a nullptr?
            free_function(user_context, p);
            p = nullptr;
        }
    }
};
} // namespace

#ifndef HALIDE_HALIDERUNTIME_H
#define HALIDE_HALIDERUNTIME_H

#ifndef COMPILING_HALIDE_RUNTIME
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#else
#include "runtime_internal.h"
#endif

#ifdef __cplusplus
// Forward declare type to allow naming typed handles.
// See Type.h for documentation.
template<typename T> struct halide_handle_traits;
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Note that you should not use "inline" along with HALIDE_ALWAYS_INLINE;
// it is not necessary, and may produce warnings for some build configurations.
#ifdef _MSC_VER
#define HALIDE_ALWAYS_INLINE __forceinline
#define HALIDE_NEVER_INLINE __declspec(noinline)
#else
#define HALIDE_ALWAYS_INLINE __attribute__((always_inline)) inline
#define HALIDE_NEVER_INLINE __attribute__((noinline))
#endif

/** \file
 *
 * This file declares the routines used by Halide internally in its
 * runtime. On platforms that support weak linking, these can be
 * replaced with user-defined versions by defining an extern "C"
 * function with the same name and signature.
 *
 * When doing Just In Time (JIT) compilation methods on the Func being
 * compiled must be called instead. The corresponding methods are
 * documented below.
 *
 * All of these functions take a "void *user_context" parameter as their
 * first argument; if the Halide kernel that calls back to any of these
 * functions has been compiled with the UserContext feature set on its Target,
 * then the value of that pointer passed from the code that calls the
 * Halide kernel is piped through to the function.
 *
 * Some of these are also useful to call when using the default
 * implementation. E.g. halide_shutdown_thread_pool.
 *
 * Note that even on platforms with weak linking, some linker setups
 * may not respect the override you provide. E.g. if the override is
 * in a shared library and the halide object files are linked directly
 * into the output, the builtin versions of the runtime functions will
 * be called. See your linker documentation for more details. On
 * Linux, LD_DYNAMIC_WEAK=1 may help.
 *
 */

// Forward-declare to suppress warnings if compiling as C.
struct halide_buffer_t;
struct buffer_t;

/** Print a message to stderr. Main use is to support tracing
 * functionality, print, and print_when calls. Also called by the default
 * halide_error.  This function can be replaced in JITed code by using
 * halide_custom_print and providing an implementation of halide_print
 * in AOT code. See Func::set_custom_print.
 */
// @{
extern void halide_print(void *user_context, const char *);
extern void halide_default_print(void *user_context, const char *);
typedef void (*halide_print_t)(void *, const char *);
extern halide_print_t halide_set_custom_print(halide_print_t print);
// @}

/** Halide calls this function on runtime errors (for example bounds
 * checking failures). This function can be replaced in JITed code by
 * using Func::set_error_handler, or in AOT code by calling
 * halide_set_error_handler. In AOT code on platforms that support
 * weak linking (i.e. not Windows), you can also override it by simply
 * defining your own halide_error.
 */
// @{
extern void halide_error(void *user_context, const char *);
extern void halide_default_error(void *user_context, const char *);
typedef void (*halide_error_handler_t)(void *, const char *);
extern halide_error_handler_t halide_set_error_handler(halide_error_handler_t handler);
// @}

/** Cross-platform mutex. Must be initialized with zero and implementation
 * must treat zero as an unlocked mutex with no waiters, etc.
 */
struct halide_mutex {
    uintptr_t _private[1];
};

/** Cross platform condition variable. Must be initialized to 0. */
struct halide_cond {
    uintptr_t _private[1];
};

/** A basic set of mutex and condition variable functions, which call
 * platform specific code for mutual exclusion. Equivalent to posix
 * calls. */
//@{
extern void halide_mutex_lock(struct halide_mutex *mutex);
extern void halide_mutex_unlock(struct halide_mutex *mutex);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_broadcast(struct halide_cond *cond);
extern void halide_cond_signal(struct halide_cond *cond);
extern void halide_cond_wait(struct halide_cond *cond, struct halide_mutex *mutex);
//@}

/** Define halide_do_par_for to replace the default thread pool
 * implementation. halide_shutdown_thread_pool can also be called to
 * release resources used by the default thread pool on platforms
 * where it makes sense. (E.g. On Mac OS, Grand Central Dispatch is
 * used so %Halide does not own the threads backing the pool and they
 * cannot be released.)  See Func::set_custom_do_task and
 * Func::set_custom_do_par_for. Should return zero if all the jobs
 * return zero, or an arbitrarily chosen return value from one of the
 * jobs otherwise.
 */
//@{
typedef int (*halide_task_t)(void *user_context, int task_number, uint8_t *closure);
extern int halide_do_par_for(void *user_context,
                             halide_task_t task,
                             int min, int size, uint8_t *closure);
extern void halide_shutdown_thread_pool();
//@}

/** Set a custom method for performing a parallel for loop. Returns
 * the old do_par_for handler. */
typedef int (*halide_do_par_for_t)(void *, halide_task_t, int, int, uint8_t*);
extern halide_do_par_for_t halide_set_custom_do_par_for(halide_do_par_for_t do_par_for);

/** An opaque struct representing a semaphore. Used by the task system for async tasks. */
struct halide_semaphore_t {
    uint64_t _private[2];
};

/** A struct representing a semaphore and a number of items that must
 * be acquired from it. Used in halide_parallel_task_t below. */
struct halide_semaphore_acquire_t {
    struct halide_semaphore_t *semaphore;
    int count;
};
extern int halide_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_semaphore_try_acquire(struct halide_semaphore_t *, int n);
typedef int (*halide_semaphore_init_t)(struct halide_semaphore_t *, int);
typedef int (*halide_semaphore_release_t)(struct halide_semaphore_t *, int);
typedef bool (*halide_semaphore_try_acquire_t)(struct halide_semaphore_t *, int);


/** A task representing a serial for loop evaluated over some range.
 * Note that task_parent is a pass through argument that should be
 * passed to any dependent taks that are invokved using halide_do_parallel_tasks
 * underneath this call. */
typedef int (*halide_loop_task_t)(void *user_context, int min, int extent,
                                  uint8_t *closure, void *task_parent);

/** A parallel task to be passed to halide_do_parallel_tasks. This
 * task may recursively call halide_do_parallel_tasks, and there may
 * be complex dependencies between seemingly unrelated tasks expressed
 * using semaphores. If you are using a custom task system, care must
 * be taken to avoid potential deadlock. This can be done by carefully
 * respecting the static metadata at the end of the task struct.*/
struct halide_parallel_task_t {
    // The function to call. It takes a user context, a min and
    // extent, a closure, and a task system pass through argument.
    halide_loop_task_t fn;

    // The closure to pass it
    uint8_t *closure;

    // The name of the function to be called. For debugging purposes only.
    const char *name;

    // An array of semaphores that must be acquired before the
    // function is called. Must be reacquired for every call made.
    struct halide_semaphore_acquire_t *semaphores;
    int num_semaphores;

    // The entire range the function should be called over. This range
    // may be sliced up and the function called multiple times.
    int min, extent;

    // A parallel task provides several pieces of metadata to prevent
    // unbounded resource usage or deadlock.

    // The first is the minimum number of execution contexts (call
    // stacks or threads) necessary for the function to run to
    // completion. This may be greater than one when there is nested
    // parallelism with internal producer-consumer relationships
    // (calling the function recursively spawns and blocks on parallel
    // sub-tasks that communicate with each other via semaphores). If
    // a parallel runtime calls the function when fewer than this many
    // threads are idle, it may need to create more threads to
    // complete the task, or else risk deadlock due to committing all
    // threads to tasks that cannot complete without more.
    //
    // FIXME: Note that extern stages are assumed to only require a
    // single thread to complete. If the extern stage is itself a
    // Halide pipeline, this may be an underestimate.
    int min_threads;

    // The calls to the function should be in serial order from min to min+extent-1, with only
    // one executing at a time. If false, any order is fine, and
    // concurrency is fine.
    bool serial;
};

/** Enqueue some number of the tasks described above and wait for them
 * to complete. While waiting, the calling threads assists with either
 * the tasks enqueued, or other non-blocking tasks in the task
 * system. Note that task_parent should be NULL for top-level calls
 * and the pass through argument if this call is being made from
 * another task. */
extern int halide_do_parallel_tasks(void *user_context, int num_tasks,
                                    struct halide_parallel_task_t *tasks,
                                    void *task_parent);

/** If you use the default do_par_for, you can still set a custom
 * handler to perform each individual task. Returns the old handler. */
//@{
typedef int (*halide_do_task_t)(void *, halide_task_t, int, uint8_t *);
extern halide_do_task_t halide_set_custom_do_task(halide_do_task_t do_task);
extern int halide_do_task(void *user_context, halide_task_t f, int idx,
                          uint8_t *closure);
//@}

/** The version of do_task called for loop tasks. By default calls the
 * loop task with the same arguments. */
// @{
  typedef int (*halide_do_loop_task_t)(void *, halide_loop_task_t, int, int, uint8_t *, void *);
extern halide_do_loop_task_t halide_set_custom_do_loop_task(halide_do_loop_task_t do_task);
extern int halide_do_loop_task(void *user_context, halide_loop_task_t f, int min, int extent,
                               uint8_t *closure, void *task_parent);
//@}

/** Provide an entire custom tasking runtime via function
 * pointers. Note that do_task and semaphore_try_acquire are only ever
 * called by halide_default_do_par_for and
 * halide_default_do_parallel_tasks, so it's only necessary to provide
 * those if you are mixing in the default implementations of
 * do_par_for and do_parallel_tasks. */
// @{
typedef int (*halide_do_parallel_tasks_t)(void *, int, struct halide_parallel_task_t *,
                                          void *task_parent);
extern void halide_set_custom_parallel_runtime(
    halide_do_par_for_t,
    halide_do_task_t,
    halide_do_loop_task_t,
    halide_do_parallel_tasks_t,
    halide_semaphore_init_t,
    halide_semaphore_try_acquire_t,
    halide_semaphore_release_t
    );
// @}

/** The default versions of the parallel runtime functions. */
// @{
extern int halide_default_do_par_for(void *user_context,
                                     halide_task_t task,
                                     int min, int size, uint8_t *closure);
extern int halide_default_do_parallel_tasks(void *user_context,
                                            int num_tasks,
                                            struct halide_parallel_task_t *tasks,
                                            void *task_parent);
extern int halide_default_do_task(void *user_context, halide_task_t f, int idx,
                                  uint8_t *closure);
extern int halide_default_do_loop_task(void *user_context, halide_loop_task_t f,
                                       int min, int extent,
                                       uint8_t *closure, void *task_parent);
extern int halide_default_semaphore_init(struct halide_semaphore_t *, int n);
extern int halide_default_semaphore_release(struct halide_semaphore_t *, int n);
extern bool halide_default_semaphore_try_acquire(struct halide_semaphore_t *, int n);
// @}

struct halide_thread;

/** Spawn a thread. Returns a handle to the thread for the purposes of
 * joining it. The thread must be joined in order to clean up any
 * resources associated with it. */
extern struct halide_thread *halide_spawn_thread(void (*f)(void *), void *closure);

/** Join a thread. */
extern void halide_join_thread(struct halide_thread *);

/** Set the number of threads used by Halide's thread pool. Returns
 * the old number.
 *
 * n < 0  : error condition
 * n == 0 : use a reasonable system default (typically, number of cpus online).
 * n == 1 : use exactly one thread; this will always enforce serial execution
 * n > 1  : use a pool of exactly n threads.
 *
 * Note that the default iOS and OSX behavior will treat n > 1 like n == 0;
 * that is, any positive value other than 1 will use a system-determined number
 * of threads.
 *
 * (Note that this is only guaranteed when using the default implementations
 * of halide_do_par_for(); custom implementations may completely ignore values
 * passed to halide_set_num_threads().)
 */
extern int halide_set_num_threads(int n);

/** Halide calls these functions to allocate and free memory. To
 * replace in AOT code, use the halide_set_custom_malloc and
 * halide_set_custom_free, or (on platforms that support weak
 * linking), simply define these functions yourself. In JIT-compiled
 * code use Func::set_custom_allocator.
 *
 * If you override them, and find yourself wanting to call the default
 * implementation from within your override, use
 * halide_default_malloc/free.
 *
 * Note that halide_malloc must return a pointer aligned to the
 * maximum meaningful alignment for the platform for the purpose of
 * vector loads and stores. The default implementation uses 32-byte
 * alignment, which is safe for arm and x86. Additionally, it must be
 * safe to read at least 8 bytes before the start and beyond the
 * end.
 */
//@{
extern void *halide_malloc(void *user_context, size_t x);
extern void halide_free(void *user_context, void *ptr);
extern void *halide_default_malloc(void *user_context, size_t x);
extern void halide_default_free(void *user_context, void *ptr);
typedef void *(*halide_malloc_t)(void *, size_t);
typedef void (*halide_free_t)(void *, void *);
extern halide_malloc_t halide_set_custom_malloc(halide_malloc_t user_malloc);
extern halide_free_t halide_set_custom_free(halide_free_t user_free);
//@}

/** Halide calls these functions to interact with the underlying
 * system runtime functions. To replace in AOT code on platforms that
 * support weak linking, define these functions yourself, or use
 * the halide_set_custom_load_library() and halide_set_custom_get_library_symbol()
 * functions. In JIT-compiled code, use JITSharedRuntime::set_default_handlers().
 *
 * halide_load_library and halide_get_library_symbol are equivalent to
 * dlopen and dlsym. halide_get_symbol(sym) is equivalent to
 * dlsym(RTLD_DEFAULT, sym).
 */
//@{
extern void *halide_get_symbol(const char *name);
extern void *halide_load_library(const char *name);
extern void *halide_get_library_symbol(void *lib, const char *name);
extern void *halide_default_get_symbol(const char *name);
extern void *halide_default_load_library(const char *name);
extern void *halide_default_get_library_symbol(void *lib, const char *name);
typedef void *(*halide_get_symbol_t)(const char *name);
typedef void *(*halide_load_library_t)(const char *name);
typedef void *(*halide_get_library_symbol_t)(void *lib, const char *name);
extern halide_get_symbol_t halide_set_custom_get_symbol(halide_get_symbol_t user_get_symbol);
extern halide_load_library_t halide_set_custom_load_library(halide_load_library_t user_load_library);
extern halide_get_library_symbol_t halide_set_custom_get_library_symbol(halide_get_library_symbol_t user_get_library_symbol);
//@}

/** Called when debug_to_file is used inside %Halide code.  See
 * Func::debug_to_file for how this is called
 *
 * Cannot be replaced in JITted code at present.
 */
extern int32_t halide_debug_to_file(void *user_context, const char *filename,
                                    int32_t type_code,
                                    struct halide_buffer_t *buf);

/** Types in the halide type system. They can be ints, unsigned ints,
 * or floats (of various bit-widths), or a handle (which is always 64-bits).
 * Note that the int/uint/float values do not imply a specific bit width
 * (the bit width is expected to be encoded in a separate value).
 */
typedef enum halide_type_code_t
#if __cplusplus >= 201103L
: uint8_t
#endif
{
    halide_type_int = 0,   //!< signed integers
    halide_type_uint = 1,  //!< unsigned integers
    halide_type_float = 2, //!< floating point numbers
    halide_type_handle = 3 //!< opaque pointer type (void *)
} halide_type_code_t;

// Note that while __attribute__ can go before or after the declaration,
// __declspec apparently is only allowed before.
#ifndef HALIDE_ATTRIBUTE_ALIGN
    #ifdef _MSC_VER
        #define HALIDE_ATTRIBUTE_ALIGN(x) __declspec(align(x))
    #else
        #define HALIDE_ATTRIBUTE_ALIGN(x) __attribute__((aligned(x)))
    #endif
#endif

/** A runtime tag for a type in the halide type system. Can be ints,
 * unsigned ints, or floats of various bit-widths (the 'bits'
 * field). Can also be vectors of the same (by setting the 'lanes'
 * field to something larger than one). This struct should be
 * exactly 32-bits in size. */
struct halide_type_t {
    /** The basic type code: signed integer, unsigned integer, or floating point. */
#if __cplusplus >= 201103L
    HALIDE_ATTRIBUTE_ALIGN(1) halide_type_code_t code; // halide_type_code_t
#else
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t code; // halide_type_code_t
#endif

    /** The number of bits of precision of a single scalar value of this type. */
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t bits;

    /** How many elements in a vector. This is 1 for scalar types. */
    HALIDE_ATTRIBUTE_ALIGN(2) uint16_t lanes;

#ifdef __cplusplus
    /** Construct a runtime representation of a Halide type from:
     * code: The fundamental type from an enum.
     * bits: The bit size of one element.
     * lanes: The number of vector elements in the type. */
    HALIDE_ALWAYS_INLINE halide_type_t(halide_type_code_t code, uint8_t bits, uint16_t lanes = 1)
        : code(code), bits(bits), lanes(lanes) {
    }

    /** Default constructor is required e.g. to declare halide_trace_event
     * instances. */
    HALIDE_ALWAYS_INLINE halide_type_t() : code((halide_type_code_t)0), bits(0), lanes(0) {}

    /** Compare two types for equality. */
    HALIDE_ALWAYS_INLINE bool operator==(const halide_type_t &other) const {
        return as_u32() == other.as_u32();
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_type_t &other) const {
        return !(*this == other);
    }

    HALIDE_ALWAYS_INLINE bool operator<(const halide_type_t &other) const {
        return as_u32() < other.as_u32();
    }

    /** Size in bytes for a single element, even if width is not 1, of this type. */
    HALIDE_ALWAYS_INLINE int bytes() const { return (bits + 7) / 8; }

private:
    HALIDE_ALWAYS_INLINE uint32_t as_u32() const {
        uint32_t u;
        memcpy(&u, this, sizeof(u));
        return u;
    }
#endif
};

enum halide_trace_event_code_t {halide_trace_load = 0,
                                halide_trace_store = 1,
                                halide_trace_begin_realization = 2,
                                halide_trace_end_realization = 3,
                                halide_trace_produce = 4,
                                halide_trace_end_produce = 5,
                                halide_trace_consume = 6,
                                halide_trace_end_consume = 7,
                                halide_trace_begin_pipeline = 8,
                                halide_trace_end_pipeline = 9,
                                halide_trace_tag = 10 };

struct halide_trace_event_t {
    /** The name of the Func or Pipeline that this event refers to */
    const char *func;

    /** If the event type is a load or a store, this points to the
     * value being loaded or stored. Use the type field to safely cast
     * this to a concrete pointer type and retrieve it. For other
     * events this is null. */
    void *value;

    /** For loads and stores, an array which contains the location
     * being accessed. For vector loads or stores it is an array of
     * vectors of coordinates (the vector dimension is innermost).
     *
     * For realization or production-related events, this will contain
     * the mins and extents of the region being accessed, in the order
     * min0, extent0, min1, extent1, ...
     *
     * For pipeline-related events, this will be null.
     */
    int32_t *coordinates;

    /** For halide_trace_tag, this points to a read-only null-terminated string
     * of arbitrary text. For all other events, this will be null.
     */
    const char *trace_tag;

    /** If the event type is a load or a store, this is the type of
     * the data. Otherwise, the value is meaningless. */
    struct halide_type_t type;

    /** The type of event */
    enum halide_trace_event_code_t event;

    /* The ID of the parent event (see below for an explanation of
     * event ancestry). */
    int32_t parent_id;

    /** If this was a load or store of a Tuple-valued Func, this is
     * which tuple element was accessed. */
    int32_t value_index;

    /** The length of the coordinates array */
    int32_t dimensions;

#ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_event_t() {}
#endif
};

/** Called when Funcs are marked as trace_load, trace_store, or
 * trace_realization. See Func::set_custom_trace. The default
 * implementation either prints events via halide_print, or if
 * HL_TRACE_FILE is defined, dumps the trace to that file in a
 * sequence of trace packets. The header for a trace packet is defined
 * below. If the trace is going to be large, you may want to make the
 * file a named pipe, and then read from that pipe into gzip.
 *
 * halide_trace returns a unique ID which will be passed to future
 * events that "belong" to the earlier event as the parent id. The
 * ownership hierarchy looks like:
 *
 * begin_pipeline
 * +--trace_tag (if any)
 * +--trace_tag (if any)
 * ...
 * +--begin_realization
 * |  +--produce
 * |  |  +--load/store
 * |  |  +--end_produce
 * |  +--consume
 * |  |  +--load
 * |  |  +--end_consume
 * |  +--end_realization
 * +--end_pipeline
 *
 * Threading means that ownership cannot be inferred from the ordering
 * of events. There can be many active realizations of a given
 * function, or many active productions for a single
 * realization. Within a single production, the ordering of events is
 * meaningful.
 *
 * Note that all trace_tag events (if any) will occur just after the begin_pipeline
 * event, but before any begin_realization events. All trace_tags for a given Func
 * will be emitted in the order added.
 */
// @}
extern int32_t halide_trace(void *user_context, const struct halide_trace_event_t *event);
extern int32_t halide_default_trace(void *user_context, const struct halide_trace_event_t *event);
typedef int32_t (*halide_trace_t)(void *user_context, const struct halide_trace_event_t *);
extern halide_trace_t halide_set_custom_trace(halide_trace_t trace);
// @}

/** The header of a packet in a binary trace. All fields are 32-bit. */
struct halide_trace_packet_t {
    /** The total size of this packet in bytes. Always a multiple of
     * four. Equivalently, the number of bytes until the next
     * packet. */
    uint32_t size;

    /** The id of this packet (for the purpose of parent_id). */
    int32_t id;

    /** The remaining fields are equivalent to those in halide_trace_event_t */
    // @{
    struct halide_type_t type;
    enum halide_trace_event_code_t event;
    int32_t parent_id;
    int32_t value_index;
    int32_t dimensions;
    // @}

    #ifdef __cplusplus
    // If we don't explicitly mark the default ctor as inline,
    // certain build configurations can fail (notably iOS)
    HALIDE_ALWAYS_INLINE halide_trace_packet_t() {}

    /** Get the coordinates array, assuming this packet is laid out in
     * memory as it was written. The coordinates array comes
     * immediately after the packet header. */
    HALIDE_ALWAYS_INLINE const int *coordinates() const {
        return (const int *)(this + 1);
    }

    HALIDE_ALWAYS_INLINE int *coordinates() {
        return (int *)(this + 1);
    }

    /** Get the value, assuming this packet is laid out in memory as
     * it was written. The packet comes immediately after the coordinates
     * array. */
    HALIDE_ALWAYS_INLINE const void *value() const {
        return (const void *)(coordinates() + dimensions);
    }

    HALIDE_ALWAYS_INLINE void *value() {
        return (void *)(coordinates() + dimensions);
    }

    /** Get the func name, assuming this packet is laid out in memory
     * as it was written. It comes after the value. */
    HALIDE_ALWAYS_INLINE const char *func() const {
        return (const char *)value() + type.lanes * type.bytes();
    }

    HALIDE_ALWAYS_INLINE char *func() {
        return (char *)value() + type.lanes * type.bytes();
    }

    /** Get the trace_tag (if any), assuming this packet is laid out in memory
     * as it was written. It comes after the func name. If there is no trace_tag,
     * this will return a pointer to an empty string. */
    HALIDE_ALWAYS_INLINE const char *trace_tag() const {
        const char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }

    HALIDE_ALWAYS_INLINE char *trace_tag() {
        char *f = func();
        // strlen may not be available here
        while (*f++) {
            // nothing
        }
        return f;
    }
    #endif
};



/** Set the file descriptor that Halide should write binary trace
 * events to. If called with 0 as the argument, Halide outputs trace
 * information to stdout in a human-readable format. If never called,
 * Halide checks the for existence of an environment variable called
 * HL_TRACE_FILE and opens that file. If HL_TRACE_FILE is not defined,
 * it outputs trace information to stdout in a human-readable
 * format. */
extern void halide_set_trace_file(int fd);

/** Halide calls this to retrieve the file descriptor to write binary
 * trace events to. The default implementation returns the value set
 * by halide_set_trace_file. Implement it yourself if you wish to use
 * a custom file descriptor per user_context. Return zero from your
 * implementation to tell Halide to print human-readable trace
 * information to stdout. */
extern int halide_get_trace_file(void *user_context);

/** If tracing is writing to a file. This call closes that file
 * (flushing the trace). Returns zero on success. */
extern int halide_shutdown_trace();

/** All Halide GPU or device backend implementations provide an
 * interface to be used with halide_device_malloc, etc. This is
 * accessed via the functions below.
 */

/** An opaque struct containing per-GPU API implementations of the
 * device functions. */
struct halide_device_interface_impl_t;

/** Each GPU API provides a halide_device_interface_t struct pointing
 * to the code that manages device allocations. You can access these
 * functions directly from the struct member function pointers, or by
 * calling the functions declared below. Note that the global
 * functions are not available when using Halide as a JIT compiler.
 * If you are using raw halide_buffer_t in that context you must use
 * the function pointers in the device_interface struct.
 *
 * The function pointers below are currently the same for every GPU
 * API; only the impl field varies. These top-level functions do the
 * bookkeeping that is common across all GPU APIs, and then dispatch
 * to more API-specific functions via another set of function pointers
 * hidden inside the impl field.
 */
struct halide_device_interface_t {
    int (*device_malloc)(void *user_context, struct halide_buffer_t *buf,
                         const struct halide_device_interface_t *device_interface);
    int (*device_free)(void *user_context, struct halide_buffer_t *buf);
    int (*device_sync)(void *user_context, struct halide_buffer_t *buf);
    void (*device_release)(void *user_context,
                          const struct halide_device_interface_t *device_interface);
    int (*copy_to_host)(void *user_context, struct halide_buffer_t *buf);
    int (*copy_to_device)(void *user_context, struct halide_buffer_t *buf,
                          const struct halide_device_interface_t *device_interface);
    int (*device_and_host_malloc)(void *user_context, struct halide_buffer_t *buf,
                                  const struct halide_device_interface_t *device_interface);
    int (*device_and_host_free)(void *user_context, struct halide_buffer_t *buf);
    int (*buffer_copy)(void *user_context, struct halide_buffer_t *src,
                       const struct halide_device_interface_t *dst_device_interface, struct halide_buffer_t *dst);
    int (*device_crop)(void *user_context, const struct halide_buffer_t *src,
                       struct halide_buffer_t *dst);
    int (*device_slice)(void *user_context, const struct halide_buffer_t *src,
                        int slice_dim, int slice_pos, struct halide_buffer_t *dst);
    int (*device_release_crop)(void *user_context, struct halide_buffer_t *buf);
    int (*wrap_native)(void *user_context, struct halide_buffer_t *buf, uint64_t handle,
                       const struct halide_device_interface_t *device_interface);
    int (*detach_native)(void *user_context, struct halide_buffer_t *buf);
    int (*compute_capability)(void *user_context, int *major, int *minor);
    const struct halide_device_interface_impl_t *impl;
};

/** Release all data associated with the given device interface, in
 * particular all resources (memory, texture, context handles)
 * allocated by Halide. Must be called explicitly when using AOT
 * compilation. This is *not* thread-safe with respect to actively
 * running Halide code. Ensure all pipelines are finished before
 * calling this. */
extern void halide_device_release(void *user_context,
                                  const struct halide_device_interface_t *device_interface);

/** Copy image data from device memory to host memory. This must be called
 * explicitly to copy back the results of a GPU-based filter. */
extern int halide_copy_to_host(void *user_context, struct halide_buffer_t *buf);

/** Copy image data from host memory to device memory. This should not
 * be called directly; Halide handles copying to the device
 * automatically.  If interface is NULL and the buf has a non-zero dev
 * field, the device associated with the dev handle will be
 * used. Otherwise if the dev field is 0 and interface is NULL, an
 * error is returned. */
extern int halide_copy_to_device(void *user_context, struct halide_buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);

/** Copy data from one buffer to another. The buffers may have
 * different shapes and sizes, but the destination buffer's shape must
 * be contained within the source buffer's shape. That is, for each
 * dimension, the min on the destination buffer must be greater than
 * or equal to the min on the source buffer, and min+extent on the
 * destination buffer must be less that or equal to min+extent on the
 * source buffer. The source data is pulled from either device or
 * host memory on the source, depending on the dirty flags. host is
 * preferred if both are valid. The dst_device_interface parameter
 * controls the destination memory space. NULL means host memory. */
extern int halide_buffer_copy(void *user_context, struct halide_buffer_t *src,
                              const struct halide_device_interface_t *dst_device_interface,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for the same coordinate range in the source buffer. Modifies the
 * device, device_interface, and the device_dirty flag only. Only
 * supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the cropped view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that src
 * and dst are required to have the same number of dimensions.
 *
 * Note also that (in theory) device interfaces which support cropping may
 * still not support cropping a crop (instead, create a new crop of the parent
 * buffer); in practice, no known implementation has this limitation, although
 * it is possible that some future implementations may require it. */
extern int halide_device_crop(void *user_context,
                              const struct halide_buffer_t *src,
                              struct halide_buffer_t *dst);

/** Give the destination buffer a device allocation which is an alias
 * for a similar coordinate range in the source buffer, but with one dimension
 * sliced away in the dst. Modifies the device, device_interface, and the
 * device_dirty flag only. Only supported by some device APIs (others will return
 * halide_error_code_device_crop_unsupported). Call
 * halide_device_release_crop instead of halide_device_free to clean
 * up resources associated with the sliced view. Do not free the
 * device allocation on the source buffer while the destination buffer
 * still lives. Note that the two buffers do not share dirty flags, so
 * care must be taken to update them together as needed. Note that the dst buffer
 * must have exactly one fewer dimension than the src buffer, and that slice_dim
 * and slice_pos must be valid within src. */
extern int halide_device_slice(void *user_context,
                               const struct halide_buffer_t *src,
                               int slice_dim, int slice_pos,
                               struct halide_buffer_t *dst);

/** Release any resources associated with a cropped/sliced view of another
 * buffer. */
extern int halide_device_release_crop(void *user_context,
                                      struct halide_buffer_t *buf);

/** Wait for current GPU operations to complete. Calling this explicitly
 * should rarely be necessary, except maybe for profiling. */
extern int halide_device_sync(void *user_context, struct halide_buffer_t *buf);

/** Allocate device memory to back a halide_buffer_t. */
extern int halide_device_malloc(void *user_context, struct halide_buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);

/** Free device memory. */
extern int halide_device_free(void *user_context, struct halide_buffer_t *buf);

/** Wrap or detach a native device handle, setting the device field
 * and device_interface field as appropriate for the given GPU
 * API. The meaning of the opaque handle is specific to the device
 * interface, so if you know the device interface in use, call the
 * more specific functions in the runtime headers for your specific
 * device API instead (e.g. HalideRuntimeCuda.h). */
// @{
extern int halide_device_wrap_native(void *user_context,
                                     struct halide_buffer_t *buf,
                                     uint64_t handle,
                                     const struct halide_device_interface_t *device_interface);
extern int halide_device_detach_native(void *user_context, struct halide_buffer_t *buf);
// @}

/** Versions of the above functions that accept legacy buffer_t structs. */
// @{
extern int halide_copy_to_host_legacy(void *user_context, struct buffer_t *buf);
extern int halide_copy_to_device_legacy(void *user_context, struct buffer_t *buf,
                                 const struct halide_device_interface_t *device_interface);
extern int halide_device_sync_legacy(void *user_context, struct buffer_t *buf);
extern int halide_device_malloc_legacy(void *user_context, struct buffer_t *buf,
                                const struct halide_device_interface_t *device_interface);
extern int halide_device_free_legacy(void *user_context, struct buffer_t *buf);
// @}

/** Selects which gpu device to use. 0 is usually the display
 * device. If never called, Halide uses the environment variable
 * HL_GPU_DEVICE. If that variable is unset, Halide uses the last
 * device. Set this to -1 to use the last device. */
extern void halide_set_gpu_device(int n);

/** Halide calls this to get the desired halide gpu device
 * setting. Implement this yourself to use a different gpu device per
 * user_context. The default implementation returns the value set by
 * halide_set_gpu_device, or the environment variable
 * HL_GPU_DEVICE. */
extern int halide_get_gpu_device(void *user_context);

/** Set the soft maximum amount of memory, in bytes, that the LRU
 *  cache will use to memoize Func results.  This is not a strict
 *  maximum in that concurrency and simultaneous use of memoized
 *  reults larger than the cache size can both cause it to
 *  temporariliy be larger than the size specified here.
 */
extern void halide_memoization_cache_set_size(int64_t size);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, determine if the result is in the cache and
 *  return it if so. (The internals of the cache key should be
 *  considered opaque by this function.) If this routine returns true,
 *  it is a cache miss. Otherwise, it will return false and the
 *  buffers passed in will be filled, via copying, with memoized
 *  data. The last argument is a list if halide_buffer_t pointers which
 *  represents the outputs of the memoized Func. If the Func does not
 *  return a Tuple, there will only be one halide_buffer_t in the list. The
 *  tuple_count parameters determines the length of the list.
 *
 * The return values are:
 * -1: Signals an error.
 *  0: Success and cache hit.
 *  1: Success and cache miss.
 */
extern int halide_memoization_cache_lookup(void *user_context, const uint8_t *cache_key, int32_t size,
                                           struct halide_buffer_t *realized_bounds,
                                           int32_t tuple_count, struct halide_buffer_t **tuple_buffers);

/** Given a cache key for a memoized result, currently constructed
 *  from the Func name and top-level Func name plus the arguments of
 *  the computation, store the result in the cache for futre access by
 *  halide_memoization_cache_lookup. (The internals of the cache key
 *  should be considered opaque by this function.) Data is copied out
 *  from the inputs and inputs are unmodified. The last argument is a
 *  list if halide_buffer_t pointers which represents the outputs of the
 *  memoized Func. If the Func does not return a Tuple, there will
 *  only be one halide_buffer_t in the list. The tuple_count parameters
 *  determines the length of the list.
 *
 * If there is a memory allocation failure, the store does not store
 * the data into the cache.
 */
extern int halide_memoization_cache_store(void *user_context, const uint8_t *cache_key, int32_t size,
                                          struct halide_buffer_t *realized_bounds,
                                          int32_t tuple_count,
                                          struct halide_buffer_t **tuple_buffers);

/** If halide_memoization_cache_lookup succeeds,
 * halide_memoization_cache_release must be called to signal the
 * storage is no longer being used by the caller. It will be passed
 * the host pointer of one the buffers returned by
 * halide_memoization_cache_lookup. That is
 * halide_memoization_cache_release will be called multiple times for
 * the case where halide_memoization_cache_lookup is handling multiple
 * buffers.  (This corresponds to memoizing a Tuple in Halide.) Note
 * that the host pointer must be sufficient to get to all information
 * the relase operation needs. The default Halide cache impleemntation
 * accomplishes this by storing extra data before the start of the user
 * modifiable host storage.
 *
 * This call is like free and does not have a failure return.
  */
extern void halide_memoization_cache_release(void *user_context, void *host);

/** Free all memory and resources associated with the memoization cache.
 * Must be called at a time when no other threads are accessing the cache.
 */
extern void halide_memoization_cache_cleanup();

/** Create a unique file with a name of the form prefixXXXXXsuffix in an arbitrary
 * (but writable) directory; this is typically $TMP or /tmp, but the specific
 * location is not guaranteed. (Note that the exact form of the file name
 * may vary; in particular, the suffix may be ignored on non-Posix systems.)
 * The file is created (but not opened), thus this can be called from
 * different threads (or processes, e.g. when building with parallel make)
 * without risking collision. Note that the caller is always responsible
 * for deleting this file. Returns nonzero value if an error occurs.
 */
extern int halide_create_temp_file(void *user_context,
  const char *prefix, const char *suffix,
  char *path_buf, size_t path_buf_size);

/** Annotate that a given range of memory has been initialized;
 * only used when Target::MSAN is enabled.
 *
 * The default implementation uses the LLVM-provided AnnotateMemoryIsInitialized() function.
 */
extern void halide_msan_annotate_memory_is_initialized(void *user_context, const void *ptr, uint64_t len);

/** Mark the data pointed to by the buffer_t as initialized (but *not* the buffer_t itself),
 * using halide_msan_annotate_memory_is_initialized() for marking.
 *
 * The default implementation takes pains to only mark the active memory ranges
 * (skipping padding), and sorting into ranges to always mark the smallest number of
 * ranges, in monotonically increasing memory order.
 *
 * Most client code should never need to replace the default implementation.
 */
extern void halide_msan_annotate_buffer_is_initialized(void *user_context, struct halide_buffer_t *buffer);
extern void halide_msan_annotate_buffer_is_initialized_as_destructor(void *user_context, void *buffer);

/** The error codes that may be returned by a Halide pipeline. */
enum halide_error_code_t {
    /** There was no error. This is the value returned by Halide on success. */
    halide_error_code_success = 0,

    /** An uncategorized error occurred. Refer to the string passed to halide_error. */
    halide_error_code_generic_error = -1,

    /** A Func was given an explicit bound via Func::bound, but this
     * was not large enough to encompass the region that is used of
     * the Func by the rest of the pipeline. */
    halide_error_code_explicit_bounds_too_small = -2,

    /** The elem_size field of a halide_buffer_t does not match the size in
     * bytes of the type of that ImageParam. Probable type mismatch. */
    halide_error_code_bad_type = -3,

    /** A pipeline would access memory outside of the halide_buffer_t passed
     * in. */
    halide_error_code_access_out_of_bounds = -4,

    /** A halide_buffer_t was given that spans more than 2GB of memory. */
    halide_error_code_buffer_allocation_too_large = -5,

    /** A halide_buffer_t was given with extents that multiply to a number
     * greater than 2^31-1 */
    halide_error_code_buffer_extents_too_large = -6,

    /** Applying explicit constraints on the size of an input or
     * output buffer shrank the size of that buffer below what will be
     * accessed by the pipeline. */
    halide_error_code_constraints_make_required_region_smaller = -7,

    /** A constraint on a size or stride of an input or output buffer
     * was not met by the halide_buffer_t passed in. */
    halide_error_code_constraint_violated = -8,

    /** A scalar parameter passed in was smaller than its minimum
     * declared value. */
    halide_error_code_param_too_small = -9,

    /** A scalar parameter passed in was greater than its minimum
     * declared value. */
    halide_error_code_param_too_large = -10,

    /** A call to halide_malloc returned NULL. */
    halide_error_code_out_of_memory = -11,

    /** A halide_buffer_t pointer passed in was NULL. */
    halide_error_code_buffer_argument_is_null = -12,

    /** debug_to_file failed to open or write to the specified
     * file. */
    halide_error_code_debug_to_file_failed = -13,

    /** The Halide runtime encountered an error while trying to copy
     * from device to host. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_host_failed = -14,

    /** The Halide runtime encountered an error while trying to copy
     * from host to device. Turn on -debug in your target string to
     * see more details. */
    halide_error_code_copy_to_device_failed = -15,

    /** The Halide runtime encountered an error while trying to
     * allocate memory on device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_malloc_failed = -16,

    /** The Halide runtime encountered an error while trying to
     * synchronize with a device. Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_sync_failed = -17,

    /** The Halide runtime encountered an error while trying to free a
     * device allocation. Turn on -debug in your target string to see
     * more details. */
    halide_error_code_device_free_failed = -18,

    /** Buffer has a non-zero device but no device interface, which
     * violates a Halide invariant. */
    halide_error_code_no_device_interface = -19,

    /** An error occurred when attempting to initialize the Matlab
     * runtime. */
    halide_error_code_matlab_init_failed = -20,

    /** The type of an mxArray did not match the expected type. */
    halide_error_code_matlab_bad_param_type = -21,

    /** There is a bug in the Halide compiler. */
    halide_error_code_internal_error = -22,

    /** The Halide runtime encountered an error while trying to launch
     * a GPU kernel. Turn on -debug in your target string to see more
     * details. */
    halide_error_code_device_run_failed = -23,

    /** The Halide runtime encountered a host pointer that violated
     * the alignment set for it by way of a call to
     * set_host_alignment */
    halide_error_code_unaligned_host_ptr = -24,

    /** A fold_storage directive was used on a dimension that is not
     * accessed in a monotonically increasing or decreasing fashion. */
    halide_error_code_bad_fold = -25,

    /** A fold_storage directive was used with a fold factor that was
     * too small to store all the values of a producer needed by the
     * consumer. */
    halide_error_code_fold_factor_too_small = -26,

    /** User-specified require() expression was not satisfied. */
    halide_error_code_requirement_failed = -27,

    /** At least one of the buffer's extents are negative. */
    halide_error_code_buffer_extents_negative = -28,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct, and it could not be upgraded to a halide_buffer_t. */
    halide_error_code_failed_to_upgrade_buffer_t = -29,

    /** A compiled pipeline was passed the old deprecated buffer_t
     * struct in bounds inference mode, but the returned information
     * can't be expressed in the old buffer_t. */
    halide_error_code_failed_to_downgrade_buffer_t = -30,

    /** A specialize_fail() schedule branch was selected at runtime. */
    halide_error_code_specialize_fail = -31,

    /** The Halide runtime encountered an error while trying to wrap a
     * native device handle.  Turn on -debug in your target string to
     * see more details. */
    halide_error_code_device_wrap_native_failed = -32,

    /** The Halide runtime encountered an error while trying to detach
     * a native device handle.  Turn on -debug in your target string
     * to see more details. */
    halide_error_code_device_detach_native_failed = -33,

    /** The host field on an input or output was null, the device
     * field was not zero, and the pipeline tries to use the buffer on
     * the host. You may be passing a GPU-only buffer to a pipeline
     * which is scheduled to use it on the CPU. */
    halide_error_code_host_is_null = -34,

    /** A folded buffer was passed to an extern stage, but the region
     * touched wraps around the fold boundary. */
    halide_error_code_bad_extern_fold = -35,

    /** Buffer has a non-null device_interface but device is 0, which
     * violates a Halide invariant. */
    halide_error_code_device_interface_no_device= -36,

    /** Buffer has both host and device dirty bits set, which violates
     * a Halide invariant. */
    halide_error_code_host_and_device_dirty = -37,

    /** The halide_buffer_t * passed to a halide runtime routine is
     * nullptr and this is not allowed. */
    halide_error_code_buffer_is_null = -38,

    /** The Halide runtime encountered an error while trying to copy
     * from one buffer to another. Turn on -debug in your target
     * string to see more details. */
    halide_error_code_device_buffer_copy_failed = -39,

    /** Attempted to make cropped/sliced alias of a buffer with a device
     * field, but the device_interface does not support cropping. */
    halide_error_code_device_crop_unsupported = -40,

    /** Cropping/slicing a buffer failed for some other reason. Turn on -debug
     * in your target string. */
    halide_error_code_device_crop_failed = -41,

    /** An operation on a buffer required an allocation on a
     * particular device interface, but a device allocation already
     * existed on a different device interface. Free the old one
     * first. */
    halide_error_code_incompatible_device_interface = -42,

    /** The dimensions field of a halide_buffer_t does not match the dimensions of that ImageParam. */
    halide_error_code_bad_dimensions = -43,

    /** An expression that would perform an integer division or modulo
     * by zero was evaluated. */
    halide_error_code_integer_division_by_zero = -44,

};

/** Halide calls the functions below on various error conditions. The
 * default implementations construct an error message, call
 * halide_error, then return the matching error code above. On
 * platforms that support weak linking, you can override these to
 * catch the errors individually. */

/** A call into an extern stage for the purposes of bounds inference
 * failed. Returns the error code given by the extern stage. */
extern int halide_error_bounds_inference_call_failed(void *user_context, const char *extern_stage_name, int result);

/** A call to an extern stage failed. Returned the error code given by
 * the extern stage. */
extern int halide_error_extern_stage_failed(void *user_context, const char *extern_stage_name, int result);

/** Various other error conditions. See the enum above for a
 * description of each. */
// @{
extern int halide_error_explicit_bounds_too_small(void *user_context, const char *func_name, const char *var_name,
                                                      int min_bound, int max_bound, int min_required, int max_required);
extern int halide_error_bad_type(void *user_context, const char *func_name,
                                 uint8_t code_given, uint8_t correct_code,
                                 uint8_t bits_given, uint8_t correct_bits,
                                 uint16_t lanes_given, uint16_t correct_lanes);
extern int halide_error_bad_dimensions(void *user_context, const char *func_name,
                                       int32_t dimensions_given, int32_t correct_dimensions);
extern int halide_error_access_out_of_bounds(void *user_context, const char *func_name,
                                             int dimension, int min_touched, int max_touched,
                                             int min_valid, int max_valid);
extern int halide_error_buffer_allocation_too_large(void *user_context, const char *buffer_name,
                                                    uint64_t allocation_size, uint64_t max_size);
extern int halide_error_buffer_extents_negative(void *user_context, const char *buffer_name, int dimension, int extent);
extern int halide_error_buffer_extents_too_large(void *user_context, const char *buffer_name,
                                                 int64_t actual_size, int64_t max_size);
extern int halide_error_constraints_make_required_region_smaller(void *user_context, const char *buffer_name,
                                                                 int dimension,
                                                                 int constrained_min, int constrained_extent,
                                                                 int required_min, int required_extent);
extern int halide_error_constraint_violated(void *user_context, const char *var, int val,
                                            const char *constrained_var, int constrained_val);
extern int halide_error_param_too_small_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t min_val);
extern int halide_error_param_too_small_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t min_val);
extern int halide_error_param_too_small_f64(void *user_context, const char *param_name,
                                            double val, double min_val);
extern int halide_error_param_too_large_i64(void *user_context, const char *param_name,
                                            int64_t val, int64_t max_val);
extern int halide_error_param_too_large_u64(void *user_context, const char *param_name,
                                            uint64_t val, uint64_t max_val);
extern int halide_error_param_too_large_f64(void *user_context, const char *param_name,
                                            double val, double max_val);
extern int halide_error_out_of_memory(void *user_context);
extern int halide_error_buffer_argument_is_null(void *user_context, const char *buffer_name);
extern int halide_error_debug_to_file_failed(void *user_context, const char *func,
                                             const char *filename, int error_code);
extern int halide_error_unaligned_host_ptr(void *user_context, const char *func_name, int alignment);
extern int halide_error_host_is_null(void *user_context, const char *func_name);
extern int halide_error_failed_to_upgrade_buffer_t(void *user_context,
                                                   const char *input_name,
                                                   const char *reason);
extern int halide_error_failed_to_downgrade_buffer_t(void *user_context,
                                                     const char *input_name,
                                                     const char *reason);
extern int halide_error_bad_fold(void *user_context, const char *func_name, const char *var_name,
                                 const char *loop_name);
extern int halide_error_bad_extern_fold(void *user_context, const char *func_name,
                                        int dim, int min, int extent, int valid_min, int fold_factor);

extern int halide_error_fold_factor_too_small(void *user_context, const char *func_name, const char *var_name,
                                              int fold_factor, const char *loop_name, int required_extent);
extern int halide_error_requirement_failed(void *user_context, const char *condition, const char *message);
extern int halide_error_specialize_fail(void *user_context, const char *message);
extern int halide_error_no_device_interface(void *user_context);
extern int halide_error_device_interface_no_device(void *user_context);
extern int halide_error_host_and_device_dirty(void *user_context);
extern int halide_error_buffer_is_null(void *user_context, const char *routine);
extern int halide_error_integer_division_by_zero(void *user_context);
// @}

/** Optional features a compilation Target can have.
 */
typedef enum halide_target_feature_t {
    halide_target_feature_jit = 0,  ///< Generate code that will run immediately inside the calling process.
    halide_target_feature_debug = 1,  ///< Turn on debug info and output for runtime code.
    halide_target_feature_no_asserts = 2,  ///< Disable all runtime checks, for slightly tighter code.
    halide_target_feature_no_bounds_query = 3, ///< Disable the bounds querying functionality.

    halide_target_feature_sse41 = 4,  ///< Use SSE 4.1 and earlier instructions. Only relevant on x86.
    halide_target_feature_avx = 5,  ///< Use AVX 1 instructions. Only relevant on x86.
    halide_target_feature_avx2 = 6,  ///< Use AVX 2 instructions. Only relevant on x86.
    halide_target_feature_fma = 7,  ///< Enable x86 FMA instruction
    halide_target_feature_fma4 = 8,  ///< Enable x86 (AMD) FMA4 instruction set
    halide_target_feature_f16c = 9,  ///< Enable x86 16-bit float support

    halide_target_feature_armv7s = 10,  ///< Generate code for ARMv7s. Only relevant for 32-bit ARM.
    halide_target_feature_no_neon = 11,  ///< Avoid using NEON instructions. Only relevant for 32-bit ARM.

    halide_target_feature_vsx = 12,  ///< Use VSX instructions. Only relevant on POWERPC.
    halide_target_feature_power_arch_2_07 = 13,  ///< Use POWER ISA 2.07 new instructions. Only relevant on POWERPC.

    halide_target_feature_cuda = 14,  ///< Enable the CUDA runtime. Defaults to compute capability 2.0 (Fermi)
    halide_target_feature_cuda_capability30 = 15,  ///< Enable CUDA compute capability 3.0 (Kepler)
    halide_target_feature_cuda_capability32 = 16,  ///< Enable CUDA compute capability 3.2 (Tegra K1)
    halide_target_feature_cuda_capability35 = 17,  ///< Enable CUDA compute capability 3.5 (Kepler)
    halide_target_feature_cuda_capability50 = 18,  ///< Enable CUDA compute capability 5.0 (Maxwell)

    halide_target_feature_opencl = 19,  ///< Enable the OpenCL runtime.
    halide_target_feature_cl_doubles = 20,  ///< Enable double support on OpenCL targets

    halide_target_feature_opengl = 21,  ///< Enable the OpenGL runtime.
    halide_target_feature_openglcompute = 22, ///< Enable OpenGL Compute runtime.

    halide_target_feature_unused_23 = 23, ///< Unused. (Formerly: Enable the RenderScript runtime.)

    halide_target_feature_user_context = 24,  ///< Generated code takes a user_context pointer as first argument

    halide_target_feature_matlab = 25,  ///< Generate a mexFunction compatible with Matlab mex libraries. See tools/mex_halide.m.

    halide_target_feature_profile = 26, ///< Launch a sampling profiler alongside the Halide pipeline that monitors and reports the runtime used by each Func
    halide_target_feature_no_runtime = 27, ///< Do not include a copy of the Halide runtime in any generated object file or assembly

    halide_target_feature_metal = 28, ///< Enable the (Apple) Metal runtime.
    halide_target_feature_mingw = 29, ///< For Windows compile to MinGW toolset rather then Visual Studio

    halide_target_feature_c_plus_plus_mangling = 30, ///< Generate C++ mangled names for result function, et al

    halide_target_feature_large_buffers = 31, ///< Enable 64-bit buffer indexing to support buffers > 2GB. Ignored if bits != 64.

    halide_target_feature_hvx_64 = 32, ///< Enable HVX 64 byte mode.
    halide_target_feature_hvx_128 = 33, ///< Enable HVX 128 byte mode.
    halide_target_feature_hvx_v62 = 34, ///< Enable Hexagon v62 architecture.
    halide_target_feature_fuzz_float_stores = 35, ///< On every floating point store, set the last bit of the mantissa to zero. Pipelines for which the output is very different with this feature enabled may also produce very different output on different processors.
    halide_target_feature_soft_float_abi = 36, ///< Enable soft float ABI. This only enables the soft float ABI calling convention, which does not necessarily use soft floats.
    halide_target_feature_msan = 37, ///< Enable hooks for MSAN support.
    halide_target_feature_avx512 = 38, ///< Enable the base AVX512 subset supported by all AVX512 architectures. The specific feature sets are AVX-512F and AVX512-CD. See https://en.wikipedia.org/wiki/AVX-512 for a description of each AVX subset.
    halide_target_feature_avx512_knl = 39, ///< Enable the AVX512 features supported by Knight's Landing chips, such as the Xeon Phi x200. This includes the base AVX512 set, and also AVX512-CD and AVX512-ER.
    halide_target_feature_avx512_skylake = 40, ///< Enable the AVX512 features supported by Skylake Xeon server processors. This adds AVX512-VL, AVX512-BW, and AVX512-DQ to the base set. The main difference from the base AVX512 set is better support for small integer ops. Note that this does not include the Knight's Landing features. Note also that these features are not available on Skylake desktop and mobile processors.
    halide_target_feature_avx512_cannonlake = 41, ///< Enable the AVX512 features expected to be supported by future Cannonlake processors. This includes all of the Skylake features, plus AVX512-IFMA and AVX512-VBMI.
    halide_target_feature_hvx_use_shared_object = 42, ///< Deprecated
    halide_target_feature_trace_loads = 43, ///< Trace all loads done by the pipeline. Equivalent to calling Func::trace_loads on every non-inlined Func.
    halide_target_feature_trace_stores = 44, ///< Trace all stores done by the pipeline. Equivalent to calling Func::trace_stores on every non-inlined Func.
    halide_target_feature_trace_realizations = 45, ///< Trace all realizations done by the pipeline. Equivalent to calling Func::trace_realizations on every non-inlined Func.
    halide_target_feature_cuda_capability61 = 46,  ///< Enable CUDA compute capability 6.1 (Pascal)
    halide_target_feature_hvx_v65 = 47, ///< Enable Hexagon v65 architecture.
    halide_target_feature_hvx_v66 = 48, ///< Enable Hexagon v66 architecture.
    halide_target_feature_cl_half = 49,  ///< Enable half support on OpenCL targets
    halide_target_feature_strict_float = 50, ///< Turn off all non-IEEE floating-point optimization. Currently applies only to LLVM targets.
    halide_target_feature_legacy_buffer_wrappers = 51,  ///< Emit legacy wrapper code for buffer_t (vs halide_buffer_t) when AOT-compiled.
    halide_target_feature_tsan = 52, ///< Enable hooks for TSAN support.
    halide_target_feature_asan = 53, ///< Enable hooks for ASAN support.
    halide_target_feature_d3d12compute = 54, ///< Enable Direct3D 12 Compute runtime.
    halide_target_feature_check_unsafe_promises = 55, ///< Insert assertions for promises.
    halide_target_feature_hexagon_dma = 56, ///< Enable Hexagon DMA buffers.
    halide_target_feature_embed_bitcode = 57,  ///< Emulate clang -fembed-bitcode flag.
    halide_target_feature_coreir = 58, ///< Enable output to CoreIR.
    halide_target_feature_coreir_valid = 59, ///< Enable output signal valid for CoreIR.
    halide_target_feature_hls = 60, ///< Enable output to HLS.
    halide_target_feature_end = 61 ///< A sentinel. Every target is considered to have this feature, and setting this feature does nothing.
} halide_target_feature_t;

/** This function is called internally by Halide in some situations to determine
 * if the current execution environment can support the given set of
 * halide_target_feature_t flags. The implementation must do the following:
 *
 * -- If there are flags set in features that the function knows *cannot* be supported, return 0.
 * -- Otherwise, return 1.
 * -- Note that any flags set in features that the function doesn't know how to test should be ignored;
 * this implies that a return value of 1 means "not known to be bad" rather than "known to be good".
 *
 * In other words: a return value of 0 means "It is not safe to use code compiled with these features",
 * while a return value of 1 means "It is not obviously unsafe to use code compiled with these features".
 *
 * The default implementation simply calls halide_default_can_use_target_features.
 *
 * Note that `features` points to an array of `count` uint64_t; this array must contain enough
 * bits to represent all the currently known features. Any excess bits must be set to zero.
 */
// @{
extern int halide_can_use_target_features(int count, const uint64_t *features);
typedef int (*halide_can_use_target_features_t)(int count, const uint64_t *features);
extern halide_can_use_target_features_t halide_set_custom_can_use_target_features(halide_can_use_target_features_t);
// @}

/**
 * This is the default implementation of halide_can_use_target_features; it is provided
 * for convenience of user code that may wish to extend halide_can_use_target_features
 * but continue providing existing support, e.g.
 *
 *     int halide_can_use_target_features(int count, const uint64_t *features) {
 *          if (features[halide_target_somefeature >> 6] & (1LL << (halide_target_somefeature & 63))) {
 *              if (!can_use_somefeature()) {
 *                  return 0;
 *              }
 *          }
 *          return halide_default_can_use_target_features(count, features);
 *     }
 */
extern int halide_default_can_use_target_features(int count, const uint64_t *features);


typedef struct halide_dimension_t {
    int32_t min, extent, stride;

    // Per-dimension flags. None are defined yet (This is reserved for future use).
    uint32_t flags;

#ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_dimension_t() : min(0), extent(0), stride(0), flags(0) {}
    HALIDE_ALWAYS_INLINE halide_dimension_t(int32_t m, int32_t e, int32_t s, uint32_t f = 0) :
        min(m), extent(e), stride(s), flags(f) {}

    HALIDE_ALWAYS_INLINE bool operator==(const halide_dimension_t &other) const {
        return (min == other.min) &&
            (extent == other.extent) &&
            (stride == other.stride) &&
            (flags == other.flags);
    }

    HALIDE_ALWAYS_INLINE bool operator!=(const halide_dimension_t &other) const {
        return !(*this == other);
    }
#endif
} halide_dimension_t;

#ifdef __cplusplus
} // extern "C"
#endif

typedef enum {halide_buffer_flag_host_dirty = 1,
              halide_buffer_flag_device_dirty = 2} halide_buffer_flags;

/**
 * The raw representation of an image passed around by generated
 * Halide code. It includes some stuff to track whether the image is
 * not actually in main memory, but instead on a device (like a
 * GPU). For a more convenient C++ wrapper, use Halide::Buffer<T>. */
typedef struct halide_buffer_t {
    /** A device-handle for e.g. GPU memory used to back this buffer. */
    uint64_t device;

    /** The interface used to interpret the above handle. */
    const struct halide_device_interface_t *device_interface;

    /** A pointer to the start of the data in main memory. In terms of
     * the Halide coordinate system, this is the address of the min
     * coordinates (defined below). */
    uint8_t* host;

    /** flags with various meanings. */
    uint64_t flags;

    /** The type of each buffer element. */
    struct halide_type_t type;

    /** The dimensionality of the buffer. */
    int32_t dimensions;

    /** The shape of the buffer. Halide does not own this array - you
     * must manage the memory for it yourself. */
    halide_dimension_t *dim;

    /** Pads the buffer up to a multiple of 8 bytes */
    void *padding;

#ifdef __cplusplus
    /** Convenience methods for accessing the flags */
    // @{
    HALIDE_ALWAYS_INLINE bool get_flag(halide_buffer_flags flag) const {
        return (flags & flag) != 0;
    }

    HALIDE_ALWAYS_INLINE void set_flag(halide_buffer_flags flag, bool value) {
        if (value) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }

    HALIDE_ALWAYS_INLINE bool host_dirty() const {
        return get_flag(halide_buffer_flag_host_dirty);
    }

    HALIDE_ALWAYS_INLINE bool device_dirty() const {
        return get_flag(halide_buffer_flag_device_dirty);
    }

    HALIDE_ALWAYS_INLINE void set_host_dirty(bool v = true) {
        set_flag(halide_buffer_flag_host_dirty, v);
    }

    HALIDE_ALWAYS_INLINE void set_device_dirty(bool v = true) {
        set_flag(halide_buffer_flag_device_dirty, v);
    }
    // @}

    /** The total number of elements this buffer represents. Equal to
     * the product of the extents */
    HALIDE_ALWAYS_INLINE size_t number_of_elements() const {
        size_t s = 1;
        for (int i = 0; i < dimensions; i++) {
            s *= dim[i].extent;
        }
        return s;
    }

    /** A pointer to the element with the lowest address. If all
     * strides are positive, equal to the host pointer. */
    HALIDE_ALWAYS_INLINE uint8_t *begin() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride < 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        return host + index * type.bytes();
    }

    /** A pointer to one beyond the element with the highest address. */
    HALIDE_ALWAYS_INLINE uint8_t *end() const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            if (dim[i].stride > 0) {
                index += dim[i].stride * (dim[i].extent - 1);
            }
        }
        index += 1;
        return host + index * type.bytes();
    }

    /** The total number of bytes spanned by the data in memory. */
    HALIDE_ALWAYS_INLINE size_t size_in_bytes() const {
        return (size_t)(end() - begin());
    }

    /** A pointer to the element at the given location. */
    HALIDE_ALWAYS_INLINE uint8_t *address_of(const int *pos) const {
        ptrdiff_t index = 0;
        for (int i = 0; i < dimensions; i++) {
            index += dim[i].stride * (pos[i] - dim[i].min);
        }
        return host + index * type.bytes();
    }

    /** Attempt to call device_sync for the buffer. If the buffer
     * has no device_interface (or no device_sync), this is a quiet no-op.
     * Calling this explicitly should rarely be necessary, except for profiling. */
    HALIDE_ALWAYS_INLINE int device_sync(void *ctx = NULL) {
        if (device_interface && device_interface->device_sync) {
            return device_interface->device_sync(ctx, this);
        }
        return 0;
    }

    /** Check if an input buffer passed extern stage is a querying
     * bounds. Compared to doing the host pointer check directly,
     * this both adds clarity to code and will facilitate moving to
     * another representation for bounds query arguments. */
    HALIDE_ALWAYS_INLINE bool is_bounds_query() const {
        return host == NULL && device == 0;
    }

#endif
} halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HALIDE_ATTRIBUTE_DEPRECATED
#ifdef HALIDE_ALLOW_DEPRECATED
#define HALIDE_ATTRIBUTE_DEPRECATED(x)
#else
#ifdef _MSC_VER
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __declspec(deprecated(x))
#else
#define HALIDE_ATTRIBUTE_DEPRECATED(x) __attribute__((deprecated(x)))
#endif
#endif
#endif

/** The old buffer_t, included for compatibility with old code. Don't
 * use it. */
#ifndef BUFFER_T_DEFINED
#define BUFFER_T_DEFINED
typedef struct buffer_t {
    uint64_t dev;
    uint8_t* host;
    int32_t extent[4];
    int32_t stride[4];
    int32_t min[4];
    int32_t elem_size;
    HALIDE_ATTRIBUTE_ALIGN(1) bool host_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) bool dev_dirty;
    HALIDE_ATTRIBUTE_ALIGN(1) uint8_t _padding[10 - sizeof(void *)];
} buffer_t;
#endif // BUFFER_T_DEFINED

/** Copies host pointer, mins, extents, strides, and device state from
 * an old-style buffer_t into a new-style halide_buffer_t. If bounds_query_only is nonzero,
 * the copy is only done if the old_buf has null host and dev (ie, a bounds query is being
 * performed); otherwise new_buf is left untouched. (This is used for input buffers to avoid
 * benign data races.) The dimensions and type fields of the new buffer_t should already be
 * set. Returns an error code if the upgrade could not be performed. */
extern int halide_upgrade_buffer_t(void *user_context, const char *name,
                                   const buffer_t *old_buf, halide_buffer_t *new_buf,
                                   int bounds_query_only);

/** Copies the host pointer, mins, extents, strides, and device state
 * from a halide_buffer_t to a buffer_t. Also sets elem_size. Useful
 * for backporting the results of bounds inference. */
extern int halide_downgrade_buffer_t(void *user_context, const char *name,
                                     const halide_buffer_t *new_buf, buffer_t *old_buf);

/** Copies the dirty flags and device allocation state from a new
 * buffer_t back to a legacy buffer_t. */
extern int halide_downgrade_buffer_t_device_fields(void *user_context, const char *name,
                                                   const halide_buffer_t *new_buf, buffer_t *old_buf);

/** halide_scalar_value_t is a simple union able to represent all the well-known
 * scalar values in a filter argument. Note that it isn't tagged with a type;
 * you must ensure you know the proper type before accessing. Most user
 * code will never need to create instances of this struct; its primary use
 * is to hold def/min/max values in a halide_filter_argument_t. (Note that
 * this is conceptually just a union; it's wrapped in a struct to ensure
 * that it doesn't get anonymized by LLVM.)
 */
struct halide_scalar_value_t {
    union {
        bool b;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        void *handle;
    } u;
    #ifdef __cplusplus
    HALIDE_ALWAYS_INLINE halide_scalar_value_t() {u.u64 = 0;}
    #endif
};

enum halide_argument_kind_t {
    halide_argument_kind_input_scalar = 0,
    halide_argument_kind_input_buffer = 1,
    halide_argument_kind_output_buffer = 2
};

/*
    These structs must be robust across different compilers and settings; when
    modifying them, strive for the following rules:

    1) All fields are explicitly sized. I.e. must use int32_t and not "int"
    2) All fields must land on an alignment boundary that is the same as their size
    3) Explicit padding is added to make that so
    4) The sizeof the struct is padded out to a multiple of the largest natural size thing in the struct
    5) don't forget that 32 and 64 bit pointers are different sizes
*/

/**
 * halide_filter_argument_t is essentially a plain-C-struct equivalent to
 * Halide::Argument; most user code will never need to create one.
 */
struct halide_filter_argument_t {
    const char *name;       // name of the argument; will never be null or empty.
    int32_t kind;           // actually halide_argument_kind_t
    int32_t dimensions;     // always zero for scalar arguments
    struct halide_type_t type;
    // These pointers should always be null for buffer arguments,
    // and *may* be null for scalar arguments. (A null value means
    // there is no def/min/max specified for this argument.)
    const struct halide_scalar_value_t *def;
    const struct halide_scalar_value_t *min;
    const struct halide_scalar_value_t *max;
};

struct halide_filter_metadata_t {
    /** version of this metadata; currently always 0. */
    int32_t version;

    /** The number of entries in the arguments field. This is always >= 1. */
    int32_t num_arguments;

    /** An array of the filters input and output arguments; this will never be
     * null. The order of arguments is not guaranteed (input and output arguments
     * may come in any order); however, it is guaranteed that all arguments
     * will have a unique name within a given filter. */
    const struct halide_filter_argument_t* arguments;

    /** The Target for which the filter was compiled. This is always
     * a canonical Target string (ie a product of Target::to_string). */
    const char* target;

    /** The function name of the filter. */
    const char* name;
};

/** The functions below here are relevant for pipelines compiled with
 * the -profile target flag, which runs a sampling profiler thread
 * alongside the pipeline. */

/** Per-Func state tracked by the sampling profiler. */
struct halide_profiler_func_stats {
    /** Total time taken evaluating this Func (in nanoseconds). */
    uint64_t time;

    /** The current memory allocation of this Func. */
    uint64_t memory_current;

    /** The peak memory allocation of this Func. */
    uint64_t memory_peak;

    /** The total memory allocation of this Func. */
    uint64_t memory_total;

    /** The peak stack allocation of this Func's threads. */
    uint64_t stack_peak;

    /** The average number of thread pool worker threads active while computing this Func. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this Func. A global constant string. */
    const char *name;

    /** The total number of memory allocation of this Func. */
    int num_allocs;
};

/** Per-pipeline state tracked by the sampling profiler. These exist
 * in a linked list. */
struct halide_profiler_pipeline_stats {
    /** Total time spent inside this pipeline (in nanoseconds) */
    uint64_t time;

    /** The current memory allocation of funcs in this pipeline. */
    uint64_t memory_current;

    /** The peak memory allocation of funcs in this pipeline. */
    uint64_t memory_peak;

    /** The total memory allocation of funcs in this pipeline. */
    uint64_t memory_total;

    /** The average number of thread pool worker threads doing useful
     * work while computing this pipeline. */
    uint64_t active_threads_numerator, active_threads_denominator;

    /** The name of this pipeline. A global constant string. */
    const char *name;

    /** An array containing states for each Func in this pipeline. */
    struct halide_profiler_func_stats *funcs;

    /** The next pipeline_stats pointer. It's a void * because types
     * in the Halide runtime may not currently be recursive. */
    void *next;

    /** The number of funcs in this pipeline. */
    int num_funcs;

    /** An internal base id used to identify the funcs in this pipeline. */
    int first_func_id;

    /** The number of times this pipeline has been run. */
    int runs;

    /** The total number of samples taken inside of this pipeline. */
    int samples;

    /** The total number of memory allocation of funcs in this pipeline. */
    int num_allocs;
};

/** The global state of the profiler. */

struct halide_profiler_state {
    /** Guards access to the fields below. If not locked, the sampling
     * profiler thread is free to modify things below (including
     * reordering the linked list of pipeline stats). */
    struct halide_mutex lock;

    /** The amount of time the profiler thread sleeps between samples
     * in milliseconds. Defaults to 1 */
    int sleep_time;

    /** An internal id used for bookkeeping. */
    int first_free_id;

    /** The id of the current running Func. Set by the pipeline, read
     * periodically by the profiler thread. */
    int current_func;

    /** The number of threads currently doing work. */
    int active_threads;

    /** A linked list of stats gathered for each pipeline. */
    struct halide_profiler_pipeline_stats *pipelines;

    /** Retrieve remote profiler state. Used so that the sampling
     * profiler can follow along with execution that occurs elsewhere,
     * e.g. on a DSP. If null, it reads from the int above instead. */
    void (*get_remote_profiler_state)(int *func, int *active_workers);

    /** Sampling thread reference to be joined at shutdown. */
    struct halide_thread *sampling_thread;
};

/** Profiler func ids with special meanings. */
enum {
    /// current_func takes on this value when not inside Halide code
    halide_profiler_outside_of_halide = -1,
    /// Set current_func to this value to tell the profiling thread to
    /// halt. It will start up again next time you run a pipeline with
    /// profiling enabled.
    halide_profiler_please_stop = -2
};

/** Get a pointer to the global profiler state for programmatic
 * inspection. Lock it before using to pause the profiler. */
extern struct halide_profiler_state *halide_profiler_get_state();

/** Get a pointer to the pipeline state associated with pipeline_name.
 * This function grabs the global profiler state's lock on entry. */
extern struct halide_profiler_pipeline_stats *halide_profiler_get_pipeline_state(const char *pipeline_name);

/** Reset profiler state cheaply. May leave threads running or some
 * memory allocated but all accumluated statistics are reset.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
extern void halide_profiler_reset();

/** Reset all profiler state.
 * WARNING: Do NOT call this method while any halide pipeline is
 * running; halide_profiler_memory_allocate/free and
 * halide_profiler_stack_peak_update update the profiler pipeline's
 * state without grabbing the global profiler state's lock. */
void halide_profiler_shutdown();

/** Print out timing statistics for everything run since the last
 * reset. Also happens at process exit. */
extern void halide_profiler_report(void *user_context);

/// \name "Float16" functions
/// These functions operate of bits (``uint16_t``) representing a half
/// precision floating point number (IEEE-754 2008 binary16).
//{@

/** Read bits representing a half precision floating point number and return
 *  the float that represents the same value */
extern float halide_float16_bits_to_float(uint16_t);

/** Read bits representing a half precision floating point number and return
 *  the double that represents the same value */
extern double halide_float16_bits_to_double(uint16_t);

// TODO: Conversion functions to half

//@}

#ifdef __cplusplus
} // End extern "C"
#endif

#ifdef __cplusplus

namespace {
template<typename T> struct check_is_pointer;
template<typename T> struct check_is_pointer<T *> {};
}

/** Construct the halide equivalent of a C type */
template<typename T>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of() {
    // Create a compile-time error if T is not a pointer (without
    // using any includes - this code goes into the runtime).
    check_is_pointer<T> check;
    (void)check;
    return halide_type_t(halide_type_handle, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<float>() {
    return halide_type_t(halide_type_float, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<double>() {
    return halide_type_t(halide_type_float, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<bool>() {
    return halide_type_t(halide_type_uint, 1);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint8_t>() {
    return halide_type_t(halide_type_uint, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint16_t>() {
    return halide_type_t(halide_type_uint, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint32_t>() {
    return halide_type_t(halide_type_uint, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<uint64_t>() {
    return halide_type_t(halide_type_uint, 64);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int8_t>() {
    return halide_type_t(halide_type_int, 8);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int16_t>() {
    return halide_type_t(halide_type_int, 16);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int32_t>() {
    return halide_type_t(halide_type_int, 32);
}

template<>
HALIDE_ALWAYS_INLINE halide_type_t halide_type_of<int64_t>() {
    return halide_type_t(halide_type_int, 64);
}

#endif

#endif // HALIDE_HALIDERUNTIME_H

#ifdef COMPILING_HALIDE_RUNTIME
#include "HalideRuntime.h"
#define HALIDE_BUFFER_HELPER_ATTRS __attribute__((always_inline, weak))
#else
#define HALIDE_BUFFER_HELPER_ATTRS inline
#endif

// Structs are annoying to deal with from within Halide Stmts. These
// utility functions are for dealing with buffer_t in that
// context. They are not intended for use outside of Halide code, and
// not exposed in HalideRuntime.h. The symbols are private to the
// module and should be inlined and then stripped. This blob of code
// also gets copy-pasted into C outputs.

extern "C" {

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_dimensions(const halide_buffer_t *buf) {
    return buf->dimensions;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t *_halide_buffer_get_host(const halide_buffer_t *buf) {
    return buf->host;
}

HALIDE_BUFFER_HELPER_ATTRS
uint64_t _halide_buffer_get_device(const halide_buffer_t *buf) {
    return buf->device;
}

HALIDE_BUFFER_HELPER_ATTRS
const struct halide_device_interface_t *_halide_buffer_get_device_interface(const halide_buffer_t *buf) {
    return buf->device_interface;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_min(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_max(const halide_buffer_t *buf, int d) {
    return buf->dim[d].min + buf->dim[d].extent - 1;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_extent(const halide_buffer_t *buf, int d) {
    return buf->dim[d].extent;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_get_stride(const halide_buffer_t *buf, int d) {
    return buf->dim[d].stride;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_host_dirty(halide_buffer_t *buf, bool val) {
    buf->set_host_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_set_device_dirty(halide_buffer_t *buf, bool val) {
    buf->set_device_dirty(val);
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_host_dirty(const halide_buffer_t *buf) {
    return buf->host_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_get_device_dirty(const halide_buffer_t *buf) {
    return buf->device_dirty();
}

HALIDE_BUFFER_HELPER_ATTRS
halide_dimension_t *_halide_buffer_get_shape(halide_buffer_t *buf) {
    return buf->dim;
}

HALIDE_BUFFER_HELPER_ATTRS
bool _halide_buffer_is_bounds_query(const halide_buffer_t *buf) {
    return buf->host == NULL && buf->device == 0;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t _halide_buffer_get_type_code(const halide_buffer_t *buf) {
    return buf->type.code;
}

HALIDE_BUFFER_HELPER_ATTRS
uint8_t _halide_buffer_get_type_bits(const halide_buffer_t *buf) {
    return buf->type.bits;
}

HALIDE_BUFFER_HELPER_ATTRS
uint16_t _halide_buffer_get_type_lanes(const halide_buffer_t *buf) {
    return buf->type.lanes;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init(halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     void *host,
                                     uint64_t device, halide_device_interface_t *device_interface,
                                     int type_code, int type_bits,
                                     int dimensions,
                                     halide_dimension_t *shape,
                                     uint64_t flags) {
    dst->host = (uint8_t *)host;
    dst->device = device;
    dst->device_interface = device_interface;
    dst->type.code = (halide_type_code_t)type_code;
    dst->type.bits = (uint8_t)type_bits;
    dst->type.lanes = 1;
    dst->dimensions = dimensions;
    dst->dim = dst_shape;
    if (shape != dst->dim) {
        for (int i = 0; i < dimensions; i++) {
            dst->dim[i] = shape[i];
        }
    }
    dst->flags = flags;
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_init_from_buffer(halide_buffer_t *dst,
                                                 halide_dimension_t *dst_shape,
                                                 const halide_buffer_t *src) {
    dst->host = src->host;
    dst->device = src->device;
    dst->device_interface = src->device_interface;
    dst->type = src->type;
    dst->dimensions = src->dimensions;
    dst->dim = dst_shape;
    dst->flags = src->flags;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
    }
    return dst;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_crop(void *user_context,
                                     halide_buffer_t *dst,
                                     halide_dimension_t *dst_shape,
                                     const halide_buffer_t *src,
                                     const int *min, const int *extent) {
    *dst = *src;
    dst->dim = dst_shape;
    int64_t offset = 0;
    for (int i = 0; i < dst->dimensions; i++) {
        dst->dim[i] = src->dim[i];
        dst->dim[i].min = min[i];
        dst->dim[i].extent = extent[i];
        offset += (min[i] - src->dim[i].min) * src->dim[i].stride;
    }
    if (dst->host) {
        dst->host += offset * src->type.bytes();
    }
    dst->device_interface = 0;
    dst->device = 0;
    if (src->device_interface) {
        src->device_interface->device_crop(user_context, src, dst);
    }
    return dst;
}


// Called on return from an extern stage where the output buffer was a
// crop of some other larger buffer. This happens for extern stages
// with distinct store_at/compute_at levels. Each call to the stage
// only fills in part of the buffer.
HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crop_after_extern_stage(void *user_context,
                                                   void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    halide_buffer_t *crop = buffers[0];
    halide_buffer_t *parent = buffers[1];

    if (crop->device) {
        if (!parent->device) {
            // We have been given a device allocation by the extern
            // stage. It only represents the cropped region, so we
            // can't just give it to the parent.
            if (crop->device_dirty()) {
                crop->device_interface->copy_to_host(user_context, crop);
            }
            crop->device_interface->device_free(user_context, crop);
        } else {
            // We are a crop of an existing device allocation.
            if (crop->device_dirty()) {
                parent->set_device_dirty();
            }
            crop->device_interface->device_release_crop(user_context, crop);
        }
    }
    if (crop->host_dirty()) {
        parent->set_host_dirty();
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
int _halide_buffer_retire_crops_after_extern_stage(void *user_context,
                                                    void *obj) {
    halide_buffer_t **buffers = (halide_buffer_t **)obj;
    while (*buffers) {
        _halide_buffer_retire_crop_after_extern_stage(user_context, buffers);
        buffers += 2;
    }
    return 0;
}

HALIDE_BUFFER_HELPER_ATTRS
halide_buffer_t *_halide_buffer_set_bounds(halide_buffer_t *buf,
                                           int dim, int min, int extent) {
    buf->dim[dim].min = min;
    buf->dim[dim].extent = extent;
    return buf;
}

}

#undef HALIDE_BUFFER_HELPER_ATTRS

#ifndef HALIDE_FUNCTION_ATTRS
#define HALIDE_FUNCTION_ATTRS
#endif



#ifdef __cplusplus
extern "C" {
#endif


void extract_camera_pipeline(void *_output, void* _input, int32_t _13, int32_t _16, int32_t _18, int32_t _29, int32_t _32){
  int32_t _278 = _18 * 2;
  int32_t _279 = _18 * 3;
  int32_t _280 = _18 * 4;
  int32_t _281 = _18 * 5;
  int32_t _282 = _18 * 6;
  int32_t _283 = _282 + 4;
  int32_t _284 = _282 + 3;
  int32_t _285 = _282 + 2;
  int32_t _286 = _281 + 4;
  int32_t _287 = _281 + 3;
  int32_t _288 = _281 + 2;
  int32_t _289 = _16 * _18;
  int32_t _290 = _289 + _13;
  for (int _output_s0_c = 0; _output_s0_c < 0 + 3; _output_s0_c++)
  {
   bool _291 = _output_s0_c == 1;
   bool _292 = _output_s0_c == 0;
   int32_t _293 = _output_s0_c * _32;
   for (int _output_s0_y = 0; _output_s0_y < 0 + 58; _output_s0_y++)
   {
    int32_t _294 = _output_s0_y & 1;
    bool _295 = _294 == 0;
    int32_t _296 = _18 * _output_s0_y;
    int32_t _297 = _296 - _290;
    int32_t _298 = _output_s0_y * _29;
    int32_t _299 = _298 + _293;
    int32_t _300 = _18 + 4;
    int32_t _301 = _18 + 3;
    int32_t _302 = _18 + 2;
    for (int _output_s0_x = 0; _output_s0_x < 0 + 58; _output_s0_x++)
    {
     int32_t _303 = _output_s0_x + _297;
     int32_t _304 = _303 + _279;
     int32_t _305 = _304 + 1;
     uint8_t _306 = ((const uint8_t *)_input)[_305];
     int32_t _307 = _304 + 5;
     uint8_t _308 = ((const uint8_t *)_input)[_307];
     int32_t _309 = _303 + _301;
     uint8_t _310 = ((const uint8_t *)_input)[_309];
     int32_t _311 = _303 + _287;
     uint8_t _312 = ((const uint8_t *)_input)[_311];
     int32_t _313 = _304 + 3;
     uint8_t _314 = ((const uint8_t *)_input)[_313];
     uint8_t _315 = ::halide_cpp_max(_306, _308);
     uint8_t _316 = ::halide_cpp_max(_315, _310);
     uint8_t _317 = ::halide_cpp_max(_316, _312);
     uint8_t _318 = ::halide_cpp_min(_314, _317);
     uint8_t _319 = ::halide_cpp_min(_306, _308);
     uint8_t _320 = ::halide_cpp_min(_319, _310);
     uint8_t _321 = ::halide_cpp_min(_320, _312);
     uint8_t _322 = ::halide_cpp_max(_318, _321);
     int32_t _323 = _output_s0_x & 1;
     bool _324 = _323 == 0;
     int32_t _325 = _303 + _278;
     int32_t _326 = _325 + 2;
     uint8_t _327 = ((const uint8_t *)_input)[_326];
     int32_t _328 = _325 + 6;
     uint8_t _329 = ((const uint8_t *)_input)[_328];
     int32_t _330 = _303 + 4;
     uint8_t _331 = ((const uint8_t *)_input)[_330];
     int32_t _332 = _303 + _280;
     int32_t _333 = _332 + 4;
     uint8_t _334 = ((const uint8_t *)_input)[_333];
     int32_t _335 = _325 + 4;
     uint8_t _336 = ((const uint8_t *)_input)[_335];
     uint8_t _337 = ::halide_cpp_max(_327, _329);
     uint8_t _338 = ::halide_cpp_max(_337, _331);
     uint8_t _339 = ::halide_cpp_max(_338, _334);
     uint8_t _340 = ::halide_cpp_min(_339, _336);
     uint8_t _341 = ::halide_cpp_min(_327, _329);
     uint8_t _342 = ::halide_cpp_min(_341, _331);
     uint8_t _343 = ::halide_cpp_min(_342, _334);
     uint8_t _344 = ::halide_cpp_max(_340, _343);
     int32_t _345 = _325 + 1;
     uint8_t _346 = ((const uint8_t *)_input)[_345];
     int32_t _347 = _325 + 5;
     uint8_t _348 = ((const uint8_t *)_input)[_347];
     int32_t _349 = _303 + 3;
     uint8_t _350 = ((const uint8_t *)_input)[_349];
     int32_t _351 = _332 + 3;
     uint8_t _352 = ((const uint8_t *)_input)[_351];
     int32_t _353 = _325 + 3;
     uint8_t _354 = ((const uint8_t *)_input)[_353];
     uint8_t _355 = ::halide_cpp_max(_346, _348);
     uint8_t _356 = ::halide_cpp_max(_355, _350);
     uint8_t _357 = ::halide_cpp_max(_356, _352);
     uint8_t _358 = ::halide_cpp_min(_357, _354);
     uint8_t _359 = ::halide_cpp_min(_346, _348);
     uint8_t _360 = ::halide_cpp_min(_359, _350);
     uint8_t _361 = ::halide_cpp_min(_360, _352);
     uint8_t _362 = ::halide_cpp_max(_358, _361);
     int32_t _363 = _304 + 2;
     uint8_t _364 = ((const uint8_t *)_input)[_363];
     int32_t _365 = _304 + 6;
     uint8_t _366 = ((const uint8_t *)_input)[_365];
     int32_t _367 = _303 + _300;
     uint8_t _368 = ((const uint8_t *)_input)[_367];
     int32_t _369 = _303 + _286;
     uint8_t _370 = ((const uint8_t *)_input)[_369];
     int32_t _371 = _304 + 4;
     uint8_t _372 = ((const uint8_t *)_input)[_371];
     uint8_t _373 = ::halide_cpp_max(_364, _366);
     uint8_t _374 = ::halide_cpp_max(_373, _368);
     uint8_t _375 = ::halide_cpp_max(_374, _370);
     uint8_t _376 = ::halide_cpp_min(_375, _372);
     uint8_t _377 = ::halide_cpp_min(_364, _366);
     uint8_t _378 = ::halide_cpp_min(_377, _368);
     uint8_t _379 = ::halide_cpp_min(_378, _370);
     uint8_t _380 = ::halide_cpp_max(_376, _379);
     uint8_t _381 = _322 + _380;
     uint8_t _382 = (uint8_t)(1);
     uint8_t _383 = _381 + _382;
     uint8_t _384 = _383 >> _382;
     uint8_t _385 = _322 + _362;
     uint8_t _386 = _385 + _382;
     uint8_t _387 = _386 >> _382;
     uint8_t _388 = _380 - _322;
     uint8_t _389 = _322 - _380;
     bool _390 = _322 < _380;
     uint8_t _391 = (uint8_t)(_390 ? _388 : _389);
     uint8_t _392 = _391;
     uint8_t _393 = _362 - _322;
     uint8_t _394 = _322 - _362;
     bool _395 = _322 < _362;
     uint8_t _396 = (uint8_t)(_395 ? _393 : _394);
     uint8_t _397 = _396;
     bool _398 = _392 < _397;
     uint8_t _399 = (uint8_t)(_398 ? _384 : _387);
     uint8_t _400 = ((const uint8_t *)_input)[_304];
     int32_t _401 = _303 + _302;
     uint8_t _402 = ((const uint8_t *)_input)[_401];
     int32_t _403 = _303 + _288;
     uint8_t _404 = ((const uint8_t *)_input)[_403];
     uint8_t _405 = ::halide_cpp_max(_372, _400);
     uint8_t _406 = ::halide_cpp_max(_405, _402);
     uint8_t _407 = ::halide_cpp_max(_406, _404);
     uint8_t _408 = ::halide_cpp_min(_407, _364);
     uint8_t _409 = ::halide_cpp_min(_372, _400);
     uint8_t _410 = ::halide_cpp_min(_409, _402);
     uint8_t _411 = ::halide_cpp_min(_410, _404);
     uint8_t _412 = ::halide_cpp_max(_408, _411);
     int32_t _413 = _332 + 1;
     uint8_t _414 = ((const uint8_t *)_input)[_413];
     int32_t _415 = _332 + 5;
     uint8_t _416 = ((const uint8_t *)_input)[_415];
     int32_t _417 = _303 + _284;
     uint8_t _418 = ((const uint8_t *)_input)[_417];
     uint8_t _419 = ::halide_cpp_max(_414, _416);
     uint8_t _420 = ::halide_cpp_max(_419, _354);
     uint8_t _421 = ::halide_cpp_max(_420, _418);
     uint8_t _422 = ::halide_cpp_min(_421, _352);
     uint8_t _423 = ::halide_cpp_min(_414, _416);
     uint8_t _424 = ::halide_cpp_min(_423, _354);
     uint8_t _425 = ::halide_cpp_min(_424, _418);
     uint8_t _426 = ::halide_cpp_max(_422, _425);
     uint8_t _427 = _322 + _412;
     uint8_t _428 = _427 + _382;
     uint8_t _429 = _428 >> _382;
     uint8_t _430 = _322 + _426;
     uint8_t _431 = _430 + _382;
     uint8_t _432 = _431 >> _382;
     uint8_t _433 = _412 - _322;
     uint8_t _434 = _322 - _412;
     bool _435 = _322 < _412;
     uint8_t _436 = (uint8_t)(_435 ? _433 : _434);
     uint8_t _437 = _436;
     uint8_t _438 = _426 - _322;
     uint8_t _439 = _322 - _426;
     bool _440 = _322 < _426;
     uint8_t _441 = (uint8_t)(_440 ? _438 : _439);
     uint8_t _442 = _441;
     bool _443 = _437 < _442;
     uint8_t _444 = (uint8_t)(_443 ? _429 : _432);
     uint8_t _445 = ((const uint8_t *)_input)[_325];
     int32_t _446 = _303 + 2;
     uint8_t _447 = ((const uint8_t *)_input)[_446];
     int32_t _448 = _332 + 2;
     uint8_t _449 = ((const uint8_t *)_input)[_448];
     uint8_t _450 = ::halide_cpp_max(_336, _445);
     uint8_t _451 = ::halide_cpp_max(_450, _447);
     uint8_t _452 = ::halide_cpp_max(_451, _449);
     uint8_t _453 = ::halide_cpp_min(_452, _327);
     uint8_t _454 = ::halide_cpp_min(_336, _445);
     uint8_t _455 = ::halide_cpp_min(_454, _447);
     uint8_t _456 = ::halide_cpp_min(_455, _449);
     uint8_t _457 = ::halide_cpp_max(_453, _456);
     uint8_t _458 = _362 + _457;
     uint8_t _459 = _458 + _382;
     uint8_t _460 = _459 >> _382;
     uint8_t _461 = _457 - _362;
     uint8_t _462 = _362 - _457;
     bool _463 = _362 < _457;
     uint8_t _464 = (uint8_t)(_463 ? _461 : _462);
     uint8_t _465 = _464;
     bool _466 = _362 < _322;
     uint8_t _467 = (uint8_t)(_466 ? _394 : _393);
     uint8_t _468 = _467;
     bool _469 = _465 < _468;
     uint8_t _470 = (uint8_t)(_469 ? _460 : _387);
     int32_t _471 = _332 + 6;
     uint8_t _472 = ((const uint8_t *)_input)[_471];
     int32_t _473 = _303 + _283;
     uint8_t _474 = ((const uint8_t *)_input)[_473];
     uint8_t _475 = ::halide_cpp_max(_449, _472);
     uint8_t _476 = ::halide_cpp_max(_475, _336);
     uint8_t _477 = ::halide_cpp_max(_476, _474);
     uint8_t _478 = ::halide_cpp_min(_477, _334);
     uint8_t _479 = ::halide_cpp_min(_449, _472);
     uint8_t _480 = ::halide_cpp_min(_479, _336);
     uint8_t _481 = ::halide_cpp_min(_480, _474);
     uint8_t _482 = ::halide_cpp_max(_478, _481);
     uint8_t _483 = ((const uint8_t *)_input)[_332];
     int32_t _484 = _303 + _285;
     uint8_t _485 = ((const uint8_t *)_input)[_484];
     uint8_t _486 = ::halide_cpp_max(_334, _483);
     uint8_t _487 = ::halide_cpp_max(_486, _327);
     uint8_t _488 = ::halide_cpp_max(_487, _485);
     uint8_t _489 = ::halide_cpp_min(_488, _449);
     uint8_t _490 = ::halide_cpp_min(_334, _483);
     uint8_t _491 = ::halide_cpp_min(_490, _327);
     uint8_t _492 = ::halide_cpp_min(_491, _485);
     uint8_t _493 = ::halide_cpp_max(_489, _492);
     uint8_t _494 = _412 + _457;
     uint8_t _495 = _494 + _382;
     uint8_t _496 = _495 >> _382;
     bool _497 = _412 < _322;
     uint8_t _498 = (uint8_t)(_497 ? _434 : _433);
     uint8_t _499 = _498;
     uint8_t _500 = _457 - _412;
     uint8_t _501 = _412 - _457;
     bool _502 = _412 < _457;
     uint8_t _503 = (uint8_t)(_502 ? _500 : _501);
     uint8_t _504 = _503;
     bool _505 = _499 < _504;
     uint8_t _506 = (uint8_t)(_505 ? _429 : _496);
     uint8_t _507 = _426 + _482;
     uint8_t _508 = _507 + _382;
     uint8_t _509 = _508 >> _382;
     uint8_t _510 = _482 - _426;
     uint8_t _511 = _426 - _482;
     bool _512 = _426 < _482;
     uint8_t _513 = (uint8_t)(_512 ? _510 : _511);
     uint8_t _514 = _513;
     bool _515 = _426 < _322;
     uint8_t _516 = (uint8_t)(_515 ? _439 : _438);
     uint8_t _517 = _516;
     bool _518 = _514 < _517;
     uint8_t _519 = (uint8_t)(_518 ? _509 : _432);
     uint8_t _520 = _380 + _482;
     uint8_t _521 = _520 + _382;
     uint8_t _522 = _521 >> _382;
     bool _523 = _380 < _322;
     uint8_t _524 = (uint8_t)(_523 ? _389 : _388);
     uint8_t _525 = _524;
     uint8_t _526 = _482 - _380;
     uint8_t _527 = _380 - _482;
     bool _528 = _380 < _482;
     uint8_t _529 = (uint8_t)(_528 ? _526 : _527);
     uint8_t _530 = _529;
     bool _531 = _525 < _530;
     uint8_t _532 = (uint8_t)(_531 ? _384 : _522);
     uint8_t _533 = _432 + _322;
     uint8_t _534 = _399 + _519;
     uint8_t _535 = _534 + _382;
     uint8_t _536 = _535 >> _382;
     uint8_t _537 = _533 - _536;
     uint8_t _538 = _322 + _493;
     uint8_t _539 = _538 + _382;
     uint8_t _540 = _539 >> _382;
     uint8_t _541 = _540 + _444;
     uint8_t _542 = _426 + _493;
     uint8_t _543 = _542 + _382;
     uint8_t _544 = _543 >> _382;
     uint8_t _545 = _412 + _493;
     uint8_t _546 = _545 + _382;
     uint8_t _547 = _546 >> _382;
     uint8_t _548 = _426 - _493;
     uint8_t _549 = _493 - _426;
     bool _550 = _493 < _426;
     uint8_t _551 = (uint8_t)(_550 ? _548 : _549);
     uint8_t _552 = _551;
     uint8_t _553 = _412 - _493;
     uint8_t _554 = _493 - _412;
     bool _555 = _493 < _412;
     uint8_t _556 = (uint8_t)(_555 ? _553 : _554);
     uint8_t _557 = _556;
     bool _558 = _552 < _557;
     uint8_t _559 = (uint8_t)(_558 ? _544 : _547);
     uint8_t _560 = _559 + _399;
     uint8_t _561 = _560 + _382;
     uint8_t _562 = _561 >> _382;
     uint8_t _563 = _541 - _562;
     uint8_t _564 = _412 + _426;
     uint8_t _565 = _564 + _382;
     uint8_t _566 = _565 >> _382;
     uint8_t _567 = _566 + _444;
     uint8_t _568 = _506 + _519;
     uint8_t _569 = _568 + _382;
     uint8_t _570 = _569 >> _382;
     uint8_t _571 = _567 - _570;
     uint8_t _572 = _322 - _493;
     uint8_t _573 = _493 - _322;
     bool _574 = _493 < _322;
     uint8_t _575 = (uint8_t)(_574 ? _572 : _573);
     uint8_t _576 = _575;
     uint8_t _577 = _412 - _426;
     uint8_t _578 = _426 - _412;
     bool _579 = _426 < _412;
     uint8_t _580 = (uint8_t)(_579 ? _577 : _578);
     uint8_t _581 = _580;
     bool _582 = _576 < _581;
     uint8_t _583 = (uint8_t)(_582 ? _563 : _571);
     uint8_t _584 = (uint8_t)(_324 ? _537 : _583);
     uint8_t _585 = _429 + _322;
     uint8_t _586 = _399 + _506;
     uint8_t _587 = _586 + _382;
     uint8_t _588 = _587 >> _382;
     uint8_t _589 = _585 - _588;
     uint8_t _590 = (uint8_t)(_324 ? _322 : _589);
     uint8_t _591 = (uint8_t)(_295 ? _584 : _590);
     uint8_t _592 = _384 + _322;
     uint8_t _593 = _444 + _532;
     uint8_t _594 = _593 + _382;
     uint8_t _595 = _594 >> _382;
     uint8_t _596 = _592 - _595;
     uint8_t _597 = (uint8_t)(_324 ? _596 : _322);
     uint8_t _598 = _322 + _344;
     uint8_t _599 = _598 + _382;
     uint8_t _600 = _599 >> _382;
     uint8_t _601 = _600 + _399;
     uint8_t _602 = _344 + _362;
     uint8_t _603 = _602 + _382;
     uint8_t _604 = _603 >> _382;
     uint8_t _605 = _344 + _380;
     uint8_t _606 = _605 + _382;
     uint8_t _607 = _606 >> _382;
     uint8_t _608 = _362 - _344;
     uint8_t _609 = _344 - _362;
     bool _610 = _344 < _362;
     uint8_t _611 = (uint8_t)(_610 ? _608 : _609);
     uint8_t _612 = _611;
     uint8_t _613 = _380 - _344;
     uint8_t _614 = _344 - _380;
     bool _615 = _344 < _380;
     uint8_t _616 = (uint8_t)(_615 ? _613 : _614);
     uint8_t _617 = _616;
     bool _618 = _612 < _617;
     uint8_t _619 = (uint8_t)(_618 ? _604 : _607);
     uint8_t _620 = _619 + _444;
     uint8_t _621 = _620 + _382;
     uint8_t _622 = _621 >> _382;
     uint8_t _623 = _601 - _622;
     uint8_t _624 = _362 + _380;
     uint8_t _625 = _624 + _382;
     uint8_t _626 = _625 >> _382;
     uint8_t _627 = _626 + _399;
     uint8_t _628 = _470 + _532;
     uint8_t _629 = _628 + _382;
     uint8_t _630 = _629 >> _382;
     uint8_t _631 = _627 - _630;
     uint8_t _632 = _322 - _344;
     uint8_t _633 = _344 - _322;
     bool _634 = _344 < _322;
     uint8_t _635 = (uint8_t)(_634 ? _632 : _633);
     uint8_t _636 = _635;
     uint8_t _637 = _380 - _362;
     uint8_t _638 = _362 - _380;
     bool _639 = _362 < _380;
     uint8_t _640 = (uint8_t)(_639 ? _637 : _638);
     uint8_t _641 = _640;
     bool _642 = _636 < _641;
     uint8_t _643 = (uint8_t)(_642 ? _623 : _631);
     uint8_t _644 = _387 + _322;
     uint8_t _645 = _444 + _470;
     uint8_t _646 = _645 + _382;
     uint8_t _647 = _646 >> _382;
     uint8_t _648 = _644 - _647;
     uint8_t _649 = (uint8_t)(_324 ? _643 : _648);
     uint8_t _650 = (uint8_t)(_295 ? _597 : _649);
     uint8_t _651 = (uint8_t)(_324 ? _322 : _444);
     uint8_t _652 = (uint8_t)(_324 ? _399 : _322);
     uint8_t _653 = (uint8_t)(_295 ? _651 : _652);
     int32_t _654 = (int32_t)(_650);
     int32_t _655 = _654 * 200;
     int32_t _656 = (int32_t)(_591);
     int32_t _657 = _656 * 17;
     int32_t _658 = _655 + _657;
     int32_t _659 = (int32_t)(_653);
     int32_t _660 = _659 * 44;
     int32_t _661 = _658 - _660;
     int32_t _662 = _661 + -3900;
     int32_t _663 = _662 >> 8;
     int16_t _664 = (int16_t)(_663);
     int32_t _665 = _659 * 159;
     int32_t _666 = _654 * 38;
     int32_t _667 = _665 - _666;
     int32_t _668 = _656 * 21;
     int32_t _669 = _667 - _668;
     int32_t _670 = _669 + -2541;
     int32_t _671 = _670 >> 8;
     int16_t _672 = (int16_t)(_671);
     int32_t _673 = _656 * 228;
     int32_t _674 = _659 * 73;
     int32_t _675 = _673 - _674;
     int32_t _676 = _654 * 8;
     int32_t _677 = _675 - _676;
     int32_t _678 = _677 + -2008;
     int32_t _679 = _678 >> 8;
     int16_t _680 = (int16_t)(_679);
     int16_t _681 = (int16_t)(_291 ? _672 : _680);
     int16_t _682 = (int16_t)(_292 ? _664 : _681);
     int16_t _683 = (int16_t)(1023);
     int16_t _684 = ::halide_cpp_min(_682, _683);
     int16_t _685 = (int16_t)(0);
     int16_t _686 = ::halide_cpp_max(_684, _685);
     int32_t _687 = (int32_t)(_686);
     float _688 = (float)(_687);
     float _689 = float_from_bits(981467136 /* 0.000976562 */);
     float _690 = _688 * _689;
     float _691 = float_from_bits(1065353216 /* 1 */);
     float _692 = pow_f32(_690, _691);
     uint8_t _693 = (uint8_t)(0);
     float _694 = _691 - _692;
     float _695 = _694 * _694;
     float _696 = float_from_bits(1013181184 /* 0.013911 */);
     float _697 = _695 * _696;
     float _698 = float_from_bits(1065236522 /* 0.993044 */);
     float _699 = _694 * _698;
     float _700 = _697 + _699;
     float _701 = _691 - _700;
     float _702 = _692 * _692;
     float _703 = _702 * _696;
     float _704 = _692 * _698;
     float _705 = _703 + _704;
     float _706 = float_from_bits(1056964608 /* 0.5 */);
     bool _707 = _706 < _692;
     float _708 = (float)(_707 ? _701 : _705);
     float _709 = float_from_bits(1132462080 /* 256 */);
     float _710 = _708 * _709;
     float _711 = float_from_bits(1132396544 /* 255 */);
     float _712 = ::halide_cpp_min(_710, _711);
     float _713 = float_from_bits(0 /* 0 */);
     float _714 = ::halide_cpp_max(_712, _713);
     uint8_t _715 = (uint8_t)(_714);
     uint8_t _716 = (uint8_t)(255);
     int16_t _717 = (int16_t)(1024);
     bool _718 = _682 < _717;
     uint8_t _719 = (uint8_t)(_718 ? _715 : _716);
     bool _720 = _682 < _685;
     uint8_t _721 = (uint8_t)(_720 ? _693 : _719);
     int32_t _722 = _output_s0_x + _299;
     ((uint8_t *)_output)[_722] = _721;
    } // for _output_s0_x
   } // for _output_s0_y
  } // for _output_s0_c
}

int camera_pipeline(struct halide_buffer_t *_input_buffer, struct halide_buffer_t *_output_buffer) HALIDE_FUNCTION_ATTRS {
 void * const _ucon = nullptr;
 uint64_t _0 = (uint64_t)(_output_buffer);
 uint64_t _1 = (uint64_t)(0);
 bool _2 = _0 != _1;
 if (!_2)  {
  int32_t _3 = halide_error_buffer_argument_is_null(_ucon, "output");
  return _3;
 }
 uint64_t _4 = (uint64_t)(_input_buffer);
 uint64_t _5 = (uint64_t)(0);
 bool _6 = _4 != _5;
 if (!_6)  {
  int32_t _7 = halide_error_buffer_argument_is_null(_ucon, "input");
  return _7;
 }
 void *_8 = _halide_buffer_get_host(_input_buffer);
 void * _input = _8;
 uint8_t _9 = _halide_buffer_get_type_code(_input_buffer);
 uint8_t _10 = _halide_buffer_get_type_bits(_input_buffer);
 uint16_t _11 = _halide_buffer_get_type_lanes(_input_buffer);
 int32_t _12 = _halide_buffer_get_dimensions(_input_buffer);
 int32_t _13 = _halide_buffer_get_min(_input_buffer, 0);
 int32_t _14 = _halide_buffer_get_extent(_input_buffer, 0);
 int32_t _15 = _halide_buffer_get_stride(_input_buffer, 0);
 int32_t _16 = _halide_buffer_get_min(_input_buffer, 1);
 int32_t _17 = _halide_buffer_get_extent(_input_buffer, 1);
 int32_t _18 = _halide_buffer_get_stride(_input_buffer, 1);
 void *_19 = _halide_buffer_get_host(_output_buffer);
 void * _output = _19;
 uint8_t _20 = _halide_buffer_get_type_code(_output_buffer);
 uint8_t _21 = _halide_buffer_get_type_bits(_output_buffer);
 uint16_t _22 = _halide_buffer_get_type_lanes(_output_buffer);
 int32_t _23 = _halide_buffer_get_dimensions(_output_buffer);
 int32_t _24 = _halide_buffer_get_min(_output_buffer, 0);
 int32_t _25 = _halide_buffer_get_extent(_output_buffer, 0);
 int32_t _26 = _halide_buffer_get_stride(_output_buffer, 0);
 int32_t _27 = _halide_buffer_get_min(_output_buffer, 1);
 int32_t _28 = _halide_buffer_get_extent(_output_buffer, 1);
 int32_t _29 = _halide_buffer_get_stride(_output_buffer, 1);
 int32_t _30 = _halide_buffer_get_min(_output_buffer, 2);
 int32_t _31 = _halide_buffer_get_extent(_output_buffer, 2);
 int32_t _32 = _halide_buffer_get_stride(_output_buffer, 2);
 bool _33 = _halide_buffer_is_bounds_query(_input_buffer);
 if (_33)
 {
  struct halide_dimension_t *_34 = _halide_buffer_get_shape(_input_buffer);
  uint64_t _35 = (uint64_t)(0);
  void *_36 = (void *)(_35);
  struct halide_device_interface_t *_37 = (struct halide_device_interface_t *)(_35);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
  } s0 = {
   0,
   64,
   1,
   0,
   0,
   64,
   64,
   0
  };
  struct halide_dimension_t *_38 = (struct halide_dimension_t *)(&s0);
  struct halide_buffer_t *_39 = _halide_buffer_init(_input_buffer, _34, _36, _35, _37, 1, 8, 2, _38, _35);
  (void)_39;
 } // if _33
 bool _40 = _halide_buffer_is_bounds_query(_output_buffer);
 if (_40)
 {
  struct halide_dimension_t *_41 = _halide_buffer_get_shape(_output_buffer);
  uint64_t _42 = (uint64_t)(0);
  void *_43 = (void *)(_42);
  struct halide_device_interface_t *_44 = (struct halide_device_interface_t *)(_42);
  struct {
   const int32_t f_0;
   const int32_t f_1;
   const int32_t f_2;
   const int32_t f_3;
   const int32_t f_4;
   const int32_t f_5;
   const int32_t f_6;
   const int32_t f_7;
   const int32_t f_8;
   const int32_t f_9;
   const int32_t f_10;
   const int32_t f_11;
  } s1 = {
   0,
   58,
   1,
   0,
   0,
   58,
   58,
   0,
   0,
   3,
   3364,
   0
  };
  struct halide_dimension_t *_45 = (struct halide_dimension_t *)(&s1);
  struct halide_buffer_t *_46 = _halide_buffer_init(_output_buffer, _41, _43, _42, _44, 1, 8, 3, _45, _42);
  (void)_46;
 } // if _40
 bool _47 = _halide_buffer_is_bounds_query(_input_buffer);
 bool _48 = _halide_buffer_is_bounds_query(_output_buffer);
 bool _49 = _47 || _48;
 bool _50 = !(_49);
 if (_50)
 {
  uint8_t _51 = (uint8_t)(1);
  bool _52 = _9 == _51;
  uint8_t _53 = (uint8_t)(8);
  bool _54 = _10 == _53;
  bool _55 = _52 && _54;
  uint16_t _56 = (uint16_t)(1);
  bool _57 = _11 == _56;
  bool _58 = _55 && _57;
  if (!_58)   {
   uint8_t _59 = (uint8_t)(1);
   uint8_t _60 = (uint8_t)(8);
   uint16_t _61 = (uint16_t)(1);
   int32_t _62 = halide_error_bad_type(_ucon, "Input buffer input", _9, _59, _10, _60, _11, _61);
   return _62;
  }
  bool _63 = _12 == 2;
  if (!_63)   {
   int32_t _64 = halide_error_bad_dimensions(_ucon, "Input buffer input", _12, 2);
   return _64;
  }
  uint8_t _65 = (uint8_t)(1);
  bool _66 = _20 == _65;
  uint8_t _67 = (uint8_t)(8);
  bool _68 = _21 == _67;
  bool _69 = _66 && _68;
  uint16_t _70 = (uint16_t)(1);
  bool _71 = _22 == _70;
  bool _72 = _69 && _71;
  if (!_72)   {
   uint8_t _73 = (uint8_t)(1);
   uint8_t _74 = (uint8_t)(8);
   uint16_t _75 = (uint16_t)(1);
   int32_t _76 = halide_error_bad_type(_ucon, "Output buffer output", _20, _73, _21, _74, _22, _75);
   return _76;
  }
  bool _77 = _23 == 3;
  if (!_77)   {
   int32_t _78 = halide_error_bad_dimensions(_ucon, "Output buffer output", _23, 3);
   return _78;
  }
  bool _79 = _13 <= 0;
  int32_t _80 = _14 + _13;
  bool _81 = 64 <= _80;
  bool _82 = _79 && _81;
  if (!_82)   {
   int32_t _83 = _14 + _13;
   int32_t _84 = _83 + -1;
   int32_t _85 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 0, 0, 63, _13, _84);
   return _85;
  }
  bool _86 = 0 <= _14;
  if (!_86)   {
   int32_t _87 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 0, _14);
   return _87;
  }
  bool _88 = _16 <= 0;
  int32_t _89 = _17 + _16;
  bool _90 = 64 <= _89;
  bool _91 = _88 && _90;
  if (!_91)   {
   int32_t _92 = _17 + _16;
   int32_t _93 = _92 + -1;
   int32_t _94 = halide_error_access_out_of_bounds(_ucon, "Input buffer input", 1, 0, 63, _16, _93);
   return _94;
  }
  bool _95 = 0 <= _17;
  if (!_95)   {
   int32_t _96 = halide_error_buffer_extents_negative(_ucon, "Input buffer input", 1, _17);
   return _96;
  }
  bool _97 = _24 <= 0;
  int32_t _98 = _25 + _24;
  bool _99 = 58 <= _98;
  bool _100 = _97 && _99;
  if (!_100)   {
   int32_t _101 = _25 + _24;
   int32_t _102 = _101 + -1;
   int32_t _103 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 0, 0, 57, _24, _102);
   return _103;
  }
  bool _104 = 0 <= _25;
  if (!_104)   {
   int32_t _105 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 0, _25);
   return _105;
  }
  bool _106 = _27 <= 0;
  int32_t _107 = _28 + _27;
  bool _108 = 58 <= _107;
  bool _109 = _106 && _108;
  if (!_109)   {
   int32_t _110 = _28 + _27;
   int32_t _111 = _110 + -1;
   int32_t _112 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 1, 0, 57, _27, _111);
   return _112;
  }
  bool _113 = 0 <= _28;
  if (!_113)   {
   int32_t _114 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 1, _28);
   return _114;
  }
  bool _115 = _30 <= 0;
  int32_t _116 = _31 + _30;
  bool _117 = 3 <= _116;
  bool _118 = _115 && _117;
  if (!_118)   {
   int32_t _119 = _31 + _30;
   int32_t _120 = _119 + -1;
   int32_t _121 = halide_error_access_out_of_bounds(_ucon, "Output buffer output", 2, 0, 2, _30, _120);
   return _121;
  }
  bool _122 = 0 <= _31;
  if (!_122)   {
   int32_t _123 = halide_error_buffer_extents_negative(_ucon, "Output buffer output", 2, _31);
   return _123;
  }
  bool _124 = _15 == 1;
  if (!_124)   {
   int32_t _125 = halide_error_constraint_violated(_ucon, "input.stride.0", _15, "1", 1);
   return _125;
  }
  bool _126 = _26 == 1;
  if (!_126)   {
   int32_t _127 = halide_error_constraint_violated(_ucon, "output.stride.0", _26, "1", 1);
   return _127;
  }
  int64_t _128 = (int64_t)(_17);
  int64_t _129 = (int64_t)(_14);
  int64_t _130 = _128 * _129;
  int64_t _131 = (int64_t)(_28);
  int64_t _132 = (int64_t)(_25);
  int64_t _133 = _131 * _132;
  int64_t _134 = (int64_t)(_31);
  int64_t _135 = _133 * _134;
  int64_t _136 = (int64_t)(0);
  int64_t _137 = _136 - _129;
  bool _138 = _129 > _136;
  int64_t _139 = (int64_t)(_138 ? _129 : _137);
  uint64_t _140 = (uint64_t)(_139);
  uint64_t _141 = _140;
  uint64_t _142 = (uint64_t)(2147483647);
  bool _143 = _141 <= _142;
  if (!_143)   {
   int64_t _144 = (int64_t)(_14);
   int64_t _145 = (int64_t)(0);
   int64_t _146 = _145 - _144;
   bool _147 = _144 > _145;
   int64_t _148 = (int64_t)(_147 ? _144 : _146);
   uint64_t _149 = (uint64_t)(_148);
   uint64_t _150 = _149;
   uint64_t _151 = (uint64_t)(2147483647);
   int32_t _152 = halide_error_buffer_allocation_too_large(_ucon, "input", _150, _151);
   return _152;
  }
  int64_t _153 = (int64_t)(_17);
  int64_t _154 = (int64_t)(_18);
  int64_t _155 = _153 * _154;
  int64_t _156 = (int64_t)(0);
  int64_t _157 = _156 - _155;
  bool _158 = _155 > _156;
  int64_t _159 = (int64_t)(_158 ? _155 : _157);
  uint64_t _160 = (uint64_t)(_159);
  uint64_t _161 = _160;
  uint64_t _162 = (uint64_t)(2147483647);
  bool _163 = _161 <= _162;
  if (!_163)   {
   int64_t _164 = (int64_t)(_17);
   int64_t _165 = (int64_t)(_18);
   int64_t _166 = _164 * _165;
   int64_t _167 = (int64_t)(0);
   int64_t _168 = _167 - _166;
   bool _169 = _166 > _167;
   int64_t _170 = (int64_t)(_169 ? _166 : _168);
   uint64_t _171 = (uint64_t)(_170);
   uint64_t _172 = _171;
   uint64_t _173 = (uint64_t)(2147483647);
   int32_t _174 = halide_error_buffer_allocation_too_large(_ucon, "input", _172, _173);
   return _174;
  }
  int64_t _175 = (int64_t)(2147483647);
  bool _176 = _130 <= _175;
  if (!_176)   {
   int64_t _177 = (int64_t)(2147483647);
   int32_t _178 = halide_error_buffer_extents_too_large(_ucon, "input", _130, _177);
   return _178;
  }
  int64_t _179 = (int64_t)(_25);
  int64_t _180 = (int64_t)(0);
  int64_t _181 = _180 - _179;
  bool _182 = _179 > _180;
  int64_t _183 = (int64_t)(_182 ? _179 : _181);
  uint64_t _184 = (uint64_t)(_183);
  uint64_t _185 = _184;
  uint64_t _186 = (uint64_t)(2147483647);
  bool _187 = _185 <= _186;
  if (!_187)   {
   int64_t _188 = (int64_t)(_25);
   int64_t _189 = (int64_t)(0);
   int64_t _190 = _189 - _188;
   bool _191 = _188 > _189;
   int64_t _192 = (int64_t)(_191 ? _188 : _190);
   uint64_t _193 = (uint64_t)(_192);
   uint64_t _194 = _193;
   uint64_t _195 = (uint64_t)(2147483647);
   int32_t _196 = halide_error_buffer_allocation_too_large(_ucon, "output", _194, _195);
   return _196;
  }
  int64_t _197 = (int64_t)(_28);
  int64_t _198 = (int64_t)(_29);
  int64_t _199 = _197 * _198;
  int64_t _200 = (int64_t)(0);
  int64_t _201 = _200 - _199;
  bool _202 = _199 > _200;
  int64_t _203 = (int64_t)(_202 ? _199 : _201);
  uint64_t _204 = (uint64_t)(_203);
  uint64_t _205 = _204;
  uint64_t _206 = (uint64_t)(2147483647);
  bool _207 = _205 <= _206;
  if (!_207)   {
   int64_t _208 = (int64_t)(_28);
   int64_t _209 = (int64_t)(_29);
   int64_t _210 = _208 * _209;
   int64_t _211 = (int64_t)(0);
   int64_t _212 = _211 - _210;
   bool _213 = _210 > _211;
   int64_t _214 = (int64_t)(_213 ? _210 : _212);
   uint64_t _215 = (uint64_t)(_214);
   uint64_t _216 = _215;
   uint64_t _217 = (uint64_t)(2147483647);
   int32_t _218 = halide_error_buffer_allocation_too_large(_ucon, "output", _216, _217);
   return _218;
  }
  int64_t _219 = (int64_t)(2147483647);
  bool _220 = _133 <= _219;
  if (!_220)   {
   int64_t _221 = (int64_t)(2147483647);
   int32_t _222 = halide_error_buffer_extents_too_large(_ucon, "output", _133, _221);
   return _222;
  }
  int64_t _223 = (int64_t)(_31);
  int64_t _224 = (int64_t)(_32);
  int64_t _225 = _223 * _224;
  int64_t _226 = (int64_t)(0);
  int64_t _227 = _226 - _225;
  bool _228 = _225 > _226;
  int64_t _229 = (int64_t)(_228 ? _225 : _227);
  uint64_t _230 = (uint64_t)(_229);
  uint64_t _231 = _230;
  uint64_t _232 = (uint64_t)(2147483647);
  bool _233 = _231 <= _232;
  if (!_233)   {
   int64_t _234 = (int64_t)(_31);
   int64_t _235 = (int64_t)(_32);
   int64_t _236 = _234 * _235;
   int64_t _237 = (int64_t)(0);
   int64_t _238 = _237 - _236;
   bool _239 = _236 > _237;
   int64_t _240 = (int64_t)(_239 ? _236 : _238);
   uint64_t _241 = (uint64_t)(_240);
   uint64_t _242 = _241;
   uint64_t _243 = (uint64_t)(2147483647);
   int32_t _244 = halide_error_buffer_allocation_too_large(_ucon, "output", _242, _243);
   return _244;
  }
  int64_t _245 = (int64_t)(2147483647);
  bool _246 = _135 <= _245;
  if (!_246)   {
   int64_t _247 = (int64_t)(2147483647);
   int32_t _248 = halide_error_buffer_extents_too_large(_ucon, "output", _135, _247);
   return _248;
  }
  uint64_t _249 = (uint64_t)(0);
  void *_250 = (void *)(_249);
  bool _251 = _input != _250;
  if (!_251)   {
   int32_t _252 = halide_error_host_is_null(_ucon, "Input buffer input");
   return _252;
  }
  uint64_t _253 = (uint64_t)(0);
  void *_254 = (void *)(_253);
  bool _255 = _output != _254;
  if (!_255)   {
   int32_t _256 = halide_error_host_is_null(_ucon, "Output buffer output");
   return _256;
  }
  bool _257 = 0 <= _27;
  int32_t _258 = _28 + _27;
  bool _259 = _258 <= 58;
  bool _260 = _257 && _259;
  if (!_260)   {
   int32_t _261 = _28 + _27;
   int32_t _262 = _261 + -1;
   int32_t _263 = halide_error_explicit_bounds_too_small(_ucon, "y", "output", 0, 57, _27, _262);
   return _263;
  }
  bool _264 = 0 <= _24;
  int32_t _265 = _25 + _24;
  bool _266 = _265 <= 58;
  bool _267 = _264 && _266;
  if (!_267)   {
   int32_t _268 = _25 + _24;
   int32_t _269 = _268 + -1;
   int32_t _270 = halide_error_explicit_bounds_too_small(_ucon, "x", "output", 0, 57, _24, _269);
   return _270;
  }
  bool _271 = 0 <= _30;
  int32_t _272 = _31 + _30;
  bool _273 = _272 <= 3;
  bool _274 = _271 && _273;
  if (!_274)   {
   int32_t _275 = _31 + _30;
   int32_t _276 = _275 + -1;
   int32_t _277 = halide_error_explicit_bounds_too_small(_ucon, "c", "output", 0, 2, _30, _276);
   return _277;
  }
  // produce output
  //extract_camera_pipeline(_output, _input);
  extract_camera_pipeline(_output, _input, _13, _16, _18, _29, _32);
//   int32_t _278 = _18 * 2;
//   int32_t _279 = _18 * 3;
//   int32_t _280 = _18 * 4;
//   int32_t _281 = _18 * 5;
//   int32_t _282 = _18 * 6;
//   int32_t _283 = _282 + 4;
//   int32_t _284 = _282 + 3;
//   int32_t _285 = _282 + 2;
//   int32_t _286 = _281 + 4;
//   int32_t _287 = _281 + 3;
//   int32_t _288 = _281 + 2;
//   int32_t _289 = _16 * _18;
//   int32_t _290 = _289 + _13;
//   for (int _output_s0_c = 0; _output_s0_c < 0 + 3; _output_s0_c++)
//   {
//    bool _291 = _output_s0_c == 1;
//    bool _292 = _output_s0_c == 0;
//    int32_t _293 = _output_s0_c * _32;
//    for (int _output_s0_y = 0; _output_s0_y < 0 + 58; _output_s0_y++)
//    {
//     int32_t _294 = _output_s0_y & 1;
//     bool _295 = _294 == 0;
//     int32_t _296 = _18 * _output_s0_y;
//     int32_t _297 = _296 - _290;
//     int32_t _298 = _output_s0_y * _29;
//     int32_t _299 = _298 + _293;
//     int32_t _300 = _18 + 4;
//     int32_t _301 = _18 + 3;
//     int32_t _302 = _18 + 2;
//     for (int _output_s0_x = 0; _output_s0_x < 0 + 58; _output_s0_x++)
//     {
//      int32_t _303 = _output_s0_x + _297;
//      int32_t _304 = _303 + _279;
//      int32_t _305 = _304 + 1;
//      uint8_t _306 = ((const uint8_t *)_input)[_305];
//      int32_t _307 = _304 + 5;
//      uint8_t _308 = ((const uint8_t *)_input)[_307];
//      int32_t _309 = _303 + _301;
//      uint8_t _310 = ((const uint8_t *)_input)[_309];
//      int32_t _311 = _303 + _287;
//      uint8_t _312 = ((const uint8_t *)_input)[_311];
//      int32_t _313 = _304 + 3;
//      uint8_t _314 = ((const uint8_t *)_input)[_313];
//      uint8_t _315 = ::halide_cpp_max(_306, _308);
//      uint8_t _316 = ::halide_cpp_max(_315, _310);
//      uint8_t _317 = ::halide_cpp_max(_316, _312);
//      uint8_t _318 = ::halide_cpp_min(_314, _317);
//      uint8_t _319 = ::halide_cpp_min(_306, _308);
//      uint8_t _320 = ::halide_cpp_min(_319, _310);
//      uint8_t _321 = ::halide_cpp_min(_320, _312);
//      uint8_t _322 = ::halide_cpp_max(_318, _321);
//      int32_t _323 = _output_s0_x & 1;
//      bool _324 = _323 == 0;
//      int32_t _325 = _303 + _278;
//      int32_t _326 = _325 + 2;
//      uint8_t _327 = ((const uint8_t *)_input)[_326];
//      int32_t _328 = _325 + 6;
//      uint8_t _329 = ((const uint8_t *)_input)[_328];
//      int32_t _330 = _303 + 4;
//      uint8_t _331 = ((const uint8_t *)_input)[_330];
//      int32_t _332 = _303 + _280;
//      int32_t _333 = _332 + 4;
//      uint8_t _334 = ((const uint8_t *)_input)[_333];
//      int32_t _335 = _325 + 4;
//      uint8_t _336 = ((const uint8_t *)_input)[_335];
//      uint8_t _337 = ::halide_cpp_max(_327, _329);
//      uint8_t _338 = ::halide_cpp_max(_337, _331);
//      uint8_t _339 = ::halide_cpp_max(_338, _334);
//      uint8_t _340 = ::halide_cpp_min(_339, _336);
//      uint8_t _341 = ::halide_cpp_min(_327, _329);
//      uint8_t _342 = ::halide_cpp_min(_341, _331);
//      uint8_t _343 = ::halide_cpp_min(_342, _334);
//      uint8_t _344 = ::halide_cpp_max(_340, _343);
//      int32_t _345 = _325 + 1;
//      uint8_t _346 = ((const uint8_t *)_input)[_345];
//      int32_t _347 = _325 + 5;
//      uint8_t _348 = ((const uint8_t *)_input)[_347];
//      int32_t _349 = _303 + 3;
//      uint8_t _350 = ((const uint8_t *)_input)[_349];
//      int32_t _351 = _332 + 3;
//      uint8_t _352 = ((const uint8_t *)_input)[_351];
//      int32_t _353 = _325 + 3;
//      uint8_t _354 = ((const uint8_t *)_input)[_353];
//      uint8_t _355 = ::halide_cpp_max(_346, _348);
//      uint8_t _356 = ::halide_cpp_max(_355, _350);
//      uint8_t _357 = ::halide_cpp_max(_356, _352);
//      uint8_t _358 = ::halide_cpp_min(_357, _354);
//      uint8_t _359 = ::halide_cpp_min(_346, _348);
//      uint8_t _360 = ::halide_cpp_min(_359, _350);
//      uint8_t _361 = ::halide_cpp_min(_360, _352);
//      uint8_t _362 = ::halide_cpp_max(_358, _361);
//      int32_t _363 = _304 + 2;
//      uint8_t _364 = ((const uint8_t *)_input)[_363];
//      int32_t _365 = _304 + 6;
//      uint8_t _366 = ((const uint8_t *)_input)[_365];
//      int32_t _367 = _303 + _300;
//      uint8_t _368 = ((const uint8_t *)_input)[_367];
//      int32_t _369 = _303 + _286;
//      uint8_t _370 = ((const uint8_t *)_input)[_369];
//      int32_t _371 = _304 + 4;
//      uint8_t _372 = ((const uint8_t *)_input)[_371];
//      uint8_t _373 = ::halide_cpp_max(_364, _366);
//      uint8_t _374 = ::halide_cpp_max(_373, _368);
//      uint8_t _375 = ::halide_cpp_max(_374, _370);
//      uint8_t _376 = ::halide_cpp_min(_375, _372);
//      uint8_t _377 = ::halide_cpp_min(_364, _366);
//      uint8_t _378 = ::halide_cpp_min(_377, _368);
//      uint8_t _379 = ::halide_cpp_min(_378, _370);
//      uint8_t _380 = ::halide_cpp_max(_376, _379);
//      uint8_t _381 = _322 + _380;
//      uint8_t _382 = (uint8_t)(1);
//      uint8_t _383 = _381 + _382;
//      uint8_t _384 = _383 >> _382;
//      uint8_t _385 = _322 + _362;
//      uint8_t _386 = _385 + _382;
//      uint8_t _387 = _386 >> _382;
//      uint8_t _388 = _380 - _322;
//      uint8_t _389 = _322 - _380;
//      bool _390 = _322 < _380;
//      uint8_t _391 = (uint8_t)(_390 ? _388 : _389);
//      uint8_t _392 = _391;
//      uint8_t _393 = _362 - _322;
//      uint8_t _394 = _322 - _362;
//      bool _395 = _322 < _362;
//      uint8_t _396 = (uint8_t)(_395 ? _393 : _394);
//      uint8_t _397 = _396;
//      bool _398 = _392 < _397;
//      uint8_t _399 = (uint8_t)(_398 ? _384 : _387);
//      uint8_t _400 = ((const uint8_t *)_input)[_304];
//      int32_t _401 = _303 + _302;
//      uint8_t _402 = ((const uint8_t *)_input)[_401];
//      int32_t _403 = _303 + _288;
//      uint8_t _404 = ((const uint8_t *)_input)[_403];
//      uint8_t _405 = ::halide_cpp_max(_372, _400);
//      uint8_t _406 = ::halide_cpp_max(_405, _402);
//      uint8_t _407 = ::halide_cpp_max(_406, _404);
//      uint8_t _408 = ::halide_cpp_min(_407, _364);
//      uint8_t _409 = ::halide_cpp_min(_372, _400);
//      uint8_t _410 = ::halide_cpp_min(_409, _402);
//      uint8_t _411 = ::halide_cpp_min(_410, _404);
//      uint8_t _412 = ::halide_cpp_max(_408, _411);
//      int32_t _413 = _332 + 1;
//      uint8_t _414 = ((const uint8_t *)_input)[_413];
//      int32_t _415 = _332 + 5;
//      uint8_t _416 = ((const uint8_t *)_input)[_415];
//      int32_t _417 = _303 + _284;
//      uint8_t _418 = ((const uint8_t *)_input)[_417];
//      uint8_t _419 = ::halide_cpp_max(_414, _416);
//      uint8_t _420 = ::halide_cpp_max(_419, _354);
//      uint8_t _421 = ::halide_cpp_max(_420, _418);
//      uint8_t _422 = ::halide_cpp_min(_421, _352);
//      uint8_t _423 = ::halide_cpp_min(_414, _416);
//      uint8_t _424 = ::halide_cpp_min(_423, _354);
//      uint8_t _425 = ::halide_cpp_min(_424, _418);
//      uint8_t _426 = ::halide_cpp_max(_422, _425);
//      uint8_t _427 = _322 + _412;
//      uint8_t _428 = _427 + _382;
//      uint8_t _429 = _428 >> _382;
//      uint8_t _430 = _322 + _426;
//      uint8_t _431 = _430 + _382;
//      uint8_t _432 = _431 >> _382;
//      uint8_t _433 = _412 - _322;
//      uint8_t _434 = _322 - _412;
//      bool _435 = _322 < _412;
//      uint8_t _436 = (uint8_t)(_435 ? _433 : _434);
//      uint8_t _437 = _436;
//      uint8_t _438 = _426 - _322;
//      uint8_t _439 = _322 - _426;
//      bool _440 = _322 < _426;
//      uint8_t _441 = (uint8_t)(_440 ? _438 : _439);
//      uint8_t _442 = _441;
//      bool _443 = _437 < _442;
//      uint8_t _444 = (uint8_t)(_443 ? _429 : _432);
//      uint8_t _445 = ((const uint8_t *)_input)[_325];
//      int32_t _446 = _303 + 2;
//      uint8_t _447 = ((const uint8_t *)_input)[_446];
//      int32_t _448 = _332 + 2;
//      uint8_t _449 = ((const uint8_t *)_input)[_448];
//      uint8_t _450 = ::halide_cpp_max(_336, _445);
//      uint8_t _451 = ::halide_cpp_max(_450, _447);
//      uint8_t _452 = ::halide_cpp_max(_451, _449);
//      uint8_t _453 = ::halide_cpp_min(_452, _327);
//      uint8_t _454 = ::halide_cpp_min(_336, _445);
//      uint8_t _455 = ::halide_cpp_min(_454, _447);
//      uint8_t _456 = ::halide_cpp_min(_455, _449);
//      uint8_t _457 = ::halide_cpp_max(_453, _456);
//      uint8_t _458 = _362 + _457;
//      uint8_t _459 = _458 + _382;
//      uint8_t _460 = _459 >> _382;
//      uint8_t _461 = _457 - _362;
//      uint8_t _462 = _362 - _457;
//      bool _463 = _362 < _457;
//      uint8_t _464 = (uint8_t)(_463 ? _461 : _462);
//      uint8_t _465 = _464;
//      bool _466 = _362 < _322;
//      uint8_t _467 = (uint8_t)(_466 ? _394 : _393);
//      uint8_t _468 = _467;
//      bool _469 = _465 < _468;
//      uint8_t _470 = (uint8_t)(_469 ? _460 : _387);
//      int32_t _471 = _332 + 6;
//      uint8_t _472 = ((const uint8_t *)_input)[_471];
//      int32_t _473 = _303 + _283;
//      uint8_t _474 = ((const uint8_t *)_input)[_473];
//      uint8_t _475 = ::halide_cpp_max(_449, _472);
//      uint8_t _476 = ::halide_cpp_max(_475, _336);
//      uint8_t _477 = ::halide_cpp_max(_476, _474);
//      uint8_t _478 = ::halide_cpp_min(_477, _334);
//      uint8_t _479 = ::halide_cpp_min(_449, _472);
//      uint8_t _480 = ::halide_cpp_min(_479, _336);
//      uint8_t _481 = ::halide_cpp_min(_480, _474);
//      uint8_t _482 = ::halide_cpp_max(_478, _481);
//      uint8_t _483 = ((const uint8_t *)_input)[_332];
//      int32_t _484 = _303 + _285;
//      uint8_t _485 = ((const uint8_t *)_input)[_484];
//      uint8_t _486 = ::halide_cpp_max(_334, _483);
//      uint8_t _487 = ::halide_cpp_max(_486, _327);
//      uint8_t _488 = ::halide_cpp_max(_487, _485);
//      uint8_t _489 = ::halide_cpp_min(_488, _449);
//      uint8_t _490 = ::halide_cpp_min(_334, _483);
//      uint8_t _491 = ::halide_cpp_min(_490, _327);
//      uint8_t _492 = ::halide_cpp_min(_491, _485);
//      uint8_t _493 = ::halide_cpp_max(_489, _492);
//      uint8_t _494 = _412 + _457;
//      uint8_t _495 = _494 + _382;
//      uint8_t _496 = _495 >> _382;
//      bool _497 = _412 < _322;
//      uint8_t _498 = (uint8_t)(_497 ? _434 : _433);
//      uint8_t _499 = _498;
//      uint8_t _500 = _457 - _412;
//      uint8_t _501 = _412 - _457;
//      bool _502 = _412 < _457;
//      uint8_t _503 = (uint8_t)(_502 ? _500 : _501);
//      uint8_t _504 = _503;
//      bool _505 = _499 < _504;
//      uint8_t _506 = (uint8_t)(_505 ? _429 : _496);
//      uint8_t _507 = _426 + _482;
//      uint8_t _508 = _507 + _382;
//      uint8_t _509 = _508 >> _382;
//      uint8_t _510 = _482 - _426;
//      uint8_t _511 = _426 - _482;
//      bool _512 = _426 < _482;
//      uint8_t _513 = (uint8_t)(_512 ? _510 : _511);
//      uint8_t _514 = _513;
//      bool _515 = _426 < _322;
//      uint8_t _516 = (uint8_t)(_515 ? _439 : _438);
//      uint8_t _517 = _516;
//      bool _518 = _514 < _517;
//      uint8_t _519 = (uint8_t)(_518 ? _509 : _432);
//      uint8_t _520 = _380 + _482;
//      uint8_t _521 = _520 + _382;
//      uint8_t _522 = _521 >> _382;
//      bool _523 = _380 < _322;
//      uint8_t _524 = (uint8_t)(_523 ? _389 : _388);
//      uint8_t _525 = _524;
//      uint8_t _526 = _482 - _380;
//      uint8_t _527 = _380 - _482;
//      bool _528 = _380 < _482;
//      uint8_t _529 = (uint8_t)(_528 ? _526 : _527);
//      uint8_t _530 = _529;
//      bool _531 = _525 < _530;
//      uint8_t _532 = (uint8_t)(_531 ? _384 : _522);
//      uint8_t _533 = _432 + _322;
//      uint8_t _534 = _399 + _519;
//      uint8_t _535 = _534 + _382;
//      uint8_t _536 = _535 >> _382;
//      uint8_t _537 = _533 - _536;
//      uint8_t _538 = _322 + _493;
//      uint8_t _539 = _538 + _382;
//      uint8_t _540 = _539 >> _382;
//      uint8_t _541 = _540 + _444;
//      uint8_t _542 = _426 + _493;
//      uint8_t _543 = _542 + _382;
//      uint8_t _544 = _543 >> _382;
//      uint8_t _545 = _412 + _493;
//      uint8_t _546 = _545 + _382;
//      uint8_t _547 = _546 >> _382;
//      uint8_t _548 = _426 - _493;
//      uint8_t _549 = _493 - _426;
//      bool _550 = _493 < _426;
//      uint8_t _551 = (uint8_t)(_550 ? _548 : _549);
//      uint8_t _552 = _551;
//      uint8_t _553 = _412 - _493;
//      uint8_t _554 = _493 - _412;
//      bool _555 = _493 < _412;
//      uint8_t _556 = (uint8_t)(_555 ? _553 : _554);
//      uint8_t _557 = _556;
//      bool _558 = _552 < _557;
//      uint8_t _559 = (uint8_t)(_558 ? _544 : _547);
//      uint8_t _560 = _559 + _399;
//      uint8_t _561 = _560 + _382;
//      uint8_t _562 = _561 >> _382;
//      uint8_t _563 = _541 - _562;
//      uint8_t _564 = _412 + _426;
//      uint8_t _565 = _564 + _382;
//      uint8_t _566 = _565 >> _382;
//      uint8_t _567 = _566 + _444;
//      uint8_t _568 = _506 + _519;
//      uint8_t _569 = _568 + _382;
//      uint8_t _570 = _569 >> _382;
//      uint8_t _571 = _567 - _570;
//      uint8_t _572 = _322 - _493;
//      uint8_t _573 = _493 - _322;
//      bool _574 = _493 < _322;
//      uint8_t _575 = (uint8_t)(_574 ? _572 : _573);
//      uint8_t _576 = _575;
//      uint8_t _577 = _412 - _426;
//      uint8_t _578 = _426 - _412;
//      bool _579 = _426 < _412;
//      uint8_t _580 = (uint8_t)(_579 ? _577 : _578);
//      uint8_t _581 = _580;
//      bool _582 = _576 < _581;
//      uint8_t _583 = (uint8_t)(_582 ? _563 : _571);
//      uint8_t _584 = (uint8_t)(_324 ? _537 : _583);
//      uint8_t _585 = _429 + _322;
//      uint8_t _586 = _399 + _506;
//      uint8_t _587 = _586 + _382;
//      uint8_t _588 = _587 >> _382;
//      uint8_t _589 = _585 - _588;
//      uint8_t _590 = (uint8_t)(_324 ? _322 : _589);
//      uint8_t _591 = (uint8_t)(_295 ? _584 : _590);
//      uint8_t _592 = _384 + _322;
//      uint8_t _593 = _444 + _532;
//      uint8_t _594 = _593 + _382;
//      uint8_t _595 = _594 >> _382;
//      uint8_t _596 = _592 - _595;
//      uint8_t _597 = (uint8_t)(_324 ? _596 : _322);
//      uint8_t _598 = _322 + _344;
//      uint8_t _599 = _598 + _382;
//      uint8_t _600 = _599 >> _382;
//      uint8_t _601 = _600 + _399;
//      uint8_t _602 = _344 + _362;
//      uint8_t _603 = _602 + _382;
//      uint8_t _604 = _603 >> _382;
//      uint8_t _605 = _344 + _380;
//      uint8_t _606 = _605 + _382;
//      uint8_t _607 = _606 >> _382;
//      uint8_t _608 = _362 - _344;
//      uint8_t _609 = _344 - _362;
//      bool _610 = _344 < _362;
//      uint8_t _611 = (uint8_t)(_610 ? _608 : _609);
//      uint8_t _612 = _611;
//      uint8_t _613 = _380 - _344;
//      uint8_t _614 = _344 - _380;
//      bool _615 = _344 < _380;
//      uint8_t _616 = (uint8_t)(_615 ? _613 : _614);
//      uint8_t _617 = _616;
//      bool _618 = _612 < _617;
//      uint8_t _619 = (uint8_t)(_618 ? _604 : _607);
//      uint8_t _620 = _619 + _444;
//      uint8_t _621 = _620 + _382;
//      uint8_t _622 = _621 >> _382;
//      uint8_t _623 = _601 - _622;
//      uint8_t _624 = _362 + _380;
//      uint8_t _625 = _624 + _382;
//      uint8_t _626 = _625 >> _382;
//      uint8_t _627 = _626 + _399;
//      uint8_t _628 = _470 + _532;
//      uint8_t _629 = _628 + _382;
//      uint8_t _630 = _629 >> _382;
//      uint8_t _631 = _627 - _630;
//      uint8_t _632 = _322 - _344;
//      uint8_t _633 = _344 - _322;
//      bool _634 = _344 < _322;
//      uint8_t _635 = (uint8_t)(_634 ? _632 : _633);
//      uint8_t _636 = _635;
//      uint8_t _637 = _380 - _362;
//      uint8_t _638 = _362 - _380;
//      bool _639 = _362 < _380;
//      uint8_t _640 = (uint8_t)(_639 ? _637 : _638);
//      uint8_t _641 = _640;
//      bool _642 = _636 < _641;
//      uint8_t _643 = (uint8_t)(_642 ? _623 : _631);
//      uint8_t _644 = _387 + _322;
//      uint8_t _645 = _444 + _470;
//      uint8_t _646 = _645 + _382;
//      uint8_t _647 = _646 >> _382;
//      uint8_t _648 = _644 - _647;
//      uint8_t _649 = (uint8_t)(_324 ? _643 : _648);
//      uint8_t _650 = (uint8_t)(_295 ? _597 : _649);
//      uint8_t _651 = (uint8_t)(_324 ? _322 : _444);
//      uint8_t _652 = (uint8_t)(_324 ? _399 : _322);
//      uint8_t _653 = (uint8_t)(_295 ? _651 : _652);
//      int32_t _654 = (int32_t)(_650);
//      int32_t _655 = _654 * 200;
//      int32_t _656 = (int32_t)(_591);
//      int32_t _657 = _656 * 17;
//      int32_t _658 = _655 + _657;
//      int32_t _659 = (int32_t)(_653);
//      int32_t _660 = _659 * 44;
//      int32_t _661 = _658 - _660;
//      int32_t _662 = _661 + -3900;
//      int32_t _663 = _662 >> 8;
//      int16_t _664 = (int16_t)(_663);
//      int32_t _665 = _659 * 159;
//      int32_t _666 = _654 * 38;
//      int32_t _667 = _665 - _666;
//      int32_t _668 = _656 * 21;
//      int32_t _669 = _667 - _668;
//      int32_t _670 = _669 + -2541;
//      int32_t _671 = _670 >> 8;
//      int16_t _672 = (int16_t)(_671);
//      int32_t _673 = _656 * 228;
//      int32_t _674 = _659 * 73;
//      int32_t _675 = _673 - _674;
//      int32_t _676 = _654 * 8;
//      int32_t _677 = _675 - _676;
//      int32_t _678 = _677 + -2008;
//      int32_t _679 = _678 >> 8;
//      int16_t _680 = (int16_t)(_679);
//      int16_t _681 = (int16_t)(_291 ? _672 : _680);
//      int16_t _682 = (int16_t)(_292 ? _664 : _681);
//      int16_t _683 = (int16_t)(1023);
//      int16_t _684 = ::halide_cpp_min(_682, _683);
//      int16_t _685 = (int16_t)(0);
//      int16_t _686 = ::halide_cpp_max(_684, _685);
//      int32_t _687 = (int32_t)(_686);
//      float _688 = (float)(_687);
//      float _689 = float_from_bits(981467136 /* 0.000976562 */);
//      float _690 = _688 * _689;
//      float _691 = float_from_bits(1065353216 /* 1 */);
//      float _692 = pow_f32(_690, _691);
//      uint8_t _693 = (uint8_t)(0);
//      float _694 = _691 - _692;
//      float _695 = _694 * _694;
//      float _696 = float_from_bits(1013181184 /* 0.013911 */);
//      float _697 = _695 * _696;
//      float _698 = float_from_bits(1065236522 /* 0.993044 */);
//      float _699 = _694 * _698;
//      float _700 = _697 + _699;
//      float _701 = _691 - _700;
//      float _702 = _692 * _692;
//      float _703 = _702 * _696;
//      float _704 = _692 * _698;
//      float _705 = _703 + _704;
//      float _706 = float_from_bits(1056964608 /* 0.5 */);
//      bool _707 = _706 < _692;
//      float _708 = (float)(_707 ? _701 : _705);
//      float _709 = float_from_bits(1132462080 /* 256 */);
//      float _710 = _708 * _709;
//      float _711 = float_from_bits(1132396544 /* 255 */);
//      float _712 = ::halide_cpp_min(_710, _711);
//      float _713 = float_from_bits(0 /* 0 */);
//      float _714 = ::halide_cpp_max(_712, _713);
//      uint8_t _715 = (uint8_t)(_714);
//      uint8_t _716 = (uint8_t)(255);
//      int16_t _717 = (int16_t)(1024);
//      bool _718 = _682 < _717;
//      uint8_t _719 = (uint8_t)(_718 ? _715 : _716);
//      bool _720 = _682 < _685;
//      uint8_t _721 = (uint8_t)(_720 ? _693 : _719);
//      int32_t _722 = _output_s0_x + _299;
//      ((uint8_t *)_output)[_722] = _721;
//     } // for _output_s0_x
//    } // for _output_s0_y
//   } // for _output_s0_c
 } // if _50
 return 0;
}

#ifdef __cplusplus
}  // extern "C"
#endif

