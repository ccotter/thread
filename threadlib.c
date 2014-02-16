
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <threadlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <llist.h>

void _swapThread(_proc_state*, _proc_state*, uint32_t);
static void _threadExit(void);

void signal_handler(int i)
{
    printf("in timer\n");
    // New thread;
    struct itimerval tout_val;
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_interval.tv_usec = 0;
    tout_val.it_value.tv_sec = 0;
    tout_val.it_value.tv_usec = 250; // .250 milliseconds
    setitimer(ITIMER_REAL, &tout_val,0);
}

struct _stack_layout
{
    uint32_t eflags;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp; // popal/pushal skips this anyway
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
//    uint16_t es;
//    uint16_t cs;
//    uint16_t ss;
//    uint16_t ds;
//    uint16_t fs;
//    uint16_t gs;
    uint32_t _fill1; // Don't care.
    uint32_t eip;
} __attribute__((packed));

// Our implementation has some table(s) to keep track of threads.
// For now, our table is fixed in size, but this can easily be changed.
static thread thread_table[N_MAX_THREADS];
static int thread_table_index = -1;
static int currently_active_index = -1;

// Define methods here.

// Returns THREAD_SUCCSED on success.
// Returns <0 for all errors.
int thread_init(thread **th, thread_function f, void *data)
{
    // Do this ONCE:
    if (-1 == thread_table_index)
    {

        struct itimerval tv;
        tv.it_interval.tv_sec = 0;
        tv.it_interval.tv_usec = 0;
        tv.it_value.tv_sec = 0;
        tv.it_value.tv_usec = 250;
        // Set signal handler.
        signal(SIGALRM, signal_handler);
        setitimer(ITIMER_REAL, &tv, NULL);

        // The first entry is for the main outer thread.
        thread_table[0].state = THREAD_RUNNING;
        // We actually do not care about these values.
        thread_table[0].function = (thread_function)0xDEADBEEF;
        thread_table[0].function_data = (void*)0xDEADBEEF;
        memset(&thread_table[0].proc_state, 0xFE, sizeof(_proc_state));
        thread_table[0].stack_top = (uint32_t*)0xDEADBEEF;
        llist_init(&thread_table[0].toNotify);
        llist_init(&thread_table[0].waitingFor);

        int i;
        for (i = 1; i < N_MAX_THREADS; ++i)
        {
            thread_table[i].state = THREAD_UNUSED;
        }
        thread_table_index = 0;
        currently_active_index = 0;
    }

    // Now the real work:
    if (N_MAX_THREADS == thread_table_index)
    {
        fprintf(stderr, "Max threads %d reached.", N_MAX_THREADS);
        return -E_MAX_THREADS;
    }
    thread *tmp = &thread_table[++thread_table_index];

    // Allocate stack.
    void *stackptr = malloc(4096*10); // TODO define stack size
    if (NULL == stackptr)
    {
        return -E_MEM_ERROR;
    }
    
    // Successfully allocated object(s).
    thread_table[thread_table_index].state = THREAD_WAITING;
    llist_init(&thread_table[thread_table_index].toNotify);
    llist_init(&thread_table[thread_table_index].waitingFor);

    tmp->stack_top = (uint32_t*)((uint32_t)stackptr + 4096*10); // TODO define stack size
    tmp->function = f;
    tmp->function_data = data;

    // Init stack.
    // Give the ptr room to configure the top three 4 byte words.
    struct _stack_layout *stack = (struct _stack_layout*)((uint32_t)tmp->stack_top - sizeof(struct _stack_layout) - 12);
    memset(stack, 0, sizeof(struct _stack_layout));
    stack->eax = 0xDEADBEE1;
    stack->ebx = 0xDEADBEE2;
    stack->ecx = 0xDEADBEE3;
    stack->edx = 0xDEADBEE4;
    stack->ebp = 0xDEADBEE5;
    stack->edi = 0xDEADBEE6;
    stack->esi = 0xDEADBEE7;
    //stack->esp = (uint32_t)stack;
    stack->eip = (uint32_t)tmp->function;
    uint32_t *ptr = (uint32_t*)((uint32_t)tmp->stack_top - 12);
    ptr[0] = (uint32_t)_threadExit;
    ptr[1] = (uint32_t)data;

    // Set initial %esp.
    tmp->proc_state.esp = (uint32_t)stack;

    // Return to caller.
    *th = tmp;
    return THREAD_SUCCESS;
}

