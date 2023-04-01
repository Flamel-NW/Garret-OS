#ifndef __LIB_LIST_H__
#define __LIB_LIST_H__

#include "defs.h"

struct list {
    struct list* prev;
    struct list* next;
};

static inline void init_list(struct list* list) {
    list->prev = list->next = list;
}

static inline bool empty_list(struct list* list) {
    return list == list->prev && list == list->next;
}

// add_list - Insert a new entry between tow known consecutive entries
static inline void add_list(struct list* prev, struct list* next, 
        struct list* entry) {
    prev->next = next->prev = entry;
    entry->prev = prev;
    entry->next = next;
}

// del_list - Delete a list entry by making the prev/next entries point to each other.
static inline void del_list(struct list* entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

#endif // __LIB_LIST_H__
