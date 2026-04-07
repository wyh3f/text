#ifndef ALGORITHMS_LINKED_LIST_H_
#define ALGORITHMS_LINKED_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

/**
 * 链表元素
 */
typedef struct linked_list_element {
    void* linked_list_data_addr;
    uint16_t linked_list_dat_num;
    uint16_t linked_list_det_size;
    struct linked_list_element* prev;
    struct linked_list_element* next;
}linked_list_element;


/**
 * 链表句柄
 */
typedef struct linked_list {
    linked_list_element* element_pointer;
    linked_list_element* linked_list_head_addr;
    uint16_t linked_list_num;
}linked_list;



/**
 * 初始化列表并建立头节点
 * @param l 链表句柄
 * @param dat_num 链表实际地址存储的元素个数
 * @param det_size 链表实际地址存储的元素大小
 * @param data 实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t init_linked_list(linked_list* l,uint16_t dat_num,uint16_t det_size,const void* data);

/**
 * 在当前链表末尾新增节点
 * @param l 链表句柄
 * @param dat_num 链表实际地址存储的元素个数
 * @param det_size 链表实际地址存储的元素大小
 * @param data 实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t add_linked_list(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data);

/**
 * 遍历查找链表节点
 * @param l 链表句柄
 * @param dat_num 需要查找的链表实际地址存储的元素个数
 * @param det_size 需要查找的链表实际地址存储的元素大小
 * @param data 需要查找的实际存储的数据
 * @return 返回查找的链表元素地址，通过元素地址得到对应数据首地址
 */
linked_list_element* ergodic_linked_list(const linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data);

/**
 * 根据内容查找并插入链表(根据内容新建一个链表元素插入)
 * @param l 链表句柄
 * @param dat_num 需要查找的链表实际地址存储的元素个数
 * @param det_size 需要查找的链表实际地址存储的元素大小
 * @param data 需要查找的实际存储的数据
 * @param insertion_data 需要插入的需要查找的实际存储的数据
 * @param insertion_dat_num 需要插入的链表实际地址存储的元素个数
 * @param insertion_det_size 需要插入的链表实际地址存储的元素大小
 * @return 返回0成功 返回-1失败
 */
int8_t insertion_linked_list(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data,
                             const void* insertion_data, uint16_t insertion_dat_num, uint16_t insertion_det_size);

/**
 * 删除对应内容
 * @param l 链表句柄
 * @param dat_num 需要查找删除的链表实际地址存储的元素个数
 * @param det_size 需要查找删除的链表实际地址存储的元素大小
 * @param data 需要查找删除的实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t destroy_linked_list_node(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data);


/**
 * 根据内容修改
 * @param l 链表句柄
 * @param dat_num 需要查找的链表实际地址存储的元素个数
 * @param det_size 需要查找的链表实际地址存储的元素大小
 * @param data 需要查找的实际存储的数据
 * @param alter_data 需要插入的实际存储的数据
 * @param alter_dat_num 需要插入的链表实际地址存储的元素个数
 * @param alter_det_size 需要插入的链表实际地址存储的元素大小
 * @return 返回0成功 返回-1失败
 */
int8_t alter_linked_list_node(const linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data,
                              const void* alter_data, uint16_t alter_dat_num, uint16_t alter_det_size);

/**
 * 销毁整个链表（释放所有节点，并将链表句柄重置为初始状态）
 * @param l 链表句柄
 */
void destroy_linked_list(linked_list* l);

void test_linked_list(void);


#ifdef __cplusplus
}
#endif

#endif

