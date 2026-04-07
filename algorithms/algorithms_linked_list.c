#include <stdio.h>
#include <algorithms/algorithms_linked_list.h>
#include <stdlib.h>
#include <string.h>




/**
 * 用于创建链表元素实际内存
 * @param dat_num 链表元素指向地址的元素个数
 * @param det_size 链表元素指向地址的元素大小
 * @param data 需要拷贝的数据
 * @return 申请的链表元素地址
 */
static linked_list_element* new_linked_list_element(uint16_t dat_num,uint16_t det_size,const void* data) {
    if ((dat_num==0) || (det_size==0)) return NULL;
    linked_list_element* l = (linked_list_element*)calloc(1,sizeof(linked_list_element));
    if(l == NULL) return NULL;
    l->linked_list_data_addr = calloc(dat_num,det_size);
    if(l->linked_list_data_addr == NULL) {
        free(l);
        return NULL;
    }
    l->linked_list_dat_num=dat_num;
    l->linked_list_det_size=det_size;
    l->prev = NULL;
    l->next = NULL;
    if (data!=NULL)memcpy(l->linked_list_data_addr,data,dat_num*det_size);
    return l;
}

/**
 * 销毁链表元素实际内存
 * @param l 链表元素地址
 * @return 返回0成功 返回-1失败
 */
static int8_t destroy_linked_list_element(linked_list_element* l) {
    if (l == NULL) return -1;
    free(l->linked_list_data_addr);
    free(l);
    return 0;
}

/**
 * 插入链表元素
 * @param prev 前一个链表元素
 * @param next 后一个链表元素
 * @param l 需要插入的元素
 * @return 返回0成功 返回-1失败
 */
static int8_t insertion_linked_list_element(linked_list_element* prev,linked_list_element* next,linked_list_element* l) {
    if ((prev == NULL)||(next == NULL)||(l == NULL)) return -1;
    prev->next = l;
    l->prev = prev;
    l->next = next;
    next->prev = l;
    return 0;
}


/*
 * -----------------------------------以下为链表操作函数-------------------------------------
 */


/**
 * 初始化列表并建立头节点
 * @param l 链表句柄
 * @param dat_num 链表实际地址存储的元素个数
 * @param det_size 链表实际地址存储的元素大小
 * @param data 实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t init_linked_list(linked_list* l,uint16_t dat_num,uint16_t det_size,const void* data) {
    if ((l == NULL)||(l->linked_list_head_addr != NULL)||(l->linked_list_num != 0)) return -1;
    l->linked_list_head_addr = new_linked_list_element(dat_num,det_size,data);
    if (l->linked_list_head_addr == NULL) return -1;
    l->element_pointer = l->linked_list_head_addr;
    l->linked_list_num = 1;
    l->linked_list_head_addr->prev = l->linked_list_head_addr;
    l->linked_list_head_addr->next = l->linked_list_head_addr;
    return 0;
}

/**
 * 在当前链表末尾新增节点
 * @param l 链表句柄
 * @param dat_num 链表实际地址存储的元素个数
 * @param det_size 链表实际地址存储的元素大小
 * @param data 实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t add_linked_list(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data) {
    if (l == NULL || l->linked_list_head_addr == NULL) return -1;
    linked_list_element* new_node = new_linked_list_element(dat_num, det_size, data);
    if (new_node == NULL) return -1;
    linked_list_element* head = l->linked_list_head_addr;
    linked_list_element* tail = head->prev;
    if (insertion_linked_list_element(tail, head, new_node) != 0) {
        destroy_linked_list_element(new_node);  // 插入失败则释放
        return -1;
    }
    l->linked_list_num++;
    return 0;
}

/**
 * 遍历查找链表节点
 * @param l 链表句柄
 * @param dat_num 需要查找的链表实际地址存储的元素个数
 * @param det_size 需要查找的链表实际地址存储的元素大小
 * @param data 需要查找的实际存储的数据
 * @return 返回查找的链表元素地址，通过元素地址得到对应数据首地址
 */
