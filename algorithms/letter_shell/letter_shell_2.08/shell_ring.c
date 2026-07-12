#include "shell_ring.h"

/* 内部环形队列结构（静态单例） */
static struct {
    uint8_t buffer[SHELL_RING_SIZE];
    size_t head;      /* 指向下一个出队位置 */
    size_t tail;      /* 指向下一个入队位置 */
    size_t count;     /* 当前队列中元素个数 */
} ring = {0};

void shell_ring_init(void)
{
    ring.head = 0;
    ring.tail = 0;
    ring.count = 0;
}

size_t shell_ring_enqueue(const uint8_t *data, size_t count)
{
    if (data == NULL || count == 0) {
        return 0;
    }

    size_t enqueued = 0;
    while (enqueued < count && ring.count < SHELL_RING_SIZE) {
        ring.buffer[ring.tail] = data[enqueued];
        ring.tail = (ring.tail + 1) % SHELL_RING_SIZE;
        ring.count++;
        enqueued++;
    }
    return enqueued;
}

size_t shell_ring_dequeue(uint8_t *data, size_t count)
{
    if (data == NULL || count == 0) {
        return 0;
    }

    size_t dequeued = 0;
    while (dequeued < count && ring.count > 0) {
        data[dequeued] = ring.buffer[ring.head];
        ring.head = (ring.head + 1) % SHELL_RING_SIZE;
        ring.count--;
        dequeued++;
    }
    return dequeued;
}

