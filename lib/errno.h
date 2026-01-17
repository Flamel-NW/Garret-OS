#ifndef __LIB_ERRNO_H__
#define __LIB_ERRNO_H__

#include "defs.h"

// kernel error codes 
#define E_UNSPECIFIED       1       // Unspecified or unknown problem
#define E_BAD_PROC          2       // Process doesn't exist or otherwise
#define E_INVAL             3       // Invalid parameter
#define E_NO_MEM            4       // Request failed due to memory shortage
#define E_NO_FREE_PROC      5       // Attempt to create a new process beyond
#define E_FAULT             6       // Memory fault
#define E_SWAP_FAULT        7       // SWAP READ/WRITE fault
#define E_INVAL_ELF         8       // Invalid elf file
#define E_KILLED            9       // Process is killed
#define E_PANIC             10      // Panic Failure
#define E_TIMEOUT           11      // Timeout
#define E_TOO_BIG           12      // Argument is Too Big
#define E_NO_DEV            13      // No such Device
#define E_NA_DEV            14      // Device Not Available
#define E_BUSY              15      // Device/File is Busy
#define E_NOENT             16      // No Such File or Directory
#define E_IS_DIR            17      // Is a Directory
#define E_NOT_DIR           18      // Not a Directory
#define E_XDEV              19      // Cross Device-Link
#define E_UNIMP             20      // Unimplemented Feature
#define E_SEEK              21      // Illegal Seek
#define E_MAX_OPEN          22      // Too Many Files are Open
#define E_EXISTS            23      // File/Directory Already Exists
#define E_NOT_EMPTY         24      // Directory is Not Empty

#endif // __LIB_ERRNO_H__