linked_list_element* ergodic_linked_list(const linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data) {
    if (l == NULL || l->linked_list_head_addr == NULL || dat_num == 0||det_size == 0||data == NULL) return NULL;
    linked_list_element* cur = l->linked_list_head_addr;
    for (uint16_t i = 0; i < l->linked_list_num; i++) {
        if (cur->linked_list_dat_num*cur->linked_list_det_size>=det_size*dat_num) {
            if (memcmp(data,cur->linked_list_data_addr,dat_num*det_size)==0) {
                return cur;
            }
        }
        cur=cur->next;
    }
    return NULL;
}

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
                             const void* insertion_data, uint16_t insertion_dat_num, uint16_t insertion_det_size) {
    if (l == NULL || l->linked_list_head_addr == NULL || dat_num == 0 || det_size == 0 || data == NULL ||
        insertion_data == NULL || insertion_dat_num == 0 || insertion_det_size == 0)
        return -1;

    linked_list_element* buf = ergodic_linked_list(l, dat_num, det_size, data);
    if (buf == NULL) return -1;

    linked_list_element* new_node = new_linked_list_element(insertion_dat_num, insertion_det_size, insertion_data); // 修正
    if (new_node == NULL) return -1;

    if (insertion_linked_list_element(buf, buf->next, new_node) != 0) {
        destroy_linked_list_element(new_node);
        return -1;
    }

    l->linked_list_num++;
    return 0;
}

/**
 * 删除对应内容
 * @param l 链表句柄
 * @param dat_num 需要查找删除的链表实际地址存储的元素个数
 * @param det_size 需要查找删除的链表实际地址存储的元素大小
 * @param data 需要查找删除的实际存储的数据
 * @return 返回0成功 返回-1失败
 */
int8_t destroy_linked_list_node(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data) {
    if (l == NULL || l->linked_list_head_addr == NULL || dat_num == 0 || det_size == 0 || data == NULL) return -1;

    linked_list_element* buf = ergodic_linked_list(l, dat_num, det_size, data);
    if (buf == NULL) return -1;
    if (l->element_pointer == buf) {
        l->element_pointer = buf->next;
        if (l->element_pointer == buf)l->element_pointer = NULL;
    }
    linked_list_element* prev = buf->prev;
    linked_list_element* next = buf->next;

    if (l->linked_list_head_addr == buf) {
        l->linked_list_head_addr = (buf->next == buf) ? NULL : next;
    }
    prev->next = next;
    next->prev = prev;
    destroy_linked_list_element(buf);
    l->linked_list_num--;
    if (l->linked_list_num == 0) {
        l->linked_list_head_addr = NULL;
        l->element_pointer = NULL;
    }

    return 0;
}


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
                              const void* alter_data, uint16_t alter_dat_num, uint16_t alter_det_size) {
    if (l == NULL || l->linked_list_head_addr == NULL || dat_num == 0 || det_size == 0 || data == NULL ||
        alter_data == NULL || alter_dat_num == 0 || alter_det_size == 0)
        return -1;

    linked_list_element* buf = ergodic_linked_list(l, dat_num, det_size, data);
    if (buf == NULL) return -1;

    size_t new_size = (size_t)alter_dat_num * alter_det_size;
    size_t old_size = (size_t)buf->linked_list_dat_num * buf->linked_list_det_size;

    if (new_size > old_size) {
        return -1;
    }
    memcpy(buf->linked_list_data_addr, alter_data, new_size);
    buf->linked_list_dat_num = alter_dat_num;   // 添加
    buf->linked_list_det_size = alter_det_size; // 添加
    return 0;
}

/**
 * 销毁整个链表（释放所有节点，并将链表句柄重置为初始状态）
 * @param l 链表句柄
 */
