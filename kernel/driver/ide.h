#ifndef __KERNEL_DRIVER_IDE_H__
#define __KERNEL_DRIVER_IDE_H__

#include "defs.h"

#define SECTION_SIZE    512
#define IDE_SECTIONS    64

#define IDE_SIZE        (SECTION_SIZE * IDE_SECTIONS)

void read_ide_sects(void* dst, u32 src_sect, u64 num_sects);
void write_ide_sects(const void* src, u32 dst_sect, u64 num_sects);

#endif // __KERNEL_DRIVER_IDE_H__
