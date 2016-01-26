#ifndef _FLOW_LIST_H_
#define _FLOW_LIST_H_

#define container_of(ptr, type, member) \
    ((type*)((char*)ptr - (size_t)(&((type*)0)->member)))
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

struct list_head {
    struct list_head *prev, *next;
};

static inline void list_head_init(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

static inline void list_add(struct list_head *_new, struct list_head *head)
{
    _new->next = head->next;
    _new->prev = head;
    head->next->prev = _new;
    head->next = _new;
}

static inline void list_add_tail(struct list_head *_new, struct list_head *head)
{
    _new->next = head;
    _new->prev = head->prev;
    head->prev->next = _new;
    head->prev = _new;
}

static inline void list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

static inline int list_empty(struct list_head *head)
{
    return head->next == head;
}

static inline int list_size(struct list_head *head)
{
    int i = 0;
    struct list_head *tmp = head;
    while (tmp->next != head) {
        tmp = tmp->next;
        i++;
    }
    return i;
}

#endif
