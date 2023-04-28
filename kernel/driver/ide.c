#include "ide.h"

#include "string.h"
#include "defs.h"


static byte ide[IDE_SIZE];

void read_ide_sects(void* dst, uint32_t src_sect, size_t num_sects) {
    memcpy(dst, &ide[src_sect * SECTION_SIZE], num_sects * SECTION_SIZE);
}

void write_ide_sects(const void* src, uint32_t dst_sect, size_t num_sects) {
    memcpy(&ide[dst_sect * SECTION_SIZE], src, num_sects * SECTION_SIZE);
}
