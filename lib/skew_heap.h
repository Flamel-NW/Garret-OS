#ifndef __LIB_SKEW_HEAP_H__
#define __LIB_SKEW_HEAP_H__

#include "defs.h"

struct skew_heap {
    struct skew_heap* parent;
    struct skew_heap* left;
    struct skew_heap* right;
};

typedef i32 (*comp_fn_t) (void *left, void *right);

static inline void init_skew_heap(struct skew_heap* skew_heap) {
    skew_heap->parent = skew_heap->left = skew_heap->right = NULL;
}

static inline struct skew_heap* merge_skew_heap(struct skew_heap* left, struct skew_heap* right, comp_fn_t comp_fn) {
    if (!left) return right;
    if (!right) return left;

    struct skew_heap* left_child;
    struct skew_heap* right_child;

    if (comp_fn(left, right) == -1) {
        right_child = left->left;
        left_child = merge_skew_heap(left->right, right, comp_fn);
        
        left->left = left_child;
        left->right = right_child;
        if (left_child) left_child->parent = left;

        return left;
    } else {
        right_child = right->left;
        left_child = merge_skew_heap(left, right->right, comp_fn);
        
        right->left = left_child;
        right->right = right_child;

        if (left_child) 
            left_child->parent = right;

        return right;
    }
}

static inline struct skew_heap* add_skew_heap(struct skew_heap* left, struct skew_heap* right, comp_fn_t comp_fn) {
    init_skew_heap(right);
    return merge_skew_heap(left, right, comp_fn);
}

static inline struct skew_heap* del_skew_heap(struct skew_heap* left, struct skew_heap* right, comp_fn_t comp_fn) {
    struct skew_heap* parent = right->parent;
    struct skew_heap* replacement = merge_skew_heap(right->left, right->right, comp_fn);
    if (replacement)
        replacement->parent = parent;
    if (parent) {
        if (parent->left == right)
            parent->left = replacement;
        else
            parent->right = replacement;
        return left;
    } else {
        return replacement;
    }
}

#endif // __LIB_SKEW_HEAP_H__