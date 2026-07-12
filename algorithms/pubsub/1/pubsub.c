/**
 * @file pubsub.c
 * @brief 发布-订阅模块实现（基于环形缓冲区）
 * @note  采用方案一：回调使用订阅上下文，发布数据（args）仅用于触发。
 *        临界区保护使用 __disable_irq() / __enable_irq()，适用于 Keil/ARMCC。
 */

#include "pubsub.h"
#include <string.h>

/*==================== 临界区保护（Keil 环境下） ====================*/

/**
 * @brief 进入临界区（关闭所有中断）
 * @note  保护共享资源（队列和事件表）不被中断打断。
 *        如果使用了 RTOS，可替换为互斥锁或关调度。
 */
static inline void ps_enter_critical(void) {
    __disable_irq();   // ARMCC 内置函数，关闭全局中断
}

/**
 * @brief 退出临界区（恢复中断）
 */
static inline void ps_exit_critical(void) {
    __enable_irq();    // 恢复全局中断
}

/*==================== 事件订阅者管理 ====================*/

/**
 * @struct subscriber_t
 * @brief 单个订阅者信息
 */
typedef struct {
    ps_callback_t cb;   ///< 回调函数指针
    void *args;         ///< 订阅时传入的上下文指针（方案一：该指针传给回调）
} subscriber_t;

/**
 * @struct event_t
 * @brief 一个事件及其订阅者列表
 */
typedef struct {
    int event_id;                       ///< 事件 ID
    subscriber_t subscribers[MAX_SUBSCRIBERS]; ///< 订阅者数组
    int count;                          ///< 当前订阅者数量
} event_t;

static event_t s_events[MAX_EVENTS];    ///< 所有已注册事件的静态数组
static int s_event_count = 0;           ///< 当前已注册的事件种类数

/**
 * @brief 根据事件 ID 查找已存在的事件
 * @param event_id 事件 ID
 * @return 事件指针，未找到返回 NULL
 */
static event_t* find_event(int event_id) {
    for (int i = 0; i < s_event_count; i++) {
        if (s_events[i].event_id == event_id) {
            return &s_events[i];
        }
    }
    return NULL;
}

/**
 * @brief 查找或创建事件（仅在订阅时调用）
 * @param event_id 事件 ID
 * @return 事件指针，若事件总数已达 MAX_EVENTS 则返回 NULL
 */
static event_t* find_or_create_event(int event_id) {
    event_t *evt = find_event(event_id);
    if (evt) return evt;
    
    if (s_event_count < MAX_EVENTS) {
        evt = &s_events[s_event_count++];
        evt->event_id = event_id;
        evt->count = 0;
        return evt;
    }
    return NULL;
}

/*==================== 环形缓冲区（消息队列） ====================*/

/**
 * @struct msg_t
 * @brief 队列中的一条消息（事件发布记录）
 */
typedef struct {
    int event_id;   ///< 事件 ID
    void *args;     ///< 发布时传入的数据指针（方案一中该值不用于回调）
} msg_t;

static msg_t s_queue[PUBSUB_QUEUE_SIZE]; ///< 队列存储区
static int s_head = 0;   ///< 队首索引（读取位置）
static int s_tail = 0;   ///< 队尾索引（写入位置）
static int s_count = 0;  ///< 队列中当前消息数量

/*==================== API 实现 ====================*/

/**
 * @brief 订阅事件
 */
int ps_subscribe(int event_id, ps_callback_t cb, void *args) {
    if (!cb) return -1;   // 回调不能为空
    
    event_t *evt = find_or_create_event(event_id);
    if (!evt || evt->count >= MAX_SUBSCRIBERS) {
        return -1;        // 事件表满或该事件订阅者已满
    }
    
    // 将订阅者信息存入列表
    evt->subscribers[evt->count].cb = cb;
    evt->subscribers[evt->count].args = args;
    evt->count++;
    
    return 0;
}

/**
 * @brief 取消订阅
 */
int ps_unsubscribe(int event_id, ps_callback_t cb) {
    event_t *evt = find_event(event_id);
    if (!evt) return -1;
    
    for (int i = 0; i < evt->count; i++) {
        if (evt->subscribers[i].cb == cb) {
            // 用最后一个订阅者覆盖当前位置（高效删除，不保持顺序）
            evt->subscribers[i] = evt->subscribers[--evt->count];
            return 0;
        }
    }
    return -1;   // 未找到该回调
}

/**
 * @brief 发布事件（入队，非阻塞）
 */
int ps_publish(int event_id, void *args) {
    int ret = -1;
    
    ps_enter_critical();  // 保护队列操作
    
    if (s_count < PUBSUB_QUEUE_SIZE) {
        s_queue[s_tail].event_id = event_id;
        s_queue[s_tail].args = args;   // 存储发布数据（方案一中仅作占位）
        s_tail = (s_tail + 1) % PUBSUB_QUEUE_SIZE;
        s_count++;
        ret = 0;
    }
    // 队列满则返回 -1
    
    ps_exit_critical();
    return ret;
}

/**
 * @brief 处理队列中的所有待处理事件
 * @note  本函数会从队列中取出事件并执行对应的回调。
 *        采用方案一：回调收到的 args 是订阅时传入的上下文，
 *        发布时传入的 args 被丢弃（使用 (void)args; 消除警告）。
 *        若希望回调接收发布数据，请改用方案二。
 */
void ps_process(void) {
    int event_id;
    void *args;
    event_t *evt;
    
    // 1. 从队列中取出一个消息（临界区保护）
    ps_enter_critical();
    if (s_count == 0) {
        ps_exit_critical();
        return;
    }
    
    event_id = s_queue[s_head].event_id;
    args = s_queue[s_head].args;      // 取出发布数据（本例中不使用）
    s_head = (s_head + 1) % PUBSUB_QUEUE_SIZE;
    s_count--;
    ps_exit_critical();
    
    // ★ 消除编译警告：明确告知编译器 args 变量有意未使用 ★
    (void)args;   // 方案一：发布数据不被回调使用，故此处显式忽略
    
    // 2. 查找事件并执行所有订阅者的回调（临界区外执行，允许中断）
    evt = find_event(event_id);
    if (evt) {
        for (int i = 0; i < evt->count; i++) {
            if (evt->subscribers[i].cb) {
                // ★ 方案一关键：回调收到的参数是订阅时传入的 args ★
                evt->subscribers[i].cb(evt->subscribers[i].args);
            }
        }
    }
}

/**
 * @brief 获取待处理事件数量
 */
int ps_pending(void) {
    int cnt;
    ps_enter_critical();
    cnt = s_count;
    ps_exit_critical();
    return cnt;
}

