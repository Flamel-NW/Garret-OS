#ifndef __LIB_UNISTD_H__
#define __LIB_UNISTD_H__

// syscall number
#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_WAIT        3
#define SYS_EXEC        4
#define SYS_CLONE       5
#define SYS_YIELD       10
#define SYS_SLEEP       11
#define SYS_KILL        12
#define SYS_GETTIME     17
#define SYS_GETPID      18
#define SYS_BRK         19
#define SYS_MMAP        20
#define SYS_MUNMAP      21
#define SYS_SHMEM       22
#define SYS_PUTC        30
#define SYS_GPT         31

/* SYS_FORK flags */
#define CLONE_VM        0x00000001  // set if VM shared between processes
#define CLONE_THREAD    0x00000002  // thread group

#endif // __LIB_UNISTD_H__