void destroy_linked_list(linked_list* l) {
    if (!l || !l->linked_list_head_addr) return;
    linked_list_element* cur = l->linked_list_head_addr;
    linked_list_element* next = NULL;
    do {
        next = cur->next;
        destroy_linked_list_element(cur);
        cur = next;
    } while (cur != l->linked_list_head_addr);
    l->linked_list_head_addr = NULL;
    l->element_pointer = NULL;
    l->linked_list_num = 0;
}


// 辅助函数：打印链表中的所有 int 数据（循环链表，遍历计数次）
static void print_linked_list_int(const linked_list* l) {
    if (!l|| l->linked_list_head_addr == NULL) {
        printf("linked_list->no\n");
        return;
    }
    linked_list_element* cur = l->linked_list_head_addr;
    printf("linked_list %d node: ", l->linked_list_num);
    for (uint16_t i = 0; i < l->linked_list_num; i++) {
        // 假设存储的是单个 int
        int* val = (int*)cur->linked_list_data_addr;
        printf("%d ", *val);
        cur = cur->next;
    }
    printf("\n");
}

// 测试示例
void test_linked_list(void) {
    printf("-----------------\n");
    printf("Linked list test begins\n");
    // 1. 初始化链表句柄（必须清零）
    linked_list myList = {0};
    // 2. 创建头节点，存储数值 10
    if (init_linked_list(&myList, 1, sizeof(int), &(int){10}) != 0) {
        printf("init be defeated end\n");
        return;
    }
    printf("init: ");
    print_linked_list_int(&myList);  // 输出: 10

    // 3. 尾部追加节点
    add_linked_list(&myList, 1, sizeof(int), &(int){20});
    add_linked_list(&myList, 1, sizeof(int), &(int){30});
    add_linked_list(&myList, 1, sizeof(int), &(int){40});
    printf("add 3 node: ");
    print_linked_list_int(&myList);  // 输出: 10 20 30 40

    // 4. 查找并插入（在值为 20 的节点后插入 25）
    if (insertion_linked_list(&myList, 1, sizeof(int), &(int){20},
                              &(int){25}, 1, sizeof(int)) == 0) {
        printf(" 20 insertion 25 : ");
        print_linked_list_int(&myList);  // 输出: 10 20 25 30 40
    } else {
        printf("insertion be defeated（no look for 20）\n");
    }

    // 5. 修改节点：将值为 30 的节点改为 35（注意：必须保持修改前后数据大小相同或更小）
    if (alter_linked_list_node(&myList, 1, sizeof(int), &(int){30},
                               &(int){35}, 1, sizeof(int)) == 0) {
        printf("alter 30 -> 35 : ");
        print_linked_list_int(&myList);  // 输出: 10 20 25 35 40
    } else {
        printf("alter be defeated（no look for 30）\n");
    }

    // 6. 删除节点：删除值为 25 的节点
    if (destroy_linked_list_node(&myList, 1, sizeof(int), &(int){25}) == 0) {
        printf("delete 25 : ");
        print_linked_list_int(&myList);  // 输出: 10 20 35 40
    } else {
        printf("delete be defeated \n");
    }

    // 7. 再次删除头节点（值为 10）
    if (destroy_linked_list_node(&myList, 1, sizeof(int), &(int){10}) == 0) {
        printf("delete 10 : ");
        print_linked_list_int(&myList);  // 输出: 20 35 40
    }

    // 8. 演示查找函数（前缀匹配）: 查找值为 35 的数据（注意：由于前缀匹配，会匹配到 35 吗？不会，因为 35 的第一个字节与 3 不相等）
    linked_list_element* found = ergodic_linked_list(&myList, 1, sizeof(int), &(int){35});
    if (found) {
        printf("look for 35 node\n");
    } else {
        printf("on look for 35 node\n");
    }

    // 9. 删除剩余所有节点（逐个删或直接销毁整个链表）
    destroy_linked_list(&myList);
    printf("no linked_list: ");
    print_linked_list_int(&myList);  // 输出: 链表为空

    printf("========== linked_list end ==========\n");
}



