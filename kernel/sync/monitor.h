#ifndef __KERNEL_SYNC_MONITOR_H__
#define __KERNEL_SYNC_MONITOR_H__

#include "sem.h"

// In [OS CONCEPT] 7.7 section, the accurate define and approximate implementation of MONITOR was introduced.
// INTRODUCTION:
//  Monitors were invented by C. A. R. Hoare and Per Brinch Hansen, and were first implemented in Brinch Hansen's
//  Concurrent Pascal language. Generally, a monitor is a language construct and the compiler usually enforces mutual exclusion. Compare this with semaphores, which are usually an OS construct.
// DEFNIE & CHARACTERISTIC:
//  A monitor is a collection of procedures, variables, and data structures grouped together.
//  Processes can call the monitor procedures but cannot access the internal data structures.
//  Only one process at a time may be be active in a monitor.
//  Condition variables allow for blocking and unblocking.
//     cv.wait() blocks a process.
//        The process is said to be waiting for (or waiting on) the condition variable cv.
//     cv.signal() (also called cv.notify) unblocks a process waiting for the condition variable cv.
//        When this occurs, we need to still require that only one process is active in the monitor. This can be done in several ways:
//            on some systems the old process (the one executing the signal) leaves the monitor and the new one enters
//            on some systems the signal must be the last statement executed inside the monitor.
//            on some systems the old process will block until the monitor is available again.
//            on some systems the new process (the one unblocked by the signal) will remain blocked until the monitor is available again.
//   If a condition variable is signaled with nobody waiting, the signal is lost. Compare this with semaphores, in which a signal will allow a process that executes a wait in the future to no block.
//   You should not think of a condition variable as a variable in the traditional sense.
//     It does not have a value.
//     Think of it as an object in the OOP sense.
//     It has two methods, wait and signal that manipulate the calling process.
// IMPLEMENTATION:
//   monitor mt {
//     ----------------variable------------------
//     semaphore mutex;
//     semaphore next;
//     i32 next_count;
//     condvar {i32 count, sempahore sem}  cv[N];
//     other variables in mt;
//     --------condvar wait/signal---------------
//     cond_wait (cv) {
//         cv.count ++;
//         if(mt.next_count>0)
//            signal(mt.next)
//         else
//            signal(mt.mutex);
//         wait(cv.sem);
//         cv.count --;
//      }
//
//      cond_signal(cv) {
//          if(cv.count>0) {
//             mt.next_count ++;
//             signal(cv.sem);
//             wait(mt.next);
//             mt.next_count--;
//          }
//       }
//     --------routines in monitor---------------
//     routineA_in_mt () {
//        wait(mt.mutex);
//        ...
//        real body of routineA
//        ...
//        if(next_count>0)
//            signal(mt.next);
//        else
//            signal(mt.mutex);
//     }
//

struct monitor;

struct cond {
    struct semaphore sem;   // the sem semaphore is used to down the waiting proc, and the signaling proc should up the waiting proc
    i32 count;              // the number of waiters on cond var
    struct monitor* owner;  // the owner(monitor) of this cond var
};

struct monitor {
    struct semaphore mutex; // the mutex lock for going into the routines in monitor, should be initialized to 1;
    // the next semaphore is used to down the signaling proc itself, and the other OR wakeuped waiting proc should wake up the signaling proc.
    struct semaphore next; 
    i32 next_count;         // the number of sleeped signaling proc
    struct cond* cond;      // the cond var in monitor
};

// Initialize variables in monitor.
void init_monitor(struct monitor* monitor, u64 num_cond);

// Unlock one of threads waiting on the cond var
void signal_cond(struct cond* cond);

// Suspend calling thread on a condition variable waiting for condition atomically unlock mutex in monitor,
// and suspends calling threads on cond var after waking up locks mutex
void wait_cond(struct cond* cond);

void test_phi_sem();
void test_phi_cond();

#endif // __KERNEL_SYNC_MONITOR_H__