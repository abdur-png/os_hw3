#ifndef XV6_PROC_H
#define XV6_PROC_H

#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "spinlock.h"
#include "types.h"

// Segments in proc->gdt.
#define NSEGS 7

// Per-CPU state
struct cpu {
  uchar id;                  // Local APIC ID; index into cpus[] below
  struct context* scheduler; // swtch() here to enter scheduler
  struct taskstate ts;       // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS]; // x86 global descriptor table
  volatile uint started;     // Has the CPU started?
  int ncli;                  // Depth of pushcli nesting.
  int intena;                // Were interrupts enabled before pushcli?

  // Cpu-local storage variables; see below
  struct cpu* cpu;
  struct proc* proc; // The currently-running process.
};

extern struct cpu cpus[NCPU];
extern int ncpu;

// Per-CPU variables, holding pointers to the
// current cpu and to the current process.
// The asm suffix tells gcc to use "%gs:0" to refer to cpu
// and "%gs:4" to refer to proc.  seginit sets up the
// %gs segment register so that %gs refers to the memory
// holding those two variables in the local cpu's struct cpu.
// This is similar to how thread-local variables are implemented
// in thread libraries such as Linux pthreads.
extern struct cpu* cpu asm("%gs:0");   // &cpus[cpunum()]
extern struct proc* proc asm("%gs:4"); // cpus[cpunum()].proc

//  Saved registers for kernel context switches.
//  Don't need to save all the segment registers (%cs, etc),
//  because they are constant across kernel contexts.
//  Don't need to save %eax, %ecx, %edx, because the
//  x86 convention is that the caller has saved them.
//  Contexts are stored at the bottom of the stack they
//  describe; the stack pointer is the address of the context.
//  The layout of the context matches the layout of the stack in swtch.S
//  at the "Switch stacks" comment. Switch doesn't save eip explicitly,
//  but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate {
  UNUSED,
  EMBRYO,
  SLEEPING,
  RUNNABLE,
  RUNNING,
  ZOMBIE,
};

// Per-process state
struct proc {
  uint sz;                    // Size of process memory (bytes)
  pde_t* pgdir;               // Page table
  char* kstack;               // Bottom of kernel stack for this process
  enum procstate state;       // Process state
  int pid;                    // Process ID
  struct proc* parent;        // Parent process
  struct trapframe* tf;       // Trap frame for current syscall
  struct context* context;    // swtch() here to run process
  void* chan;                 // If non-zero, sleeping on chan
  int killed;                 // If non-zero, have been killed
  struct file* ofile[NOFILE]; // Open files
  struct inode* cwd;          // Current directory
  char name[16];              // Process name (debugging)
  int creation_time;          // The time when the process was created
  int termination_time;       // The time when the process was terminated
  int sleep_time;             // Time in SLEEPING state
  int ready_time;             // Time in RUNNABLE (or READY) state
  int running_time;           // Time in RUNNING state
};


int wait_stat(int *ctime, int *ttime, int *retime, int *rutime, int *stime) {
    struct proc *p;
    int havekids, pid;

    acquire(&ptable.lock);
    for(;;){
        // Scan through table looking for zombie children.
        havekids = 0;
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->parent != proc)
                continue;
            havekids = 1;
            if(p->state == ZOMBIE){
                // Found one.
                pid = p->pid;
                *ctime = p->creation_time;
                *ttime = p->termination_time;
                *retime = p->ready_time;
                *rutime = p->running_time;
                *stime = p->sleep_time;
                kfree(p->kstack);
                p->kstack = 0;
                freevm(p->pgdir);
                p->state = UNUSED;
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                release(&ptable.lock);
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if(!havekids || proc->killed){
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(proc, &ptable.lock);
    }
}


struct ptable {
  struct spinlock lock;
  struct proc proc[NPROC];
};

extern struct ptable ptable;

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

#endif // XV6_PROC_H