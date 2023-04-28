#include "best_fit_pmm.h"

#include "memlayout.h"
#include "list.h"
#include "defs.h"
#include "assert.h"
#include "page.h"
#include "pmm.h"
#include "stdio.h"


static struct free_area g_best_fit_free_area;


static void best_fit_init() {
    init_list(&g_best_fit_free_area.free_list);
    g_best_fit_free_area.num_free = 0;
}

static void best_fit_init_memmap(struct page* base, size_t n) {
    ASSERT(n > 0);
    for (struct page* p = base; p < base + n; p++) {
        ASSERT(!(p->flags & PAGE_RESERVED));
        p->flags = p->ref = p->property = 0;
    }
    base->flags |= PAGE_PROPERTY;
    base->property = n;
    add_list(&g_best_fit_free_area.free_list, g_best_fit_free_area.free_list.next, &base->free_list_link);
    g_best_fit_free_area.num_free += n;
}

static struct page* best_fit_alloc_pages(size_t n) {
    ASSERT(n > 0);
    if (n > g_best_fit_free_area.num_free) 
        return NULL;
    struct list* list = &g_best_fit_free_area.free_list;
    struct page* page = NULL;
    uint32_t min_property = UINT32_MAX;
    while ((list = list->next) != &g_best_fit_free_area.free_list) {
        struct page* p = LIST2PAGE(free_list_link, list);
        if (p->property >= n && p->property < min_property) {
            page = p;
            min_property = p->property;
        }
    }
    if (page) {
        del_list(&(page->free_list_link));
        if (page->property > n) {
            struct page* p = page + n;
            p->property = page->property - n;
            p->flags |= PAGE_PROPERTY;
            add_list(page->free_list_link.prev, page->free_list_link.next, &p->free_list_link);
        }
        g_best_fit_free_area.num_free -= n;
        page->flags &= ~PAGE_PROPERTY;
        page->property = 0;
    }
    return page;
}

static void best_fit_free_pages(struct page* base, size_t n) {
    ASSERT(n > 0);
    struct page* p = base;
    for ( ; p < base + n; p++) {
        ASSERT((p->flags & (PAGE_RESERVED | PAGE_PROPERTY)) == 0);
        p->flags = p->ref = 0;
    }
    base->property = n;
    base->flags |= PAGE_PROPERTY;
    g_best_fit_free_area.num_free += n;

    struct list* list = &g_best_fit_free_area.free_list;
    while (list->next < &base->free_list_link && list->next != &g_best_fit_free_area.free_list) 
        list = list->next;
    add_list(list, list->next, &base->free_list_link);

    // 合并相邻空闲区域
    list = base->free_list_link.prev;
    if (list != &g_best_fit_free_area.free_list) {
        p = LIST2PAGE(free_list_link, list);
        if (p + p->property == base) {
            p->property += base->property;
            base->flags &= ~PAGE_PROPERTY;
            base->property = 0;
            del_list(&base->free_list_link);
            base = p;
        }
    }

    list = base->free_list_link.next;
    if (list != &g_best_fit_free_area.free_list) {
        p = LIST2PAGE(free_list_link, list);
        if (base + base->property == p) {
            base->property += p->property;
            p->flags &= ~PAGE_PROPERTY;
            p->property = 0;
            del_list(&p->free_list_link);
        }
    }
}

static size_t best_fit_count_free_pages(void) {
    return g_best_fit_free_area.num_free;
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

    struct free_area temp_free_area = g_best_fit_free_area;
    init_list(&g_best_fit_free_area.free_list);
    ASSERT(empty_list(&g_best_fit_free_area.free_list));
    g_best_fit_free_area.num_free = 0;

    ASSERT(alloc_page() == NULL);

    free_page(p1);
    free_page(p2);
    free_page(p3);
    ASSERT(g_best_fit_free_area.num_free == 3);

    ASSERT((p1 = alloc_page()) != NULL);
    ASSERT((p2 = alloc_page()) != NULL);
    ASSERT((p3 = alloc_page()) != NULL);

    ASSERT(alloc_page() == NULL);

    free_page(p1);
    ASSERT(!empty_list(&g_best_fit_free_area.free_list));

    struct page* p0;
    ASSERT((p0 = alloc_page()) == p1);
    ASSERT(alloc_page() == NULL);

    ASSERT(g_best_fit_free_area.num_free == 0);
    g_best_fit_free_area = temp_free_area;

    free_page(p0);
    free_page(p2);
    free_page(p3);
}

static void check_best_fit() {
    uint32_t count = 0;
    uint32_t total = 0;
    struct list* list = &g_best_fit_free_area.free_list;
    while ((list = list->next) != &g_best_fit_free_area.free_list) {
        struct page* p = LIST2PAGE(free_list_link, list);
        ASSERT((p->flags & PAGE_PROPERTY) != 0);
        count++;
        total += p->property;
    }
    ASSERT(total == count_free_pages());

    basic_check();

    struct page* p1 = alloc_pages(5);
    struct page* p2;
    struct page* p3;

    ASSERT(p1 != NULL);
    ASSERT((p1->flags & PAGE_PROPERTY) == 0);

    struct free_area temp_free_area = g_best_fit_free_area;
    init_list(&g_best_fit_free_area.free_list);
    ASSERT(empty_list(&g_best_fit_free_area.free_list));
    ASSERT(alloc_page() == NULL);
    g_best_fit_free_area.num_free = 0;

    free_pages(p1 + 2, 3);
    ASSERT(!alloc_pages(4));
    ASSERT((p1[2].flags & PAGE_PROPERTY) != 0 && p1[2].property == 3);
    ASSERT((p2 = alloc_pages(3)));
    ASSERT(alloc_page() == NULL);
    ASSERT(p1 + 2 == p2);

    p3 = p1 + 1;
    free_page(p1);
    free_pages(p2, 3);
    ASSERT((p1->flags & PAGE_PROPERTY) != 0 && p1->property == 1);
    ASSERT((p2->flags & PAGE_PROPERTY) != 0 && p2->property == 3);

    ASSERT((p1 = alloc_page()) == p3 - 1);
    free_page(p1);
    ASSERT((p1 = alloc_pages(2)) == p3 + 1);
    free_pages(p1, 2);

    free_page(p3);

    ASSERT((p1 = alloc_pages(5)) != NULL);
    ASSERT(alloc_page() == NULL);

    ASSERT(g_best_fit_free_area.num_free == 0);
    g_best_fit_free_area = temp_free_area;

    free_pages(p1, 5);

    list = &g_best_fit_free_area.free_list;
    while ((list = list->next) != &g_best_fit_free_area.free_list) {
        struct page* p = LIST2PAGE(free_list_link, list);
        count--;
        total -= p->property;
    }
    ASSERT(count == 0);
    ASSERT(total == 0);
}

const struct pm_manager g_best_fit_pm_manager = {
    .name = "best_fit_pm_manager",
    .free_area = &g_best_fit_free_area,
    .init = best_fit_init,
    .init_memmap = best_fit_init_memmap,
    .alloc_pages = best_fit_alloc_pages,
    .free_pages = best_fit_free_pages,
    .count_free_pages = best_fit_count_free_pages,
    .check = check_best_fit,
};
