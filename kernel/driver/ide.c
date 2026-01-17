#include "ide.h"

#include "string.h"
#include "defs.h"


static u8 ide[IDE_SIZE];

void read_ide_sects(void* dst, u32 src_sect, u64 num_sects) {
    memcpy(dst, &ide[src_sect * SECTION_SIZE], num_sects * SECTION_SIZE);
}

void write_ide_sects(const void* src, u32 dst_sect, u64 num_sects) {
    memcpy(&ide[dst_sect * SECTION_SIZE], src, num_sects * SECTION_SIZE);
}
