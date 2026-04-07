#include <stdio.h>
#include <algorithms/algorithms_linked_list.h>
#include <stdlib.h>
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
 * 用于创建链表元素实际内存
 * @param dat_num
 * @param det_size
 * @param data
 * @return
 */
linked_list_element* new_linked_list_element(uint16_t dat_num,uint16_t det_size,const void* data) {
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
 * @param l
 * @return
 */
int8_t destroy_linked_list_element(linked_list_element* l) {
    if (l == NULL) return -1;
    free(l->linked_list_data_addr);
    free(l);
    return 0;
}

/**
 * 插入链表元素
 * @param prev
 * @param next
 * @param l
 * @return
 */
int8_t insertion_linked_list_element(linked_list_element* prev,linked_list_element* next,linked_list_element* l) {
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
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @return
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
 * 在当前链表新增节点
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @return
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
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @return
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
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @param insertion_data
 * @param insertion_dat_num
 * @param insertion_det_size
 * @return
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
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @return
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
 * @param l
 * @param dat_num
 * @param det_size
 * @param data
 * @param alter_data
 * @param alter_dat_num
 * @param alter_det_size
 * @return
 */
int8_t alter_linked_list_node(linked_list* l, uint16_t dat_num, uint16_t det_size, const void* data,
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
    return 0;
}

/**
 * 销毁整个链表（释放所有节点，并将链表句柄重置为初始状态）
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




void test_linked_list(void) {
    printf("Linked list test begins\n");
}

