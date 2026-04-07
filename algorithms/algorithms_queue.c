#include <stdio.h>
#include <algorithms/algorithms_queue.h>
#include <stdlib.h>
#include <string.h>




/**
 * 自动新建队列
 * @param q 队列句柄
 * @param queue_det_num 队列的元素数量
 * @param queue_det_size 队列的元素大小
 * @return 返回-1创建失败 返回0创建成功
 */
int8_t new_queue(queue* q,uint16_t queue_det_num,uint16_t queue_det_size) {
    if((q == NULL)||(queue_det_num == 0)||(queue_det_size == 0)) return -1;
    q->queue_data_addr = calloc(queue_det_num,queue_det_size);
    if(q->queue_data_addr == NULL) return -1;
    q->queue_det_num = queue_det_num;
    q->queue_det_size = queue_det_size;
    q->queue_head = 0;
    q->queue_tail = 0;
    return 0;
}

/**
 * 手动新建队列
 * @param q 队列句柄地址
 * @param queue_det_num 队列的元素数量
 * @param queue_det_size 队列的元素大小
 * @param data_addr 队列实际存储地址
 * @return 返回-1创建失败 返回0创建成功
 */
int8_t artificial_new_queue(queue* q,uint16_t queue_det_num,uint16_t queue_det_size,void* data_addr) {
    if((q == NULL)||(queue_det_num == 0)||(queue_det_size == 0)||(data_addr == NULL)) return -1;
    q->queue_data_addr = data_addr;
    q->queue_det_num = queue_det_num;
    q->queue_det_size = queue_det_size;
    q->queue_head = 0;
    q->queue_tail = 0;
    return 0;
}

/**
 * 入队
 * @param q 队列句柄地址
 * @param data_q 入队元素地址
 * @return 返回-1失败 返回0成功
 */
int8_t enqueue(queue* q,const void* data_q) {
    if ((q ==NULL)||(data_q == NULL)) return -1;
    if (q->queue_data_addr == NULL) return -1;
    if (((q->queue_head + 1) % q->queue_det_num) == q->queue_tail) return -1;
    memcpy((void *)((int8_t*)q->queue_data_addr + (q->queue_head * q->queue_det_size)),data_q,q->queue_det_size);
    q->queue_head = (q->queue_head + 1) % q->queue_det_num;
    return 0;
}

/**
 * 出队
 * @param q 队列句柄地址
 * @param out_data_q 出队元素存储地址
 * @return 返回-1失败 返回0成功
 */
int8_t dequeue(queue* q,void* out_data_q) {
    if ((q ==NULL)||(out_data_q == NULL)) return -1;
    if (q->queue_data_addr == NULL) return -1;
    if(q->queue_head == q->queue_tail) return -1;
    memcpy(out_data_q,(void *)((int8_t*)q->queue_data_addr + (q->queue_tail * q->queue_det_size)),q->queue_det_size);
    memset((void *)((int8_t*)q->queue_data_addr + (q->queue_tail * q->queue_det_size)),0,q->queue_det_size);
    q->queue_tail = (q->queue_tail + 1) % q->queue_det_num;
    return 0;
}

/**
 * 计算队列空闲元素数量
 * @param q 队列句柄地址
 * @return 返回队列空闲元素数量
 */
uint16_t queue_free_slots(const queue* q) {
    if ((q == NULL) || (q->queue_data_addr == NULL)) return 0;
    return q->queue_det_num - 1 - (q->queue_head - q->queue_tail + q->queue_det_num) % q->queue_det_num;
}

/**
 * 销毁队列
 * @param q 队列句柄地址
 * @return 返回-1失败 返回0成功
 */
int8_t destroy_queue(queue* q) {
    if (q == NULL) return -1;
    if (q->queue_data_addr != NULL) {
        free(q->queue_data_addr);
        q->queue_data_addr = NULL;
    }
    q->queue_det_num = 0;
    q->queue_det_size = 0;
    q->queue_head = 0;
    q->queue_tail = 0;
    return 0;
}


void test_queue(void) {
    printf("Queue test begins.\n");
    queue q;
    double a_buf_begins = 1.01001;
    double a_buf_begins_2 = 100.5;;
    new_queue(&q,20,8);
    enqueue(&q,(void*)&a_buf_begins);
    a_buf_begins=0;
    dequeue(&q,(void*)&a_buf_begins_2);
    printf("queue_num %d,out %f\n",queue_free_slots(&q),a_buf_begins_2);
    destroy_queue(&q);
    printf("Queue test ends.\n");
}



