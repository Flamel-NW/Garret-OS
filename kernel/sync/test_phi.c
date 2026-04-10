#include "sem.h"
#include "monitor.h"

#include "proc.h"
#include "string.h"

#define NUM_PHI     5                               // 哲学家数量
#define THINKING    0                               // 哲学家正在思考
#define HUNGRY      1                               // 哲学家想取得叉子 
#define EATING      2                               // 哲学家正在吃面
#define TIMES       4                               // 吃4次
#define SLEEP_TIME  10

// phi: 哲学家号码从0 ~ NUM_PHI - 1

static inline i32 left(i32 phi)  {          // phi 的左邻号码
    return (phi + NUM_PHI - 1) % NUM_PHI;
}

static inline i32 right(i32 phi) {          // phi 的右邻号码
    return (phi + 1) % NUM_PHI;
}

//-----------------philosopher problem using monitor ------------
// PSEUDO CODE :philosopher problem using semaphore
// system DINING_PHILOSOPHERS

// VAR
// me:    semaphore, initially 1;                    # for mutual exclusion 
// s[5]:  semaphore s[5], initially 0;               # for synchronization 
// pflag[5]: {THINK, HUNGRY, EAT}, initially THINK;  # philosopher flag 

// # As before, each philosopher is an endless cycle of thinking and eating.

// procedure philosopher(i)
//   {
//     while TRUE do
//      {
//        THINKING;
//        take_chopsticks(i);
//        EATING;
//        drop_chopsticks(i);
//      }
//   }

// # The take_chopsticks procedure involves checking the status of neighboring 
// # philosophers and then declaring one's own intention to eat. This is a two-phase 
// # protocol; first declaring the status HUNGRY, then going on to EAT.

// procedure take_chopsticks(i)
//   {
//     DOWN(me);               # critical section 
//     pflag[i] := HUNGRY;
//     test[i];
//     UP(me);                 # end critical section 
//     DOWN(s[i])              # Eat if enabled 
//    }

// void test(i)                # Let phil[i] eat, if waiting 
//   {
//     if ( pflag[i] == HUNGRY
//       && pflag[i-1] != EAT
//       && pflag[i+1] != EAT)
//        then
//         {
//           pflag[i] := EAT;
//           UP(s[i])
//          }
//     }


// # Once a philosopher finishes eating, all that remains is to relinquish the 
// # resources---its two chopsticks---and thereby release waiting neighbors.

// void drop_chopsticks(i32 i)
//   {
//     DOWN(me);                # critical section 
//     test(i-1);               # Let phil. on left eat if possible 
//     test(i+1);               # Let phil. on rght eat if possible 
//     UP(me);                  # up critical section 
//    }
//---------- philosophers problem using semaphore ----------------------


static i32 g_phi_state_sem[NUM_PHI];           // 记录每个人状态的数组
// 信号量是一个特殊的整型变量
static struct semaphore g_phi_mutex;           // 临界区互斥
static struct semaphore g_phi_sem[NUM_PHI];    // 每个哲学家一个信号量

static void phi_test_sem(i32 phi) {
    if (g_phi_state_sem[phi] == HUNGRY && 
            g_phi_state_sem[left(phi)] != EATING &&
            g_phi_state_sem[right(phi)] != EATING) {
        putstr("phi_test_sem: g_phi_state_sem["); put_i64(phi, 10); putstr("] will eating.\n");
        g_phi_state_sem[phi] = EATING;
        putstr("phi_test_sem: up g_phi_sem["); put_i64(phi, 10); putstr("].\n");
        up(&g_phi_sem[phi]);
    }
}

static void phi_take_forks_sem(i32 phi) {
    down(&g_phi_mutex);                 // 进入临界区
    g_phi_state_sem[phi] = HUNGRY;      // 记录下哲学家i饥饿的事实
    phi_test_sem(phi);                  // 试图得到两只叉子
    up(&g_phi_mutex);                   // 离开临界区
    down(&g_phi_sem[phi]);              // 如果得不到叉子就阻塞
}

static void phi_put_forks_sem(i32 phi) {
    down(&g_phi_mutex);                 // 进入临界区
    g_phi_state_sem[phi] = THINKING;    // 哲学家进餐结束
    phi_test_sem(left(phi));            // 看一下左邻居现在是否能进餐
    phi_test_sem(right(phi));           // 看一下右邻居现在是否能进餐
    up(&g_phi_mutex);                   // 离开临界区
}

static i32 phi_func_sem(void* arg) {
    i32 phi = (u64) arg;
    putstr("I am No."); put_i64(phi, 10); putstr(" g_phi_sem\n");

    for (i32 i = 0; i < TIMES; i++) {

        putstr("Iter "); put_i64(i, 10);
        putstr(", No."); put_i64(phi, 10); 
        putstr(" g_phi_sem is thinking\n"); // 哲学家正在思考

        do_sleep(SLEEP_TIME);
        phi_take_forks_sem(phi);            // 需要两只叉子，或者阻塞

        putstr("Iter "); put_i64(i, 10);
        putstr(", No."); put_i64(phi, 10); 
        putstr(" g_phi_sem is eating\n");   // 哲学家正在进食

        do_sleep(SLEEP_TIME);
        phi_put_forks_sem(phi);             // 把两把叉子同时放回桌子
    }

    putstr("No."); put_i64(phi, 10); putstr(" g_phi_sem quit\n");
    return 0;
}

