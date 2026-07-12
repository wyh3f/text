#ifndef SHELL_RING_H
#define SHELL_RING_H

#include <stdint.h>
#include <stddef.h>

/* 环形队列大小，可在编译前通过 -D 或在此处修改 */
#ifndef SHELL_RING_SIZE
#define SHELL_RING_SIZE  256
#endif

/**
 * @brief 初始化环形队列（重置为空状态）
 */
void shell_ring_init(void);

/**
 * @brief 将数据入队
 * @param data  待写入的数据指针
 * @param count 期望写入的字节数
 * @return 实际成功入队的字节数（可能小于 count，当队列满时）
 */
size_t shell_ring_enqueue(const uint8_t *data, size_t count);

/**
 * @brief 从队列中取出数据
 * @param data  用于存储读出数据的缓冲区指针
 * @param count 期望读出的字节数
 * @return 实际成功出队的字节数（可能小于 count，当队列空时）
 */
size_t shell_ring_dequeue(uint8_t *data, size_t count);

#endif /* SHELL_RING_H */

