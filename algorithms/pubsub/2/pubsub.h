/**
 * @file pubsub.h
 * @brief 超轻量级发布-订阅模块（带环形缓冲区，非阻塞发布）
 * @note  方案二：回调函数收到的 data 是发布时传入的数据指针。
 *        适合数据流风格：每次事件触发都携带不同的数据（如温度值、按键码）。
 * @warning 发布时传入的数据指针在回调执行时必须保持有效！
 *          建议传递全局变量、静态变量，或将小数据强转为指针传递。
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
 * @param data 发布时传入的数据指针（由发布者提供）
 * @note  回调函数负责将 data 转换为实际类型使用。
 *        请确保 data 指向的内存生命周期足够长（全局/静态/传值）。
 */
typedef void (*ps_callback_t)(void *data);

/*==================== API 函数 ====================*/

/**
 * @brief 订阅一个事件
 * @param event_id 事件 ID（由用户定义，建议用枚举）
 * @param cb       回调函数指针
 * @return 0 成功，-1 失败（事件或订阅者数量已达上限）
 * @note  同一个事件可以被多个订阅者订阅。
 *        取消订阅时必须传入与订阅时相同的回调函数指针。
 */
int ps_subscribe(int event_id, ps_callback_t cb);

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
 * @param data     发布时携带的数据指针（会被传递给所有订阅者的回调）
 * @return 0 成功入队，-1 队列已满（发布失败）
 * @note  本函数可在中断和任务上下文中安全调用（内部有关中断保护）。
 *        ★ 安全传递数据的方法：
 *          1. 小数据（整数/枚举）：用 (void*)(uintptr_t)value 直接传值
 *          2. 大数据（结构体）：必须传全局/静态变量的地址，或用 malloc
 *             （但嵌入式不建议动态内存），推荐用全局变量。
 */
int ps_publish(int event_id, void *data);

/**
 * @brief 处理队列中所有待处理的事件（同步执行回调）
 * @note  必须定期在主循环或低优先级任务中调用本函数。
 *        回调函数在本函数内被依次调用，执行上下文为调用本函数的线程/任务。
 *        回调收到的 data 参数正是发布时传入的 data 指针。
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

///* 定义事件 ID */
//typedef enum {
//    EVENT_BUTTON,       // 按键事件（携带按键码）
//    EVENT_TEMPERATURE,  // 温度更新事件（携带温度值）
//    EVENT_UART_RX,      // 串口接收事件（携带接收到的字符）
//} EventId;

///* 如果需要传递结构体，定义全局变量（确保生命周期足够长） */
//typedef struct {
//    float temp;
//    uint8_t sensor_id;
//} TempData;

//static TempData g_temp_data;   // 全局变量，生命周期为整个程序

///* ========== 回调函数 ========== */

///**
// * @brief 按键回调
// * @param data 发布时传入的按键码（小整数，直接强转为指针传递）
// */
//void on_button_pressed(void *data) {
//    // 将指针还原为整数值
//    int key_code = (int)(uintptr_t)data;
//    printf("Button %d pressed!\r\n", key_code);
//}

///**
// * @brief 温度回调
// * @param data 发布时传入的 TempData 结构体指针
// */
//void on_temperature_update(void *data) {
//    TempData *p = (TempData*)data;
//    printf("Sensor #%d: %.2f°C\r\n", p->sensor_id, p->temp);
//}

///**
// * @brief 串口回调
// * @param data 发布时传入的字符（强转为指针传递）
// */
//void on_uart_rx(void *data) {
//    char ch = (char)(uintptr_t)data;
//    printf("UART received: '%c' (0x%02X)\r\n", ch, ch);
//}

///* ========== 主函数 ========== */

//int main(void) {
//    // 1. 系统初始化...
//    
//    // 2. 订阅事件（方案二：无需传入上下文）
//    ps_subscribe(EVENT_BUTTON, on_button_pressed);
//    ps_subscribe(EVENT_TEMPERATURE, on_temperature_update);
//    ps_subscribe(EVENT_UART_RX, on_uart_rx);
//    
//    // 3. 主循环
//    while (1) {
//        // ---------- 模拟：按键按下，发布按键码 ----------
//        // ★ 小数据直接强转为指针传递（零拷贝，绝对安全） ★
//        ps_publish(EVENT_BUTTON, (void*)(uintptr_t)10);   // 按键码 10
//        ps_publish(EVENT_BUTTON, (void*)(uintptr_t)20);   // 按键码 20
//        
//        // ---------- 模拟：温度传感器更新 ----------
//        g_temp_data.sensor_id = 1;
//        g_temp_data.temp = 26.5f;
//        // ★ 传递全局变量的地址（确保在回调执行时该变量仍然有效） ★
//        ps_publish(EVENT_TEMPERATURE, &g_temp_data);
//        
//        // ---------- 模拟：串口收到字符 ----------
//        ps_publish(EVENT_UART_RX, (void*)(uintptr_t)'A');
//        ps_publish(EVENT_UART_RX, (void*)(uintptr_t)'B');
//        
//        // ★ 核心：处理所有待处理的事件（回调在这里被执行） ★
//        ps_process();
//        
//        // 其他任务...
//        delay_ms(100);
//    }
//}

