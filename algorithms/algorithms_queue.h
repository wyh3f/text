#ifndef ALGORITHMS_QUEUE_H_
#define ALGORITHMS_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//#include <string.h>


/**
 * 队列句柄
 */
typedef struct queue_element {
    uint16_t queue_det_num;     //队列的元素个数
    uint16_t queue_det_size;    //队列的元素大小
    uint16_t queue_head;        //队列首
    uint16_t queue_tail;        //队列尾
    void* queue_data_addr;      //指向队列的实际内存地址
}queue;


/**
 * 自动新建队列
 * @param q 队列句柄
 * @param queue_det_num 队列的元素数量
 * @param queue_det_size 队列的元素大小
 * @return 返回-1创建失败 返回0创建成功
 */
int8_t new_queue(queue* q,uint16_t queue_det_num,uint16_t queue_det_size);

/**
 * 手动新建队列
 * @param q 队列句柄地址
 * @param queue_det_num 队列的元素数量
 * @param queue_det_size 队列的元素大小
 * @param data_addr 队列实际存储地址
 * @return 返回-1创建失败 返回0创建成功
 */
int8_t artificial_new_queue(queue* q,uint16_t queue_det_num,uint16_t queue_det_size,void* data_addr);

/**
 * 入队
 * @param q 队列句柄地址
 * @param data_q 入队元素地址
 * @return 返回-1失败 返回0成功
 */
int8_t enqueue(queue* q,const void* data_q);

/**
 * 出队
 * @param q 队列句柄地址
 * @param out_data_q 出队元素存储地址
 * @return 返回-1失败 返回0成功
 */
int8_t dequeue(queue* q,void* out_data_q);

/**
 * 计算队列空闲元素数量
 * @param q 队列句柄地址
 * @return 返回队列空闲元素数量
 */
uint16_t queue_free_slots(const queue* q);

/**
 * 销毁队列
 * @param q 队列句柄地址
 * @return 返回-1失败 返回0成功
 */
int8_t destroy_queue(queue* q);



void test_queue(void);

#ifdef __cplusplus
}
#endif

#endif

