
# Time for x86 assembly. I.e. hammer time.

.globl _swapThread
#.globl _launchPad
.globl thread_yield
.globl cotter

# Takes two 32-bit pointers to struct _proc_state. The first is the current running
#0 thread, the second is the thread to switch to. The third argument is the old esp
# we really want to save.
_swapThread:
    pushl %ebp
    movl %esp, %ebp
    # Change esp
#movl 16(%ebp), %esp

    # Push registers onto current stack.
    pushal
#    pushw %es
#    pushw %ss
#    pushw %ds
#    pushw %fs
#    pushw %gs
    pushfl # EFLAGS

    # Fix return address.
#    movl 16(%ebp), %eax
#    movl %eax, 4(%ebp)
    # Swap stacks here.
    movl 8(%ebp), %eax
    movl 12(%ebp), %ebx
#    movl 16(%ebp), %ecx
    movl %esp, 0(%eax) # Only save old %esp.
    movl 0(%ebx), %esp # Fetch new %esp.

    # Now unwind.
    popfl
#    popw %gs
#    popw %fs
#    popw %ds
#    popw %ss
#    popw %es
    popal

    popl %ebp
    ret

# Everything thread's first execution point will be here.
# This function sets up the stack so that when it returns, it will run the actual
# thread's desired function correctly.
# First arg: 32-bits void* - thread functino data
# Second arg: 32-bit function pointer
# Third arg: 32-bit uint32_t* - top of stack pointer
/*_launchPad:
    # The very next instruction is the first instruction for this thread!
    movl %esp, %ebp
    movl 12(%ebp), %esp # New stack location
    addl $-12, %esp     # 3 32-bit objects on stack
    movl 8(%ebp), %eax
    movl %eax, 0(%esp) # Function to run
    movl $_threadExit, %eax
    movl %eax, 4(%esp)  # When the thread returns, it will go to _threadExit
    movl 4(%ebp), %eax
    movl %eax, 8(%esp)       # Function data (the void*)
    ret
  */  

# This function does NOT return. We will save the return address  (i.e. the caller of this function).
thread_yield:
#pushl %ebp
#    movl %esp, %ebp
    pushl %esp
    pushl 4(%esp) # void thread_schedule(uint32_t eip, uint32_t esp);
    call thread_schedule
    ret # NEVER GETS HERE!