// Actually start execution of the thread. In other words, put it into the
// pool of currently running threads.
int thread_run(thread *th)
{
    th->state = THREAD_RUNNING;
    return THREAD_SUCCESS;
}

// PRIVATE function to switch threads.
static int thread_switch(thread *t1, thread *t2, uint32_t eip)
{
    _swapThread(&t1->proc_state, &t2->proc_state, eip);
    return THREAD_SUCCESS;
}

void printstats(void)
{
    int i;
    for (i = 0; i < 3; ++i)
    {
        int *data1 = (int*)(thread_table[i].proc_state.esp + 32);
        int *data2 = (int*)(thread_table[i].proc_state.esp + 36);
        int *data3 = (int*)(thread_table[i].proc_state.esp + 40);
        int *data4 = (int*)(thread_table[i].proc_state.esp + 44);
        int *data5 = (int*)(thread_table[i].proc_state.esp + 48);
        int *data6 = (int*)(thread_table[i].proc_state.esp + 52);
        if ((uint32_t)data1 > 0xe0000000)
        {
            printf("ESP[%d]: %08x\n", i, thread_table[i].proc_state.esp);
        }
        else
        {
            printf("ESP[%d]: %08x %08x %08x %08x %08x %08x %08x\n",
                    i, thread_table[i].proc_state.esp, *data1, *data2, *data3, *data4, *data5, *data6);
        }
    }
}

// Chooses a new thread to run from the active pool, in round robin fashion.
void thread_schedule(uint32_t eip, uint32_t esp)
{
    int i;
    for (i = currently_active_index + 1; i != currently_active_index; ++i)
    {
        if (THREAD_RUNNING == thread_table[i].state)
        {
            // Found suitable candidate.
            uint32_t *ptr = (uint32_t*)esp;
            *ptr = eip;
            int old = currently_active_index;
            currently_active_index = i;
            thread_switch(&thread_table[old], &thread_table[currently_active_index], eip);
            return; // Won't actually reach this.
        }
        if (N_MAX_THREADS == i-1)
            i = -1; // Wrap around the table.
    }
    // If we got here, there are no candidates. So just choose the originally running thread if we can.
    if (THREAD_RUNNING == thread_table[i].state)
    {
        uint32_t *ptr = (uint32_t*)esp;
        *ptr = eip;
        int old = currently_active_index;
        currently_active_index = i;
        thread_switch(&thread_table[old], &thread_table[currently_active_index], esp);
        return;
    }
    // If not, we will...just die.
    fprintf(stderr, "Unable to find a suitable thread to run! Exiting now.\n");
    exit(1);
}

// Mark the thread as finished, and schedule a new thread.
static void _threadExit(void)
{
    // Notify all who are waiting.
    thread *me = &thread_table[currently_active_index];
    while (llist_size(&me->toNotify) > 0)
    {
        // Get ptr to other thread (the one waiting on the thread about to exit).
        int index = llist_get_first(&me->toNotify)->data;
        thread *other = &thread_table[index];
        // Notify the other thread.
        _llist_node *itr = llist_find(&other->waitingFor, currently_active_index);
        llist_remove(&other->waitingFor, itr);
        if (0 == llist_size(&other->waitingFor))
        {
            other->state = THREAD_RUNNING; // The other thread can now run.
        }
        llist_remove_first(&me->toNotify);
    }
    thread_table[currently_active_index].state = THREAD_FINISHED;
    thread_yield();
}

void thread_join(thread *other)
{
    if (THREAD_FINISHED == other->state)
    {
        // No need to wait!
        return;
    }
    thread *me = &thread_table[currently_active_index];
    // Add to waitingFor and toNotify lists.
    llist_add_first(&me->waitingFor, other - thread_table);
    llist_add_first(&other->toNotify, me - thread_table);
    me->state = THREAD_WAITING;
    thread_yield();
}

