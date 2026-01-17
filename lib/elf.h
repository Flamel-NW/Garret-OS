#ifndef __LIB_ELF_H__
#define __LIB_ELF_H__

#include "defs.h"

#define ELF_MAGIC   0x464C457FU

// file header
struct elf_header {
    u32 e_magic;       // must equal ELF_MAGIC
    u8 e_elf[12];
    u16 e_type;        // 1=relocatable, 2=executable, 3=shared object, 4=core image
    u16 e_machine;     // 3=x86, 4=68k, etc.
    u32 e_version;     // file version, always 1
    u64 e_entry;       // entry point if executable
    u64 e_phoff;       // file position of program header or 0
    u64 e_shoff;       // file position of section header or 0
    u32 e_flags;       // architecture-specific flags, usually 0
    u32 e_phentsize;   // sizeof an entry in program header or 0
    u16 e_phnum;       // number of entries in section header
    u16 e_shentsize;   // size of an entry in section header
    u16 e_shnum;       // number of entries in section header or 0
    u16 e_shstrndx;    // section number that contains section name strings
};

#define ELF_PT_LOAD 1       // values for prog_header::p_type

// flag bits for prog_header::p_flags
#define ELF_PF_X    1       
#define ELF_PF_W    2       
#define ELF_PF_R    4       

// program section header
struct prog_header {
    u32 p_type;        // loadable code or data, dynamic linking info, etc.
    u32 p_flags;       // read/write/execute bits
    u64 p_offset;      // file offset of segment
    u64 p_va;          // virtual address to map segment
    u64 p_pa;          // physical address, not used
    u64 p_file_size;   // size of segment in file
    u64 p_mem_size;    // size of segment in memory (bigger if contains bss)
    u64 p_align;       // required alignment, invariably hardware page size
};


#endif // __LIB_ELF_H__