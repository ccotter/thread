
#ifndef _INCLUDE_THREADLIB_H
#define _INCLUDE_THREADLIB_H

#include <errors.h>
#include <stdint.h>
#include <llist.h>

// =============================================
// Declare constants here.
// =============================================
enum
{
    N_MAX_THREADS = 10
};

enum
{
    THREAD_SUCCESS = 0,
};

enum
{
    THREAD_RUNNING = 1,
    THREAD_WAITING = 2,
    THREAD_STOPPED = 3,
    THREAD_FINISHED = 4,
    THREAD_EXCEPTION = 5,
    THREAD_UNUSED = 6,
    THREAD_ACTIVE = 7,
} ThreadState;

// =============================================
// Declare types here.
// =============================================

typedef int (*thread_function)(void*);

typedef struct
{
    uint32_t esp;
} _proc_state;

struct _thread
{
    thread_function function;
    void *function_data;
    _proc_state proc_state;
    uint32_t *stack_top;
    int state;
    llist toNotify;
    llist waitingFor;
};
typedef struct _thread thread;

// =============================================
// Declare methods here.
// =============================================

int thread_init(thread**, thread_function, void*);
int thread_run(thread*);
int thread_wait(void);
void thread_yield(void);
void thread_join(thread*);

void printstats(void);

#endif

