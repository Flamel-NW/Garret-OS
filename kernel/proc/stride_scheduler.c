#include "stride_scheduler.h"

#include "proc.h"

#define BIG_STRIDE (1 << 30)

// The compare function for two skew_heap's and the corresponding processes
static i32 stride_comp_fn(void* left, void* right) {
    struct pcb* left_proc = LIST2PCB(run_pool, left);
    struct pcb* right_proc = LIST2PCB(run_pool, right);

    i32 ret = left_proc->stride - right_proc->stride;
    return (ret > 0) - (ret < 0);
}

// stride_init initializes the run queue with correct assignment for member variables, including:
// - run_list: should be a empty list after initialization.
// - run_pool: NULL
// - num_proc: 0
// - max_time_slice: no need here, the variable would be assigned by the caller.
static void stride_init(struct run_queue* run_queue) {
    init_list(&(run_queue->run_list));
    run_queue->run_pool = NULL;
    run_queue->num_proc = 0;
}

// stride_enqueue inserts the process into the run queue. The procedure should verify/initialize the
// relevant members of process, and then put the run_pool node into the queue (since we use priority
// queue here). The procedure should also update the meta date in run_queue structure

// proc->time_slice denotes the time slices allocation for the process, which should set to
// run_queue->max_time_slice
static void stride_enqueue(struct run_queue* run_queue, struct pcb* proc) {
    run_queue->run_pool = add_skew_heap(run_queue->run_pool, &(proc->run_pool), stride_comp_fn);
    if (!proc->time_slice || proc->time_slice > run_queue->max_time_slice)
        proc->time_slice = run_queue->max_time_slice;
    proc->run_queue = run_queue;
    run_queue->num_proc++;
}

// stride_dequeue removes the process from the run queue, the operation would be finished by the
// del_skew_heap operations. Remember to update the run queue structure.
static void stride_dequeue(struct run_queue* run_queue, struct pcb* proc) {
    ASSERT(proc->run_queue == run_queue && run_queue->num_proc > 0);
    run_queue->run_pool = del_skew_heap(run_queue->run_pool, &(proc->run_pool), stride_comp_fn);
    run_queue->num_proc--;
}

// stride_pick_next pick the element from the run queue, with the minimum value of stride, and returns
// the corresponding process pointer. The process pointer would be calculated by macro LIST2PCB, see
// kernel/proc/proc.h for definition. Return NULL if there is no process in the queue

// When one proc structure is selected, remember to update the stride property of the proc.
// (stride += BIG_STRIDE / priority)
static struct pcb* stride_pick_next(struct run_queue* run_queue) {
    if (!run_queue->run_pool)
        return NULL;
    struct pcb* proc = LIST2PCB(run_pool, run_queue->run_pool);
    proc->stride += BIG_STRIDE / proc->priority;
    return proc;
}

// stride_proc_tick works with the tick event of current process. You should check whether the time
// slices for current process is exhausted and update the pcb. proc->time_slice denotes the time 
// slices left for current process. proc->need_reschedule is the flag variable for process switching
static void stride_proc_tick(struct run_queue* run_queue, struct pcb* proc) {
    if (proc->time_slice > 0)
        proc->time_slice--;
    if (!proc->time_slice)
        proc->need_reschedule = true;
}

struct scheduler g_stride_scheduler = {
    .name = "stride_scheduler",
    .init = stride_init,
    .enqueue = stride_enqueue,
    .dequeue = stride_dequeue,
    .pick_next = stride_pick_next,
    .proc_tick = stride_proc_tick,
};
