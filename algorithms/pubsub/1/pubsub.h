/**
 * @file pubsub.h
 * @brief 超轻量级发布-订阅模块（带环形缓冲区，非阻塞发布）
 * @note  方案一：回调函数收到的 args 是订阅时传入的上下文指针，
 *        发布时的 args 仅用于触发事件，不被回调使用。
 *        适合面向对象风格：例如订阅时传入设备对象，事件触发后操作该对象。
 */

#ifndef _PUBSUB_H_
#define _PUBSUB_H_

#include <stdint.h>

/*==================== 配置参数（可根据实际需求调整） ====================*/

/**
 * @def MAX_EVENTS
 * @brief 系统中支持的最大事件 ID 数量（不同事件的种类数）
 */
#define MAX_EVENTS         5

/**
 * @def MAX_SUBSCRIBERS
 * @brief 每个事件允许的最大订阅者数量
 */
#define MAX_SUBSCRIBERS    5

/**
 * @def PUBSUB_QUEUE_SIZE
 * @brief 发布消息队列的深度（缓冲多少个待处理事件）
 */
#define PUBSUB_QUEUE_SIZE  10

/*==================== 类型定义 ====================*/

/**
 * @typedef ps_callback_t
 * @brief 事件回调函数类型
 * @param args 订阅时传入的上下文指针（由订阅者提供）
 */
typedef void (*ps_callback_t)(void *args);

/*==================== API 函数 ====================*/

/**
 * @brief 订阅一个事件
 * @param event_id 事件 ID（由用户定义，建议用枚举）
 * @param cb       回调函数指针
 * @param args     回调函数收到的上下文指针（可为 NULL）
 * @return 0 成功，-1 失败（事件或订阅者数量已达上限）
 * @note  同一个事件可以被多个订阅者订阅，同一个回调函数可订阅多个事件。
 *        取消订阅时必须传入与订阅时相同的回调函数指针。
 */
int ps_subscribe(int event_id, ps_callback_t cb, void *args);

/**
 * @brief 取消订阅
 * @param event_id 事件 ID
 * @param cb       需要取消的回调函数指针（必须与订阅时一致）
 * @return 0 成功，-1 失败（事件不存在或未找到该回调）
 */
int ps_unsubscribe(int event_id, ps_callback_t cb);

/**
 * @brief 发布一个事件（非阻塞）
 * @param event_id 事件 ID
 * @param args     发布时携带的数据指针（方案一中该参数不会被回调使用，
 *                 仅作为“事件发生”的触发信号，但会存入队列并传递，可传 NULL）
 * @return 0 成功入队，-1 队列已满（发布失败）
 * @note  本函数可在中断和任务上下文中安全调用（内部有关中断保护）。
 */
int ps_publish(int event_id, void *args);

/**
 * @brief 处理队列中所有待处理的事件（同步执行回调）
 * @note  必须定期在主循环或低优先级任务中调用本函数。
 *        回调函数在本函数内被依次调用，执行上下文为调用本函数的线程/任务。
 *        请确保回调函数执行时间短，避免阻塞系统。
 */
void ps_process(void);

/**
 * @brief 获取当前队列中等待处理的事件数量
 * @return 待处理事件个数
 */
int ps_pending(void);

#endif /* _PUBSUB_H_ */



//#include "pubsub.h"
//#include <stdio.h>

///* 定义事件 ID（使用枚举） */
//typedef enum {
//    EVENT_BUTTON,       // 按键事件
//    EVENT_TEMPERATURE,  // 温度更新事件
//    EVENT_UART_RX,      // 串口接收事件
//} EventId;

///* 订阅上下文：按键事件需要知道引脚编号 */
//typedef struct {
//    uint8_t pin;
//    char name[8];
//} ButtonContext;

///* 全局上下文对象（生命周期贯穿程序） */
//static ButtonContext button_ctx = { .pin = 10, .name = "KEY1" };

///* 回调函数示例 */
//void on_button_pressed(void *ctx) {
//    ButtonContext *p = (ButtonContext*)ctx;
//    printf("Button %s (pin %d) pressed!\r\n", p->name, p->pin);
//}

//void on_temperature_update(void *ctx) {
//    float temp = *(float*)ctx;  // 注意：这里是订阅时传入的数据指针
//    printf("New temp: %.2f°C\r\n", temp);
//}

///* 全局温度变量（作为订阅上下文） */
//static float current_temperature = 0.0f;

//int main(void) {
//    // 1. 系统初始化（时钟、外设等）...
//    
//    // 2. 订阅事件
//    ps_subscribe(EVENT_BUTTON, on_button_pressed, &button_ctx);
//    ps_subscribe(EVENT_TEMPERATURE, on_temperature_update, &current_temperature);
//    
//    // 3. 主循环
//    while (1) {
//        // 模拟：检测到按键按下，发布事件（发布数据设为 NULL，因为方案一不使用）
//        ps_publish(EVENT_BUTTON, NULL);
//        
//        // 模拟：温度传感器更新，发布事件（发布数据可传任意值，但回调不用它）
//        // 若发布数据不影响回调，则传 NULL 或随便传个值均可
//        ps_publish(EVENT_TEMPERATURE, NULL); 
//        
//        // ★ 核心：处理所有待处理的事件（回调在这里被调用）★
//        ps_process();
//        
//        // 其他任务...
//        delay_ms(100);
//    }
//}