//-----------------philosopher problem using monitor ------------
//PSEUDO CODE :philosopher problem using monitor
// monitor dp
// {
//  enum {thinking, hungry, eating} state[5];
//  condition self[5];
//
//  void pickup(int i) {
//      state[i] = hungry;
//      if ((state[(i+4)%5] != eating) && (state[(i+1)%5] != eating)) {
//        state[i] = eating;
//      else
//         self[i].wait();
//   }
//
//   void putdown(int i) {
//      state[i] = thinking;
//      if ((state[(i+4)%5] == hungry) && (state[(i+3)%5] != eating)) {
//          state[(i+4)%5] = eating;
//          self[(i+4)%5].signal();
//      }
//      if ((state[(i+1)%5] == hungry) && (state[(i+2)%5] != eating)) {
//          state[(i+1)%5] = eating;
//          self[(i+1)%5].signal();
//      }
//   }
//
//   void init() {
//      for (int i = 0; i < 5; i++)
//         state[i] = thinking;
//   }
// }
//
//---------- philosophers using monitor (condition variable) ----------------------

static i32 g_phi_state_cond[NUM_PHI];
static struct monitor g_phi_monitor;

static void phi_test_cond(i32 phi) {
    if (g_phi_state_cond[phi] == HUNGRY &&
            g_phi_state_cond[left(phi)] != EATING &&
            g_phi_state_cond[right(phi)] != EATING) {
        putstr("phi_test_cond: g_phi_state_cond["); put_i64(phi, 10); putstr("] will eating.\n");
        g_phi_state_cond[phi] = EATING;
        putstr("phi_test_cond: signal g_phi_monitor->cond["); put_i64(phi, 10); putstr("].\n");
        signal_cond(&g_phi_monitor.cond[phi]);
    }
}

static void phi_take_forks_cond(i32 phi) {
    down(&(g_phi_monitor.mutex));
    //--------into routine in monitor--------------
    g_phi_state_cond[phi] = HUNGRY;             // I am hungry
    phi_test_cond(phi);                         // try to get fork
    if (g_phi_state_cond[phi] == HUNGRY)
        wait_cond(&g_phi_monitor.cond[phi]);
    //--------into routine in monitor--------------
    if (g_phi_monitor.next_count > 0)
        up(&(g_phi_monitor.next));
    else
        up(&(g_phi_monitor.mutex));
}

static void phi_put_forks_cond(i32 phi) {
    down(&(g_phi_monitor.mutex));
    //--------into routine in monitor--------------
    g_phi_state_cond[phi] = THINKING;           // I ate over
    phi_test_cond(left(phi));                   // test left and
    phi_test_cond(right(phi));                  // right neighbors
    //--------into routine in monitor--------------
    if (g_phi_monitor.next_count > 0)
        up(&(g_phi_monitor.next));
    else
        up(&(g_phi_monitor.mutex));
}

static i32 phi_func_cond(void* arg) {
    i32 phi = (u64) arg;
    putstr("I am No."); put_i64(phi, 10); putstr(" g_phi_cond\n");

    for (i32 i = 0; i < TIMES; i++) {

        putstr("Iter "); put_i64(i, 10);
        putstr(", No."); put_i64(phi, 10); 
        putstr(" g_phi_cond is thinking\n");    // 哲学家正在思考

        do_sleep(SLEEP_TIME);
        phi_take_forks_cond(phi);               // 需要两只叉子，或者阻塞

        putstr("Iter "); put_i64(i, 10);
        putstr(", No."); put_i64(phi, 10); 
        putstr(" g_phi_cond is eating\n");      // 哲学家正在进食

        do_sleep(SLEEP_TIME);
        phi_put_forks_cond(phi);                // 把两把叉子同时放回桌子
    }

    putstr("No."); put_i64(phi, 10); putstr(" g_phi_cond quit\n");
    return 0;
}

static inline void test_phi_end(i32* pids) {
    for (i32 phi = 0; phi < NUM_PHI; phi++)
        ASSERT(do_wait(pids[phi], NULL) == 0);
}

void test_phi_sem() {
    init_sem(&g_phi_mutex, 1);
    i32 pids[NUM_PHI];
    for (i32 phi = 0; phi < NUM_PHI; phi++) {
        g_phi_state_sem[phi] = THINKING;
        init_sem(&g_phi_sem[phi], 0);
        pids[phi] = kernel_proc(phi_func_sem, (void*)(u64) phi, 0);
        if (pids[phi] < 0) {
            putstr("create No."); put_i64(phi, 10); putstr(" phi_func_sem failed.\n");
            PANIC("");
        }
        strcpy(get_proc(pids[phi])->name, "phi_proc_sem");
    }
    test_phi_end(pids);
}

void test_phi_cond() {
    init_monitor(&g_phi_monitor, NUM_PHI);
    i32 pids[NUM_PHI];
    for (i32 phi = 0; phi < NUM_PHI; phi++) {
        g_phi_state_cond[phi] = THINKING;
        pids[phi] = kernel_proc(phi_func_cond, (void*)(u64) phi, 0);
        if (pids[phi] < 0) {
            putstr("create No."); put_i64(phi, 10); putstr(" phi_func_cond failed.\n");
            PANIC("");
        }
        strcpy(get_proc(pids[phi])->name, "phi_proc_cond");
    }
    test_phi_end(pids);
}
