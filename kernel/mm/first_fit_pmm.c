#include "first_fit_pmm.h"

#include "memlayout.h"
#include "list.h"
#include "defs.h"
#include "assert.h"
#include "pmm.h"


static struct free_area g_free_area;


static void first_fit_init() {
    init_list(&g_free_area.free_list);
    g_free_area.num_free = 0;
}

static void first_fit_init_memmap(struct page* base, size_t n) {
    ASSERT(n > 0);
    for (struct page* p = base; p < base + n; p++) {
        ASSERT(!test_page_reserved(p));
        p->flags = p->property = p->ref = 0;
    }
    base->property = n;
    set_page_property(base);
    g_free_area.num_free += n;
    if (empty_list(&g_free_area.free_list)) {
        add_list(&g_free_area.free_list, g_free_area.free_list.next, &(base->list_link));
    } else {
        struct list* list = &g_free_area.free_list;
        while ((list = list->next) != &g_free_area.free_list) {
            struct page* page = list2page(list);
            if (base < page) {
                add_list(list->prev, list, &base->list_link);
                break;
            } else if (list->next == &g_free_area.free_list) {
                add_list(list, list->next, &base->list_link);
            }
        }
    }
}

static struct page* first_fit_alloc_pages(size_t n) {
    ASSERT(n > 0);
    if (n > g_free_area.num_free) 
        return NULL;
    struct page* page = NULL;
    struct list* list = &g_free_area.free_list;
    while ((list = list->next) != &g_free_area.free_list) {
        struct page* p = list2page(list);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page) {
        struct list* prev = page->list_link.prev;
        struct list* next = page->list_link.next;
        del_list(&(page->list_link));
        if (page->property > n) {
            struct page* p = page + n;
            p->property = page->property - n;
            set_page_property(p);
            add_list(prev, next, &p->list_link);
        }
        g_free_area.num_free -= n;
        clear_page_property(page);
    }
    return page;
}

static void first_fit_free_pages(struct page* base, size_t n) {
    ASSERT(n > 0);
    struct page* p = base;
    for ( ; p < base + n; p++) {
        ASSERT(!test_page_reserved(p) && !test_page_property(p));
        p->flags = p->ref = 0;
    }
    base->property = n;
    set_page_property(base);
    g_free_area.num_free += n;

    if (empty_list(&g_free_area.free_list)) {
        add_list(&g_free_area.free_list, g_free_area.free_list.next, &base->list_link);
    } else {
        struct list* list = &g_free_area.free_list;
        while ((list = list->next) != &g_free_area.free_list) {
            struct page* page = list2page(list);
            if (base < page) {
                add_list(list->prev, list, &base->list_link);
                break;
            } else if (list->next == &g_free_area.free_list){
                add_list(list, list->next, &base->list_link);
            }
        }
    }

    // 合并相邻空闲区域
    struct list* list = base->list_link.prev;
    if (list != &g_free_area.free_list) {
        p = list2page(list);
        if (p + p->property == base) {
            p->property += base->property;
            clear_page_property(base);
            del_list(&base->list_link);
            base = p;
        }
    }

    list = base->list_link.next;
    if (list != &g_free_area.free_list) {
        p = list2page(list);
        if (base + base->property == p) {
            base->property += p->property;
            clear_page_property(p);
            del_list(&p->list_link);
        }
    }
}

static size_t first_fit_count_free_pages(void) {
    return g_free_area.num_free;
}

static void basic_check() {
    struct page* p1 = NULL;
    struct page* p2 = NULL;
    struct page* p3 = NULL;

    ASSERT((p1 = alloc_page()) != NULL);
    ASSERT((p2 = alloc_page()) != NULL);
    ASSERT((p3 = alloc_page()) != NULL);

    ASSERT(p1 != p2 && p2 != p3 && p3 != p1);

    ASSERT(p1->ref == 0);
    ASSERT(p2->ref == 0);
    ASSERT(p3->ref == 0);

    ASSERT((uintptr_t) p1 < FREE_END_VA);
    ASSERT((uintptr_t) p2 < FREE_END_VA);
    ASSERT((uintptr_t) p3 < FREE_END_VA);

    struct free_area temp_free_area = g_free_area;
    init_list(&g_free_area.free_list);
    ASSERT(empty_list(&g_free_area.free_list));
    g_free_area.num_free = 0;

    ASSERT(alloc_page() == NULL);

    free_page(p1);
    free_page(p2);
    free_page(p3);
    ASSERT(g_free_area.num_free == 3);

    ASSERT((p1 = alloc_page()) != NULL);
    ASSERT((p2 = alloc_page()) != NULL);
    ASSERT((p3 = alloc_page()) != NULL);

    ASSERT(alloc_page() == NULL);

    free_page(p1);
    ASSERT(!empty_list(&g_free_area.free_list));

    struct page* p0;
    ASSERT((p0 = alloc_page()) == p1);
    ASSERT(alloc_page() == NULL);

    ASSERT(g_free_area.num_free == 0);
    g_free_area = temp_free_area;

    free_page(p0);
    free_page(p2);
    free_page(p3);
}

static void first_fit_check() {
    uint32_t count = 0;
    uint32_t total = 0;
    struct list* list = &g_free_area.free_list;
    while ((list = list->next) != &g_free_area.free_list) {
        struct page* p = list2page(list);
        ASSERT(test_page_property(p));
        count++;
        total += p->property;
    }
    ASSERT(total == count_free_pages());

    basic_check();

    struct page* p1 = alloc_pages(5);
    struct page* p2;
    struct page* p3;

    ASSERT(p1 != NULL);
    ASSERT(!test_page_property(p1));

    struct free_area temp_free_area = g_free_area;
    init_list(&g_free_area.free_list);
    ASSERT(empty_list(&g_free_area.free_list));
    ASSERT(alloc_page() == NULL);
    g_free_area.num_free = 0;

    free_pages(p1 + 2, 3);
    ASSERT(alloc_pages(4) == NULL);
    ASSERT(test_page_property(p1 + 2) && p1[2].property == 3);
    ASSERT((p2 = alloc_pages(3)) != NULL);
    ASSERT(alloc_page() == NULL);
    ASSERT(p1 + 2 == p2);

    p3 = p1 + 1;
    free_page(p1);
    free_pages(p2, 3);
    ASSERT(test_page_property(p1) && p1->property == 1);
    ASSERT(test_page_property(p2) && p2->property == 3);

    ASSERT((p1 = alloc_page()) == p3 - 1);
    free_page(p1);
    ASSERT((p1 = alloc_pages(2)) == p3 + 1);
    free_pages(p1, 2);

    free_page(p3);

    ASSERT((p1 = alloc_pages(5)) != NULL);
    ASSERT(alloc_page() == NULL);

    ASSERT(g_free_area.num_free == 0);
    g_free_area = temp_free_area;

    free_pages(p1, 5);

    list = &g_free_area.free_list;
    while ((list = list->next) != &g_free_area.free_list) {
        struct page* p = list2page(list);
        count--;
        total -= p->property;
    }
    ASSERT(count == 0);
    ASSERT(total == 0);
}

const struct pm_manager g_first_fit_pm_manager = {
    .name = "first_fit_pm_manager",
    .init = first_fit_init,
    .init_memmap = first_fit_init_memmap,
    .alloc_pages = first_fit_alloc_pages,
    .free_pages = first_fit_free_pages,
    .count_free_pages = first_fit_count_free_pages,
    .check = first_fit_check,
};
